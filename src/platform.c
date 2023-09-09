#ifdef JOLLY_WIN32
#include <windows.h>
#include <fileapi.h>

#undef INTERFACE
#undef interface
#include <strlib.h>

#include "fuzzy.h"
#include "platform.h"

void read_directory(string dir, vector(string)* out_dirs, vector(string)* out_paths) {
	WIN32_FIND_DATAA dir_entry;

	// skip first two results since those seem to always be . and ..
	HANDLE directory = FindFirstFileA((char*)dir.data, &dir_entry);
	FindNextFileA(directory, &dir_entry);

	if (directory == INVALID_HANDLE_VALUE)
		return;

	while (FindNextFileA(directory, &dir_entry))	{
		int is_dir = dir_entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		string name = string_create(dir_entry.cFileName);
		if (is_dir) {
			vector_add(string)(out_dirs, &name);
		} else {
			vector_add(string)(out_paths, &name);
		}
	}

	FindClose(directory);
}
#endif

#ifdef JOLLY_LINUX
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
#endif
