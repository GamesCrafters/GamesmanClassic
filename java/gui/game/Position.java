package game;

import main.Game;
import main.GameInterface;
import patterns.*;

import java.awt.Point;

public class Position {
    private long hashedPosition;
    private String unhashedPosition;

    private int remoteness;
    
    Bin[][] currentPosition;

    public Position() {
	hashedPosition = 0;
	unhashedPosition = "";
    }

    public Position( long hashedPosition ) {
	SetHashedPosition( hashedPosition );
    }

    public long GetHashedPosition() {
	return hashedPosition;
    }

    public int getRemoteness() {
	return remoteness;
    }

    public void SetHashedPosition( long hashedPosition ) {
	this.hashedPosition = hashedPosition;

	unhashedPosition = Game.gameInterface.Unhash( hashedPosition );

	remoteness = Game.gameInterface.Remoteness( this );

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

    public Point getLocationCoords( int indexInUnhashed ) {
	return new Point( indexInUnhashed % Board.getWidth(),
			 indexInUnhashed / Board.getWidth() );
    }

    public Move[] getMoves() {
	return Game.gameInterface.GenerateMoves(this);
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
	//	return String.format( "%d", hashedPosition );
	return unhashedPosition;
    }

    public String getUnhashedPosition() {
	return unhashedPosition;
    }

    public boolean isPrimitive()
    {
	return !Game.gameInterface.Primitive(this).isUndecided();
    }
}
