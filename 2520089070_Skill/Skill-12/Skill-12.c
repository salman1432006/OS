#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 256
#define MAX_CMDS 20
#define MAX_ARGS 32

int main() {
    char input[MAX_INPUT];

    printf("Skill 12 - Multiple Pipes\n");

    while (1) {
        printf("skill12> ");

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strlen(input) == 0)
            continue;

        char *commands[MAX_CMDS];
        int cmd_count = 0;

        char *token = strtok(input, "|");

        while (token && cmd_count < MAX_CMDS) {
            commands[cmd_count++] = token;
            token = strtok(NULL, "|");
        }

        int pipes[MAX_CMDS - 1][2];

        for (int i = 0; i < cmd_count - 1; i++)
            pipe(pipes[i]);

        for (int i = 0; i < cmd_count; i++) {
            pid_t pid = fork();

            if (pid == 0) {

                if (i > 0)
                    dup2(pipes[i - 1][0], STDIN_FILENO);

                if (i < cmd_count - 1)
                    dup2(pipes[i][1], STDOUT_FILENO);

                for (int j = 0; j < cmd_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                char *args[MAX_ARGS];
                int argc = 0;

                char *arg = strtok(commands[i], " \t");

                while (arg && argc < MAX_ARGS - 1) {
                    args[argc++] = arg;
                    arg = strtok(NULL, " \t");
                }

                args[argc] = NULL;

                execvp(args[0], args);

                perror("execvp");
                exit(1);
            }
        }

        for (int i = 0; i < cmd_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < cmd_count; i++)
            wait(NULL);
    }

    return 0;
}