#include "history.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
using namespace std;

History::History() {
    char* home = getenv("HOME");
    if (home) file = string(home) + "/.shell_history";
    else file = "/.shell_history";

    load(); 
}

void History::load() {
    FILE* f_in = fopen(file.c_str(), "r");
    if (!f_in) return;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), f_in)) {
        string line(buffer);
        if (!line.empty() && line.back() == '\n') line.pop_back();
        if (!line.empty()) entries.push_back(line);
    }
    fclose(f_in);

    trimMaxxing();

    
    FILE* fout = fopen(file.c_str(), "w");
    if (fout) {
        for (auto &line : entries) fprintf(fout, "%s\n", line.c_str());
        fclose(fout);
    }
}

void History::append(const string& cmd) {
    if (cmd.empty()) return;

    
    if (!entries.empty() && entries.back() == cmd) return;

    entries.push_back(cmd);
    trimMaxxing();

    FILE* fout = fopen(file.c_str(), "w"); 
    if (!fout) return;
    for (auto &line : entries) fprintf(fout, "%s\n", line.c_str());
    fclose(fout);
}

void History::builtin(const vector<string>& args) {
    int n = 10;
    if (args.size() > 1) {
        try { n = stoi(args[1]); }
        catch (...) { cerr << "Invalid number" << endl; return; }
    }

    if (n > (int)entries.size()) n = entries.size();
    int start = entries.size() - n;
    for (int i = 0; i < n; i++) {
        cout << (i + 1) << " " << entries[start + i] << endl;
    }
}

void History::trimMaxxing() {
    if ((int)entries.size() > MAX_HISTORY) {
        entries.erase(entries.begin(), entries.end() - MAX_HISTORY);
    }
}
