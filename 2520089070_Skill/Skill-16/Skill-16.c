```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_JOBS 100
#define MAX_CMD 256
#define MAX_ARGS 64

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

#define RUNNING 1
#define STOPPED 2

int find_job(pid_t pid) {
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
                WNOHANG | WUNTRACED
            );
        } while (result == -1 && errno == EINTR);

        if (result == 0) {
            i++;
        } else if (result == jobs[i].pid) {
            if (WIFSTOPPED(status)) {
                jobs[i].status = STOPPED;
                printf("\n[%d] Stopped %s\n",
                       jobs[i].job_id, jobs[i].command);
                i++;
            } else {
                printf("\n[%d] Completed %s\n",
                       jobs[i].job_id, jobs[i].command);
                remove_job(i);
            }
        } else if (result == -1 && errno == ECHILD) {
            remove_job(i);
        } else {
            i++;
        }
    }
}

void show_jobs(void) {
    monitor_jobs();

    if (job_count == 0) {
        printf("No active jobs.\n");
        return;
    }

    printf("JOB_ID\tPID\tPGID\tSTATUS\tCOMMAND\n");

    for (int i = 0; i < job_count; i++) {
        printf("[%d]\t%d\t%d\t%s\t%s\n",
               jobs[i].job_id,
               (int)jobs[i].pid,
               (int)jobs[i].pgid,
               jobs[i].status == RUNNING ? "Running" : "Stopped",
               jobs[i].command);
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
        show_jobs();
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (background)
            setpgid(0, 0);

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

        do {
            result = waitpid(pid, &status, 0);
        } while (result == -1 && errno == EINTR);

        if (result == -1)
            perror("waitpid");
    }
}

int main(void) {
    char input[MAX_CMD];

    printf("Linux Shell with Background Job Management\n");
    printf("Type 'jobs' to list jobs and 'exit' to quit.\n");

    while (1) {
        monitor_jobs();

        printf("myshell$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        execute_command(input);
    }

    /* Reap any remaining background children */
    while (job_count > 0) {
        int status;
        pid_t pid = waitpid(-1, &status, 0);

        if (pid > 0) {
            int index = find_job(pid);
            if (index >= 0)
                remove_job(index);
        } else if (pid == -1 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }

    return 0;
}
```


