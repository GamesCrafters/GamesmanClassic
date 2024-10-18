
/************************************************************************
**
** NAME:	solveloopybi.c
**
** DESCRIPTION:	Loopy solver with bidirectional abstract game-graph traversal
**
** ABSTRACT:    The traditional loopy solver traverses the game-graph
**              downward using computational edge generation, while
**              concretizing the graph into data structures used for
**              backwards traversal (frontier percolation).
**              That proves a very space-inefficient approach.
**              In contrast the solver presented here does not concretize
**              the game graph, but instead utilizes the potential
**              ability provided by game modules to generate lists of
**              undo moves and apply them as a means of backwards traversal.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-02-06
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"

typedef struct POSITIONSTACK {

	POSITION car;
	struct POSITIONSTACK *cdr;

} POSITIONSTACK;

void push(POSITION car, POSITIONSTACK **stack) {

	POSITIONSTACK *cons;

	cons = (POSITIONSTACK *)SafeMalloc(sizeof(POSITIONSTACK));
	cons->car = car;
	cons->cdr = *stack;
	*stack = cons;

}

POSITION pop(POSITIONSTACK **stack) {

	POSITIONSTACK *cons;
	POSITION car;

	cons = *stack;
	*stack = cons->cdr;
	car = cons->car;
	SafeFree(cons);

	return car;

}

/*
   Function: InitializeFrontier

   Arguments:
   source - the source of the game-graph (a.k.a. initial position)

   Returns:
   Stack of leaf positions (a.k.a. frontier)

   Description:
   Mark all positions reachable from source as visited
   and create frontier consisting of all leaves of game-graph

   Details:

   The game graph is traversed in pre-order (DFS) as opposed to the level-order (BFS)
   approach in the original loopy solver. This is due to the observation that games
   tend to be shorter in depth, than in breadth at any particular distance from the source.

   The algorithm is still iterative however, and the only difference is that a
   stack is used instead of a queue.

   Since there is nothing obviously wrong with it, the frontier is also a stack.
   That means that the frontier will tend to be expanded in branches.

 */
POSITIONSTACK *InitializeFrontier(POSITION source) {

	POSITIONSTACK *stack, *frontier;

	/* Mark source as visited */
	MarkAsVisited(source);

	/* Push source onto stack */
	push(source, &stack);

	/* While stack is non-empty */
	while (stack != NULL) {

		POSITION position;
		VALUE value;

		/* Pop position from stack */
		position = pop(&stack);

		/* Decide if position is primitive */
		value = Primitive(position);

		/* If position is primitive */
		if (value != undecided) {

			/* Set remoteness of value to 0 (immediate) */
			SetRemoteness(position, 0);

			/* Store value of position */
			StoreValueOfPosition(position, value);

			/* Add position to frontier */
			push(position, &frontier);
		}
		/* Otherwise observe the position's children */
		else {

			MOVELIST *moves_list, *move_node;

			/* Generate list of moves available from position */
			moves_list = GenerateMoves(position);

			/* Traverse all moves available from position */
			for (move_node = moves_list;
			     move_node != NULL;
			     move_node=move_node->next) {

				POSITION child;

				/* Apply move */
				child = DoMove(position, move_node->move);

				/* Verify validity of produced position */
				if (child < 0 || child >= gNumberOfPositions) {

					/* Report bad position */
					FoundBadPosition(child, position, move_node->move);

				}

				/* If child position is not already visited */
				if (!Visited(child)) {

					/* Mark child position as visited */
					MarkAsVisited(child);

					/* Push child position onto stack for examination */
					push(child, &stack);

				}

			}
		}
	}
}

/*
   Function: PercolateFrontier

 */

void PercolateFrontier(POSITIONSTACK *frontier) {

}

/*
 */
VALUE DetermineLoopyBiValue(POSITION position) {

	/* Initialize and percolate frontier until exhausted */
	PercolateFrontier(InitializeFrontier(position));

	/* Return value of source position */
	return GetValueOfPosition(position);

}
