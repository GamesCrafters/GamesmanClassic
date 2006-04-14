package patterns;

import java.util.Vector;
import java.util.HashMap;
import java.util.Iterator;

/*This is used to hold matches found by the patterns.*/

public class Match
{
    private String type;
    private int pieceType1;
    private int pieceType2;
    private int fromCol;
    private int fromRow;
    private int toCol;
    private int toRow;
    private Match backup = null;
    
    public Match(String givenType, int type1, int type2, int col, int row)
    {
	type = givenType;
	pieceType1 = type1;
	pieceType2 = type2;
	fromCol = col;
	toCol = col;
	fromRow = row;
	toRow = row;
    }

    public Match(String givenType, int type1, int type2, int fromC, int fromR, int toC, int toR)
    {
	type = givenType;
	pieceType1 = type1;
	pieceType2 = type2;
	fromCol = fromC;
	fromRow = fromR;
	toCol = toC;
	toRow = toR;
    }

    public Match(Match m)
    {
	type = m.type;
	pieceType1 = m.pieceType1;
	pieceType2 = m.pieceType2;
	fromCol = m.fromCol;
	fromRow = m.fromRow;
	toCol = m.toCol;
	toRow = m.toRow;
    }

    /*This method returns true if the two matches are equivalent.  It should be called
     *from a variable match on a concrete match, not vice-versa.  Right now it does not
     *handle locations, only piece types.*/
    
    public boolean isEquivalent(Match otherMatch)
    {
	if(type!=otherMatch.type)
	    return false;
	if(pieceType1>=0 && otherMatch.pieceType1!=pieceType1)
	    return false;
	if(pieceType2>=0 && otherMatch.pieceType2!=pieceType2)
	    return false;
	if(pieceType1<0 && otherMatch.pieceType1==0)
	    return false;
	if(pieceType2<0 && otherMatch.pieceType2==0)
	    return false;
	if(pieceType1<0 && pieceType2==pieceType1 && otherMatch.pieceType1!=otherMatch.pieceType2)
	    return false;
	return true;
    }

    public boolean removeEquivalent(Vector<Match> matches, HashMap<Integer,Integer> curValues)
    {
	backup();
	replaceValues(curValues);
	for(int i=0;i<matches.size();i++)
	{
	    if(isEquivalent(matches.get(i)))
	    {
		addToValues(matches.get(i),curValues);
		matches.remove(i);
		restore();
		return true;
	    }
	}
	restore();
	return false;
    }

    
    public String toString()
    {
	String ret = "Match: " + type + " (" + pieceType1;
	if(pieceType2!=pieceType1)
	    ret = ret + " to " + pieceType2;
	ret = ret + " at (" + fromCol + "," + fromRow + ")";
	if(fromRow!=toRow || fromCol!=toCol)
	    ret = ret + " to (" + toCol + "," + toRow + ")";
	ret = ret + ")";
	return ret;
    }


    private void backup()
    {
	backup = new Match(this);
    }

    private void restore()
    {
	type = backup.type;
	pieceType1 = backup.pieceType1;
	pieceType2 = backup.pieceType2;
	fromCol = backup.fromCol;
	fromRow = backup.fromRow;
	toCol = backup.toCol;
	toRow = backup.toRow;
    }

    private void replaceValues(HashMap<Integer,Integer> values)
    {
	Iterator keyIter = values.keySet().iterator();
	while(keyIter.hasNext())
	{
	    Integer key = (Integer) keyIter.next();
	    int keyVal = key.intValue();
	    Integer val = values.get(key);
	    int valVal = val.intValue();
	    if(keyVal<0)
	    {
		if(pieceType1==keyVal)
		    pieceType1 = valVal;
		if(pieceType2==keyVal)
		    pieceType2 = valVal;
		if(fromCol==keyVal)
		    fromCol = valVal;
		if(fromRow==keyVal)
		    fromRow = valVal;
		if(toCol==keyVal)
		    toCol = valVal;
		if(toRow==keyVal)
		    toRow = valVal;
	    }
	}
		
    }

    public Match replacedValues(HashMap<Integer,Integer> values)
    {
	Match ret = new Match(this);
	ret.replaceValues(values);	
	return ret;
    }

    public void addToValues(Match m, HashMap<Integer,Integer> values)
    {
	if(pieceType1<0)
	    values.put(new Integer(pieceType1),m.pieceType1);
	if(pieceType2<0)
	    values.put(new Integer(pieceType2),m.pieceType2);
	if(fromCol<0)
	    values.put(new Integer(fromCol),m.fromCol);
	if(fromRow<0)
	    values.put(new Integer(fromRow),m.fromRow);
	if(toCol<0)
	    values.put(new Integer(toCol),m.toCol);
	if(toRow<0)
	    values.put(new Integer(toRow),m.toRow);
    }


    public String getType()
    {
	return type;
    }
    public int getPieceOne()
    {
	return pieceType1;
    }
    public int getPieceTwo()
    {
	return pieceType2;
    }
    public int getFromCol()
    {
	return fromCol;
    }
    public int getFromRow()
    {
	return fromRow;
    }
    public int getToCol()
    {
	return toCol;
    }
    public int getToRow()
    {
	return toRow;
    }
}
