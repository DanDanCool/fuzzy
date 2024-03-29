#include "task.h"
#include "platform.h"
#include "score.h"
#include "fuzzy.h"
#include <scheduler.h>

void pathtraverse_task(taskinfo* in, u32 gen) {
	in_pathtraverse* args = (in_pathtraverse*)in->args;

	path tmp = { args->in_dir, string_create("*") };
	vector(string) dirs, paths;
	vector_create(string)(ref(paths), 0);
	vector_create(string)(ref(dirs), 0);

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

const u16 DEFAULT_SCORE = 32;

score score_task(vector(string)* prompt, string s) {
	score res = { 0 };
	res.fuzzy = I16_MAX;
	for (u32 i = 0; i < prompt->size; i++) {
		string p = *vector_at(string)(prompt, i);
		u16 fuzzy = fuzzy_match(p, s);
		res.fuzzy = MIN(res.fuzzy, fuzzy);
	}

	return res;
}

score getch_score(table(string, score)* t, string key, vector(string)* prompt, vector(string)* strings, vector(score)* scores) {
	score* res = table_get(string, score)(t, key);
	if (!res) {
		score s = score_task(prompt, key);
		vector_add(string)(strings, &key);
		vector_add(score)(scores, &s);
		return s;
	}
	return *res;
}

void accumulate_task(taskinfo* in, u32 gen) {
	if (in->gen != gen) return;

	in_accumulate* args = (in_accumulate*)in->args;
	vector(ppath)* paths = &args->in_paths;
	vector(pairsp)* scores = &args->out_matches;

	for (u32 i = 0; i < paths->size; i++) {
		ppath p = *vector_at(ppath)(paths, i);

		score res = getch_score(args->in_scores, p->name, &args->in_prompt, &args->out_strings, &args->out_scores);
		const i16 THRESHOLD = (i16)p->name.size / 4;
		const i16 DIRECTORY_MULTIPLIER = res.fuzzy <= THRESHOLD ? 2 :  1;

		ppath path = p->parent;
		i16 path_score = 0;
		i16 special = 0;
		int path_match = false;
		while (path) {
			score* tmp = table_get(string, score)(args->in_special, path->name);
			if (tmp) {
				special = tmp->fuzzy;
				break;
			}

			score s = getch_score(args->in_scores, path->name, &args->in_prompt, &args->out_strings, &args->out_scores);
			if (s.fuzzy <= 1) {
				path_match = true;
			}

			path_score += MAX(s.fuzzy / DIRECTORY_MULTIPLIER, 1);
			path = path->parent;
		}

		path_score /= path_match ? 64 : 1;
		path_score += special;
		res.fuzzy += path_score;

		pairsp tmp = { res, p };

		if (scores->size < args->in_count) {
			heap_add(pairsp)(scores, &tmp);
		} else {
			heap_replace(pairsp)(scores, &tmp);
		}
	}
}

int lt(pairsp)(pairsp* a, pairsp* b) {
	if (!a || !b) return false;
	return a->first.fuzzy > b->first.fuzzy;
}

int _lt(pairsp)(u8* a, u8* b) {
	return lt(pairsp)((pairsp*)a, (pairsp*)b);
}

int le(pairsp)(pairsp* a, pairsp* b) {
	if (!a || !b) return false;
	return a->first.fuzzy >= b->first.fuzzy;
}

int _le(pairsp)(u8* a, u8* b) {
	return le(pairsp)((pairsp*)a, (pairsp*)b);
}

COPY_DEFINE(pairsp);
SWAP_DEFINE(pairsp);

VECTOR_DEFINE(pairsp);
HEAP_DEFINE(pairsp);
