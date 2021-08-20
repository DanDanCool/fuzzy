#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fuzzy.h"
#include "platform.h"

static fzf_dirnode new_dirnode(fzf_dirnode* parent, struct dirent* dir_entry)
{
	int is_dir = dir_entry->d_type & DT_DIR;
	int len = strlen(dir_entry->d_name) + parent->name.len + 1;

	char* name = (char*)malloc(len + 1);
	sprintf(name, "%s/%s", parent->name.str, dir_entry->d_name);

	return (fzf_dirnode){ .name = (fzf_string){ .str = name, .len = len },
	.children = NULL, .len = 0, .is_dir = is_dir };
}

void fzf_read_directory(fzf_dirnode* dir)
{
	DIR* directory = opendir(dir->name.str);
	if (!directory)
		return;

	struct dirent* dir_entry;
	size_t count = 0;

	while (dir_entry = readdir(directory))
	{
		if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
			continue;

		count++;
	}

	if (!count)
		return;

	rewinddir(directory);
	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));
	fzf_dirnode* child = dir->children;

	while (dir_entry = readdir(directory))
	{
		if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
			continue;

		*child = new_dirnode(dir, dir_entry);
		child++;
	}

	closedir(directory);
	dir->len = count;
}

void* fzf_thread_create(fzf_start_routine start_routine, void* args)
{
	pthread_t thread;
	pthread_create(&thread, NULL, start_routine, args);
	return (void*)thread;
}

void fzf_thread_detach(void* thread)
{
	pthread_detach((pthread_t)thread);
}

void fzf_thread_exit(fzf_retval retval)
{
	pthread_exit(retval);
}

void fzf_thread_join(void* thread)
{
	pthread_join((pthread_t)thread, NULL);
}

void fzf_thread_cancel(void* thread)
{
	pthread_cancel((pthread_t)thread);
}

void fzf_thread_testcancel()
{
	pthread_testcancel();
}

void* fzf_mutex_create()
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	return (void*)mutex;
}

void fzf_mutex_destroy(void* mutex)
{
	pthread_mutex_destroy(mutex);
	free(mutex);
}

void fzf_mutex_lock(void* mutex)
{
	pthread_mutex_lock(mutex);
}

void fzf_mutex_unlock(void* mutex)
{
	pthread_mutex_unlock(mutex);
}
