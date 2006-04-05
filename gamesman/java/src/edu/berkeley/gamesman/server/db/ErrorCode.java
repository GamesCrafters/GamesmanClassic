package edu.berkeley.gamesman.server.db;

/**
 * This contains all the error codes for the DbModule. Error codes for this
 * IModule fall in the range 100 - 199.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode
{
	// Example error code
	public static final int INVALID_LENGTH = 100;

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
	public interface Msg
	{
		public static final String INVALID_LENGTH = "Invalid length value for request body.";

	}
}
