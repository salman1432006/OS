#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_JOBS 100
#define MAX_CMD 256
#define MAX_ARGS 64

#define RUNNING 1
#define STOPPED 2

typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;
    char command[MAX_CMD];
    int status;
} Job;

static Job jobs[MAX_JOBS];
static int job_count = 0;
static int next_job_id = 1;

int find_job(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id)
            return i;
    }
    return -1;
}

int find_pid(pid_t pid) {
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

void add_job(pid_t pid, pid_t pgid,
             const char *command, int status) {
    if (job_count >= MAX_JOBS) {
        printf("Job table is full.\n");
        return;
    }

    jobs[job_count].job_id = next_job_id++;
    jobs[job_count].pid = pid;
    jobs[job_count].pgid = pgid;
    jobs[job_count].status = status;

    snprintf(jobs[job_count].command,
             MAX_CMD, "%s", command);

    printf("[%d] %d %s\n",
           jobs[job_count].job_id,
           (int)pid,
           status == STOPPED ? "Stopped" : "Running");

    job_count++;
}

void update_jobs(void) {
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
                printf("\n[%d] Stopped: %s\n",
                       jobs[i].job_id,
                       jobs[i].command);
                i++;
            } else if (WIFCONTINUED(status)) {
                jobs[i].status = RUNNING;
                printf("\n[%d] Running: %s\n",
                       jobs[i].job_id,
                       jobs[i].command);
                i++;
            } else if (WIFEXITED(status) ||
                       WIFSIGNALED(status)) {
                printf("\n[%d] Completed: %s\n",
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
    update_jobs();

    if (job_count == 0) {
        printf("No active jobs.\n");
        return;
    }

    printf("\nJOB ID\tPID\tPGID\tSTATUS\tCOMMAND\n");
    printf("-----------------------------------------------\n");

    for (int i = 0; i < job_count; i++) {
        printf("[%d]\t%d\t%d\t%s\t%s\n",
               jobs[i].job_id,
               (int)jobs[i].pid,
               (int)jobs[i].pgid,
               jobs[i].status == RUNNING
                   ? "Running" : "Stopped",
               jobs[i].command);
    }
}

void resume_job(int job_id) {
    int index = find_job(job_id);

    if (index == -1) {
        printf("Job %d not found.\n", job_id);
        return;
    }

    if (jobs[index].status != STOPPED) {
        printf("Job %d is not stopped.\n", job_id);
        return;
    }

    if (kill(-jobs[index].pgid, SIGCONT) == -1) {
        perror("SIGCONT");
        return;
    }

    jobs[index].status = RUNNING;

    printf("[%d] Resumed: %s\n",
           jobs[index].job_id,
           jobs[index].command);
}

int parse_command(char *input, char *args[]) {
    int argc = 0;
    char *token = strtok(input, " \t\r\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\r\n");
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
            input[len - 1] == '\t' ||
            input[len - 1] == '\r')) {
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

    if (strcmp(args[0], "bg") == 0) {
        if (args[1] == NULL) {
            printf("Usage: bg <job_id>\n");
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

        resume_job((int)id);
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (setpgid(0, 0) == -1) {
            perror("setpgid");
            _exit(1);
        }

        execvp(args[0], args);

        perror("execvp");
        _exit(127);
    }

    if (setpgid(pid, pid) == -1 &&
        errno != EACCES && errno != ESRCH) {
        perror("setpgid");
    }

    if (background) {
        add_job(pid, pid, original, RUNNING);
    } else {
        int status;
        pid_t result;

        do {
            result = waitpid(pid, &status, WUNTRACED);
        } while (result == -1 && errno == EINTR);

        if (result == pid && WIFSTOPPED(status)) {
            add_job(pid, pid, original, STOPPED);
        } else if (result == -1) {
            perror("waitpid");
        }
    }
}

int main(void) {
    char input[MAX_CMD];

    sigset_t mask;

    if (sigprocmask(SIG_SETMASK, NULL, &mask) == 0)
        printf("Signal mask inspected successfully.\n");

    printf("Linux Shell - Signal and Job Management\n");
    printf("Commands: jobs, bg <job_id>, exit\n");
    printf("Use Ctrl+Z to stop a foreground process.\n");

    while (1) {
        update_jobs();

        printf("myshell$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        execute_command(input);
    }

    while (job_count > 0) {
        int status;
        pid_t pid = waitpid(-1, &status, 0);

        if (pid > 0) {
            int index = find_pid(pid);

            if (index >= 0)
                remove_job(index);
        } else if (pid == -1 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }

    printf("Shell terminated.\n");
    return 0;
}