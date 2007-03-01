package edu.berkeley.gamesman.server.registration;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;

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

	/**
	 * 
	 *
	 */
	public RegistrationModule() {
		super();
		//usersOnline = new Hashtable();
		//this hashtable may need to be populated with a list of valid game names
		//openGames = new Hashtable();
	}
	
	// fields revised
	private static Hashtable<String, UserNode> usersOnline = new Hashtable<String, UserNode>();
	private static Hashtable<String, Hashtable<Integer, TableNode>> openTables = new Hashtable<String, Hashtable<Integer, TableNode>>(); //, usersOnline;
	// wtf?
	//private static int gameID;

	/**
	 * 
	 */
	public void initialize(String workingDir, String[] configArgs) throws ModuleInitializationException {
		return;
	}

	/**
	 * 
	 */
	public boolean typeSupported(String requestTypeName) {
		//All supported types
	return 	requestTypeName.equalsIgnoreCase(RequestType.REGISTER_USER) 	||
		 	requestTypeName.equalsIgnoreCase(RequestType.GET_USERS) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.GET_GAMES) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.REGISTER_GAME)||
		 	requestTypeName.equalsIgnoreCase(RequestType.UNREGISTER_GAME)  ||
		 	requestTypeName.equalsIgnoreCase(RequestType.JOIN_GAME_NUMBER) ||
		 	requestTypeName.equalsIgnoreCase(RequestType.JOIN_GAME_USER)	||
		 	requestTypeName.equalsIgnoreCase(RequestType.REFRESH_STATUS)	||
		 	requestTypeName.equalsIgnoreCase(RequestType.ACCEPT_CHALLENGE) || 
		 	requestTypeName.equalsIgnoreCase(RequestType.UNREGISTER_USER); 
	}
	
	/**
	 * 
	 */
	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		String type;
		IModuleRequest mreq = req;
		IModuleResponse mres =  res;
		type = mreq.getType();
		if (type.equalsIgnoreCase(RequestType.REGISTER_USER)) {
			registerUser(mreq, mres);
		}
		else if (type.equalsIgnoreCase(RequestType.GET_USERS)) {
			getUsersOnline(mreq, mres);
		}
		else if (type.equalsIgnoreCase(RequestType.GET_GAMES)) {
			getOpenGames(mreq, mres);
		}
		else if (type.equalsIgnoreCase(RequestType.REGISTER_GAME)) {
			registerNewGame(mreq, mres);
		}
		else if (type.equalsIgnoreCase(RequestType.UNREGISTER_GAME)) {
			unregisterGame(mreq, mres);
		}
		else if (type.equalsIgnoreCase(RequestType.JOIN_GAME_NUMBER)) {
			joinGameNumber(mreq, mres);
		}
		else if (type.equalsIgnoreCase(RequestType.JOIN_GAME_USER)) {
			//Ambiguous meaning in the future if we implement multiple
			//open-games per user. So might as well make it a client-side
			//feature, instead of a dedicated server request. 
		}
		else if (type.equalsIgnoreCase(RequestType.REFRESH_STATUS)) {
			refreshHostStatus(mreq, mres); 
		}
		else if (type.equalsIgnoreCase(RequestType.ACCEPT_CHALLENGE)) {
			acceptChallenge(mreq, mres); 
		}
		else if (type.equalsIgnoreCase(RequestType.UNREGISTER_USER)) {
			unregisterUser(mreq, mres);
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
	private void registerUser(IModuleRequest req, IModuleResponse res) throws ModuleException {
		String userName, gameName, secretKey, userPassword;
		int checkStatus;
		
		//get userName and gameName from the request object
		userName = req.getHeader(Macros.HN_NAME);
		gameName = req.getHeader(Macros.HN_GAME);
		userPassword = req.getHeader(Macros.HN_PASSWORD);
		
		// a newcomer
		if ((checkStatus = checkName(userName, userPassword)) == Macros.CHECK_NEW_USER) {
			addNewUser(userName, gameName, userPassword);
			secretKey = usersOnline.get(userName).getSecretKey();
			
			// ugly 1.4 hack:
			//secretKey = (String)((PropertyBucket)getUser(userName)).getProperty(Macros.PROPERTY_SECRET_KEY);
			res.setHeader(Macros.HN_SECRET_KEY, secretKey);
		} else if(checkStatus == Macros.CHECK_EXISTING_USER) { // user already registered with us
			usersOnline.get(userName).addGameType(gameName);
		} else {
			res.setReturnCode(checkStatus);
			res.setReturnMessage(ErrorCode.Msg.INVALID_USER_NAME);
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
	//modded
	private void getUsersOnline(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		String gameName, onlineUser;
		LinkedList<String> onlineGames;
		OutputStream outStream;
		Enumeration users;
		byte [] byteArr;
		UserNode u;
		
		try {
			outStream = res.getOutputStream();
		}
		catch (IOException ioe) {
			throw new ModuleException(ErrorCode.IO_EXCEPTION, ErrorCode.Msg.IO_EXCEPTION);
		}
		
		/**
		 * Get the name of game being requested and write each user to the output stream
		 * delimited with a newline
		 */
		gameName = req.getHeader(Macros.HN_GAME);
		int length = 0;
		for (users = usersOnline.keys(); users.hasMoreElements();) {
			onlineUser = (String)users.nextElement();
			u = usersOnline.get(onlineUser);
			
			onlineGames = u.getGameTypes();
			
			if (onlineGames.contains(gameName)) {
				onlineUser += "\n";
				byteArr = onlineUser.getBytes();
				length += byteArr.length;
				try {
					outStream.write(byteArr);
				}
				catch (IOException ioe) {
					throw new ModuleException(ErrorCode.IO_EXCEPTION, ErrorCode.Msg.IO_EXCEPTION);
				}
			}
			res.setHeader(Macros.HN_LENGTH, ""+length);
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
		TableNode t;
		Integer foo;
		int index;
		Hashtable<Integer, TableNode> gameSessions;
		Enumeration<Integer> gameSessionTable;
		
		//filter for the game client is looking for
		gameName = req.getHeader(Macros.HN_GAME);
		
		//get all sessions of that particular game
		//Note that if gameName isn't being hosted, then an empty hashtable will be returned by getGameSessions
		gameSessions = getGameSessions(gameName);
		
		for (index = 0, gameSessionTable = gameSessions.keys(); 
						gameSessionTable.hasMoreElements(); index++) {
			
			//list of properties descibing this game session instance
			foo = gameSessionTable.nextElement();
			t = gameSessions.get(foo);
			host = t.getHost();
			gameID = t.getGameID();
			variationNumber = t.getVariation();
			/* get host and variation 
			propBucket = (PropertyBucket)gameSessions.get(gameID);
			host = (String)propBucket.getProperty(Macros.PROPERTY_HOST);
			variationNumber = (String)propBucket.getProperty(Macros.PROPERTY_VARIATION);
			*/
			
			//add headers to response
			//note that index is necessary since there will be several game sessions
			res.setHeader(Macros.PROPERTY_GAME_ID + index, gameID.toString());
			res.setHeader(Macros.PROPERTY_HOST + index, host);
			res.setHeader(Macros.PROPERTY_VARIATION + index, variationNumber);
		}
		//the client will need to know how many headers to extract, that's what this
		//index is for
		res.setHeader(Macros.HN_GAME_SESSIONS_INDEX, (new Integer(index).toString()));
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
		UserNode u;
		//used to descibe the particular game session to be hosted
		//PropertyBucket propBucket;
		
		//extract request headers
		userName = req.getHeader(Macros.HN_NAME);
		secretKey= req.getHeader(Macros.HN_SECRET_KEY);
		variation = req.getHeader(Macros.HN_VARIANT);
		gameMessage = req.getHeader(Macros.HN_GAME_MESSAGE);
		gameName = req.getHeader(Macros.HN_GAME);
		u = usersOnline.get(userName);
		
		//Validity Checks
		//propBucket = getUser(userName);
		if(u == null) {
			res.setReturnCode(ErrorCode.INVALID_USER_NAME);
			res.setReturnMessage(ErrorCode.INVALID_USER_NAME + userName);
			return;
		} else if(!u.getGameTypes().contains(gameName)) {
			res.setReturnCode(ErrorCode.USER_NOT_PLAYING_GAME);
			res.setReturnMessage(ErrorCode.USER_NOT_PLAYING_GAME+" "+userName+" "+gameName);
			return;
		} else if(!isValidUserKey(userName, secretKey)) {
			res.setReturnCode(ErrorCode.INVALID_KEY);
			return;
		} else if(!isValidVariant(gameName, variation)) {
			res.setReturnCode(ErrorCode.INVALID_VARIANT);
			return;
		} else {
			TableNode t = new TableNode(gameName, userName, variation, gameMessage);
			UserNode host = usersOnline.get(userName);
			
			host.addTable(t);
			getGameSessions(gameName).put(t.getGameID(), t);
			
			// TODO make sure this can stay out
			//At this point the game has been registered successfully so respond with Macros.ACK
			//res.setHeader(Macros.HN_STATUS, Macros.ACK); // Convert to use responseCode/responseMessage
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

		String userName = req.getHeader(Macros.HN_NAME);
		String secretKey = req.getHeader(Macros.HN_SECRET_KEY);
		int gameID = Integer.parseInt(req.getHeader(Macros.HN_GAME_ID));
		String gameName = req.getHeader(Macros.HN_GAME);
		UserNode u = usersOnline.get(userName);
		TableNode t = getGameSessions(gameName).get(gameID);

		if(!isValidUserKey(userName, secretKey)) {
			res.setReturnCode(ErrorCode.INVALID_KEY);
			return;
		} else if(!u.getHostedGames().contains(gameID)) {
			res.setReturnCode(ErrorCode.INVALID_GAME_NUMBER);
			return;
		} else {
			unregisterGameHelper(u, t, gameName, gameID);
		}
	}

	private void unregisterGameHelper(UserNode u, TableNode t, String gameName, int gameID) {
		u.getHostedGames().remove(t);
		getGameSessions(gameName).remove(gameID);		
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
	private void joinGameNumber(IModuleRequest req, IModuleResponse res)  throws ModuleException {
		//Variable Declarations
		
		Hashtable<Integer, TableNode> gameSessions;
		
		//extract header values
		String userName = req.getHeader(Macros.HN_NAME);
		String secretKey = req.getHeader(Macros.HN_SECRET_KEY);
		int gameID = Integer.parseInt(req.getHeader(Macros.HN_GAME_ID));
		String gameName = req.getHeader(Macros.HN_GAME);
		
		
		UserNode u = usersOnline.get(userName);
		
		//get all game sessions of gameName
		gameSessions = getGameSessions(gameName);
		
		if(!isValidUserKey(userName, secretKey)) {
			res.setReturnCode(ErrorCode.INVALID_KEY);
			return;
		} else if(!gameSessions.containsKey(gameID)) {
			res.setReturnCode(ErrorCode.INVALID_GAME_NUMBER);
			return;
		} else if(!u.getGameTypes().contains(gameName)) {
			res.setReturnCode(ErrorCode.USER_NOT_PLAYING_GAME);
			return;
		} else {
			TableNode t = gameSessions.get(gameID);
			t.addInterestedUser(u);

			LinkedList<UserNode> interestedUsers = t.getInterestedUsers();
			int usersProcessedByHost = t.getUsersProcessed();
			synchronized (interestedUsers) {
				interestedUsers.add(u);
			}
			try {
				// A classic condition variable
				//while (userPropBucket.getProperty(Macros.PROPERTY_HOST_ACCEPTED) == null) {
				while(usersProcessedByHost == t.getUsersProcessed()) {
					usersProcessedByHost = t.getUsersProcessed();
					// go to sleep and wait for another thread to wake us up when they refresh
					synchronized (this) {
						wait();
					}
				}
			}
			catch (InterruptedException ie) {
				throw new ModuleException(ErrorCode.INTERRUPT_EXCEPTION, ErrorCode.Msg.INTERRUPT_EXCEPTION);
			}
			String acceptedPlayer = t.getAcceptedPlayer();

			if (t==null || acceptedPlayer.equalsIgnoreCase(gameName)) {
				res.setReturnCode(ErrorCode.HOST_DECLINED);
			}
		}
	}

	/**
	 * Unregisters a user from a particular game.
	 * 
	 * @param req
	 * @param res
	 */
	private void unregisterUser(IModuleRequest req, IModuleResponse res) {
 
		String userName = req.getHeader(Macros.HN_NAME);
		String secretKey = req.getHeader(Macros.HN_SECRET_KEY);
		String gameName = req.getHeader(Macros.HN_GAME);
		
		if(!isValidUserKey(userName, secretKey)) {
			res.setReturnCode(ErrorCode.INVALID_KEY);
			return;
		} else {
			UserNode u = usersOnline.get(userName);
			if(!u.getGameTypes().contains(gameName)) {
				res.setReturnCode(ErrorCode.USER_NOT_PLAYING_GAME);
				return;
			}
			LinkedList<TableNode> tables = u.getHostedGames();
			LinkedList<TableNode> toRemove = new LinkedList<TableNode>();
			// silly concurrent modification exception...
			// this is slightly inefficient...optimize?
			for(TableNode t : tables) {
				if(t.getGameName().equalsIgnoreCase(gameName)) {
					toRemove.add(t);
				}
			}
			for(TableNode t : toRemove) {
				u.removeHostedGame(t);
			}
			u.removeGameType(gameName);
		}
	}
// works up to here	
	/**
	 * Request from game host to accept/decline game challenge
	 * Host indicates whether or not to play against the first opponent to join
	 * @param req
	 * @param res
	 * @return
	 * @modifies this
	 */
	private void acceptChallenge(IModuleRequest req, IModuleResponse res) {
		 
		String userName = req.getHeader(Macros.HN_NAME);
		String secretKey = req.getHeader(Macros.HN_SECRET_KEY);
		String gameName = req.getHeader(Macros.HN_GAME);
		int gameID = Integer.parseInt(req.getHeader(Macros.HN_GAME_ID));
		if(!isValidUserKey(userName, secretKey)) {
			res.setReturnCode(ErrorCode.INVALID_KEY);
		}
		
		String challengeResponse = req.getHeader(Macros.HN_CHALLENGE_ACCEPTED);

		boolean hostAgreed = (challengeResponse.equalsIgnoreCase(Macros.ACCEPTED));
		
		UserNode u = usersOnline.get(userName);
		UserNode luckyUser = null;
		TableNode t = getGameSessions(gameName).get(gameID);
		LinkedList<UserNode> interested = t.getInterestedUsers();
		
		synchronized (interested) {
			luckyUser = interested.poll();
		}
		if(luckyUser == null) {
			res.setReturnCode(ErrorCode.DEFAULT_CODE);
			return;
		}
		if(hostAgreed) {
			t.setAcceptedPlayer(luckyUser.getName());
			//host said yes, so we let the other users know that they
			//got denied, inform the P2P module of the new game, let
			//the selected client know he's been accepted, unregister
			//the open-game, and finally give the host the ack signel. 
			
			//choose user in FIFO order
			/* I have no idea what this does
			 luckyUser = (String) guestPropBucket.getProperty(Macros.PROPERTY_USER_NAME);
			 guestPropBucket.setProperty(Macros.PROPERTY_HOST_ACCEPTED, Macros.HOST_ACCEPT);
			 */
			
			/**
			 * <s>inform all other users waiting on the queue that they will not be able to join</s>
			 * no longer need to do this, since they'll wake up and find out
			 */
			/*
			for (Iterator<UserNode> iter = interested.iterator(); iter.hasNext();) {
				UserNode unlucky = iter.next();
				if(unlucky == luckyUser) continue;
				
			}
			*/
			//Inform the P2P module of this game
			P2PModule.registerNewGame(userName, luckyUser.getName(), t.getGameID());
			//P2PModule.registerNewGame(userName, luckyUser, 0);
			
			//TODO: the following line breaks the abstraction barrier. A cleaner solution
			//should be employed to remove the game record
			unregisterGameHelper(u, t, gameName, gameID);
			
			//Wake all waiting threads so they can be notified of the results
			synchronized (this) {
				notifyAll();
			}
			
			//res.setHeader(Macros.HN_STATUS, Macros.ACK); // Convert to use responseCode/responseMessage
		} else {
			//host said no, so we need to let that client know that he 
			//was denied, and take him off of the list of interested-clients
			//so that on the next call to refresh, the server gets someone
			//new instead. Do we need to lock access to the list, if a 
			//client tries to add himself at the same time as the servlet
			//is removing the front of the list? -Filip
			/**
			 * We definitely need a lock for the linked list because unlike the Hashtables
			 * used in this class, linkedlists are not synchronized -Victor
			 */
			
			//No longer need this code, since I implemented a cleaner way to access the host's property bucket
			//gameName = (String)hostPropBucket.getProperty(Macros.PROPERTY_GAME_NAME);
			//gameSessions = getGameSessions(gameName);
			//gameId = (String) hostPropBucket.getProperty(Macros.PROPERTY_GAME_ID);
			//hostPropBucket = (PropertyBucket) gameSessions.get(gameId); 
			
			// guestPropBucket.setProperty(Macros.PROPERTY_HOST_ACCEPTED, Macros.HOST_DECLINE);
			
			/**
			 * Only notify the thread whose property bucket we just removed from the FIFO queue
			 * Note I need to verify that this will indeed wake up the thread we removed
			 * Actually notify() arbitrarily wakes up a thread waiting, therefore notifyAll() had to be used
			 * along with a condition variable
			 */
			synchronized (this) {
				notifyAll();
			}
			//res.setHeader(Macros.HN_STATUS, Macros.ACK); // Convert to use responseCode/responseMessage
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
	private void refreshHostStatus(IModuleRequest req, IModuleResponse res) throws ModuleException{

		
		String userName = req.getHeader(Macros.HN_NAME);
		String secretKey = req.getHeader(Macros.HN_SECRET_KEY);
		int gameID = Integer.parseInt(req.getHeader(Macros.HN_GAME_ID));
		String gameName = req.getHeader(Macros.HN_GAME);
		if(!isValidUserKey(userName, secretKey)) {
			res.setReturnCode(ErrorCode.INVALID_KEY);
			return;
		}
		
		TableNode t = getGameSessions(gameName).get(gameID);
		LinkedList<UserNode> interestedList = t.getInterestedUsers();
		
		OutputStream outStream;
		try {
			outStream = res.getOutputStream();
		}
		catch (IOException ioe) {
			throw new ModuleException(ErrorCode.IO_EXCEPTION, ErrorCode.Msg.IO_EXCEPTION);
		}
		int length = 0;
		synchronized (interestedList) {
			String currentUserName;
			byte[] byteArr;
			
			for(Iterator<UserNode> iter = interestedList.iterator() ; iter.hasNext() ;) {
				currentUserName = iter.next().getName() + "\n";
				byteArr = currentUserName.getBytes();
				length += byteArr.length;
				try {
					outStream.write(byteArr);
				} catch(IOException e) {
					throw new ModuleException(ErrorCode.IO_EXCEPTION, ErrorCode.Msg.IO_EXCEPTION);
				}
			}
		}
		res.setHeader(Macros.HN_LENGTH, ""+length);
		
	}
	
	/**
	 * Add a new game session under gameName described by propBucket and gameID
	 * 
	 * deprecated 
	 * 
	 * @param gameName
	 * @param gameID
	 * @param propBucket
	 * @return
	 * @modifies this
	 */
	/*
	private void addGameSession(String gameName, Integer gameID, PropertyBucket propBucket) {
		//Hashtable gameSessions;
		//gameSessions = getGameSessions(gameName);
		//gameSessions.put(gameID, propBucket);
	}
	*/
	/**
	 * Remove the current gameName session correponding to gameID
	 * 
	 * deprecated
	 * 
	 * @param gameName
	 * @param gameID
	 * @return
	 * @modifies this
	 */
	/*
	private void removeGameSession(String gameName, Integer gameID) {
		//Hashtable gameSessions;
		//gameSessions = getGameSessions(gameName);
		//gameSessions.remove(gameID);
	}
	*/
	
	/**
	 * Search open games for all game sessions of type gameName
	 * Note if a game sessions doesn't exit create a new structure to hold them
	 * @param gameName
	 * @return a gameSessions data structure with all games of type gameName
	 */
	
	private Hashtable<Integer, TableNode> getGameSessions(String gameName) {
		Hashtable<Integer, TableNode> gameSessions;
		System.out.println(openTables.keys() + "\n" + openTables.values()); 
		if (openTables.containsKey(gameName)) {
			gameSessions = openTables.get(gameName);
		} else {
			gameSessions = new Hashtable<Integer, TableNode>();
			openTables.put(gameName, gameSessions);
		}
		
		return gameSessions;
	}
	
	/**
	 * Add a new data structure to store all game sessions of type gameName
	 * 
	 * obsolete
	 * @param gameName
	 * @return
	 * @modifies this
	 */
	/*
	private void addGameSessions(String gameName) {
		//openGames.put(gameName, new Hashtable());
	}
	*/
	/**
	 * Add a mapping to indicate that userName will be hosting gameName
	 * @param userName
	 * @param gameName
	 * @return
	 * @modifies this
	 */
	/*
	private void hostGame(String userName, String gameName) {
		PropertyBucket propBucket = getUser(userName);
		propBucket.setProperty(Macros.PROPERTY_HOSTING_GAME, gameName);
	}
	*/
	/**
	 * Remove the current game host mapping
	 * @param userName
	 * @return
	 * @modifies this
	 */
	/*
	private void stopHostingGame(String userName) {
		PropertyBucket propBucket = getUser(userName);
		propBucket.removeProperty(Macros.PROPERTY_HOSTING_GAME);
	}
	*/
	/**
	 * Check whether userName is hosting a game
	 * 
	 * deprecated
	 * 
	 * @param userName
	 * @return whether or not userName is a game host
	 */
	/*
	private boolean isValidGameHost(String userName) {
		PropertyBucket propBucket = getUser(userName);
		return propBucket.getProperty(Macros.PROPERTY_HOSTING_GAME) != null;
	}
	*/
	/**
	 * Check that variation is a valid gameName type
	 * AS OF YET UNWRITTEN
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
	 
	private int checkName(String name, String passwd) {
	 /**
	 * check that name is not being duplicated
	 */
	 if(!usersOnline.containsKey(name)) 
		 return Macros.CHECK_NEW_USER;
	 else {
		 UserNode u = usersOnline.get(name);
		 if(u.getPassword().equals(passwd)) {
			 return Macros.CHECK_EXISTING_USER;
		 }
	 }
	 return Macros.CHECK_INVALID_USER;
	}
	
	/**
	 * Return whether or not userName is already hosting a game
	 * @param userName
	 * @return 	true if user not hosting a game
	 * 			false otherwise
	 */
	/*
	private boolean isNotHostingGame(String userName) {
		return !isValidGameHost(userName);
	}
	*/
	/**
	 * Add a mapping with the given user and game
	 * @param name
	 * @param game
	 * @return
	 * @modifies this
	 */
	
	private void addNewUser(String userName, String firstGame, String userPassword) throws ModuleException {
		
		String secretKey = generateKeyString(userName);
		UserNode u = new UserNode(userName, userPassword, secretKey, firstGame);
		
		/**
		 * Check that we are not trying to add a user that's already in the table
		 * This should have already been checked implicitly by the callee; this is only
		 * for debugging.
		 */ 
		if (Macros.REG_MOD_DEBUGGING && isUserOnline(userName)) 
			throw new ModuleException(ErrorCode.HASHTABLE_COLLISION, ErrorCode.Msg.HASHTABLE_COLLISION);
		//map user name to corresponding user node
		//this is a critical section so use a mutex
		synchronized (usersOnline) {
			usersOnline.put(userName, u);
		}
		
	}
	
	
	
	/**
	 * Remove the userName, property bucket mapping from usersOnline
	 * @param userName
	 * @return 	the property bucket to which userName was mapped
	 * 			or null if a mapping did not exist
	 * @modifies this
	 */
	/*
	private PropertyBucket removeUser(String userName) {
		return (PropertyBucket)usersOnline.remove(userName);
	}
	*/
	/**
	 * Return the corresponding property bucket associated with userName
	 * in the usersOnline hashtable or null if the mapping does not exist
	 * 
	 * deprecated!
	 * 
	 * @param userName
	 * @return
	 */
	/*
	private static PropertyBucket getUser(String userName) {
		return (PropertyBucket)usersOnline.get(userName);
	}
	*/
	/**
	 * Generate a unique game id for a game
	 * @return gameID
	 */
	/*
	private synchronized static int generateGameID() {
		/**
		 * TODO: deal with overflow, possibly generate gameID another way
		 * It is very unlikely that billions of users will be playing at the same time,
		 * but we can't overlook this
		 *
		return gameID++;
	}
    */
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
	protected static String generateKeyString(String user) {
		return (new Integer(generateKey(user))).toString();
	}

	/**
	 * 
	 * @param userName
	 */
	protected boolean isUserOnline(String userName) {
		return (usersOnline.get(userName) != null);
	}

	/**
	 * Verify that secretKey corresponds to userName
	 * @param userName
	 * @param secretKey
	 * @return
	 */
	static public boolean isValidUserKey(String userName, String secretKey) {
		UserNode u = usersOnline.get(userName);
		if(u == null) return false;
		/**
		 * We must verify that the user even exists
		 * The key can't possibly be valid if the user doesn't exist
		 */
		return u.getSecretKey().equalsIgnoreCase(secretKey);
	}

	/*
	protected static int getGameID() {
		return gameID;
	}
	*/
	/*
	static String getRegisteredKey(String registeredUser) {
		
		return "";
	}
	*/
	/**
	 * For simulated testing purposes
	 *
	 */
	protected synchronized void wakeThreads() {
		notifyAll();
	}

	
	
	
	/*
	 * Class storing all kinds of info about a user: 
	 * 
	 * 
	 */
	private class UserNode {
		String userName, userPassword, secretKey;
		
		// TODO do we want to use a linked list??
		
		LinkedList<String> gameTypes = new LinkedList<String>();
		LinkedList<TableNode> myHostedGames = new LinkedList<TableNode>();
	
		UserNode(String name, String pass, String key, String gameName) {
			userName = name;
			userPassword = pass;
			secretKey = key;
			// TODO maybe not put this in constructor
			synchronized(gameTypes) {
				gameTypes.add(gameName);
			}
		}
		String getName() {
			return userName;
		}
		LinkedList<TableNode> getHostedGames() {
			return myHostedGames;
		}
		LinkedList<String> getGameTypes() {
			return gameTypes;
		}
		String getSecretKey() {
			return secretKey;
		}
		String getPassword() {
			return userPassword;
		}
		
		// nuke this user
		void delete() {
			LinkedList<TableNode> toRemove = new LinkedList<TableNode>();
			synchronized(myHostedGames) {
				for(TableNode t : myHostedGames) {
					toRemove.add(t);
				}
				for(TableNode t : toRemove) {
					myHostedGames.remove(t);
				}
			}
		}
		
		void removeGameType(String type) {
			synchronized(gameTypes) {
				gameTypes.remove(type);
				if(gameTypes.isEmpty()) {
					this.delete();
				}
			}
		}
		void removeHostedGame(TableNode t) {
			synchronized(myHostedGames) {
				myHostedGames.remove(t);
			}
		}
		void addTable(TableNode t) {
			synchronized(myHostedGames) {
				myHostedGames.add(t);
			}
		}
		
		void addGameType(String typeName) {
			synchronized(gameTypes) {
				gameTypes.add(typeName);
			}
		}
	}

	private static class TableNode {
		static int globalGameCounter = 0;
		
		String gameName;
		String host;
		LinkedList<UserNode> interestedUsers = new LinkedList<UserNode>();
		String variation;
		String gameDescription; // is broadcasted 
		
		String acceptedUser = null;
		
		int usersProcessed = 0;
		
		int gameID;
		TableNode(String gameName, String host, String var, String gameD) {
			gameID = globalGameCounter++;
			this.gameName = gameName;
			this.host = host;
			this.variation = var;
			this.gameDescription = gameD;
		}
		void setAcceptedPlayer(String p) {
			acceptedUser = p;
		}
		int getUsersProcessed() {
			return usersProcessed;
		}
		String getAcceptedPlayer() {
			return acceptedUser;
		}
		LinkedList<UserNode> getInterestedUsers() {
			return interestedUsers;
		}
		int getGameID() {
			return gameID;
		}
		String getGameName() {
			return gameName;
		}
		String getHost() {
			return host;
		}
		String getVariation() {
			return variation;
		}
		void addInterestedUser(UserNode u) {
			interestedUsers.add(u);
		}
	}
}