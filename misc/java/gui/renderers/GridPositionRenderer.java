package renderers;

import javax.swing.*;
import java.awt.*;

import java.awt.Image;
import java.awt.Point;

import game.Board;
import game.Move;
import game.Bin;

import game.Position;
import game.Move;
import patterns.SimplePattern;

import java.util.List;
import java.util.LinkedList;
import java.util.ListIterator;

public class GridPositionRenderer extends PositionRenderer
{
    public GridPositionRenderer( Board b, BoardRenderer br ) {
	this.board = b;
	this.br = br;
	enable = true;

	currentMove = null;
	animating = true;
	
	animations = new LinkedList<Animation>();

    }

    public synchronized void draw( Graphics2D g2d ) {
	//System.out.println( "GridPositionRenderer.draw" );
	//System.out.println( currentMove );
	int squareW = br.getSquareWidth();
	int squareH = br.getSquareHeight();

	if( enable && board != null ) {


	    if( currentMove == null || currentAnimation == null ) {
		//System.out.println("Painting pieces");

		drawStandardBoard( g2d, squareW, squareH );
	    } else {
		SimplePattern.PatternTypes animType = currentAnimation.type;
		Point animFrom = currentAnimation.animFrom;
		Point animTo = currentAnimation.animTo;

		// we're currently animating something
		// do more stuff
		for(int row=0;row<board.getHeight();row++)
		{
		    for(int col=0;col<board.getWidth();col++)
		    {
			int pieceType = currentMove.GetPositionAfterMove().getLocation(col,row).getPieceIndex();
			if( pieceType == Bin.EMPTY ) {
			    pieceType = currentMove.GetPositionBeforeMove().getLocation(col,row).getPieceIndex();
			}

			if( (col != animFrom.x || row != animFrom.y) &&
			    (col != animTo.x || row != animTo.y) ) {

			    Color color;
			    if( !involvedInAnimation( new Point(col, row) ) ) {
				pieceType = currentMove.GetPositionAfterMove().getLocation(col,row).getPieceIndex();
			    } else {
				pieceType = currentMove.GetPositionBeforeMove().getLocation(col,row).getPieceIndex();
			    }

			    // draw the current board's piece
			    if (pieceType != Bin.EMPTY)
			    {
				drawPiece( g2d, 
					   (int) ((col+.5)*squareW),
					   (int) ((row+.5)*squareH),
					   (int) (squareW*.4),
					   Bin.getColor((char) pieceType));
			    }
			} else {
			    int frame = (ctr-1) % ANIM_LENGTH;
			    double radius = 0.0;
			    int xloc = (int) ((col+.5)*squareW);
			    int yloc = (int) ((row+.5)*squareW);
			    int pixelradius = 0;
			    Color color = Bin.getColor((char) pieceType);

			    //System.out.println( frame );
			    switch( animType ) {
				case PLACE:
				    radius = .4*(((double) frame)/ANIM_LENGTH);
				    pixelradius = (int) (squareW*radius);
				    
				    //System.out.printf( "Place: (%f - %c)\n", radius, pieceType );
				    break;
				case REMOVE:
				    radius = .4-.4*(((double) frame)/ANIM_LENGTH);
				    pixelradius = (int) (squareW*radius);

				    //System.out.printf( "Remove: (%f - %c)\n", radius, pieceType );
				    break;
				case REPLACE:
				    int half = ANIM_LENGTH/2;

				    if( frame >= half) {
					// new colors, growing
					radius = .4*(((double) (frame-half))/half);
					pixelradius = (int) (squareW*radius);
				    } else {
					// old color, shrinking
					radius = .4-.4*(((double) frame)/(half));
					pixelradius = (int) (squareW*radius);
					
					char oldPieceType = currentMove.GetPositionBeforeMove().getLocation( col, row ).getID();
					color = Bin.getColor( oldPieceType );
				    }
				    //System.out.printf( "Replace:  %f %s\n", radius, color );
				    break;
				case SLIDE:
				    radius = .4 + .1*Math.sin( Math.PI*(((double) frame)/ANIM_LENGTH));

				    int deltax = (animTo.x*squareW-animFrom.x*squareW)/ANIM_LENGTH;
				    int deltay = (animTo.y*squareH-animFrom.y*squareH)/ANIM_LENGTH;
				    xloc = (int) ((animFrom.x+.5)*squareW + deltax*frame);
				    yloc = (int) ((animFrom.y+.5)*squareH + deltay*frame);

				    pixelradius = (int) (squareW*radius);
				    //System.out.printf( "Slide: (%d, %d) %f delta (%d, %d) \'%c\'\n", xloc, yloc, radius, deltax, deltay, pieceType );
				    break;
				default:
				    break;
			    }
			    drawPiece( g2d, xloc, yloc, pixelradius,color );
			}

			
		    }
		}
	    }
	}
    }

    private synchronized void drawPiece( Graphics2D g2d, int xCenter, int yCenter, int radius, Color color ) {
	g2d.setColor( color );
	g2d.fillOval( xCenter-radius, yCenter-radius, radius*2, radius*2 );
    }

    private synchronized void drawStandardBoard( Graphics2D g2d, int squareW, int squareH ) {
	for(int row=0;row<board.getHeight();row++)
	{
	    for(int col=0;col<board.getWidth();col++)
	    {
		int pieceType = board.getPosition().getLocation(col,row).getPieceIndex();
		if (pieceType != Bin.EMPTY)
		{
		    drawPiece( g2d, (int) ((col+.5)*squareW), (int) ((row+.5)*squareH),
			       (int) (squareW*.4), Bin.getColor((char) pieceType));
		}
	    }
	}
    }

    private synchronized void calculateNextAnimation() {
	animations.clear();

	Animation anim;
	SimplePattern.PatternTypes animType;
	Point animFrom;
	Point animTo;

	if( !animating || currentMove == null ) {
	    //System.out.println("animating false or currentMove == null" );
	    //animType = PositionRenderer.AnimType.NONE;
	    animType = SimplePattern.PatternTypes.NONE;
	    animFrom = animTo = null;
	    return;
	}

	SimplePattern pattern = currentMove.getPattern();
	anim = new Animation( pattern.getPatternType(),
			      pattern.getSourceLocation(),
			      pattern.getDestLocation() );

	if( !animatingUndo )
	    animations.add( anim );

	Position before = ( !animatingUndo ? 
			   currentMove.GetPositionBeforeMove() :
			   currentMove.GetPositionAfterMove() );

	Position after = ( !animatingUndo ?
			   currentMove.GetPositionAfterMove() :
			   currentMove.GetPositionBeforeMove() );
	
	Bin beforeBin;
	Bin afterBin;

	//System.out.println( "Before: " + before );
	//System.out.println( "After:  " + after );

	for( int y = 0; y < Board.getHeight(); ++y ) {
	    for( int x = 0; x < Board.getWidth(); ++x ) {
		//System.out.printf( "Scanning (%d, %d)...\n", x, y );

		if( !animatingUndo &&
		    ((x == currentMove.getPattern().getSourceLocation().x &&
		     y == currentMove.getPattern().getSourceLocation().y) ||
		    (x == currentMove.getPattern().getDestLocation().x &&
		     y == currentMove.getPattern().getDestLocation().y)) ) {
		    //System.out.println( "skipping" );
		    continue;
		}


		beforeBin = before.getLocation( x, y );
		afterBin = after.getLocation( x, y );
		    
		//System.out.printf( "(%d, %d) -> \'%c\'?=\'%c\'\n", x, y, beforeBin.getID(), afterBin.getID() );
		if( !beforeBin.equals( afterBin ) ) {
		    animFrom = new Point( x, y );
		    animTo = new Point( x, y );
		    
		    if( beforeBin.isEmpty() ) {
			animType = SimplePattern.PatternTypes.PLACE;
		    } else if( afterBin.isEmpty() ) {
			animType = SimplePattern.PatternTypes.REMOVE;
		    } else {
			animType = SimplePattern.PatternTypes.REPLACE;
		    }
		    animations.add( new Animation(animType, animFrom, animTo ) );
		}
		    
	    }
	}
    }

    public synchronized void animateMove( Move m ) {
	ctr = 0;
	currentMove = m;
	animating = true;
	animatingUndo = false;
	calculateNextAnimation();
	
	//System.out.println( m );
	//System.out.println( currentAnimation );
	//System.out.println( animations.get(0) );
	//System.out.println( "startinganimation" );
    }

    public synchronized void animateUndoMove( Move undo ) {
	ctr = 0;
	currentMove = undo;
	animating = true;
	animatingUndo = true;
	calculateNextAnimation();
    }

    public synchronized boolean involvedInAnimation( Point loc ) {
	ListIterator<Animation> iter = animations.listIterator();
	Animation anim;

	if( currentAnimation != null && 
	    (loc.equals( currentAnimation.animFrom ) ||
	     loc.equals( currentAnimation.animTo ) ) ) {
	    return true;
	}

	while( iter.hasNext() ) {
	    anim = iter.next();

	    if( loc.equals( anim.animFrom ) ||
		loc.equals( anim.animTo ) ) {
		return true;
	    }
	}
	return false;
    }
}
