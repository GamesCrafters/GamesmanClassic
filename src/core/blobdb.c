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

#include <zlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "autoguistrings.h"
#include <dirent.h>
#include "gamesman.h"
#include "interact.h"
#include "blobdb.h"

void            blobdb_free                     ();

VALUE           blobdb_get_value                (POSITION pos);
REMOTENESS      blobdb_get_remoteness           (POSITION pos);

BOOLEAN         blobdb_save_database            ();
BOOLEAN         blobdb_load_database            ();

POSITION        flip_pos                        (POSITION pos);

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

POSITION flip_pos(POSITION pos) {
    POSITION new_pos;
    new_pos.player = pos.opponent;
    new_pos.opponent = pos.player;
    return new_pos;
}

typedef struct {
    FILE *file;
    void *buffer;
} GzFileHandle;

static GzFileHandle open_gzip_as_file(const char *filename) {
    GzFileHandle handle;
    handle.file = NULL;
    handle.buffer = NULL;

    gzFile gz = gzopen(filename, "rb");
    if (!gz) {
        printf("Error: gzopen failed for %s\n", filename);
        return handle;
    }

    size_t capacity = 1024 * 1024;
    size_t size = 0;

    unsigned char *buffer = (unsigned char *)malloc(capacity);
    if (!buffer) {
        gzclose(gz);
        return handle;
    }

    while (1) {
        if (size == capacity) {
            capacity *= 2;
            unsigned char *new_buffer = (unsigned char *)realloc(buffer, capacity);
            if (!new_buffer) {
                free(buffer);
                gzclose(gz);
                return handle;
            }
            buffer = new_buffer;
        }

        int bytes_read = gzread(
            gz,
            buffer + size,
            (unsigned int)(capacity - size)
        );

        if (bytes_read < 0) {
            int errnum = 0;
            const char *err = gzerror(gz, &errnum);
            printf("Error: gzread failed for %s: %s\n", filename, err);
            free(buffer);
            gzclose(gz);
            return handle;
        }

        if (bytes_read == 0) {
            break;
        }

        size += (size_t)bytes_read;
    }

    gzclose(gz);

    FILE *f = fmemopen(buffer, size, "rb");
    if (!f) {
        printf("Error: fmemopen failed for %s\n", filename);
        free(buffer);
        return handle;
    }

    handle.file = f;
    handle.buffer = buffer;
    return handle;
}

static void close_gzip_file_handle(GzFileHandle *handle) {
    if (handle->file) {
        fclose(handle->file);
        handle->file = NULL;
    }

    if (handle->buffer) {
        free(handle->buffer);
        handle->buffer = NULL;
    }
}

/* We need to print out the response in JSON format for server interaction. */
void blobDetailedPositionResponse(STRING board, char *positionStringBuffer) {

    // Begin the JSON format with result =>> to prompt the server
    printf("result =>> {");

    int turn;
    char *currBoard;
    // Check to see if the position is valid or not
	if (!ParseStandardOnelinePositionString(board, &turn, &currBoard)) {
		printf("}");
        return;
	}

    int remoteness;
    STRING value;
    FILE *f;
    GzFileHandle data_handle;
    char filename[256];
    POSITION gameBoard;

    // Takes in board position string, not autogui string
    gameBoard = StringToPosition(board);

    // Generates moves for current position
    MOVELIST *moves = GenerateMoves(gameBoard);
    MOVELIST *moveHead = moves;
    char autoguiMoveStringBuffer[64];
    int new_turn = (turn % 2) + 1;

    GetBlobFileNameFromPosition(gameBoard, filename);
    data_handle = open_gzip_as_file(filename);
    f = data_handle.file;

    if (!f) {
        printf("Blob gz file can't be opened.");
        return;
    }
    UINT64 information = GetInfoFromBlobFile(gameBoard, f);
    value = GetPrimitiveFromInfo(information);
    remoteness = GetRemotenessFromInfo(information);

    char autoguiBoardArr[MAX_POSITION_STRING_LENGTH];
    autoguiBoardArr[MAX_POSITION_STRING_LENGTH - 1] = '\0';

    char positionString[MAX_POSITION_STRING_LENGTH];
    positionString[MAX_POSITION_STRING_LENGTH - 1] = '\0';

    char moveString[MAX_MOVE_STRING_LENGTH];
    moveString[MAX_MOVE_STRING_LENGTH - 1] = '\0';

    if (turn == 2) {
        PositionToAutoGUIString(flip_pos(gameBoard), autoguiBoardArr);
        BlobPositionToString(flip_pos(gameBoard), positionString);
    } else {
        PositionToAutoGUIString(gameBoard, autoguiBoardArr);
        BlobPositionToString(gameBoard, positionString);
    }

    AutoGUIMakePositionString(turn, positionString, positionStringBuffer);
    printf("\"position\":\"%s\",", positionStringBuffer);

    AutoGUIMakePositionString(turn, autoguiBoardArr, positionStringBuffer);
    printf("\"autoguiPosition\":\"%s\",", positionStringBuffer);

	printf("\"remoteness\":%d,", remoteness);
    printf("\"positionValue\":\"%s\",", value);
	printf("\"moves\":[");
    POSITION newBoard;
    if (moveHead == NULL && remoteness != 0) {
        newBoard = DoMove(gameBoard, ~0ULL);
        if (new_turn == 2) {
            PositionToAutoGUIString(flip_pos(newBoard), autoguiBoardArr);
            BlobPositionToString(flip_pos(newBoard), positionString);
        } else {
            PositionToAutoGUIString(newBoard, autoguiBoardArr);
            BlobPositionToString(newBoard, positionString);
        }

        GetBlobFileNameFromPosition(newBoard, filename);
        close_gzip_file_handle(&data_handle);

        data_handle = open_gzip_as_file(filename);
        f = data_handle.file;

        if (!f) {
            printf("Blob gz file can't be opened.");
            return;
        }

        information = GetInfoFromBlobFile(newBoard, f);
        value = GetPrimitiveFromInfo(information);
        remoteness = GetRemotenessFromInfo(information);

        MoveToAutoGUIString(gameBoard, ~0ULL, autoguiMoveStringBuffer);

        AutoGUIMakePositionString(new_turn, positionString, positionStringBuffer);
        printf("{\"position\":\"%s\",", positionStringBuffer);

        AutoGUIMakePositionString(new_turn, autoguiBoardArr, positionStringBuffer);
        printf("\"autoguiPosition\":\"%s\",", positionStringBuffer);

        printf("\"remoteness\":%d,", remoteness);
        printf("\"positionValue\":\"%s\",", value);
        printf("\"autoguiMove\":\"%s\",", autoguiMoveStringBuffer);
        MoveToString(~0ULL, moveString);
        printf("\"move\":\"%s\"}", moveString);
        printf("]}");
        close_gzip_file_handle(&data_handle);
        return;
    }
    
    while (moveHead) {
        newBoard = DoMove(gameBoard, moveHead->move);
        if (new_turn == 2) {
            PositionToAutoGUIString(flip_pos(newBoard), autoguiBoardArr);
            BlobPositionToString(flip_pos(newBoard), positionString);
        } else {
            PositionToAutoGUIString(newBoard, autoguiBoardArr);
            BlobPositionToString(newBoard, positionString);
        }

        GetBlobFileNameFromPosition(newBoard, filename);
        close_gzip_file_handle(&data_handle);

        data_handle = open_gzip_as_file(filename);
        f = data_handle.file;

        if (!f) {
            printf("Blob gz file can't be opened.");
            return;
        }

        information = GetInfoFromBlobFile(newBoard, f);
        value = GetPrimitiveFromInfo(information);
        remoteness = GetRemotenessFromInfo(information);

        MoveToAutoGUIString(gameBoard, moveHead->move, autoguiMoveStringBuffer);

        AutoGUIMakePositionString(new_turn, positionString, positionStringBuffer);
        printf("{\"position\":\"%s\",", positionStringBuffer);

        AutoGUIMakePositionString(new_turn, autoguiBoardArr, positionStringBuffer);
        printf("\"autoguiPosition\":\"%s\",", positionStringBuffer);

        printf("\"remoteness\":%d,", remoteness);
        printf("\"positionValue\":\"%s\",", value);
        printf("\"autoguiMove\":\"%s\",", autoguiMoveStringBuffer);
        MoveToString(moveHead->move, moveString);
        printf("\"move\":\"%s\"}", moveString);
        moveHead = moveHead->next;
        if (moveHead) {
            printf(",");
        }
    }

    printf("]}");
    close_gzip_file_handle(&data_handle);
}
