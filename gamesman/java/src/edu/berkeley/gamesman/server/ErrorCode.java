package edu.berkeley.gamesman.server;

/**
 * This contains all the error codes for the GamesmanServlet. Error codes for this
 * IModule fall in the range 1 - 99.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode
{
	public static final int VALID_REQUEST = 0;
	public static final int BAD_REQUEST_TYPE = 1;
	public static final int GENERAL_EXCEPTION = 2;
	public static final int SERVER_NOT_INITIALIZED = 3;
	
	/**
	 * Contains the error messages for the GamesmanServlet.
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
		public static final String BAD_REQUEST_TYPE = "Bad request type";
		public static final String GENERAL_EXCEPTION = "An error has occurred";
		public static final String SERVER_NOT_INITIALIZED = "Server did not initialize properly";

	}
	
}
