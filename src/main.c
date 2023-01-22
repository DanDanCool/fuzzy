#include <stdio.h>
#include <string.h>

#include "fuzzy.h"
#include "platform.h"

int main(int argc, char** argv)
{
	char* prompt = "cmake";

	if (argc > 1)
		prompt = argv[1];

	char* ignore[] = {
		"../build"
	};

	fzf_init(ignore, 1);

    sleep(1);

	fzf_string input = (fzf_string){ .str = prompt, .len = strlen(prompt) };
	fzf_start(&input);

	int count = 5;
	while (count--)
	{
		fzf_output out = fzf_get_output();

		printf("results\n");
		for (int i = 0; i < out.len; i++)
		{
			printf("%s\n", out.results[i].str);
		}
	}
}
