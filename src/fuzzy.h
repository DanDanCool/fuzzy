#pragma once

#include <strlib.h>
#include <vector.h>
#include <macros.h>

typedef struct path path;
typedef path* ppath;
typedef struct score score;
typedef struct path_ignore path_ignore;

struct path {
	path* parent;
	string name;
};

STR_DECLARE(path);
VECTOR_DECLARE(ppath);

path* path_create(string name, path* parent);
void path_destroy(path* p);

struct score {
	u16 fuzzy;
};

VECTOR_DECLARE(score);
STRTABLE_DECLARE(score);

JOLLY_API
void fzf_init();

JOLLY_API
void fzf_term();

JOLLY_API
void fzf_start(cstr prompt);

JOLLY_API
vector(string) fzf_scores();

JOLLY_API
vector(string) fzf_paths();
