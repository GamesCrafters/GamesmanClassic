package game;

import java.util.HashMap;
import java.util.Vector;
import java.util.Iterator;

import main.Game;
import main.GameInterface;
import patterns.*;

import java.awt.Point;

public class Move {

    private int hashedMove;

    private Position positionBeforeMove;
    private Position positionAfterMove;

    private MoveType moveType;

    SimplePattern pattern;

    private Value value;

    public Move( Position originalPosition, int hashedMove ) {
	try {
	    this.hashedMove = hashedMove;
	    this.positionBeforeMove = originalPosition;
	    this.positionAfterMove = Game.gameInterface.DoMove( originalPosition, hashedMove );
	    this.value = new Value( Value.UNDECIDED );
	    this.pattern = null;
	    
	    FindPattern();
	} catch( Exception e ) { //do nothing, this exception comes from Value
	}
    }

    public Move( Position originalPosition, int hashedMove, Value value ) {
	this.hashedMove = hashedMove;
	this.positionBeforeMove = originalPosition;
	this.positionAfterMove = Game.gameInterface.DoMove( originalPosition, hashedMove );
	this.value = value;
	this.pattern = null;

	FindPattern();
    }

    public int GetHashedMove() {
	return hashedMove;
    }

    public void SetHashedMove( int hashedMove ) {
	this.hashedMove = hashedMove;
    }

    public MoveType GetType() {
	return moveType;
    }
    public Position GetPositionBeforeMove() {
	return positionBeforeMove;
    }

    public Position GetPositionAfterMove() {
	return positionAfterMove;
    }

    public Value GetValue() {
	return value;
    }

    public String toString() {

	StringBuffer str = new StringBuffer();
	str.append( "Before: \"" + positionBeforeMove +"\"\n");
	str.append( "After:  \"" + positionAfterMove + "\"\n" );
	str.append( "Hash: " + hashedMove + "\n" );
	return str.toString();

	/*	ret = "";
	ret = ret + "Hash: " + hashedMove;
	return ret;*/
    }


    private void FindPattern() {
	pattern = SimplePattern.bestMatch( this );

	if( pattern == null ) {
	    System.out.println( "drat.  we failed to find a match." );
	}
    }

    public SimplePattern getPattern() {
	return pattern;
    }
}
