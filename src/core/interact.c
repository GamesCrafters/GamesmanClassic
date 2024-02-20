#include "interact.h"
#include "autoguistrings.h"
#include "hashwindow.h"
#include "sharddb.h"
#include "quartodb.h"
#include <stdarg.h>

POSITION StringToPosition(char *positionString);
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer);
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer);


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
	case 'D':
		return "draw";
	default:
		return "error";
	}
}

void InteractPrintJSONMEXValue(POSITION position) {
	if (kCombinatorial && !gTwoBits) {
		printf(",\"mex\":");
		int theMex = MexLoad(position);
		if(theMex == (MEX) 0)
			printf("\"0\"");
		else if(theMex == (MEX)1)
			printf("\"*\"");
		else
			printf("\"*%d\"", (int)theMex);
	}
}

void InteractPrintJSONPositionValue(VALUE val) {
	char value_char = gValueLetter[val];
	printf(",\"positionValue\":\"%s\"", InteractValueCharToValueString(value_char));
}

void ServerInteractLoop(void) {
	int input_size = 512;
	char* input = (char *) SafeMalloc(input_size);
	char *positionStringBuffer = (char *) SafeMalloc(MAX_POSITION_STRING_LENGTH);
	char *moveStringBuffer = (char *) SafeMalloc(MAX_MOVE_BUTTON_STRING_LENGTH);
	positionStringBuffer[0] = '\0';
	moveStringBuffer[0] = '\0';
	#define RESULT "result =>> "
	POSITION position;
	POSITION childPosition;
	MOVELIST *currentMove = NULL;
	MOVELIST *movesHead = NULL;
	STRING invalidBoardString = 
		"\n" RESULT "{\"error\":\"Invalid board string.\"}";
	char* inputPositionString = NULL;
	VALUE val = lose;
	if (kSupportsShardGamesman) {
		sharddb_cache_init();
	}

	BOOLEAN positionStringMatchesAutoGUIPositionString = gPositionToStringFunPtr == NULL;
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
		if (FirstWordMatches(input, "position_response") || FirstWordMatches(input, "p")) {
			if (!InteractReadBoardString(input, &inputPositionString)) {
				printf("%s", invalidBoardString);
				continue;
			}
			if (kUsesQuartoGamesman) {
				quartoDetailedPositionResponse(inputPositionString, positionStringBuffer);
				continue;
			}
			char oppTurnChar = (inputPositionString[0] == '1') ? '2' : '1';
			position = StringToPosition(inputPositionString);
			if (position == NULL_POSITION) {
				printf("%s", invalidBoardString);
				continue;
			}
			if (kSupportsShardGamesman) {
				shardGamesmanDetailedPositionResponse(
					inputPositionString, position, positionStringBuffer, moveStringBuffer);
				continue;
			}
			POSITIONLIST *childPositionsSentinel = StorePositionInList(NULL_POSITION, NULL);
			POSITIONLIST *childPositionsTail = childPositionsSentinel;
			printf(RESULT "{\"position\":\"%s\"", inputPositionString);
			if (positionStringMatchesAutoGUIPositionString) {
				printf(",\"autoguiPosition\":\"%s\"", inputPositionString);
			} else {
				PositionToAutoGUIString(position, positionStringBuffer);
				positionStringBuffer[0] = inputPositionString[0]; // Handle impartial games
				printf(",\"autoguiPosition\":\"%s\"", positionStringBuffer);
			}

			val = GetValueOfPosition(position);
			InteractPrintJSONPositionValue(val); // e.g. will print ,"value":"win"

			if (val != drawwin && val != drawlose && val != drawdraw) {
				printf(",\"remoteness\":%d", Remoteness(position));
			}

			InteractPrintJSONMEXValue(position);
			if (gPutWinBy) printf(",\"winby\":%d", WinByLoad(position));
			if (kUsePureDraw && (val == drawwin || val == drawlose)) {
				// If using Pure Draw Analysis, the absence of drawlevel and drawremoteness 
				// means that this position is not part of a pure draw cluster
				printf(",\"drawLevel\":%d,\"drawRemoteness\":%d", DrawLevelLoad(position), Remoteness(position));
			}

			printf(",\"moves\":[");
			if (Primitive(position) == undecided) {
				currentMove = movesHead = GenerateMoves(position);
				while (currentMove) {
					childPosition = DoMove(position, currentMove->move);
					childPositionsTail = AppendToTailOfPositionList(childPosition, childPositionsTail);

					PositionToAutoGUIString(childPosition, positionStringBuffer);
					if (positionStringBuffer[0] == '0' && !kPartizan) positionStringBuffer[0] = oppTurnChar; // Handle impartial games
					printf("{\"autoguiPosition\":\"%s\"", positionStringBuffer);

					if (!positionStringMatchesAutoGUIPositionString) {
						gPositionToStringFunPtr(childPosition, positionStringBuffer);
						if (positionStringBuffer[0] == '0' && !kPartizan) positionStringBuffer[0] = oppTurnChar; // Handle impartial games
					}
					printf(",\"position\":\"%s\"", positionStringBuffer);

					// if (gInteractCustomDoMoveFunPtr == NULL) {
					// } else {
					// 	gInteractCustomDoMoveFunPtr(inputPositionString, currentMove->move, positionStringBuffer);
					// }

					val = GetValueOfPosition(childPosition);
					InteractPrintJSONPositionValue(val);
					if (val != drawwin && val != drawlose && val != drawdraw) {
						printf(",\"remoteness\":%d", Remoteness(childPosition));
					}
					InteractPrintJSONMEXValue(childPosition);
					if (gPutWinBy) printf(",\"winby\":%d", WinByLoad(childPosition));
					if (kUsePureDraw && (val == drawwin || val == drawlose)) {
						// If using Pure Draw Analysis, the absence of drawlevel and drawremoteness 
						// means that this position is not part of a pure draw cluster
						printf(",\"drawLevel\":%d,\"drawRemoteness\":%d", DrawLevelLoad(childPosition), Remoteness(childPosition));
					}

					MoveToString(currentMove->move, moveStringBuffer);
					printf(",\"move\":\"%s\"", moveStringBuffer);

					MoveToAutoGUIString(position, currentMove->move, moveStringBuffer);
					printf(",\"autoguiMove\":\"%s\"", moveStringBuffer);

					currentMove = currentMove->next;
					printf("}");
					if (currentMove) {
						printf(",");
					}
				}

				if (gGenerateMultipartMoveEdgesFunPtr != NULL) {
					MULTIPARTEDGELIST *currEdge = NULL;
					if (childPositionsSentinel->next != NULL) {
						currEdge = gGenerateMultipartMoveEdgesFunPtr(position, movesHead, childPositionsSentinel->next);
					}
					if (currEdge != NULL) {
						MULTIPARTEDGELIST *allEdges = currEdge;
						printf("],\"partMoves\":[");
						while (currEdge != NULL) {
							printf("{");
							if (currEdge->from != NULL_POSITION) {
								PositionToAutoGUIString(currEdge->from, positionStringBuffer);
								printf("\"from\":\"%s\",", positionStringBuffer);
							}

							if (currEdge->to != NULL_POSITION) {
								printf("\"to\":\"%s\",", positionStringBuffer);
							}
							
							MoveToAutoGUIString(position, currEdge->partMove, moveStringBuffer);
							printf("\"autoguiMove\":\"%s\",", moveStringBuffer);
							
							if (currEdge->to == NULL_POSITION) {
								MoveToString(currEdge->fullMove, moveStringBuffer);
							} else {
								MoveToString(currEdge->partMove, moveStringBuffer);
							}
							printf("\"move\":\"%s\"}", moveStringBuffer);

							currEdge = currEdge->next;
							if (currEdge) {
								printf(",");
							}
						}
						FreeMultipartEdgeList(allEdges);
					}
				}
				FreePositionList(childPositionsSentinel);
				FreeMoveList(movesHead);
			}
			printf("]}");
		} else if (FirstWordMatches(input, "start_response")) {
			if (kExclusivelyTierGamesman) {
				gInitializeHashWindow(gInitialTier, FALSE);
			}
			if (gRandomInitialPositionFunPtr != NULL) {
				PositionToAutoGUIString(gRandomInitialPositionFunPtr(), positionStringBuffer);
			} else {
				PositionToAutoGUIString(gInitialPosition, positionStringBuffer);
			}
			if (positionStringBuffer[0] == '0' && !kPartizan) positionStringBuffer[0] = '1'; // Handle Impartial Games
			printf(RESULT "{\"autoguiPosition\":\"%s\"", positionStringBuffer);
			if (!positionStringMatchesAutoGUIPositionString) {
				gPositionToStringFunPtr(gInitialPosition, positionStringBuffer);
				if (positionStringBuffer[0] == '0' && !kPartizan) positionStringBuffer[0] = '1'; // Handle Impartial Games
			}
			printf(",\"position\":\"%s\"}", positionStringBuffer);
		} else if (FirstWordMatches(input, "start")) {
			InteractCheckErrantExtra(input, 1);
			printf(RESULT POSITION_FORMAT, gInitialPosition);
		} else if (FirstWordMatches(input, "exit")) {
			InteractCheckErrantExtra(input, 1);
			printf("\n");
			if (kSupportsShardGamesman) {
				sharddb_cache_deallocate();
			}
			break;
		} else if (FirstWordMatches(input, "value")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 2);
			// printf(RESULT "%c", gValueLetter[GetValueOfPosition(position)]);
		} else if (FirstWordMatches(input, "child_positions")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 2);
			// printf(RESULT "[");
			// currentMove = movesHead = GenerateMoves(position);
			// while (currentMove) {
			// 	childPosition = DoMove(position, currentMove->move);
			// 	currentMove = currentMove->next;
			// 	printf(POSITION_FORMAT, childPosition);
			// 	if (currentMove) {
			// 		printf(", ");
			// 	}
			// }
			// printf("]");
			// FreeMoveList(movesHead);
		} else if (FirstWordMatches(input, "moves")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 2);
			// printf(RESULT "[");
			// currentMove = movesHead = GenerateMoves(position);
			// while (currentMove) {
			// 	printf("%d", currentMove->move);
			// 	currentMove = currentMove->next;
			// 	if (currentMove) {
			// 		printf(", ");
			// 	}
			// }
			// printf("]");
			// FreeMoveList(movesHead);
		} else if (FirstWordMatches(input, "named_moves")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 2);
			// printf(RESULT "[");
			// currentMove = movesHead = GenerateMoves(position);
			// while (currentMove) {
			// 	InteractMoveToString(position, currentMove->move, moveStringBuffer);
			// 	printf("%s", moveStringBuffer);
			// 	currentMove = currentMove->next;
			// 	if (currentMove) {
			// 		printf(", ");
			// 	}
			// }
			// printf("]");
			// FreeMoveList(movesHead);
		} else if (FirstWordMatches(input, "board")) {
			// if (InteractReadPosition(input, &position)) {
			// 	InteractCheckErrantExtra(input, 2);
			// 	InteractPositionToString(position, positionStringBuffer);
			// 	printf(RESULT "\"%s\"", positionStringBuffer);
			// }
		} else if (FirstWordMatches(input, "remoteness")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 2);
			// printf(RESULT "%d", Remoteness(position));
		} else if (FirstWordMatches(input, "mex")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 2);
			// if(kCombinatorial && !gTwoBits) {
			// 	printf(RESULT "%c", mex);
			// } else {
			// 	printf(RESULT "not implemented");
			// }
		} else if (FirstWordMatches(input, "result")) {
			// if (!InteractReadPosition(input, &position)) {
			// 	continue;
			// }
			// InteractCheckErrantExtra(input, 3);
			// char * unparsed_move = strchr(input, ' ');
			// if (unparsed_move && *unparsed_move) {
			// 	unparsed_move = strchr(unparsed_move + 1, ' ');
			// }
			// if (unparsed_move) {
			// 	move = atoi(unparsed_move);
			// 	position = DoMove(position, move);
			// 	printf(RESULT POSITION_FORMAT, position);
			// } else {
			// 	printf(" error =>> missing move number in result request\n");
			// }
		} else if (FirstWordMatches(input, "position")) {
			// if (!InteractReadBoardString(input, &inputPositionString)) {
			// 	printf("%s", invalidBoardString);
			// 	continue;
			// }
			// position = InteractStringToPosition(inputPositionString);
			// printf("board: " POSITION_FORMAT, position);
		} else {
			printf(" error =>> unknown command: '%s'", input);
			printf(" valid commands are:\n");
			printf("   start_response                                      [JSON Response Providing Initial Position String]\n");
			printf("   position_response <position string>                 [JSON Response Providing Solved Data for Position Represented by Input String and Solved Data for its Child Positions]\n");
			printf("   position <position string>                          [Hash of Input Position String, Provided By InteractStringToPosition()]\n");
			printf("   start                                               [Hash of Initial Position]\n");
			printf("   board <hashed position>                             [Position String of Input Hashed Position, Provided by InteractPositionToString()]\n");
			printf("   result <hashed position> <hashed move>              [Hashed Child Position Resulting from Making Input Hashed Move to Input Hashed Position]\n");
			printf("   named_moves <hashed position>                       [Names of Legal Moves from Input Hashed Position, Provided by InteractMoveToString()]\n");
			printf("   child_positions <hashed position>                   [Hashed Child Positions of Input Hashed Position]\n");
			printf("   value <hashed position>                             [Character Representing Value of Input Hashed Position]\n");
			printf("   remoteness <hashed position>                        [Remoteness of Input Hashed Position]\n");
			printf("   mex <hashed position>                               [Mex of Input Hashed Position]\n");
			printf("   exit                                                [Quit the Program]\n");
			continue;
		}
	}
	SafeFree(input);
	SafeFree(positionStringBuffer);
	SafeFree(moveStringBuffer);
	#undef RESULT
}

BOOLEAN GetValueInner(char * board_string, char * key, get_value_func_t func, void * target) {
	char * outer = board_string;
	char * semicolon;
	int i;
	BOOLEAN result = FALSE;
	for (outer = board_string; *outer; outer++) {
		if (*outer == ',') {
			outer += 1;
			for(i = 0; key[i] && outer[i] == key[i]; i++) {}
			if ( ( !outer[i] || outer[i] == '=' ) && (!key[i]) ) {
				/* Match. */
				i += 1; /* Skip '='. */
				semicolon = strchr(outer + i, ',');
				if (semicolon) {
					*semicolon = '\0';
				}
				result = func(outer + i, target);
				if (semicolon) {
					*semicolon = ',';
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