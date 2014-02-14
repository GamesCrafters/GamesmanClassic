/* Test script for migo.c */
#include "migo.c"       /* For testing of static functions, need to be in one compilation unit. */

char* errstr;

int comparePosToStr(GoPosition pos, const char* str);

int main(int argc, char* argv[]) {
	if (!testGameCreation())
		printf("Game creation failed!\n");
	if (!testStonePlacement())
		printf("Stone placement failed!\n");
	if (!testCountLiberties())
		printf("Liberty counting failed!\n");
	if (!testDoMove())
		printf("DoMove() failed!\n");
	return 0;
}

int testGameCreation(void) {
	/* FIXME:  Test for nulls, etc */
	const char* empty3x3 =
	        "    A B C\n"
	        "  3 . . .  3\n"
	        "  2 . . .  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	int result;
	GoGame myGame = newGoGame(3);
	char* posstr = stringifyGoPosition(myGame->position);
	result = (strcmp(posstr, empty3x3) == 0);
#ifdef VERBOSE_TESTS
	printf("%s\n", posstr);
#endif
	free(posstr);
	delGoGame(myGame);
	return result;
}

int testStonePlacement(void) {
	/* FIXME:  nulls, etc */
	/* FIXME:  Last move indicator */
	const char* black_center =
	        "    A B C\n"
	        "  3 . . .  3\n"
	        "  2 . X .  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	const char* white_b3 =
	        "    A B C\n"
	        "  3 . O .  3\n"
	        "  2 . X .  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	int res1, res2;
	char* posstr;
	GoGame myGame = newGoGame(3);
	playStone(getGoIntersection(myGame->position->board, 1, 1), STONE_BLACK);
	posstr = stringifyGoPosition(myGame->position);
	res1 = strcmp(posstr, black_center) == 0;
#ifdef VERBOSE_TESTS
	printf("%s\n", posstr);
#endif
	free(posstr);
	playStone(getGoIntersection(myGame->position->board, 1, 2), STONE_WHITE);
	posstr = stringifyGoPosition(myGame->position);
	res2 = strcmp(posstr, white_b3) == 0;
#ifdef VERBOSE_TESTS
	printf("%s\n", posstr);
#endif
	free(posstr);
	delGoGame(myGame);
	return (res1 && res2);  /* FIXME:  Error messages */
}

int testCountLiberties(void) {
	GoGame myGame = newGoGame(3);
	playStone(getGoIntersection(myGame->position->board, 1, 1), STONE_BLACK);
	int res = countLiberties(getStoneString(getGoIntersection(myGame->position->board, 1, 1))) == 4;
	delGoGame(myGame);
	return res;
}

int testDoMove(void) {
	/* FIXME:  nulls, etc */
	const char* black_center =
	        "    A B C\n"
	        "  3 . . .  3\n"
	        "  2 . X .  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	const char* white_b3 =
	        "    A B C\n"
	        "  3 . O .  3\n"
	        "  2 . X .  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	const char* black_c3 =
	        "    A B C\n"
	        "  3 . O X  3\n"
	        "  2 . X .  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	const char* white_c2 =
	        "    A B C\n"
	        "  3 . O .  3\n"
	        "  2 . X O  2\n"
	        "  1 . . .  1\n"
	        "    A B C\n";
	int res1, res2, res3, res4;
	GoMove nextMove;
	GoGame myGame = newGoGame(3);
	nextMove = newGoMove(2, 2);
	myGame->position = doMove_imp(myGame->position, nextMove);
	delGoMove(nextMove);
	res1 = comparePosToStr(myGame->position, black_center);
	nextMove = newGoMove(2, 3);
	myGame->position = doMove_imp(myGame->position, nextMove);
	delGoMove(nextMove);
	res2 = comparePosToStr(myGame->position, white_b3);
	nextMove = newGoMove(3, 3);
	myGame->position = doMove_imp(myGame->position, nextMove);
	delGoMove(nextMove);
	res3 = comparePosToStr(myGame->position, black_c3);
	nextMove = newGoMove(3, 2);
	myGame->position = doMove_imp(myGame->position, nextMove);
	delGoMove(nextMove);
	res4 = comparePosToStr(myGame->position, white_c2);
	delGoGame(myGame);
	return (res1 && res2 && res3 && res4);  /* FIXME:  Error messages */
}

int comparePosToStr(GoPosition pos, const char* str) {
	char* posstr = stringifyGoPosition(pos);
	int result = strcmp(posstr, str) == 0;
#ifdef VERBOSE_TESTS
	printf("%s\n", posstr);
#endif
	free(posstr);
	return result;
}


char* PositionToString(POSITION pos) {
	// FIXME: this is just a stub
	return "Implement Me";
}

char * PositionToEndData(POSITION pos) {
	return NULL;
}
