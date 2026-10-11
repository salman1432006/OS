#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define CHILDREN 10

int main() {
    printf("Skill 23 - Process Stress Test\n");
    printf("Creating %d child processes...\n", CHILDREN);

    pid_t pids[CHILDREN];

    for (int i = 0; i < CHILDREN; i++) {

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            continue;
        }

        if (pids[i] == 0) {
            printf("Child %d: PID=%d\n",
                   i + 1,
                   getpid());

            usleep(200000);

            exit(0);
        }
    }

    int completed = 0;

    for (int i = 0; i < CHILDREN; i++) {
        int status;

        if (waitpid(pids[i], &status, 0) > 0)
            completed++;
    }

    printf("Completed children: %d/%d\n",
           completed,
           CHILDREN);

    printf("Stress test completed successfully.\n");

    return 0;
}