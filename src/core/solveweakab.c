#include "solveweakab.h"

/* INFINITY can sometimes already be defined in math.h. */
#ifdef INFINITY
#undef INFINITY
#endif

#define INFINITY (gMaxRemoteness + 1)

/*
   Function: invert_score
    Create inverse of given score (depending on maximum remoteness)

   Arguments:
    score - score to invert
    max_remoteness - maximum remoteness (determines range scores for win/lose/tie)

 */
int ctra = 0;

SCORE invert_score(SCORE score, REMOTENESS max_remoteness) {

	return -score;

}


/* PROBLEM:
   ALL DRAWS MUST BE 0 as there are no score symmetries
 */

SCORE generate_score(VALUE value, REMOTENESS remoteness, REMOTENESS min_remoteness, REMOTENESS max_remoteness) {

	SCORE score;

	/* Normalize minimum remoteness in case it goes below 0 */
	if (min_remoteness < 0)
		min_remoteness = 0;

	switch(value) {

	case win:
		/* If value is win, subtract remoteness from maximum score and add min_remoteness */
		score = max_remoteness + min_remoteness - remoteness;
		break;
	case lose:
		/* If value is lose, add remoteness to minimum score and subtract min_remoteness */
		score = -max_remoteness - min_remoteness + remoteness;
		break;
	case tie:
		/* If value is tie, score is 0 */
		score = 0;
		break;
	case undecided:
		fprintf(stderr, "ERROR: generate_score invoked with undecided game value\n");
		score = INT_MAX;
		break;
	default:
		printf("Invalid game value: %d\n", value);
	}

	if (abs(score) >= INFINITY) {
		fprintf(stderr, "SCORE %d out of bounds, value = %d, remoteness = %d\n", score, value, remoteness);
	}
	return score;

}



SCORE alpha_beta(POSITION position, SCORE alpha, SCORE beta, REMOTENESS min_remoteness, REMOTENESS max_remoteness) {

	VALUE value;
	REMOTENESS remoteness;
	MOVELIST *moves_list, *move_node;
	POSITION child, best_child;
	SCORE score, best_score;
	BOOLEAN set = FALSE;
	BOOLEAN run = FALSE;

	if (alpha>=beta) {

		fprintf(stderr, "alpha_beta invoked with alpha=%d, beta=%d\n", alpha, beta);

	}

	best_score = -INFINITY;
	ctra++;
	if (!(ctra & 0xFFFF)) {
		printf("evaluated %d positions\n", ctra);
	}

	/* First examine if the game value of position is known already */
	value = GetValueOfPosition(position);

	/* If game value of position is known already */
	if (value != undecided) {

		/* Extract remoteness of position */
		remoteness = Remoteness(position);

	}
	/* If game value of position is not known yet */
	else {

		/* Check if the position is terminal and extract value */
		value = Primitive(position);

		/* If position is terminal (i.e. value can be determined directly) */
		if (value != undecided) {

			/* Set remoteness as 0 (terminal position) */
			remoteness = 0;

		}
		/* If this position is non-terminal and has never been visited */
		else {

			/* Generate possible moves from this position */
			moves_list = GenerateMoves(position);

			if (moves_list == NULL) {
				fprintf(stderr,"ERROR: empty move list\n");
			}

			/* For every possible move until alpha >= beta */
			for (move_node = moves_list;
			     move_node != NULL && alpha < beta;
			     move_node = move_node->next)
			{
				/*SCORE tempa, tempb;*/
				run = TRUE;

				/* Obtain position resulting from application of move */
				child = DoMove(position, move_node->move);

				/* Normalize child position if symmetry handling is enabled */
				if (gSymmetries) {

					/* Normalize child position */
					child = gCanonicalPosition(child);

				}

				/* If position hash value is illegal, report error */
				if (child >= gNumberOfPositions) {

					/* Report bad position */
					FoundBadPosition(child, position, move_node->move);

				}

				/* Run the alpha_beta algorithm on the child board with inverted alpha and beta */
				score = invert_score(alpha_beta(child,
				                                invert_score(beta, max_remoteness),
				                                invert_score(alpha, max_remoteness),
				                                min_remoteness - 1,
				                                max_remoteness),
				                     max_remoteness);

				if (score > best_score) {

					best_score = score;
					best_child = child;
					set = TRUE;

					/* If inverse of min score exceeds alpha */
					if (best_score > alpha) {

						/* Replace alpha with inverse of min score */
						alpha = best_score;

					}

				}

				/* Undo move for efficiency if GPS is enabled */
				if (gUseGPS)
					gUndoMove(move_node->move);

			}

			/* Deallocate list of moves */
			FreeMoveList(moves_list);

			/* Set value of position to depending on value of best child */
			switch (GetValueOfPosition(best_child)) {

			case win:
				/* If child's can force win, this board's value is lose */
				value = lose;
				break;
			case lose:
				/* If child can't do better than lose, this board's value is win */
				value = win;
				break;
			case tie:
				/* If child's value is tie, this board's value is tie */
				value = tie;
				break;
			case undecided:
				break;

			}

			/* Set remoteness of position value to one more than that of chosen child */
			remoteness = Remoteness(best_child) + 1;

		}

		/* Store remoteness of value in database */
		SetRemoteness(position, remoteness);

		/* Store value of position in database */
		StoreValueOfPosition(position, value);

	}

	/* Generate score for this board */
	return generate_score(value, remoteness, min_remoteness, max_remoteness);

}

SCORE MTD(POSITION position, SCORE score) {

	SCORE upperbound, lowerbound, beta;

	upperbound = +INFINITY;
	lowerbound = -INFINITY;
	int counter = 1;

	/* Repeat until zeroed in on score */
	do {
		/* beta is a *valid* version of the approximate score
		   i.e. it is either the approximate score,
		   or in case the approximate score is the lowerbound, it is the minimal valid score,
		   since the lowerbound score is invalid and we need a valid alpha
		 */
		beta = (lowerbound == score) ? score + 1 : score;

		printf("Alpha-beta run #%d alpha=%d, beta=%d\n", counter, beta - 1, beta);

		/* Run the alpha-beta pruning minimax search with alpha = beta - 1 */
		score = alpha_beta(position, beta - 1, beta, gMinRemoteness, gMaxRemoteness);

		printf("Score obtained in alpha-beta run #%d is %d\n", counter++, score);

		/* If score is less than beta, change upperbound to equal score */
		if (score < beta) {
			upperbound = score;
		}
		/* Otherwise change lowerbound to equal score */
		else {
			lowerbound = score;
		}
	} while (lowerbound < upperbound);

	return score;

}


VALUE DetermineValueAlphaBeta(POSITION position) {

	printf("starting alpha_beta with alpha = %d, beta = %d, min_remoteness = %d, max_remoteness = %d\n",
	       -INFINITY,
	       +INFINITY,
	       gMinRemoteness,
	       gMaxRemoteness);

	MTD(position, -INFINITY);
	//alpha_beta(position, -INFINITY, +INFINITY, gMinRemoteness, gMaxRemoteness);
	return GetValueOfPosition(position);

}
