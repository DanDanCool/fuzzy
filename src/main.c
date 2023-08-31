#include <stdio.h>

#include "vector.h"

int main(int argc, char** argv) {
	char* prompt = "cmake";

	if (argc > 1)
		prompt = argv[1];

	vector(i32) v;
	vector_create(i32)(&v, 0);

	for (i32 i = 0; i < 100; i++) {
		vector_add(i32)(&v, &i);
	}

	vector_destroy(i32)(v);
}
