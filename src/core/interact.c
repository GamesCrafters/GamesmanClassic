#include "interact.h"

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

void ServerInteractLoop(void) {
	BOOLEAN running = TRUE;
	int input_size = 100;
	STRING input = (STRING) SafeMalloc(input_size);
	#define RESULT "result =>> "
    POSITION pos;
    POSITION choice;
	MOVELIST *all_next_moves = NULL;
	MOVELIST *current_move = NULL;
	MOVE move;
	STRING move_string = NULL;
	STRING board = NULL;
	MEX mex = 0;
	/* Set stdout to do by line buffering so that sever interaction works right.
	 * Otherwise the "ready =>>" message will sit in the buffer forever while
	 * the server waits for it.
	 */
	setvbuf(stdout, NULL, _IOLBF, 1024);
	while (running) {
		memset(input, 0, input_size);
		printf("\n ready =>> ");
		fflush(stdout);
		/* Flush stdout so that the server knows the process is ready.
		 * In other words, this flush really matters.
		 */
		fgets(input, input_size - 1, stdin);
		/* The -1 is to ensure input is always null-terminated.
		 * However, the next statements don't require that it be so.
		 */
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
		if (FirstWordMatches(input, "shutdown") || FirstWordMatches(input, "quit")) {
			InteractCheckErrantExtra(input, 1);
			running = FALSE;
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
				SafeFree(board);
			}
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
		} else {
			printf(" error =>> unknown command: '%s'", input);
			continue;
		}
	}
	SafeFree(input);
	#undef RESULT
}
