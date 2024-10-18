package patterns;

import game.Bin;
import game.Position;
import game.Move;

public class Remove extends SimplePattern
{
    public static boolean isMatch(int oldPiece, int newPiece)
    {
	//	return !oldPiece.isEmpty() && newPiece.isEmpty();
	/*	if(oldPiece!=Bin.EMPTY && newPiece==Bin.EMPTY)
	    return true;
	else
	return false;*/

	return (oldPiece!=Bin.ID_EMPTY && newPiece==Bin.ID_EMPTY);
	   
    }

    public Remove() {
	patternType = SimplePattern.PatternTypes.REMOVE;
    }

    protected boolean matches( Move m ) {
	String firstPos = m.GetPositionBeforeMove().toString();
	String secondPos = m.GetPositionAfterMove().toString();

	int count = 0;
	int size = firstPos.length();

	Bin firstPlace;
	Bin secondPlace;


	for( int i = 0; i < size; ++i ) {
	    firstPlace = Bin.getArchetype( firstPos.charAt( i ) );
	    secondPlace = Bin.getArchetype( secondPos.charAt( i ) );

	    if( firstPlace.getID() == secondPlace.getID() )
		continue;
	    else if( !firstPlace.isEmpty() ) {
		// we know that the positions are different and the first is not empty
		// sounds like a remove
		from = to = m.GetPositionBeforeMove().getLocationCoords( i );

		++count;
		break;

	    } else {
		// looks more like a place, do nothing
	    }
	}

	if( count == 1 )
	    return true;
	else
	    return false;
    }

    public String toString() {
	return "remove";
    }
}
