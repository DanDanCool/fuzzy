#pragma once

#include <vector.h>
#include <table.h>

#include "fuzzy.h"

typedef struct in_pathtraverse in_pathtraverse;
typedef struct in_score in_score;
typedef struct in_accumulate in_accumulate;

typedef struct fzf_state fzf_state;
typedef void (*pfn_callback)(fzf_state* state, void* args);

struct in_pathtraverse {
	pfn_callback callback;
	ppath in_dir;
	vector(ppath) out_dirs;
	vector(ppath) out_paths;
	u32 id;
};

typedef struct taskinfo taskinfo;

// finds paths
void pathtraverse_task(taskinfo* in, u32 gen);

PAIR_DECLARE(score, ppath);
typedef pair(score, ppath) pairsp;

VECTOR_DECLARE(pairsp);
HEAP_DECLARE(pairsp);

struct in_accumulate {
	pfn_callback callback;

	vector(string) out_strings;
	vector(score) out_scores;
	vector(pairsp) out_matches; // heap
								//
	vector(ppath) in_paths;
	vector(string) in_prompt;
	table(string, score)* in_scores;

	u32 in_count; // how many entries to return
	u32 id;
};

// accumulates scores and orders them through a heap
void accumulate_task(taskinfo* in, u32 gen);
