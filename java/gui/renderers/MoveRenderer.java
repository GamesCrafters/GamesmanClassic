package renderers;

import java.awt.Graphics.*;
import java.awt.Graphics2D;
import java.awt.event.*;

import main.GameDisplay;

import game.Board;
import game.Move;
import game.Value;

public interface MoveRenderer
{
    public void setController( GameDisplay gd );

    public void draw(Graphics2D g2d);
    public void enable(boolean show);
    public void showValues(boolean showValues);
    public void toggleShowValues();
    public void updateMoveList();
    public void animateMove(Move m);
    public void mouseMoved(MouseEvent e);
    public void mouseClicked(MouseEvent e);
    public void undoMove(Move m);
}
