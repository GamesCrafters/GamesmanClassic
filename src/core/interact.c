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

char * TierstringFromPosition(POSITION pos) {
	TIER tier;
	TIERPOSITION tierpos;
	gUnhashToTierPosition(pos, &tierpos, &tier);
	return StrFromI(tier);
}

char * StringFormat(size_t max_size, char * format_str, ...) {
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
char * PositionToString(POSITION pos);
char * PositionToEndData(POSITION pos);
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
			if (first[i] == ' ') {
				out[i] = '+';
		}
		else {
				out[i] = first[i];
			}
		}
	}
	va_end(lst);
	return out;
}

/* Reads a position from stdin, returns NULL on error. Otherwise, returns a
 * pointer to the rest of the string.
 */
STRING InteractReadPosition(STRING input, POSITION * result) {
	char * next_word = strchr(input, ' ');
	char * end = NULL;
	if (!next_word) {
		printf(" error =>> missing position in %s request", input);
		return NULL;
	}
	*result = strtoul(next_word, &end, 10);
	/* POSITION might be larger than unsigned long,
	 * in which case the above line needs to be changed.
	 * Unfortunately, the C standard provides no stroull.
	 */
	return end;
}

/* Reads a long from stdin, returns NULL on error. Otherwise, returns a
 * pointer to the rest of the string.
 */
STRING InteractReadLong(STRING input, long * result) {
	char * next_word = strchr(input, ' ');
	char * end = NULL;
	if (!next_word) {
		printf(" error =>> missing expected integer in %s request", input);
		return NULL;
	}
	// Skip the space.
	next_word = next_word + 1;
	printf("reading from %s\n", next_word);
	*result = strtol(next_word, &end, 10);
	return end;
}

STRING InteractReadBoardString(STRING input, char ** result) {
	/* Extract a valid board string surrounded in "
	 * result will be *inside* input and null-terminated.
	 * Note that input will be modified in the process.
	 */
	char * start_of_board_string = strchr(input, '"');
	char * end_of_board_string;
	char * scan;
	if (!start_of_board_string) {
		printf(" error =>> missing board string in %s request", input);
		return NULL;
	}
	++start_of_board_string;
	end_of_board_string = strchr(start_of_board_string, '"');
	if (!end_of_board_string) {
		printf(" error =>> missing end quotes on board string in %s request", input);
		return NULL;
	}
	scan = start_of_board_string;
	*end_of_board_string = '\0';
	while (*scan) {
		if (iscntrl(*scan)) {
			/* Might want to do additional error checking here. */
			printf(" error =>> incorrect char %c in board string in %s request", *scan, input);
			return NULL;
		} else if (*scan == '+') {
			*scan = ' ';
		}
		++scan;
	}
	/* Re-use the input string. */
	*result = start_of_board_string;
	/* Skip the null character. */
	++end_of_board_string;
	return end_of_board_string;
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
	case 'U':
		return "undecided";
	default:
		return "error";
	}
}

void InteractPrintJSONPositionValue(POSITION pos) {
	char value_char = gValueLetter[GetValueOfPosition(pos)];
	printf("\"value\":\"%s\"", InteractValueCharToValueString(value_char));
}

void InteractFreeBoardSting(STRING board) {
	if (!strcmp(board, "Implement Me")) {
	} else {
		SafeFree(board);
	}
}

void ServerInteractLoop(void) {
	int input_size = 512;
	char* input = (char *) SafeMalloc(input_size);
	#define RESULT "result =>> "
	POSITION pos;
	POSITION choice;
	MOVELIST *all_next_moves = NULL;
	MOVELIST *current_move = NULL;
	STRING invalid_board_string = 
		"\n" RESULT "{\"status\":\"error\",\"reason\":\"Invalid board string.\"}";
	MOVE move;
	STRING move_string = NULL;
	char * board = NULL;
        char * data = NULL;
	MEX mex = 0;
	TIER tier = 0;
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
		} else if (FirstWordMatches(input, "start_board")) {
			board = PositionToString(gInitialPosition);
			if (!strcmp(board, "Implement Me")) {
				printf(RESULT "not implemented");
			} else {
				printf(RESULT "\"%s\"", board);
			}
			InteractFreeBoardSting(board);
		} else if (FirstWordMatches(input, "start_response")) {
			board = PositionToString(gInitialPosition);
			printf(RESULT "{\"status\":\"ok\",\"response\":");
			if (!strcmp(board, "Implement Me")) {
				printf("\"not implemented\"");
			} else {
				printf("\"%s\"", board);
			}
			printf("}");
			InteractFreeBoardSting(board);
		} else if (FirstWordMatches(input, "end_response")) {
			if (!InteractReadBoardString(input, &board)) {
				printf("%s", invalid_board_string);
				continue;
			}
			pos = StringToPosition(board);
			if (pos == -1) {
				printf("%s", invalid_board_string);
				continue;
			}
			printf(RESULT "{\"status\":\"ok\",\"response\":{");
			/* For debuggin purposes. */
			/*printf("\"board\": \"%s\",",  board);*/
			InteractPrintJSONPositionValue(pos);
			data = PositionToEndData(pos);
			if (data) {
				printf(",%s", data);
				SafeFree(data);
			}
			printf("}}");
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
				printf("%s", invalid_board_string);
				continue;
			}
			pos = StringToPosition(board);
			if (pos == -1) {
				printf("%s", invalid_board_string);
				continue;
			}
			printf(RESULT "{\"status\":\"ok\",\"response\":{");
			printf("\"board\":\"%s\",", board);
			printf("\"remoteness\":%d,", Remoteness(pos));
			InteractPrintJSONPositionValue(pos);
			printf("}}");
		} else if (FirstWordMatches(input, "position")) {
			if (!InteractReadBoardString(input, &board)) {
				printf("%s", invalid_board_string);
				continue;
			}
			pos = StringToPosition(board);
			printf("board: " POSITION_FORMAT,pos);

		} else if (FirstWordMatches(input, "r") || FirstWordMatches(input, "next_move_values_response")) {
			if (!InteractReadBoardString(input, &board)) {
				printf("%s", invalid_board_string);
				continue;
			}
			if(kSupportsTierGamesman && gTierGamesman && GetValue(board, "tier", GetUnsignedLongLong, &tier)) {
				gInitializeHashWindow(tier, TRUE);
			}
			pos = StringToPosition(board);
			if (pos == -1) {
				printf("%s", invalid_board_string);
				continue;
			}
			printf(RESULT "{\"status\":\"ok\",\"response\":[");
			if (Primitive(pos) == undecided) {
				current_move = all_next_moves = GenerateMoves(pos);
				while (current_move) {
					choice = DoMove(pos, current_move->move);
					board = PositionToString(choice);
					printf("{\"board\":\"%s\",", board);
					InteractFreeBoardSting(board);
					printf("\"remoteness\":%d,", Remoteness(choice));
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
			}
			printf("]}");
		} else if (FirstWordMatches(input, "tree_response")) {
			char * next_word = InteractReadBoardString(input, &board);
			if (!next_word) {
				printf("%s", invalid_board_string);
				continue;
			}
			long depth = 0;
			next_word = InteractReadLong(next_word, &depth);
			if (!next_word) {
				continue;
			}
			if(kSupportsTierGamesman && gTierGamesman && GetValue(board, "tier", GetUnsignedLongLong, &tier)) {
				gInitializeHashWindow(tier, TRUE);
			}
			pos = StringToPosition(board);
			if (pos == -1) {
				printf("%s", invalid_board_string);
				continue;
			}
			printf(RESULT "{\"status\":\"ok\",\"response\":");
			PrintLevel(pos, depth, NULL);
			printf("}");
		} else {
			printf(" error =>> unknown command: '%s'", input);
			printf(" valid moves are:\n");
			printf("   start_response\n");
			printf("   tree_response <board string> <depth>\n");
			printf("   next_move_values_response <board string>\n");
			printf("   move_value_response <board string>\n");
			printf("   position <board string>\n");
			printf("   result <hashed position>\n");
			printf("   mex <hashed position>\n");
			printf("   remoteness <hashed position>\n");
			printf("   board <hashed position>\n");
			printf("   named_moves <hashed position>\n");
			printf("   choices <hashed position>\n");
			printf("   value <hashed position>\n");
			printf("   end_response <board string>\n");
			printf("   start\n");
			printf("   quit\n");
			printf("   shutdown\n");
			printf("   exit\n");
			continue;
		}
	}
	SafeFree(input);
	#undef RESULT
}

BOOLEAN GetValueInner(char * board_string, char * key, get_value_func_t func, void * target) {
	char * outer = board_string;
	char * semicolon;
	int i;
	BOOLEAN result = FALSE;
	for (outer = board_string; *outer; outer++) {
		if (*outer == ';') {
			outer += 1;
			for(i = 0; key[i] && outer[i] == key[i]; i++) {}
			if ( ( !outer[i] || outer[i] == '=' ) && (!key[i]) ) {
				/* Match. */
				i += 1; /* Skip '='. */
				semicolon = strchr(outer + i, ';');
				if (semicolon) {
					*semicolon = '\0';
				}
				result = func(outer + i, target);
				if (semicolon) {
					*semicolon = ';';
				}
				if (result && semicolon) {
					/* Check for duplicates. */
					result = ! GetValueInner(semicolon + 1, key, func, target);
				}
				return result;
			}
		}
	}
	return result;
}

BOOLEAN GetInt(char* value, int* placeholder){
	if (value == NULL || placeholder == NULL){
		return FALSE;
	}
	*placeholder = atoi(value);
	return TRUE;
}

BOOLEAN GetUnsignedLongLong(char* value, unsigned long long* placeholder){
	if (value == NULL || placeholder == NULL){
		return FALSE;
	}
        errno = 0;
	*placeholder = strtoull(value, NULL, 10);
	return errno == 0;
}

BOOLEAN GetChar(char* value, char* placeholder) {
	if (value == NULL || placeholder == NULL){
		return FALSE;
	}
	*placeholder = *value; 
	return TRUE;
}

void PrintLevel(POSITION pos, unsigned int depth, char * move_string) {
	TIER tier = 0;
	if (kSupportsTierGamesman && gTierGamesman) {
		TIERPOSITION tierpos = 0;
		gUnhashToTierPosition(pos, &tierpos, &tier);
		gInitializeHashWindow(tier, TRUE);
		pos = gHashToWindowPosition(tierpos, tier);
	}
	char * board = PositionToString(pos);
	printf("{\"board\":\"%s\",", board);
	printf("\"remoteness\":%d,", Remoteness(pos));
	if (move_string) {
		printf("\"move\":\"%s\",", move_string);
	}
	if (depth == 0 || Primitive(pos) != undecided) {
		InteractPrintJSONPositionValue(pos);
	} else {
		printf("\"children\":[");
		if (Primitive(pos) == undecided) {
			MOVELIST *all_next_moves = GenerateMoves(pos);
			MOVELIST *current_move = all_next_moves;
			while (current_move) {
				POSITION choice = DoMove(pos, current_move->move);
				char * move_string = MoveToString(current_move->move);
				PrintLevel(choice, depth - 1, move_string);
				if (tier) {
					gInitializeHashWindow(tier, TRUE);
				}
				SafeFree(move_string);
				current_move = current_move->next;
				if (current_move) {
				  printf(",");
				}
			}
			FreeMoveList(all_next_moves);
		}
		printf("]");
	}
	printf("}");
	InteractFreeBoardSting(board);
}
