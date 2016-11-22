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

/** Name of the program */
static const char *progname = "dsort"; /* default name */

/* === Type Definitions === */

/* === Prototypes === */

/* === Implementations === */

int main(int argc, char **argv) {

}
