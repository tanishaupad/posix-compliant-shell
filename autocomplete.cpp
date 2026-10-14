#include "autocomplete.h"
#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;

static vector<string> matches;
static size_t match_index = 0;


vector<string> get_commands() {
    vector<string> cmds;
    const char* path_env = getenv("PATH");
    if (!path_env) return cmds;

    string path_str(path_env);
    size_t start = 0, end;
    while ((end = path_str.find(':', start)) != string::npos) {
        string dir = path_str.substr(start, end - start);
        DIR* dp = opendir(dir.c_str());
        if (dp) {
            struct dirent* ep;
            while ((ep = readdir(dp))) {
                if (ep->d_type == DT_REG || ep->d_type == DT_LNK || ep->d_type == DT_UNKNOWN)
                    cmds.push_back(ep->d_name);
            }
            closedir(dp);
        }
        start = end + 1;
    }
    
    string dir = path_str.substr(start);
    DIR* dp = opendir(dir.c_str());
    if (dp) {
        struct dirent* ep;
        while ((ep = readdir(dp))) {
            if (ep->d_type == DT_REG || ep->d_type == DT_LNK || ep->d_type == DT_UNKNOWN)
                cmds.push_back(ep->d_name);
        }
        closedir(dp);
    }
    return cmds;
}


char* generator(const char* text, int state) {
    static vector<string> candidates;
    static size_t index;
    if (state == 0) {
        candidates.clear();
        index = 0;

        string prefix(text);

        
        vector<string> cmds = get_commands();
        for (auto& c : cmds) {
            if (c.find(prefix) == 0) candidates.push_back(c);
        }

        
        DIR* dp = opendir(".");
        if (dp) {
            struct dirent* ep;
            while ((ep = readdir(dp))) {
                string name = ep->d_name;
                if (name.find(prefix) == 0) candidates.push_back(name);
            }
            closedir(dp);
        }
    }

    if (index < candidates.size())
        return strdup(candidates[index++].c_str());
    else
        return nullptr;
}


char** autocomplete(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;  
    return rl_completion_matches(text, generator);
}


void init_autocomplete() {
    rl_attempted_completion_function = autocomplete;
}
