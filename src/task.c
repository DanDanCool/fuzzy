#include "task.h"
#include "platform.h"
#include "score.h"
#include "fuzzy.h"

void pathtraverse_task(void* in) {
	in_pathtraverse* args = (in_pathtraverse*)in;

	path tmp = { args->in_dir, string_create("*") };
	vector(string) dirs, paths;
	vector_create(string)(&paths, 0);
	vector_create(string)(&dirs, 0);

	string fullpath = str(path)(&tmp);
	read_directory(fullpath, &dirs, &paths);

	for (u32 i = 0; i < dirs.size; i++) {
		path* p = path_create(*vector_at(string)(&dirs, i), args->in_dir);
		vector_add(ppath)(&args->out_dirs, &p);
	}

	for (u32 i = 0; i < paths.size; i++) {
		path* p = path_create(*vector_at(string)(&paths, i), args->in_dir);
		vector_add(ppath)(&args->out_paths, &p);
	}

	string_destroy(&tmp.name);
	string_destroy(&fullpath);
	vector_destroy(string)(&paths);
	vector_destroy(string)(&dirs);
}

void score_task(void* in) {
	in_score* args = (in_score*)in;
	vector(string)* in_prompt = &args->in_prompt;
	vector(string)* in_strings = &args->in_strings;

	for (u32 i = 0; i < in_strings->size; i++) {
		string str = *vector_at(string)(in_strings, i);
		score s = { 0 };
		s.fuzzy = U16_MAX;
		for (u32 j = 0; j < in_prompt->size; j++) {
			string prompt = *vector_at(string)(in_prompt, j);
			score res = { fuzzy_match(prompt, str) };
			s.fuzzy = MIN(s.fuzzy, res.fuzzy);
		}

		vector_add(score)(&args->out_scores, &s);
	}
}

static score score_tally(table(string, score)* scores, ppath path);

void accumulate_task(void* in) {
	in_accumulate* args = (in_accumulate*)in;
	vector(ppath)* in_paths = &args->in_paths;

	vector(pairsp) scores;
	vector_create(pairsp)(&scores, 0);

	for (u32 i = 0; i < in_paths->size; i++) {
		pairsp tmp = { 0 };
		ppath p = *vector_at(ppath)(in_paths, i);
		tmp.first = score_tally(args->in_scores, p);
		tmp.second = p;

		if (scores.size < args->in_count) {
			heap_add(pairsp)(&scores, &tmp);
		} else {
			heap_replace(pairsp)(&scores, &tmp);
		}
	}

	args->out_scores = scores;
}

static score score_tally(table(string, score)* scores, ppath path) {
	const u16 DIRECTORY_MULTIPLIER = 4;
	score* tmp = table_get(string, score)(scores, path->name);
	score res = {0};
	if (!tmp) {
		res.fuzzy = U16_MAX;
		return res;
	}

	res = *tmp;
	path = path->parent;
	while (path) {
		score* s = table_get(string, score)(scores, path->name);
		u32 fuzzy = s ? s->fuzzy : 4;
		res.fuzzy += fuzzy / DIRECTORY_MULTIPLIER;
		path = path->parent;
	}

	return res;
}

int lt(pairsp)(pairsp* a, pairsp* b) {
	if (!a || !b) return false;
	return a->first.fuzzy < b->first.fuzzy;
}

int _lt(pairsp)(u8* a, u8* b) {
	return lt(pairsp)((pairsp*)a, (pairsp*)b);
}

int le(pairsp)(pairsp* a, pairsp* b) {
	if (!a || !b) return false;
	return a->first.fuzzy <= b->first.fuzzy;
}

int _le(pairsp)(u8* a, u8* b) {
	return le(pairsp)((pairsp*)a, (pairsp*)b);
}

COPY_DEFINE(pairsp);
SWAP_DEFINE(pairsp);

VECTOR_DEFINE(pairsp);
HEAP_DEFINE(pairsp);
