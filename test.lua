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

	void fzf_setup(char** ignore, int len);
	fzf_output fzf_get_output(fzf_string* input);
	int fzf_char_match(fzf_string* s1, fzf_string* s2);
	int fzf_fuzzy_match(fzf_string* s1, fzf_string* s2);
]]

print("fzf_setup")

local ignore = ffi.new('char*[?]', 2)
ignore[0] = ffi.new('char[?]', #'./.git')
ffi.copy(ignore[0], './.git')

ignore[1] = ffi.new('char[?]', #'./build')
ffi.copy(ignore[1], './build')

lib.fzf_setup(ignore, 2)

--local prompt = ffi.new('fzf_string[?]', 1)
--prompt[0].str = ffi.new('char[?]', #'fuzzy' + 1)
--ffi.copy(prompt[0].str, 'fuzzy')
--prompt[0].len = #'fuzzy'
--
--local comp = ffi.new('fzf_string[?]', 1)
--comp[0].str = ffi.new('char[?]', #'tuzzy' + 1)
--ffi.copy(comp[0].str, 'tuzzy')
--comp[0].len = #'tuzzy'

--local res = lib.fzf_fuzzy_match(prompt, comp)
--print(res)

local prompt = ffi.new('fzf_string[?]', 1)
prompt[0].str = ffi.new('char[?]', #'fuzzy' + 1)
ffi.copy(prompt[0].str, 'fuzzy')
prompt[0].len = #'fuzzy'

local output = lib.fzf_get_output(prompt)

local len = tonumber(output.len) - 1
for i = 0, len do
	local string = output.results[i]
	print(ffi.string(string.str, string.len))
	print(i)
end
