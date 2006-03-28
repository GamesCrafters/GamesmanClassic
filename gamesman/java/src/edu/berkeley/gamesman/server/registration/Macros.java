/**
 * 
 */
package edu.berkeley.gamesman.server.registration;

/**
 * @author Victor Perez
 *	Commonly used header types and values
 */
public final class Macros {
	
	/**
	 * Headers/Values
	 */
	public static final String TYPE = "TYPE";
	public static final String NAME = "NAME";
	public static final String GAME = "GAME";
	public static final String SECRET_KEY = "SECRETKEY";
	public static final String GAME_SESSIONS_INDEX 	= "GAME_SESSIONS_INDEX";
	public static final String VARIATION = "VARIATION";
	public static final String GAME_MESSAGE = "GAMEMESSAGE";
	public static final String GAME_ID = "GAMEID";
	public static final String OPPONENT_USERNAME = "OPPONENTUSERNAME";
	public static final String CHALLENGE_ACCEPTED = "CHALLENGEACCEPTED"; 
	public static final String ACCEPTED = "ACCEPTED";
	public static final String STATUS = "STATUS";
	public static final String ACK = "ACK";
	public static final String DENY = "DENY";
	public static final String ONLINE_USERS = "ONLINE_USERS";
	
	/**
	 * Request Type Macros
	 */
	public static final String REG_MOD_REGISTER_USER = "RegModRegisterUser";
	public static final String REG_MOD_GET_USERS_ONLINE = "RegModGetUsersOnline";
	public static final String REG_MOD_GET_OPEN_GAMES = " RegModGetOpenGames";
	public static final String REG_MOD_REGISTER_NEW_GAME = "RegModRegisterNewGame";
	public static final String REG_MOD_UNREGISTER_GAME = "RegModUnRegisterGame";
	public static final String REG_MOD_JOIN_GAME_NUMBER = "RegModJoinGameNumber";
	public static final String REG_MOD_JOIN_GAME_USER = "RegModJoinGameUser";
	public static final String REG_MOD_REFRESH_STATUS = "RegModRefreshStatus";
	public static final String REG_MOD_ACCEPT_CHALLENGE = "RegModAcceptChallenge";
	public static final String REG_MOD_UNREGISTER_USER = "RegModUnRegisterUser";
	
	/**
	 * Exception Macros
	 */
	public static final int UNKNOWN_REQUEST_TYPE_CODE 	= 1;
	public static final String UNKNOWN_REQUEST_TYPE_MSG = "UNKNOWN REQUEST TYPE";
	public static final int IO_EXCEPTION_CODE 			= 2;
	public static final String IO_EXCEPTION_TYPE_MSG 	= "IO EXCEPTION";
	public static final int INTERRUPT_EXCEPTION_CODE	= 3;
	public static final String INTERRUPT_EXCEPTION_MSG  = "INTERRUPT EXCEPTION";
	public static final int HASHTABLE_COLLISION_CODE	= 4;
	public static final String HASHTABLE_COLLISION_MSG  = "HASHTABLE COLLISION";
	
	/**
	 * RegistrationModule.PropertyBucket Macros
	 */
	public static final String PROPERTY_HOST 				= "PROPERTY HOST";
	public static final String PROPERTY_VARIATION 			= "PROPERTY VARIATION";
	public static final String PROPERTY_INTERESTED_USERS 	= "PROPERTY INTERESTED USERS";
	public static final String PROPERTY_GAME_ID 			= "PROPERTY GAME ID";
	public static final String PROPERTY_GAME_MESSAGE		= "PROPERTY GAME MESSAGE";
	public static final String PROPERTY_HOSTING_GAME		= "PROPERTY HOSTING GAME";
	public static final String PROPERTY_SECRET_KEY 			= "PROPERTY SECRET KEY";
	public static final String PROPERTY_GAME_NAME			= "PROPERTY GAME NAME";
	public static final String PROPERTY_HOST_ACCEPTED		= "PROPERTY HOST ACCEPTED";
	
	public static final Boolean HOST_ACCEPT 				= new Boolean(true);
	public static final Boolean HOST_DECLINE				= new Boolean(false);
	
	/**
	 * Error Code Macros
	 */
	public static final String ERROR_CODE = "ERRORCODE";
	public static final Integer INVALID_USER_NAME 	= new Integer(2);
	public static final Integer INVALID_KEY 		= new Integer(3);
	public static final Integer INVALID_VARIANT 	= new Integer(4);
	public static final Integer GENERIC_ERROR_CODE  = new Integer(1);
	public static final Integer VALID_CODE			= new Integer(0);
	public static final Integer USER_ALREADY_EXISTS = new Integer(5);
	public static final Integer USER_ALREADY_HAS_OPEN_GAME = new Integer(6);
	public static final Integer INVALID_GAME_NUMBER = new Integer(7);
	public static final Integer HOST_DECLINED		= new Integer(8);
	
	/**
	 * Debugging Flags
	 */
	public static final boolean REG_MOD_DEBUGGING = true;
}
