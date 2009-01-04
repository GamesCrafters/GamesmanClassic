package edu.berkeley.gamesman.server;

/**
 * Exception class for all IModule processing exceptions. IModule subclasses are
 * only allowed to throw this Exception class. All constructors require an exception
 * code and a String message. The exception code is used by the Gamesman servlet
 * to know how to properly respond to client requests in the event of an exception.
 * Properly specifying the exception code will result in the correct error code
 * returned to the client via the HTTP response.
 * 
 * @author Matt Jacobsen
 *
 */
public class ModuleException extends Exception
{
	protected int code;
	
	/**
	 * Default constructor. Takes the exception code and a message. 
	 * 
	 * @param code exception code for why this exception was thrown
	 * @param msg message for why this exception was thrown
	 */
	public ModuleException(int code, String msg)
	{
		super(msg);
		this.code = code;
	}

	/**
	 * Alternate constructor. Takes the exception code, a message, and
	 * the root cause Throwable.
	 * 
	 * @param code exception code for why this exception was thrown
	 * @param msg message for why this exception was thrown
	 * @param rootCause underlying Throwable that caused this exception to be thrown
	 */
	public ModuleException(int code, String msg, Throwable rootCause)
	{
		super(msg, rootCause);
		this.code = code;
	}

	/**
	 * Returns the exception code with which this instance was created.
	 * 
	 * @return exception code
	 */
	public int getCode()
	{
		return this.code;
	}
}
