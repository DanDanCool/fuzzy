#pragma once

#include <stdint.h>

typedef struct fzf_dirnode
{
	fzf_string name;
	struct fzf_dirnode* children;
	size_t len;
	int is_dir;
} fzf_dirnode;

#ifdef FZF_WIN32
typedef uint32_t fzf_retval;
#endif

#ifdef FZF_LINUX
typedef void* fzf_retval;
#endif

typedef fzf_retval (*fzf_start_routine)(void*);

void fzf_read_directory(fzf_dirnode* dir);

void* fzf_thread_create(fzf_start_routine start_routine, void* args);
void fzf_thread_detach(void* thread);
void fzf_thread_exit(fzf_retval retval);

void fzf_thread_join(void* thread);
void fzf_thread_cancel(void* thread);
void fzf_thread_testcancel();

void* fzf_mutex_create();
void fzf_mutex_destroy(void* mutex);
void fzf_mutex_lock(void* mutex);
void fzf_mutex_unlock(void* mutex);
