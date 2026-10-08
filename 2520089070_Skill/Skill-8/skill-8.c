#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

void expand_variables(char *input)
{
    char result[MAX_INPUT];
    int i = 0;
    int j = 0;

    while (input[i] != '\0' && j < MAX_INPUT - 1)
    {
        if (input[i] == '$')
        {
            i++;

            char name[128];
            int k = 0;

            while ((input[i] >= 'A' && input[i] <= 'Z') ||
                   (input[i] >= 'a' && input[i] <= 'z') ||
                   (input[i] >= '0' && input[i] <= '9') ||
                   input[i] == '_')
            {
                if (k < (int)sizeof(name) - 1)
                    name[k++] = input[i];

                i++;
            }

            name[k] = '\0';

            if (k > 0)
            {
                char *value = getenv(name);

                if (value != NULL)
                {
                    while (*value != '\0' &&
                           j < MAX_INPUT - 1)
                    {
                        result[j++] = *value++;
                    }
                }

                continue;
            }

            result[j++] = '$';
        }
        else
        {
            result[j++] = input[i++];
        }
    }

    result[j] = '\0';
    strcpy(input, result);
}

int main(void)
{
    char input[MAX_INPUT];

    printf("========== SKILL 08 ==========\n");
    printf("Environment variable expansion + built-ins.\n");
    printf("Try: echo $HOME\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("skill08> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strlen(input) == 0)
            continue;

        expand_variables(input);

        char *args[MAX_ARGS];
        int count = 0;

        char *token = strtok(input, " \t");

        while (token != NULL && count < MAX_ARGS - 1)
        {
            args[count++] = token;
            token = strtok(NULL, " \t");
        }

        args[count] = NULL;

        if (count == 0)
            continue;

        if (strcmp(args[0], "echo") == 0)
        {
            for (int i = 1; args[i] != NULL; i++)
            {
                printf("%s", args[i]);

                if (args[i + 1] != NULL)
                    printf(" ");
            }

            printf("\n");
        }
        else if (strcmp(args[0], "env") == 0)
        {
            extern char **environ;

            for (char **env = environ;
                 *env != NULL;
                 env++)
            {
                printf("%s\n", *env);
            }
        }
        else
        {
            printf("Unknown built-in: %s\n", args[0]);
        }
    }

    return 0;
}