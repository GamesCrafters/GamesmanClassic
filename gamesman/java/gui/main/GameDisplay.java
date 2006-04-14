package main;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import game.*;

import renderers.BoardRenderer;
import renderers.stdBRenderer;

public class GameDisplay
{
    private JPanel myBoardPanel;
	
    private JFrame myFrame;
    private BoardRenderer myBoardRenderer;
	
    private Board myBoard;
	
    public GameDisplay(Board board, int width, int height)
    {
	myBoard = board;
	myFrame = new JFrame();
	myFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	myFrame.setSize(width, height);
	myBoardRenderer = new stdBRenderer(500,500,board);
	myBoardPanel = myBoardRenderer.getPanel();
	myFrame.getContentPane().add(BorderLayout.CENTER,myBoardPanel);
	JMenuBar mBar = new JMenuBar();
		
	JMenu fileMenu = new JMenu("File");			
	JMenuItem exitItem = new JMenuItem("Exit");
	exitItem.addActionListener(new exitListener());
	fileMenu.add(exitItem);
	mBar.add(fileMenu);
	/*	
	JMenu boardMenu = new JMenu("Board");
	JMenuItem changeRenderItem = new JMenuItem("Change Board Style");
	changeRenderItem.addActionListener(new changeRendererListener());
	boardMenu.add(changeRenderItem);
	mBar.add(boardMenu);
	*/
	JMenu gameMenu = new JMenu("Game");
	JMenuItem undoMoveItem = new JMenuItem("Undo last move");
	undoMoveItem.addActionListener(new undoMoveListener());
	gameMenu.add(undoMoveItem);
	JMenuItem colorMovesItem = new JMenuItem("Show Move Values");
	colorMovesItem.addActionListener(new colorMovesListener());
	gameMenu.add(colorMovesItem);
	mBar.add(gameMenu);
	
	myFrame.setJMenuBar(mBar);
    }
	

    public void MakeMove(Board b, Move m)
    {
	b.DoMove(m);
	myBoardRenderer.MakeMove(m);
    }
    /* The draw method draws the current game display (including all menus,
     * the board, and other featues) and returns 0 if everything is successful,
     * or a nonzero value if something was unable to be drawn.
     */
    public int draw()
    {
	myFrame.setVisible(true);
	myFrame.validate();
	myFrame.pack();
	//myFrame.pack();
	/*for(int i=0;i<500;i++)
	  {
	  //myFrame.repaint();
	  try
	  {
	  Thread.sleep(10);
	  }
	  catch(Exception exc)
	  {}
	  }*/
	return 0;
    }
	
    private class exitListener implements ActionListener
    {
	public void actionPerformed(ActionEvent Event)
	{
	    myFrame.dispose();
	    myFrame.setVisible(false);
	    System.exit(0);
	}
    }
	
    /*
    private class changeRendererListener implements ActionListener
    {
	public void actionPerformed(ActionEvent Event)
	{
			
		    
	    Object[] possibleRendererValues = { stdBRenderer.getShortDesc(), simplePNGBRenderer.getShortDesc()};
	    String selectedValue = 
		(String) JOptionPane.showInputDialog(null,
						     "Choose Renderer", "Input",
						     JOptionPane.INFORMATION_MESSAGE, null,
						     possibleRendererValues, possibleRendererValues[0]);
      

	    if(selectedValue == null)
		return;
	    if(selectedValue.equals(possibleRendererValues[0]))
	    {
    		myBoardRenderer = new stdBRenderer(myBoardRenderer);
	    }
	    else if(selectedValue.equals(possibleRendererValues[1]))
	    {
		myBoardRenderer = new simplePNGBRenderer(myBoardRenderer);
	    }
	    myFrame.getContentPane().remove(myBoardPanel);
	    myBoardPanel = myBoardRenderer.getPanel();
	    myFrame.getContentPane().add(BorderLayout.CENTER,myBoardPanel);
	    myFrame.getContentPane().validate();
	    myFrame.setVisible(true);
	}
    }
    */

    private class undoMoveListener implements ActionListener
    {
	public void actionPerformed(ActionEvent e)
	{
	    Move lastMove = myBoard.undoMove();
	    myBoardRenderer.undoMove(lastMove);
	}
    }

    private class colorMovesListener implements ActionListener
    {
	public void actionPerformed(ActionEvent e)
	{
	    myBoardRenderer.displayMoves(true);
	}
    }


}
	
	
