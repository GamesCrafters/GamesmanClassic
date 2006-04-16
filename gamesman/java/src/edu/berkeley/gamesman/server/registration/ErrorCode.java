package edu.berkeley.gamesman.server.registration;

/**
 * This contains all the error codes for the RegistrationModule. Error codes for this
 * IModule fall in the range 300 - 399.
 * 
 * @author Matt Jacobsen
 *
 */
public interface ErrorCode
{
	public static final int DEFAULT_CODE		=0;
	public static final int UNKNOWN_REQUEST_TYPE = 300;
	public static final int IO_EXCEPTION = 301;
	public static final int INTERRUPT_EXCEPTION = 302;
	public static final int HASHTABLE_COLLISION = 303;

	public static final int INVALID_USER_NAME = 304;
	public static final int INVALID_KEY = 305;
	public static final int INVALID_VARIANT = 306;
	public static final int GENERIC_ERROR = 307;
	public static final int USER_ALREADY_EXISTS = 308;
	public static final int USER_ALREADY_HAS_OPEN_GAME = 309;
	public static final int INVALID_GAME_NUMBER = 310;
	public static final int HOST_DECLINED = 311;
	
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
	public interface Msg
	{
		public static final String UNKNOWN_REQUEST_TYPE = "Unknown request type";
		public static final String IO_EXCEPTION = "IO exception";
		public static final String INTERRUPT_EXCEPTION = "Interrupt exception";
		public static final String HASHTABLE_COLLISION = "Hashtable collision";
		public static final String INVALID_USER_NAME = "You've selected and invalid user name";

	}
}