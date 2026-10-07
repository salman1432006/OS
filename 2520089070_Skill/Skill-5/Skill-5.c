#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 256
#define MAX_TOKENS 64

typedef struct
{
    char value[MAX_INPUT];
} Token;

void add_token(Token tokens[], int *count, const char *word)
{
    if (*count >= MAX_TOKENS)
        return;

    strncpy(tokens[*count].value, word, MAX_INPUT - 1);
    tokens[*count].value[MAX_INPUT - 1] = '\0';

    (*count)++;
}

int tokenize_quotes(const char *input, Token tokens[])
{
    int i = 0;
    int count = 0;

    while (input[i] != '\0')
    {
        while (isspace((unsigned char)input[i]))
            i++;

        if (input[i] == '\0')
            break;

        char word[MAX_INPUT];
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]))
        {
            if (input[i] == '\'' ||
                input[i] == '"')
            {
                char quote = input[i++];

                while (input[i] != '\0' &&
                       input[i] != quote)
                {
                    if (j < MAX_INPUT - 1)
                        word[j++] = input[i];

                    i++;
                }

                if (input[i] == quote)
                    i++;
            }
            else
            {
                if (j < MAX_INPUT - 1)
                    word[j++] = input[i];

                i++;
            }
        }

        word[j] = '\0';

        if (j > 0)
            add_token(tokens, &count, word);
    }

    return count;
}

int main(void)
{
    char input[MAX_INPUT];

    printf("========================================\n");
    printf("       SKILL 05 - QUOTE PARSER\n");
    printf("========================================\n");

    printf("Supports:\n");
    printf("- Single quotes\n");
    printf("- Double quotes\n");
    printf("- Spaces inside quoted strings\n");
    printf("- Multiple tokens\n");

    printf("\nType 'exit' to quit.\n\n");

    while (1)
    {
        printf("skill05> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting Skill 05...\n");
            break;
        }

        if (strlen(input) == 0)
            continue;

        Token tokens[MAX_TOKENS];

        int count = tokenize_quotes(input, tokens);

        printf("\n========== TOKENS ==========\n");

        for (int i = 0; i < count; i++)
        {
            printf("Token %d: [%s]\n",
                   i + 1,
                   tokens[i].value);
        }

        printf("============================\n");
    }

    return 0;
}