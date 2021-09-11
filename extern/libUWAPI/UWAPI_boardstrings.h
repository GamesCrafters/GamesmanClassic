#ifndef UWAPI_BOARDSTRINGS_H
#define UWAPI_BOARDSTRINGS_H

#ifndef BOOLEAN
#define BOOLEAN int
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

enum UWAPI_Turn
{
    UWAPI_TURN_A = 'A',
    UWAPI_TURN_B = 'B'
};

// Regular 2D

enum UWAPI_Board_Regular2DShiftDirection
{
    UWAPI_BOARD_REGULAR2D_SHIFT_DIRECTION_LEFT = 'L',
    UWAPI_BOARD_REGULAR2D_SHIFT_DIRECTION_RIGHT = 'R'
};

BOOLEAN UWAPI_Board_Regular2D_ParsePositionString(
    char const *str,
    enum UWAPI_Turn *turn,
    unsigned int *num_rows,
    unsigned int *num_columns,
    char **board);
char *UWAPI_Board_Regular2D_GetAdditionalParam(char const *str, char const *key);
char *UWAPI_Board_Regular2D_MakePositionString(
    enum UWAPI_Turn turn,
    unsigned int num_rows,
    unsigned int num_columns,
    char const *board);
char *UWAPI_Board_Regular2D_MakePositionStringWithAdditionalParams(
    enum UWAPI_Turn turn,
    unsigned int num_rows,
    unsigned int num_columns,
    char const *board,
    ...);
char *UWAPI_Board_Regular2D_MakeAddString(char piece, unsigned int to);
char *UWAPI_Board_Regular2D_MakeMoveString(unsigned int from, unsigned int to);
char *UWAPI_Board_Regular2D_MakeShiftString(
    enum UWAPI_Board_Regular2DShiftDirection dir,
    unsigned int row,
    unsigned int amt);

// Custom

BOOLEAN UWAPI_Board_Custom_ParsePositionString(
    char const *str,
    char **board);
char *UWAPI_Board_Custom_MakePositionString(
    char const *board);

// Helper methods

char *UWAPI_StringDup(char const *s);
char *UWAPI_ToStringi(int);

#endif // UWAPI_BOARDSTRINGS_H