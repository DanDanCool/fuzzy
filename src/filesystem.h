#pragma once

typedef struct fzf_dirnode
{
	fzf_string name;
	struct fzf_dirnode* children;
	size_t len;
	int is_dir;
} fzf_dirnode;

void fzf_read_directory(fzf_dirnode* dir);
