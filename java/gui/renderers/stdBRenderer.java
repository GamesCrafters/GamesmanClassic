package renderers;

import javax.swing.*;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Color;
import java.util.*;
import java.awt.Dimension;
import java.awt.event.*;
import java.awt.Point;

import main.GameDisplay;

import game.Board;
import game.Bin;
import game.Move;
import game.Value;

public class stdBRenderer implements BoardRenderer
{
    private GameDisplay gd;

    private boardPanel myBoardPanel;
    private Board myBoard;
    private int myWidth;
    private int myHeight;

    private int squareW;
    private int squareH;

    private MoveRenderer mrender;

    private Thread posrenderThread;
    private PositionRenderer posrender;
	
    public stdBRenderer(GameDisplay gd, int width, int height, Board board)
    {
	this.gd = gd;
	myBoard		= board;
	myWidth		= width;
	myHeight	= height;
	myBoardPanel = new boardPanel();
	mrender = new stdMRenderer(gd, myBoard,this);
	posrender = new GridPositionRenderer(myBoard,this);
	posrenderThread = null;

	squareW = myWidth/myBoard.getWidth();
	squareH = myHeight/myBoard.getHeight();
    }
	
    public stdBRenderer(GameDisplay gd, BoardRenderer renderer)
    {
	this.gd = gd;
	myBoard = renderer.getBoard();
	myWidth = renderer.getWidth();
	myHeight = renderer.getHeight();
	myBoardPanel = new boardPanel();
	mrender = new stdMRenderer(gd, myBoard,this);

	posrender = new GridPositionRenderer(myBoard,this);
	posrenderThread = null;
	
	squareW = myWidth/myBoard.getWidth();
	squareH = myHeight/myBoard.getHeight();
    }

    public void setController( GameDisplay gd ) {
	this.gd = gd;
    }
	
    public JPanel getPanel()
    {
	return (JPanel) myBoardPanel;
    }
	
    public static String getShortDesc()
    {
	return "Standard Renderer";
    }
	
    public Board getBoard()
    {
	return myBoard;
    }
    public int getHeight()
    {
	return myHeight;
    }
    public int getWidth()
    {
	return myWidth;
    }


    public int getSquareWidth() {
	return squareW;
    }
    public int getSquareHeight() {
	return squareH;
    }

    public void repaint()
    {
	myBoardPanel.repaint();
    }
    public void reSize(int width, int height)
    {
       	int myWidth 	= width;
	int myHeight 	= height;
	squareW = myWidth/myBoard.getWidth();
	squareH = myHeight/myBoard.getHeight();
    }
	
    public void displayMoves(boolean show)
    {
	mrender.enable(show);
    }

    public void displayMoveValues( boolean showVal ) {
	mrender.showValues( showVal);
    }
    public void toggleDisplayMoveValues() {
	mrender.toggleShowValues();
    }
	
    public void animateMove(Move move)
    {
	//FILL IN LATER
    }
	
    private class boardPanel extends JPanel implements MouseMotionListener, MouseListener
    {
	public boolean showingMoves;
	public boardPanel()
	{
	    addMouseMotionListener(this);
	    addMouseListener(this);
	    showingMoves = true;
	}
		
	public Dimension getMinimumSize()
	{
	    return new Dimension(myWidth, myHeight);
	}
		
	public Dimension getPreferredSize()
	{
	    return new Dimension(myWidth, myHeight);
	}
	public Dimension getMaximumSize()
	{
	    return new Dimension(myWidth, myHeight);
	}

	public void paintComponent(Graphics g)
	{
	    Graphics2D g2d = (Graphics2D) g;		
	    squareW = myWidth/myBoard.getWidth();
	    squareH = myHeight/myBoard.getHeight();
			
	    /*Clear panel*/
	    g2d.setColor(Board.getBackgroundColor());
	    g2d.fillRect(0,0,myWidth,myHeight);
			
	    /*Draw Lines*/
	    drawGrid(g2d, squareW, squareH);
	    /* Draw decorations */
	    drawDecorations(g2d, squareW, squareH);
			
	    //System.out.println( "stdbrender.paintcomponent" );
						
	    mrender.draw(g2d);
	    posrender.draw(g2d);
	}

	public void drawDecorations( Graphics2D g2d, int squareW, int squareH ) {
	    Iterator<Board.Decoration> iter = Board.getDecorationsIterator();
	    Board.Decoration decor;
	    while( iter.hasNext() ) {
		decor = iter.next();
		int offset = decor.getOffset();
		int repeat = decor.getRepeatRate();
		double size = decor.getPercentSize();

		g2d.setColor( decor.getColor() );

		switch( decor.getType() ) {
		    case Line:
			Board.Decoration.Orientation lineOrient = decor.getOrientation();
			switch( lineOrient ) {
			    case HORIZONTAL:
				for( int i = offset; i < Board.getHeight(); i += repeat ) {				
				    drawLine( g2d, squareW, squareH,
					      0, i, Board.getWidth()-1, i, size );
				}
				break;
			    case VERTICAL:
				for( int i = offset; i < Board.getWidth(); i += repeat ) {
				    drawLine( g2d, squareW, squareH,
					      i, 0, i, Board.getHeight()-1, size );
				}
				break;
			    case SLANT_BACKWARD:

				int temp;
				// work up the left side and then across the top
				for( int i = -(Board.getHeight()-1); i < Board.getWidth(); ++i ) {
				    if( i < 0 )
					// add a multiple of width to make this number positive, so we can mod it properly
					temp = i+Board.getWidth()*(1+Board.getHeight()/Board.getWidth());
				    else
					//nothing needs to be done
					temp = i;

				    if( temp%repeat == offset ) {
					drawLine( g2d, squareW, squareH,
						  i, 0,
						  i+Board.getHeight()-1, Board.getHeight()-1, size);
				    }
				}
			    case SLANT_FORWARD:
				for( int i = 0; i < 2*(Board.getWidth()-1); ++i ) {
				    if( i%repeat == offset ) {
					drawLine( g2d, squareW, squareH,
						  i-(Board.getHeight()-1), Board.getHeight()-1,
						  i, 0, size);
				    }
				}
				break;
			}
			break;
		    case Circle:
			for( int row = offset; row < Board.getWidth(); row += repeat ) {
			    for( int col = offset; col < Board.getHeight(); col += repeat ) {
				g2d.fillOval((int) (col*squareW + squareW*(0.5-size/2)),
					     (int) (row*squareH + squareH*(0.5-size/2)),
					     (int) (squareW-squareW*2*(0.5-size/2)),
					     (int) (squareH-squareH*2*(0.5-size/2)));;
			    }
			}
			break;
		    default:
			System.out.println("Unrecognized decoration type");
			break;
		}
	    }

	}

	private void drawGrid( Graphics2D g2d, int squareW, int squareH ) {
	    Color gridColor = Board.getGridColor();
	    if( gridColor != null ) {
		g2d.setColor(gridColor);
		for(int row=0;row<=myBoard.getHeight();row++)
		{
		    g2d.drawLine(0,row*squareH,myWidth,row*squareH);
		}
		g2d.drawLine(0,myBoard.getHeight()*squareH,myWidth,myBoard.getHeight()*squareH);
		for(int col=0;col<=myBoard.getWidth();col++)
		{
		    g2d.drawLine(col*squareW, 0, col*squareW, myHeight);
		}
	    } 	    
	}

	private void drawLine( Graphics2D g2d, int squareW, int squareH, int startx, int starty, int endx, int endy, double size) {
	    int width = Board.getWidth()-1;
	    int height = Board.getHeight()-1;

	    if( startx == endx ) { // slope is infinite
		starty = (starty < 0) ? 0 : starty;
		endy = (endy > height) ? height : endy;

		g2d.fillRect( (int) (squareW*(startx+.5-size/2)),
		  (int) (squareH*(starty+.5)),
		  (int) (squareW*size),
		  (int) ((endy-starty)*squareH) );
		return;

	    } else if( starty == endy ) {
		startx = (startx < 0) ? 0 : startx;
		endx = (endx > width) ? width : endx;

		g2d.fillRect( (int) (squareW*(startx+.5)),
			      (int) (squareH*(starty+.5-size/2)),
			      (int) ((endx-startx)*squareW),
			      (int) (squareH*size) );
		return;


	    } else {
		int slope = (endy-starty)/(endx-startx);

		//System.out.printf( "(%d, %d) -> (%d, %d) slope: %d width: %d height: %d\n",
		//		    startx, starty, endx, endy, slope, width, height );

		if( startx < 0 ) {
		    starty += -startx*slope;
		    startx = 0;
		}
		if( startx > width ) {
		    starty += (startx-width)*slope;
		    startx = width;
		}

		if( starty < 0 ) {
		    startx += -starty*slope;
		    starty = 0;
		}
		if( starty > height ) {
		    startx += (starty-height)*slope;
		    starty = height;
		}

		if( endx < 0 ) {
		    endy += -endx*slope;
		    endx = 0;
		}
		if( endx > width ) {
		    endy += -(endx-width)*slope;
		    endx = width;
		}

		if( endy < 0 ) {
		    endx += -endy*slope;
		    endy = 0;
		}
		if( endy > height ) {
		    endx += -(endy-height)*slope;
		    endy = height;
		    }
	        //slope = (endy-starty)/(endx-startx);
		//System.out.printf( "(%d, %d) -> (%d, %d) slope: %d width: %d height: %d\n\n",
		//		    startx, starty, endx, endy, slope, width, height );
	    }




	    double width2 = size/(Math.sqrt(2)*2);
	    int[] xPoints = { (int) ((startx+.5-width2)*squareW),
			      (int) ((startx+.5+width2)*squareW),
			      (int) ((endx+.5+width2)*squareW),
			      (int) ((endx+.5-width2)*squareW),
			      (int) ((startx+.5-width2)*squareW),

			      (int) ((startx+.5+width2)*squareW),
			      (int) ((startx+.5-width2)*squareW),
			      (int) ((endx+.5-width2)*squareW),
			      (int) ((endx+.5+width2)*squareW),
			      (int) ((startx+.5+width2)*squareW)
	    };
	    
	    int[] yPoints = { (int) ((starty+.5+width2)*squareH),
			      (int) ((starty+.5-width2)*squareH),
			      (int) ((endy+.5-width2)*squareH),
			      (int) ((endy+.5+width2)*squareH),
			      (int) ((starty+.5+width2)*squareH),

			      (int) ((starty+.5+width2)*squareH),
			      (int) ((starty+.5-width2)*squareH),
			      (int) ((endy+.5-width2)*squareH),
			      (int) ((endy+.5+width2)*squareH),
			      (int) ((starty+.5+width2)*squareH)
			      
	    };

	    // draw the line
	    g2d.fillPolygon( xPoints, yPoints, 10 );
	    //g2d.drawPolygon( xPoints, yPoints, 10 );
	}
	
	public void mouseDragged(MouseEvent e)
	{
	    //myBoardRenderer.mouseDragged(e);
	}
	public void mouseMoved(MouseEvent e)
	{
	    mrender.mouseMoved(e);
	}

	public void mouseClicked(MouseEvent e)
	{
	    mrender.mouseClicked(e);
	}

	public void mouseEntered(MouseEvent e)
	{
	}
	    
	public void mouseExited(MouseEvent e)
	{
	}

	public void mousePressed(MouseEvent e)
	{
	}

	public void mouseReleased(MouseEvent e)
	{
	}

    }

    public void MakeMove(Move m)
    {
	//mrender.animateMove(m);
	mrender.updateMoveList();
	mrender.enable( false );

	posrender.animateMove( m );
	posrenderThread = new Thread( posrender );
	posrenderThread.start();
    }
    
    public void undoMove(Move m)
    {
	mrender.undoMove(m);
	mrender.enable( false );

	posrender.animateUndoMove( m );
	posrenderThread = new Thread( posrender );
	posrenderThread.start();//	repaint();
    }
}
