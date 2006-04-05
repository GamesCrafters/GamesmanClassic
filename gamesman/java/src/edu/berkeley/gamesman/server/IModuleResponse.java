package edu.berkeley.gamesman.server;

import java.io.IOException;
import java.io.OutputStream;

/**
 * This class represents the response object that is passed to IModule instances.
 * The response headers can be written to this instance, as can the body of the
 * response (as an OutputStream).
 * 
 * @author Matt Jacobsen
 *
 */
public interface IModuleResponse
{
	/** Response header names */
	public static final String HN_RETURN_CODE = "ReturnCode";
	public static final String HN_RETURN_MESSAGE = "ReturnMessage";

	/**
	 * Sets the return code for this IModuleResponse. Same as error code. The default
	 * code is 0, which corresponds to no error. The return code can only
	 * be set once. Calls to this method after the code has been set will have no effect.
	 * 
	 * @param code return code integer
	 */
	public void setReturnCode(int code);

	/**
	 * Sets the return message for this IModuleResponse. The return message can only
	 * be set once. Calls to this method after the message has been set will have no effect.
	 * 
	 * @param msg message to return as value for header defined in RETURN_MESSAGE
	 */
	public void setReturnMessage(String msg);
		
	/**
	 * Sets the specified header for the response.
	 * 
	 * @param name name of the header
	 * @param value value for the header
	 */
	public void setHeader(String name, String value);

	/**
	 * Returns the OutputStream to write to for the response.
	 * 
	 * @return OutputStream for the response
	 * @throws IOException
	 */
	public OutputStream getOutputStream() throws IOException;

}
