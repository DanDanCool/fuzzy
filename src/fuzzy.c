#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fuzzy.h"
#include "platform.h"

// struct passed to async_setup()
typedef struct
{
	char** ignore;
	int len;
} fzf_setup_args;

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

typedef struct
{
    fzf_string* data;
    int len;
    int sz;
} vec_entry;

static vec_entry s_entries;
static fzf_dirnode s_root;
static void* fzf_thread;
static void* fzf_mutex;

static void add_result(fzf_dirnode* node, int score)
{
	fzf_mutex_lock(fzf_mutex);

	if (s_results.len < MAX_RESULTS)
	{
		s_results.scores[s_results.len].node = node;
		s_results.scores[s_results.len].score = score;

		size_t i = s_results.len;
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
		size_t i = s_results.len - 1;

		while (i && score < s_results.greatest[i - 1]->score)
		{
			s_results.greatest[i] = s_results.greatest[i - 1];
			i--;
		}

		s_results.greatest[i] = empty;
		s_results.greatest[i]->score = score;
		s_results.greatest[i]->node = node;
	}

	fzf_mutex_unlock(fzf_mutex);
}

static fzf_string str_tolower(const char* str)
{
	size_t len = strlen(str);
	char* lower = (char*)malloc(len + 1);
	lower[len] = 0;

	for (size_t i = 0; i < len; i++)
		lower[i] = (char)tolower(str[i]);

	return (fzf_string){ .str = lower, .len = len };
}

static void recurse_directories(fzf_dirnode* node, char** ignore, int len)
{
	for (size_t i = 0; i < node->len; i++)
	{
		fzf_dirnode* child = &node->children[i];

		if (!child->is_dir) continue;

		int skip = 0;
		for (int i = 0; i < len; i++)
		{
			if (!strcmp(ignore[i], child->name.str))
			{
				skip = 1;
				break;
			}
		}

		if (skip) continue;

		fzf_read_directory(child);
		recurse_directories(child, ignore, len);
	}
}

static fzf_retval async_init(void* args)
{
	fzf_read_directory(&s_root);

	fzf_setup_args* setup_args = (fzf_setup_args*)args;
	recurse_directories(&s_root, setup_args->ignore, setup_args->len);

	free(setup_args);
	fzf_thread_exit(0);
}

void fzf_init(char** ignore, int len)
{
	s_root = (fzf_dirnode){ .name = (fzf_string){ .str = ".", .len = 1 },
		.children = NULL, .len = 0, .is_dir = 1 };

	fzf_setup_args* args = (fzf_setup_args*)malloc(sizeof(fzf_setup_args));
	*args = (fzf_setup_args){ .ignore = ignore, .len = len };

	void* thread = fzf_thread_create(async_init, (void*)args);
	fzf_thread_detach(thread);

	fzf_mutex = fzf_mutex_create();
}

static void scores_recurse(fzf_dirnode* node, fzf_string* prompt)
{
	for (size_t i = 0; i < node->len; i++)
	{
		fzf_thread_testcancel();
		fzf_dirnode* child = &node->children[i];

		if (child->is_dir)
		{
			scores_recurse(child, prompt);
			continue;
		}

		fzf_string name = str_tolower(child->name.str);
		int score = fzf_fuzzy_match(prompt, &name) - 2 * fzf_char_match(prompt, &name) - (int)(name.len / 2);
		add_result(child, score);
		free(name.str);
	}
}

static fzf_retval async_start(void* args)
{
	fzf_string* prompt = (fzf_string*)args;

	for (size_t i = 0; i < s_root.len; i++)
	{
		fzf_thread_testcancel();
		fzf_dirnode* child = &s_root.children[i];

		if (child->is_dir)
		{
			scores_recurse(child, prompt);
			continue;
		}

		fzf_string name = str_tolower(child->name.str);
		int score = fzf_fuzzy_match(prompt, &name) - 2 * fzf_char_match(prompt, &name) - (int)(name.len / 2);
		add_result(child, score);
		free(name.str);
	}

	fzf_thread_exit(0);
}

void fzf_start(fzf_string* prompt)
{
	if (fzf_thread)
	{
		fzf_thread_cancel(fzf_thread);
		fzf_thread_join(fzf_thread);
	}

	memset(&s_results, 0, sizeof(s_results));
	s_results.scores[0].node = NULL;
	s_results.scores[0].score = 9999;
	s_results.greatest[0] = &s_results.scores[0];
	s_results.len++;

	fzf_thread = fzf_thread_create(async_start, (void*)prompt);
	fzf_thread_detach(fzf_thread);
}

fzf_output fzf_get_output()
{
	fzf_output out;
	fzf_string* str = out.results;

	fzf_mutex_lock(fzf_mutex);

	size_t count = 0;
	for (size_t i = s_results.len; i > 0; i--)
	{
		fzf_dirnode* node = s_results.greatest[i - 1]->node;
		if (!node) continue;

		*str = node->name;
		count++;
		str++;
	}

	fzf_mutex_unlock(fzf_mutex);

	out.len = count;
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
		return (int)str2->len;

	if (str2->len == 0)
		return (int)str1->len;

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
