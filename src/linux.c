#include <sys/types.h>
#include <dirent.h>

#include "filesystem.h"

static fzf_dirnode new_dirnode(fzf_dirnode* parent, struct dirent* dir_entry)
{
	int is_dir = dir_entry->d_type & DT_DIR;
	int len = strlen(dir_entry->d_name) + parent->name.len + 1;

	char* name = (char*)malloc(len + 1);
	sprintf(name, "%s/%s", parent->name.str, dir_entry->d_name);

	return (fzf_dirnode){ .name = (fzf_string){ .str = name, .len = len },
	.children = NULL, .len = 0, .is_dir = is_dir };
}

void read_directory(fzf_dirnode* dir)
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

	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));
	dir->len = count;

	rewinddir(directory);
	fzf_dirnode* child = dir->children;

	while (dir_entry = readdir(directory))
	{
		if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
			continue;

		*child = new_dirnode(dir, dir_entry);
		child++;
	}

	closedir(directory);
}
