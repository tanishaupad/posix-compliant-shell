#pragma once
#include <string>
#include <vector>
using namespace std;

class Redirection {
public:
    string infile;
    string outfile;
    bool append = false;

    void parse(vector<string>& args);
    void apply();
    void apply_in();
    void apply_out();
};

