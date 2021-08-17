#include <sys/types.h>
#include <dirent.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fuzzy.h"

typedef struct fzf_dirnode
{
	fzf_string name;
	struct fzf_dirnode* children;
	size_t len;
	int is_dir;
} fzf_dirnode;

typedef struct
{
	fzf_dirnode* node;
	int score;
} fzf_result;

static struct
{
	fzf_result scores[MAX_RESULTS];
	fzf_result* greatest[MAX_RESULTS];
	size_t len;
} s_results;

static fzf_dirnode s_root;

static void add_result(fzf_dirnode* node, int score)
{
	if (s_results.len < MAX_RESULTS)
	{
		s_results.scores[s_results.len].node = node;
		s_results.scores[s_results.len].score = score;

		int i = s_results.len;
		while (i && score < s_results.greatest[i - 1]->score)
		{
			s_results.greatest[i] = s_results.greatest[i - 1];
			i--;
		}

		s_results.greatest[i] = &s_results.scores[s_results.len];
		s_results.len++;
	}
	else if (score < s_results.greatest[s_results.len - 1]->score)
	{
		fzf_result* empty = s_results.greatest[s_results.len - 1];
		int i = s_results.len - 1;

		while (i && score < s_results.greatest[i - 1]->score)
		{
			s_results.greatest[i] = s_results.greatest[i - 1];
			i--;
		}

		s_results.greatest[i] = empty;
		s_results.greatest[i]->score = score;
		s_results.greatest[i]->node = node;
	}
}

static fzf_dirnode new_dirnode(fzf_dirnode* parent, struct dirent* dir_entry)
{
	int is_dir = dir_entry->d_type == DT_DIR ? 1 : 0;
	int len = strlen(dir_entry->d_name) + parent->name.len + 1;
	char* name = (char*)malloc(len + 1);
	sprintf(name, "%s/%s", parent->name.str, dir_entry->d_name);

	return (fzf_dirnode){ .name = (fzf_string){ .str = name, .len = len },
	.children = NULL, .len = 0, .is_dir = is_dir };
}

static void read_directory(fzf_dirnode* dir)
{
	DIR* directory = opendir(dir->name.str);
	if (!directory)
		return;

	struct dirent* dir_entry = readdir(directory);

	size_t count = 0;
	while (dir_entry)
	{
		if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
		{
			dir_entry = readdir(directory);
			continue;
		}

		count++;
		dir_entry = readdir(directory);
	}

	if (!count)
		return;

	dir->children = (fzf_dirnode*)malloc(count * sizeof(fzf_dirnode));
	dir->len = count;

	rewinddir(directory);
	dir_entry = readdir(directory);

	size_t i = 0;
	while (dir_entry)
	{
		if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
		{
			dir_entry = readdir(directory);
			continue;
		}

		dir->children[i] = new_dirnode(dir, dir_entry);
		dir_entry = readdir(directory);
		i++;
	}

	closedir(directory);
}

static fzf_string str_tolower(const char* str)
{
	int len = strlen(str);
	char* lower = (char*)malloc(len + 1);
	lower[len] = 0;

	for (int i = 0; i < len; i++)
		lower[i] = (char)tolower(str[i]);

	return (fzf_string){ .str = lower, .len = len };
}

void fzf_setup(char** ignore, int len)
{
	void setup_recurse(fzf_dirnode* node)
	{
		for (size_t i = 0; i < node->len; i++)
		{
			fzf_dirnode* child = &node->children[i];

			if (child->is_dir)
			{
				int skip = 0;
				for (int i = 0; i < len; i++)
				{
					if (!strcmp(ignore[i], child->name.str))
					{
						skip = 1;
						break;
					}
				}

				if (skip)
					continue;

				read_directory(child);
				setup_recurse(child);
			}
		}
	}

	s_root = (fzf_dirnode){ .name = (fzf_string){ .str = ".", .len = 1 },
		.children = NULL, .len = 0, .is_dir = 1 };

	read_directory(&s_root);
	setup_recurse(&s_root);
}

fzf_output fzf_get_output(fzf_string* prompt)
{
	{
		memset(&s_results, 0, sizeof(s_results));
		fzf_string name = str_tolower(s_root.children[0].name.str);
		s_results.scores[0].node = &s_root.children[0];
		s_results.scores[0].score = fzf_fuzzy_match(prompt, &name);
		s_results.greatest[0] = &s_results.scores[0];
		s_results.len++;
		free(name.str);
	}

	void scores_recurse(fzf_dirnode* node)
	{
		for (size_t i = 0; i < node->len; i++)
		{
			fzf_dirnode* child = &node->children[i];

			if (child->is_dir)
			{
				scores_recurse(child);
				continue;
			}

			fzf_string name = str_tolower(child->name.str);
			int score = fzf_fuzzy_match(prompt, &name) - 2 * fzf_char_match(prompt, &name) - name.len / 2;
			add_result(child, score);
			free(name.str);
		}
	}

	for (size_t i = 1; i < s_root.len; i++)
	{
		fzf_dirnode* child = &s_root.children[i];

		if (child->is_dir)
		{
			scores_recurse(child);
			continue;
		}

		fzf_string name = str_tolower(child->name.str);
		int score = fzf_fuzzy_match(prompt, &name) - 2 * fzf_char_match(prompt, &name) - name.len / 2;
		add_result(child, score);
		free(name.str);
	}

	fzf_output out;
	fzf_string* str = out.results;
	for (int i = s_results.len - 1; i >= 0; i--)
	{
		*str = s_results.greatest[i]->node->name;
		str++;
	}

	out.len = s_results.len;
	return out;
}

int fzf_char_match(fzf_string* str1, fzf_string* str2)
{
	char chars[256];
	memset(chars, 0, 256);

	for (int i = 0; i < str2->len; i++)
		chars[str2->str[i]] = 1;

	int matches = 0;
	for (int i = 0; i < str1->len; i++)
		matches += chars[str1->str[i]];

	return matches;
}

// implements levenshtein distance
int fzf_fuzzy_match(fzf_string* str1, fzf_string* str2)
{
	if (str1->len == 0)
		return str2->len;

	if (str2->len == 0)
		return str1->len;

	size_t* edits = (size_t*)malloc((str2->len + 1) * sizeof(size_t));
	for (size_t i = 0; i <= str2->len; i++)
		edits[i] = i;

	for (int i = 0; i < str1->len; i++)
	{
		size_t corner = i;
		edits[0] = i + 1;

		for (int j = 0; j < str2->len; j++)
		{
			size_t upper = edits[j + 1];
			if (str1->str[i] == str2->str[j])
			{
				edits[j + 1] = corner;
			}
			else
			{
				size_t min = corner > upper ? upper : corner;
				edits[j + 1] = (min > edits[j] ? edits[j] : min) + 1;
			}

			corner = upper;
		}
	}

	int result = (int)edits[str2->len];
	free(edits);

	return result;
}
