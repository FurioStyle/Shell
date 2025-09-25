#include "miprof.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <sys/resource.h>
#include <signal.h>
#include <fstream>
#include <iomanip>
#include <chrono>

using namespace std;

// =============================
// miprof: ejecución con medición
// =============================
void runWithProfile(const vector<char*> &args, bool saveToFile=false, const string &filename="", int maxtime=-1) {
    pid_t pid = fork();
    auto start = chrono::high_resolution_clock::now();

    if (pid == 0) { // hijo
        if (maxtime > 0) {
            alarm(maxtime); // terminar si excede tiempo
        }
        execvp(args[0], const_cast<char* const*>(args.data()));
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // padre
        int status;
        waitpid(pid, &status, 0);

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> real_time = end - start;

        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);

        // resultados
        ostringstream result;
        result << "Comando: ";
        for (int i=0; args[i]!=nullptr; i++) result << args[i] << " ";
        result << "\nTiempo real: " << fixed << setprecision(3) << real_time.count() << "s"
               << "\nTiempo usuario: " << usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1e6 << "s"
               << "\nTiempo sistema: " << usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1e6 << "s"
               << "\nMaxRSS: " << usage.ru_maxrss << " KB\n";

        if (saveToFile) {
            ofstream out(filename, ios::app);
            out << result.str() << "\n";
            out.close();
        } else {
            cout << result.str() << endl;
        }
    }
}

// =============================
// Procesar comando "miprof"
// =============================
void handleMiprof(const vector<string> &tokens) {
    if (tokens.size() < 3) {
        cerr << "Uso: miprof [ejec|ejcsave archivo|ejecutar maxtime] comando args...\n";
        return;
    }

    if (tokens[1] == "ejec") {
        vector<char*> args;
        for (size_t i=2; i<tokens.size(); i++) {
            args.push_back(strdup(tokens[i].c_str()));
        }
        args.push_back(nullptr);
        runWithProfile(args);
    }
    else if (tokens[1] == "ejcsave") {
        if (tokens.size() < 4) {
            cerr << "Uso: miprof ejcsave archivo comando args...\n";
            return;
        }
        string file = tokens[2];
        vector<char*> args;
        for (size_t i=3; i<tokens.size(); i++) {
            args.push_back(strdup(tokens[i].c_str()));
        }
        args.push_back(nullptr);
        runWithProfile(args, true, file);
    }
    else if (tokens[1] == "ejecutar") {
        if (tokens.size() < 4) {
            cerr << "Uso: miprof ejecutar maxtime comando args...\n";
            return;
        }
        int maxtime = stoi(tokens[2]);
        vector<char*> args;
        for (size_t i=3; i<tokens.size(); i++) {
            args.push_back(strdup(tokens[i].c_str()));
        }
        args.push_back(nullptr);
        runWithProfile(args, false, "", maxtime);
    }
    else {
        cerr << "Subcomando no reconocido para miprof: " << tokens[1] << "\n";
        return; // vuelve al prompt
    }
}
