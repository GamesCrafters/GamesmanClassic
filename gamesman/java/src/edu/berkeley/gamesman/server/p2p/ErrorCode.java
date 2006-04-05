package edu.berkeley.gamesman.server.p2p;

/**
 * This contains all the error codes for the P2PModule. Error codes for this
 * IModule fall in the range 200 - 299.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode
{
	public static final int NO_SUCH_GAME = 200;
	public static final int THREAD_INTERRUPTED = 201;

	// Obsolete error codes (these now only give warnings)
	public static final int INVALID_START_OF_GAME = 202;
	public static final int GAME_ALREADY_INITIALIZED = 203;
	public static final int WRONG_PLAYER_TURN = 204;

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
	public interface Msg
	{
		public static final String NO_SUCH_GAME = "No valid game found for specified users/players";
		public static final String THREAD_INTERRUPTED = "Thread interrupted while waiting for players";

		// Obsolete error codes (these now only give warnings)
		public static final String INVALID_START_OF_GAME = "Invalid start of game";
		public static final String GAME_ALREADY_INITIALIZED = "Game already initialized";
		public static final String WRONG_PLAYER_TURN = "Wrong player turn";
	}
}