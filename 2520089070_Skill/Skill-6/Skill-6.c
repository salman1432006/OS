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

    printf("========== SKILL 06 ==========\n");
    printf("Escape sequences + child process execution.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("skill06> ");
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

            perror("execvp");
            exit(EXIT_FAILURE);
        }

        int status;

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
        }
        else if (WIFEXITED(status))
        {
            printf("Child exited with status %d\n",
                   WEXITSTATUS(status));
        }
    }

    return 0;
}