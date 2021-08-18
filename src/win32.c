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
	HANDLE directory = FindFirstFile("../*", &dir_entry);

	if (directory == INVALID_HANDLE_VALUE)
		return;

	int count = 1;
	while (FindNextFile(directory, &dir_entry))
	{
		if (!strcmp(dir_entry.cFileName, ".") || !strcmp(dir_entry.cFileName, ".."))
			continue;

		count++;
	}

	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));
	dir->len = count;

	// seems kind of bad... but I don't want to allocate more memory than required
	FindClose(directory);
	directory = FindFirstFile("../*", &dir_entry);

	fzf_dirnode* child = dir->children;
	*child = new_dirnode(dir, &dir_entry);
	child++;

	while (FindNextFile(directory, &dir_entry))
	{
		if (!strcmp(dir_entry.cFileName, ".") || !strcmp(dir_entry.cFileName, ".."))
			continue;

		*child = new_dirnode(dir, &dir_entry);
		child++;
	}

	FindClose(directory);
}
