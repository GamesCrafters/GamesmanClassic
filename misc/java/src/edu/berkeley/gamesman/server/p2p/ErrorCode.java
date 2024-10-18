package edu.berkeley.gamesman.server.p2p;

/**
 * This contains all the error codes for the P2PModule. Error codes for this
 * IModule fall in the range 200 - 299.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode extends edu.berkeley.gamesman.server.ErrorCode
{
	public static final int UNKNOWN_REQUEST_TYPE = 200;
	public static final int INVALID_CREDENTIALS = 201;
	
	public static final int NO_GAME_FOUND = 202;
	public static final int INVALID_MOVE_INFO = 203;
	public static final int MOVE_OUT_OF_TURN = 204;
	public static final int GAME_ABANDONED = 204;	

	/**
	 * Contains the error messages for the P2PModule.
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
		public static final String INVALID_CREDENTIALS = "Invalid session key";		
		
		public static final String NO_GAME_FOUND = "No game found with the specified game id and username";
		public static final String INVALID_MOVE_INFO = "Missing or invalid move information";
		public static final String MOVE_OUT_OF_TURN = "Move made out of turn";		
		public static final String GAME_ABANDONED = "One or more players has resigned from the game";
	}
}