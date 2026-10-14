#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include <fcntl.h>
#include <array>
#include <readline/readline.h>
#include <readline/history.h>

#include "redirection.h"
#include "builtins.h"
#include "process.h"
#include "utils.h"
#include "history.h"
#include "autocomplete.h"

using namespace std;

// ---------------- PIPELINE FUNCTIONS ----------------
vector<vector<string>> split_pipes(const string& cmdline) {
    vector<vector<string>> result;
    stringstream ss(cmdline);
    string part;
    while (getline(ss, part, '|')) {
        size_t start = part.find_first_not_of(" \t\n\r");
        size_t end   = part.find_last_not_of(" \t\n\r");
        if (start != string::npos) {
            string cleaned = (end != string::npos) ? part.substr(start, end - start + 1) : part.substr(start);
            result.push_back(tokenizer(cleaned));
        }
    }
    return result;
}

void run_pipeline(vector<vector<string>>& stages, bool background) {
    int n = (int)stages.size();
    if (n == 0) return;

    vector<array<int,2>> pipes(max(0, n - 1));
    for (int i = 0; i < n - 1; ++i)
        if (pipe(pipes[i].data()) < 0) { perror("pipe"); return; }

    vector<pid_t> pids; pids.reserve(n);

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return; }

        if (pid == 0) {
            if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
            if (i < n - 1) dup2(pipes[i][1], STDOUT_FILENO);

            for (int k = 0; k < n - 1; ++k) { close(pipes[k][0]); close(pipes[k][1]); }

            vector<string> args = stages[i];
            Redirection redir; redir.parse(args);

            if (i == 0) redir.apply_in();
            if (i == n - 1) redir.apply_out();

            if (args.empty()) _exit(0);

            const string &cmd = args[0];
            if (cmd == "pwd") { pwd_custom(); _exit(0); }
            else if (cmd == "echo") { echo_custom(args); _exit(0); }
            else if (cmd == "ls") { ls_custom(args); _exit(0); }
            else if (cmd == "pinfo") { process_info_custom(args); _exit(0); }
            else if (cmd == "search") { search_custom(args); _exit(0); }

            vector<char*> cargs;
            cargs.reserve(args.size() + 1);
            for (auto &s : args) cargs.push_back(const_cast<char*>(s.c_str()));
            cargs.push_back(nullptr);

            execvp(cargs[0], cargs.data());
            perror("execvp"); _exit(127);
        } else pids.push_back(pid);
    }

    for (int k = 0; k < n - 1; ++k) { close(pipes[k][0]); close(pipes[k][1]); }

    if (!background) for (pid_t pid : pids) waitpid(pid, nullptr, 0);
    else { for (pid_t pid : pids) cout << pid << " "; cout << endl; }
}

// ---------------END PIPELINE FUNCTIONS -----------------

vector<string> split_commands(const string& line) {
    vector<string> result;
    stringstream ss(line);
    string cmd;
    while (getline(ss, cmd, ';')) {
        size_t start = cmd.find_first_not_of(" \t\n\r");
        size_t end   = cmd.find_last_not_of(" \t\n\r");
        if (start != string::npos && end != string::npos)
            result.push_back(cmd.substr(start, end - start + 1));
        else if (start != string::npos)
            result.push_back(cmd.substr(start));
    }
    return result;
}

int main() {
    char cwd[10000];
    getcwd(cwd, sizeof(cwd));
    string homedir(cwd);

    History history;
    history.load();  

    rl_readline_name = "minishell";
    rl_attempted_completion_function = autocomplete;

    while (true) {
        string prompt = display(homedir);  
        char* input = readline(prompt.c_str());
        if (!input) { cout << endl; break; }

        string line(input); free(input);
        if (line.empty()) continue;

        history.append(line);          
        add_history(line.c_str());     

        vector<string> commands = split_commands(line);

        for (auto& cmdline : commands) {
            if (cmdline.empty()) continue;
            vector<string> args = tokenizer(cmdline);
            if (args.empty()) continue;

            bool background = false;
            if (!args.empty() && args.back() == "&") { background = true; args.pop_back(); }
            if (args.empty()) continue;

            auto parts = split_pipes(cmdline);
            if (parts.size() > 1) { run_pipeline(parts, background); continue; }

            Redirection redir; redir.parse(args);
            if (args.empty()) { cerr << "no command specified\n"; continue; }

            if (args[0] == "exit" || args[0] == "quit") return 0;
            if (args[0] == "cd") { cd_custom(args, homedir); continue; }
            if (args[0] == "history") { history.builtin(args); continue; }

            bool is_builtin_nonstate = (args[0] == "pwd" || args[0] == "echo" || args[0] == "ls" || args[0] == "pinfo" || args[0] == "search");

            if (is_builtin_nonstate && !background && redir.infile.empty() && redir.outfile.empty()) {
                if (args[0] == "pwd") pwd_custom();
                else if (args[0] == "echo") echo_custom(args);
                else if (args[0] == "ls") ls_custom(args);
                else if (args[0] == "pinfo") process_info_custom(args);
                else if (args[0] == "search") search_custom(args);
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) { perror("fork"); continue; }

            if (pid == 0) {
                if (is_builtin_nonstate) {
                    redir.apply();
                    if (args[0] == "pwd") pwd_custom();
                    else if (args[0] == "echo") echo_custom(args);
                    else if (args[0] == "ls") ls_custom(args);
                    else if (args[0] == "pinfo") process_info_custom(args);
                    else if (args[0] == "search") search_custom(args);
                    _exit(0);
                }
                redir.apply();
                vector<char*> cargs;
                for (auto &s : args) cargs.push_back(const_cast<char*>(s.c_str()));
                cargs.push_back(nullptr);
                execvp(cargs[0], cargs.data());
                perror("execvp");
                _exit(127);
            } else {
                if (!background) waitpid(pid, nullptr, 0);
                else cout << pid << endl;
            }
        }
    }
    return 0;
}
