#include "score.h"
#include "memory.h"

u16 char_match(string s1, string s2) {
	char chars[256];
	zero8(chars, sizeof(chars));

	for (u64 i = 0; i < s2.size; i++)
		chars[s2.data[i]] = 1;

	u16 matches = 0;
	for (int i = 0; i < s1.size; i++)
		matches += chars[s1.data[i]];

	return matches;
}

// implements levenshtein distance
u16 fuzzy_match(string s1, string s2) {
	if (s1.size == 0)
		return (u16)s2.size;

	if (s2.size == 0)
		return (u16)s1.size;

	vector(u16) edits;
	vector_create(u16)(&edits, (u32)s2.size + 1);

	for (u64 i = 0; i <= s2.size; i++) {
		u16 val = (u16)i;
		vector_set(u16)(&edits, &val, (u32)i);
	}

	for (u64 i = 0; i < s1.size; i++)	{
		u16 corner = (u16)i;
		u16 val = (u16)i + 1;
		vector_set(u16)(&edits, &val, 0);

		for (u64 j = 0; j < s2.size; j++) {
			u16 upper = *vector_at(u16)(&edits, (u32)j + 1);
			if (s1.data[i] == s2.data[j]) {
				u16 val = corner;
				vector_set(u16)(&edits, &val, (u32)j + 1);
			} else {
				u16 cur = *vector_at(u16)(&edits, (u32)j);
				u16 min = MIN(corner, upper);
				min = MIN(min, cur) + 1;
				vector_set(u16)(&edits, &min, (u32)j + 1);
			}

			corner = upper;
		}
	}

	u16 result = *vector_at(u16)(&edits, (u32)s2.size);
	vector_destroy(u16)(&edits);

	return result;
}
