#include "fuzzy.h"
#include "platform.h"
#include "task.h"

#include <scheduler.h>

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

static threadid s_thread;

typedef struct fzf_state fzf_state;
struct fzf_state {
	string rawprompt;
	vector(string) prompt;
	vector(string) waitscore;
	vector(ppath) paths;
	vector(ppath) dirs;
	vector(pairsp) matches;
	table(string, score) scores;

	mutex lock;
	_Atomic u32 id;
	_Atomic u32 run;
};

static fzf_state s_state = {0};

void state_create(fzf_state* state) {
	vector_create(string)(&state->waitscore, 0);
	vector_create(ppath)(&state->paths, 0);
	vector_create(ppath)(&state->dirs, 0);
	vector_create(pairsp)(&state->matches, 0);
	table_create(string, score)(&state->scores, 0);

	state->lock = mutex_create();
	atomic_store_explicit(&state->id, 0, memory_order_release);
	atomic_store_explicit(&state->run, 1, memory_order_release);
}

void state_update(fzf_state* state, cstr prompt) {
	string tmp = string_create(prompt);
	if (eq(string)(&tmp, &state->rawprompt)) {
		string_destroy(&tmp);
		return;
	}

	state->matches.size = 0;
	u32 next = atomic_load_explicit(&state->id, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&state->id, &next, next + 1, memory_order_release, memory_order_relaxed));

	mutex_acquire(state->lock);
	string_destroy(&state->rawprompt);
	state->rawprompt = tmp;
	mutex_release(state->lock);
}

void state_destroy(fzf_state* state) {
	vector_destroy(string)(&state->prompt);
	vector_destroy(string)(&state->waitscore);
	vector_destroy(ppath)(&state->paths);
	vector_destroy(ppath)(&state->dirs);
	vector_destroy(pairsp)(&state->matches);
	table_destroy(string, score)(&state->scores);
	mutex_destroy(state->lock);
}

void pathtraverse_callback(fzf_state* state, void* in);
void score_callback(fzf_state* state, void* in);
void accumulate_callback(fzf_state* state, void* in);

int main_thread(void* args) {
	fzf_state state = { 0 };

	const u32 TASK_LIMIT = 8;
	const u32 SCORE_LIMIT = 64;
	const u32 ACCUMULATE_LIMIT = 64;

	scheduler sched = { 0 };
	scheduler_create(&sched, 1);

	u32 id = atomic_load_explicit(&s_state.id, memory_order_acquire);
	u32 run = atomic_load_explicit(&s_state.run, memory_order_relaxed);

	u32 waitidx = 0;
	u32 pathidx = 0;

	path* p = path_create(string_create("."), NULL);
	vector_add(ppath)(&s_state.dirs, &p);

	while (run) {
		u32 next = atomic_load_explicit(&s_state.id, memory_order_acquire);
		mutex_acquire(s_state.lock);

		if (next != id) {
			scheduler_waitall(&sched);
			vector_destroy(string)(&s_state.prompt);
			s_state.prompt = string_split(s_state.rawprompt, "/.");
			s_state.matches.size = 0;
			id = next;

			waitidx = 0;
			pathidx = 0;
		}

		taskinfo* info = queue_del(taskinfo)(&sched.done);
		while (info) {
			pfn_callback callback = *(pfn_callback*)info->args;
			callback(&s_state, info->args);
			free8(info->args);
			free8(info);

			info = queue_del(taskinfo)(&sched.done);
		}

		u32 count = MIN(TASK_LIMIT, s_state.dirs.size);
		for (u32 i = 0; i < count; i++) {
			if (!scheduler_cansubmit(&sched)) break;
			in_pathtraverse* args = (in_pathtraverse*)alloc8(sizeof(in_pathtraverse)).data;
			args->callback = pathtraverse_callback;
			args->in_dir = *vector_del(ppath)(&s_state.dirs, 0);
			vector_create(ppath)(&args->out_dirs, 0);
			vector_create(ppath)(&args->out_paths, 0);
			args->id = id;

			taskinfo* task = (taskinfo*)alloc8(sizeof(taskinfo)).data;
			task->task = pathtraverse_task;
			task->args = (void*)args;

			scheduler_submit(&sched, task);
		}

		for (u32 i = 0; i < TASK_LIMIT; i++) {
			if (s_state.prompt.size == 0) break;
			if (waitidx >= s_state.waitscore.size) break;
			if (!scheduler_cansubmit(&sched)) break;
			in_score* args = (in_score*)alloc8(sizeof(in_score)).data;
			args->callback = score_callback;
			args->in_prompt = s_state.prompt;
			vector_create(string)(&args->in_strings, SCORE_LIMIT);
			vector_create(score)(&args->out_scores, SCORE_LIMIT);
			args->id = id;

			u32 limit = MIN(s_state.waitscore.size, waitidx + SCORE_LIMIT);
			for (; waitidx < limit; waitidx++) {
				vector_add(string)(&args->in_strings, vector_at(string)(&s_state.waitscore, waitidx));
			}

			taskinfo* task = (taskinfo*)alloc8(sizeof(taskinfo)).data;
			task->task = score_task;
			task->args = (void*)args;

			scheduler_submit(&sched, task);
		}

		for (u32 i = 0; i < TASK_LIMIT; i++) {
			if (s_state.prompt.size == 0) break;
			if (pathidx >= s_state.paths.size) break;
			if (!scheduler_cansubmit(&sched)) break;
			in_accumulate* args = (in_accumulate*)alloc8(sizeof(in_accumulate)).data;
			args->callback = accumulate_callback;
			args->in_count = ACCUMULATE_LIMIT;
			args->in_scores = &s_state.scores;
			args->id = id;
			vector_create(pairsp)(&args->out_scores, ACCUMULATE_LIMIT);
			vector_create(ppath)(&args->in_paths, ACCUMULATE_LIMIT);

			u32 limit = MIN(s_state.paths.size, pathidx + ACCUMULATE_LIMIT);
			for (; pathidx < limit; pathidx++) {
				vector_add(ppath)(&args->in_paths, vector_at(ppath)(&s_state.paths, pathidx));
			}

			taskinfo* task = (taskinfo*)alloc8(sizeof(taskinfo)).data;
			task->task = accumulate_task;
			task->args = (void*)args;

			scheduler_submit(&sched, task);
		}

		mutex_release(s_state.lock);
	}

	return 0;
}

void fzf_init() {
	state_create(&s_state);
	s_thread = thread_create(main_thread, NULL);
}

void fzf_term() {
	atomic_store_explicit(&s_state.run, 0, memory_order_relaxed);
	thread_join(s_thread);
	state_destroy(&s_state);
}

void fzf_start(cstr prompt) {
	state_update(&s_state, prompt);
}

vector(string) fzf_scores() {
	vector(pairsp) tmp;
	vector_create(pairsp)(&tmp, s_state.matches.reserve);

	mutex_acquire(s_state.lock);
	copy256(s_state.matches.data, tmp.data, tmp.reserve);
	mutex_release(s_state.lock);

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
	mutex_acquire(s_state.lock);
	vector(string) res;
	vector_create(string)(&res, s_state.paths.size);
	for (u32 i = 0; i < s_state.paths.size; i++) {
		ppath p = *vector_at(ppath)(&s_state.paths, i);
		string fullpath = str(path)(p);
		vector_add(string)(&res, &fullpath);
	}

	mutex_release(s_state.lock);
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
		vector_add(string)(&state->waitscore, &p->name);
	}

	for (u32 i = 0; i < args->out_paths.size; i++) {
		ppath p = *vector_at(ppath)(&args->out_paths, i);
		vector_add(ppath)(&state->paths, vector_at(ppath)(&args->out_paths, i));
		vector_add(string)(&state->waitscore, &p->name);
	}

	vector_destroy(ppath)(&args->out_dirs);
	vector_destroy(ppath)(&args->out_paths);
}

void score_callback(fzf_state* state, void* in) {
	in_score* args = (in_score*)in;

	u32 id = atomic_load_explicit(&state->id, memory_order_acquire);
	if (id == args->id) {
		for (u32 i = 0; i < args->out_scores.size; i++) {
			string key = *vector_at(string)(&args->in_strings, i);
			score* val = vector_at(score)(&args->out_scores, i);
			table_set(string, score)(&state->scores, key, val);
		}
	}

	vector_destroy(string)(&args->in_strings);
	vector_destroy(score)(&args->out_scores);
}

void accumulate_callback(fzf_state* state, void* in) {
	in_accumulate* args = (in_accumulate*)in;

	const u32 MAX_RESULTS = 16;

	u32 id = atomic_load_explicit(&state->id, memory_order_acquire);
	if (id == args->id) {
		u32 count = args->out_scores.size;
		for (u32 i = 0; i < count; i++) {
			pairsp* p = heap_del(pairsp)(&args->out_scores, 0);
			if (state->matches.size < MAX_RESULTS) {
				heap_add(pairsp)(&state->matches, p);
			} else {
				heap_replace(pairsp)(&state->matches, p);
			}
		}
	}

	vector_destroy(ppath)(&args->in_paths);
	vector_destroy(pairsp)(&args->out_scores);
}

COPY_DEFINE(ppath);
SWAP_DEFINE(ppath);
COPY_DEFINE(score);
SWAP_DEFINE(score);

VECTOR_DEFINE(ppath);
VECTOR_DEFINE(score);
STRTABLE_DEFINE(score);
