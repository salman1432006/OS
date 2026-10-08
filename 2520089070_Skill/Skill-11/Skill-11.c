#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 20
#define MAX_INPUT 256
#define MAX_CMDS 20

char history[MAX_HISTORY][MAX_INPUT];
int history_count = 0;

void add_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count++], cmd);
    } else {
        for (int i = 1; i < MAX_HISTORY; i++)
            strcpy(history[i-1], history[i]);
        strcpy(history[MAX_HISTORY-1], cmd);
    }
}

void show_history() {
    for (int i = 0; i < history_count; i++)
        printf("%d  %s\n", i + 1, history[i]);
}

void show_pipeline(char *input) {
    char *commands[MAX_CMDS];
    int count = 0;

    char *token = strtok(input, "|");

    while (token && count < MAX_CMDS) {
        commands[count++] = token;
        token = strtok(NULL, "|");
    }

    printf("Pipeline has %d command(s):\n", count);

    for (int i = 0; i < count; i++) {
        while (*commands[i] == ' ')
            commands[i]++;

        printf("  Stage %d: %s\n", i + 1, commands[i]);
    }
}

int main() {
    char input[MAX_INPUT];

    printf("Skill 11 - History + Pipeline Structure\n");
    printf("Commands: history, exit\n\n");

    while (1) {
        printf("skill11> ");

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            continue;

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "history") == 0) {
            show_history();
            continue;
        }

        add_history(input);

        char copy[MAX_INPUT];
        strcpy(copy, input);

        if (strchr(copy, '|'))
            show_pipeline(copy);
        else
            printf("Command stored: %s\n", input);
    }

    return 0;
}