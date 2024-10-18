package edu.berkeley.gamesman.server.registration;

import java.util.Date;
import java.util.Properties;

/*
 * Class storing all kinds of info about a user: 
 * 
 * 
 */
public class UserNode {
	protected Properties props = null;
	protected String secretKey = null;
	protected Date lastActiveDate = null;
	protected boolean playingGame = false;

	public UserNode(String secretKey, Properties props) 
	{
		this.props = props;
		this.secretKey = secretKey;
		this.lastActiveDate = new Date();
		this.playingGame = false;
	}
	
	public synchronized boolean isPlayingGame()
	{
		return this.playingGame;
	}
	
	public synchronized void setPlayingGame(boolean val)
	{
		this.playingGame = val;
	}
	
	public String getUsername() 
	{
		return this.props.getProperty("USERNAME");
	}
		
	public String getSecretKey() 
	{
		return this.secretKey;
	}
	
	public synchronized Date getLastActiveDate() 
	{
		return this.lastActiveDate;
	}

	public synchronized void updateLastActiveDate() 
	{
		this.lastActiveDate = new Date();
	}

	public String getPassword() 
	{
		return this.props.getProperty("PASSWORD");
	}
}
