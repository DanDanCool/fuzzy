#include <stdio.h>

<<<<<<< HEAD
//#include "fuzzy.h"
#include <windows.h>
#include <fileapi.h>
=======
#include <sys/types.h>
#include <dirent.h>

#include "fuzzy.h"
>>>>>>> f1822ba28ee1bafbd00ed5d8f40a983c052519e7

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

<<<<<<< HEAD
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
=======
	//DIR* directory = opendir(".");
	//if (!directory)
	//	return 0;

	//struct dirent* dir_entry;
	//size_t count = 0;

	//while (dir_entry = readdir(directory))
	//{
	//	if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
	//		continue;

	//	count++;
	//	printf("%s\n", dir_entry->d_name);
	//}

	//printf("%i\n", count);

	//closedir(directory);
>>>>>>> f1822ba28ee1bafbd00ed5d8f40a983c052519e7
}
