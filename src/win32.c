#include <windows.h>
#include <fileapi.h>

#include "filesystem.h"

void fzf_read_directory(fzf_dirnode* dir)
{
	WIN32_FIND_DATA dir_entry;
	HANDLE directory = FindFirstFile("../*", &dir_entry);

	if (directory == INVALID_HANDLE_VALUE)
		return;

	int count = 1;
	while(FindNextFile(directory, &dir_entry))
	{
		if (!strcmp(dir_entry.cFileName, ".") || !strcmp(dir_entry.cFileName, ".."))
			continue;

		count++;
	}

	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));

	printf("%i\n", count);

	FindClose(directory);


}
