/**
 * @file dsort.c
 *
 * @author Ulrike Schaefer 1327450
 *
 * 
 * @brief dsort executes command1 and command2 and prints the duplicated output of those commands in a sorted order
 *
 * @details dsort execute command1 and command2, save the ouput of th commands in an array, 
			this array gets sorted and only duplicated lines will be printed. 
 *
 * @date 22.11.2016
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>

/* === Constants === */

/**
 * LINE_SIZE
 * @brief max length for the strings in string_list - 1023 real characters and a \0 
 */
#define LINE_SIZE (1024)


/* === Macros === */

/**
 * DEBUG
 * @brief print debug messages if the debug flag is set 
 */
#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif


/* === Global Variables === */

/**
 * progname
 * @brief name of the program
 */
static const char *progname = "dsort";

/**
 * command_output (=struct string_list)
 * string-List that contains the Output of command1 and command2 (= input)
 */
static struct string_list command_output;


/* === Type Definitions === */

/**
 * struct string_list
 * @brief struct that represents a list of strings 
 */
struct string_list {
    /** currently contained strings in the list */
    char** content; 
    /** amount of strings stored in the list */
    int amnt_strings;
    /** current amount of allocated memory for strings - string-list is full if amnt_strings == max_amnt_strings */ 
    int max_amnt_strings; 
};


/* === Prototypes === */

/**
 * free_resources
 * @brief free allocated resources
 */
static void free_resources(void);

/**
 * bail_out
 * @brief terminate program on program error
 * @param exitcode exit code
 * @param fmt format string
 */
static void bail_out(int exitcode, const char *fmt, ...);

/**
 * read_from_child
 * @brief run command in child and pipe stdout of the child to command_output of the parent
 */
static void read_from_child(char* command);

/**
 * write_to_child
 * @brief run command in child and command_output of the parent to stdin of the child
 */
static void write_to_child(char* command);

/**
 * wait_for_child
 * @brief partent-process waits for child-process to terminate before parent terminates
 */
static void wait_for_child(pid_t child_pid);

/**
 * cmpstringp
 * @brief method to compare two strings - for qsort
 * @param p1 one string
 * @param p2 other string
 */
static int cmpstringp(const void *p1, const void *p2);

/* === Implementations === */

static void free_resources(void)
{
    DEBUG("free_resources");
    /* clean up resources */
    for (int i = 0; i < command_output.amnt_strings; ++i) {
        free(command_output.content[i]);
    }
    free(&command_output.content[0]);
    command_output.content = NULL;
    command_output.amnt_strings = 0;
    command_output.max_amnt_strings = 0;
}

static void bail_out(int exitcode, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", progname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    } if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(exitcode);
}

static void read_from_child(char* command) {
    DEBUG("read_from_child %s\n",command);
    /* create argument list for executing the command */
    char *cmd[] = { "bash", "-c", command, (char *) 0};
    /* create the pipe */
    int command_pipe[2];
    if(pipe(command_pipe) == -1) {
        bail_out(errno, "create pipe failed\n");
    } 
    /* flush stdout before fork so that buffer is empty */
    if(fflush(stdout) != 0) {
        bail_out(errno, "flush of stdout failed\n");
    }
    /* do fork */
    pid_t pid;
    switch (pid = fork()) {
        case -1: 
            DEBUG("read_from_child - ERROR\n");
            bail_out(errno,"fork failed\n");
            break;
        case 0:
            /* child - execute command and rewire stdout to write end of pipe */
            DEBUG("read_from_child - CHILD\n");
            /* close unused pipe end - read */
            if(close(command_pipe[0]) != 0) {
                bail_out(errno,"close pipe read end of child failed\n");
            }
            /* rewire stdout to the write pipe end */
            if(dup2(command_pipe[1], fileno(stdout)) < 0) {
                bail_out(errno,"rewire stdout to write pipe end failed\n");
            }
            /* close write pipe after redirect */
            if(close(command_pipe[1]) != 0) {
                bail_out(errno,"close pipe write end of child failed after rewire\n");
            }
            /* execute command */
            (void)execv("/bin/bash", cmd);
            bail_out(errno,"executing %s failed\n",command);
            break;
        default:
            /* parent - read whatever the child writes and write it to command_output */
            DEBUG("read_from_child - PARENT\n");
            /* close unused pipe end - write */
            if(close(command_pipe[1]) != 0) {
                bail_out(errno,"close pipe read end of parent failed\n");
            }
            /* create buffer for next line */
            char next_line[LINE_SIZE];
            /* create stream for reading the output from the child */
            FILE* output_stream;
            if ((output_stream = fdopen(command_pipe[0], "r")) == NULL) {
                bail_out(errno,"creating of write-stream failed\n");
            }
            /* get next line of output and save in command output */
            while(fgets(next_line, sizeof(next_line), output_stream) != NULL) {
                 if(command_output.content == NULL) {
                     command_output.max_amnt_strings = 5;
                     command_output.content = malloc(command_output.max_amnt_strings * sizeof(char*));
                 } else if(command_output.max_amnt_strings == command_output.amnt_strings) {
                     command_output.max_amnt_strings += 5;
                     command_output.content = realloc(command_output.content, command_output.max_amnt_strings * sizeof(char*));
                 }
                 command_output.content[command_output.amnt_strings] = strdup(next_line);
                 ++command_output.amnt_strings;
            } if(fclose(output_stream) != 0) {
                bail_out(errno,"close output-read-stream failed\n");
            }
            wait_for_child(pid);
    }   
}

static void write_to_child(char* command) {
    DEBUG("write_to_child %s\n",command);
    /* create argument list for executing the command */
    char *cmd[] = { "bash", "-c", command, (char *) 0}; 
    /* create the pipe */
    int command_pipe[2];
    if(pipe(command_pipe) == -1) {
        bail_out(errno, "create pipe failed\n");
    }   
    /* flush stdout before fork so that buffer is empty */
    if(fflush(stdout) != 0) {
        bail_out(errno, "flush of stdout failed\n");
    }   
    /* do fork */
    pid_t pid = fork(); 
    switch (pid) {
        case -1: 
            DEBUG("write_to_child - ERROR\n");
            bail_out(errno,"fork failed\n");
            break;
        case 0:
            /* child - execute command and get input for the command from read pipe rewired to stdin */
            DEBUG("write_to_child - CHILD\n");
            /* close unused pipe end - write */
            if(close(command_pipe[1]) != 0) {
                bail_out(errno,"close pipe write end of child failed\n");
            }
            /* rewire stdin to the read pipe end */
            if(dup2(command_pipe[0], fileno(stdin)) < 0) {
                bail_out(errno,"rewire stdin to read pipe end failed\n");
            }
            /* close read pipe after redirect */
            if(close(command_pipe[0]) != 0) {
                bail_out(errno,"close pipe read end of child failed after rewire\n");
            }
            /* execute command */
            (void)execv("/bin/bash", cmd);
            bail_out(errno,"executing %s failed\n",command);
            break;
        default:
            /* parent - write whatever is in command_output into the write end of the pipe */
            DEBUG("write_to_child - PARENT\n");
            /* close unused pipe end - read */
            if(close(command_pipe[0]) != 0) {
                bail_out(errno,"close pipe read end of parent failed\n");
            }
            /* create stream for writing command_output to the child */
            FILE *input_stream;
            if ((input_stream = fdopen(command_pipe[1], "w")) == NULL) {
                bail_out(errno,"creating write-stream failed\n");
            }
            /* write item for item from command_output into input stream */
            for (int i = 0; i < command_output.amnt_strings; ++i) {
                if(fputs(command_output.content[i], input_stream) == EOF) {
                    bail_out(EXIT_FAILURE,"writing to child failed\n");
                }
            }
            if(fclose(input_stream) != 0) {
                bail_out(errno,"close input-write-stream failed\n");
            }
            wait_for_child(pid);
    }   
}

static void wait_for_child(pid_t child_pid) {
    DEBUG("wait_for_child\n");
    int status;
    pid_t pid;
    while((pid = wait(&status)) != child_pid) {
        if(pid != -1) {
            continue; /* other child */
        } if(errno == EINTR) {
            continue; /* interrupted */
        }
        bail_out(errno,"wait for child failed\n");
    } if(WEXITSTATUS(status) == EXIT_SUCCESS) {
        DEBUG("child exit successfully\n");
    } else {
        DEBUG("error in child occured\n");
    }
}

static int cmpstringp(const void *p1, const void *p2) {
    /* The actual arguments to this function are "pointers to
    pointers to char", but strcmp(3) arguments are "pointers
    to char", hence the following cast plus dereference */
    return strcmp(*(char * const *)p1, *(char * const *)p2);
}

/**
 * main
 * @brief starting point of program
 * @param argc number of program arguments
 * @param argv program arguments
 */
int main(int argc, char **argv) {
    /* check if correct intput was passed on */
    if(argc > 0) {
        progname = argv[0];
    } if (argc != 3) {
        bail_out(EXIT_FAILURE, "Usage: %s <command1> <command2>\n", progname);
    }
    /* exectue commands and read output from clients into command_output */
    command_output.content = NULL;
    command_output.amnt_strings = 0;
    command_output.max_amnt_strings = 0;
    read_from_child(argv[1]);
    for (int i = 0; i < command_output.amnt_strings; ++i) {
        DEBUG("%d: %s\n", i, command_output.content[i]);
    }
    read_from_child(argv[2]);
    for (int i = 0; i < command_output.amnt_strings; ++i) {
        DEBUG("%d: %s\n", i, command_output.content[i]);
    }
    /* sort command_output */
    qsort(command_output.content, command_output.amnt_strings, sizeof(char*), cmpstringp);
    for (int i = 0; i < command_output.amnt_strings; ++i) {
        DEBUG("%d: %s\n", i, command_output.content[i]);
    } 
    /* execute uniq -d and print */
    write_to_child("uniq -d");    
    /* free resources and exit program without error */    
    free_resources();  
    return EXIT_SUCCESS;
}
