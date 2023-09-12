#include <stdio.h>

#include <Windows.h>
#include <synchapi.h>

#undef interface
#undef INTERFACE

#include <vector.h>
#include "fuzzy.h"

int main(int argc, char** argv) {
	fzf_init();
	memptr buf = alloc256(1000);

	fzf_start("testprompt");

	const char* prompts[5];
	prompts[0] = "hello";
	prompts[1] = "orld";
	prompts[2] = "eeyore";
	prompts[3] = "debug";
	prompts[4] = "test";

	int idx = 0;
	for (int i = 0; i < 100; i++) {
		vector(string) paths = fzf_scores();

		printf("--------------------\n");
		for (u32 i = 0; i < paths.size; i++) {
			string* p = vector_at(string)(&paths, i);
			printf("%s\n", (char*)p->data);
		}
		printf("--------------------\n");

		fzf_start(prompts[idx]);
		idx = (idx + 1) % 5;
		//Sleep(100);
	}

	fzf_term();
}
