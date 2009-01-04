package edu.berkeley.gamesman.server.registration;

import java.util.LinkedList;

public class GameNode {
	private static int globalGameCounter = 0;

	protected String id;
	protected String name;
	protected String variant;
	protected String description;	
	protected boolean hostMovesFirst = false;
	protected boolean selectedAccept = false;
	protected boolean acceptSet = false;
	protected LinkedList<UserNode> interested;
	protected UserNode selected;
	protected UserNode host;

	public GameNode(UserNode host, String name, String variant, String desc, boolean hostMovesFirst) 
	{
		synchronized (GameNode.class)
		{
			id = String.valueOf(globalGameCounter++);
		}
		this.name = name;
		this.description = desc;
		this.variant = variant;
		this.selected = null;
		this.interested = new LinkedList<UserNode>();
		this.host = host;
		this.hostMovesFirst = hostMovesFirst;
		this.selectedAccept = false;
		this.acceptSet = false;
	}
	
	public String getId() 
	{
		return this.id;
	}

	public String getName()
	{
		return this.name;
	}

	public String getDescription()
	{
		return this.description;
	}

	public String getVariant()
	{
		return this.variant;
	}
	
	public synchronized UserNode getSelectedPlayer()
	{
		return this.selected;
	}
	
	public synchronized void setSelectedPlayer(UserNode u)
	{
		this.acceptSet = false;
		this.selectedAccept = false;
		this.selected = u;
	}

	public synchronized boolean didSelectedPlayerRespond()
	{
		return this.acceptSet;			
	}
	
	public synchronized void setSelectedPlayerAccept(boolean val)
	{
		this.acceptSet = true;
		this.selectedAccept = val;
	}
	
	public synchronized boolean didSelectedPlayerAccept()
	{
		return this.selectedAccept;			
	}

	public synchronized void addInterestedPlayer(UserNode u) 
	{
		if (!this.interested.contains(u))
			this.interested.add(u);
	}

	public synchronized LinkedList<UserNode> getInterestedPlayers() 
	{
		return this.interested;
	}
	
	public UserNode getHost()
	{
		return this.host;
	}
	
	public boolean doesHostMoveFirst()
	{
		return this.hostMovesFirst;
	}
}