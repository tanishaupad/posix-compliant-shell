#pragma once
#include <string>
#include <vector>
using namespace std;

vector<string> tokenizer(string &line);
bool dfs(string& target, const string& path);
string display(string homedir);
