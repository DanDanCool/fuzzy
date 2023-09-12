#include <stdio.h>

#include <Windows.h>
#include <synchapi.h>

#undef interface

#include <vector.h>
#include "fuzzy.h"

int main(int argc, char** argv) {
	fzf_init();
	const char* prompt = "fuzzy";
	memptr buf = alloc256(1000);

	fzf_start(prompt);

	for (int i = 0; i < 100; i++) {
		vector(string) paths = fzf_scores();

		printf("--------------------\n");
		for (u32 i = 0; i < paths.size; i++) {
			string* p = vector_at(string)(&paths, i);
			printf("%s\n", (char*)p->data);
		}
		printf("--------------------\n");

		scanf("%s", buf.data);
		fzf_start(buf.data);
		Sleep(100);
	}

	fzf_term();
}
