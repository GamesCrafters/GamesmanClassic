/* Include support for standard output */
#include <stdio.h>

/* Include support for variable number of parameters */
#include <stdarg.h>

/* Include support for gamesman types */
#include "gamesman.h"

/* Include define constants and function prototypes */
#include "diagnostic.h"

/* Array of diagnostic flags set */
BOOLEAN diagnostic_flags[CONSOLE_FLAGS][CONSOLE_LEVELS];

/*
   Function: diagnostic_set_flag
    Toggles all levels of debugging flag on

   Arguments:
    flag - flag to toggle on
 */
void diagnostic_set_flag(char flag) {

	int level;

	/* Normalize flag */
	flag = tolower(flag) - 'a';

	/* For every possible level for flag */
	for (level = 0; level < CONSOLE_LEVELS; level++) {

		/* Toggle flag level on */
		diagnostic_flags[flag][level] = TRUE;

	}

}

/*
   Function: diagnostic_set_flag_level
     Toggles given level of debugging flag on

   Arguments:
     flag  - flag to operate on
     level - level of flag to enable
 */
void diagnostic_set_flag_level(char flag, unsigned short level) {

	/* Normalize flag */
	flag = tolower(flag) - 'a';

	/* Toggle flag level on */
	diagnostic_flags[flag][level] = TRUE;

}

/*
   Function: diagnostic
     Prints diagnostic message to debugging console if flag level is set

   Arguments:
     flag   - debug flag of message
     level  - debug level of message
     format - format string
     ...    - variable number of arguments corresponding to format string
 */
void diagnostic(char flag, unsigned short level, char *format, ...) {

	FILE *obtain_console();
	void release_console(FILE *console);

	va_list args;

	/* Initialize variable argument list */
	va_start(args, format);

	/* If flag-level is set */
	if (diagnostic_flags[tolower(flag) -'a'][level]) {

		FILE *console;

		/* Obtain the console stream */
		console = obtain_console();

		/* Print diagnostic message to debug console */
		fprintf(console, format, args);

		/* Release the console stream */
		release_console(console);
	}

	/* Finalize variable argument list */
	va_end(args);

}

FILE *obtain_console() {

	/* Open console file for appending */
	return fopen(CONSOLE_FILENAME, "a");

}

void release_console(FILE *console) {

	/* Close console file */
	fclose(console);
}
