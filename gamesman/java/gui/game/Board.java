package game;

//import java.util.HashMap;
import java.awt.Color;
import java.util.Stack;

import main.Game;
import main.GameInterface;

public class Board {

    //    public static int PIECE_ON_VERTEX = 0;
    //    public static int PIECE_IN_BIN = 1;

    private static int width;
    private static int height;
    private static int displayType;
    private static Color color;
    

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

    public static Color getColor() {
	return color;
    }
    public static void setColor(Color newColor)
    {
	color = newColor;
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

}
