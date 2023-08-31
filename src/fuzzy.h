#pragma once

typedef struct string string;
typedef string* pstring;
typedef struct path path;
typedef path* ppath;
typedef struct score score;
typedef struct path_ignore path_ignore;

struct string {
	char buf[STRING_LENGTH];
};

struct path {
	vector(pstring) subpaths;
};

struct score {
	u16 fuzzy;
};

struct path_ignore {
	vector(ppath) paths;
	vector(u8) flags;
};

typedef struct fzf_args fzf_args;
struct fzf_args {
	cstr ignore;
	u8 max_results;
	u8 bfs_depth;
};

// ignores paths/files in .gitignore whose path is provided by ignore
void fzf_init(fzf_args* args);
void fzf_destroy();
void fzf_start(cstr prompt, u32 size);
void fzf_scores(vector_ppath* v);
