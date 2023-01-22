#pragma once

#include "structures.h"

typedef struct thread thread;
typedef struct semaphore semaphore;
typedef struct scheduler scheduler;

typedef void (*pfn_task)(thread* t);

enum
{
	THREAD_INVALID = 0,
	THREAD_RUNNING = 1 << 0,
	THREAD_KILL = 1 << 1,
	THREAD_PAUSE = 1 << 2,
	THREAD_DETACH = 1 << 3,
};

// sem_t is 4 bytes while pthread_t is 8 bytes
// user responsible for providing thread memory
struct semaphore
{
	u64 handle;
};

struct thread
{
	u64 handle;
	semaphore lock;
	void* data;
	atomic(u32) flags;

	pfn_task task;
	pfn_task on_pause;
	pfn_task on_kill;
};

VECTOR_DEFINE(thread);

typedef struct task_t task_t;
QUEUE_DEFINE(task_t)

struct scheduler;
{
	vector(thread) threads;
	atomic_queue(u32) taskq;
	atomic_queue(vector(u32)) deps;
	atomic_mem_pool tasks;
	atomic_mem_list locks;
};

void thread_create(thread* t, u32 flags);
void thread_start(thread* t);

void thread_join(thread* t);
int thread_tryjoin(thread* t);

void thread_kill(thread* t);

// should only be called with threads created with the THREAD_PAUSE flag
void thread_pause(thread* t);
void thread_unpause(thread* t);

void thread_priority(thread* t, int priority);

// check if the thread should be killed/paused
void thread_trykill(thread* t);
void thread_trypause(thread* t);

// call when the thread has finished execution
void thread_exit(thread* t);

// yield remaining scheduler time
void thread_yield(thread* t);

void semaphore_init(semaphore* sem);
void semaphore_destroy(semaphore* sem);

void semaphore_wait(semaphore* sem);
int semaphore_trywait(semaphore* sem);

void semaphore_signal(semaphore* sem);
int semaphore_value(semaphore* sem);

void scheduler_init(scheduler* s);
void scheduler_destroy(scheduler* s);

u32 task_create(scheduler* s, pfn_task task, void* data);
u32 dependency_create(scheduler* s, u32 task);
void dependency_add(scheduler* s, u32 task, u32 dependency);

void task_wait(scheduler* s, u32 task); // increment task refcount
void task_signal(scheduler* s, u32 task); // decrement task refcount

void scheduler_submit(scheduler* s, u32* tasks, u32 count);
void scheduler_waittask(scheduler* s, u32* tasks, u32 count);
void scheduler_waitall(scheduler* s);
