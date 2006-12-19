package renderers;

import javax.swing.JPanel;

import main.GameDisplay;

import game.Board;
import game.Move;

public interface BoardRenderer
{
    public void setController( GameDisplay gd );

    public JPanel getPanel();
    public void reSize(int width, int height);
    public void repaint();

    public void displayMoves(boolean show);
    public void displayMoveValues(boolean showVal );
    public void toggleDisplayMoveValues();
    public void animateMove(Move move);
    
    public void MakeMove(Move m);
    public Board getBoard();
    public int getWidth();
    public int getHeight();

    public void undoMove(Move m);

    public int getSquareWidth();
    public int getSquareHeight();
}

