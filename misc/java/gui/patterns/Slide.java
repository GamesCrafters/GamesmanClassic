package patterns;

import java.awt.Point;

import game.Position;
import game.Move;
import game.Bin;

public class Slide extends SimplePattern
{
    public Slide() {
	patternType = SimplePattern.PatternTypes.SLIDE;
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

	    /*if( firstPlace.getID() == secondPlace.getID() )
		continue;
	    else if( firstPlace.isEmpty() ) {
		// a piece was placed here.  would have been the end point
		// of the slide.
		to = m.GetPositionBeforeMove().getLocationCoords( i );
		++count;

	    } else if( secondPlace.isEmpty() ) {
		// the piece disappeared, must have been from here
		from = m.GetPositionAfterMove().getLocationCoords( i );
		++count;

	    } else {
		//the piece changed, ignore
		
		}*/

	    if( firstPlace.getID() == secondPlace.getID() )
		continue;
	    else if(  secondPlace.isEmpty() ) {
		// the piece disappeared, must have been from here
		from = m.GetPositionAfterMove().getLocationCoords( i );
		++count;
	    } else {
		// a piece was placed here.  would have been the end point
		// of the slide.
		to = m.GetPositionBeforeMove().getLocationCoords( i );
		++count;
	    }
	}

	if( count == 2 && canSlide( from, to ))
	    return true;
	else
	    return false;
    }

    public boolean canSlide( Point p1, Point p2 ) {
	return (( p1.x == p2.x ) ||
		( p1.y == p2.y ) ||
		( Math.abs( p1.x - p2.x ) == Math.abs( p1.y - p2.y ) ));
    }

    public String toString() {
	return "slide!";
    }
}
