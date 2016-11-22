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
#include<stdarg.h>


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


/* === Type Definitions === */


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


/* === Implementations === */

static void free_resources(void)
{
    /* clean up resources */
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

int main(int argc, char **argv) {
    if(argc > 0) {
		progname = argv[0];
	}
    if (argc != 3) {
        bail_out(EXIT_FAILURE, "Usage: %s <command1> <command2>", progname);
    }
}
