package game;

//import java.util.HashMap;
import java.awt.Color;
import java.util.Stack;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Iterator;

import main.Game;
import main.GameInterface;

public class Board {

    public static final int GRID = 0;
    public static final int HEX = 1;

    private static int width;
    private static int height;
    private static int displayType;

    private static Color bgcolor;
    private static Color gridColor = null;
    private static Collection<Board.Decoration> decorations = new LinkedList<Board.Decoration>();

    private Stack<Move> pastMoves;
    private Stack<Position> pastPositions;
    public static void setDimensions( int height, int width ) {
	setHeight( height );
	setWidth( width );
    }

    public static int getWidth() {
	return width;
    }
    public static void setWidth( int boardWidth ) {
	width = boardWidth;
    }

    public static int getHeight() {
	return height;
    }
    public static void  setHeight( int boardHeight ) {
	height = boardHeight;
    }

    public static Color getBackgroundColor() {
	return bgcolor;
    }
    public static void setBackgroundColor(Color newColor)
    {
	bgcolor = newColor;
    }

    public static Color getGridColor() {
	return gridColor;
    }
    public static void setGridColor( Color newColor ) {
	gridColor = newColor;
    }

    public static void addDecoration( Board.Decoration decoration ) {
	decorations.add( decoration );
    }
    public static Iterator<Board.Decoration> getDecorationsIterator() {
	return decorations.iterator();
    }

    public static int displayType() {
	return displayType;
    }
    public static void setDisplayType( int type ) {
	displayType = type;
    }




    private Position currentPosition;

    public Board() {
	this.currentPosition = Game.gameInterface.InitialPosition();

	pastMoves = new Stack<Move>();
	pastPositions = new Stack<Position>();
    }

    public Position getPosition() {
	return currentPosition;
    }

    public void DoMove( Move m ) {
	pastMoves.add(m);
	pastPositions.add(currentPosition);
	currentPosition = Game.gameInterface.DoMove(currentPosition, m);
    }

    public Move lastMove()
    {
	if(pastMoves.empty())
	    return null;
	return pastMoves.peek();
    }

    public Move undoMove()
    {
	if(pastMoves.empty())
	    return null;
	currentPosition = pastPositions.pop();
	return pastMoves.pop();
    }


    public Index getIndex( int loc ) {
	return new Index( loc );
    }


    public class Index {
	protected int[] location;
	protected int type = Board.GRID;
       
	// we will generalize this one later
	public Index( int x, int y ) {
	    location = new int[2];
	    location[0] = x;
	    location[1] = y;
	}

	public Index( int loc ) {
	    location = new int[2];
	    location[0] = loc / Board.getWidth();
	    location[1] = loc % Board.getWidth();
	}

	public int[] getLocation() {
	    return location.clone();
	}

	/*public boolean canSlideTo( Index to ) {
	    return ( location[0]==to.location[0] || 
		     location[1]==to.location[1] );
		     }*/

	public String toString() {
	    StringBuffer s = new StringBuffer();

	    s.append( "(" );
	    s.append( location[0] );
	    s.append( ", " );
	    s.append( location[1] );
	    s.append( ")" );

	    return s.toString();
	}
    }

    public static class Decoration {
	public enum Orientation { NONE, VERTICAL, HORIZONTAL, SLANT_FORWARD, SLANT_BACKWARD };
	public enum Type { Line, Circle };

	protected Orientation orientation;
	protected Type type;

	protected int repeat;
	protected int offset;
	protected double percentSize;
	protected Color color;

	public Orientation getOrientation() {
	    return orientation;
	}
	public Type getType() {
	    return type;
	}
	public int getRepeatRate() {
	    return repeat;
	}
	public int getOffset() {
	    return offset;
	}

	// returns the percentage of the square size that this element should be
	public double getPercentSize() {
	    return percentSize;
	}
	public Color getColor() {
	    return color;
	}

	public String toString() {
	    StringBuffer sb = new StringBuffer();
	    sb.append( type );
	    sb.append( ": (");
	    sb.append( orientation );
	    sb.append( ", " );
	    sb.append(repeat );
	    sb.append( ", " );
	    sb.append( offset );
	    sb.append( ")" );
	    
	    return sb.toString();
	}

	public static class Circle extends Board.Decoration {
	    public Circle( Color color, int repeat, int offset, double size ) {
		this.type = Type.valueOf( "Circle" );
		this.orientation = Orientation.valueOf( "NONE" );
		this.color = color;
		this.repeat = repeat;
		this.offset = offset;
		this.percentSize = size;
	    }
	}


	public static class Line extends Board.Decoration {
	    public Line( Color color, String orientation, int repeat, int offset, double size ) {
		this.type = Type.valueOf( "Line" );
		this.orientation = Orientation.valueOf( orientation.toUpperCase().replace("-","_") );
		this.color = color;
		this.repeat = repeat;
		this.offset = offset;
		this.percentSize = size;
	    }
	}

    }
}
