package patterns;

import game.Bin;

public class Remove
{
    public static boolean isMatch(int oldPiece, int newPiece)
    {
	//	return !oldPiece.isEmpty() && newPiece.isEmpty();
	/*	if(oldPiece!=Bin.EMPTY && newPiece==Bin.EMPTY)
	    return true;
	else
	return false;*/

	return (oldPiece!=Bin.ID_EMPTY && newPiece==Bin.ID_EMPTY);
	   
    }
}
