#include "utils.h"
#include <sstream>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
using namespace std;

vector<string> tokenizer(string &line) {
    vector<string> tokens;
    string token;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (c == '"') {
            in_quotes = !in_quotes; 
            continue;               
        }

        if (isspace(c) && !in_quotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty())
        tokens.push_back(token);

    return tokens;
}


bool dfs(string& target, const string& path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) return false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;

        if (name == target) {
            closedir(dir);
            return true;
        }

        if (entry->d_type == DT_DIR && name != "." && name != "..") {
            if (dfs(target, path + "/" + name)) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}

string display(string homedir) {
    char hostname[10000];
    char username[10000];
    gethostname(hostname, 10000);
    getlogin_r(username, 10000);

    char directory[10000];
    getcwd(directory, sizeof(directory));
    string dir(directory);

    if (dir.find(homedir) == 0) {
        dir.replace(0, homedir.size(), "~");
    }

    return "<" +  string(username) + "@" + string(hostname) + ": " + string(dir) + ">";
}
