package edu.berkeley.gamesman.server;

/**
 * Holds all the value request types for the GamesmanServlet application.
 * Note: clients will need to send requests with a header name of "type" and
 * one of the listed request type values to communicate with the Gamesman Server
 * application. Also note that request header names and values are case insensitive.
 * 
 * DO NOT add your module specific header values here, do that in 
 * your own package (i.e. in a Const file).
 * 
 * @author Matt Jacobsen
 *
 */
public interface RequestType
{
	/** DbModule request types */
	public static final String GET_VALUE_OF_POSITIONS = "GetValueOfPositions";
	public static final String INIT_DATABASE = "InitDatabase";

	/** P2PModule request types */
	public static final String INIT_GAME = "InitGame";
	public static final String SEND_MOVE = "SendMove";
	public static final String GAME_OVER = "GameOver";
	public static final String RESIGN = "Resign";

	/** Registration request types */
	public static final String REGISTER_USER = "RegisterUser";
	public static final String GET_USERS = "GetUsers";
	public static final String GET_GAMES = "GetGames";
	public static final String REGISTER_GAME = "RegisterGame";
	public static final String UNREGISTER_GAME = "UnRegisterGame";
	public static final String JOIN_GAME_NUMBER = "JoinGameNumber";
	public static final String JOIN_GAME_USER = "JoinGameUser";
	public static final String REFRESH_STATUS = "RefreshStatus";
	public static final String ACCEPT_CHALLENGE = "AcceptChallenge";
	public static final String UNREGISTER_USER = "UnRegisterUser";	
}
