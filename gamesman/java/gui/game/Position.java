package game;

import main.CInterface;
import patterns.*;

public class Position {
    private long hashedPosition;
    
    Bin[][] currentPosition;

    public Position() {
	hashedPosition = 0;
    }

    public Position( long hashedPosition ) {
	SetHashedPosition( hashedPosition );
    }

    public long GetHashedPosition() {
	return hashedPosition;
    }

    public void SetHashedPosition( long hashedPosition ) {
	this.hashedPosition = hashedPosition;

	String unhashedPosition = CInterface.Unhash( hashedPosition );

	currentPosition = new Bin[ Board.getHeight() ][ Board.getWidth() ];

	for( int i = 0; i < Board.getHeight(); ++i ) {
	    for( int j = 0; j < Board.getWidth(); ++j ) {
		currentPosition[i][j] = Bin.getArchetype( unhashedPosition.charAt( i*Board.getWidth() + j ) );
	    }
	}
    }


    public Bin getLocation( int width, int height ) {
	return currentPosition[height][width];
    }


    public Move[] getMoves() {
	return CInterface.GenerateMoves(this);
    }


    public void print() {
	for( int i = 0; i < Board.getWidth(); ++ i )
	    System.out.print( "-" );
	System.out.println();

	for( int i = 0; i < Board.getHeight(); ++i ) {
	    for( int j = 0; j < Board.getWidth(); ++j ) {
		System.out.print( getLocation( i, j) );
	    }
	    System.out.println();
	}


	for( int i = 0; i < Board.getWidth(); ++ i )
	    System.out.print( "-" );
	System.out.println();
    }

    public String toString() {
	return String.format( "%d", hashedPosition );
    }

    public boolean isPrimitive()
    {
	return !CInterface.Primitive(this).isUndecided();
    }
}
