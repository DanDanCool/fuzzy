local ffi = require('ffi')

local lib = ffi.load('build/libfuzzy.so')

ffi.cdef[[
	typedef struct
	{
		char* str;
		size_t len;
	} fzf_string;

	typedef struct
	{
		fzf_string results[40];
		size_t len;
	} fzf_output;

	void fzf_init(char** ignore, int len);
	void fzf_start(fzf_string* prompt);
	fzf_output fzf_get_output();
	int fzf_char_match(fzf_string* s1, fzf_string* s2);
	int fzf_fuzzy_match(fzf_string* s1, fzf_string* s2);
]]

print("fzf_setup")

local ignore = ffi.new('char*[?]', 2)
ignore[0] = ffi.new('char[?]', #'./.git')
ffi.copy(ignore[0], './.git')

ignore[1] = ffi.new('char[?]', #'./build')
ffi.copy(ignore[1], './build')

lib.fzf_init(ignore, 2)

local prompt = ffi.new('fzf_string[?]', 1)
prompt[0].str = ffi.new('char[?]', #'main' + 1)
ffi.copy(prompt[0].str, 'main')
prompt[0].len = #'main'

lib.fzf_start(prompt)

for i = 0, 5 do
local output = lib.fzf_get_output()
local len = tonumber(output.len) - 1
print(len)
for i = 0, len do
	local string = output.results[i]
	print(ffi.string(string.str, string.len))
end
end
