#ifndef POOL_H
#define POOL_H
#include <sys/types.h>

typedef struct Pool
{
    int id;       // pool ID
    pid_t pid;    // process ID of the pool
    int paused;   // suspended jobs
    int actives;  // currently running jobs
    int finished; // finished jobs
    int fd_write; // file descriptor for writing to the pool
    int fd_read;  // file descriptor for reading from the pool
    struct Pool *next;

} Pool;

//create a new pool and return a pointer to it
Pool *create_pool(void);

//find a pool with available capacity (less than N active + paused + finished jobs)
Pool *find_empty_pool(void);

//find a pool by its ID and return a pointer to it
Pool *find_pool(int id);

//remove a pool by its ID
void remove_pool(int id);

//destroy all pools and free them
void destroy_pools(void);

#endif