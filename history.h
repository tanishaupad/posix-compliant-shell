#pragma once
#include <string>
#include <vector>
using namespace std;
#define MAX_HISTORY 20

class History {
public:
    vector<string> entries;
    string file;

    History();
    void load();
    void append(const string& cmd);
    void builtin(const vector<string>& args);
    void trimMaxxing();
};
