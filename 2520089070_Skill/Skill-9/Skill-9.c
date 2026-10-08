#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 256
#define MAX_PATH_LEN 4096

int main(void)
{
    char input[MAX_INPUT];

    printf("========== SKILL 09 ==========\n");
    printf("cd built-in with directory validation.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        char cwd[MAX_PATH_LEN];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("skill09:%s$ ", cwd);
        else
            printf("skill09$ ");

        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strncmp(input, "cd", 2) == 0 &&
            (input[2] == '\0' || input[2] == ' '))
        {
            char *path = input + 2;

            while (*path == ' ')
                path++;

            if (*path == '\0')
                path = getenv("HOME");

            if (path == NULL)
            {
                fprintf(stderr, "HOME is not set.\n");
                continue;
            }

            if (chdir(path) != 0)
                perror("cd");
        }
        else
        {
            printf("Only cd is implemented in Skill 09.\n");
        }
    }

    return 0;
}