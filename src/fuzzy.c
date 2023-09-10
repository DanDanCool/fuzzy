#include "fuzzy.h"
#include "platform.h"
#include "task.h"

#include <scheduler.h>
#include <memory.h>

enum {
	STATE_NEW_PROMPT = 1 << 0,
};

string str(path)(ppath p) {
	vector(string) tmp;
	vector_create(string)(&tmp, 0);
	ppath node = p->parent;
	while (node) {
		vector_add(string)(&tmp, &node->name);
		node = node->parent;
	}

#ifdef JOLLY_WIN32
	const char* pathdiv = "\\";
#else
	const char* pathdiv = "/";
#endif

	string div = string_create(pathdiv);
	vector(string) strings;
	u32 size = tmp.size * 2 + 1;
	vector_create(string)(&strings, size);
	for (i32 i = tmp.size - 1; i >= 0; i--) {
		vector_add(string)(&strings, vector_at(string)(&tmp, i));
		vector_add(string)(&strings, &div);
	}

	vector_add(string)(&strings, &p->name);

	string res = string_combine(strings);
	vector_destroy(string)(&strings);
	string_destroy(&div);
	return res;
}

path* path_create(string name, path* parent) {
	path* p = (path*)alloc8(sizeof(path)).data;
	p->name = name;
	p->parent = parent;
	return p;
}

void path_destroy(path* p) {
	free8(p);
}

typedef struct fzf_state fzf_state;
struct fzf_state {
	string rawprompt;
	vector(string) prompt;
	vector(ppath) paths;
	vector(ppath) dirs;
	vector(pairsp) matches;
	table(string, score) scores;

	threadid thread;
	mutex lock;
	u32 id;
	_Atomic u32 next;
	_Atomic u32 run;
};

static fzf_state* s_state = NULL;

int main_thread(void* in);

fzf_state* state_create() {
	fzf_state* state = (fzf_state*)alloc8(sizeof(fzf_state)).data;

	state->rawprompt = string_create("");
	vector_create(string)(&state->prompt, 0);
	vector_create(ppath)(&state->paths, 0);
	vector_create(ppath)(&state->dirs, 0);
	vector_create(pairsp)(&state->matches, 0);
	table_create(string, score)(&state->scores, 0);

	path* p = path_create(string_create("."), NULL);
	vector_add(ppath)(&state->dirs, &p);

	state->lock = mutex_create();
	state->id = 0;
	atomic_store_explicit(&state->next, 0, memory_order_release);
	atomic_store_explicit(&state->run, 1, memory_order_release);

	state->thread = thread_create(main_thread, NULL);
	return state;
}

void state_update(fzf_state* state, cstr prompt) {
	string tmp = string_create(prompt);
	if (eq(string)(&tmp, &state->rawprompt)) {
		string_destroy(&tmp);
		return;
	}

	mutex_acquire(state->lock);
	string_destroy(&state->rawprompt);
	u32 next = atomic_load_explicit(&state->next, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&state->next, &next, next + 1, memory_order_release, memory_order_relaxed));
	state->rawprompt = tmp;
	state->matches.size = 0;
	mutex_release(state->lock);
}

void state_destroy(fzf_state* state) {
	atomic_store_explicit(&state->run, 0, memory_order_relaxed);
	thread_join(state->thread);
	vector_destroy(string)(&state->prompt);
	vector_destroy(ppath)(&state->paths);
	vector_destroy(ppath)(&state->dirs);
	vector_destroy(pairsp)(&state->matches);
	table_destroy(string, score)(&state->scores);
	mutex_destroy(state->lock);

	free8((u8*)state);
}

void pathtraverse_callback(fzf_state* state, void* in);
void score_callback(fzf_state* state, void* in);
void accumulate_callback(fzf_state* state, void* in);

int main_thread(void* in) {
	fzf_state* state = s_state;

	const u32 TASK_LIMIT = 8;
	const u32 SCORE_LIMIT = 64;
	const u32 ACCUMULATE_LIMIT = 64;

	scheduler sched = { 0 };
	scheduler_create(&sched, 4);

	u32 run = atomic_load_explicit(&state->run, memory_order_relaxed);
	u32 pathidx = 0;

	const u32 MAX_BACKOFF = 256;
	const u32 INITIAL_BACKOFF = 64;
	u32 backoff = INITIAL_BACKOFF;

	while (run) {
		int stall = true;
		u32 next = atomic_load_explicit(&state->next, memory_order_acquire);
		mutex_acquire(state->lock);

		if (next != state->id) {
			scheduler_waitall(&sched);
			vector_destroy(string)(&state->prompt);
			state->prompt = string_split(state->rawprompt, "/");
			table_clear(string, score)(&state->scores);
			state->matches.size = 0;
			state->id = next;

			backoff = INITIAL_BACKOFF;
			stall = false;
			pathidx = 0;
		}

		taskinfo* info = queue_del(taskinfo)(&sched.done);
		while (info) {
			pfn_callback callback = *(pfn_callback*)info->args;
			callback(state, info->args);
			free8(info->args);
			free8(info);

			info = queue_del(taskinfo)(&sched.done);
			stall = false;
		}

		u32 count = MIN(TASK_LIMIT, state->dirs.size);
		for (u32 i = 0; i < count; i++) {
			if (!scheduler_cansubmit(&sched)) break;
			in_pathtraverse* args = (in_pathtraverse*)alloc8(sizeof(in_pathtraverse)).data;
			args->callback = pathtraverse_callback;
			args->in_dir = *vector_del(ppath)(&state->dirs, 0);
			vector_create(ppath)(&args->out_dirs, 0);
			vector_create(ppath)(&args->out_paths, 0);
			args->id = state->id;

			taskinfo* task = (taskinfo*)alloc8(sizeof(taskinfo)).data;
			task->task = pathtraverse_task;
			task->args = (void*)args;

			scheduler_submit(&sched, task);
			stall = false;
		}

		for (u32 i = 0; i < TASK_LIMIT; i++) {
			if (state->prompt.size == 0) break;
			if (pathidx >= state->paths.size) break;
			if (!scheduler_cansubmit(&sched)) break;
			in_accumulate* args = (in_accumulate*)alloc8(sizeof(in_accumulate)).data;
			args->callback = accumulate_callback;

			vector_create(string)(&args->out_strings, 0);
			vector_create(score)(&args->out_scores, 0);
			vector_create(pairsp)(&args->out_matches, ACCUMULATE_LIMIT);

			vector_create(ppath)(&args->in_paths, ACCUMULATE_LIMIT);
			args->in_prompt = state->prompt;
			args->in_scores = &state->scores;

			args->in_count = ACCUMULATE_LIMIT;
			args->id = state->id;

			u32 limit = MIN(state->paths.size, pathidx + ACCUMULATE_LIMIT);
			for (; pathidx < limit; pathidx++) {
				vector_add(ppath)(&args->in_paths, vector_at(ppath)(&state->paths, pathidx));
			}

			taskinfo* task = (taskinfo*)alloc8(sizeof(taskinfo)).data;
			task->task = accumulate_task;
			task->args = (void*)args;

			scheduler_submit(&sched, task);
			stall = false;
		}

		mutex_release(state->lock);
		run = atomic_load_explicit(&state->run, memory_order_relaxed);

		if (!stall) thread_yield();
		else {
			thread_sleep(backoff);
			backoff = MIN(MAX_BACKOFF, backoff * 2);
		}
	}

	scheduler_destroy(&sched);

	return 0;
}

void fzf_init() {
	s_state = state_create();
}

void fzf_term() {
	state_destroy(s_state);
}

void fzf_start(cstr prompt) {
	state_update(s_state, prompt);
}

vector(string) fzf_scores() {
	vector(pairsp) tmp;

	mutex_acquire(s_state->lock);
	vector_create(pairsp)(&tmp, s_state->matches.size);
	copy256(s_state->matches.data, tmp.data, tmp.reserve);
	tmp.size = s_state->matches.size;
	mutex_release(s_state->lock);

	vector(string) res;
	vector_create(string)(&res, 0);
	while (tmp.size) {
		pairsp sp = *heap_del(pairsp)(&tmp, 0);
		string name = str(path)(sp.second);
		vector_add(string)(&res, &name);
	}

	vector_destroy(pairsp)(&tmp);
	return res;
}

vector(string) fzf_paths() {
	mutex_acquire(s_state->lock);
	vector(string) res;
	vector_create(string)(&res, s_state->paths.size);
	for (u32 i = 0; i < s_state->paths.size; i++) {
		ppath p = *vector_at(ppath)(&s_state->paths, i);
		string fullpath = str(path)(p);
		vector_add(string)(&res, &fullpath);
	}

	mutex_release(s_state->lock);
	return res;
}

void fzf_cleanup(u8* memory) {
	free256(memory);
}

void pathtraverse_callback(fzf_state* state, void* in) {
	in_pathtraverse* args = (in_pathtraverse*)in;

	for (u32 i = 0; i < args->out_dirs.size; i++) {
		ppath p = *vector_at(ppath)(&args->out_dirs, i);
		vector_add(ppath)(&state->dirs, &p);
	}

	for (u32 i = 0; i < args->out_paths.size; i++) {
		ppath p = *vector_at(ppath)(&args->out_paths, i);
		vector_add(ppath)(&state->paths, &p);
	}

	vector_destroy(ppath)(&args->out_dirs);
	vector_destroy(ppath)(&args->out_paths);
}

void accumulate_callback(fzf_state* state, void* in) {
	in_accumulate* args = (in_accumulate*)in;
	vector(pairsp)* matches = &args->out_matches;

	const u32 MAX_RESULTS = 64;

	if (state->id == args->id) {
		u32 count = matches->size;
		for (u32 i = 0; i < count; i++) {
			pairsp* p = heap_del(pairsp)(matches, 0);
			if (state->matches.size < MAX_RESULTS) {
				heap_add(pairsp)(&state->matches, p);
			} else {
				heap_replace(pairsp)(&state->matches, p);
			}
		}

		count = args->out_strings.size;
		for (u32 i = 0; i < count; i++) {
			string key = *vector_at(string)(&args->out_strings, i);
			table_set(string, score)(&state->scores, key, vector_at(score)(&args->out_scores, i));
		}
	}

	vector_destroy(string)(&args->out_strings);
	vector_destroy(score)(&args->out_scores);
	vector_destroy(pairsp)(&args->out_matches);
	vector_destroy(ppath)(&args->in_paths);
}

COPY_DEFINE(ppath);
SWAP_DEFINE(ppath);
COPY_DEFINE(score);
SWAP_DEFINE(score);

VECTOR_DEFINE(ppath);
VECTOR_DEFINE(score);
STRTABLE_DEFINE(score);
