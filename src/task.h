#pragma once

typedef struct in_gitignore in_gitignore;
typedef struct in_pathtraverse in_pathtraverse;
typedef struct in_score in_score;
typedef struct in_accumulate in_accumulate;

struct in_gitignore
{
	mem_pool* in_strallocator;
	mem_arena* in_allocator;
	path_ignore out_ignore;
};

// looks through gitignore, adds ignore paths
void gitignore_task(void* in);

struct in_pathtraverse
{
	mem_pool* in_strallocator;
	mem_arena* in_allocator;
	vector(pstring) out_strings;
	vector(ppath) out_directories; // should initalize with desired paths to traverse
	vector(ppath) out_paths;
};

// finds paths
void pathtraverse_task(void* in);

struct in_score
{
	vector(pstring)* in_prompt;
	vector(pstring) in_strings;
	vector(score) out_scores;
};

// scores strings
void score_task(void* in);

struct in_accumulate
{
	hash_table(pstring, score)* in_scores;
	vector(ppath) in_paths;
	vector(ppath) out_totalscores; // heap
};

// accumulates scores and orders them through a heap
void accumulate_task(void* in);
