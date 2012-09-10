#ifndef GMCORE_DIAGNOSTIC_H
#define GMCORE_DIAGNOSTIC_H

/***
 * Define constants
 ***/

/* File name for console output */
#define CONSOLE_FILENAME "/var/log/gamesman.log"

/* Number of possible flags (a-z) */
#define CONSOLE_FLAGS    26

/* Number of possible levels per flag */
#define CONSOLE_LEVELS   10

/***
 * Function prototypes
 ***/

/*
   Function: diagnostic_set_flag
    Toggles all levels of debugging flag on

   Arguments:
    flag - flag to toggle on
 */
void diagnostic_set_flag(char flag);

/*
   Function: diagnostic_set_flag_level
     Toggles given level of debugging flag on

   Arguments:
     flag  - flag to operate on
     level - level of flag to enable
 */
void diagnostic_set_flag_level(char flag, unsigned short level);

/*
   Function: diagnostic
     Prints diagnostic message to debugging console if flag level is set

   Arguments:
     flag   - debug flag of message
     level  - debug level of message
     format - format string
     ...    - variable number of arguments corresponding to format string
 */
void diagnostic(char flag, unsigned short level, char *format, ...);

#endif /* GMCORE_DIAGNOSTIC_H */
