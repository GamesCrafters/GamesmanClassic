package renderers;

/*import java.awt.Graphics.*;
import java.awt.Graphics2D;
import java.awt.Color;
*/
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;

import game.Board;
import game.Move;
import game.Value;

import main.CInterface;

import patterns.Match;

public class stdMRenderer implements MoveRenderer
{
    private Board myBoard;
    private BoardRenderer brender;
    private boolean displayEnabled;
    private boolean showValue;
    private Move[] moveList;
    private Shape[] moveShapeList;
    private Move selectedMove;
    private Shape selectedShape;

    public stdMRenderer(Board b, BoardRenderer boardRender)
    {
	myBoard = b;
	brender = boardRender;
	showValue = false;
	displayEnabled = true;
	updateMoveList();
	
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
	    moveList = CInterface.GenerateMoves(myBoard.getPosition());
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
		if(!showValue)
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
		g2d.draw(moveShapeList[i]);
 
	    }
	    if(selectedShape!=null)
	    {
		g2d.setColor(Color.black);
		g2d.fill(selectedShape);
	    }
	}
    }
    
    
    public void enable(boolean showval)
    {
	displayEnabled = true;
	showValue = showval;
	brender.repaint();
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
	Match match = mv.getPrimaryMatch();
	int squareW = brender.getWidth()/myBoard.getWidth();
	int squareH = brender.getHeight()/myBoard.getHeight();
	int smalldim;
	if(squareW>squareH)
	    smalldim = squareH;
	else
	    smalldim = squareW;
	if(match.getType().equals("Slide"))
	{
	    return (new Arrow(match.getFromCol()*squareW+squareW/2,match.getFromRow()*squareH+squareH/2,match.getToCol()*squareW+squareW/2,match.getToRow()*squareH+squareH/2, smalldim/8)).getPoly();
	}
	if(match.getType().equals("Place"))
	{
	    double w = smalldim*.4;
	    return new Ellipse2D.Double(match.getFromCol()*squareW+squareW/2-w/2,match.getFromRow()*squareH+squareH/2-w/2,w,w);
	}
	else
	{
	    //Should not be reached yet... unsupported, throw exception soon.
	    return null;
	}
    }

    
    public void mouseClicked(MouseEvent e)
    {
	if(selectedMove!=null)
	{	
	    animateMove(selectedMove);
	    myBoard.DoMove(selectedMove);
	    updateMoveList();
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
		brender.repaint();

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
