# fuzzy
Please don't use this.

[fzf](https://github.com/junegunn/fzf) is a much better tool

To use with my vim config, build and copy the shared library into ~/.config/nvim/bin

Only tested on linux and windows, probably won't work on other platforms.

# reasoning
I was rewriting a bunch of vim plugins from scratch and the included vim plugin in fzf
was good, but far too feature rich then I needed. Using ffi and lua a shared library
written in C can seamlessly integrate with neovim without the need for rpc.
