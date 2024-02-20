/************************************************************************
**
** NAME:        mchopsticks.c
**
** DESCRIPTION: Chopsticks
**
** AUTHOR:      Cameron Cheung
**
** DATE:        2023-11-15
**
************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName = "Cameron Cheung";
CONST_STRING kGameName = "Chopsticks";
CONST_STRING kDBName = "chopsticks";
POSITION gNumberOfPositions = 1250;
POSITION gInitialPosition = 312;
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
POSITION GetCanonicalPosition(POSITION);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void *gGameSpecificTclInit = NULL;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kDebugMenu = FALSE;
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = "";
CONST_STRING kHelpExample = "";
void DebugMenu() {}
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

/*
  Position Encoding:
  (# of fingers on Player 2's right hand) * 250 +
  (# of fingers on Player 2's left hand) * 50 +
  (# of fingers on Player 1's right hand) * 10 +
  (# of fingers on Player 1's left hand) * 2
  + turn bit (0 = Player 1's turn; 1 = Player 2's turn)

  [Note: # of fingers ranges from 0 to 4 inclusive]

  Move Name Examples:
  RL (Current player's right hand attacks opponent's left hand)
  RR (Current player's right hand attacks oppoennt's right hand)
  L2 (Current player's left hand transfers 2 to current player's right hand)
  R1 (Current player's right hand transfers 1 to current player's left hand)
  L0 (Current player's left hand transfers 0 to current player's right hand)

  Move Encoding:
  A move is encoded as:
  amount * 6 + target * 2 + usingRightHand
  where
  -  amount ranges from 0 to 4 inclusive. If target is 1 or 2, this field can be
  left undefined.
  -  target is 0 (current player's other hand), 1 (opponent's left hand), 2
  (opponent's right hand)
  -  usingRightHand indicates which hand is the hand transferring fingers to the
  other hand or attacking. 0=left hand is, 1=right hand is

*/

/*
    Variants:
    - allowPassTransfers means that you can make transfers that are equivalent
   to passing your turn. So for example, if lefthand has 2 fingers and righthand
   has 1 finger, you can transfer 1 finger from the left hand to the right hand.
   Another example of a pass-equivalent turn is transferring 0 fingers from one
   hand to the other. By default, we don't allow this.
    - allowFullTransfers means that you can transfer all fingers from one hand
   to the other hand, as long as doing so (1) doesn't cause the other hand to
   reach 5 fingers or more and (2) isn't equivalent to a pass transfer if
   allowPassTransfers is false. So for example, if you have 3 fingers on left
   hand and 1 finger on right hand, you can transfer 1 finger from the right
   hand to the left hand, which kills the right hand. By default, we don't allow
   this.
*/
BOOLEAN allowPassTransfers = FALSE;
BOOLEAN allowFullTransfers = FALSE;

/*********** BEGIN SOLVING FUNCIONS ***********/

POSITION hashPosition(BOOLEAN isP2Turn, int p1Left, int p1Right, int p2Left,
                      int p2Right);
void unhashPosition(POSITION position, BOOLEAN *isP2Turn, int *p1Left,
                    int *p1Right, int *p2Left, int *p2Right);
MOVE hashMove(BOOLEAN usingRightHand, int target, int amount);
void unhashMove(MOVE move, BOOLEAN *usingRightHand, int *target, int *amount);
void PositionToString(POSITION position, char *positionStringBuffer);

/* TODO: Add a hashing function and unhashing function, if needed. */
POSITION hashPosition(BOOLEAN isP2Turn, int p1Left, int p1Right, int p2Left,
                      int p2Right) {
    if (p1Left > 4) p1Left = 0;
    if (p1Right > 4) p1Right = 0;
    if (p2Left > 4) p2Left = 0;
    if (p2Right > 4) p2Right = 0;
    return (isP2Turn ? 1 : 0) + (p1Left << 1) + p1Right * 10 + p2Left * 50 +
           p2Right * 250;
}

void unhashPosition(POSITION position, BOOLEAN *isP2Turn, int *p1Left,
                    int *p1Right, int *p2Left, int *p2Right) {
    *isP2Turn = position & 1;
    *p1Left = (position >> 1) % 5;
    *p1Right = (position / 10) % 5;
    *p2Left = (position / 50) % 5;
    *p2Right = position / 250;
}

MOVE hashMove(BOOLEAN usingRightHand, int target, int amount) {
    return amount * 6 + (target << 1) + (usingRightHand ? 1 : 0);
}

void unhashMove(MOVE move, BOOLEAN *usingRightHand, int *target, int *amount) {
    *usingRightHand = move & 1;
    *target = (move >> 1) % 3;
    *amount = move / 6;
}

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
    gCanonicalPosition = GetCanonicalPosition;
    gPositionToStringFunPtr = &PositionToString;
    gInitialPosition = hashPosition(FALSE, 1, 1, 1, 1);
    gSymmetries = TRUE;
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() { return hashPosition(FALSE, 1, 1, 1, 1); }

BOOLEAN isLegalTransfer(int left, int right, int i) {
    int newLeft = left + i;
    int newRight = right - i;
    return newLeft < 5 && newRight < 5 &&
           (allowPassTransfers || (newLeft != left && newLeft != right)) &&
           (allowFullTransfers || (left != -i && right != i));
}

MOVELIST *GenerateMovesHelper(POSITION position, int *numTransferMoves) {
    BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);
    int i;
    int num = 0;

    MOVELIST *moves = NULL;
    if (isP2Turn) {
        if (p2Left) {
            if (p1Left) {  // P2's left hand can attack P1's left hand
                moves = CreateMovelistNode(hashMove(FALSE, 1, 0), moves);
                // amount doesn't matter if attacking; domove will handle it
            }
            if (p1Right) {  // P2's left hand can attack P1's right hand
                moves = CreateMovelistNode(hashMove(FALSE, 2, 0), moves);
            }
        }

        if (p2Right) {
            if (p1Left) {  // P2's right hand can attack P1's left hand
                moves = CreateMovelistNode(hashMove(TRUE, 1, 0), moves);
            }
            if (p1Right) {  // P2's right hand can attack P1's right hand
                moves = CreateMovelistNode(hashMove(TRUE, 2, 0), moves);
            }
        }

        for (i = -p2Left; i <= p2Right;
             i++) {  // negative i means transferring |i| from left to right
            if (isLegalTransfer(p2Left, p2Right, i)) {
                if (i <= 0) {
                    moves = CreateMovelistNode(hashMove(FALSE, 0, -i), moves);
                } else {
                    moves = CreateMovelistNode(hashMove(TRUE, 0, i), moves);
                }
                num++;
            }
        }

    } else {
        if (p1Left) {
            if (p2Left) {  // P1's left hand can attack P2's left hand
                moves = CreateMovelistNode(hashMove(FALSE, 1, 0), moves);
            }
            if (p2Right) {  // P1's left hand can attack P2's right hand
                moves = CreateMovelistNode(hashMove(FALSE, 2, 0), moves);
            }
        }

        if (p1Right) {
            if (p2Left) {  // P1's right hand can attack P2's left hand
                moves = CreateMovelistNode(hashMove(TRUE, 1, 0), moves);
            }
            if (p2Right) {  // P1's right hand can attack P2's right hand
                moves = CreateMovelistNode(hashMove(TRUE, 2, 0), moves);
            }
        }

        for (i = -p1Left; i <= p1Right;
             i++) {  // negative i means transferring |i| from left to right
            if (isLegalTransfer(p1Left, p1Right, i)) {
                if (i <= 0) {
                    moves = CreateMovelistNode(hashMove(FALSE, 0, -i), moves);
                } else {
                    moves = CreateMovelistNode(hashMove(TRUE, 0, i), moves);
                }
                num++;
            }
        }
    }

    *numTransferMoves = num;
    return moves;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
    int numTransferMoves = 0;
    return GenerateMovesHelper(position, &numTransferMoves);
}

/* Return the position that results from making the
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
    BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);

    BOOLEAN usingRightHand;
    int target, amount;
    unhashMove(move, &usingRightHand, &target, &amount);

    /*
        NOTE: hashPosition converts any values above 4 to 0.
    */

    if (isP2Turn) {  // is player 2's turn
        if (usingRightHand) {
            if (target == 1) {  // P2's right hand attacking P1's left hand
                return hashPosition(!isP2Turn, p1Left + p2Right, p1Right,
                                    p2Left, p2Right);
            } else if (target ==
                       2) {  // P2's right hand attacking P1's right hand
                return hashPosition(!isP2Turn, p1Left, p1Right + p2Right,
                                    p2Left, p2Right);
            } else {  // P2 transferring `amount` from their right hand to left
                      // hand
                return hashPosition(!isP2Turn, p1Left, p1Right, p2Left + amount,
                                    p2Right - amount);
            }
        } else {
            if (target == 1) {  // P2's left hand attacking P1's left hand
                return hashPosition(!isP2Turn, p1Left + p2Left, p1Right, p2Left,
                                    p2Right);
            } else if (target ==
                       2) {  // P2's left hand attacking P1's right hand
                return hashPosition(!isP2Turn, p1Left, p1Right + p2Left, p2Left,
                                    p2Right);
            } else {  // P2 transferring `amount` from their left hand to right
                      // hand
                return hashPosition(!isP2Turn, p1Left, p1Right, p2Left - amount,
                                    p2Right + amount);
            }
        }
    } else {  // is player 1's turn
        if (usingRightHand) {
            if (target == 1) {  // P1's right hand attacking P2's left hand
                return hashPosition(!isP2Turn, p1Left, p1Right,
                                    p2Left + p1Right, p2Right);
            } else if (target ==
                       2) {  // P1's right hand attacking P2's right hand
                return hashPosition(!isP2Turn, p1Left, p1Right, p2Left,
                                    p2Right + p1Right);
            } else {  // P1 transferring `amount` from their right hand to left
                      // hand
                return hashPosition(!isP2Turn, p1Left + amount,
                                    p1Right - amount, p2Left, p2Right);
            }
        } else {
            if (target == 1) {  // P1's left hand attacking P2's left hand
                return hashPosition(!isP2Turn, p1Left, p1Right, p2Left + p1Left,
                                    p2Right);
            } else if (target ==
                       2) {  // P1's left hand attacking P2's right hand
                return hashPosition(!isP2Turn, p1Left, p1Right, p2Left,
                                    p2Right + p1Left);
            } else {  // P1 transferring `amount` from their left hand to right
                      // hand
                return hashPosition(!isP2Turn, p1Left - amount,
                                    p1Right + amount, p2Left, p2Right);
            }
        }
    }
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
    BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);
    if ((p1Left == 0 && p1Right == 0) || (p2Left == 0 && p2Right == 0)) {
        return lose;
    } else {
        return undecided;
    }
}

POSITION DoSymmetry(POSITION position, int symmetryID) {
    BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);
    int temp;
    if (symmetryID & 0b001) {  // swap P1's hands
        temp = p1Left;
        p1Left = p1Right;
        p1Right = temp;
    }
    if (symmetryID & 0b010) {  // swap P2's hands
        temp = p2Left;
        p2Left = p2Right;
        p2Right = temp;
    }
    if (symmetryID &
        0b100) {  // swap P1's hands for P2's hands AND switch the tuen
        temp = p1Left;
        p1Left = p2Left;
        p2Left = temp;
        temp = p1Right;
        p1Right = p2Right;
        p2Right = temp;
        isP2Turn = !isP2Turn;
    }
    return hashPosition(isP2Turn, p1Left, p1Right, p2Left, p2Right);
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
    POSITION minHash = position;
    POSITION symmetricPosition;
    for (int i = 1; i < 8; i++) {
        symmetricPosition = DoSymmetry(position, i);
        if (symmetricPosition < minHash) {
            minHash = symmetricPosition;
        }
    }
    return minHash;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);
    printf(
        "\n       R       L\n  P2  %c%c%c%c   %c%c%c%c\n                      "
        "Player %d's Turn\n                      %s\n  P1  %c%c%c%c   "
        "%c%c%c%c\n       L       R\n\n",
        p2Right > 3 ? '|' : '\'', p2Right > 2 ? '|' : '\'',
        p2Right > 1 ? '|' : '\'', p2Right > 0 ? '|' : '\'',
        p2Left > 0 ? '|' : '\'', p2Left > 1 ? '|' : '\'',
        p2Left > 2 ? '|' : '\'', p2Left > 3 ? '|' : '\'', isP2Turn ? 2 : 1,
        GetPrediction(position, playerName, usersTurn), p1Left > 3 ? '|' : ',',
        p1Left > 2 ? '|' : ',', p1Left > 1 ? '|' : ',', p1Left > 0 ? '|' : ',',
        p1Right > 0 ? '|' : ',', p1Right > 1 ? '|' : ',',
        p1Right > 2 ? '|' : ',', p1Right > 3 ? '|' : ',');
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move,
                                 STRING playerName) {
    USERINPUT ret;
    do {
        printf("%8s's move: ", playerName);
        ret = HandleDefaultTextInput(position, move, playerName);
        if (ret != Continue) return (ret);

    } while (TRUE);
    return (Continue); /* this is never reached, but lint is now happy */
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
    return ((input[0] == 'L' || input[0] == 'l' || input[0] == 'R' ||
             input[0] == 'r') &&
            (input[1] == 'L' || input[1] == 'l' || input[1] == 'R' ||
             input[1] == 'r' || (input[1] >= '0' && input[1] <= '4')));
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
    BOOLEAN usingRightHand = input[0] == 'r' || input[0] == 'R';
    int amount = 0;
    int target = 0;
    if (input[1] == 'L' || input[1] == 'l') {
        target = 1;
    } else if (input[1] == 'R' || input[1] == 'r') {
        target = 2;
    } else {
        amount = input[1] - '0';
    }
    return hashMove(usingRightHand, target, amount);
}

/* Return the string representation of the move.
Ideally this matches with what the user is supposed to
type when they specify moves. */
void MoveToString(MOVE move, char *moveStringBuffer) {
    BOOLEAN usingRightHand;
    int target, amount;
    unhashMove(move, &usingRightHand, &target, &amount);
    moveStringBuffer[0] = usingRightHand ? 'R' : 'L';
    if (target == 1) {
        moveStringBuffer[1] = 'L';
    } else if (target == 2) {
        moveStringBuffer[1] = 'R';
    } else {
        moveStringBuffer[1] = '0' + amount;
    }
    moveStringBuffer[2] = '\0';
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char moveString[10];
    MoveToString(computersMove, moveString);
    printf("%s's move: %s\n", computersName, moveString);
}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() { return 4; }

/* Return the current variant id. */
int getOption() {
    return (allowFullTransfers ? 2 : 0) | (allowPassTransfers ? 1 : 0);
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
    allowFullTransfers = option & 0b10;
    allowPassTransfers = option & 0b01;
}

void GameSpecificMenu() {
    char inp;
    while (TRUE) {
        printf("\n\n\n");
        printf(
            "        ----- Game-specific options for Chopsticks -----\n\n");
        printf("        Select an option:\n\n");
        printf("        p)      Allow Pass-Equivalent Transfers (Currently %s)\n", allowPassTransfers ? "TRUE" : "FALSE");
        printf("        f)      Allow Full Transfers (Currently %s)\n", allowFullTransfers ? "TRUE" : "FALSE");
        printf("        b)      Back to previous menu\n\n");
        printf("\nSelect an option: ");
        inp = GetMyChar();
        if (inp == 'p' || inp == 'P') {
            setOption(getOption() ^ 0b01);
        } else if (inp == 'f' || inp == 'F') {
            setOption(getOption() ^ 0b10);
        } else if (inp == 'b' || inp == 'B') {
            return;
        } else if (inp == 'q' || inp == 'Q') {
            exit(0);
        } else {
            printf("Invalid input.\n");
        }
    }
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

void PositionToString(POSITION position, char *positionStringBuffer) {
    BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);
    snprintf(positionStringBuffer, 10, "%c_%d%d%d%d", isP2Turn ? '2' : '1', p1Left, p1Right, p2Left, p2Right);
}

POSITION StringToPosition(char *positionString) {
    for (int i = 2; i < 6; i++) {
        if (positionString[i] < '0' || positionString[i] > '4') {
            return NULL_POSITION;
        }
    }
    return hashPosition(
        positionString[0] == '2', 
        positionString[2] - '0', positionString[3] - '0', 
        positionString[4] - '0', positionString[5] - '0'
    );
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	BOOLEAN isP2Turn;
    int p1Left, p1Right, p2Left, p2Right;
    unhashPosition(position, &isP2Turn, &p1Left, &p1Right, &p2Left, &p2Right);
    char fingers[17];
    memset(fingers, '-', 16);
    fingers[16] = '\0';
    int i;
    for (i = 0; i < p1Left; i++) {
        fingers[i] = 'f';
    }
    for (i = 4; i < 4 + p1Right; i++) {
        fingers[i] = 'f';
    }
    for (i = 8; i < 8 + p2Left; i++) {
        fingers[i] = 'f';
    }
    for (i = 12; i < 12 + p2Right; i++) {
        fingers[i] = 'f';
    }
    AutoGUIMakePositionString(isP2Turn ? 2 : 1, fingers, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    BOOLEAN isP2Turn = position & 1;
    BOOLEAN usingRightHand;
    int target, amount;
    unhashMove(move, &usingRightHand, &target, &amount);

    if (target == 0) { // is transfer move
        int offset = 24;
        int numTransferMoves;
        MOVELIST *moves = GenerateMovesHelper(position, &numTransferMoves);
        MOVELIST *head = moves;

        int idx = 0;
        while(moves != NULL) {
            if (moves->move == move || idx > 4) {
                break;
            }
            idx++;
            moves = moves->next;
        }

        if (isP2Turn) { offset += 9; }
        if (!(numTransferMoves & 1)) { offset += 5; }
        offset += (5 - (numTransferMoves | 1)) >> 1;
    
        FreeMoveList(head);
        char token = '0';
        if (((!isP2Turn && usingRightHand) || (isP2Turn && !usingRightHand)) && amount != 0) {
            token = '0' + 10 - amount;
        } else {
            token = '0' + amount;
        }
        AutoGUIMakeMoveButtonStringA(token, offset + idx, 'x', autoguiMoveStringBuffer);
    } else { // is attack move
        int fromCenter, toCenter;
        int offset = 16;

        if (usingRightHand) {
            fromCenter = isP2Turn ? 3 : 1;
            if (target == 2) {
                offset = 20;
            }
        } else {
            fromCenter = isP2Turn ? 2 : 0;
            if (target == 1) {
                offset = 20;
            }
        }

        if (target == 1) {
            toCenter = isP2Turn ? 0 : 2;
        } else {
            toCenter = isP2Turn ? 1 : 3;
        }
        AutoGUIMakeMoveButtonStringM(offset + fromCenter, offset + toCenter, 'x', autoguiMoveStringBuffer);
    }
}