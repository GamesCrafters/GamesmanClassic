package patterns;

import java.util.Vector;
import java.util.HashMap;

import game.MoveType;

public class Pattern
{
    private Vector<Match> myMatches;
    private String myName;
    private Match myNewMatch;
    private MoveType moveType;

    public Pattern(String name, Vector<Match> matches, Match newMatch, int type, int targ, int obj)
    {
	myName = name;
	myMatches = matches;
	myNewMatch = newMatch;
	try
	{
	    moveType = new MoveType(type,targ,obj);
	}
	catch(Exception e)
	{
	    System.out.println(e.getMessage());
	    System.out.println(e.getStackTrace());
	}
	
    }

    /*Very overly-simplistic right now... It will produce both false negatives and false positives in even slightly complex cases.  Good enough for now, though, since it handles sliding.*/
    public boolean applies(Vector<Match> matches)
    {
	
	HashMap<Integer,Integer> curValues = new HashMap<Integer,Integer>();

	if(myMatches.size()>matches.size())
	    return false;
	for(int i=0;i<myMatches.size();i++)
	{
	    if(!myMatches.get(i).removeEquivalent(matches,curValues))
		return false;		
	}
	matches.add(myNewMatch.replacedValues(curValues));
	return true;
    }

    public MoveType getMoveType()
    {
	return moveType;
    }
}
