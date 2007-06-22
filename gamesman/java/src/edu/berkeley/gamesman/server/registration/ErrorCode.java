package edu.berkeley.gamesman.server.registration;

/**
 * This contains all the error codes for the RegistrationModule. Error codes for this
 * IModule fall in the range 300 - 399.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode extends edu.berkeley.gamesman.server.ErrorCode
{

	public static final int UNKNOWN_REQUEST_TYPE = 300;
	public static final int IO_EXCEPTION = 301;

	public static final int INVALID_CREDENTIALS = 302;
	public static final int EXISTING_USER = 303;
	public static final int NO_EXISTING_USER = 304;
	
	public static final int NO_USER_FOUND = 305;
	
	public static final int INVALID_GAME_INFO = 306;
	public static final int GAME_CLOSED = 307;
	public static final int OFFER_REVOKED = 308;
	public static final int INVALID_CHALLENGER_INFO = 309;	
	public static final int INVALID_USERNAME = 311;
	public static final int PREV_CHALLENGER_ALREADY_ACCEPTED = 312;			
	
	/**
	 * Contains the error messages for the RegistrationModule.
	 * 
	 * Usage: throw new ModuleException(ErrorCode.SOME_VALUE, ErrorCode.Msg.SOME_VALUE)
	 * or     throw new ModuleException(ErrorCode.SOME_VALUE, ErrorCode.Msg.SOME_VALUE, ex)
	 * The result will be:
	 *        response.setReturnCode(ErrorCode.SOME_VALUE);
	 *        response.setReturnMessage(ErrorCode.Msg.SOME_VALUE + " " + ex.getMessage());
	 *        response.setReturnValue(IModuleResponse.DENY);
	 *        
	 * @author Matt Jacobsen
	 *
	 */
	public interface Msg extends edu.berkeley.gamesman.server.ErrorCode.Msg
	{
		public static final String UNKNOWN_REQUEST_TYPE = "Unknown request type";
		public static final String IO_EXCEPTION = "IO exception";
		public static final String INVALID_CREDENTIALS = "Invalid username and/or password and/or session key";
		public static final String EXISTING_USER = "User already exists, please specify a different username and password";
		public static final String NO_EXISTING_USER = "No user exists with the specified username";
		public static final String NO_USER_FOUND = "No user logged in with the specified username and session key. Perhaps you need to login/relogin.";
		
		public static final String INVALID_GAME_INFO = "Missing or invalid game information";				
		public static final String GAME_CLOSED = "Specified game is no longer open";
		public static final String OFFER_REVOKED = "The game host has offered the challenge to another player";
		public static final String INVALID_CHALLENGER_INFO = "Missing or invalid game challenger information";
		public static final String INVALID_USERNAME = "Username cannot contain spaces or ':' characters";
		public static final String PREV_CHALLENGER_ALREADY_ACCEPTED = "The previously selected challenger has accepted";		
	}
}