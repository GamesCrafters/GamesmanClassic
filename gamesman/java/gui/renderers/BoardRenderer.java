package renderers;

import javax.swing.JPanel;

import game.Board;
import game.Move;

public interface BoardRenderer
{
    public JPanel getPanel();
    public void reSize(int width, int height);
    public void repaint();
    public void displayMoves(boolean showVal);
    public void hideMoves();
    public void animateMove(Move move);
    
    public void MakeMove(Move m);
    public Board getBoard();
    public int getWidth();
    public int getHeight();

    public void undoMove(Move m);
}

