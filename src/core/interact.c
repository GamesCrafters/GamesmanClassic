#include "interact.h"
#include "hashwindow.h"
#include <stdarg.h>

/* In case strdup isn't defined. */
char * StringDup( char * s ) {
	char * a = (char *)SafeMalloc(strlen(s) + 1);
	/* 1 is for null character. */
	if (a) {
		strcpy(a, s);
	}
	return a;
}

void SafeFreeString(char * string) {
	SafeFree((void *) string);
}

char * StrFromI( long long i ) {
	char * str = (char *) SafeMalloc( 22 ); /* 20 for 64 bit number + sign + null character. */
	if ( str ) {
		sprintf( str, "%lld", i);
	}
	return str;
}

char * StrFormat(size_t max_size, char * format_str, ...) {
	va_list args;
	char * str = (char *) SafeMalloc( max_size + 1 );
	if (str) {
		va_start(args, format_str);
		vsnprintf(str, max_size + 1, format_str, args);
	}
	va_end(args);
	return str;
}

STRING MoveToString(MOVE mv);
STRING PositionToString(POSITION pos);
POSITION StringToPosition(STRING str);

static char * AllocVa(va_list lst, size_t accum, size_t * total) {
	char * key = va_arg(lst, char *);
	char * val;
	char * out;
	size_t self_size;
	size_t key_size;
	size_t val_size;
	size_t n;
	if (!key) {
		return NULL;
	}
	if ( *key ) {
		/* The key is not the empty string. */
		val = va_arg(lst, char *);
		if (!val) {
			return NULL;
		}
		key_size = strlen(key);
		val_size = strlen(val);
		self_size = key_size + val_size + 2;
		/* Request enough memory for ;key=val */
		out = AllocVa( lst, accum + self_size, total );
		if ( out ) {
			size_t i = accum;
			out[i++] = ';';
			for (n = 0; n < key_size; n++) {
				out[i + n] = key[n];
			}
			i += key_size;
			out[i++] = '=';
			for (n = 0; n < val_size; n++) {
				out[i + n] = val[n];
			}
		}
		SafeFree(val);
		return out;
	} else {
		/* Base case, alloc the array. */
		out = (char *) SafeMalloc(accum + 1);
		if (out) {
			out[accum] = '\0';
			*total = accum;
		}
		return out;
	}
}

char * MakeBoardString(char * first, ...) {
	va_list lst;
	va_start(lst, first);
	size_t first_len = strlen(first);
	size_t total;
	char * out = AllocVa(lst, first_len, &total);
	size_t i;
	if (out) {
		for (i = 0; i < first_len; i++) {
			out[i] = first[i];
		}
	}
	va_end(lst);
	return out;
}

/* Reads a position from stdin, returns FALSE on error. */
BOOLEAN InteractReadPosition(STRING input, POSITION * result) {
	char * next_word = strchr(input, ' ');
	if (!next_word) {
		printf(" error =>> missing position in %s request", input);
		return FALSE;
	}
	*result = strtoul(next_word, NULL, 10);
	/* POSITION might be larger than unsigned long,
	 * in which case the above line needs to be changed.
	 * Unfortunately, the C standard provides no stroull.
	 */
	if (kSupportsTierGamesman && gTierGamesman) {
		gInitializeHashWindowToPosition(result);
	}
	return TRUE;
}

BOOLEAN InteractReadBoardString(STRING input, STRING * result) {
	/* Extract a valid board string surrounded in "
	 * result will be *inside* input and null-terminated.
	 * Note that input will be modified in the process.
	 */
	char * start_of_board_string = strchr(input, '"');
	char * end_of_board_string;
	char * scan;
	if (!start_of_board_string) {
		printf(" error =>> missing board string in %s request", input);
		return FALSE;
	}
	++start_of_board_string;
	end_of_board_string = strchr(start_of_board_string, '"');
	if (!end_of_board_string) {
		printf(" error =>> missing end quotes on board string in %s request", input);
		return FALSE;
	}
	scan = start_of_board_string;
	*end_of_board_string = '\0';
	while (*scan) {
		switch (*scan) {
		case ' ':
			break;
		case 'x':
			break;
		case 'o':
			break;
		case 'X':
			*scan = 'x';
			break;
		case 'O':
			*scan = 'o';
			break;
		case '-':
		case '_':
			*scan = ' ';
			break;
		default:
			if (iscntrl(*scan)) {
				/* Might want to do additional error checking here. */
				printf(" error =>> incorrect char %c in board string in %s request", *scan, input);
				return FALSE;
			}
		}
		++scan;
	}
	/* Re-use the input string. */
	*result = start_of_board_string;
	return TRUE;
}

void InteractCheckErrantExtra(STRING input, int max_words) {
	int i = 1;
	input = strchr(input, ' ');
	while (input && ++i <= max_words) {
		++input;
		input = strchr(input, ' ');
	}
	if (i > max_words) {
		printf(" error =>> errant extra input: %s\n", input);
	}
}

STRING InteractValueCharToValueString(char value_char) {
	switch(value_char) {
	case 'T':
		return "tie";
	case 'W':
		return "win";
	case 'L':
		return "lose";
	default:
		return "error";
	}
}

void InteractPrintJSONPositionValue(POSITION pos) {
	char value_char = gValueLetter[GetValueOfPosition(pos)];
	if (value_char != 'U') {
		/* The quote at the beginning is because value might be omitted. */
		printf(",\"value\":\"%s\"", InteractValueCharToValueString(value_char));
	}
}

void InteractFreeBoardSting(STRING board) {
	if (!strcmp(board, "Implement Me")) {
	} else {
		/* SafeFree(board); */
	}
}

void ServerInteractLoop(void) {
	int input_size = 512;
	STRING input = (STRING) SafeMalloc(input_size);
	#define RESULT "result =>> "
	POSITION pos;
	POSITION choice;
	MOVELIST *all_next_moves = NULL;
	MOVELIST *current_move = NULL;
	STRING invalid_board_string = 
		"\n" RESULT "{\"status\":\"error\",\"reason\":\"Invalid board string.\"}";
	MOVE move;
	STRING move_string = NULL;
	STRING board = NULL;
	MEX mex = 0;
	/* Set stdout to do by line buffering so that sever interaction works right.
	 * Otherwise the "ready =>>" message will sit in the buffer forever while
	 * the server waits for it.
	 */
	setvbuf(stdout, NULL, _IOLBF, 1024);
	while (TRUE) {
		memset(input, 0, input_size);
		printf("\n ready =>> ");
		fflush(stdout);
		/* Flush stdout so that the server knows the process is ready.
		 * In other words, this flush really matters.
		 */
		if(!fgets(input, input_size - 1, stdin)) {
			/* The -1 is to ensure input is always null-terminated.
			 */
			/* If the pipe to the server gets broken (probably though the server
			 * receiving SIGKILL) then this process needs to exit.
			 */
			break;
		}
		printf("\n");
		if (!strchr(input, '\n')) {
			printf(" error =>> input too long");
			/* Clear out any excess so that it won't be read in after displaying
			 * the ready prompt.
			 */
			while(getchar() != '\n') {};
			continue;
		}
		/* Clear the '\n' so that string comparison is clearer. */
		*strchr(input, '\n') = '\0';
		if (FirstWordMatches(input, "shutdown") || FirstWordMatches(input, "quit") || FirstWordMatches(input, "exit")) {
			InteractCheckErrantExtra(input, 1);
			printf("\n");
			break;
		} else if (FirstWordMatches(input, "start")) {
			InteractCheckErrantExtra(input, 1);
			printf(RESULT POSITION_FORMAT, gInitialPosition);
		} else if (FirstWordMatches(input, "value")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			printf(RESULT "%c", gValueLetter[GetValueOfPosition(pos)]);
		} else if (FirstWordMatches(input, "choices")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			printf(RESULT "[");
			current_move = all_next_moves = GenerateMoves(pos);
			while (current_move) {
				choice = DoMove(pos, current_move->move);
				current_move = current_move->next;
				printf(POSITION_FORMAT, choice);
				if (current_move) {
					printf(", ");
				}
			}
			printf("]");
			FreeMoveList(all_next_moves);
		} else if (FirstWordMatches(input, "moves")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			printf(RESULT "[");
			current_move = all_next_moves = GenerateMoves(pos);
			while (current_move) {
				printf("%d", current_move->move);
				current_move = current_move->next;
				if (current_move) {
					printf(", ");
				}
			}
			printf("]");
			move_string = NULL;
			FreeMoveList(all_next_moves);
		} else if (FirstWordMatches(input, "named_moves")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			printf(RESULT "[");
			current_move = all_next_moves = GenerateMoves(pos);
			while (current_move) {
				move_string = MoveToString(current_move->move);
				printf("%s", move_string);
				SafeFree(move_string);
				current_move = current_move->next;
				if (current_move) {
					printf(", ");
				}
			}
			printf("]");
			move_string = NULL;
			FreeMoveList(all_next_moves);
		} else if (FirstWordMatches(input, "board")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			board = PositionToString(pos);
			if (!strcmp(board, "Implement Me")) {
				printf(RESULT "not implemented");
			} else {
				printf(RESULT "\"%s\"", board);
			}
			InteractFreeBoardSting(board);
		} else if (FirstWordMatches(input, "remoteness")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			printf(RESULT "%d", Remoteness(pos));
		} else if (FirstWordMatches(input, "mex")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 2);
			if(!kPartizan && !gTwoBits) {
				printf(RESULT "%c", mex);
			} else {
				printf(RESULT "not implemented");
			}
		} else if (FirstWordMatches(input, "result")) {
			if (!InteractReadPosition(input, &pos)) {
				continue;
			}
			InteractCheckErrantExtra(input, 3);
			char * unparsed_move = strchr(input, ' ');
			if (unparsed_move && *unparsed_move) {
				unparsed_move = strchr(unparsed_move + 1, ' ');
			}
			if (unparsed_move) {
				move = atoi(unparsed_move);
				pos = DoMove(pos, move);
				printf(RESULT POSITION_FORMAT, pos);
			} else {
				printf(" error =>> missing move number in result request\n");
			}
		} else if (FirstWordMatches(input, "move_value_response")) {
			if (!InteractReadBoardString(input, &board)) {
				printf(invalid_board_string);
				continue;
			}
			pos = StringToPosition(board);
			if (kSupportsTierGamesman && gTierGamesman) {
				gInitializeHashWindowToPosition(&pos);
			}
			if (pos == -1) {
				printf(invalid_board_string);
				continue;
			}
			printf(RESULT "{\"status\":\"ok\",\"response\":{");
			printf("\"board\":\"%s\",", board);
			printf("\"remoteness\":%d", Remoteness(pos));
			InteractPrintJSONPositionValue(pos);
			printf("}}");
		} else if (FirstWordMatches(input, "position")) {
            if (!InteractReadBoardString(input, &board)) {
                printf(invalid_board_string);
                continue;
            }
            pos = StringToPosition(board);
            printf("board: " POSITION_FORMAT,pos);
            
        } else if (FirstWordMatches(input, "next_move_values_response")) {
			if (!InteractReadBoardString(input, &board)) {
				printf(invalid_board_string);
				continue;
			}
			pos = StringToPosition(board);
			if (kSupportsTierGamesman && gTierGamesman) {
				gInitializeHashWindowToPosition(&pos);
			}
			if (pos == -1) {
				printf(invalid_board_string);
				continue;
			}
			printf(RESULT "{\"status\":\"ok\",\"response\":[");
			current_move = all_next_moves = GenerateMoves(pos);
			while (current_move) {
				/* There needs to be some consensus on whether board strings need to be freed. */
				choice = DoMove(pos, current_move->move);
				board = PositionToString(choice);
				printf("{\"board\":\"%s\"", board);
				InteractFreeBoardSting(board);
				printf(",\"remoteness\":%d", Remoteness(choice));
				InteractPrintJSONPositionValue(choice);
				move_string = MoveToString(current_move->move);
				printf(",\"move\":\"%s\"", move_string);
				SafeFree(move_string);
				current_move = current_move->next;
				printf("}");
				if (current_move) {
					printf(", ");
				}
			}
			move_string = NULL;
			FreeMoveList(all_next_moves);
			printf("]}");
		} else {
			printf(" error =>> unknown command: '%s'", input);
			continue;
		}
	}
	SafeFree(input);
	#undef RESULT
}

int FindAndStore(char* string, char* key, int* placeholder){
  int count = 0;
  char* temp = NULL;
  char* end = NULL;
  while (string != NULL){
    temp = strchr(string,'=');
    if(temp == NULL){
      printf("ERROR: string is not in key=value; format");
      return 0;
    }
    *temp = '\0';
    end = strchr(temp+1, ';');
    if(strcmp(string, key) == 0){
        if(end != NULL){
          *end = '\0';
        }
        *placeholder = atoi(temp+1);
        count++;
    }
    if( end != NULL){
        *end = ';';
        end++;
    }
    *temp = '=';
    string = end;
  }
  if (count == 0 || count > 1){
    printf("ERROR: couldn't find %s as a key within the string\n", key);
    return 0;
  } else if (count > 1){
    printf("ERROR: found the key more than once in the string\n");
    return 0;
  }
  return 1;
}


int StringLookup(char* string, ...){
  va_list arguments;
  va_start( arguments, string);
  char* key;
  int* placeholder;
  while(*(key =  va_arg(arguments, char*)) != '\0'){
    placeholder = va_arg(arguments, int*);
    if(FindAndStore(string, key, placeholder) == 0){
      return 0;
    }
  }
  va_end(arguments);
  return 1;
}
