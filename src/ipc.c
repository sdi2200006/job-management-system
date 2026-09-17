#include "../include/ipc.h"

// creates the named pipes at this path with this name
int create_named_pipe(const char *path)
{
    // if path is NULL, print error and return -1
    if (path == NULL)
    {
        fprintf(stderr, "create_named_pipe: path is NULL\n");
        return -1;
    }
    // if already exists, remove it
    if (unlink(path) == -1)
    {
        // if error is not "No such file or directory", print error and return -1
        if (errno != ENOENT)
        {
            perror("unlink named pipe");
            return -1;
        }
    }
    // create the named pipe with read and write permissions for all
    if (mkfifo(path, 0666) == -1)
    {
        perror("mkfifo named pipe");
        return -1;
    }
    // return 0 on success
    return 0;
}

// removes the named pipes at this path with this name
int remove_named_pipe(const char *path)
{
    // if path is NULL, print error and return -1
    if (path == NULL)
    {
        fprintf(stderr, "remove_named_pipe: path is NULL\n");
        return -1;
    }
    // remove the named pipe
    if (unlink(path) == -1)
    {
        // if error is not "No such file or directory", print error and return -1
        if (errno != ENOENT)
        {
            perror("unlink named pipe");
            return -1;
        }
    }
    // return 0 on success
    return 0;
}

// opens the named pipes at this path with this name and this flag (O_RDONLY, O_WRONLY, ...)
int open_named_pipe(const char *path, int flags)
{
    // if path is NULL, print error and return -1
    if (path == NULL)
    {
        fprintf(stderr, "open_named_pipe: path is NULL\n");
        return -1;
    }
    // open the named pipe with this flags
    int fd = open(path, flags);
    // if error, print error and return -1
    if (fd == -1)
    {
        perror("open named pipe");
        return -1;
    }
    // return the file descriptor on success
    return fd;
}

// writes to the named pipe with this file descriptor, this buffer and this size
ssize_t write_to_pipe(int fd, const void *buffer, size_t size)
{
    // if buffer is NULL, print error and return -1
    if (buffer == NULL)
    {
        fprintf(stderr, "write_to_pipe: buffer is NULL\n");
        return -1;
    }
    // if file descriptor is invalid, print error and return -1
    if (fd < 0)
    {
        fprintf(stderr, "write_to_pipe: file descriptor is invalid\n");
        return -1;
    }
    // write to the named pipe
    ssize_t b = write(fd, buffer, size);
    // if error, print error and return -1
    if (b == -1)
    {
        perror("write to named pipe");
        return -1;
    }
    // return the number of bytes written on success
    return b;
}

//  reads from the named pipe with this file descriptor, this buffer and this size
ssize_t read_from_pipe(int fd, void *buffer, size_t size)
{
    // if buffer is NULL, print error and return -1
    if (buffer == NULL)
    {
        fprintf(stderr, "read_from_pipe: buffer is NULL\n");
        return -1;
    }
    // if file descriptor is invalid, print error and return -1
    if (fd < 0)
    {
        fprintf(stderr, "read_from_pipe: file descriptor is invalid\n");
        return -1;
    }
    // read from the named pipe
    ssize_t b = read(fd, buffer, size);
    // if error, print error and return -1
    if (b == -1)
    {
        perror("read from named pipe");
        return -1;
    }
    // return the number of bytes readen on success
    return b;
}