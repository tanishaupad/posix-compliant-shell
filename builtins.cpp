#include "builtins.h"
#include "utils.h"
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <iomanip>
using namespace std;

void cd_custom(vector<string>& args, string homedir) {
    if (args.size() > 2) { cerr << "Invalid arguments" << endl; return; }
    string path;
    static string prevdir = homedir;

    if (args.size() == 1 || args[1] == "~") {
        path = homedir;
    }
    else if (args[1] == "-") {
        char dir[10000];
        getcwd(dir, sizeof(dir));
        string currdir(dir);

        if (chdir(prevdir.c_str()) != 0) {
            cerr << "Directory change failed" << endl;
            return;
        } else {
            cout << prevdir << endl;
            prevdir = currdir;
        }
    }
    else {
        path = args[1];
    }

    if (chdir(path.c_str()) != 0) {
        cerr << "cd error" << endl;
        return;
    }
}

void pwd_custom() {
    char dir[10000];
    if (getcwd(dir, sizeof(dir)) != nullptr) cout << dir << endl;
    else perror("pwd error");
}

void echo_custom(vector<string>& args) {
    for (size_t i = 1; i < args.size(); i++) {
        cout << args[i];
        if (i < args.size() - 1) cout << " ";
    }
    cout << endl;
}

void ls_custom(vector<string>& args) {
    bool showAll = false, longFormat = false;
    vector<string> paths;

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i][0] == '-') {
            if (args[i].find('a') != string::npos) showAll = true;
            if (args[i].find('l') != string::npos) longFormat = true;
        } else {
            paths.push_back(args[i]);
        }
    }

    if (paths.empty()) paths.push_back(".");

    for (size_t p = 0; p < paths.size(); p++) {
        if (paths.size() > 1) cout << paths[p] << ":" << endl;

        DIR *dir = opendir(paths[p].c_str());
        if (!dir) { perror("ls"); continue; }

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;
            if (!showAll && name[0] == '.') continue;

            if (longFormat) {
                struct stat st;
                string fullPath = paths[p] + "/" + name;
                if (stat(fullPath.c_str(), &st) == -1) { perror("stat"); continue; }

                cout << ((S_ISDIR(st.st_mode)) ? 'd' : '-');
                cout << ((st.st_mode & S_IRUSR) ? 'r' : '-');
                cout << ((st.st_mode & S_IWUSR) ? 'w' : '-');
                cout << ((st.st_mode & S_IXUSR) ? 'x' : '-');
                cout << ((st.st_mode & S_IRGRP) ? 'r' : '-');
                cout << ((st.st_mode & S_IWGRP) ? 'w' : '-');
                cout << ((st.st_mode & S_IXGRP) ? 'x' : '-');
                cout << ((st.st_mode & S_IROTH) ? 'r' : '-');
                cout << ((st.st_mode & S_IWOTH) ? 'w' : '-');
                cout << ((st.st_mode & S_IXOTH) ? 'x' : '-');

                cout << " " << st.st_nlink;

                struct passwd *pw = getpwuid(st.st_uid);
                struct group  *gr = getgrgid(st.st_gid);
                cout << " " << (pw ? pw->pw_name : to_string(st.st_uid));
                cout << " " << (gr ? gr->gr_name : to_string(st.st_gid));
                cout << " " << st.st_size;

                char timeBuf[80];
                strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", localtime(&st.st_mtime));
                cout << " " << timeBuf;
                cout << " " << name << endl;
            } else {
                cout << name << endl;
            }
        }
        closedir(dir);
        if (p < paths.size() - 1) cout << endl;
    }
}

void search_custom(vector<string>& args) {
    if (args.size() != 2) {
        cerr << "Usage: search <file_path>" << endl;
        return;
    }

    string target = args[1];
    char cwd[10000];
    getcwd(cwd, sizeof(cwd));
    string curr(cwd);

    if (!dfs(target, curr)) cout << "False" << endl;
    else cout << "True" << endl;
}
