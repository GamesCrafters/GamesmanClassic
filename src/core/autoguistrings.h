#ifndef GMCORE_AUTOGUI_STRINGS_H
#define GMCORE_AUTOGUI_STRINGS_H

#include "types.h"
#include <string.h>
#include <stdio.h>

/* These can be increased in the future. At the time of writing,
no positions strings nor autogui position strings are longer than 128
and no move strings or autogui move strings are longer than 32. */
#define MAX_POSITION_STRING_LENGTH 128
#define MAX_MOVE_STRING_LENGTH 32

/**
 * @brief Parse a position string that is in STANDARD ONELINE POSITION STRING FORMAT,
 * i.e. it is formatted as <turn>_<board>, where...
 *      `turn` is a character. ('1' if it's Player 1's turn or '2' if it's Player 2's turn.)
 *      `board` is a string of characters that does NOT contain colons or semicolons.
 * Example tic-tac-toe position string: 2_ox-xxo-xo
 * 
 * @param positionString The position string to parse.
 * @param turn Pointed value is set to 1 if it's Player 1's Turn, or 2 otherwise.
 * @param board Pointer to `positionString` + 2.
 * @return TRUE if parsed successfully, else FALSE
 */
BOOLEAN ParseStandardOnelinePositionString(char *positionString, int *turn, char **board);

/**
 * @brief Write a position string in STANDARD ONELINE POSITION STRING FORMAT to the input buffer.
 * See above for the explanation of STANDARD ONELINE POSITION STRING FORMAT.
 * 
 * @param turn An integer, which is 1 if it's Player 1's turn, or 2 if it's Player 2's turn. 
 * If the game is impartial and we want to input an arbitrary turn, then this value is 0.
 * @param board A null-terminated string that SHOULD CONTAIN NEITHER SEMICOLONS NOR COLONS
 * whose length should be less than MAX_POSITION_STRING_LENGTH.
 * @param buf The buffer to write the formatted position string to.
 */
void AutoGUIMakePositionString(int turn, char *board, char *buf);

/**
 * @brief Write an AutoGUI A-type move button string to the input buffer. An A-type
 * move button string is of the format "A_<token>_<at>_<sound>" (Example: A_y_30_x)
 * 
 * @param token An alphanumeric character, if we have a custom Move Button Shape
 * SVG mapped to that character; otherwise it is '-' if we are using the standard
 * circle move button.
 * @param at An integer, representing the ID of the coordinate at which this button 
 * should be centered.
 * @param sound This is alphanumeric character if you have a sound file mapped to the character;
 * otherwise it is '-'.
 * @param buf The buffer that this formatted move button string will be written to.
 */
void AutoGUIMakeMoveButtonStringA(char token, unsigned int at, char sound, char *buf);

/**
 * @brief Write an AutoGUI T-type (text) move button string to the input buffer. A T-type
 * move button string is of the format "T_<text>_<at>_<sound>" (Example: T_hello_30_z)
 * 
 * @param text A null-terminated string. This string will be the move button shape, e.g.,
 * if text is "hello", then there will be a "hello" string that can be clicked to make the move.
 * @param at An integer, representing the ID of the coordinate at which this button 
 * should be centered.
 * @param sound This is alphanumeric character if you have a sound file mapped to the
 * character; otherwise it is '-'.
 * @param buf The buffer that this formatted move button string will be written to.
 */
void AutoGUIMakeMoveButtonStringT(char *text, unsigned int at, char sound, char *buf); 

/**
 * @brief Write an AutoGUI M-type (arrow) move button string to the input buffer. An M-type
 * move button string is of the format "M_<from>_<to>_<sound>" (Example: M_30_25_f)
 * 
 * @param from An integer, representing the ID of the coordinate for the arrow source.
 * @param to An integer, representing the ID of the coordinate for the arrow destination.
 * @param sound This is alphanumeric character if you have a sound file mapped to the 
 * character; otherwise it is '-'.
 * @param buf The buffer that this formatted move button string will be written to.
 */
void AutoGUIMakeMoveButtonStringM(unsigned int from, unsigned int to, char sound, char *buf);

/**
 * @brief Write an AutoGUI L-type (line) move button string to the input buffer. An A-type
 * move button string is of the format "L_<token>_<at>_<sound>" (Example: L_43_2_h)
 * 
 * @param p1 An integer, representing the ID of the coordinate for one line endpoint.
 * @param p2 An integer, representing the ID of the coordinate for the other line endpoint.
 * @param sound This is alphanumeric character if you have a sound file mapped to the character;
 * otherwise it is '-'.
 * @param buf The buffer that this formatted move button string will be written to.
 */
void AutoGUIMakeMoveButtonStringL(unsigned int p1, unsigned int p2, char sound, char *buf);

/**
 * @brief Write an empty string to the input buffer `buf`.
 */
void AutoGUIWriteEmptyString(char *buf);

#endif // GMCORE_AUTOGUI_STRINGS_H

