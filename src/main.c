#include <stdio.h>

//#include "fuzzy.h"
#include <windows.h>
#include <fileapi.h>

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

	HANDLE directory;
	WIN32_FIND_DATA dir_entry;

	directory = FindFirstFile("../*", &dir_entry);
	if (directory == INVALID_HANDLE_VALUE)
	{
		printf("Invalid handle value!");
		return 0;
	}

	int count = 1;
	printf("%s\n", dir_entry.cFileName);

	while(FindNextFile(directory, &dir_entry))
	{
		printf("%s\n", dir_entry.cFileName);

		if (dir_entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			printf("directory!\n");
		count++;
	}

	printf("%i\n", count);

	FindClose(directory);
}
