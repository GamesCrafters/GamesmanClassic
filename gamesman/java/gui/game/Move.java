package game;

import java.util.HashMap;
import java.util.Vector;
import java.util.Iterator;

import main.CInterface;
import patterns.*;

public class Move {

    private int hashedMove;
    private Position positionBeforeMove;

    private Position positionAfterMove;
    private MoveType moveType;

    private Vector<Match> myMatches;

    private Value value;
    
    public static HashMap<String,Pattern> PatternMap = null;


    public Move( Position originalPosition, int hashedMove ) {
	try {
	    this.hashedMove = hashedMove;
	    this.positionBeforeMove = originalPosition;
	    this.value = new Value( Value.UNDECIDED );

	    DecomposeMove();
	} catch( Exception e ) { //do nothing, this exception comes from Value
	}
    }

    public Move( Position originalPosition, int hashedMove, Value value ) {
	this.hashedMove = hashedMove;
	this.positionBeforeMove = originalPosition;
	this.value = value;

	DecomposeMove();
    }

    public int GetHashedMove() {
	return hashedMove;
    }

    public void SetHashedMove( int hashedMove ) {
	this.hashedMove = hashedMove;
    }

    public MoveType GetType() {
	return moveType;
    }
    public Position GetPositionBeforeMove() {
	return positionBeforeMove;
    }

    public Value GetValue() {
	return value;
    }

    public String toString() {

	String ret = "";
	for(int i=0;i<myMatches.size();i++)
	{
	    ret = ret + myMatches.get(i).toString() + "\n";
	}
	ret = ret + "Hash: " + hashedMove;
	return ret;
    }

    /**
     * 	This is where our pattern matching should go.  
     *	Preconditions: hashedMove and positionBeforeMove are valid.
     *  Postconditions: moveType is filled out correctly and whatever else we need is also completed (locations of changes, etc)
     */
    private void DecomposeMove() throws UnsupportedOperationException {
	positionAfterMove = CInterface.DoMove( positionBeforeMove, hashedMove );
	Vector<Match> elementaryMatches = new Vector<Match>();
	for(int row=0;row<Board.getHeight();row++)
	{
	    for(int col=0;col<Board.getWidth();col++)
	    {
		int fromPiece = positionBeforeMove.getLocation(col,row).getPieceIndex();
		int toPiece = positionAfterMove.getLocation(col,row).getPieceIndex();

		if(Place.isMatch(fromPiece,toPiece))
		    elementaryMatches.add(new Match("Place",fromPiece,toPiece,col,row));
		else if(Remove.isMatch(fromPiece,toPiece))
		    elementaryMatches.add(new Match("Remove",fromPiece,toPiece,col,row));
		else if(Replace.isMatch(fromPiece,toPiece))
		    elementaryMatches.add(new Match("Replace",fromPiece,toPiece,col,row));
	    }
	}

	/*for(int i=0;i<elementaryMatches.size();i++)
	{
	  System.out.print("" + elementaryMatches.get(i) + "   ");
	  }*/
	if(PatternMap==null)
	{
	    Move.setDefaultPatterns();
	}
	//System.out.println();

	Vector<Match> matchesSoFar = new Vector<Match>(elementaryMatches);
	boolean changeMade = true;
	while(changeMade)
	{
	    changeMade = false;
	    Iterator PatternIter = PatternMap.values().iterator();
	    while(PatternIter.hasNext())
	    {
		Pattern pat = (Pattern) PatternIter.next();
		Vector<Match> bak = new Vector<Match>(matchesSoFar);
		if(pat.applies(matchesSoFar))
		{
		    changeMade = true;
		}
		else
		    matchesSoFar = bak;
	    }
	}
		    
	    
	myMatches = matchesSoFar;
	try
	{
	    Match moveMatch;
	    //System.out.println(myMatches.size());
	    if(myMatches.size()!=1)
	    {
		moveMatch = getPrimaryMatch();
		//System.out.println(moveMatch);
	    }
	    else
	    {
		moveMatch = myMatches.get(0);
	    }
	    /*Handle elementary matches*/
	    if(moveMatch.getType()=="Replace")
	    {
		moveType = new MoveType(MoveType.TYPE_POSITION,MoveType.TARGET_SINGLE,MoveType.OBJECT_EXISTING);
	    }
	    else if(moveMatch.getType()=="Place")
	    {
		moveType = new MoveType(MoveType.TYPE_POSITION,MoveType.TARGET_SINGLE,MoveType.OBJECT_EMPTY);
	    }
	    else if(moveMatch.getType()=="Place")
	    {
		moveType = new MoveType(MoveType.TYPE_POSITION,MoveType.TARGET_SINGLE,MoveType.OBJECT_EXISTING);
	    }
	    /*Handle other matches*/
	    else
	    {
		moveType = PatternMap.get(moveMatch.getType()).getMoveType();
	    }
	}
	catch(Exception e)
	{
	    System.out.println(e.getMessage());
	    e.printStackTrace();
	}

       
    }

    public static void setDefaultPatterns()
    {
	Move.PatternMap = new HashMap<String,Pattern>();

	Vector<Match> matches = new Vector<Match>();
	matches.add(new Match("Remove",-1,0,-2,-3));
	matches.add(new Match("Place",0,-1,-4,-5));
	Match newMatch = new Match("Slide",-1,-1,-2,-3,-4,-5);
	Move.PatternMap.put("Slide",new Pattern("Slide",matches,newMatch,MoveType.TYPE_SLIDE,MoveType.TARGET_SINGLE,MoveType.OBJECT_EMPTY));
    }

    //Just use following priority for now: slide->place->remove->replace... Needs to be an option once file parsing is in.
    public Match getPrimaryMatch()
    {
	if(myMatches.size()==1)
	    return myMatches.get(0);
	Match bestSoFar = myMatches.get(0);
	for(int i=1;i<myMatches.size();i++)
	{
	    Match curMatch = myMatches.get(i);
	    if(curMatch.getType().equals("Slide"))
	    {
	    bestSoFar = curMatch;
	    }
	    else if(curMatch.getType().equals("Place"))
	    {
		if(!bestSoFar.getType().equals("Slide"))
		{
		    bestSoFar = curMatch;
		}
	    }
	    else if(curMatch.getType().equals("Remove"))
	    {
		if(bestSoFar.getType().equals("Replace"))
		{
		    bestSoFar = curMatch;
		}
	    }
	}
	return bestSoFar;
	
    }

	
}
