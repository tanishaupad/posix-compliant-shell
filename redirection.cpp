#include "redirection.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

void Redirection::parse(vector<string>& args) {
    infile.clear();
    outfile.clear();
    append = false;

    vector<string> clean;
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "<" && i + 1 < args.size()) {
            infile = args[i + 1];
            i++;
        } else if (args[i] == ">" && i + 1 < args.size()) {
            outfile = args[i + 1];
            append = false;
            i++;
        } else if (args[i] == ">>" && i + 1 < args.size()) {
            outfile = args[i + 1];
            append = true;
            i++;
        } else {
            clean.push_back(args[i]);
        }
    }
    args = clean; 
}

void Redirection::apply() {
    apply_out();
}

void Redirection::apply_in() {
    if (!infile.empty()) {
        int fd = open(infile.c_str(), O_RDONLY);
        if (fd < 0) { perror("open infile"); return; }
        if (dup2(fd, STDIN_FILENO) < 0) { perror("dup2 infile"); }
        close(fd);
    }
}

void Redirection::apply_out() {
    if (!outfile.empty()) {
        int fd;
        if (append) fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        else        fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror("open outfile"); return; }
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2 outfile"); }
        close(fd);
    }
}