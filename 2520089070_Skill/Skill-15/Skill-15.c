#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_INPUT 512
#define MAX_CMDS 20
#define MAX_ARGS 32

int main() {
    char input[MAX_INPUT];

    printf("Skill 15 - Combined Pipes + Redirection\n");

    while (1) {
        printf("skill15> ");

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strlen(input) == 0)
            continue;

        char *commands[MAX_CMDS];
        int count = 0;

        char *token = strtok(input, "|");

        while (token && count < MAX_CMDS) {
            commands[count++] = token;
            token = strtok(NULL, "|");
        }

        int pipes[MAX_CMDS - 1][2];

        for (int i = 0; i < count - 1; i++)
            pipe(pipes[i]);

        for (int i = 0; i < count; i++) {

            pid_t pid = fork();

            if (pid == 0) {

                if (i > 0)
                    dup2(pipes[i - 1][0], STDIN_FILENO);

                if (i < count - 1)
                    dup2(pipes[i][1], STDOUT_FILENO);

                char *input_pos = strchr(commands[i], '<');
                char *output_pos = strchr(commands[i], '>');

                if (input_pos) {
                    *input_pos = '\0';

                    char *file = strtok(input_pos + 1, " \t");

                    int fd = open(file, O_RDONLY);

                    if (fd < 0) {
                        perror("input");
                        exit(1);
                    }

                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if (output_pos) {
                    int append = (output_pos[1] == '>');

                    *output_pos = '\0';

                    char *file = strtok(output_pos + (append ? 2 : 1), " \t");

                    int flags = O_WRONLY | O_CREAT;

                    if (append)
                        flags |= O_APPEND;
                    else
                        flags |= O_TRUNC;

                    int fd = open(file, flags, 0644);

                    if (fd < 0) {
                        perror("output");
                        exit(1);
                    }

                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                for (int j = 0; j < count - 1; j++) {
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

                if (argc == 0)
                    exit(0);

                execvp(args[0], args);

                perror("execvp");
                exit(1);
            }
        }

        for (int i = 0; i < count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < count; i++)
            wait(NULL);
    }

    return 0;
}