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
import edu.berkeley.gamesman.server.ModuleRequest;
import edu.berkeley.gamesman.server.ModuleResponse;

/**
 * 
 * @author vperez
 *
 */
public class RegistrationModule implements IModule
{

	public RegistrationModule()
	{
		super();
		// TODO Auto-generated constructor stub
		usersOnline = new Hashtable();
		secretKeys =  new Hashtable();
		
		//this hashtable may need to be populated with a list of valid game names
		openGames = new Hashtable();
		gameHosts = new Hashtable();
	}

	public boolean typeSupported(String requestTypeName)
	{
		// TODO Auto-generated method stub
		return false;
	}

	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		// TODO Auto-generated method stub
		String type;
		ModuleRequest mreq = (ModuleRequest) req;
		ModuleResponse mres = (ModuleResponse) res;
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
			//TODO
		}
		else if (type == Macros.REG_MOD_JOIN_GAME_USER) {
			//TODO
		}
		else if (type == Macros.REG_MOD_REFRESH_STATUS) {
			//TODO
		}
		else if (type == Macros.REG_MOD_ACCEPT_CHALLENGE) {
			//TODO
		}
		else if (type == Macros.REG_MOD_UNREGISTER_USER) {
			//TODO
		}
		else {
			//the request type cannot be handled
			throw new ModuleException (Macros.UNKNOWN_REQUEST_TYPE_CODE, Macros.UNKNOWN_REQUEST_TYPE_MSG);
		}
	}
	
	/**
	 * Register the current user with the gameName he/she request
	 * Respond with the status of this request
	 * 		If sucessful return a secretKey that will be used for the duration of the session
	 * 		else if the request is denied return the corresponding error code
	 * @param req
	 * @param res
	 */
	void registerUser(ModuleRequest req, ModuleResponse res) {
		String userName, gameName, status, secretKey;
		int errorCode;
		
		//get userName and gameName from the request object
		userName = req.getHeader(Macros.NAME);
		gameName = req.getHeader(Macros.GAME);
		
		if ((errorCode = isValidName(userName)) == Macros.VALID) {
			addUser(userName, gameName);
			status = Macros.ACK;
			secretKey = generateKeyString(userName);
			
			//set response headers
			res.setHeader(Macros.SECRET_KEY, secretKey);
			secretKeys.put(userName, secretKey);
			res.setHeader(Macros.STATUS, status);
		}
		else {
			status = Macros.DENY;
			res.setHeader(Macros.STATUS, status);
			res.setHeader(Macros.ERROR_CODE, errorCodeToString(errorCode));
		}
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	void getUsersOnline(ModuleRequest req, ModuleResponse res) throws ModuleException {
		String gameName, onlineUser, onlineGame;
		OutputStream outStream;
		Enumeration users;
		byte [] byteArr;
		try {
			outStream = res.getOutputStream();
		}
		catch (IOException ioe) {
			throw new ModuleException(Macros.IO_EXCEPTION_CODE, Macros.IO_EXCEPTION_TYPE_MSG);
		}
		
		//Get Name of game being requested and write each user to the output stream
		//delimit with a newline
		gameName = req.getHeader(Macros.GAME);
		for (users = usersOnline.keys(); users.hasMoreElements();) {
			onlineUser = (String)users.nextElement();
			onlineGame = (String)usersOnline.get(onlineUser);
			if (onlineGame.equals(gameName)) {
				try {
					onlineUser += "\n";
					byteArr = onlineUser.getBytes();
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
	 * @modifies res
	 */
	private void getOpenGames(ModuleRequest req, ModuleResponse res) {
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
	 */
	void registerNewGame(ModuleRequest req, ModuleResponse res) {
		String userName, secretKey, variation, gameMessage, gameName;
		boolean validKey, notHostingGame, validVariant;
		Integer gameID;
		Hashtable gameSessions;
		
		//used to descibe the particular game session to be hosted
		PropertyBucket propBucket = new PropertyBucket();
		
		//extract request headers
		userName = req.getHeader(Macros.NAME);
		secretKey= req.getHeader(Macros.SECRET_KEY);
		variation = req.getHeader(Macros.VARIATION);
		gameMessage = req.getHeader(Macros.GAME_MESSAGE);
		
		//TODO: note that userName may not exist and that would lead to a null game
		//need to handle this
		gameName = (String)usersOnline.get(userName);
		
		//Verify secretKey/userName
		validKey = isValidUserKey(userName, secretKey);
		notHostingGame = notHosting(userName);
		validVariant = isValidVariation(gameName, variation);
		if (validKey && notHostingGame && validVariant) {
			//client will now be hosting game
			gameHosts.put(userName, gameName);
			
			//populate property bucket
			propBucket.setProperty(Macros.PROPERTY_HOST, userName);
			propBucket.setProperty(Macros.PROPERTY_INTERESTED_USERS, new LinkedList());
			propBucket.setProperty(Macros.PROPERTY_VARIATION, variation);
			gameID = new Integer (generateGameID());
			
			gameSessions = (Hashtable)openGames.get(gameName);
			//TODO: this check only applies if we don't initially populate openGames with
			//all validGames
			if (gameSessions == null) {
				openGames.put(gameName, new Hashtable());
				gameSessions = (Hashtable)openGames.get(gameName);
			}
			gameSessions.put(gameID, propBucket);
			
			//At this point the game has been registered successfully
			res.setHeader(Macros.STATUS, Macros.ACK);
		}
		else {
			//the request has failed
			res.setHeader(Macros.STATUS, Macros.DENY);
			//TODO: change this so that the specific error code is used
			res.setHeader(Macros.ERROR_CODE, Macros.GENERIC_ERROR_CODE.toString());
		}
	}
	
	/**
	 * Unregister the client's currently hosted game
	 * @param req
	 * @param res
	 */
	void unregisterGame(ModuleRequest req, ModuleResponse res) {
		String userName, secretKey, gameName, gameHost;
		boolean validKey, validGameHost;
		Hashtable gameSessions;
		Enumeration gameSessionTable;
		Integer gameID;
		PropertyBucket propBucket;
		
		//extract header values
		userName = req.getHeader(Macros.NAME);
		secretKey = req.getHeader(Macros.SECRET_KEY);
		validKey = isValidUserKey(userName, secretKey);
		validGameHost = isValidGameHost(userName);
		if (validKey && validGameHost) {
			gameName = (String)gameHosts.get(userName);
			
			//remove userName/game Session records
			gameHosts.remove(userName);
			gameSessions = (Hashtable)openGames.get(gameName);
			for(gameSessionTable = gameSessions.keys(); gameSessionTable.hasMoreElements();) {
				gameID = (Integer) gameSessionTable.nextElement();
				propBucket = (PropertyBucket)gameSessions.get(gameID);
				gameHost = (String) propBucket.getProperty(Macros.PROPERTY_HOST);
				if (userName.equals(gameHost)) {
					//remove game record
					gameSessions.remove(gameID);
				}
			}
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
	 * @param userName
	 * @return
	 */
	private boolean isValidGameHost(String userName) {
		return gameHosts.get(userName) != null;
	}
	
	/**
	 * 
	 * @return
	 */
	private int generateGameID() {
		//TODO: deal with overflow, possibly generate gameID another way
		//It is very unlikely that billions of users will be playing at the same time,
		//but we can't overlook this
		return gameID++;
	}
	
	/**
	 * 
	 * @param gameName
	 * @param variation
	 * @return
	 */
	private boolean isValidVariation(String gameName, String variation) {
		//check that gameName is not null
		//For now just return true, but we need to figure out how to tell
		//whether or not the user has specified the correct variation
		//maybe the client side should take care of this before sending a request
		return true;
	}
	
	/**
	 * Make sure name is not duplicated or 
	 * "invalid" (characters, eventually include vulgarity)
	 * @param name
	 * @return
	 */
	private int isValidName(String name) {
		return 0;
	}
	
	/**
	 * 
	 * @param userName
	 * @return
	 */
	private boolean notHosting(String userName) {
		return gameHosts.get(userName) == null;
	}
	
	/**
	 * 
	 * @param userName
	 * @param secretKey
	 * @return
	 */
	private boolean isValidUserKey(String userName, String secretKey) {
		String key;
		//we must verify that the user even exists
		//the key can't possibly be valid if the user doesn't exist
		if (usersOnline.get(userName) == null) return false;
		key = (String)secretKeys.get(userName);	
		return key.equals(secretKey);
	}
	
	/**
	 * Add a mapping with the given user and game
	 * @param name
	 * @param game
	 */
	private void addUser(String userName, String gameName) {
		usersOnline.put(userName, gameName);
	}
	
	/**
	 * 
	 * @param user
	 * @return
	 */
	private int generateKey(String user) {
		//for now just return the user String hashCode
		return user.hashCode();
	}
	
	/**
	 * 
	 * @param user
	 * @return
	 */
	private String generateKeyString(String user) {
		return (new Integer(generateKey(user))).toString();
	}
	
	/**
	 * 
	 * @param errorCode
	 * @return
	 */
	private String errorCodeToString(int errorCode) {
		return (new Integer(errorCode)).toString();
	}
	
	
	
	//fields
	private Hashtable usersOnline, secretKeys, openGames, gameHosts;
	private static int gameID;
	
	/**
	 * 
	 * @author vperez
	 *
	 */
	private class PropertyBucket {
		
		/**
		 * 
		 *
		 */
		public PropertyBucket() {
			
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
		
		Hashtable properties;
	}
}
