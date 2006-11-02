package renderers;

import javax.swing.*;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Color;
import java.util.*;
import java.awt.Dimension;
import java.awt.event.*;

import game.Board;
import game.Bin;
import game.Move;
import game.Value;

public class stdBRenderer implements BoardRenderer
{
    private boardPanel myBoardPanel;
    private Board myBoard;
    private int myWidth;
    private int myHeight;

    private MoveRenderer mrender;
	
    public stdBRenderer(int width, int height, Board board)
    {
	myBoard		= board;
	myWidth		= width;
	myHeight	= height;
	myBoardPanel = new boardPanel();
	mrender = new stdMRenderer(myBoard,this);
    }
	
    public stdBRenderer(BoardRenderer renderer)
    {
	myBoard = renderer.getBoard();
	myWidth = renderer.getWidth();
	myHeight = renderer.getHeight();
	myBoardPanel = new boardPanel();
	mrender = new stdMRenderer(myBoard,this);
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
	
    public void repaint()
    {
	myBoardPanel.repaint();
    }
    public void reSize(int width, int height)
    {
	int myWidth 	= width;
	int myHeight 	= height;
    }
	
    public void displayMoves(boolean showVal)
    {
	mrender.enable(showVal);
    }
	
    public void hideMoves()
    {
	//FILL IN LATER
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
	    int squareW = myWidth/myBoard.getWidth();
	    int squareH = myHeight/myBoard.getHeight();
			
	    /*Clear panel*/
	    //g2d.setColor(new Color(200,200,200));
	    g2d.setColor(Board.getColor());
	    g2d.fillRect(0,0,myWidth,myHeight);
			
	    /*Draw Lines*/
	    g2d.setColor(new Color(0,0,0));	
	    for(int row=0;row<=myBoard.getHeight();row++)
	    {
		g2d.drawLine(0,row*squareH,myWidth,row*squareH);
	    }
	    g2d.drawLine(0,myBoard.getHeight()*squareH,myWidth,myBoard.getHeight()*squareH);
	    for(int col=0;col<=myBoard.getWidth();col++)
	    {
		g2d.drawLine(col*squareW, 0, col*squareW, myHeight);
	    } 
			
						
	    for(int row=0;row<myBoard.getHeight();row++)
	    {
		for(int col=0;col<myBoard.getWidth();col++)
		{
		    int pieceType = myBoard.getPosition().getLocation(col,row).getPieceIndex();
		    if (pieceType != Bin.EMPTY)
		    {
			g2d.setColor(Bin.getColor((char) pieceType));
			g2d.fillOval((int) (col*squareW + squareW*.1),
				     (int) (row*squareH + squareH*.1),
				     (int) (squareW*.8),
				     (int) (squareH*.8));;
		    }
		}
	    }
			
	    mrender.draw(g2d);
										
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
	mrender.animateMove(m);
	mrender.updateMoveList();
    }
    
    public void undoMove(Move m)
    {
	mrender.undoMove(m);
	repaint();
    }
}
