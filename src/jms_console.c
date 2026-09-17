#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/ipc.h"

// reads input from the stdin or a file and writes it to the named pipe
int reads_input(int fd_write, int fd_read, FILE *operations_file)
{
    char buffer[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    FILE *from;

    // if operations_file is not NULL read from it, otherwise read from stdin
    if (operations_file != NULL)
    {
        from = operations_file;
    }
    else
    {
        from = stdin;
    }
    // read lines from the input
    while (fgets(buffer, BUFFER_SIZE, from) != NULL)
    {
        // remove newline and other whitespace characters from the buffer
        int len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ' || buffer[len - 1] == '\t'))
        {
            buffer[len - 1] = '\0';
            len--;
        }
        // if the line is empty, continue to the next line
        if (len == 0)
        {
            continue;
        }
        // check if the command is "shutdown" for stopping
        int is_shutdown = 0;
        if (strcmp(buffer, "shutdown") == 0)
            is_shutdown = 1;
        // write the command to the named pipe
        if (write_to_pipe(fd_write, buffer, strlen(buffer) + 1) == -1)
        {
            fprintf(stderr, "reads_input: failed to write\n");
            return -1;
        }
        // read the reply from the named pipe and print it
        while (1)
        {
            ssize_t n = read_from_pipe(fd_read, reply, BUFFER_SIZE - 1);
            if (n == -1)
            {
                fprintf(stderr, "reads_input: failed to read\n");
                return -1;
            }
            if (n == 0)
                continue;

            reply[n] = '\0';
            char *end = strstr(reply, "OK");
            if (end == NULL)
            {
                printf("%s", reply);
                continue;
            }
            else if (end != NULL)
            {
                *end = '\0';
                printf("%s", reply);
                break;
            }
        }
        // if the command is "shutdown", return 1
        if (is_shutdown)
            return 1;
    }
    // return 0 if the end of the file
    return 0;
}

int main(int argc, char *argv[])
{
    // printf("Hello, JMS Console!\n"); // DELETE AT THE END  !!!

    char *jms_in = NULL;
    char *jms_out = NULL;
    char *operations_file = NULL;

    // parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        // check for -w
        if (strcmp(argv[i], "-w") == 0)
        {
            // check if the next argument is not another flag or if jms_in is already set, print error and return 1
            if (i + 1 < argc && jms_in == NULL && strcmp(argv[i + 1], "-r") != 0 && strcmp(argv[i + 1], "-o") != 0)
            {
                jms_in = argv[++i];
            }
            else if (jms_in != NULL)
            {
                fprintf(stderr, "Error: Multiple input files specified for -w\n");
                return 1;
            }
            else
            {
                fprintf(stderr, "Error: Missing input file after -w\n");
                return 1;
            }
        }
        // check for -r
        else if (strcmp(argv[i], "-r") == 0)
        {
            // check if the next argument is not another flag or if jms_out is already set, print error and return 1
            if (i + 1 < argc && jms_out == NULL && strcmp(argv[i + 1], "-w") != 0 && strcmp(argv[i + 1], "-o") != 0)
            {
                jms_out = argv[++i];
            }
            else if (jms_out != NULL)
            {
                fprintf(stderr, "Error: Multiple output files specified for -r\n");
                return 1;
            }
            else
            {
                fprintf(stderr, "Error: Missing output file after -r\n");
                return 1;
            }
        }
        // check for -o
        else if (strcmp(argv[i], "-o") == 0)
        {
            // check if the next argument is not another flag or if operations_file is already set, print error and return 1
            if (i + 1 < argc && operations_file == NULL && strcmp(argv[i + 1], "-w") != 0 && strcmp(argv[i + 1], "-r") != 0)
            {
                operations_file = argv[++i];
            }
            else if (operations_file != NULL)
            {
                fprintf(stderr, "Error: Multiple operations files specified for -o\n");
                return 1;
            }
            else
            {
                fprintf(stderr, "Error: Missing operations file after -o\n");
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

    // check if jms_in and jms_out are set, if not print error and return 1
    if (!jms_in || !jms_out)
    {
        fprintf(stderr, "Usage: ./jms_console -w jms_in -r jms_out [-o operations_file]\n");
        return 1;
    }

    // fprintf(stderr, "JMS Input: %s\n", jms_in);                                        // DELETE AT THE END  !!!
    // fprintf(stderr, "JMS Output: %s\n", jms_out);                                      // DELETE AT THE END  !!!
    // fprintf(stderr, "Operations File: %s\n", operations_file ? operations_file : "-"); // DELETE AT THE END  !!!

    // open the named pipes for writing and reading
    int fd_write = open_named_pipe(jms_in, O_WRONLY);
    // if error, print error and return 1
    if (fd_write < 0)
    {
        return 1;
    }

    // open the named pipe for reading, if error, print error and return 1
    int fd_read = open_named_pipe(jms_out, O_RDONLY);
    // if error, print error and return 1
    if (fd_read < 0)
    {
        close(fd_write);
        return 1;
    }

    // fprintf(stderr, "Opened named pipes: %s (write), %s (read)\n", jms_in, jms_out); // DELETE AT THE END  !!!

    // if operations_file is not NULL, read from it, otherwise read from stdin
    if (operations_file != NULL)
    {
        // open the operations file for reading, if error, print error and return 1
        FILE *fp = fopen(operations_file, "r");
        if (fp == NULL)
        {
            perror("jms_console:fopen");
            close(fd_write);
            close(fd_read);
            return 1;
        }

        // read the operations from the file and write them to the named pipe, if error, print error and return 1
        int result = reads_input(fd_write, fd_read, fp);
        // close the operations file
        fclose(fp);

        // if -1 that means there was an error, if 1 that means the shutdown command was encountered, if 0 that means the end of the file was reached
        if (result == -1)
        {
            close(fd_write);
            close(fd_read);
            return 1;
        }
        //  if 1 that means there was shutdown
        else if (result == 1)
        {
            close(fd_write);
            close(fd_read);
            return 0;
        }
    }
    // if 0 that means the end of the file was reached and continues reading from stdin
    // read from stdin and write to the named pipe
    if (reads_input(fd_write, fd_read, NULL) == -1)
    {
        close(fd_write);
        close(fd_read);
        return 1;
    }
    // close the named pipes
    close(fd_write);
    close(fd_read);
    return 0;
}