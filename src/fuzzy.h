#pragma once

#include <string.h>

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

void fzf_setup(char** ignore, int len);
fzf_output fzf_get_output(fzf_string* prompt);
int fzf_char_match(fzf_string* s1, fzf_string* s2);
int fzf_fuzzy_match(fzf_string* s1, fzf_string* s2);
