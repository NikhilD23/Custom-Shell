#include <iostream>
#include <unistd.h>   // fork(), execvp(), chdir()
#include <sys/stat.h> // ✅ mkdir()
#include <sys/types.h>
#include <vector>
#include <sstream>
#include <fcntl.h>    // open() for file redirection
#include <sys/wait.h> // waitpid()

using namespace std;

// Store command history
vector<string> commandHistory;

// Function to split input into tokens
vector<string> splitCommand(const string& input, char delimiter) {
    vector<string> parts;
    stringstream ss(input);
    string segment;
    while (getline(ss, segment, delimiter)) {
        parts.push_back(segment);
    }
    return parts;
}

// Function to execute built-in or system commands
void executeCommand(vector<string> args, string outputFile = "") {
    if (args.empty()) return;

    if (args[0] == "exit") {
        exit(0);
    }

    string command = args[0];

    // ✅ Built-in "cd" command
    if (command == "cd") {
        if (args.size() < 2) cout << "cd: missing argument\n";
        else if (chdir(args[1].c_str()) != 0) perror("cd failed");
        return;
    }

    // ✅ Built-in "history" command
    if (command == "history") {
        for (int i = 0; i < commandHistory.size(); i++) {
            cout << i + 1 << " " << commandHistory[i] << endl;
        }
        return;
    }

    // ✅ Built-in "clear" command
    if (command == "clear") {
        cout << "\033[2J\033[H"; // ANSI escape sequence to clear terminal
        return;
    }

    // ✅ Built-in "mkdir" command
    if (command == "mkdir") {
        if (args.size() < 2) {
            cout << "mkdir: missing directory name\n";
        } else {
            if (mkdir(args[1].c_str(), 0755) != 0) perror("mkdir failed");
        }
        return;
    }

    // ✅ Built-in "rmdir" command
    if (command == "rmdir") {
        if (args.size() < 2) {
            cout << "rmdir: missing directory name\n";
        } else {
            if (rmdir(args[1].c_str()) != 0) perror("rmdir failed");
        }
        return;
    }

    // ✅ Handle Redirection (>)
    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (!outputFile.empty()) {
            int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("open failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // Redirect stdout to file
            close(fd);
        }

        vector<char*> argv;
        for (auto &arg : args) argv.push_back(&arg[0]);
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        perror("execvp failed");
        exit(1);
    } else { // Parent process
        waitpid(pid, nullptr, 0);
    }
}

// Function to execute piped commands (cmd1 | cmd2)
void executePipedCommands(string input) {
    vector<string> commands = splitCommand(input, '|');
    if (commands.size() < 2) {
        vector<string> cmdParts = splitCommand(commands[0], ' ');
        executeCommand(cmdParts);
        return;
    }

    int pipefd[2];
    pipe(pipefd);
    pid_t pid1 = fork();

    if (pid1 == 0) { // First child (cmd1)
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        vector<string> cmd1Parts = splitCommand(commands[0], ' ');
        executeCommand(cmd1Parts);
        exit(0);
    }

    pid_t pid2 = fork();

    if (pid2 == 0) { // Second child (cmd2)
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        vector<string> cmd2Parts = splitCommand(commands[1], ' ');
        executeCommand(cmd2Parts);
        exit(0);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
}

// Main shell loop
int main() {
    string input;
    while (true) {
        cout << "myShell> ";
        getline(cin, input);

        if (input == "exit") break;

        commandHistory.push_back(input); // Save to history

        // Handle redirection (>)
        vector<string> redirectionParts = splitCommand(input, '>');
        if (redirectionParts.size() == 2) {
            vector<string> cmdParts = splitCommand(redirectionParts[0], ' ');
            string outputFile = redirectionParts[1];
            executeCommand(cmdParts, outputFile);
        } else {
            executePipedCommands(input);
        }
    }
    return 0;
}
