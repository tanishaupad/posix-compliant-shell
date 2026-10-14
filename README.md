
# Unix-style shell in C++

A small interactive shell built as an operating systems project. It uses process creation, pipes, file descriptor redirection, and other Unix system interfaces to run commands. The implementation is written in C++17 and was developed for macOS.

## Features

- Runs external programs with `fork` and `execvp`, waiting for foreground processes with `waitpid`.
- Supports pipelines (`|`), input redirection (`<`), output redirection (`>`), append redirection (`>>`), semicolon-separated commands, and a trailing `&` for background execution.
- Includes `cd`, `pwd`, `echo`, `ls`, `pinfo`, `search`, and `history` built-ins, plus `exit`/`quit`.
- Provides a Readline prompt with command history and tab completion. History is stored in `~/.shell_history`.

## Build and run

Requirements: a C++17 compiler, `make`, and Readline headers and library. On macOS, the process-inspection implementation also needs Apple's `libproc` headers.

```sh
make CXX=clang++
./main.out
```

Use `make clean` to remove the executable. The Makefile defaults to `g++`; `clang++` successfully builds this project on macOS. Linux support has not been verified because `pinfo` currently calls macOS-specific `libproc` APIs.

Example session:

```text
pwd
echo hello world
ls -la
echo hello | cat
history 5
exit
```

## Implementation

`main.cpp` parses command lines and starts commands and pipelines. `process.cpp` handles process execution and inspection. `redirection.cpp` maps files to standard input or output with `dup2`. `builtins.cpp` implements built-in commands, while `history.cpp`, `autocomplete.cpp`, and `utils.cpp` support the interactive prompt.

## Scope and limitations

The repository name reflects the use of POSIX-style system calls; this is **not a fully POSIX-compliant shell**. Its parser handles a subset of shell syntax and does not implement shell variables, expansion, command substitution, or general quoting and escaping. The `pinfo` built-in uses macOS-specific APIs. Background execution is basic and does not provide full job control. The shell's “home” shortcut refers to its startup directory.

The source was extracted from the shell assignment in [operating-systems-projects](https://github.com/tanishaupad/operating-systems-projects/tree/main/2025201088_A2).
