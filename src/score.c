#include "score.h"

int fzf_char_match(fzf_string* str1, fzf_string* str2)
{
	char chars[256];
	memset(chars, 0, 256);

	for (int i = 0; i < str2->len; i++)
		chars[str2->str[i]] = 1;

	int matches = 0;
	for (int i = 0; i < str1->len; i++)
		matches += chars[str1->str[i]];

	return matches;
}

// implements levenshtein distance
int fzf_fuzzy_match(fzf_string* str1, fzf_string* str2)
{
	if (str1->len == 0)
		return (int)str2->len;

	if (str2->len == 0)
		return (int)str1->len;

	size_t* edits = (size_t*)malloc((str2->len + 1) * sizeof(size_t));
	for (size_t i = 0; i <= str2->len; i++)
		edits[i] = i;

	for (int i = 0; i < str1->len; i++)
	{
		size_t corner = i;
		edits[0] = i + 1;

		for (int j = 0; j < str2->len; j++)
		{
			size_t upper = edits[j + 1];
			if (str1->str[i] == str2->str[j])
			{
				edits[j + 1] = corner;
			}
			else
			{
				size_t min = corner > upper ? upper : corner;
				edits[j + 1] = (min > edits[j] ? edits[j] : min) + 1;
			}

			corner = upper;
		}
	}

	int result = (int)edits[str2->len];
	free(edits);

	return result;
}
