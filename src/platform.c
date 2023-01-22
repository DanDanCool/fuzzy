#ifdef win32
#include <windows.h>
#include <fileapi.h>
#include <processthreadsapi.h>
#include <synchapi.h>

#include <string.h>
#include <stdlib.h>

#include "fuzzy.h"
#include "platform.h"

// yeah I know this is really bad, but the alternative is QueueAPC and SleepEx
// which will probably be worse
static int s_signal = 0;

static fzf_dirnode new_dirnode(fzf_dirnode* parent, WIN32_FIND_DATA* dir_entry)
{
	int is_dir = dir_entry->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	size_t dir_len = strlen(dir_entry->cFileName);
	size_t len = dir_len + parent->name.len + 1;

	char* name = (char*)malloc(len + 1);
	memcpy(name, parent->name.str, parent->name.len);
	memcpy(name + parent->name.len + 1, dir_entry->cFileName, dir_len);
	name[parent->name.len] = '/';
	name[len] = 0;

	return (fzf_dirnode){ .name = (fzf_string){ .str = name, .len = len },
	.children = NULL, .len = 0, .is_dir = is_dir };
}

void fzf_read_directory(fzf_dirnode* dir)
{
	WIN32_FIND_DATA dir_entry;
	char* search = (char*)malloc(dir->name.len + 3);
	memcpy(search, dir->name.str, dir->name.len);
	memcpy(search + dir->name.len, "/*\0", 3);

	// skip first two results since those seem to always be . and ..
	HANDLE directory = FindFirstFile(search, &dir_entry);
	FindNextFile(directory, &dir_entry);

	if (directory == INVALID_HANDLE_VALUE)
		return;

	int count = 0;
	while (FindNextFile(directory, &dir_entry))
		count++;

	if (!count)
		return;

	// seems kind of bad... but I don't want to allocate more memory than required
	FindClose(directory);
	directory = FindFirstFile(search, &dir_entry);
	FindNextFile(directory, &dir_entry);

	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));
	fzf_dirnode* child = dir->children;
	while (FindNextFile(directory, &dir_entry))
	{
		*child = new_dirnode(dir, &dir_entry);
		child++;
	}

	FindClose(directory);
	free(search);
	dir->len = count;
}

void* fzf_thread_create(fzf_start_routine start_routine, void* args)
{
	HANDLE thread = CreateThread(NULL, 0, start_routine, args, 0, NULL);
	return (void*)thread;
}

void fzf_thread_detach(void* thread)
{
	CloseHandle((HANDLE)thread);
}

void fzf_thread_exit(fzf_retval retval)
{
	ExitThread(retval);
}

void fzf_thread_join(void* thread)
{
	WaitForSingleObject((HANDLE)thread, INFINITE);
	s_signal = 0;
}

void fzf_thread_cancel(void* thread)
{
	// we can get away with this because there will ALWAYS be one thread using this signal
	s_signal = 1;
}

void fzf_thread_testcancel()
{
	if (s_signal)
	{
		s_signal = 0;
		ExitThread(0);
	}
}

void* fzf_mutex_create()
{
	HANDLE mutex = CreateMutex(NULL, 0, NULL);
	return (void*)mutex;
}

void fzf_mutex_destroy(void* mutex)
{
	CloseHandle((HANDLE)mutex);
}

void fzf_mutex_lock(void* mutex)
{
	WaitForSingleObject((HANDLE)mutex, INFINITE);
}

void fzf_mutex_unlock(void* mutex)
{
	ReleaseMutex((HANDLE)mutex);
}
#endif

#ifdef linux
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>

#include "fuzzy.h"
#include "platform.h"

static fzf_dirnode new_dirnode(fzf_dirnode* parent, struct dirent* dir_entry)
{
	int is_dir = dir_entry->d_type & DT_DIR;
	size_t dir_len = strlen(dir_entry->d_name);
	size_t len = dir_len + parent->name.len + 1;

	char* name = (char*)malloc(len + 1);
	memcpy(name, parent->name.str, parent->name.len);
	memcpy(name + parent->name.len + 1, dir_entry->d_name, dir_len);
	name[parent->name.len] = '/';
	name[len] = 0;

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
#endif
