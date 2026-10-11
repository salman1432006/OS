#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 256
#define MAX_ARGS 32

int main() {
    char input[MAX_INPUT];

    printf("Skill 21 - Integrated Shell\n");
    printf("Supports commands, arguments and error recovery.\n");

    while (1) {
        printf("skill21> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            continue;

        if (strcmp(input, "exit") == 0) {
            printf("Cleaning up and exiting.\n");
            break;
        }

        char *args[MAX_ARGS];
        int argc = 0;

        char *token = strtok(input, " \t");

        while (token && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        if (argc == 0)
            continue;

        if (strcmp(args[0], "cd") == 0) {
            const char *path = argc > 1 ? args[1] : getenv("HOME");

            if (chdir(path) != 0)
                perror("cd");
            else
                printf("Directory changed successfully.\n");

            continue;
        }

        if (strcmp(args[0], "pwd") == 0) {
            char cwd[4096];

            if (getcwd(cwd, sizeof(cwd)))
                printf("%s\n", cwd);
            else
                perror("pwd");

            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(args[0], args);

            fprintf(stderr,
                    "Error: command '%s' not found or could not execute.\n",
                    args[0]);

            exit(127);
        }

        int status;

        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            continue;
        }

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);

            printf("Command finished with status %d\n", code);
        } else if (WIFSIGNALED(status)) {
            printf("Command terminated by signal %d\n",
                   WTERMSIG(status));
        }
    }

    return 0;
}