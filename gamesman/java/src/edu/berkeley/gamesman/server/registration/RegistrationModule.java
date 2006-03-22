package edu.berkeley.gamesman.server.registration;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.LinkedList;
import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;

/**
 * 
 * @author Victor Perez
 *
 */
public class RegistrationModule implements IModule
{

	/**
	 * 
	 *
	 */
	public RegistrationModule() {
		super();
		usersOnline = new Hashtable();
		//this hashtable may need to be populated with a list of valid game names
		openGames = new Hashtable();
	}
	
	/**
	 * 
	 */
	public void initialize(String[] configArgs) throws ModuleInitializationException {
		return;
	}

	/**
	 * 
	 */
	public boolean typeSupported(String requestTypeName) {
		//All supported types
	return 	requestTypeName.equals(Macros.REG_MOD_REGISTER_USER) 	||
		 	requestTypeName.equals(Macros.REG_MOD_GET_USERS_ONLINE) ||
		 	requestTypeName.equals(Macros.REG_MOD_GET_OPEN_GAMES) 	||
		 	requestTypeName.equals(Macros.REG_MOD_REGISTER_NEW_GAME)||
		 	requestTypeName.equals(Macros.REG_MOD_UNREGISTER_GAME)  ||
		 	requestTypeName.equals(Macros.REG_MOD_JOIN_GAME_NUMBER) ||
		 	requestTypeName.equals(Macros.REG_MOD_JOIN_GAME_USER)	||
		 	requestTypeName.equals(Macros.REG_MOD_REFRESH_STATUS)	||
		 	requestTypeName.equals(Macros.REG_MOD_ACCEPT_CHALLENGE) || 
		 	requestTypeName.equals(Macros.REG_MOD_UNREGISTER_USER); 
	}
	
	/**
	 * 
	 */
	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		String type;
		IModuleRequest mreq = req;
		IModuleResponse mres =  res;
		type = mreq.getHeader(Macros.TYPE);
		if (type == Macros.REG_MOD_REGISTER_USER) {
			registerUser(mreq, mres);
		}
		else if (type == Macros.REG_MOD_GET_USERS_ONLINE) {
			getUsersOnline(mreq, mres);
		}
		else if (type == Macros.REG_MOD_GET_OPEN_GAMES) {
			getOpenGames(mreq, mres);
		}
		else if (type == Macros.REG_MOD_REGISTER_NEW_GAME) {
			registerNewGame(mreq, mres);
		}
		else if (type == Macros.REG_MOD_UNREGISTER_GAME) {
			unregisterGame(mreq, mres);
		}
		else if (type == Macros.REG_MOD_JOIN_GAME_NUMBER) {
			joinGameNumber(mreq, mres);
		}
		else if (type == Macros.REG_MOD_JOIN_GAME_USER) {
			//Ambiguous meaning in the future if we implement multiple
			//open-games per user. So might as well make it a client-side
			//feature, instead of a dedicated server request. 
		}
		else if (type == Macros.REG_MOD_REFRESH_STATUS) {
			refreshHostStatus(mreq, mres); 
		}
		else if (type == Macros.REG_MOD_ACCEPT_CHALLENGE) {
			acceptChallenge(mreq, mres); 
		}
		else if (type == Macros.REG_MOD_UNREGISTER_USER) {
			unregisterUser(mreq, mres);
		}
		else {
			//the request type cannot be handled
			throw new ModuleException (Macros.UNKNOWN_REQUEST_TYPE_CODE, Macros.UNKNOWN_REQUEST_TYPE_MSG);
		}
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	private void joinGameNumber(IModuleRequest req, IModuleResponse res)  throws ModuleException {
		String userName, secretKey, gameID, gameName;
		boolean validKey;
		LinkedList interestedUsers;
		Hashtable gameSessions;
		PropertyBucket propBucket;
		
		userName = req.getHeader(Macros.NAME);
		secretKey = req.getHeader(Macros.SECRET_KEY);
		gameID = req.getHeader(Macros.GAME_ID);
		
		validKey = isValidUserKey(userName, secretKey);
		gameName = (String)usersOnline.get(userName);
		
		gameSessions = (Hashtable)openGames.get(gameName);
		
		propBucket = (PropertyBucket)gameSessions.get(new Integer(gameID));
		if (propBucket == null) {
			res.setHeader(Macros.STATUS, Macros.DENY);
			res.setHeader(Macros.ERROR_CODE, Macros.GENERIC_ERROR_CODE.toString());
			return;
		}
		else {
			interestedUsers = (LinkedList)propBucket.getProperty(Macros.PROPERTY_INTERESTED_USERS);
			interestedUsers.add(this);
			try {
				// go to sleep and wait for another thread to wake us up when they refresh
				this.wait();
			}
			catch (InterruptedException ie) {
				throw new ModuleException(Macros.INTERRUPT_EXCEPTION_CODE, Macros.INTERRUPT_EXCEPTION_MSG);
			}
			res.setHeader(Macros.STATUS, Macros.ACK);
			
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
	private void registerUser(IModuleRequest req, IModuleResponse res) throws ModuleException {
		String userName, gameName, status, secretKey;
		Integer checkStatus;
		
		//get userName and gameName from the request object
		userName = req.getHeader(Macros.NAME);
		gameName = req.getHeader(Macros.GAME);
		
		//Name check successful
		if ((checkStatus = checkName(userName)).equals(Macros.VALID_CODE)) {
			addUser(userName, gameName);
			status = Macros.ACK;
			secretKey = (String)((PropertyBucket)getUser(userName)).getProperty(Macros.PROPERTY_SECRET_KEY);
			res.setHeader(Macros.SECRET_KEY, secretKey);
			res.setHeader(Macros.STATUS, status);
		}
		else {
			status = Macros.DENY;
			res.setHeader(Macros.STATUS, status);
			res.setHeader(Macros.ERROR_CODE, checkStatus.toString());
		}
	}
	
	/**
	 * Request the list of players online, filtering based on the requested game
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	private void getUsersOnline(IModuleRequest req, IModuleResponse res) throws ModuleException {
		String gameName, onlineUser, onlineGame;
		OutputStream outStream;
		Enumeration users;
		byte [] byteArr;
		PropertyBucket propBucket;
		
		try {
			outStream = res.getOutputStream();
		}
		catch (IOException ioe) {
			throw new ModuleException(Macros.IO_EXCEPTION_CODE, Macros.IO_EXCEPTION_TYPE_MSG);
		}
		
		/**
		 * Get the name of game being requested and write each user to the output stream
		 * delimited with a newline
		 */
		gameName = req.getHeader(Macros.GAME);
		for (users = usersOnline.keys(); users.hasMoreElements();) {
			onlineUser = (String)users.nextElement();
			propBucket = (PropertyBucket) usersOnline.get(onlineUser);
			onlineGame = (String)propBucket.getProperty(Macros.GAME);
			
			if (onlineGame.equals(gameName)) {
				onlineUser += "\n";
				byteArr = onlineUser.getBytes();
				try {
					outStream.write(byteArr);
				}
				catch (IOException ioe) {
					throw new ModuleException(Macros.IO_EXCEPTION_CODE, Macros.IO_EXCEPTION_TYPE_MSG);
				}
			}
		}
	}
	
	/**
	 * Serach all available open games and filter for the one the client specifies
	 * Add headers with the properties of all avaliable games that meet the criteria
	 * Properties are distinguished by the concatenation of an index after the property
	 * name
	 * @param req
	 * @param res
	 * @return
	 */
	private void getOpenGames(IModuleRequest req, IModuleResponse res) {
		String gameName, host, variationNumber; 
		Integer gameID;
		int index;
		Hashtable gameSessions;
		Enumeration gameSessionTable;
		PropertyBucket propBucket;
		
		//filter the game client is looking for
		gameName = req.getHeader(Macros.GAME);
		
		//get all sessions of that particular game
		gameSessions = (Hashtable)openGames.get(gameName);
		if (gameSessions == null) {
			res.setHeader(Macros.GAME_SESSIONS_INDEX, (new Integer(0).toString()));
			return; 
		}
		for (index = 0, gameSessionTable = gameSessions.keys(); 
						gameSessionTable.hasMoreElements(); index++) {
			
			//list of properties descibing this game session instance
			gameID = (Integer) gameSessionTable.nextElement();
			propBucket = (PropertyBucket)gameSessions.get(gameID);
			host = (String)propBucket.getProperty(Macros.PROPERTY_HOST);
			variationNumber = (String)propBucket.getProperty(Macros.PROPERTY_VARIATION);
			
			//add headers to response
			//note that index is necessary since there will be several game sessions
			res.setHeader(Macros.PROPERTY_GAME_ID + index, gameID.toString());
			res.setHeader(Macros.PROPERTY_HOST + index, host);
			res.setHeader(Macros.PROPERTY_VARIATION + index, variationNumber);
		}
		//the client will need to know how many headers to extract, that's what this
		//index is for
		res.setHeader(Macros.GAME_SESSIONS_INDEX, (new Integer(index).toString()));
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
	private void registerNewGame(IModuleRequest req, IModuleResponse res) {
		String userName, secretKey, variation, gameMessage, gameName;
		boolean validKey, notHostingGame, validVariant;
		Integer gameID;
		//used to descibe the particular game session to be hosted
		PropertyBucket propBucket;
		
		//extract request headers
		userName = req.getHeader(Macros.NAME);
		secretKey= req.getHeader(Macros.SECRET_KEY);
		variation = req.getHeader(Macros.VARIATION);
		gameMessage = req.getHeader(Macros.GAME_MESSAGE);
		
		//Validity Checks
		propBucket = getUser(userName);
		/*
		 * Because userName may not exist propBucket may be null and we have to check for that
		 */
		if (propBucket == null) gameName = null;
		else gameName = (String)propBucket.getProperty(Macros.GAME);
		
		//Verify secretKey/userName
		validKey = isValidUserKey(userName, secretKey);
		//check that the user is not already hosting a game
		notHostingGame = isNotHostingGame(userName);
		//at the moment this condition is not actually being checked so validVariant is always true
		//but we definitely need a way to verify
		validVariant = isValidVariant(gameName, variation);
		if (validKey && notHostingGame && validVariant) {
			//client will now be hosting game
			hostGame(userName, gameName);
			
			//populate property bucket
			propBucket.setProperty(Macros.PROPERTY_HOST, userName);
			propBucket.setProperty(Macros.PROPERTY_INTERESTED_USERS, new LinkedList());
			propBucket.setProperty(Macros.PROPERTY_VARIATION, variation);
			propBucket.setProperty(Macros.PROPERTY_GAME_MESSAGE, gameMessage);
			
			//generate gameID and add the new game session to online games
			gameID = new Integer (generateGameID());
			//Nasty bug was killed here...I was forgetting to store the gameID in the property bucket
			propBucket.setProperty(Macros.PROPERTY_GAME_ID, gameID);
			addGameSession(gameName, gameID, propBucket);
			
			//At this point the game has been registered successfully so respond with Macros.ACK
			res.setHeader(Macros.STATUS, Macros.ACK);
		}
		else {
			//the request has failed
			res.setHeader(Macros.STATUS, Macros.DENY);
			if (!validKey) res.setHeader(Macros.ERROR_CODE, Macros.INVALID_KEY.toString());
			else if (!notHostingGame) res.setHeader(Macros.ERROR_CODE, Macros.USER_ALREADY_HAS_OPEN_GAME.toString());
			else if (!validVariant) res.setHeader(Macros.ERROR_CODE, Macros.INVALID_VARIANT.toString());
		}
	}
	
	/**
	 * Unregister the client's currently hosted game
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	private void unregisterGame(IModuleRequest req, IModuleResponse res) {
		String userName, secretKey, gameName;
		boolean validKey, validGameHost;
		Integer gameID;
		PropertyBucket propBucket;
		
		//extract header values
		userName = req.getHeader(Macros.NAME);
		secretKey = req.getHeader(Macros.SECRET_KEY);
		validKey = isValidUserKey(userName, secretKey);
		validGameHost = isValidGameHost(userName);
		if (validKey && validGameHost) {
			//remove userName/game session records
			propBucket = getUser(userName);
			gameName = (String)propBucket.getProperty(Macros.GAME);
			gameID = (Integer)propBucket.getProperty(Macros.PROPERTY_GAME_ID);
			
			//remove all game host mappings
			//TODO: do we want to remove the user from usersOnline or just from hosting
			removeGameSession(gameName,gameID);
			stopHostingGame(userName);
			
			//take out everything added to the bucket during game registration
			propBucket.removeProperty(Macros.PROPERTY_HOST);
			propBucket.removeProperty(Macros.PROPERTY_INTERESTED_USERS);
			propBucket.removeProperty(Macros.PROPERTY_VARIATION);
			propBucket.removeProperty(Macros.PROPERTY_GAME_MESSAGE);
			propBucket.removeProperty(Macros.PROPERTY_GAME_ID);
			
			//request was successful
			res.setHeader(Macros.STATUS, Macros.ACK);
		}
		else {
			//request has failed
			res.setHeader(Macros.STATUS, Macros.DENY);
			//TODO: make the error code more specific
			res.setHeader(Macros.ERROR_CODE, Macros.GENERIC_ERROR_CODE.toString());
		}
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	private void unregisterUser(IModuleRequest req, IModuleResponse res) {
		String userName, secretKey;
		boolean validKey; 
		userName = req.getHeader(Macros.NAME);
		secretKey = req.getHeader(Macros.SECRET_KEY);
		validKey = isValidUserKey(userName, secretKey);
		if (validKey) { 
			if (isValidGameHost(userName)) {
				unregisterGame(req, res); 
			}
			removeUser(userName); 
			res.setHeader(Macros.STATUS, Macros.ACK);
		} else {
			//request has failed
			res.setHeader(Macros.STATUS, Macros.DENY);
			res.setHeader(Macros.ERROR_CODE, Macros.INVALID_KEY.toString());
		}
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	private void acceptChallenge(IModuleRequest req, IModuleResponse res) {
		String userName, secretKey, luckyUser, gameName, gameId, challengeResponse;
		Hashtable gameSessions; 
		PropertyBucket propBucket; 
		LinkedList interestedList; 
		boolean validKey, hostAgreed; 
		userName = req.getHeader(Macros.NAME);
		secretKey = req.getHeader(Macros.SECRET_KEY);
		validKey = isValidUserKey(userName, secretKey);
		//TODO: for now challenge response will be empty string, but it needs to be initialized
		challengeResponse = req.getHeader(Macros.CHALLENGE_ACCEPTED);
		if (validKey) {
			hostAgreed = (challengeResponse == Macros.ACCEPTED); 
			if (hostAgreed) {
				//host said yes, so we let the other users know that they
				//got denied, inform the P2P module of the new game, let
				//the selected client know he's been accepted, unregister
				//the open-game, and finally give the host the ack signel. 
				gameName = (String)usersOnline.get(userName);
				gameSessions = (Hashtable)openGames.get(gameName); 
				gameId = (String) req.getHeader(Macros.GAME_ID); 
				propBucket = (PropertyBucket) gameSessions.get(gameId); 
				interestedList = (LinkedList) propBucket.getProperty(Macros.PROPERTY_INTERESTED_USERS);
				luckyUser = (String) interestedList.removeFirst(); 
				//TODO: uncomment the following line ... commented out for compilation sake
				//edu.berkeley.gamesman.server.p2p.P2PModule.registerNewGame(userName, luckyUser); 
				//TODO: Wake up the luckyUser here
				//TODO: Wake up the other users with the bad news
				unregisterGame(req, res);
				res.setHeader(Macros.STATUS, Macros.ACK);
			} else {
				//host said no, so we need to let that client know that he 
				//was denied, and take him off of the list of interested-clients
				//so that on the next call to refresh, the server gets someone
				//new instead. Do we need to lock access to the list, if a 
				//client tries to add himself at the same time as the servlet
				//is removing the front of the list? 
				gameName = (String)usersOnline.get(userName);
				gameSessions = (Hashtable)openGames.get(gameName); 
				gameId = (String) req.getHeader(Macros.GAME_ID); 
				propBucket = (PropertyBucket) gameSessions.get(gameId); 
				interestedList = (LinkedList) propBucket.getProperty(Macros.PROPERTY_INTERESTED_USERS);
				interestedList.removeFirst(); 
				res.setHeader(Macros.STATUS, Macros.ACK);
			}
		} else {
			//request has failed
			res.setHeader(Macros.STATUS, Macros.DENY);
			//TODO: make the error code more specific
			res.setHeader(Macros.ERROR_CODE, Macros.GENERIC_ERROR_CODE.toString());
		}
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	private void refreshHostStatus(IModuleRequest req, IModuleResponse res) {
		String userName, secretKey, luckyUser, gameName, gameId;
		Hashtable gameSessions; 
		PropertyBucket propBucket; 
		LinkedList interestedList; 
		boolean validKey, validHost; 
		userName = req.getHeader(Macros.NAME);
		secretKey = req.getHeader(Macros.SECRET_KEY);
		validKey = isValidUserKey(userName, secretKey);
		validHost = isValidGameHost(userName); 
		//Client should not call refresh unless it thinks it's a game host. 
		//We could change this easily. 
		if (validKey && validHost) { 
			gameName = (String)usersOnline.get(userName);
			gameSessions = (Hashtable)openGames.get(gameName); 
			gameId = (String) req.getHeader(Macros.GAME_ID); 
			propBucket = (PropertyBucket) gameSessions.get(gameId); 
			interestedList = (LinkedList) propBucket.getProperty(Macros.PROPERTY_INTERESTED_USERS);
			luckyUser = (String) interestedList.getFirst();
			//Being careful here! If there -isn't- anyone waiting, getFirst I think
			//returns null, which we'll send along. The client will have to then
			//have to interpret an ACK/Null combo as "no-one's waiting", no? 
			res.setHeader(Macros.OPPONENT_USERNAME, luckyUser); 
			res.setHeader(Macros.STATUS, Macros.ACK);
		} else {
			//request has failed
			res.setHeader(Macros.STATUS, Macros.DENY);
			//TODO: make the error code more specific
			res.setHeader(Macros.ERROR_CODE, Macros.GENERIC_ERROR_CODE.toString());
		}
	}
	
	/**
	 * Add a new game session under gameName described by propBucket and gameID 
	 * @param gameName
	 * @param gameID
	 * @param propBucket
	 * @return
	 * @modifies this
	 */
	private void addGameSession(String gameName, Integer gameID, PropertyBucket propBucket) {
		Hashtable gameSessions;
		gameSessions = getGameSessions(gameName);
		gameSessions.put(gameID, propBucket);
	}
	
	/**
	 * Remove the current gameName session correponding to gameID
	 * @param gameName
	 * @param gameID
	 * @return
	 * @modifies this
	 */
	private void removeGameSession(String gameName, Integer gameID) {
		Hashtable gameSessions;
		gameSessions = getGameSessions(gameName);
		gameSessions.remove(gameID);
	}
	
	/**
	 * Search open games for all game sessions of type gameName
	 * Note if a game sessions doesn't exit create a new structure to hold them
	 * @param gameName
	 * @return a gameSessions data structure with all games of type gameName
	 */
	private Hashtable getGameSessions(String gameName) {
		Hashtable gameSessions;
		gameSessions = (Hashtable)openGames.get(gameName);
		if (gameSessions == null) {
			addGameSessions(gameName);
			gameSessions = (Hashtable) openGames.get(gameName);
		}
		return gameSessions;
	}
	
	/**
	 * Add a new data structure to store all game sessions of type gameName
	 * @param gameName
	 * @return
	 * @modifies this
	 */
	private void addGameSessions(String gameName) {
		openGames.put(gameName, new Hashtable());
	}
	
	/**
	 * Add a mapping to indicate that userName will be hosting gameName
	 * @param userName
	 * @param gameName
	 * @return
	 * @modifies this
	 */
	private void hostGame(String userName, String gameName) {
		PropertyBucket propBucket = getUser(userName);
		propBucket.setProperty(Macros.PROPERTY_HOSTING_GAME, gameName);
	}
	
	/**
	 * Remove the current game host mapping
	 * @param userName
	 * @return
	 * @modifies this
	 */
	private void stopHostingGame(String userName) {
		PropertyBucket propBucket = getUser(userName);
		propBucket.removeProperty(Macros.PROPERTY_HOSTING_GAME);
	}
	
	/**
	 * Check whether userName is hosting a game
	 * @param userName
	 * @return whether or not userName is a game host
	 */
	private boolean isValidGameHost(String userName) {
		PropertyBucket propBucket = getUser(userName);
		return propBucket.getProperty(Macros.PROPERTY_HOSTING_GAME) != null;
	}
	
	/**
	 * Generate a unique game id for a game
	 * @return gameID
	 */
	private static int generateGameID() {
		/**
		 * TODO: deal with overflow, possibly generate gameID another way
		 * It is very unlikely that billions of users will be playing at the same time,
		 * but we can't overlook this
		 */
		return gameID++;
	}
	
	/**
	 * Check that variation is a valid gameName type
	 * @param gameName
	 * @param variation
	 * @return whether or not variation is valid
	 */
	private boolean isValidVariant(String gameName, String variation) {
		/**
		 * Check that gameName is not null
		 * For now just return true, but we need to figure out how to tell
		 * whether or not the user has specified the correct variation
		 * maybe the client side should take care of this before sending a request -Victor
		 * [We don't want bad people to inject bad games though. Never trust the
		 * client, is the rule. -Filip] 
		 */
		return true;
	}
	
	/**
	 * Make sure name is not duplicated or 
	 * "invalid" (characters, eventually include vulgarity)
	 * @param name
	 * @return
	 */
	private Integer checkName(String name) {
		/**
		 * check that name is not being duplicated
		 */
		if (isUserOnline(name)) return Macros.USER_ALREADY_EXISTS;
		else return Macros.VALID_CODE;
	}
	
	/**
	 * Return whether or not userName is already hosting a game
	 * @param userName
	 * @return 	true if user not hosting a game
	 * 			false otherwise
	 */
	private boolean isNotHostingGame(String userName) {
		return !isValidGameHost(userName);
	}
	
	/**
	 * Verify that secretKey corresponds to userName
	 * @param userName
	 * @param secretKey
	 * @return
	 */
	protected boolean isValidUserKey(String userName, String secretKey) {
		String key;
		PropertyBucket propBucket;
		/**
		 * We must verify that the user even exists
		 * The key can't possibly be valid if the user doesn't exist
		 */
		if ((propBucket = getUser(userName)) == null) return false;
		key = (String) propBucket.getProperty(Macros.PROPERTY_SECRET_KEY);
		return key.equals(secretKey);
	}
	
	/**
	 * Add a mapping with the given user and game
	 * @param name
	 * @param game
	 * @return
	 * @modifies this
	 */
	private void addUser(String userName, String gameName) throws ModuleException {
		/**
		 * Modifying so that userOnline is now a Hashtable mapping userNames 
		 * to PropertyBucket objects
		 */
		PropertyBucket propBucket = new PropertyBucket();
		String secretKey;
		/**
		 * Check that we are not trying to add a user that's already in the table
		 * This should have already been checked implicitly by the callee; this is only
		 * for debugging.
		 */
		if (Macros.REG_MOD_DEBUGGING && isUserOnline(userName)) 
			throw new ModuleException(Macros.HASHTABLE_COLLISION_CODE, Macros.HASHTABLE_COLLISION_MSG);
		
		//store the game name
		propBucket.setProperty(Macros.GAME, gameName);
		//in order to avoid using another data structure for user secret keys
		//add it to the property bucket
		secretKey = generateKeyString(userName);
		propBucket.setProperty(Macros.PROPERTY_SECRET_KEY, secretKey);
		
		//map user name to corresponding property bucket
		usersOnline.put(userName, propBucket);	
	}
	
	/**
	 * Remove the userName, property bucket mapping from usersOnline
	 * @param userName
	 * @return 	the property bucket to which userName was mapped
	 * 			or null if a mapping did not exist
	 * @modifies this
	 */
	private PropertyBucket removeUser(String userName) {
		return (PropertyBucket)usersOnline.remove(userName);
	}
	
	/**
	 * Return the corresponding property bucket associated with userName
	 * in the usersOnline hashtable or null if the mapping does not exist
	 * @param userName
	 * @return
	 */
	private PropertyBucket getUser(String userName) {
		return (PropertyBucket)usersOnline.get(userName);
	}
	
	/**
	 * 
	 * @param userName
	 */
	protected boolean isUserOnline(String userName) {
		return (usersOnline.get(userName) != null);
	}
	
	/**
	 * Get the hash value of the user string to use as secret key
	 * @param user
	 * @return
	 */
	private static int generateKey(String user) {
		//for now just return the user String hashCode
		return user.hashCode();
	}
	
	/**
	 * Convert the key to a string
	 * @param user
	 * @return
	 */
	private static String generateKeyString(String user) {
		return (new Integer(generateKey(user))).toString();
	}
	
	//fields
	private Hashtable usersOnline, openGames;
	private static int gameID;
	
	/**
	 * 
	 * @author Victor Perez
	 *
	 */
	private class PropertyBucket {
		
		/**
		 * 
		 *
		 */
		public PropertyBucket() {
			properties = new Hashtable();
		}
		
		/**
		 * 
		 * @param propertyName
		 * @param property
		 */
		public void setProperty(String propertyName, Object property) {
			properties.put(propertyName, property);
		}
		
		/**
		 * 
		 * @param propertyName
		 * @return
		 */
		public Object getProperty(String propertyName) {
			return properties.get(propertyName);
		}
		
		/**
		 * 
		 * @param propertyName
		 */
		public void removeProperty(String propertyName) {
			properties.remove(propertyName);
		}
		
		/**
		 * 
		 *
		 */
		public void removeAllProperties() {
			properties = new Hashtable();
		}
		
		private Hashtable properties;
	}
}
