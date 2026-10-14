#pragma once
#include <readline/readline.h>
#include <readline/history.h>

char* generator(const char* text, int state);
char** autocomplete(const char* text, int start, int end);
void initialize_autocomplete();
