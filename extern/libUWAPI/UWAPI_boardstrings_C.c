#include "UWAPI_boardstrings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BOOLEAN UWAPI_Board_Custom_ParsePositionString(char const *str, char **board)
{
    // Format: "C_<board>"

    static char *sep = "_";
    char *str_dup = UWAPI_StringDup(str);
    char *pch;

    // Expect C
    pch = strtok(str_dup, sep);
    if (strlen(pch) != 1 || pch[0] != 'C')
    {
        free(str_dup);
        return FALSE;
    }

    if (board != NULL)
        *board = UWAPI_StringDup(&(pch[2]));

    free(str_dup);
    return TRUE;
}

char *UWAPI_Board_Custom_MakePositionString(char const *board)
{
    // Format: "C_<board>"

    size_t len = strlen(board);
    char *str = malloc(
        1 + 1 +
        /* board */ len + /* null terminator */ 1);

    if (str == NULL)
        return str;

    sprintf(str, "C_%s", board);
    return str;
}
