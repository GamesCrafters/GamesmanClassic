/**
 * 
 */
package edu.berkeley.gamesman.server.registration;

/**
 * @author vperez
 *	Commonly used header types and values
 */
public final class Macros {

	public static final String STATUS = "STATUS";
	public static final String ACK = "ACK";
	public static final String DENY = "DENY";
	public static final String ONLINE_USERS = "ONLINE_USERS";
	public static final int VALID = 0;
	
	//Headers
	public static final String TYPE = "TYPE";
	public static final String NAME = "NAME";
	public static final String GAME = "GAME";
	public static final String SECRET_KEY = "SECRETKEY";
	public static final String ERROR_CODE = "ERRORCODE";
	public static final String VARIATION = "VARIATION";
	public static final String GAME_MESSAGE = "GAMEMESSAGE";
	public static final String GAME_ID = "GAMEID";
	public static final String OPPONENT_USERNAME = "OPPONENTUSERNAME";
	public static final String ACCEPTED = "ACCEPTED";
	
	//Request Types
	public static final String REG_MOD_REGISTER_USER = "RegModRegisterUser";
	public static final String REG_MOD_GET_USERS_ONLINE = "RegModGetUsersOnline";
	public static final String REG_MOD_REGISTER_NEW_GAME = "RegModRegisterNewGame";
	public static final String REG_MOD_UNREGISTER_GAME = "RegModUnRegisterGame";
	public static final String REG_MOD_JOIN_GAME_NUMBER = "RegModJoinGameNumber";
	public static final String REG_MOD_JOIN_GAME_USER = "RegModJoinGameUser";
	public static final String REG_MOD_REFRESH_STATUS = "RegModRefreshStatus";
	public static final String REG_MOD_ACCEPT_CHALLENGE = "RegModAcceptChallenge";
	public static final String REG_MOD_UNREGISTER_USER = "RegModUnRegisterUser";
}
