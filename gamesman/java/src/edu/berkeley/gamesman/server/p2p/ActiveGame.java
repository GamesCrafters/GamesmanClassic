package edu.berkeley.gamesman.server.p2p;

import java.util.Date;

import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.registration.UserNode;

/**
 * Class representing a game between two players. Holds two Strings representing
 * players and a serial game ID representing the game they're playing.
 * 
 * @author Ramesh Sridharan
 * 
 */
public class ActiveGame
{
	protected UserNode player1;
	protected UserNode player2;
	protected String id;
	protected boolean p1Turn = true;
	protected boolean p1Abandoned = false;
	protected boolean p2Abandoned = false;
	protected String lastMove;
	protected Date lastActiveDate = null;
	
	public ActiveGame(String gameId, UserNode p1, UserNode p2)
	{
		this.player1 = p1;
		this.player2 = p2;
		this.id = gameId;
		this.p1Turn = true;
		this.lastMove = null;
		this.lastActiveDate = new Date();
		this.p1Abandoned = false;
		this.p2Abandoned = false;
	}

	public UserNode getPlayer1()
	{
		return player1;
	}
	
	public UserNode getPlayer2()
	{
		return player2;
	}

	public String getId() {
		return id;
	}
	
	public synchronized boolean isPlayer1Turn()
	{
		return this.p1Turn;
	}
	
	public synchronized String getLastMove()
	{
		return this.lastMove;
	}
	
	public synchronized void setLastMove(String move)
	{
		this.lastMove = move;
	}

	public synchronized boolean isPlayer1Abandoned()
	{
		return this.p1Abandoned;
	}
	
	public synchronized void setPlayer1Abandoned(boolean val)
	{
		this.p1Abandoned = val;
	}
	
	public synchronized boolean isPlayer2Abandoned()
	{
		return this.p2Abandoned;
	}
	
	public synchronized void setPlayer2Abandoned(boolean val)
	{
		this.p2Abandoned = val;
	}
	
	public synchronized Date getLastActiveDate() 
	{
		return this.lastActiveDate;
	}

	public synchronized void updateLastActiveDate() 
	{
		this.lastActiveDate = new Date();
	}
	
	public synchronized void switchTurn()
	{
		this.p1Turn = !this.p1Turn;
	}
	
	public String toString()
	{
		String theString;
		if (player1.getUsername().compareTo(player2.getUsername()) > 0)
		{
			theString = (player2.getUsername() + "===with===" + player1.getUsername());
		}
		else
		{
			theString = (player1.getUsername() + "===with===" + player2.getUsername());
		}
		theString+= " playing"+id;
		return theString;
	}
	
	public int hashCode()
	{
		return this.toString().hashCode();
	}
	
	public boolean equals(Object o)
	{
		if (this.getClass() != o.getClass())
		{
			return false;
		}
		else
		{
			ActiveGame that = (ActiveGame) o;
			return (this.toString().equals(that.toString()));
		}
	}
}