#include <stdio.h>

#include "fuzzy.h"

int main(int argc, char** argv)
{
	char* prompt = "cmake";

	if (argc > 1)
		prompt = argv[1];

	char* ignore[] = {
		"../build"
	};

	fzf_setup(ignore, 1);

	fzf_string input = (fzf_string){ .str = prompt, .len = strlen(prompt) };
	fzf_output out = fzf_get_output(&input);

	printf("results\n");
	for (int i = 0; i < out.len; i++)
	{
		printf("%s\n", out.results[i]);
	}
}
