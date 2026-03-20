/************************************************************************
**
** NAME:	blobdb.c
**
** DESCRIPTION:	Accessor functions for blob solver.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2026-01-12
**
**************************************************************************/

#include "gamesman.h"
#include "interact.h"
#include "blobdb.h"

void            blobdb_free                     ();

VALUE           blobdb_get_value                (POSITION pos);
REMOTENESS      blobdb_get_remoteness           (POSITION pos);

BOOLEAN         blobdb_save_database            ();
BOOLEAN         blobdb_load_database            ();

void blobdb_init(DB_Table *new_db) {

	new_db->put_value = NULL;
	new_db->put_remoteness = NULL;
	new_db->mark_visited = NULL;
	new_db->unmark_visited = NULL;
	new_db->put_mex = NULL;
	new_db->free_db = blobdb_free;

	new_db->get_value = NULL;
	new_db->get_remoteness = NULL;
	new_db->check_visited = NULL;
	new_db->get_mex = NULL;
	new_db->save_database = blobdb_save_database;
	new_db->load_database = blobdb_load_database;
}

void blobdb_free() {
	return;
}

BOOLEAN blobdb_save_database () {
	return FALSE;
}

BOOLEAN blobdb_load_database() {
	return TRUE;
}


/* We need to print out the response in JSON format for server interaction. */
void blobDetailedPositionResponse(STRING board, char *positionStringBuffer) {

    // Begin the JSON format with result =>> to prompt the server
    printf("result =>> {");

    int turn;
    char *currBoard;
	if (!ParseStandardOnelinePositionString(board, &turn, &currBoard)) {
		printf("}");
        return;
	}

    int boardLength = strlen(currBoard);
    char boardArr[boardLength + 1];
    memcpy(boardArr, currBoard, sizeof(char)*boardLength);

    boardArr[boardLength] = '\0';

    int remoteness;
    VALUE value;
    FILE *f;
    char filename[256];
    POSITION gameBoard;

    // Change this later, we need a function that changes the board encoding to a POSITION. We can define this in the m___ file.
    gameBoard = StringToPosition(board);

    GetBlobFileNameFromPosition(gameBoard, filename);
    f = fopen(filename, "rb");
    UINT64 information = GetInfoFromBlobFile(gameBoard, f);
    value = GetPrimitiveFromInfo(information);
    remoteness = GetRemotenessFromInfo(information);

    printf("\"position\":\"%s\",\"autoguiPosition\":\"%s\",", board, board);
	printf("\"remoteness\":%d,", remoteness);
    printf("\"positionValue\":\"%s\",", value);
	printf("\"moves\":[");

    AutoGUIMakePositionString(turn, boardArr, positionStringBuffer);

    MOVELIST *moves = GenerateMoves(gameBoard);
    MOVELIST *moveHead = moves;
    char autoguiMoveStringBuffer[64];

    while (moveHead) {
        POSITION newBoard = DoMove(gameBoard, moveHead->move);

        GetBlobFileNameFromPosition(newBoard, filename);
        f = fopen(filename, "rb");
        UINT64 information = GetInfoFromBlobFile(newBoard, f);
        value = GetPrimitiveFromInfo(information);
        remoteness = GetRemotenessFromInfo(information);

        MoveToAutoGUIString(gameBoard, moveHead->move, autoguiMoveStringBuffer);
        printf("{\"position\":\"%s\",", positionStringBuffer);
        printf("\"autoguiPosition\":\"%s\",", positionStringBuffer);
        printf("\"remoteness\":%d,", remoteness);
        printf("\"positionValue\":\"%s\",", value);
        printf("\"autoguiMove\":\"A_-_%d\",", autoguiMoveStringBuffer);
        printf("\"move\":\"%llu\"}", moveHead->move);
        moveHead = moveHead->next;
    }

    printf("]}");
    fclose(f);
}