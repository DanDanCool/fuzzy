#include <windows.h>
#include <fileapi.h>

#include <stdio.h>

#include "fuzzy.h"
#include "filesystem.h"

static fzf_dirnode new_dirnode(fzf_dirnode* parent, WIN32_FIND_DATA* dir_entry)
{
	int is_dir = dir_entry->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	int len = strlen(dir_entry->cFileName) + parent->name.len + 1;

	char* name = (char*)malloc(len + 1);
	sprintf(name, "%s/%s", parent->name.str, dir_entry->cFileName);

	return (fzf_dirnode){ .name = (fzf_string){ .str = name, .len = len },
	.children = NULL, .len = 0, .is_dir = is_dir };
}

void fzf_read_directory(fzf_dirnode* dir)
{
	WIN32_FIND_DATA dir_entry;
	char* search = (char*)malloc(dir->name.len + 3);
	sprintf(search, "%s/*", dir->name.str);

	// skip first two results since those seem to always be . and ..
	HANDLE directory = FindFirstFile(search, &dir_entry);
	FindNextFile(directory, &dir_entry);

	if (directory == INVALID_HANDLE_VALUE)
		return;

	int count = 0;
	while (FindNextFile(directory, &dir_entry))
		count++;

	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));
	dir->len = count;

	// seems kind of bad... but I don't want to allocate more memory than required
	FindClose(directory);
	directory = FindFirstFile(search, &dir_entry);
	FindNextFile(directory, &dir_entry);

	fzf_dirnode* child = dir->children;
	while (FindNextFile(directory, &dir_entry))
	{
		*child = new_dirnode(dir, &dir_entry);
		child++;
	}

	FindClose(directory);
	free(search);
}
