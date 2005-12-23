#include "solveweakab.h"

/*
 Function: invert_score
    Create inverse of given score (depending on maximum remoteness)
 
 Arguments:
    score - score to invert
    max_remoteness - maximum remoteness (determines range scores for win/lose/tie)

*/
SCORE invert_score(SCORE score, REMOTENESS max_remoteness) {

  /* If score is in the tie range, retain it */
  if (score >= -max_remoteness && score <= max_remoteness) {
    return score;
  }
  /* Otherwise, negate it */
  else {
    return -score;
  }

}

SCORE generate_score(VALUE value, REMOTENESS remoteness, REMOTENESS min_remoteness, REMOTENESS max_remoteness) {

  SCORE score;

  /* Normalize minimum remoteness in case it goes below 0 */
  if (min_remoteness < 0)
    min_remoteness = 0;

  switch(value) {

  case win:
    /* If value is win, subtract remoteness from maximum score and add min_remoteness */
    return 2*max_remoteness + min_remoteness - remoteness;
  case lose:
    /* If value is lose, add remoteness to minimum score and subtract min_remoteness */
    return -2*max_remoteness - min_remoteness + remoteness;
  case tie:
    /* If value is tie, subtract remoteness from maximum tie score and add min_remoteness */
    return max_remoteness + min_remoteness - remoteness;
  }

}

SCORE minimax(POSITION position, SCORE alpha, SCORE beta, REMOTENESS min_remoteness, REMOTENESS max_remoteness) {

  VALUE value;
  REMOTENESS remoteness;
  MOVELIST *moves_list, *move_node;
  POSITION child, best_child;
  SCORE score;

  /* First examine if the game value of position is known already */
  value = GetValueOfPosition(position);
  
  /* If the game value of this position is not known yet */
  if (value == undecided) {
    
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
      
      /* For every possible move until alpha >= beta */
      for (move_node = moves_list;
	   move_node != NULL && alpha < beta;
	   move_node = move_node->next)
	{
	  
	  /* Obtain position resulting from application of move */
	  child = DoMove(position, move_node->move);
	  
	  /* Normalize child position if symmetry handling is enabled */
	  if (gSymmetries) {
	    
	    /* Normalize child position */
	    child = gCanonicalPosition(child);
	    
	  }
	  
	  /* If position hash value is illegal, report error */
	  if (child < 0 || child >= gNumberOfPositions) {
	    
	    /* Report bad position */
	    FoundBadPosition(child, position, move_node->move);
	    
	  }
	  
	  /* Run the minimax algorithm on the child board with inverted alpha and beta */
	  score = invert_score(minimax(child,
				       invert_score(beta, max_remoteness),
				       invert_score(alpha, max_remoteness),
				       min_remoteness - 1,
				       max_remoteness),
				max_remoteness);
	  
	  /* If inverse of min score exceeds alpha */
	  if (score > alpha) {
	    
	    /* Replace alpha with inverse of min score */
	    alpha = score;
	    
	    /* Record child as best child */
	    best_child = child;
	    
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
	
      }
      
      /* Set remoteness of position value to one more than that of chosen child */
      remoteness = Remoteness(child) + 1;
      
    }
    
    /* Store value of position in database */
    StoreValueOfPosition(position, value);
    
    /* Store remoteness of value in database */
    SetRemoteness(position, remoteness);
    
  }
  
  /* Generate score for this board */
  return generate_score(value, remoteness, min_remoteness, max_remoteness);
  
}


VALUE DetermineValueAlphaBeta(POSITION position) {
  
  printf("starting minimax with alpha = %d, beta = %d, min_remoteness = %d, max_remoteness = %d\n",
	 - 2*gMaxRemoteness - 1,
	 2*gMaxRemoteness + 1,
	 gMinRemoteness,
	 gMaxRemoteness);
  minimax(position, - 2*gMaxRemoteness - 1, 2*gMaxRemoteness + 1, gMinRemoteness, gMaxRemoteness);
  return GetValueOfPosition(position);

}
