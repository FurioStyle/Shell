#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include "miprof.h"
#include <limits.h>


using namespace std;

//=========================
// Colores para el prompt
//=========================

#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE  "\033[1;34m"
#define COLOR_RESET "\033[0m"

//========================================
// Tokenizar una cadena de caracteres
//========================================
vector<char*> parseCommand(const string &input) {
    vector<char*> args;
    stringstream ss(input);
    string token;
    while (ss >> token) {
        char *arg = new char[token.size() + 1];
        strcpy(arg, token.c_str());
        args.push_back(arg);
    }
    args.push_back(nullptr);
    return args;
}
//==============================
// Ejecutar comandos con pipes
//==============================
void executePipedCommands(vector<string> &commands) {
    int num_cmds = commands.size();
    int pipefd[2 * (num_cmds - 1)];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd + i * 2) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (i > 0) dup2(pipefd[(i - 1) * 2], 0);
            if (i < num_cmds - 1) dup2(pipefd[i * 2 + 1], 1);

            for (int j = 0; j < 2 * (num_cmds - 1); j++) close(pipefd[j]);

            vector<char*> args = parseCommand(commands[i]);
            if (execvp(args[0], args.data()) == -1) {
                cerr << "Comando no reconocido: " << args[0] << endl;
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2 * (num_cmds - 1); i++) close(pipefd[i]);
    for (int i = 0; i < num_cmds; i++) wait(NULL);
}
//===================================
// Ejecutar comando simple (sin pipes)
//===================================
void executeSimpleCommand(const string &command) {
    vector<char*> args = parseCommand(command);
    if (args.empty() || args[0] == nullptr) return;

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args.data()) == -1) cerr << "Comando no reconocido: " << args[0] << endl;
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }

    for (auto arg : args) delete[] arg;
}

int main() {
    while (true) {
        // Mostrar prompt
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) { perror("getcwd"); continue; }
        cout << COLOR_YELLOW "-SOShell-" COLOR_RESET << COLOR_BLUE << cwd << COLOR_RESET << "-$ ";

        string input;
        if (!getline(cin, input)) break;
        if (input.empty()) continue;

        // Separar la línea en tokens
        istringstream iss(input);
        vector<string> tokens;
        string token;
        while (iss >> token) tokens.push_back(token);
        if (tokens.empty()) continue;

        // Comando exit
        if (tokens[0] == "exit") break;

        // Comando cd
        if (tokens[0] == "cd") {
            const char* path = (tokens.size() > 1) ? tokens[1].c_str() : getenv("HOME");
            if (chdir(path) != 0) perror("cd error");
            continue;
        }

        // Comando miprof
        if (tokens[0] == "miprof") {
            handleMiprof(tokens);
            continue;
        }

        // ======= Verificar si hay pipes =======
        bool hasPipe = false;
        for (auto &t : tokens) if (t == "|") { hasPipe = true; break; }

        if (hasPipe) {
            // Separar la línea en comandos por pipes
            vector<string> commands;
            string cmd;
            for (auto &t : tokens) {
                if (t == "|") {
                    if (!cmd.empty()) commands.push_back(cmd);
                    cmd.clear();
                } else {
                    if (!cmd.empty()) cmd += " ";
                    cmd += t;
                }
            }
            if (!cmd.empty()) commands.push_back(cmd);

            // Ejecutar comandos con pipes
            executePipedCommands(commands);
        } else {
            // Ejecutar comando simple
            executeSimpleCommand(input);
        }
    }

    return 0;
}
