#pragma once

#define MAX_RESULTS 40

typedef struct
{
	char* str;
	size_t len;
} fzf_string;

typedef struct
{
	fzf_string results[MAX_RESULTS];
	size_t len;
} fzf_output;

void fzf_init(char** ignore, int len);
void fzf_start(fzf_string* prompt);
fzf_output fzf_get_output();
int fzf_char_match(fzf_string* s1, fzf_string* s2);
int fzf_fuzzy_match(fzf_string* s1, fzf_string* s2);
