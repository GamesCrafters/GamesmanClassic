package renderers;

import javax.swing.*;
import java.awt.*;

import game.Board;
import game.Move;

import patterns.SimplePattern;
import java.util.List;
import java.util.LinkedList;

public abstract class PositionRenderer implements Runnable
{
    protected Board board;
    protected boolean enable;
    protected BoardRenderer br;

    protected Move currentMove;
    protected boolean animating = false;
    protected boolean animatingUndo = false;

    protected Animation currentAnimation;

    protected int ctr;

    protected List<Animation> animations;


    public static final int ANIM_LENGTH = 40;
    public static final int SLEEP_MILLISEC = 25;

    public abstract void draw(Graphics2D g2d);
    public abstract void animateMove( Move m );
    public abstract void animateUndoMove( Move undo );
    
    public void run() {
	while( animating ) {
	    doRun();

	    try {
		//notify();
		Thread.sleep( SLEEP_MILLISEC );
	    } catch( InterruptedException e ) {
		break;
	    }

	}
	br.displayMoves(true);
    }

    public synchronized void doRun() {
	if( ctr % ANIM_LENGTH == 0 ) {
	    if( !animations.isEmpty() ) {
		//System.out.println( "new animation!" );
		currentAnimation = animations.remove(0);
	    } else {
		//System.out.println( "out of animations, stopping..." );
		animating = false;
		currentMove = null;
	    }
	}
	
	//System.out.println( "ctr:  " + ctr );
	br.repaint();

	ctr++;
	//System.out.println( ctr );	
    }

    public void enable( boolean enable ) {
	this.enable = enable;
    }


    protected class Animation
    {
	public SimplePattern.PatternTypes type;
	public Point animFrom;
	public Point animTo;
	public Animation( SimplePattern.PatternTypes type, Point animFrom, Point animTo ) {
	    this.type = type;
	    this.animFrom = animFrom;
	    this.animTo = animTo;
	}
    }

}
