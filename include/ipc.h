#ifndef IPC_H
#define IPC_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// size of the buffer for reading/writing to the named pipe
#define BUFFER_SIZE 1024

// function which creates the named pipes at this path with this name 
// ok returns 0 and error returns -1
int create_named_pipe(const char *path);

// function which removes the named pipes at this path with this name 
// ok returns 0 and error returns -1
int remove_named_pipe(const char *path);

// function which opens the named pipes at this path with this name and this flag (O_RDONLY, O_WRONLY,...)
//  ok returns the file descriptor and error returns -1
int open_named_pipe(const char *path, int flags);

// function which writes to the named pipe with this file descriptor, this buffer and this size
// ok returns the number of bytes written and error returns -1
ssize_t write_to_pipe(int fd, const void *buffer, size_t size);

// function which reads from the named pipe with this file descriptor, this buffer and this size
// ok returns the number of bytes readen and error returns -1
ssize_t read_from_pipe(int fd, void *buffer, size_t size);

#endif