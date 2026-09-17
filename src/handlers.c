#include "../include/shared.h"

int N = 0;

Job jobs[MAX_JOBS];
int jb_count = 0;
int current_job_id = 1;

Pool *pool_head = NULL;
int current_pool_id = 1;

// function to set the maximum number of jobs for each pool
void set_N(int n)
{
    N = n;
}

// function to find a job by its ID and return its index in the jobs array, or -1 if not found
int find_job(int id)
{
    for (int i = 0; i < jb_count; i++)
    {
        if (jobs[i].id == id)
            return i;
    }
    return -1;
}

// function to update the status of all jobs and pools by checking if any active job has finished
void update_status(void)
{
    for (int i = 0; i < jb_count; i++)
    {
        // if the job is not finished, check if it has finished by sending a signal 0 to its PID
        if (jobs[i].status != FINISHED)
        {
            // if kill returns -1, it means the process is finished
            if (kill(jobs[i].pid, 0) == -1)
            {
                // find the pool that runs this job and update its active and finished counts accordingly
                Pool *p = find_pool(jobs[i].pool_id);
                if (p != NULL)
                {
                    // if the job was active, decrease the active count
                    if (jobs[i].status == ACTIVE)
                    {
                        (p->actives)--;
                    }
                    //  if it was suspended, decrease the paused count
                    else if (jobs[i].status == SUSPENDED)
                    {
                        (p->paused)--;
                    }
                    // set the job status to finished and end time to current time
                    jobs[i].status = FINISHED;
                    jobs[i].end = time(NULL);
                    (p->finished)++;
                }
            }
        }
    }
}

int handle_submit(int fd, const char *buffer)
{
    update_status();

    const char *job = buffer + 7;

    // if  MAX_JOBS, return an error message to the client and do not submit the job
    if (jb_count >= MAX_JOBS)
    {
        write_to_pipe(fd, "too many jobs\n", strlen("too many jobs\n"));
        write_to_pipe(fd, "OK\n", strlen("OK\n"));
        return 0;
    }

    // find an empty pool or create a new one
    Pool *p = find_empty_pool();
    // if there is no empty pool
    if (p == NULL)
    {
        // create a new pool and add it to the pool list
        p = create_pool();
        if (p == NULL)
        {
            perror("create_pool");
            return 0;
        }
    }
    if (write(p->fd_write, job, strlen(job)) == -1)
    {
        perror("write to pool");
        return 0;
    }

    char pool_reply[BUFFER_SIZE];
    int pid;

    // read the reply from the pool, which should contain the PID of the submitted job
    ssize_t n = read(p->fd_read, pool_reply, BUFFER_SIZE - 1);
    if (n <= 0)
    {
        perror("read from pool");
        return 0;
    }

    // take pid of job
    pid = atoi(pool_reply);

    // add the job to the jobs array with its data
    jobs[jb_count].pool_id = p->id;
    jobs[jb_count].id = current_job_id;
    jobs[jb_count].status = ACTIVE;
    strcpy(jobs[jb_count].command, job);
    jobs[jb_count].submit = time(NULL);
    jobs[jb_count].start = time(NULL);
    jobs[jb_count].end = 0;
    jobs[jb_count].pid = pid;

    // increase the active count of the pool by 1
    (p->actives)++;

    // send a reply to the client with the job ID and PID and an OK message
    char reply[BUFFER_SIZE];
    sprintf(reply, "JobID: %d, PID: %d\n", current_job_id, pid);
    write_to_pipe(fd, reply, strlen(reply));
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    // increase the job count and current job ID for the next job
    jb_count++;
    current_job_id++;
    return 0;
}

int handle_status(int fd, const char *buffer)
{
    update_status();

    char *num = (char *)buffer + 7;
    if (*num == '\0' || !isdigit(*num))
    {
        write_to_pipe(fd, "invalid status\n", strlen("invalid status\n"));
        write_to_pipe(fd, "OK\n", strlen("OK\n"));
        return 0;
    }

    // convert the job ID from string to integer
    int id = atoi(num);

    char reply[BUFFER_SIZE];
    // find the job by its ID
    int pos = find_job(id);
    // if the job is not found, return an error message to the client
    if (pos == -1)
    {
        sprintf(reply, "JobID %d not found\n", id);
    }
    // if the job is active, return its status and how long it has been running
    else if (jobs[pos].status == ACTIVE)
    {
        sprintf(reply, "JobID %d Status: Active (running for %ld seconds)\n", id, time(NULL) - jobs[pos].start);
    }
    // if the job is finished, return its status
    else if (jobs[pos].status == FINISHED)
    {
        sprintf(reply, "JobID %d Status: Finished\n", id);
    }
    // if the job is suspended, return its status
    else
    {
        sprintf(reply, "JobID %d Status: Suspended\n", id);
    }
    // send the reply to the client and an OK message
    write_to_pipe(fd, reply, strlen(reply));
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_status_all(int fd, const char *buffer)
{
    update_status();

    // check if there is n for seconds
    int n;
    // if there is n, set has_n to true
    bool has_n = false;
    char *num = (char *)buffer + 10;

    // string to integer and set has_n to true
    if (*num == ' ')
    {
        n = atoi(num + 1);
        has_n = true;
    }

    // if there are no jobs, return a message to the client and an OK message
    if (jb_count == 0)
    {
        write_to_pipe(fd, "No jobs\n", strlen("No jobs\n"));
        write_to_pipe(fd, "OK\n", strlen("OK\n"));
        return 0;
    }

    char reply[BUFFER_SIZE];
    for (int i = 0; i < jb_count; i++)
    {
        // if there is n, only return the jobs that have been running for more than n seconds
        if (has_n && (time(NULL) - jobs[i].submit) > n)
            continue;

        // if the job is active, return its status and how long it has been running
        if (jobs[i].status == ACTIVE)
        {
            sprintf(reply, "JobID %d Status: Active (running for %ld seconds)\n", jobs[i].id, time(NULL) - jobs[i].start);
        }
        // if the job is finished, return its status
        else if (jobs[i].status == FINISHED)
        {
            sprintf(reply, "JobID %d Status: Finished\n", jobs[i].id);
        }
        // if the job is suspended, return its status
        else if (jobs[i].status == SUSPENDED)
        {
            sprintf(reply, "JobID %d Status: Suspended\n", jobs[i].id);
        }
        write_to_pipe(fd, reply, strlen(reply));
    }
    // send an OK message for ending
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_show_active(int fd)
{
    update_status();

    bool found = false;
    char reply[BUFFER_SIZE];
    write_to_pipe(fd, "Active jobs:\n", strlen("Active jobs:\n"));
    for (int i = 0; i < jb_count; i++)
    {
        // if the job is active, return its ID to the client
        if (jobs[i].status == ACTIVE)
        {
            sprintf(reply, "JobID %d\n", jobs[i].id);
            write_to_pipe(fd, reply, strlen(reply));
            // set found to true (1+ active job)
            found = true;
        }
    }

    // if there are no active jobs, return a message to the client
    if (found == false)
        write_to_pipe(fd, "No active jobs\n", strlen("No active jobs\n"));
    // send an OK message for ending
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_show_pools(int fd)
{
    update_status();
    // if there are no pools, return a message to the client and an OK message
    if (pool_head == NULL)
    {
        write_to_pipe(fd, "No pools\n", strlen("No pools\n"));
        write_to_pipe(fd, "OK\n", strlen("OK\n"));
        return 0;
    }
    char reply[BUFFER_SIZE];
    write_to_pipe(fd, "Pool & NumOfJobs:\n", strlen("Pool & NumOfJobs:\n"));
    Pool *cur = pool_head;
    while (1)
    {
        // return the pool ID and the number of active + suspended
        sprintf(reply, "%d %d\n", cur->pid, cur->actives + cur->paused);
        write_to_pipe(fd, reply, strlen(reply));
        // if there is no next pool, break
        if ((cur = cur->next) == NULL)
            break;
    }
    // send an OK message for ending
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_show_finished(int fd)
{
    update_status();

    bool found = false;
    char reply[BUFFER_SIZE];
    write_to_pipe(fd, "Finished jobs:\n", strlen("Finished jobs:\n"));

    for (int i = 0; i < jb_count; i++)
    {
        // if the job is finished, return its ID
        if (jobs[i].status == FINISHED)
        {
            sprintf(reply, "JobID %d\n", jobs[i].id);
            write_to_pipe(fd, reply, strlen(reply));
            // set found to true (1+ finished job)
            found = true;
        }
    }

    // if there are no finished jobs, return a message to the client
    if (found == false)
        write_to_pipe(fd, "No finished jobs\n", strlen("No finished jobs\n"));
    // send an OK message for ending
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_suspend(int fd, const char *buffer)
{
    update_status();

    char reply[BUFFER_SIZE];
    char *num = (char *)buffer + 8;
    // if the job ID is not in or is  a digit, return an error message to the client and an OK message
    if (*num == '\0' || !isdigit(*num))
    {
        write_to_pipe(fd, "invalid suspend\n", strlen("invalid suspend\n"));
        write_to_pipe(fd, "OK\n", strlen("OK\n"));
        return 0;
    }

    // job ID from string to integer
    int id = atoi(num);
    // find the job by its ID
    int pos = find_job(id);
    // if the job is not found, return an error message to the client
    if (pos == -1)
    {
        sprintf(reply, "JobID %d not found\n", id);
    }
    // if the job is already finished, return its status
    else if (jobs[pos].status == FINISHED)
    {
        sprintf(reply, "JobID %d is finished\n", id);
    }
    // if the job is already suspended, return its status
    else if (jobs[pos].status == SUSPENDED)
    {
        sprintf(reply, "JobID %d is suspended\n", id);
    }
    // if the job is active
    else
    {
        // send a SIGSTOP signal to the job process to suspend
        if (kill(jobs[pos].pid, SIGSTOP) == -1)
        {
            perror("kill with SIGSTOP");
            sprintf(reply, "failed to suspend JobID %d\n", id);
        }
        // if the signal is sent successfully, update the job status to suspended and update the pool counts accordingly
        else
        {
            jobs[pos].status = SUSPENDED;
            // find the pool that runs this job and update its active and paused counts accordingly
            Pool *p = find_pool(jobs[pos].pool_id);
            if (p != NULL)
            {
                (p->actives)--;
                (p->paused)++;
            }
            sprintf(reply, "Sent suspend signal to JobID %d\n", id);
        }
    }

    // send the reply to the client and an OK message
    write_to_pipe(fd, reply, strlen(reply));
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_resume(int fd, const char *buffer)
{
    update_status();

    char reply[BUFFER_SIZE];
    char *num = (char *)buffer + 7;
    // if the job ID is not in or is  a digit, return an error message to the client and an OK message
    if (*num == '\0' || !isdigit(*num))
    {
        write_to_pipe(fd, "invalid resume\n", strlen("invalid resume\n"));
        write_to_pipe(fd, "OK\n", strlen("OK\n"));
        return 0;
    }

    // job ID from string to integer
    int id = atoi(num);
    // find the job by its ID
    int pos = find_job(id);
    // if the job is not found, return an error message to the client
    if (pos == -1)
    {
        sprintf(reply, "JobID %d not found\n", id);
    }
    // if the job is already FINISHED, return its status
    else if (jobs[pos].status == FINISHED)
    {
        sprintf(reply, "JobID %d is finished\n", id);
    }
    // if the job is already active, return its status
    else if (jobs[pos].status == ACTIVE)
    {
        sprintf(reply, "JobID %d is active\n", id);
    }
    // if the job is suspended
    else
    {
        // send a SIGCONT signal to the job process to resume
        if (kill(jobs[pos].pid, SIGCONT) == -1)
        {
            perror("kill with SIGCONT");
            sprintf(reply, "failed to resume JobID %d\n", id);
        }
        // if the signal is sent successfully, update the job status to active and update the pool counts accordingly
        else
        {
            jobs[pos].status = ACTIVE;
            // find the pool that runs this job and update its active and paused counts accordingly
            Pool *p = find_pool(jobs[pos].pool_id);
            if (p != NULL)
            {
                (p->paused)--;
                (p->actives)++;
            }
            sprintf(reply, "Sent resume signal to JobID %d\n", id);
        }
    }

    // send the reply to the client and an OK message
    write_to_pipe(fd, reply, strlen(reply));
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 0;
}

int handle_shutdown(int fd)
{
    update_status();

    // counts the number of active jobs before shutdown
    int active_jobs = 0;
    //for each job
    for (int i = 0; i < jb_count; i++)
    {
        // if the job is active or suspended, 
        if (jobs[i].status == ACTIVE || jobs[i].status == SUSPENDED)
        {
            // send a SIGTERM signal to the job process to terminate
            kill(jobs[i].pid, SIGTERM);
            active_jobs++;
        }
    }

    // send a reply to the client with the number of served jobs and the number of active jobs before shutdown and an OK message
    char reply[BUFFER_SIZE];
    sprintf(reply, "Served %d jobs, %d were still in progress\n", jb_count, active_jobs);
    write_to_pipe(fd, reply, strlen(reply));
    write_to_pipe(fd, "OK\n", strlen("OK\n"));
    return 1;
}