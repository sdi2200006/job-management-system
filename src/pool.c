#include "../include/shared.h"

// function to find an empty pool and return a pointer to it, or NULL if no empty pool is found
Pool *find_empty_pool(void)
{
    Pool *cur = pool_head;
    while (cur != NULL)
    {
        // if the pool has less than N jobs (active + paused + finished), return it
        if ((cur->actives + cur->paused + cur->finished) < N)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

// function to create a new pool and return a pointer to it, or NULL if error
Pool *create_pool(void)
{
    // create two pipes for communication between the coordinator and the pool
    int fd1[2];
    int fd2[2];

    // create the pipes, if error return NULL
    if (pipe(fd1) == -1)
    {
        perror("pipe");
        return NULL;
    }
    // create the second pipe, if error return NULL and close the first pipe
    if (pipe(fd2) == -1)
    {
        perror("pipe fd2");
        close(fd1[0]);
        close(fd1[1]);
        return NULL;
    }

    // fork a new process for the pool, if error return NULL and close the pipes
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        return NULL;
    }

    // child process becomes the pool, parent process continues as the coordinator
    if (pid == 0)
    {
        // pool reads from fd1[0] from coord and writes to fd2[1] to coord
        close(fd1[1]);
        close(fd2[0]);
        char buffer[BUFFER_SIZE];
        while (1)
        {
            // read a command from the coordinator, if error continue
            ssize_t n = read(fd1[0], buffer, BUFFER_SIZE - 1);
            if (n <= 0)
                continue;

            buffer[n] = '\0';
            // execute the command in the buffer and write the PID of the job to fd2[1]
            pid_t j = fork();
            // child process becomes the job, parent process continues as the pool
            if (j == 0)
            {
                execlp("sh", "sh", "-c", buffer, (char *)NULL);
                exit(1);
            }
            else if (j > 0)
            {
                char reply[BUFFER_SIZE];
                snprintf(reply, sizeof(reply), "%d", j);
                write(fd2[1], reply, strlen(reply));
            }
        }
    }
    else
    {
        // coordinator writes to fd1[1] to pool and reads from fd2[0] from pool, close the unused ends of the pipes
        close(fd1[0]);
        close(fd2[1]);

        // create a new pool struct and add it to the linked list of pools
        Pool *new_pool = malloc(sizeof(Pool));
        if (new_pool == NULL)
        {
            perror("malloc");
            close(fd1[1]);
            close(fd2[0]);
            return NULL;
        }

        // initialize the pool data
        new_pool->id = current_pool_id++;
        new_pool->pid = pid;
        new_pool->actives = 0;
        new_pool->finished = 0;
        new_pool->paused = 0;
        new_pool->fd_write = fd1[1];
        new_pool->fd_read = fd2[0];
        new_pool->next = NULL;

        // add the new pool to the linked list of pools
        if (pool_head == NULL)
        {
            pool_head = new_pool;
        }
        else
        {
            Pool *cur = pool_head;
            while (cur->next != NULL)
            {
                cur = cur->next;
            }
            cur->next = new_pool;
        }
        // return a pointer to the new pool
        return new_pool;
    }
}

// function to find a pool by its ID and return a pointer to it, or NULL if not found
Pool *find_pool(int id)
{
    Pool *cur = pool_head;
    while (cur != NULL)
    {
        if (cur->id == id)
        {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

// function to remove a pool by its ID, if not found do nothing
void remove_pool(int id)
{
    Pool *cur = pool_head;
    Pool *prev = NULL;

    while (cur != NULL)
    {
        if (cur->id == id)
        {
            // if its the first pool, update the head of the list, otherwise update the previous pool's next pointer
            if (prev == NULL)
                pool_head = cur->next;
            // if its not the first pool, update the previous pool's next pointer to skip the current pool
            else
                prev->next = cur->next;
            // free the current pool
            free(cur);
            return;
        }
        // update the previous and current pointers to continue
        prev = cur;
        cur = cur->next;
    }
}

// function to destroy all pools and free them
void destroy_pools(void)
{
    Pool *cur = pool_head;
    while (cur != NULL)
    {
        Pool *next = cur->next;
        free(cur);
        cur = next;
    }
    pool_head = NULL;
}