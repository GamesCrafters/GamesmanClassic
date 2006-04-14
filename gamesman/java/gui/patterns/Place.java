package patterns;

import game.Bin;

public class Place
{
    public static boolean isMatch(int oldPiece, int newPiece)
    {
	return ( oldPiece == Bin.ID_EMPTY && newPiece != Bin.ID_EMPTY );
	//	return (oldPiece.isEmpty() && !newPiece.isEmpty());
    }
}
