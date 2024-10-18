#include "UWAPI_boardstrings.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *UWAPI_StringDup(char const *s)
{
    char *a = malloc(strlen(s) + 1); // Additional 1 for null terminator
    if (a)
        strcpy(a, s);
    return a;
}

char *UWAPI_ToStringi(int i)
{
    // Ints don't exceed 11 digits
    char *a = malloc(11 + 1); // Additional 1 for null terminator
    if (a)
        sprintf(a, "%d", i);
    return a;
}
