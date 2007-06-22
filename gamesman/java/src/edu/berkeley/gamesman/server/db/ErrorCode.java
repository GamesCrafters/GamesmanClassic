package edu.berkeley.gamesman.server.db;

/**
 * This contains all the error codes for the DbModule. Error codes for this
 * IModule fall in the range 100 - 199.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode extends edu.berkeley.gamesman.server.ErrorCode
{
	//error codes
	public static final int INVALID_LENGTH = 100;
	public static final int NO_DB = 101;
	public static final int INVALID_POSITION = 102;
	public static final int MISSING_GAME = 103;
	public static final int MISSING_VARIANT = 104;
	public static final int MISSING_LENGTH = 105;
	public static final int INTERNAL_DB_ERROR = 106;
	

	/**
	 * Contains the error messages for the DbModule.
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
		public static final String INVALID_LENGTH = "Invalid length value for request body.";
		public static final String NO_DB = "Database not located on server for this game.";
		public static final String INVALID_POSITION = "Position requested is invalid.";
		public static final String MISSING_GAME = DbModule.HN_GAME_NAME + " not specified";
		public static final String MISSING_VARIANT = DbModule.HN_GAME_VARIANT + " not specified";
		public static final String MISSING_LENGTH = DbModule.HN_LENGTH + " not specified";
		public static final String INTERNAL_DB_ERROR = "Internal database error";

	}
}
