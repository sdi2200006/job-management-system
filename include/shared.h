#ifndef SHARED_H
#define SHARED_H

#include "handlers.h"
#include "pool.h"

// maximum number of jobs for each pool
extern int N;


// array of all jobs
extern Job jobs[MAX_JOBS];
// number of jobs in the array
extern int jb_count;
// current job ID for the next  job
extern int current_job_id;


// head of the linked list of pools 
extern Pool *pool_head;
// current pool ID for the next pool
extern int current_pool_id;

#endif