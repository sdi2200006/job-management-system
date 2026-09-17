# job-management-system
A C-based job management system developed as part of a Systems Programming assignment.
The project implements a coordinator-console architecture for submitting and managing jobs using processes, pools, named pipes, UNIX signals and inter-process communication.

## Features
The system supports the following commands:
- submit <command>
- status <job_id>
- status-all [time]
- show-active
- show-finished
- show-pools
- suspend <job_id>
- resume <job_id>
- shutdown

The coordinator manages submitted jobs through pools and keeps track of their execution state.

## Architecture
The system consists of two main executables:

### JMS Coordinator
- Creating and opening the named pipes used for communication
- Receiving commands from the console
- Managing pools and jobs
- Executing submitted commands
- Tracking job status
- Suspending and resuming jobs
- Returning results to the console
- Handling system shutdown

### JMS Console
- Connecting to the coordinator through named pipes
- Reading commands from standard input or an operations file
- Sending commands to the coordinator
- Receiving and displaying responses

Communication between the console and coordinator is performed using named pipes.

## Project Structure
```text
job-management-system/
├── include/
│   ├── handlers.h
│   ├── ipc.h
│   ├── pool.h
│   └── shared.h
├── src/
│   ├── handlers.c
│   ├── ipc.c
│   ├── jms_console.c
│   ├── jms_coord.c
│   └── pool.c
├── input.txt
├── Makefile
└── README.md
````

## Source Files

### ipc.c / ipc.h
Implements the inter-process communication utilities used by the application.
- Creating named pipes
- Opening named pipes
- Removing named pipes
- Reading from pipes
- Writing to pipes

### pool.c / pool.h
Implements the management of process pools.
- Creating pools
- Finding available pools
- Searching for pools
- Removing pools
- Managing the pool list

### handlers.c / handlers.h
Contains the implementation of the supported commands:
- submit
- status
- status-all
- show-active
- show-finished
- show-pools
- suspend
- resume
- shutdown

### shared.h
Contains shared data structures and global state used by the coordinator, including job and pool information.

## Compilation
Compile the project using:
```bash
make
```

### Start the Coordinator
Run:
```bash
./jms_coord -l ./data -n 3
```
where:
- "-l" specifies the working directory
- "-n" specifies the maximum number of jobs handled by each pool

### Start the Console
Open another terminal and run:
```bash
./jms_console -w jms_in -r jms_out [-o input.txt]
```

## Example Commands
```text
submit /bin/sleep 10
status 1
status-all
show-active
show-finished
show-pools
suspend 1
resume 1
shutdown
```

## System Programming Concepts
The project makes use of several UNIX system programming concepts:
- fork() for creating processes
- execlp() for executing submitted commands
- Named pipes (FIFOs) for communication between the console and coordinator
- Pipes for process communication
- SIGSTOP for suspending jobs
- SIGCONT for resuming jobs
- SIGTERM for terminating processes
- kill(pid, 0) for checking whether a process is still active
- Process and job status management

## What I Practiced
- Creating and managing UNIX processes in C
- Implementing inter-process communication
- Working with named pipes and regular pipes
- Handling UNIX signals
- Designing a job management system
- Managing process pools
- Tracking job execution states
- Organizing a multi-file C project using header and source files
- Building C applications using Makefiles

## Technologies
- C
- UNIX/Linux
- POSIX Processes
- Named Pipes (FIFOs)
- UNIX Signals
