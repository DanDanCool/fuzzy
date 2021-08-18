#include <stdio.h>

#include "fuzzy.h"
#include "filesystem.h"

int main(int argc, char** argv)
{
	//char* prompt = "cmake";

	//if (argc > 1)
	//	prompt = argv[1];

	//char* ignore[] = {
	//	"../build"
	//};

	//fzf_setup(ignore, 1);

	//fzf_string input = (fzf_string){ .str = prompt, .len = strlen(prompt) };
	//fzf_output out = fzf_get_output(&input);

	//printf("results\n");
	//for (int i = 0; i < out.len; i++)
	//{
	//	printf("%s\n", out.results[i]);
	//}

	fzf_dirnode root = { (fzf_string){ .str = ".", .len = 1 },
		.children = NULL, .len = 0, .is_dir = 1 };

	fzf_read_directory(&root);

	for (size_t i = 0; i < root.len; i++)
	{
		printf("name: %s, dir: %i\n", root.children[i].name.str, root.children[i].is_dir);
	}
}
