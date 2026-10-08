#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_INPUT 256
#define MAX_ARGS 32

int main() {
    char input[MAX_INPUT];

    printf("Skill 13 - Input / Output Redirection\n");

    while (1) {
        printf("skill13> ");

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        char *infile = strstr(input, "<");
        char *outfile = strstr(input, ">");

        char *args[MAX_ARGS];
        int argc = 0;

        char *cmd = strtok(input, "<>");

        if (!cmd)
            continue;

        char *arg = strtok(cmd, " \t");

        while (arg && argc < MAX_ARGS - 1) {
            args[argc++] = arg;
            arg = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        pid_t pid = fork();

        if (pid == 0) {

            if (infile) {
                char *file = strtok(infile + 1, " \t");

                int fd = open(file, O_RDONLY);

                if (fd < 0) {
                    perror("input file");
                    exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (outfile) {
                char *file = strtok(outfile + 1, " \t");

                int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd < 0) {
                    perror("output file");
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }

        waitpid(pid, NULL, 0);
    }

    return 0;
}