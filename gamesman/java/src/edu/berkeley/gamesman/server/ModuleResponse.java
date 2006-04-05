package edu.berkeley.gamesman.server;

import java.io.IOException;
import java.io.OutputStream;

import javax.servlet.http.HttpServletResponse;

/**
 * Implementation of an IModuleResponse that wraps the HttpServletResponse
 * object.
 * 
 * @author Matt Jacobsen
 *
 */
public class ModuleResponse implements IModuleResponse
{
	protected HttpServletResponse response = null;
	protected boolean returnCodeSet = false;
	protected boolean returnMessageSet = false;
	protected boolean returnValueSet = false;
	
	/**
	 * Default constructor. Takes the HttpServletResponse to wrap.
	 * 
	 * @param res HttpServletResponse to wrap
	 */
	public ModuleResponse(HttpServletResponse res)
	{
		this.response = res;
		this.returnCodeSet = false;
		this.returnMessageSet = false;
		this.returnValueSet = false;
	}

	/**
	 * Sets the return value for this IModuleResponse. Must be either ACK or DENY.
	 * The return value can only be set once. Calls to this method after the value 
	 * has been set will have no effect.
	 * 
	 * @param value either ACK or DENY
	 */
	public void setReturnValue(String value)
	{
		if (!returnValueSet)
		{
			setHeader(HN_RETURN_VALUE, value);
			returnValueSet = true;
		}
	}

	/**
	 * Sets the return code for this IModuleResponse. Same as error code. The default
	 * code is 0, which corresponds to no error. The return code can only
	 * be set once. Calls to this method after the code has been set will have no effect.
	 * 
	 * @param code return code integer
	 */
	public void setReturnCode(int code)
	{
		if (!returnCodeSet)
		{
			setHeader(HN_RETURN_CODE, String.valueOf(code));
			returnCodeSet = true;
		}
	}

	/**
	 * Sets the return message for this IModuleResponse. The return message can only
	 * be set once. Calls to this method after the message has been set will have no effect.
	 * 
	 * @param msg message to return as value for header defined in RETURN_MESSAGE
	 */
	public void setReturnMessage(String msg)
	{
		if (!returnMessageSet)
		{
			setHeader(HN_RETURN_MESSAGE, msg);
			returnMessageSet = true;
		}
	}
	
	/**
	 * Sets the specified header for the response.
	 * 
	 * @param name name of the header
	 * @param value value for the header
	 */	
	public void setHeader(String name, String value)
	{
		this.response.setHeader(name, value);
	}

	/**
	 * Returns the OutputStream to write to for the response.
	 * 
	 * @return OutputStream for the response
	 * @throws IOException
	 */
	public OutputStream getOutputStream() throws IOException
	{
		return this.response.getOutputStream();
	}

}
