// Author: Milos Ilic
// email: ilkesd91@gmail.com
// date: 11.07.2021.


#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <pwd.h>
#include <proc/readproc.h>

using namespace std;

string getStdoutFromCommand(string cmd) {
    string data;
    FILE *stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");

    if (stream) {
        while (!feof(stream)) {
            if (fgets(buffer, max_buffer, stream) != NULL) {
                data.append(buffer);
            }
        }

        pclose(stream);
    }

    return data;
}

void logAppThread() {
    vector<string> vec;
    int count = 0;
    FILE *fp = nullptr;
    const char *homedir = nullptr;
    PROCTAB* proc;
    proc_t proc_info;
    
    homedir = getenv("HOME");
    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    string path(homedir);
    path.append("/log-app");
    
    while (true) {
    fp = fopen(path.c_str(), "w");

    proc = openproc(PROC_FILLCOM | PROC_FILLSTATUS | PROC_FILLSTAT);

    memset(&proc_info, 0, sizeof(proc_info));

    while (readproc(proc, &proc_info) != NULL) {
        if (proc_info.vm_size != 0 && proc_info.state != 'Z') {
            vec.push_back(proc_info.cmd);
        }
    }
  
    vec.erase(unique(vec.begin(), vec.end()), vec.end());
  
    for (auto x : vec) {
        for (int j = 0; j < x.size(); j++) {
            if (x[j] == ' ' || x[j] == '-') {
                count++;
            }sudo apt-get install libprocps-dev
        }
        if (!x.empty() && count == 0) {
            fputs(x.c_str(), fp);
            fputs("\n", fp);
        }
        count = 0;
    }

    vec.erase(vec.begin(), vec.end());
    closeproc(proc);
    if (fp) {
        fclose(fp);
    }
    sleep(1);
    }
}

void activeCameraThread() {
    string ret;
    bool isActive = false;
    const char *homedir = nullptr;
    ofstream log_camera;
    
    homedir = getenv("HOME");
    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    string path(homedir);
    path.append("/log-camera");
    
    while (true) {
        log_camera.open(path, ofstream::out | ofstream::trunc);
        ret = getStdoutFromCommand("lsof /dev/video*");
        isActive = !ret.empty();
        long error = ret.find("error");
        
        if (isActive && error == 13) {
            isActive = false;
        }
        
        if (isActive) {
            log_camera << "active camera: " << "true" << endl;
        }
        else {
            log_camera << "active camera: " << "false" << endl;
        }
        log_camera.close();
        sleep(1);
    }
}

int main() {
    thread t1(logAppThread);
    thread t2(activeCameraThread);
    
    t2.join();
    t1.join();

    return 0;
}
