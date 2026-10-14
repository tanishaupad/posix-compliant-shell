#include "process.h"
#include <iostream>
#include <unistd.h>
#include <sys/sysctl.h>
#include <signal.h>
#include <libproc.h>
#include <fcntl.h>
using namespace std;

void sysCommands(vector<string>& args, bool background) {
    pid_t pid = fork();

    if (pid < 0) { cerr << "Fork error" << endl; return; }

    if (pid == 0) {
        vector<char*> c_args;
        for (string& s: args) c_args.push_back((char*)s.c_str());
        c_args.push_back(nullptr);

        int status = execvp(c_args[0], c_args.data());
        if (status == -1) { cerr << "Command not executed" << endl; _exit(1); }
    }
    else {
        if (background) cout << pid << endl;
        else waitpid(pid, NULL, 0);
    }
}

char statusChar(int s) {
    switch (s) {
        case 1: // SRUN
        case 6: // SONPROC
            return 'R';
        case 2: // SSLEEP
            return 'S';
        case 3: // SSTOP
            return 'T';
        case 4: // SZOMB
            return 'Z';
        case 5: // SIDL
            return 'I';
        default:
            return '?';
    }
}

void process_info_custom(vector<string>& args) {
    pid_t pid = (args.size() == 1) ? getpid() : stoi(args[1]);

    if (kill(pid, 0) == -1) {
        cerr << "pinfo: process " << pid << " does not exist" << endl;
        return;
    }
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) <= 0) {
        perror("proc_pidpath");
        return;
    }

    // Basic info
    struct proc_bsdinfo bsdinfo;
    if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &bsdinfo, sizeof(bsdinfo)) <= 0) {
        perror("proc_pidinfo (bsdinfo)");
        return;
    }
    string state(1, statusChar(bsdinfo.pbi_status));

    
    if ((bsdinfo.pbi_flags & PROC_FLAG_CTTY) && bsdinfo.e_tpgid != 0 && bsdinfo.pbi_pgid == bsdinfo.e_tpgid) {
        state.push_back('+'); 
    }

    cout << "Process Status -- " << state << endl;
    cout << "Executable Path -- " << pathbuf << endl;

    
    struct proc_taskinfo taskinfo;
    if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskinfo, sizeof(taskinfo)) > 0) {
        cout << "Memory (resident size) -- " << taskinfo.pti_resident_size << " bytes" << endl;
    }
}

