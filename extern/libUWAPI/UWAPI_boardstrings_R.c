#include "UWAPI_boardstrings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static BOOLEAN isalnumdash(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '*';
}

BOOLEAN UWAPI_Board_Regular2D_ParsePositionString(char const *str, enum UWAPI_Turn *turn, unsigned int *num_rows, unsigned int *num_columns, char **board)
{
    // Format: "R_<turn>_<rows>_<columns>_<cells...>"

    static char *sep = "_";
    char *str_dup = UWAPI_StringDup(str);
    char *pch;

    // Expect R
    pch = strtok(str_dup, sep);
    if (strlen(pch) != 1 || pch[0] != 'R')
    {
        free(str_dup);
        return FALSE;
    }

    // Expect A/B for turn
    pch = strtok(NULL, sep);
    if (strlen(pch) != 1 || (pch[0] != UWAPI_TURN_A && pch[0] != UWAPI_TURN_B))
    {
        free(str_dup);
        return FALSE;
    }
    if (turn != NULL)
        *turn = (enum UWAPI_Turn)pch[0];

    // Expect num rows
    pch = strtok(NULL, sep);
    unsigned long num_rows_ = strtoul(pch, NULL, 0);
    if (num_rows_ == ULONG_MAX || num_rows_ > UINT_MAX)
    {
        // Failed to convert number or number out of range
        free(str_dup);
        return FALSE;
    }
    if (num_rows != NULL)
        *num_rows = (unsigned int)num_rows_;

    // Expect num columns
    pch = strtok(NULL, sep);
    unsigned long num_columns_ = strtoul(pch, NULL, 0);
    if (num_columns_ == ULONG_MAX || num_columns_ > UINT_MAX)
    {
        // Failed to convert number or number out of range
        free(str_dup);
        return FALSE;
    }
    if (num_columns != NULL)
        *num_columns = (unsigned int)num_columns_;

    // Expect board string of correct size
    pch = strtok(NULL, sep);
    unsigned int num_cells = num_rows_ * num_columns_;
    unsigned int i = 0;
    while (i < num_cells)
    {
        char b = pch[i];
        if (b == '\0' || !isalnumdash(b))
        {
            // Board string too short & only alphanumeric characters are allowed
            free(str_dup);
            return FALSE;
        }
        i++;
    }
    if (pch[i] != '\0')
    {
        free(str_dup);
        return FALSE; // Board string too long
    }
    if (board != NULL)
        *board = UWAPI_StringDup(pch);

    free(str_dup);
    return TRUE;
}

char *UWAPI_Board_Regular2D_MakePositionString(enum UWAPI_Turn turn, unsigned int num_rows, unsigned int num_columns, char const *board)
{
    // Format: "R_<turn>_<rows>_<columns>_<cells...>"

    unsigned int num_cells = num_rows * num_columns;

    // Make sure the board size matches the number of cells specified by num rows & columns
    // and only alphanumeric characters are included
    unsigned int i = 0;
    while (i < num_cells)
    {
        char b = board[i];
        if (b == '\0' || !isalnumdash(b))
        {
            // Board string too short & only alphanumeric characters are allowed
            return NULL;
        }
        i++;
    }
    if (board[i] != '\0')
    {
        return NULL; // Board string too long
    }

    // Unsigned ints don't exceed 10 digits
    char *str = malloc(
        1 + 1 +
        /* turn */ 1 + 1 +
        /* rows */ 10 + 1 +
        /* columns */ 10 + 1 +
        num_cells + /* null terminator */ 1);

    if (str == NULL)
        return str;

    sprintf(str, "R_%c_%d_%d_%s", turn, num_rows, num_columns, board);
    return str;
}

char *UWAPI_Board_Regular2D_MakeAddString(char piece, unsigned int to)
{
    // Format: "A_<piece>_<to>"

    char *str = malloc(
        1 + 1 +
        /* piece */ 1 + 1 +
        /* to */ 10 + /* null terminator */ 1);

    if (str == NULL)
        return str;

    sprintf(str, "A_%c_%d", piece, to);
    return str;
}

char *UWAPI_Board_Regular2D_MakeMoveString(char from, unsigned int to)
{
    // Format: "M_<from>_<to>"

    char *str = malloc(
        1 + 1 +
        /* from */ 10 + 1 +
        /* to */ 10 + /* null terminator */ 1);

    if (str == NULL)
        return str;

    sprintf(str, "M_%d_%d", from, to);
    return str;
}

char *UWAPI_Board_Regular2D_MakeShiftString(enum UWAPI_Board_Regular2DShiftDirection dir, unsigned int row, unsigned int amt)
{
    // Format: "S_<dir>_<row>_<amt>"

    char *str = malloc(
        1 + 1 +
        /* dir */ 1 + 1 +
        /* row */ 10 + 1 +
        /* amt */ 1 + /* null terminator */ 1);

    if (str == NULL)
        return str;

    sprintf(str, "S_%c_%d_%d", dir, row, amt);
    return str;
}
