package renderers;

/*import java.awt.Graphics.*;
import java.awt.Graphics2D;
import java.awt.Color;
*/
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;

import main.GameDisplay;

import game.Board;
import game.Move;
import game.Value;

import main.Game;
import main.GameInterface;

import patterns.SimplePattern;

public class stdMRenderer implements MoveRenderer
{
    private GameDisplay gd;

    private Board myBoard;
    private BoardRenderer brender;
    private boolean displayEnabled;
    private boolean showValues;
    private Move[] moveList;
    private Shape[] moveShapeList;
    private Move selectedMove;
    private Shape selectedShape;

    public stdMRenderer(GameDisplay gd, Board b, BoardRenderer boardRender)
    {
	this.gd = gd;
	myBoard = b;
	brender = boardRender;
	showValues = false;
	displayEnabled = true;
	updateMoveList();
	
    }

    public void setController( GameDisplay gd ) {
	this.gd = gd;
    }

    public void updateMoveList()
    {
	if(myBoard.getPosition().isPrimitive())
	{
	    moveList = new Move[0];
	    moveShapeList = new Shape[0];
	}
	else
	{
	    moveList = Game.gameInterface.GenerateMoves(myBoard.getPosition());
	    moveShapeList = new Shape[moveList.length];
	    for(int i=0;i<moveList.length;i++)
	    {
		moveShapeList[i] = MoveToShape(moveList[i]);
	    }
	}
    }

    public void animateMove(Move m)
    {
    }
    public void draw(Graphics2D g2d)
    {
	if(displayEnabled)
	{
	    for(int i=0;i<moveList.length;i++)
	    {
		Color moveColor;
		if(!showValues)
		   g2d.setColor(Color.cyan);
		else
		{
		    //Fill in once CInterface allows for finding value
		    Value v = moveList[i].GetValue();
		    if(v.isWin())
			g2d.setColor(Color.green);
		    else if(v.isLose())
			g2d.setColor(Color.red.darker());
		    else if(v.isTie())
			g2d.setColor(Color.yellow);
		    else
			g2d.setColor(Color.yellow);
		}
		//if(selectedShape == moveShapeList[i])
		//  g2d.setColor(Color.black);
		g2d.fill(moveShapeList[i]);
		g2d.setColor(Color.black);

		//int thickness = 2;
		//Delta Remoteness
		int thickness = 2;
		if( showValues )
		    thickness = Math.max( 0, 15-moveList[i].GetPositionAfterMove().getRemoteness());
		
		Stroke oldStroke = g2d.getStroke();
		g2d.setStroke( new BasicStroke(thickness) );
		g2d.draw( moveShapeList[i]);
		g2d.setStroke( oldStroke );

		//g2d.draw( (new BasicStroke(2)).createStrokedShape(moveShapeList[i]));
	    }
	    if(selectedShape!=null)
	    {
		g2d.setColor(Color.black);
		g2d.fill(selectedShape);
	    }
	}
    }
    
    
    public void enable(boolean show)
    {
	displayEnabled = show;
	brender.repaint();
    }

    public void showValues( boolean showValues ) {
	this.showValues = showValues;
    }

    public void toggleShowValues() {
	this.showValues = !this.showValues;
    }

    public void disable()
    {
	displayEnabled = false;
    }

    private void drawSlide(Graphics2D g2d, int fromSquareX, int fromSquareY, int toSquareX, int toSquareY)
    {
	
	/*drawArrow((Graphics) g2d, 
		  fromSquareX*squareW+squareW/2, fromSquareY*squareH+squareH/2,
		  toSquareX*squareW+squareW/2, toSquareY*squareH+squareH/2);*/
	/*int smalldim;
	if(squareW<squareH)
	    smalldim = squareW;
	else
	    smalldim = squareH;
	
	//a.fill(g2d);
	g2d.setColor(Color.black);
	//a.draw(g2d);
	*/
    }

    private void drawPlace(Graphics2D g2d, int squareX, int squareY)
    {
	int squareW = brender.getWidth()/myBoard.getWidth();
	int squareH = brender.getHeight()/myBoard.getHeight();
	g2d.fillOval(squareX*squareW+squareW/4,squareY*squareH+squareH/4, squareW/2, squareH/2);
    }

    private void drawRemove(Graphics2D g2d, int squareX, int squareY)
    {
	int squareW = brender.getWidth()/myBoard.getWidth();
	int squareH = brender.getHeight()/myBoard.getHeight();
	g2d.fillOval(squareX*squareW+squareW/4,squareY*squareH+squareH/4,  squareW/2, squareH/2);
    }

    private Shape MoveToShape(Move mv)
    {
	SimplePattern pattern = mv.getPattern();
	Point source = pattern.getSourceLocation();
	Point dest = pattern.getDestLocation();

	int squareW = brender.getWidth()/myBoard.getWidth();
	int squareH = brender.getHeight()/myBoard.getHeight();
	int smalldim = Math.min( squareW, squareH );
	/*if(squareW>squareH)
	    smalldim = squareH;
	else
	smalldim = squareW;*/

	switch( pattern.getPatternType() ) {
	    case SLIDE:
		return (new Arrow( source.x*squareW+squareW/2,
				   source.y*squareH+squareH/2,
				   dest.x*squareW+squareW/2,
				   dest.y*squareW+squareW/2,
				   smalldim/8)).getPoly();
		//break;
	    case PLACE:
	    case REMOVE:
	    case REPLACE:
		double w = smalldim*.4;
		
		return new Ellipse2D.Double( dest.x*squareW+squareW/2-w/2,
					     dest.y*squareH+squareH/2-w/2,
					     w,
					     w);
		//break;
	    default:
		return null;
		//break;
	}
	/*if( pattern.toString().equals( "slide" ) ) {
	    return (new Arrow( source.y*squareW+squareW/2,
			       source.x*squareH+squareH/2,
			       dest.y*squareW+squareW/2,
			       dest.x*squareW+squareW/2,
			       smalldim/8)).getPoly();

	} else if( pattern.toString().equals( "place" ) ||
		   pattern.toString().equals( "remove" ) ||
		   pattern.toString().equals( "replace" ) ) {

	    double w = smalldim*.4;

	    return new Ellipse2D.Double( dest.y*squareW+squareW/2-w/2,
					 dest.x*squareH+squareH/2-w/2,
					 w,
					 w);
	}
	else
	{
	    //Should not be reached yet... unsupported, throw exception soon.
	    return null;
	    }*/
    }

    
    public void mouseClicked(MouseEvent e)
    {
	if(selectedMove!=null)
	{	
	    //animateMove(selectedMove);
	    
	    //System.out.println( selectedMove );

	    /*myBoard.DoMove(selectedMove);
	      updateMoveList();*/
	    gd.MakeMove( selectedMove );
	    selectedMove=null;
	    selectedShape=null;
	}
	
	brender.repaint();
    }

    public void mouseMoved(MouseEvent e)
    {
	boolean onShape = false;
	for(int i=0;i<moveShapeList.length;i++)
	{
	    if(moveShapeList[i].contains(e.getX(),e.getY()))
	    {
		onShape=true;
		selectedShape = moveShapeList[i];
		selectedMove = moveList[i];
		//brender.repaint();

	    }
	}
	if(!onShape)
	{
	    selectedShape = null;
	    selectedMove = null;
	}
	brender.repaint();
    }

    public void undoMove(Move m)
    {
	if(m==null)
	    return;
	updateMoveList();
    }
}
