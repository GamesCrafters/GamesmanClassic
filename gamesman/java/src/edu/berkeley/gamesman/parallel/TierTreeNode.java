package edu.berkeley.gamesman.parallel;

import java.util.ArrayList;

public class TierTreeNode implements Comparable<TierTreeNode>
{
	public boolean wasPrim=true;
	public boolean solved;
	public long tierNum, size;
	public ArrayList<TierTreeNode> deps = new ArrayList<TierTreeNode>();
	public ArrayList<TierTreeNode> parents = new ArrayList<TierTreeNode>();
	public boolean selfDependent;
	public double priority = 0;
	
	
	public TierTreeNode(long tierNum, long size, boolean solved)
	{
		this.tierNum = tierNum;
		this.size = size;
		this.solved = solved;
	}
	
	public String toString()
	{
		return "(" + tierNum + " - " + priority + ")";
	}
	
	public boolean equals(Object o)
	{
		return (o instanceof TierTreeNode)?((TierTreeNode)o).tierNum==tierNum:false;
	}
	
	public void propagateDependencies(double amt)
	{
		//add to my own priority
		this.priority += amt;
		
		//split this one's dependencies into the number of children and propagate
		//use amt not priority - becuyase if we don't it messes up big time
		if (deps.size() > 0)
		{
			double eachPri = amt/deps.size();
			for (int a=0;a<deps.size();a++)
			{
				TierTreeNode ttn = deps.get(a);
				ttn.propagateDependencies(eachPri);
			}
		}
	}


	public int compareTo(TierTreeNode ttno)
	{
		//We have to reverse it since priorityqueues use
		//the least element at the top
		if (priority > ttno.priority)
		{
			return -1;
		}
		else if (priority == ttno.priority)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	

	public long marker = -1; //This is for the priority huerestic use only.
	public int calculateHeuristic(int levels, long tierNum)
	{
		int count = 0;
		if (marker != tierNum)
		{
			count = 1;
			//if this is a top tier, make it value of 2
			if (parents.size() == 0)
				count++;
		}
		marker = tierNum;	//don't count this one again.
		
		if (levels != 0)
		{
			//check all the parents
			for (int a=0;a<parents.size();a++)
			{
				count += parents.get(a).calculateHeuristic(levels-1, tierNum);
			}
		}
		return count;
	}
	
}
