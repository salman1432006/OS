#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

int main(void)
{
    char input[MAX_INPUT];

    printf("========== SKILL 07 ==========\n");
    printf("Process synchronization and command resolution.\n");
    printf("Uses fork(), execvp() and waitpid().\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("skill07> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strlen(input) == 0)
            continue;

        char *args[MAX_ARGS];
        int count = 0;

        char *token = strtok(input, " \t");

        while (token != NULL && count < MAX_ARGS - 1)
        {
            args[count++] = token;
            token = strtok(NULL, " \t");
        }

        args[count] = NULL;

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            execvp(args[0], args);

            fprintf(stderr,
                    "Command not found: %s\n",
                    args[0]);

            exit(127);
        }

        int status;

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            continue;
        }

        if (WIFEXITED(status))
        {
            printf("Child PID %d exited with status %d\n",
                   pid,
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child terminated by signal %d\n",
                   WTERMSIG(status));
        }
    }

    return 0;
}