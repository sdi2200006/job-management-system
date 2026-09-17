#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../include/shared.h"
#define PIPE_IN "jms_in"
#define PIPE_OUT "jms_out"

int main(int argc, char *argv[])
{
    char *path = NULL;
    int jobs_pool = -1;

    for (int i = 1; i < argc; i++)
    {
        // check for -l
        if (strcmp(argv[i], "-l") == 0)
        {
            // check if the next argument is not another flag or if path is already set, print error and return 1
            if (i + 1 < argc && path == NULL && strcmp(argv[i + 1], "-n") != 0)
            {
                path = argv[++i];
            }
            else if (path != NULL)
            {
                fprintf(stderr, "Error: Multiple paths specified for -l\n");
                return 1;
            }
            // if the next argument is another flag, print error and return 1
            else
            {
                fprintf(stderr, "Error: Missing path after -l\n");
                return 1;
            }
        }

        else if (strcmp(argv[i], "-n") == 0)
        {
            // check if the next argument is not another flag or if jobs_pool is already set, print error and return 1
            if (i + 1 < argc && jobs_pool == -1 && strcmp(argv[i + 1], "-l") != 0)
            {
                // take the integer value
                jobs_pool = atoi(argv[++i]);

                // if the value is not a positive integer, print error and return 1
                if (jobs_pool <= 0)
                {
                    fprintf(stderr, "Error: Jobs pool must be a integer bigger than 0\n");
                    return 1;
                }
            }
            // if jobs_pool is already set, print error and return 1
            else if (jobs_pool != -1)
            {
                fprintf(stderr, "Error: Multiple jobs pool values specified for -n\n");
                return 1;
            }

            else
            {
                fprintf(stderr, "Error: Missing jobs pool value after -n\n");
                return 1;
            }
        }
        // if the argument is not recognized, print error and return 1
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    // check if path and jobs_pool are set, if not print error and return 1
    if (!path || jobs_pool == -1)
    {
        fprintf(stderr, "Usage: ./jms_coord -l path -n jobs_pool\n");
        return 1;
    }
    // create the directory if it does not exist, if error return 1
    if (mkdir(path, 0744) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkdir");
            return 1;
        }
    }

    // fprintf(stderr, "Path: %s\n", path);           // DELETE AT THE END  !!!
    // fprintf(stderr, "Jobs Pool: %d\n", jobs_pool); // DELETE AT THE END  !!!

    // create the named pipes, if error return 1
    if (create_named_pipe(PIPE_IN) == -1)
    {
        return 1;
    }

    if (create_named_pipe(PIPE_OUT) == -1)
    {
        remove_named_pipe(PIPE_IN);
        return 1;
    }
    // set the jobs pool size
    set_N(jobs_pool);

    // fprintf(stderr, "Created named pipes: %s, %s\n", PIPE_IN, PIPE_OUT);

    // open the named pipe for writing, if error return 1
    int fd_read = open_named_pipe(PIPE_IN, O_RDONLY);
    if (fd_read < 0)
    {
        remove_named_pipe(PIPE_IN);
        remove_named_pipe(PIPE_OUT);
        return 1;
    }
    // open the named pipe for reading, if error return 1
    int fd_write = open_named_pipe(PIPE_OUT, O_WRONLY);
    if (fd_write < 0)
    {
        close(fd_read);
        remove_named_pipe(PIPE_IN);
        remove_named_pipe(PIPE_OUT);
        return 1;
    }

    // fprintf(stderr, "Opened named pipes: jms_in (read), jms_out (write)\n");

    char buffer[BUFFER_SIZE];

    while (1)
    {
        update_status();
        ssize_t n = read_from_pipe(fd_read, buffer, BUFFER_SIZE - 1);
        if (n == -1)
        {
            close(fd_write);
            close(fd_read);
            remove_named_pipe(PIPE_IN);
            remove_named_pipe(PIPE_OUT);
            return 1;
        }

        if (n == 0)
        {
            continue;
        }

        buffer[n] = '\0';
        while (n > 0 && (buffer[n - 1] == '\n' || buffer[n - 1] == '\r' || buffer[n - 1] == ' ' || buffer[n - 1] == '\t'))
        {
            buffer[n - 1] = '\0';
            n--;
        }
        if (n == 0)
        {
            continue;
        }
        // printf("Received: %s\n", buffer); // DELETE AT THE END  !!!

        if (strncmp(buffer, "submit ", strlen("submit ")) == 0)
        {
            if (handle_submit(fd_write, buffer) != 0)
            {
                break;
            }
        }
        else if (strncmp(buffer, "status ", strlen("status ")) == 0)
        {
            if (handle_status(fd_write, buffer) != 0)
            {
                break;
            }
        }
        else if (strncmp(buffer, "status-all", 10) == 0 && (buffer[10] == '\0' || buffer[10] == ' '))
        {
            if (handle_status_all(fd_write, buffer) != 0)
            {
                break;
            }
        }
        else if (strcmp(buffer, "show-active") == 0)
        {
            if (handle_show_active(fd_write) != 0)
            {
                break;
            }
        }
        else if (strcmp(buffer, "show-pools") == 0)
        {
            if (handle_show_pools(fd_write) != 0)
            {
                break;
            }
        }
        else if (strcmp(buffer, "show-finished") == 0)
        {
            if (handle_show_finished(fd_write) != 0)
            {
                break;
            }
        }
        else if (strncmp(buffer, "suspend ", strlen("suspend ")) == 0)
        {
            if (handle_suspend(fd_write, buffer) != 0)
            {
                break;
            }
        }
        else if (strncmp(buffer, "resume ", strlen("resume ")) == 0)
        {
            if (handle_resume(fd_write, buffer) != 0)
            {
                break;
            }
        }
        else if (strcmp(buffer, "shutdown") == 0)
        {
            if (handle_shutdown(fd_write) != 0)
            {
                break;
            }
        }
        else
        {
            write_to_pipe(fd_write, "unknown command\n", strlen("unknown command\n") + 1);
            write_to_pipe(fd_write, "OK\n", strlen("OK\n"));
        }
    }
    // close the named pipes, remove them and return 0
    close(fd_write);
    close(fd_read);
    remove_named_pipe(PIPE_IN);
    remove_named_pipe(PIPE_OUT);
    return 0;
}