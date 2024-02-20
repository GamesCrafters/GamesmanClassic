#include "autoguistrings.h"

BOOLEAN ParseStandardOnelinePositionString(char *positionString, int *turn, char **entityString) {
    // Format: "<turn>_<entityString>" (`turn` is either '0', '1' or '2')
    // Example (TicTacToe): 1_---------, 2_--x------, 1_--x-o----, 2_x-x-o----
    // 1 if player 1's turn, 2 if player 2's turn. 0 if the game is impartial and
    // it can be either player's turn
    // Note: We establish a limit of 128 on the length of a position string.
    // It can probably be increased without issue, but no games at the time of
    // writing have a position string this long, so we don't find it necessary.

    size_t len = strnlen(positionString, MAX_POSITION_STRING_LENGTH << 1);
    if (len < 3 || (positionString[0] != '1' && positionString[0] != '2') || len > MAX_POSITION_STRING_LENGTH) {
        return 0;
    } else {
        *turn = positionString[0] - '0';
        *entityString = positionString + 2;
        return 1;
    }
}

/* `entityString` must be null-terminated. */
void AutoGUIMakePositionString(int turn, char *entityString, char *buf) {
    snprintf(buf, MAX_POSITION_STRING_LENGTH, "%d_%s", turn, entityString);
}

void AutoGUIMakeMoveButtonStringA(char token, unsigned int at, char sound, char *buf) {
    snprintf(buf, MAX_MOVE_STRING_LENGTH, "A_%c_%d_%c", token, at, sound);
}

/* `text` must be null-terminated. */
void AutoGUIMakeMoveButtonStringT(char *text, unsigned int at, char sound, char *buf) {
    snprintf(buf, MAX_MOVE_STRING_LENGTH, "T_%s_%d_%c", text, at, sound);
}

void AutoGUIMakeMoveButtonStringM(unsigned int from, unsigned int to, char sound, char *buf) {
    snprintf(buf, MAX_MOVE_STRING_LENGTH, "M_%d_%d_%c", from, to, sound);
}

void AutoGUIMakeMoveButtonStringL(unsigned int p1, unsigned int p2, char sound, char *buf) {
    snprintf(buf, MAX_MOVE_STRING_LENGTH, "L_%d_%d_%c", p1, p2, sound);
}

void AutoGUIWriteEmptyString(char *buf) {
    buf[0] = '\0';
}