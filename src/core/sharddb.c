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
#define MAX_C4_SHARD_SIZE 52428800 // All un-gzipped shards are less than 50 MiB.

/*internal declarations and definitions*/

void            sharddb_free                     ();

/* Value */
VALUE           sharddb_get_value                (POSITION pos);

/* Remoteness */
REMOTENESS      sharddb_get_remoteness           (POSITION pos);

/* Visited */

/* Mex */

/* saving to/reading from a file */
BOOLEAN         sharddb_save_database            ();
BOOLEAN         sharddb_load_database            ();

/* LUR Cache */
typedef struct elem {
	POSITION p;
	VALUE v;
	REMOTENESS r;
	struct elem *d_prev;
	struct elem *d_next;
	struct elem *s_next;
} elem_t;

typedef struct elem_disk {
	POSITION p;
	VALUE v;
	REMOTENESS r;
} elem_disk_t;

static BOOLEAN sharddb_cache_load_from_disk(void);
static BOOLEAN sharddb_cache_dump_to_disk(void);
static BOOLEAN sharddb_cache_table_remove(elem_t *e);
static void sharddb_cache_put(POSITION p, VALUE v, REMOTENESS r);
static void sharddb_cache_get(VALUE *v, REMOTENESS *r, POSITION p);

/*
** Code
*/

void sharddb_init(DB_Table *new_db) {
	new_db->put_value = NULL;
	new_db->put_remoteness = NULL;
	new_db->mark_visited = NULL;
	new_db->unmark_visited = NULL;
	new_db->put_mex = NULL;
	new_db->free_db = sharddb_free;

	new_db->get_value = sharddb_get_value;
	new_db->get_remoteness = sharddb_get_remoteness;
	new_db->check_visited = NULL;
	new_db->get_mex = NULL;
	new_db->save_database = sharddb_save_database;
	new_db->load_database = sharddb_load_database;
}

void sharddb_free() {
	return;
}

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

VALUE sharddb_get_value(POSITION pos) {
	VALUE value;
	REMOTENESS remoteness;
	sharddb_cache_get(&value, &remoteness, pos);
	return value;
}

REMOTENESS sharddb_get_remoteness(POSITION pos)
{
	VALUE value;
	REMOTENESS remoteness;
	sharddb_cache_get(&value, &remoteness, pos);
	return remoteness;
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
	(void)position;
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

		sharddb_cache_get(&value, &remoteness, choice);
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

/* LRU Cache */
static char CACHE_FILENAME[100];
static unsigned long long CACHE_SIZE; 	// In bytes.
static const double ALPHA = 0.75;		// Hash table target load factor
static unsigned long long NUM_BUCKETS;
static unsigned long long MAX_ELEMENTS;

static elem_t **hash_table = NULL;
static elem_t *head = NULL;
static elem_t *tail = NULL;
static unsigned long long cache_size = 0;

/* https://www.geeksforgeeks.org/program-to-find-the-next-prime-number/ */
static BOOLEAN is_prime(unsigned long long n) {
    if (n <= 1) return FALSE;
    if (n <= 3) return TRUE;
    if (n%2 == 0 || n%3 == 0) return FALSE;
	unsigned long long i;
    for (i = 5; i*i <= n; i += 6) {
        if (n%i == 0 || n%(i + 2) == 0) {
           return FALSE;
		}
	}
    return TRUE;
}

static unsigned long long prev_prime(unsigned long long n) {
	/* Returns the largest prime number that is smaller than
	   or equal to N, unless N is less than 2, in which case
	   2 is returned. */
	if (n < 2) return 2;
	while (!is_prime(n)) --n;
	return n;
}

void sharddb_cache_init(void) {
	if (hash_table) return;
	int opt = getOption();
	snprintf(CACHE_FILENAME, 100, "./data/mconnect4_%d_sharddb/lru.bin", opt);
	CACHE_SIZE = (opt == 1) ? (1ULL << 25) : (1ULL << 27); // 32 MiB for 6x6, 128 MiB for 6x7.
	NUM_BUCKETS = prev_prime(CACHE_SIZE/(sizeof(elem_t)*ALPHA + sizeof(elem_t*)));
	MAX_ELEMENTS = NUM_BUCKETS * ALPHA;
	hash_table = SafeCalloc(NUM_BUCKETS, sizeof(elem_t*));
	head = SafeCalloc(1, sizeof(elem_t));
	tail = SafeCalloc(1, sizeof(elem_t));
	head->d_prev = NULL;
	head->d_next = tail;
	tail->d_prev = head;
	tail->d_next = NULL;
	cache_size = 0;
	if (!sharddb_cache_load_from_disk()) {
		printf("sharddb_cache_init: load cache from disk failed.");
	}
}

void sharddb_cache_deallocate(void) {
	if (!hash_table) return;
	if (!sharddb_cache_dump_to_disk()) {
		printf("sharddb_cache_deallocate: cache dump failed.");
	}
	/* Deallocate all variables on heap. */
	SafeFree(hash_table);
	hash_table = NULL;
	elem_t *walker = head, *tmp;
	while (walker) {
		tmp = walker;
		walker = walker->d_next;
		SafeFree(tmp);
	}
	head = tail = NULL;
}

static BOOLEAN sharddb_cache_load_from_disk(void) {
	FILE *f = fopen(CACHE_FILENAME, "rb");
	if (!f) return FALSE;
	elem_disk_t e;
	while (fread(&e, sizeof(elem_disk_t), 1, f)) {
		sharddb_cache_put(e.p, e.v, e.r);
	}
	fclose(f);
	return TRUE;
}

static BOOLEAN sharddb_cache_dump_to_disk(void) {
	FILE *f = fopen(CACHE_FILENAME, "wb");
	if (!f) return FALSE;
	/* Write in reverse chronological order so that old elements are loaded first. */
	elem_t *walker = tail->d_prev;
	while (walker != head) {
		fwrite(walker, sizeof(elem_disk_t), 1, f);
		walker = walker->d_prev;
	}
	fclose(f);
	return TRUE;
}

static BOOLEAN sharddb_cache_table_remove(elem_t *e) {
	/* Look for existing element in table. */
	unsigned long long slot = e->p % NUM_BUCKETS;
	elem_t **walker = &hash_table[slot];
	while (*walker) {
		if ((*walker)->p == e->p) {
			/* Found, remove it from table. */
			*walker = (*walker)->s_next;
			return TRUE;
		}
		walker = &((*walker)->s_next);
	}
	return FALSE;
}

static void sharddb_cache_put(POSITION p, VALUE v, REMOTENESS r) {
	/* Look for existing element in table. */
	unsigned long long slot = p % NUM_BUCKETS;
	elem_t *walker = hash_table[slot];
	while (walker) {
		if (walker->p == p) {
			/* This should never happen. */
			printf("sharddb_cache_put: inserting existing element.");
			return;
		}
		walker = walker->s_next;
	}
	elem_t *e;
	if (cache_size == MAX_ELEMENTS) {
		/* Evict least recently used element from cache. */
		e = tail->d_prev;
		tail->d_prev = e->d_prev;
		e->d_prev->d_next = tail;
		if (!sharddb_cache_table_remove(e)) {
			/* This should never happen. */
			printf("sharddb_cache_put: failed to find existing element in hash table.");
			return;
		}
	} else {
		/* Cache is not full, create a new element and add it. */
		e = SafeCalloc(1, sizeof(elem_t));
		++cache_size;
	}
	/* Put new values inside. */
	e->p = p;
	e->v = v;
	e->r = r;
	/* Insert as new head of linked list. */
	e->d_prev = head;
	e->d_next = head->d_next;
	head->d_next = e;
	e->d_next->d_prev = e;
	/* Insert into hash table at SLOT. */
	e->s_next = hash_table[slot];
	hash_table[slot] = e;
}

static void sharddb_cache_get(VALUE *v, REMOTENESS *r, POSITION p) {
	/* Look inside cache first. */
	unsigned long long slot = p % NUM_BUCKETS;
	elem_t *walker = hash_table[slot];
	while (walker) {
		if (walker->p == p) {
			/* Cache hit, read from cache and bring element to head. */
			*v = walker->v;
			*r = walker->r;
			walker->d_next->d_prev = walker->d_prev;
			walker->d_prev->d_next = walker->d_next;
			walker->d_prev = head;
			walker->d_next = head->d_next;
			head->d_next = walker;
			walker->d_next->d_prev = walker;
			return;
		}
		walker = walker->s_next;
	}
	/* Cache miss, read from disk and put in cache. */
	unsigned long long key = p & 0xFFFFFFFFFFFFF;
	gzFile file = NULL;
	int shardId, leading3digits;
	getShardIDAndLeading3Digits(&shardId, &leading3digits, key);
	key &= ((1ULL << 28) - 1);
	char filename[100];
	snprintf(filename, 100, "./data/mconnect4_%d_sharddb/%d/solved-%d.gz", getOption(), leading3digits, shardId);
	file = gzopen(filename, "rb");
    if (!file) {
        return;
    }
	char *gzbuffer = (char *) calloc(MAX_C4_SHARD_SIZE, 1);
	gzread(file, gzbuffer, MAX_C4_SHARD_SIZE);
	gzclose(file);
	char size = 28;
	POSITION i = 0;
	size = gzbuffer[i++];
    char res = initializesegment(0, gzbuffer, size, key, &i);
	SafeFree(gzbuffer);
	getValueRemotenessFromByte(v, r, res);
	sharddb_cache_put(p, *v, *r);
}
