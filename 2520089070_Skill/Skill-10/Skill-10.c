#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 256
#define MAX_PATH_LEN 4096

int main(void)
{
    char input[MAX_INPUT];

    printf("========== SKILL 10 ==========\n");
    printf("pwd + export environment variables.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("skill10$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        /* pwd */
        if (strcmp(input, "pwd") == 0)
        {
            char cwd[MAX_PATH_LEN];

            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s\n", cwd);
            else
                perror("pwd");
        }

        /* export NAME=value */
        else if (strncmp(input, "export ", 7) == 0)
        {
            char *assignment = input + 7;
            char *equals = strchr(assignment, '=');

            if (equals == NULL || equals == assignment)
            {
                printf("Invalid export syntax.\n");
                continue;
            }

            *equals = '\0';

            char *name = assignment;
            char *value = equals + 1;

            if (setenv(name, value, 1) != 0)
            {
                perror("export");
            }
            else
            {
                printf("Exported %s=%s\n",
                       name,
                       value);
            }
        }

        else
        {
            printf("Unknown command: %s\n", input);
        }
    }

    return 0;
}