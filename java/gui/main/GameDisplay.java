package main;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import game.*;

import renderers.BoardRenderer;
import renderers.stdBRenderer;

import renderers.MoveRenderer;
import renderers.PositionRenderer;
import renderers.GridPositionRenderer;

public class GameDisplay extends JFrame
{
    private JPanel myBoardPanel;
	
    private BoardRenderer myBoardRenderer;

    private Board myBoard;
	
    public GameDisplay(Board board, int width, int height)
    {
	myBoard = board;

	setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	setSize(width, height);

	myBoardRenderer = new stdBRenderer(this, 500,500,board);
	myBoardPanel = myBoardRenderer.getPanel();
	
	getContentPane().add(BorderLayout.CENTER,myBoardPanel);

	JMenuBar mBar = new JMenuBar();
		
	JMenu fileMenu = new JMenu("File");			
	JMenuItem exitItem = new JMenuItem("Exit");
	exitItem.addActionListener(new GameDisplayActionListener( GameDisplayActionListener.EXIT ) );
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
	undoMoveItem.addActionListener( new GameDisplayActionListener(GameDisplayActionListener.UNDO_MOVE));
	gameMenu.add(undoMoveItem);

	JMenuItem colorMovesItem = new JMenuItem("Show Move Values");
	colorMovesItem.addActionListener( new GameDisplayActionListener(GameDisplayActionListener.TOGGLE_SHOW_VALUES));
	gameMenu.add(colorMovesItem);
	mBar.add(gameMenu);
	
	setJMenuBar(mBar);
    }
	

    public void MakeMove(Move m)
    {
	myBoard.DoMove(m);
	myBoardRenderer.MakeMove(m);
    }

    /* The draw method draws the current game display (including all menus,
     * the board, and other featues) and returns 0 if everything is successful,
     * or a nonzero value if something was unable to be drawn.
     */
    public int draw()
    {
	setVisible(true);
	///validate();
	pack();
	//myFrame.pack();
	return 0;
    }
	
    private class GameDisplayActionListener implements ActionListener
    {
	int action;
	
	public static final int EXIT = 0;
	public static final int UNDO_MOVE = 1;
	public static final int TOGGLE_SHOW_VALUES = 2;

	GameDisplayActionListener( int action ) {
	    this.action = action;
	}

	public void actionPerformed( ActionEvent e )
	{
	    switch( action ) {
		case EXIT: 			do_exit(); break;
		case UNDO_MOVE:			do_undo_move(); break;
		case TOGGLE_SHOW_VALUES:	do_toggle_show_values(); break;
		default:
		    System.out.println( "Unknown action in GameDisplay" );
		    break;
	    }
	}

	private void do_exit()
	{
	    dispose();
	    setVisible(false);
	    System.exit(0);
	}

	private void do_undo_move()
	{
	    Move lastMove = myBoard.undoMove();
	    myBoardRenderer.undoMove(lastMove);   
	}

	private void do_toggle_show_values()
	{
	    myBoardRenderer.toggleDisplayMoveValues();
	    myBoardRenderer.repaint();
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
}
	
	
