#include <stdio.h>
#include <vector.h>

#include "fuzzy.h"

#include <Windows.h>
#include <synchapi.h>

int main(int argc, char** argv) {
	fzf_init();
	const char* prompt = "fuzzy";

	fzf_start(prompt);

	while (true) {
		vector(string) paths = fzf_scores();

		paths = fzf_paths();
		for (u32 i = 0; i < paths.size; i++) {
			string* p = vector_at(string)(&paths, i);
			printf("%s\n", (char*)p->data);
			string_destroy(p);
		}

		fzf_cleanup(paths.data);
		Sleep(1000);
	}

	fzf_term();
}
