package patterns;

import game.Bin;

public class Replace
{
    public static boolean isMatch(int oldPiece, int newPiece)
    {
	//return !oldPiece.isEmpty() && !newPiece.isEmpty() && !oldPiece.equals( newPiece );
	/*	if(oldPiece!=Bin.EMPTY && newPiece!=Bin.EMPTY && oldPiece!=newPiece)
	    return true;
	else
	return false;*/

	return (oldPiece!=Bin.ID_EMPTY && newPiece!=Bin.ID_EMPTY && oldPiece!=newPiece);
    }
}
