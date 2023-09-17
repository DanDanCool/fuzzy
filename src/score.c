#include "score.h"
#include <vector.h>

i16 char_match(string s1, string s2) {
	char chars[256];
	zero8(chars, sizeof(chars));

	for (u64 i = 0; i < s2.size; i++)
		chars[s2.data[i]] = 1;

	i16 matches = 0;
	for (int i = 0; i < s1.size; i++)
		matches += chars[s1.data[i]];

	return matches;
}

// implements levenshtein distance
i16 fuzzy_match(string s1, string s2) {
	if (s1.size == 0)
		return (i16)s2.size;

	if (s2.size == 0)
		return (i16)s1.size;

	vector(i16) edits;
	vector_create(i16)(ref(edits), (u32)s2.size + 1);

	for (u64 i = 0; i <= s2.size; i++) {
		i16 val = (i16)i;
		vector_set(i16)(&edits, &val, (u32)i);
	}

	for (u64 i = 0; i < s1.size; i++)	{
		i16 corner = (i16)i;
		i16 val = (i16)i + 1;
		vector_set(i16)(&edits, &val, 0);

		for (u64 j = 0; j < s2.size; j++) {
			i16 upper = *vector_at(i16)(&edits, (u32)j + 1);
			if (s1.data[i] == s2.data[j]) {
				i16 val = corner;
				vector_set(i16)(&edits, &val, (u32)j + 1);
			} else {
				i16 cur = *vector_at(i16)(&edits, (u32)j);
				i16 min = MIN(corner, upper);
				min = MIN(min, cur) + 1;
				vector_set(i16)(&edits, &min, (u32)j + 1);
			}

			corner = upper;
		}
	}

	i16 result = *vector_at(i16)(&edits, (u32)s2.size);
	vector_destroy(i16)(&edits);

	return result;
}
