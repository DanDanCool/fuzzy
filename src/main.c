#include <stdio.h>
#include <vector.h>

#include "fuzzy.h"

#include <Windows.h>
#include <synchapi.h>

int main(int argc, char** argv) {
	fzf_init();
	const char* prompt = "fuzzy";
	memptr buf = alloc256(1000);

	fzf_start(prompt);

	while (true) {
		vector(string) paths = fzf_scores();

		printf("--------------------\n");
		for (u32 i = 0; i < paths.size; i++) {
			string* p = vector_at(string)(&paths, i);
			printf("%s\n", (char*)p->data);
			fzf_cleanup(p->data);
		}

		fzf_cleanup(paths.data);
		printf("--------------------\n");

		scanf("%s", buf.data);
		fzf_start(buf.data);
		Sleep(1000);
	}

	fzf_term();
}
