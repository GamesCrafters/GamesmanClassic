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
	 * Header names
	 */
	public static final String HN_NAME = "Name";
	public static final String HN_GAME = "Game";
	public static final String HN_SECRET_KEY = "SecretKey";
	public static final String HN_GAME_SESSIONS_INDEX 	= "GameSessionsCount";
	public static final String HN_VARIANT = "Variant";
	public static final String HN_GAME_MESSAGE = "GameMessage";
	public static final String HN_GAME_ID = "GameId";
	public static final String HN_OPPONENT_USERNAME = "OpponentUsername";
	public static final String HN_CHALLENGE_ACCEPTED = "ChanllengeAccepted"; 
	public static final String HN_STATUS = "Status";
	public static final String HN_ONLINE_USERS = "OnlineUsers";

	/**
	 * Header values
	 */
	public static final String ACCEPTED = "Accepted";
	public static final int VALID_CODE = 0;
	
	// Need to get rid of these and convert over to use response.setReturnCode/setReturnMessage
	public static final String ACK = "ACK";
	public static final String DENY = "DENY";
	
	
	/**
	 * RegistrationModule.PropertyBucket Macros
	 */
	public static final String PROPERTY_HOST 				= "PropertyHost";
	public static final String PROPERTY_VARIATION 			= "PropertyVariant";
	public static final String PROPERTY_INTERESTED_USERS 	= "PropertyInterestedUsers";
	public static final String PROPERTY_GAME_ID 			= "PropertyGameId";
	public static final String PROPERTY_GAME_MESSAGE		= "PropertyGameMessage";
	public static final String PROPERTY_HOSTING_GAME		= "PropertyHostingGame";
	public static final String PROPERTY_SECRET_KEY 			= "PropertySecretKey";
	public static final String PROPERTY_GAME_NAME			= "PropertyGameName";
	public static final String PROPERTY_HOST_ACCEPTED		= "PropertyHostAccepted";
	public static final String PROPERTY_USER_NAME			= "UserName";
	
	public static final Boolean HOST_ACCEPT 				= new Boolean(true);
	public static final Boolean HOST_DECLINE				= new Boolean(false);
	public static final String 	DUMMY_USER					= "*.* DUMMY USER *.*";
		
	/**
	 * Debugging Flags
	 */
	public static final boolean REG_MOD_DEBUGGING = true;
}
