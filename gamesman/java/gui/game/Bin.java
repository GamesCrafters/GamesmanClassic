package game;

import java.awt.Color;
import java.util.Vector;
import java.util.Iterator;

import java.util.Set;
import java.util.Map;
import java.util.TreeMap;

import main.CInterface;

/**
 * This class represents a "bin", or one place on a board.  Most bins
 * will contain a single piece, such as in Othello or Tic-Tac-Toe.
 * Bins can contain more than one piece in games such as Mancala.
 */

public class Bin {

    public static int EMPTY = 0;
    public static int ID_EMPTY = EMPTY;

    public static int NUM_EMPTY = 0;
    public static int NUM_SINGLE = 1;
    public static int NUM_MULTIPLE = 2;

    public static int SHAPE_EMPTY = 0;
    public static int SHAPE_X = 1;
    public static int SHAPE_O = 2;
    public static int SHAPE_CIRCLE = 3;
    public static int SHAPE_PLUS = 4;
    public static int SHAPE_CUSTOM = 5;

    private int num;
    private char id;
    private int shape;
    private Color color;

    public Bin( char id, Color color, int shape ) {
	this.id = id;
	this.shape = shape;
	this.color = color;

	if( this.shape == SHAPE_EMPTY )
	    num = NUM_EMPTY;
	else 
	    num = NUM_SINGLE;
    }

    public Bin( char id, Color color, int shape, int num ) {
	this.num = num;
	this.id = id;
	this.shape = shape;
	this.color = color;
    }

    /**
     * Returns whether there is a single piece in this bin (as in most games)
     */
    public boolean singlePiece() {
	return true;
	//this needs to be fixed to support multiple pieces/bin
    }

    /**
     * Returns the color of the pieces inside this bin.
     */
    public Color colorOfPieces() {
	return color;
    }

    public char getID() {
	return id;
    }

    /**
     *	Returns one of the SHAPE_ static variables above.  This indicates how
     * the pieces in this bin should be drawn.
     */
    public int getType() {
	return shape;
    }

    public boolean isEmpty() {
	return num == NUM_EMPTY;
    }

    
    public String toString() {
	return "" + id;
    }

    public boolean equals( Object obj ) {
	if( obj instanceof Bin ) {
	    return this.id == ((Bin) obj).id;
	} else {
	    return false;
	}
    }


    /*Piece index stuff below this point was added to help integrate with pattern-matching and graphics by providing a standard index for each type of piece.*/

    public int getPieceIndex() 
    {
	if( num == NUM_EMPTY )
	    return ID_EMPTY;
	else
	    return (int) id;

    }
	//    return DescIndexMap.get(new Character(desc));*/
	/*if(desc=='X' || desc=='B')
	    return PIECE_1;
	else if(desc=='O' || desc=='W')
	    return PIECE_2;
	else
	return EMPTY;*/
    //    }

    /*    public static int getNumTypes()
    {
	return PieceDescs.size();
	}*/

    /*    public static void setMappings(HashMap<Character,Integer> DIMap, Vector<Character> IDMap, Vector<Color> ICMap, Vector<Integer> ISMap)
    {
	DescIndexMap = DIMap;
	PieceDescs = IDMap;
	PieceColors = ICMap;
	PieceShapes = ISMap;
	}*/

    public static Color getColor(char index)
    {
	//return PieceColors.get(index);
	Bin b = possibleBins.get( index );
	if( b != null ) {
	    return b.colorOfPieces();
	}
	else {
	    return null;
	}
    }


    /*    private static HashMap<Character,Integer> DescIndexMap;
    private static Vector<Character> PieceDescs;
    private static Vector<Color> PieceColors;
    private static Vector<Integer> PieceShapes;*/

    public static void addArchetype( Bin b ) {
	possibleBins.put( b.getID(), b );
    }

    public static Bin getArchetype( char desc ) {
	Bin b = possibleBins.get( desc );

	if( b == null )
	    System.out.println( "No archetype available" );

	return b;
    }

    public static Iterator<Character> ArchetypeIDIterator() {
	Set<Character> s = possibleBins.keySet();
	return s.iterator();
    }

    private static TreeMap<Character,Bin> possibleBins = new TreeMap<Character,Bin>();
}
