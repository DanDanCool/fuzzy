#include "fuzzy.h"
#include "platform.h"

// while most file system limit file names to 256 characters, 64 is correct enough, also convenient for avx
#define STRING_LENGTH 64

enum
{
	STATE_NEW_PROMPT = 1 << 0,
};

static thread s_thread;

struct fzf_state
{
	fzf_args args;
	scheduler sched;
	queue(mem_pool) str_allocator;
	queue(mem_arena) allocator;

	_Atomic cstr raw_prompt;
	vector(pstring) prompt;
	_Atomic u32 flags;

	path_ignore ignore;
	vector(ppath) paths;
	vector(ppath) total_scores; // heap of total path scores
	hash_table(pstring, score) scores;
};

void state_init(fzf_state* state)
{

}

void state_destroy(fzf_state** data)
{
	fzf_state* state = *data;

	{
		make_iterator(queue(mem_pool), iter, &state->str_allocator);
		for_each(pool, iter)
			pool_destroy(pool);
	}

	{
		make_iterator(queue(mem_arena), iter, &state->allocator);
		for_each(arena, iter)
			arena_destroy(arena);
	}

	vector_destroy(ppath)(&state->ignore.paths);
	vector_destroy(u8)(&state->ignore.flags);
	vector_destroy(ppath)(&state->paths);
	vector_destroy(ppath)(&total_scores);
	table_destroy(pstring, score)(&scores);
	free(state);
}

void main_thread(thread* t)
{
	MEM_GUARD(fzf_state*, state_destroy) state = (fzf_state*)t->data;

	// look through ignore
	// start recursing over directories

	while (true)
	{
		thread_trykill(t);
		thread_trypause(t);

		// prompt received
		if (!spinlock_trywait(&state->flags, STATE_NEW_PROMPT))
		{

		}

		spinlock_signal(&state->flags, STATE_NEW_PROMPT);

		// request for scores

		thread_yield(t);
	}
}

void fzf_init(fzf_args* args)
{
	fzf_state* state = (fzf_state*)jolly_alloc(sizeof(fzf_state));
	*state = {};
	state->args = *args;
	state->args.ignore = cstr_copy(args->ignore);

	s_thread = {};
	s_thread.task = main_thread;
	s_thread.data = (void*)state;
	thread_create(&s_thread, THREAD_KILL | THREAD_PAUSE);
	thread_start(&s_thread);
}

void fzf_destroy()
{
	// cleanup handled in thread
	thread_kill(&s_thread);
}

void fzf_start(cstr prompt, u32 size)
{
	fzf_state* state = (fzf_state*)s_thread.data;

	cstr copy = (cstr)jolly_alloc(size);
	vector(u8) dst = { copy, 0, size };
	vector(u8) src = { prompt, 0, size };
	cstr old = atomic_exchange_explicit(&state->prompt, copy, memory_order_relaxed);
	spinlock_wait(&state->flags, STATE_NEW_PROMPT);

	free(old);
}

void fzf_scores(vector_ppath* v)
{
	fzf_state* state = (fzf_state*)s_thread.data;

	vector(u8) dst = { (u8*)vector_at(ppath)(v, 0), 0, v->reserve };
	vector(u8) src = { (u8*)vector_at(ppath)(&state->total_scores, 0), 0, state->total_scores.size };
	vector_cpy8(&dst, &src);
}
