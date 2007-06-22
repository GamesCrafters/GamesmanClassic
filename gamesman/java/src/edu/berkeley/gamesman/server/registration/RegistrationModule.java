package edu.berkeley.gamesman.server.registration;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URLEncoder;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Properties;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;
import edu.berkeley.gamesman.server.RequestType;
import edu.berkeley.gamesman.server.p2p.P2PModule;
/**
 * 
 * @author Victor Perez, Filip Furmanek, Ramesh Sridharan
 *
 */
public class RegistrationModule implements IModule
{
	public static final String HN_USERNAME = "Username";
	public static final String HN_PASSWORD = "Password";
	public static final String HN_SECRET_KEY = "SecretKey";

	public static final String HN_GAME_NAME = "GameName";
	public static final String HN_GAME_VARIANT = "GameVariant";
	public static final String HN_GAME_DESCRIPTION = "GameDescription";
	public static final String HN_GAME_ID = "GameId";
	public static final String HN_GAME_HOST_MOVES_FIRST = "GameHostMovesFirst";

	public static final String HN_LENGTH = "Length";
	public static final String HN_ACCEPT = "Accept";
	
	public static final String HN_STATUS = "Status";
	public static final String HN_MOVE_FIRST = "MoveFirst";
	
	public static final String HN_CHALLENGER_USERNAME = "ChallengerUsername";
	
	
	private static final long TIMEOUT = 20 * 60 * 1000; // 20 mins
	private static Hashtable<String, UserNode> usersOnline = new Hashtable<String, UserNode>();
	private static Hashtable<String, GameNode> openGamesByUsername = new Hashtable<String, GameNode>();
	private static Hashtable<String, GameNode> openGamesByGameId = new Hashtable<String, GameNode>();
	private static Timer reaper = null;
	
	static 
	{
		reaper = new Timer(true);
		long period = 3 * 60 * 1000; // 3 mins
		TimerTask task = new TimerTask()
		{
			public void run()
			{
				long now = new Date().getTime();
				synchronized (usersOnline)
				{		
					synchronized (openGamesByUsername)
					{
						synchronized (openGamesByGameId)
						{
							Set<String> keys = new HashSet<String>(usersOnline.keySet());
							Iterator<String> e = keys.iterator();
							while (e.hasNext())
							{
								String key = e.next();
								UserNode node = usersOnline.get(key);						
								if ((node.getLastActiveDate().getTime() + TIMEOUT) < now)
								{
									usersOnline.remove(key);
									if (openGamesByUsername.containsKey(node.getUsername()))
									{
										GameNode game = openGamesByUsername.remove(node.getUsername());
										openGamesByGameId.remove(game.getId());
									}
								}
							}
						}
					}
				}
			}
		};
		// Run every "period" seconds
		reaper.scheduleAtFixedRate(task, period, period);
	}
	
	protected File baseDir;
	
	/**
	 * 
	 *
	 */
	public RegistrationModule() {
		super();
	}
	
	/**
	 * 
	 */
	public void initialize(String workingDir, String[] configArgs) throws ModuleInitializationException {
		baseDir = new File(workingDir + "users/");
		if (!baseDir.exists())
		{
			if (!baseDir.mkdirs())
				throw new ModuleInitializationException("Cannot create base directory: " + baseDir.getAbsolutePath());
		}
	}

	/**
	 * 
	 */
	public boolean typeSupported(String requestTypeName) {
		//All supported types
	return 	requestTypeName.equalsIgnoreCase(RequestType.LOGON_USER) 	||
			requestTypeName.equalsIgnoreCase(RequestType.LOGOFF_USER) ||
			requestTypeName.equalsIgnoreCase(RequestType.CREATE_USER) ||
			requestTypeName.equalsIgnoreCase(RequestType.DELETE_USER) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.GET_USERS) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.REGISTER_GAME)||
		 	requestTypeName.equalsIgnoreCase(RequestType.UNREGISTER_GAME)  ||
		 	requestTypeName.equalsIgnoreCase(RequestType.JOIN_GAME) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.GET_GAME_STATUS) ||		 	
		 	requestTypeName.equalsIgnoreCase(RequestType.SELECT_CHALLENGER) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.DESELECT_CHALLENGER) ||		 			 	
		 	requestTypeName.equalsIgnoreCase(RequestType.RECEIVED_CHALLENGE)	||
		 	requestTypeName.equalsIgnoreCase(RequestType.ACCEPT_CHALLENGE) || 
			requestTypeName.equalsIgnoreCase(RequestType.ACCEPTED_CHALLENGE); 
	}
	
	/**
	 * 
	 */
	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		String type = req.getType();
		if (type.equalsIgnoreCase(RequestType.LOGON_USER)) {
			logonUser(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.LOGOFF_USER)) {
			logoffUser(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.CREATE_USER)) {
			createUser(req, res);
		}				
		else if (type.equalsIgnoreCase(RequestType.DELETE_USER)) {
			deleteUser(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.GET_USERS)) {
			getUsersOnline(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.REGISTER_GAME)) {
			registerGame(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.UNREGISTER_GAME)) {
			unregisterGame(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.JOIN_GAME)) {
			joinGame(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.GET_GAME_STATUS)) {
			getGameStatus(req, res);
		}		
		else if (type.equalsIgnoreCase(RequestType.SELECT_CHALLENGER)) {
			selectChallenger(req, res); 
		}
		else if (type.equalsIgnoreCase(RequestType.DESELECT_CHALLENGER)) {
			deselectChallenger(req, res); 
		}		
		else if (type.equalsIgnoreCase(RequestType.RECEIVED_CHALLENGE)) {
			receivedChallenge(req, res); 
		}
		else if (type.equalsIgnoreCase(RequestType.ACCEPT_CHALLENGE)) {
			acceptChallenge(req, res); 
		}	
		else if (type.equalsIgnoreCase(RequestType.ACCEPTED_CHALLENGE)) {
			acceptedChallenge(req, res); 
		}			
		else {
			//the request type cannot be handled
			throw new ModuleException (ErrorCode.UNKNOWN_REQUEST_TYPE, ErrorCode.Msg.UNKNOWN_REQUEST_TYPE);
		}
	}
	
	/**
	 * Register the current user with the gameName he/she requests
	 * Respond with the status of this request
	 * 		If sucessful return a secretKey that will be used for the duration of the session
	 * 		else if the request is denied return the corresponding error code
	 * @param req
	 * @param res
	 * @modifies this
	 * @return
	 */
	protected void logonUser(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		String username = req.getHeader(HN_USERNAME);
		String password = req.getHeader(HN_PASSWORD);
		
		if (username == null || password == null || username.trim().equals("") || password.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
			res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
		}
		else
		{
			// Check if this username/password exists
			String encUsername = URLEncoder.encode(username);
			File f = new File(baseDir, encUsername);
			if (f.exists())
			{
				// Now check that the username and password match up
				Properties props = getUserProperties(f);
				if (username.equalsIgnoreCase(props.getProperty("USERNAME")) && 
						password.equals(props.getProperty("PASSWORD")))
				{
					String secretKey = generateSecretKey(username);
					synchronized (usersOnline)
					{
						// See if any games already exist by this user
						if (openGamesByUsername.containsKey(username))
						{
							GameNode game = openGamesByUsername.remove(username);
							openGamesByGameId.remove(game.getId());							
						}
						usersOnline.put(username, new UserNode(secretKey, props));
					}
					res.setHeader(HN_SECRET_KEY, secretKey);
				}
				else
				{
					res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
					res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
				}
			}
			else
			{
				res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
				res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
			}
		}
	}

	protected void logoffUser(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		UserNode node = getCredentials(req, res, false, false);
		if (node != null)
		{
			synchronized (usersOnline)
			{		
				synchronized (openGamesByUsername)
				{
					synchronized (openGamesByGameId)
					{
						usersOnline.remove(node.getUsername());
						if (openGamesByUsername.containsKey(node.getUsername()))
						{
							GameNode game = openGamesByUsername.remove(node.getUsername());
							openGamesByGameId.remove(game.getId());
						}
					}
				}
			}			
		}
	}	
	
	protected void createUser(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		String username = req.getHeader(HN_USERNAME);
		String password = req.getHeader(HN_PASSWORD);
		
		if (username == null || password == null || username.trim().equals("") || password.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
			res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
		}
		else if (username.indexOf(' ') != -1 || username.indexOf(':') != -1)
		{
			res.setReturnCode(ErrorCode.INVALID_USERNAME);
			res.setReturnMessage(ErrorCode.Msg.INVALID_USERNAME);
		}		
		else
		{
			// Check if this username already exists
			String encUsername = URLEncoder.encode(username).toLowerCase();
			File f = new File(baseDir, encUsername);
			if (f.exists())
			{
				res.setReturnCode(ErrorCode.EXISTING_USER);
				res.setReturnMessage(ErrorCode.Msg.EXISTING_USER);
			}
			else
			{
				Properties props = new Properties();
				props.put("USERNAME", username);
				props.put("PASSWORD", password);
				props.put("CREATED DATE", new Date().toGMTString());
				FileOutputStream os = null;
				try
				{
					try
					{
						if (!f.createNewFile())
							throw new ModuleException(ErrorCode.IO_EXCEPTION, "Cannot create file: " + f.getAbsolutePath());
						os = new FileOutputStream(f);
						props.storeToXML(os, null);	
						os.flush();
					}
					finally
					{
						if (os != null)
							os.close();
					}
				}
				catch (IOException ex)
				{
					throw new ModuleException(ErrorCode.IO_EXCEPTION, "Cannot create user: " + username, ex);
				}
			}
			
		}
	}
	
	protected void deleteUser(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		String username = req.getHeader(HN_USERNAME);
		String password = req.getHeader(HN_PASSWORD);
		
		if (username == null || password == null || username.trim().equals("") || password.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
			res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
		}
		else
		{
			// Check if this username/password exists
			String encUsername = URLEncoder.encode(username);
			File f = new File(baseDir, encUsername);
			if (f.exists())
			{
				// Now check that the username and password match up (necessary if we're deleting the user)
				Properties props = getUserProperties(f);
				if (username.equalsIgnoreCase(props.getProperty("USERNAME")) && 
						password.equals(props.getProperty("PASSWORD")))
				{
					if (!f.delete())
						throw new ModuleException(ErrorCode.IO_EXCEPTION, "Cannot delete user file: " + f.getAbsolutePath());
					
					// Remove from usersOnline if logged in
					synchronized (usersOnline)
					{
						if (usersOnline.containsKey(username))
							usersOnline.remove(username);
					}
				}
				else
				{
					res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
					res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
				}
			}
			else
			{
				res.setReturnCode(ErrorCode.NO_EXISTING_USER);
				res.setReturnMessage(ErrorCode.Msg.NO_EXISTING_USER);
			}
		}
	}	
	
	/**
	 * Request the list of players online, filtering based on the requested game
	 * stores in datastream of res
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	protected void getUsersOnline(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		// No need to check for secretKey (this request is for public data)
		UserNode node = getCredentials(req, res, false, true);
		if (node != null)
			node.setPlayingGame(false);
		
		try 
		{
			OutputStream outStream = null;
			try
			{
				outStream = res.getOutputStream();
				synchronized (usersOnline)
				{
					synchronized(openGamesByUsername)
					{
						res.setHeader(HN_LENGTH, String.valueOf(usersOnline.size()));
						long now = new Date().getTime();
						Enumeration<String> e = usersOnline.keys();
						while (e.hasMoreElements())
						{
							String key = e.nextElement();
							UserNode user = usersOnline.get(key);
							Date idleTime = new Date(now - user.getLastActiveDate().getTime());
							String data = user.getUsername() + ":" + (user.isPlayingGame()?"1":"0") + 
								":" + idleTime.getMinutes() + ":" + idleTime.getSeconds();
							if (openGamesByUsername.containsKey(user.getUsername()))
							{
								GameNode game = openGamesByUsername.get(user.getUsername());
								data += ":" + game.getId() + ":" + game.getName() + ":" + 
									game.getVariant() + ":" + game.getDescription();
							}
							data += "\n";

							// Write out the entry
							outStream.write(data.getBytes());						
						}
					}
				}
			}
			finally
			{
				if (outStream != null)
					outStream.close();
			}
		}
		catch (IOException ex) 
		{
			throw new ModuleException(ErrorCode.IO_EXCEPTION, ErrorCode.Msg.IO_EXCEPTION, ex);
		}
	}
	
	/**
	 * Register a new game request. Verify that the user is registered and not
	 * already hosting a game. Also check that the variant is valid
	 * 		On success register the game and respond with ACK status
	 * 		else respond with DENY and corresponding error code
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	protected void registerGame(IModuleRequest req, IModuleResponse res) {

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameName = req.getHeader(HN_GAME_NAME);
		String gameVariant = req.getHeader(HN_GAME_VARIANT);
		String gameDescription = req.getHeader(HN_GAME_DESCRIPTION);
		String gameHostMovesFirst = req.getHeader(HN_GAME_HOST_MOVES_FIRST);		
		boolean gameHostFirst = false;
				
		//Validity Checks
		if (gameName == null || gameName.trim().equals("") ||
				gameVariant == null || gameVariant.trim().equals("") || 
				gameHostMovesFirst == null || gameHostMovesFirst.trim().equals("") ||
				gameDescription == null)
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
		}
		else
		{
			if (gameHostMovesFirst.equalsIgnoreCase("TRUE") || gameHostMovesFirst.equalsIgnoreCase("YES") ||
					gameHostMovesFirst.equalsIgnoreCase("1"))
			{
				gameHostFirst = true;
			}
			
			// Create a new GameNode
			GameNode game = new GameNode(node, gameName, gameVariant, gameDescription, gameHostFirst);
			synchronized (openGamesByUsername)
			{
				synchronized (openGamesByGameId)
				{
					openGamesByUsername.put(node.getUsername(), game);
					openGamesByGameId.put(game.getId(), game);			
					res.setHeader(HN_GAME_ID, game.getId());
				}
			}
		}
	}
	
	/**
	 * Unregister the client's currently hosted game
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	protected void unregisterGame(IModuleRequest req, IModuleResponse res) {

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		GameNode game = null;
		String gameId = req.getHeader(HN_GAME_ID);
		
		//Validity Checks
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
		}
		else
		{
			synchronized (openGamesByUsername)
			{
				if (openGamesByUsername.containsKey(node.getUsername()) && 
						openGamesByUsername.get(node.getUsername()).getId().equals(gameId))
				{
					game = openGamesByUsername.get(node.getUsername());
					if (game.getSelectedPlayer() != null && game.didSelectedPlayerRespond())
					{
						res.setHeader(HN_CHALLENGER_USERNAME, game.getSelectedPlayer().getUsername());
						res.setReturnCode(ErrorCode.PREV_CHALLENGER_ALREADY_ACCEPTED);
						res.setReturnMessage(ErrorCode.Msg.PREV_CHALLENGER_ALREADY_ACCEPTED);
					}
					else
					{
						synchronized (openGamesByGameId)
						{
							openGamesByUsername.remove(node.getUsername());
							openGamesByGameId.remove(game.getId());							
						}								
					}
				}
			}
		}
	}

	/**
	 * Request to join host ganme identified by th GAME_ID header in req
	 * If the host is busy, the guest (client) will be put to sleep and later woken up
	 * by the host along with a notification of whether or not the host selected this client
	 * as a player (accepted the challenge)
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	protected void joinGame(IModuleRequest req, IModuleResponse res) throws ModuleException {

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		
		// Check that the game details are specified
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}
		
		// Look for the game online
		synchronized (openGamesByGameId)
		{
			if (!openGamesByGameId.containsKey(gameId))
			{
				res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
				res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			}
			else
			{
				GameNode game = openGamesByGameId.get(gameId);
				game.addInterestedPlayer(node);
			}
		}
	}

	/**
	 * Used by game host to find interested game opponents
	 * 
	 * Writes the list into the bytestream! (sounds like lifestream)
	 * 
	 * @param req
	 * @param res
	 * @return
	 */
	protected void getGameStatus(IModuleRequest req, IModuleResponse res) throws ModuleException{

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		
		// Check that the game details are specified
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}
		
		// Look for the game online
		synchronized (openGamesByGameId)
		{
			if (!openGamesByGameId.containsKey(gameId))
			{
				res.setReturnCode(ErrorCode.GAME_CLOSED);
				res.setReturnMessage(ErrorCode.Msg.GAME_CLOSED);				
			}
			else
			{
				GameNode game = openGamesByGameId.get(gameId);
				res.setHeader(HN_GAME_ID, game.getId());
				res.setHeader(HN_GAME_NAME, game.getName());
				res.setHeader(HN_GAME_VARIANT, game.getVariant());
				res.setHeader(HN_GAME_DESCRIPTION, game.getDescription());
				
				// If we're getting status, we can be sure that we're not
				// waiting for the selected user to accept. So set the selected
				// user to null (to clean up dropped clients, etc.).
				game.setSelectedPlayer(null);
				
				// Add the interested usernames
				try 
				{
					OutputStream outStream = null;
					try
					{
						outStream = res.getOutputStream();
						long now = new Date().getTime();						
						res.setHeader(HN_LENGTH, String.valueOf(game.getInterestedPlayers().size()));
						Iterator<UserNode> itr = game.getInterestedPlayers().iterator();
						while (itr.hasNext())
						{
							UserNode user = itr.next();
							Date idleTime = new Date(now - user.getLastActiveDate().getTime());
							String data = user.getUsername() + ":" + (user.isPlayingGame()?"1":"0") + 
								":" + idleTime.getMinutes() + ":" + idleTime.getSeconds() + "\n";

							// Write out the entry
							outStream.write(data.getBytes());						
						}
					}
					finally
					{
						if (outStream != null)
							outStream.close();
					}
				}
				catch (IOException ex) 
				{
					throw new ModuleException(ErrorCode.IO_EXCEPTION, ErrorCode.Msg.IO_EXCEPTION, ex);
				}					
			}
		}	
	}	
		
	/**
	 * Request from game host to accept/decline game challenge
	 * Host indicates whether or not to play against the first opponent to join
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	protected void selectChallenger(IModuleRequest req, IModuleResponse res) {
		 
		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		String challengerUsername = req.getHeader(HN_CHALLENGER_USERNAME);
		
		// Check that the details are specified
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}
		if (challengerUsername == null || challengerUsername.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_CHALLENGER_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_CHALLENGER_INFO);
			return;
		}
		
		// Look for the game online
		synchronized (usersOnline)
		{
			synchronized (openGamesByGameId)
			{
				if (!openGamesByGameId.containsKey(gameId))
				{
					res.setReturnCode(ErrorCode.GAME_CLOSED);
					res.setReturnMessage(ErrorCode.Msg.GAME_CLOSED);				
				}
				else
				{
					UserNode challenger = null;
					GameNode game = openGamesByGameId.get(gameId);
					
					// First check if there's already a selected player that's accepted and still online
					if (game.getSelectedPlayer() != null && game.didSelectedPlayerRespond() && 
							game.didSelectedPlayerAccept() && usersOnline.containsKey(challengerUsername))
					{
						res.setHeader(HN_CHALLENGER_USERNAME, game.getSelectedPlayer().getUsername());
						res.setReturnCode(ErrorCode.PREV_CHALLENGER_ALREADY_ACCEPTED);
						res.setReturnMessage(ErrorCode.Msg.PREV_CHALLENGER_ALREADY_ACCEPTED);									
					}
					else
					{
						// OK, select this new challenger
						ListIterator<UserNode> itr = game.getInterestedPlayers().listIterator();
						while (itr.hasNext())
						{
							UserNode user = itr.next();
							if (user.getUsername().equalsIgnoreCase(challengerUsername))
							{
								challenger = user;
								itr.remove();
								break;
							}
						}
						
						if (challenger == null)
						{
							res.setReturnCode(ErrorCode.INVALID_CHALLENGER_INFO);
							res.setReturnMessage(ErrorCode.Msg.INVALID_CHALLENGER_INFO);
						}
						else
						{
							game.setSelectedPlayer(challenger);
						}
					}
				}
			}
		}		
	}
	
	/**
	 * Request from game host to accept/decline game challenge
	 * Host indicates whether or not to play against the first opponent to join
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	protected void deselectChallenger(IModuleRequest req, IModuleResponse res) {
		 
		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		
		// Check that the details are specified
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}
		
		// Look for the game online
		synchronized (openGamesByGameId)
		{
			if (!openGamesByGameId.containsKey(gameId))
			{
				res.setReturnCode(ErrorCode.GAME_CLOSED);
				res.setReturnMessage(ErrorCode.Msg.GAME_CLOSED);				
			}
			else
			{
				GameNode game = openGamesByGameId.get(gameId);
				
				// First check if there's already a selected player that's accepted
				if (game.getSelectedPlayer() != null && game.didSelectedPlayerRespond() && game.didSelectedPlayerAccept())
				{
					res.setHeader(HN_CHALLENGER_USERNAME, game.getSelectedPlayer().getUsername());
					res.setReturnCode(ErrorCode.PREV_CHALLENGER_ALREADY_ACCEPTED);
					res.setReturnMessage(ErrorCode.Msg.PREV_CHALLENGER_ALREADY_ACCEPTED);									
				}
				else
				{
					// OK, deselect the challenger
					game.setSelectedPlayer(null);
				}
			}
		}		
	}
	
	protected void receivedChallenge(IModuleRequest req, IModuleResponse res) throws ModuleException {

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		
		// Check that the game details are specified
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}
		
		// Look for the game online
		synchronized (openGamesByGameId)
		{
			if (!openGamesByGameId.containsKey(gameId))
			{
				res.setReturnCode(ErrorCode.GAME_CLOSED);
				res.setReturnMessage(ErrorCode.Msg.GAME_CLOSED);				
			}
			else
			{
				GameNode game = openGamesByGameId.get(gameId);
				if (game.getSelectedPlayer() == null)
				{
					res.setHeader(HN_STATUS, "2");
				}
				else
				{
					if (game.getSelectedPlayer() != node)
					{
						if (game.getInterestedPlayers().contains(node))
						{
							res.setHeader(HN_STATUS, "2");
						}
						else
						{
							res.setHeader(HN_STATUS, "0");
						}						
					}
					else
					{
						res.setHeader(HN_STATUS, "1");
					}
				}
			}
		}
	}
	
	protected void acceptChallenge(IModuleRequest req, IModuleResponse res) throws ModuleException {

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		String accept = req.getHeader(HN_ACCEPT);	
		boolean willAccept = false;
		
		// Check that the game details are specified
		if (gameId == null || gameId.trim().equals("") || 
				accept == null || accept.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}

		// Get the accept value
		if (accept.equalsIgnoreCase("TRUE") || accept.equalsIgnoreCase("YES") ||
				accept.equalsIgnoreCase("1"))
		{
			willAccept = true;
		}		
		
		// Look for the game online
		synchronized (openGamesByGameId)
		{
			if (!openGamesByGameId.containsKey(gameId))
			{
				res.setReturnCode(ErrorCode.GAME_CLOSED);
				res.setReturnMessage(ErrorCode.Msg.GAME_CLOSED);				
			}
			else
			{
				GameNode game = openGamesByGameId.get(gameId);
				if (game.getSelectedPlayer() != node)
				{
					res.setReturnCode(ErrorCode.OFFER_REVOKED);
					res.setReturnMessage(ErrorCode.Msg.OFFER_REVOKED);				
				}
				else
				{
					// OK, set the accept flag
					game.setSelectedPlayerAccept(willAccept);
					if (willAccept)
					{
						// Identify player as playing a game
						node.setPlayingGame(true);
						
						// Start the game and let the player know if he/she is to move first
						if (game.doesHostMoveFirst())
						{
							P2PModule.startGame(gameId, game.getHost(), node);
							res.setHeader(HN_MOVE_FIRST, "0");							
						}
						else
						{
							P2PModule.startGame(gameId, node, game.getHost());
							res.setHeader(HN_MOVE_FIRST, "1");							
						}						
					}
					else
					{
						game.setSelectedPlayer(null);
					}
				}
			}
		}
	}	
	
	protected void acceptedChallenge(IModuleRequest req, IModuleResponse res) throws ModuleException {

		// Get the user
		UserNode node = getCredentials(req, res, true, true);
		if (node == null)
			return;
		else
			node.setPlayingGame(false);
		
		String gameId = req.getHeader(HN_GAME_ID);
		
		// Check that the game details are specified
		if (gameId == null || gameId.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_GAME_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_GAME_INFO);
			return;
		}
		
		// Look for the game online
		synchronized (usersOnline)
		{
			synchronized (openGamesByUsername)
			{
				synchronized (openGamesByGameId)
				{
					if (!openGamesByGameId.containsKey(gameId))
					{
						res.setReturnCode(ErrorCode.GAME_CLOSED);
						res.setReturnMessage(ErrorCode.Msg.GAME_CLOSED);				
					}
					else
					{
						GameNode game = openGamesByGameId.get(gameId);
						if (game.getSelectedPlayer() == null)
						{
							res.setHeader(HN_STATUS, "0");
						}
						else if (!usersOnline.containsKey(game.getSelectedPlayer().getUsername()))
						{
							res.setHeader(HN_STATUS, "0");
						}
						else 
						{
							if (!game.didSelectedPlayerRespond())
							{
								res.setHeader(HN_STATUS, "2");
							}					
							else
							{
								if (!game.didSelectedPlayerAccept())
								{
									res.setHeader(HN_STATUS, "0");
								}
								else
								{
									res.setHeader(HN_STATUS, "1");
									
									// OK, start the game
									openGamesByGameId.remove(gameId);
									openGamesByUsername.remove(game.getHost().getUsername());
									
									// Identify host player as playing a game
									node.setPlayingGame(true);
									
									// Already handed off to the p2p module
									res.setHeader(HN_MOVE_FIRST, (game.doesHostMoveFirst()?"1":"0"));							
								}
							}
						}
					}
				}
			}
		}
	}
	
	public static UserNode getCredentials(IModuleRequest req, IModuleResponse res, 
			boolean respondErrorIfMissing, boolean updateActiveIfFound) {

		String username = req.getHeader(HN_USERNAME);
		String password = req.getHeader(HN_PASSWORD);
		String secretKey = req.getHeader(HN_SECRET_KEY);
		
		// Check that the credentials are specified
		if (username == null || username.trim().equals("") || 
				password == null || password.trim().equals("") || 
				secretKey == null || secretKey.trim().equals(""))
		{
			if (respondErrorIfMissing)
			{
				res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
				res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
			}
			return null;
		}
		
		// Check the secretKey
		if (!isSecretKeyValid(username, secretKey))
		{
			if (respondErrorIfMissing)
			{
				res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
				res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
			}
			return null;
		}
		
		// Look for the user online
		UserNode node = null;
		synchronized (usersOnline)
		{
			node = usersOnline.get(username);
			if (node != null)
			{
				// Verify that the username, password, and secretKey all match up
				if (!username.equalsIgnoreCase(node.getUsername()) || 
						!password.equals(node.getPassword()) || 
						!secretKey.equals(node.getSecretKey()))
				{
					if (respondErrorIfMissing)
					{
						res.setReturnCode(ErrorCode.INVALID_CREDENTIALS);
						res.setReturnMessage(ErrorCode.Msg.INVALID_CREDENTIALS);
					}
					return null;
				}
				else
				{
					// Update the last active date
					if (updateActiveIfFound)
						node.updateLastActiveDate();
				}
			}
			else
			{
				if (respondErrorIfMissing)
				{
					// No user found logged in
					res.setReturnCode(ErrorCode.NO_USER_FOUND);
					res.setReturnMessage(ErrorCode.Msg.NO_USER_FOUND);
				}
			}
		}
		return node;
	}
	
	protected Properties getUserProperties(File f) throws ModuleException
	{
		Properties props = new Properties();
		FileInputStream is = null;
		try
		{
			try
			{
				is = new FileInputStream(f);
				props.loadFromXML(is);
				return props;
			}
			finally
			{
				if (is != null)
					is.close();
			}
		}
		catch (IOException ex)
		{
			throw new ModuleException(ErrorCode.IO_EXCEPTION, "Cannot read user file: " + f.getAbsolutePath(), ex);
		}
	}
	
	/**
	 * Convert the key to a string
	 * @param user
	 * @return
	 */
	protected static String generateSecretKey(String user) {
		return (new Integer(user.hashCode())).toString();
	}

	/**
	 * Verify that secretKey corresponds to userName
	 * @param userName
	 * @param secretKey
	 * @return
	 */
	public static boolean isSecretKeyValid(String userName, String secretKey) {
		try
		{
			Integer.parseInt(secretKey);
			return true;
		}
		catch (NumberFormatException ex)
		{
			return false;
		}
	}
}