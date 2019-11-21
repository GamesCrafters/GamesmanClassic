package patterns;

import java.util.Vector;
import java.awt.Point;

import java.util.SortedSet;
import java.util.TreeSet;
import java.util.Iterator;

import game.Bin;
import game.Move;
import game.MoveType;

public abstract class SimplePattern
{
    public static SortedSet<MoveType> activePatterns = new TreeSet<MoveType>();

    public enum PatternTypes { PLACE, REMOVE, REPLACE, SLIDE, NONE };
    PatternTypes patternType;

    public static void addType( MoveType move ) {
	if( activePatterns.contains( move ) ) {
	    return;
	}

	activePatterns.add( move );
    }

    public static void printActivePatterns() {
	Iterator<MoveType> iter = activePatterns.iterator();
	while( iter.hasNext() ) {
	    System.out.println( iter.next() );
	}
    }

    protected Point from;
    protected Point to;

    /* returns the best SimplePattern that represents this move */
    public static SimplePattern bestMatch( Move m ) {
	MoveType activeType;
	SimplePattern p;
	Iterator<MoveType> typeIter = activePatterns.iterator();

	while( typeIter.hasNext() ) {

	    activeType = typeIter.next();
	    p = activeType.getSimplePattern();

	    if( p.matches( m ) ) {
		return p;
	    }
	}
	return null;
    }

    /* This will be overridden by all subclasses */
    protected boolean matches(Move m) {
	return false;
    }

    public String toString() {
	return "simplepattern";
    }

    public Point getSourceLocation() {
	return from;
    }
    
    public Point getDestLocation() {
	return to;
    }

    public PatternTypes getPatternType() {
	return patternType;
    }
}
