
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_JOBS 100
#define MAX_CMD 256
#define MAX_ARGS 64

#define RUNNING  1
#define STOPPED  2

typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;
    char command[MAX_CMD];
    int status;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;
int next_job_id = 1;

int find_job_by_id(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id)
            return i;
    }
    return -1;
}

int find_job_by_pid(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid)
            return i;
    }
    return -1;
}

void remove_job(int index) {
    if (index < 0 || index >= job_count)
        return;

    for (int i = index; i < job_count - 1; i++)
        jobs[i] = jobs[i + 1];

    job_count--;
}

void add_job(pid_t pid, pid_t pgid, const char *command) {
    if (job_count >= MAX_JOBS) {
        printf("Job table is full.\n");
        return;
    }

    jobs[job_count].job_id = next_job_id++;
    jobs[job_count].pid = pid;
    jobs[job_count].pgid = pgid;
    jobs[job_count].status = RUNNING;

    snprintf(jobs[job_count].command,
             MAX_CMD, "%s", command);

    printf("[%d] %d\n",
           jobs[job_count].job_id, (int)pid);

    job_count++;
}

void monitor_jobs(void) {
    int i = 0;

    while (i < job_count) {
        int status;
        pid_t result;

        do {
            result = waitpid(
                jobs[i].pid,
                &status,
                WNOHANG | WUNTRACED | WCONTINUED
            );
        } while (result == -1 && errno == EINTR);

        if (result == 0) {
            i++;
        } else if (result == jobs[i].pid) {
            if (WIFSTOPPED(status)) {
                jobs[i].status = STOPPED;
                i++;
            } else if (WIFCONTINUED(status)) {
                jobs[i].status = RUNNING;
                i++;
            } else if (WIFEXITED(status) ||
                       WIFSIGNALED(status)) {
                printf("\n[%d] Done %s\n",
                       jobs[i].job_id,
                       jobs[i].command);
                remove_job(i);
            } else {
                i++;
            }
        } else if (result == -1 && errno == ECHILD) {
            remove_job(i);
        } else {
            i++;
        }
    }
}

void list_jobs(void) {
    monitor_jobs();

    if (job_count == 0) {
        printf("No active jobs.\n");
        return;
    }

    printf("\nJOB ID\tPID\tPGID\tSTATUS\t\tCOMMAND\n");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < job_count; i++) {
        const char *state =
            jobs[i].status == RUNNING ? "Running" : "Stopped";

        printf("[%d]\t%d\t%d\t%s\t\t%s\n",
               jobs[i].job_id,
               (int)jobs[i].pid,
               (int)jobs[i].pgid,
               state,
               jobs[i].command);
    }
}

void foreground_job(int job_id) {
    int index = find_job_by_id(job_id);

    if (index == -1) {
        printf("fg: job %d not found\n", job_id);
        return;
    }

    pid_t pid = jobs[index].pid;
    pid_t pgid = jobs[index].pgid;

    /*
     * This implementation requires an interactive terminal.
     * A shell running without a controlling terminal cannot
     * perform terminal handoff.
     */
    if (isatty(STDIN_FILENO)) {
        if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
            perror("tcsetpgrp");
            return;
        }
    }

    if (jobs[index].status == STOPPED) {
        if (kill(-pgid, SIGCONT) == -1) {
            perror("SIGCONT");

            if (isatty(STDIN_FILENO))
                tcsetpgrp(STDIN_FILENO, getpgrp());

            return;
        }

        jobs[index].status = RUNNING;
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, WUNTRACED);
    } while (result == -1 && errno == EINTR);

    if (isatty(STDIN_FILENO)) {
        if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
            perror("tcsetpgrp");
    }

    index = find_job_by_pid(pid);

    if (index == -1)
        return;

    if (result == pid && WIFSTOPPED(status)) {
        jobs[index].status = STOPPED;
        printf("[%d] Stopped %s\n",
               jobs[index].job_id,
               jobs[index].command);
    } else if (result == pid &&
               (WIFEXITED(status) || WIFSIGNALED(status))) {
        remove_job(index);
    } else if (result == -1) {
        perror("waitpid");
    }
}

int parse_command(char *input, char *args[]) {
    int argc = 0;
    char *token = strtok(input, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    args[argc] = NULL;
    return argc;
}

void execute_command(char *input) {
    char original[MAX_CMD];
    char *args[MAX_ARGS];

    snprintf(original, sizeof(original), "%s", input);

    size_t len = strlen(input);

    while (len > 0 &&
           (input[len - 1] == '\n' ||
            input[len - 1] == ' ' ||
            input[len - 1] == '\t')) {
        input[--len] = '\0';
    }

    if (len == 0)
        return;

    int background = 0;

    if (input[len - 1] == '&') {
        background = 1;
        input[--len] = '\0';
    }

    int argc = parse_command(input, args);

    if (argc == 0)
        return;

    if (strcmp(args[0], "exit") == 0)
        exit(0);

    if (strcmp(args[0], "jobs") == 0) {
        list_jobs();
        return;
    }

    if (strcmp(args[0], "fg") == 0) {
        if (args[1] == NULL) {
            printf("Usage: fg <job_id>\n");
            return;
        }

        char *end;
        errno = 0;
        long id = strtol(args[1], &end, 10);

        if (errno != 0 || *end != '\0' ||
            id <= 0 || id > 2147483647L) {
            printf("Invalid job ID.\n");
            return;
        }

        foreground_job((int)id);
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (background) {
            if (setpgid(0, 0) == -1) {
                perror("setpgid");
                _exit(1);
            }
        } else if (isatty(STDIN_FILENO)) {
            /*
             * Basic foreground execution. Full interactive
             * shell job control requires signal management.
             */
            setpgid(0, 0);
        }

        execvp(args[0], args);
        perror("execvp");
        _exit(127);
    }

    if (background) {
        if (setpgid(pid, pid) == -1 &&
            errno != EACCES && errno != ESRCH) {
            perror("setpgid");
        }

        add_job(pid, pid, original);
    } else {
        int status;
        pid_t result;

        if (isatty(STDIN_FILENO))
            tcsetpgrp(STDIN_FILENO, pid);

        do {
            result = waitpid(pid, &status, WUNTRACED);
        } while (result == -1 && errno == EINTR);

        if (isatty(STDIN_FILENO))
            tcsetpgrp(STDIN_FILENO, getpgrp());

        if (result == pid && WIFSTOPPED(status))
            add_job(pid, pid, original);
        else if (result == -1)
            perror("waitpid");
    }
}

int main(void) {
    char input[MAX_CMD];

    printf("Linux Shell: Job Listing and Foreground Switching\n");
    printf("Commands: jobs, fg <job_id>, exit\n");

    while (1) {
        monitor_jobs();

        printf("myshell$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        execute_command(input);
    }

    return 0;
}
```
