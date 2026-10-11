#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 256

int main() {
    char *buffer = malloc(MAX_INPUT);

    if (!buffer) {
        perror("malloc");
        return 1;
    }

    printf("Skill 22 - Memory Testing\n");
    printf("Enter text: ");

    if (fgets(buffer, MAX_INPUT, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("You entered: %s\n", buffer);
    }

    free(buffer);

    printf("Memory released successfully.\n");

    return 0;
}