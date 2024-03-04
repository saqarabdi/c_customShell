#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 100
#define MAX_LINE 1024
#define MAX_HISTORY 100

char history[MAX_HISTORY][MAX_LINE];
int history_index = 0;

void add_history(char *line) {
    strcpy(history[history_index % MAX_HISTORY], line);
    history_index++;
}

void print_history() {
    for (int i = 0; i < history_index; i++) {
        printf("%d  %s", i % MAX_HISTORY, history[i % MAX_HISTORY]);
    }
}

void clear_history() {
    for (int i = 0; i < MAX_HISTORY; i++) {
        history[i][0] = '\0';  // Clear each history entry
    }
    history_index = 0;
}

int execute_command(char **args, int arg_count);

int execute_history(int offset) {
    if (offset < 0 || offset >= MAX_HISTORY || history[offset % MAX_HISTORY][0] == '\0') {
        printf("Invalid history offset\n");
        return 1;
    }

    char *args[MAX_ARGS];
    int arg_count = 0;
    args[arg_count++] = strtok(history[offset % MAX_HISTORY], " ");
    while ((args[arg_count++] = strtok(NULL, " ")) != NULL) {
        if (arg_count >= MAX_ARGS) {
            fprintf(stderr, "Too many arguments\n");
            arg_count--;
            break;
        }
    }

    return execute_command(args, arg_count);  // Reuse execute_command function
}

int execute_command(char **args, int arg_count) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "cd: invalid number of arguments\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
    } else if (strcmp(args[0], "history") == 0) {
        if (arg_count == 1) {
            print_history();
        } else if (strcmp(args[1], "-c") == 0) {
            clear_history();
        } else {
            int offset = atoi(args[1]);
            return execute_history(offset);
        }
    } else {
        // Execute external command using fork and execvp
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
        } else if (pid == 0) {
            // Child process
            if (execvp(args[0], args) < 0) {
                perror("execvp");
                exit(1);
            }
        } else {
            // Parent process
            wait(NULL);  // Wait for child to finish
        }
    }

    return 0;
}

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("sish> ");
        fflush(stdout);

        if (fgets(line, MAX_LINE, stdin) == NULL) {
            perror("fgets");
            exit(1);
        }

        // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';

        // Tokenize the input line
        int arg_count = 0;
        args[arg_count++] = strtok(line, " ");
        while ((args[arg_count++] = strtok(NULL, " ")) != NULL) {
            if (arg_count >= MAX_ARGS) {
                fprintf(stderr, "Too many arguments\n");
                arg_count--;
                break;
            }
        }

        // Execute the command
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
        } else if (pid == 0) {
            // Child process
            if (execvp(args[0], args) < 0) {
                perror("execvp");
                exit(1);
            }
        } else {
            // Parent process
            wait(NULL);  // Wait for child to finish
        }
    }

    return 0;
}
