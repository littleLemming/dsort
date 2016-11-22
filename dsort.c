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


/* === Constants === */

/**
 * @brief max length for the strings in string_list - 1023 real characters and a \0 
 */
#define LINE_SIZE (1024)


/* === Macros === */

/**
 * @brief print debug messages if the debug flag is set 
 */
#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif


/* === Global Variables === */

/* Name of the program */
static const char *progname = "dsort"; /* default name */

/* String-List that contains the Output of command1 and command2 */
static struct string_list command_output;


/* === Type Definitions === */

/* Struct that represents a list of strings */
struct string_list {
    /* currently contained strings in the list */
    char** content;
    /* amount of strings stored in the list */
    int amnt_strings;
    /* current amount of allocated memory for strings - string-list is full if amnt_strings == max_amnt_strings */
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
 * write_to_child
 * @brief run command in child and pipe command_output of the parent to stdin of the child
 */
static void write_to_child(char* command);

/**
 * read_from_child
 * @brief run command in child and pipe stdout of the child to command_output of the parent
 */
static void read_from_child(char* command);


/* === Implementations === */

static void free_resources(void)
{
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
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(exitcode);
}

static void write_to_child(char* command) {
    DEBUG("write_to_childe %s\n",command);
    pid_t pid = fork ();
    switch (pid) {
        case -1:
            DEBUG("write_to_child - ERROR\n");
            break;
        case 0:
            DEBUG("write_to_child - CHILD\n");
            break;
        default:
            DEBUG("write_to_child - PARENT\n");
            break;
    }
}

static void read_from_child(char* command) {
    DEBUG("read_from_child %s\n",command);
    pid_t pid = fork (); 
    switch (pid) {
        case -1: 
            DEBUG("read_from_child - ERROR\n");
            break;
        case 0:
            DEBUG("read_from_child - CHILD\n");
            break;
        default:
            DEBUG("read_from_child - PARENT\n");
            break;
    }   
}

int main(int argc, char **argv) {
    if(argc > 0) {
        progname = argv[0];
    }
    if (argc != 3) {
        bail_out(EXIT_FAILURE, "Usage: %s <command1> <command2>", progname);
    }
    read_from_child(argv[1]);
    read_from_child(argv[2]);
    return EXIT_SUCCESS;
}
