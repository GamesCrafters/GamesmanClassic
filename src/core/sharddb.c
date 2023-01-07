/************************************************************************
**
** NAME:	sharddb.c
**
** DESCRIPTION:	Accessor functions for shard-solved Connect 4.
**              WORKS FOR 6x6 and 6x7 CONNECT 4 ONLY AS OF 11/07/2022.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2022-10-30
**
**************************************************************************/

#include <zlib.h>
#include <sys/stat.h>
#include "gamesman.h"
#include <dirent.h>
#include "interact.h"
#include "sharddb.h"
#define RESULT "result =>> "

/*internal declarations and definitions*/

void            sharddb_free                     ();

/* Value */
VALUE           sharddb_get_value                (POSITION pos);
VALUE           sharddb_set_value                (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS      sharddb_get_remoteness           (POSITION pos);
void            sharddb_set_remoteness           (POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN         sharddb_check_visited            (POSITION pos);
void            sharddb_mark_visited             (POSITION pos);
void            sharddb_unmark_visited           (POSITION pos);

/* Mex */
MEX             sharddb_get_mex                  (POSITION pos);
void            sharddb_set_mex                  (POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN         sharddb_save_database            ();
BOOLEAN         sharddb_load_database            ();

/*
** Code
*/

void sharddb_init(DB_Table *new_db) {
	new_db->put_value = sharddb_set_value;
	new_db->put_remoteness = sharddb_set_remoteness;
	new_db->mark_visited = sharddb_mark_visited;
	new_db->unmark_visited = sharddb_unmark_visited;
	new_db->put_mex = sharddb_set_mex;
	new_db->free_db = sharddb_free;

	new_db->get_value = sharddb_get_value;
	new_db->get_remoteness = sharddb_get_remoteness;
	new_db->check_visited = sharddb_check_visited;
	new_db->get_mex = sharddb_get_mex;
	new_db->save_database = sharddb_save_database;
	new_db->load_database = sharddb_load_database;
}

void sharddb_free() {
	return;
}


VALUE sharddb_set_value(POSITION pos, VALUE val) {
	return 0;
}

//char initializesegment(POSITION offset, FILE *file, int size, POSITION key) {
char initializesegment(POSITION offset, char *gzbuffer, int size, POSITION key, POSITION *gzOffset) {
    int64_t ptr = 0;
	ptr = gzbuffer[(*gzOffset)++];
    if (ptr == 0) {
        char c;
		c = gzbuffer[(*gzOffset)++];
		if (key >= offset && key < offset + (1l<<size)) {
			return c;
		} else {
			return 0;
		}
    } else if (ptr == 1) {
		POSITION savedOffset = (*gzOffset);
		char res = initializesegment(offset, gzbuffer, size-1, key, gzOffset);
		if (res) {
			return res;
		} else {
			(*gzOffset) = savedOffset;
			return initializesegment(offset+(1l<<(size-1)), gzbuffer, size-1, key, gzOffset);
		}
    } else {
        char res = initializesegment(offset, gzbuffer, size-1, key, gzOffset);
		if (res) {
			return res;	
		} else {
			return initializesegment(offset+(1l<<(size-1)), gzbuffer, size-1, key, gzOffset);
		}
    }
}

void getValueRemotenessFromByte(VALUE *value, REMOTENESS *remoteness, unsigned char res) {
	if (res > 0 && res < 64) {
		(*value) = lose;
		(*remoteness) = res - 1;
	} else if (res > 127 && res < 192) {
		(*value) = tie;
		(*remoteness) = res - 128;
	} else if (res > 191) {
		(*value) = win;
		(*remoteness) = 255 - res;
	} else {
		(*value) = tie;
		(*remoteness) = 255;
	}
}

void getShardIDAndLeading3Digits(int *shardId, int *leading3digits, POSITION key) {
	int d = key >> 28;
	(*shardId) = d;
	while (d >= 1000) d /= 10;
	*leading3digits = d;
}

void sharddb_get_valueremoteness(VALUE *value, REMOTENESS *remoteness, POSITION key) {
	key &= 0xFFFFFFFFFFFFF;
	gzFile file = NULL;
	int shardId, leading3digits;
	getShardIDAndLeading3Digits(&shardId, &leading3digits, key);
	key &= ((1ULL << 28) - 1);
	
	char filename[100];
	snprintf(filename, 100, "./data/mconnect4_%d_sharddb/%d/solved-%d.gz", getOption(), leading3digits, shardId);
	/*if (getOption() == 2) {
		snprintf(filename, 100, "/data/solved/connect4_v2/solved_76/%d/solved-%d.gz", leading3digits, shardId);
	} else {
		snprintf(filename, 100, "/data/solved/connect4_v2/solved_66/%d/solved-%d.gz", leading3digits, shardId);
	}*/

	file = gzopen(filename, "rb");
    if (!file) {
        return;
    }
	char *gzbuffer = (char *) calloc(52428800, 1);
	gzread(file, gzbuffer, 52428800);
	gzclose(file);
	char size = 28;
	POSITION i = 0;
	size = gzbuffer[i++];
    char res = initializesegment(0, gzbuffer, size, key, &i);

	SafeFree(gzbuffer);

	getValueRemotenessFromByte(value, remoteness, res);
}

VALUE sharddb_get_value(POSITION pos) {
	VALUE value;
	REMOTENESS remoteness;
	sharddb_get_valueremoteness(&value, &remoteness, pos);
	return value;
}

REMOTENESS sharddb_get_remoteness(POSITION pos)
{
	VALUE value;
	REMOTENESS remoteness;
	sharddb_get_valueremoteness(&value, &remoteness, pos);
	return remoteness;
}

void sharddb_set_remoteness (POSITION pos, REMOTENESS val) {
	return;
}

BOOLEAN sharddb_check_visited(POSITION pos) {
	return FALSE;
}


void sharddb_mark_visited (POSITION pos) {
	return;
}

void sharddb_unmark_visited (POSITION pos) {
	return;
}

void sharddb_set_mex(POSITION pos, MEX mex) {
	return;
}

MEX sharddb_get_mex(POSITION pos) {
	return 0;
}

BOOLEAN sharddb_save_database () {
	return FALSE;
}

BOOLEAN sharddb_load_database() {
	return TRUE;
}

int IROWCOUNT = 6;
int ICOLUMNCOUNT = 6;

MOVELIST *IGenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  for (int i = 0; i < ICOLUMNCOUNT; i++) {
    MOVE start = (IROWCOUNT + 1) * (i + 1) - 1;
    while ((position & (1ULL<<start)) == 0) start--; // This is what sigbit does
    if (start != (IROWCOUNT + 1) * (i + 1) - 1) { // If there is space in this column
      moves = CreateMovelistNode(start, moves);
    }
  }
  return moves;
}

POSITION IInteractStringToPosition(STRING str) {
  enum UWAPI_Turn turn;
  unsigned int num_rows, num_columns; // Unused
  STRING board;
  if (!UWAPI_Board_Regular2D_ParsePositionString(str, &turn, &num_rows, &num_columns, &board)) {
	// Failed to parse string
	return INVALID_POSITION;
	}

  POSITION pos = 0;
  for (int c = 0; c < ICOLUMNCOUNT; c++) {
    POSITION colbits = 0;
    BOOLEAN endFound = FALSE;
    for (int r = 0; !endFound && r < IROWCOUNT; r++) {
      char piece = board[IROWCOUNT * (c + 1) - 1 - r];
      if (piece == 'O') {
        colbits |= (1 << r);
      } else if (piece == '-') {
        endFound = TRUE;
        colbits |= (1 << r);
      }
    }
    if (!endFound) colbits |= (1 << IROWCOUNT);
    pos |= (colbits << ((IROWCOUNT + 1) * (ICOLUMNCOUNT - 1 - c)));
  }
  if (turn == UWAPI_TURN_B) pos |= (1ULL << 63);
  SafeFree(board);
  return pos;
}

STRING IInteractPositionToString(POSITION position) {
  char pieces[(IROWCOUNT + 1) * ICOLUMNCOUNT + 1];
  int piecesPlaced = 0;
  int k = 0;
  BOOLEAN pastSigBit = FALSE;

  for (int i = (IROWCOUNT + 1) * ICOLUMNCOUNT - 1; i >= 0; i--) {
    BOOLEAN bit = (position >> i) & 1;
    if (i % (IROWCOUNT + 1) == IROWCOUNT) {
      pastSigBit = bit;
    } else if (bit) {
      if (pastSigBit) {
        pieces[k] = 'O';
        piecesPlaced++;
      } else {
        pieces[k] = '-';
        pastSigBit = TRUE;
      }
      k++;
    } else {
      if (pastSigBit) {
        pieces[k] = 'X';
        piecesPlaced++;
      } else {
        pieces[k] = '-';
      }
      k++;
    }
  }
  for (int i = (IROWCOUNT * ICOLUMNCOUNT); i < (IROWCOUNT + 1) * ICOLUMNCOUNT; i++) {
    pieces[i] = '-';
  }

  pieces[(IROWCOUNT + 1) * ICOLUMNCOUNT] = '\0';
  enum UWAPI_Turn turn = (piecesPlaced & 1) ? UWAPI_TURN_B : UWAPI_TURN_A; 
  return UWAPI_Board_Regular2D_MakePositionString(turn, IROWCOUNT + 1, ICOLUMNCOUNT, pieces);
}

STRING IInteractMoveToString(POSITION position, MOVE move) {
  return UWAPI_Board_Regular2D_MakeAddString('a', (IROWCOUNT + 1) * ICOLUMNCOUNT - 1 - (move / (IROWCOUNT + 1)));
}

STRING ValueCharToValueString(char value_char) {
	switch(value_char) {
	case 'T':
		return "tie";
	case 'W':
		return "win";
	case 'L':
		return "lose";
	case 'U':
		return "undecided";
	default:
		return "error";
	}
}

void shardGamesmanDetailedPositionResponse(STRING board, POSITION pos) {
	
	printf(RESULT "{\"status\":\"ok\",\"response\":{");
	if (getOption() == 2) {
		ICOLUMNCOUNT = 7;
	} else {
		ICOLUMNCOUNT = 6;
	}
	VALUE value;
	REMOTENESS remoteness;
	printf("\"board\": \"%s\",", board);
	printf("\"remoteness\":%d,", Remoteness(pos));
	InteractPrintJSONPositionValue(pos);
	printf(",\"moves\":[");

	MOVELIST *all_next_moves = IGenerateMoves(pos);
	MOVELIST *current_move = all_next_moves;
	POSITIONLIST *nextPositions = NULL;
	STRING move_string = NULL;
	while (current_move) {
		POSITION choice = DoMove(pos, current_move->move);
		nextPositions = StorePositionInList(choice, nextPositions);
		STRING nextBoard = IInteractPositionToString(choice);
		printf("{\"board\":\"%s\",", nextBoard);
		SafeFree(nextBoard);

		sharddb_get_valueremoteness(&value, &remoteness, choice);
		printf("\"remoteness\":%d,", remoteness);
		char value_char = gValueLetter[value];
		printf("\"value\":\"%s\"", ValueCharToValueString(value_char));
		move_string = IInteractMoveToString(pos, current_move->move);
					
		printf(",\"move\":\"%s\"", move_string);
		SafeFree(move_string);

		move_string = gMoveToStringFunPtr(current_move->move);
		printf(",\"moveName\":\"%s\"", move_string);
		SafeFree(move_string);

		current_move = current_move->next;
		printf("}");
		if (current_move) {
			printf(",");
		}
	}
	move_string = NULL;
	FreeMoveList(all_next_moves);
	FreePositionList(nextPositions);

	printf("]}}");
}