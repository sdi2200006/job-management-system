#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "ipc.h"

#define MAX_JOBS 512
#define ACTIVE 0
#define FINISHED 1
#define SUSPENDED 2

typedef struct
{
    pid_t pid;                 // process ID of the job
    int id;                    // job ID
    int status;                // ACTIVE = 0, FINISHED = 1, SUSPENDED = 2
    char command[BUFFER_SIZE]; // command of the job
    time_t submit;             // when was submitted
    time_t start;              // when started
    time_t end;                // when finished
    int pool_id;               // pool's ID that runs this job
} Job;

// function to set the maximum number of jobs for each pool
void set_N(int n);

// function to update the status of all jobs, checking if any active job has finished
void update_status(void);

// function to handle the submit command, which submits a new job to an empty pool
int handle_submit(int fd, const char *buffer);

// function to handle the status command, which returns the status of a job with a given ID
int handle_status(int fd, const char *buffer);

// function to handle the status-all command, which returns the status of all jobs
int handle_status_all(int fd, const char *buffer);

// function to handle the show-active command, which returns the status of all active jobs
int handle_show_active(int fd);

// function to handle the show-pools command, which returns the status of all pools
int handle_show_pools(int fd);

// function to handle the show-finished command, which returns the all finished jobs
int handle_show_finished(int fd);

// function to handle the suspend command, which suspends a job with a given ID
int handle_suspend(int fd, const char *buffer);

// function to handle the resume command, which resumes a job with a given ID
int handle_resume(int fd, const char *buffer);

// function to handle the shutdown command, which terminates all pools and jobs and exits the coordinator
int handle_shutdown(int fd);

#endif