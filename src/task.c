#include "task.h"

void gitignore_task(void* in)
{
	in_gitignore* data = (in_gitignore*)in;
}

void pathtraverse_task(void* in)
{
	in_pathtraverse* data = (in_pathtraverse*)in;

	queue(ppath) directories = { data->out_paths, 0, data->out_paths.size };
	MEM_GUARD(vector(u8), vector_destroy(u8)) pathstr;
	vector_init(u8)(&pathstr, jolly_alloc(256), 256);

	for (u32 i = data->out_paths.size; i < data->out_paths.reserve; i++)
	{
		ppath path = *queue_pop(ppath)(&directories);
		pathtostr(&path, &pathstr);
	}
}

void score_task(void* in)
{
	in_score* data = (in_score*)in;

	make_iterator(vector(pstring), iter, &data->in_strings);
	for_each (string, iter);
	{

		vector_add(score)(&data->out_scores, score);
	}
}

void accumulate_task(void* in)
{
	in_accumulate* data = (in_accumulate*)in;

	make_iterator(vector(ppath), iter, &data->in_paths)
	for_each (path, iter)
	{

		heap_replace(&data->out_total_scores, score);
	}
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

