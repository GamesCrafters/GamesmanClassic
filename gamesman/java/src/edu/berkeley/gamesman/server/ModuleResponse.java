package edu.berkeley.gamesman.server;

import java.io.ByteArrayOutputStream;
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
	protected int returnCode = ErrorCode.VALID_REQUEST;
	protected String returnMsg = null;
	protected ByteArrayOutputStream output = null;
	
	/**
	 * Default constructor. Takes the HttpServletResponse to wrap.
	 * 
	 * @param res HttpServletResponse to wrap
	 */
	public ModuleResponse(HttpServletResponse res)
	{
		this.response = res;
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
		this.returnCode = code;
	}

	/**
	 * Sets the return message for this IModuleResponse. The return message can only
	 * be set once. Calls to this method after the message has been set will have no effect.
	 * 
	 * @param msg message to return as value for header defined in RETURN_MESSAGE
	 */
	public void setReturnMessage(String msg)
	{
		this.returnMsg = msg;
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
		this.output = new ByteArrayOutputStream();
		return this.output;
	}

	public void flush() throws IOException
	{
		setHeader(HN_RETURN_CODE, String.valueOf(this.returnCode));
		if (this.returnMsg != null)
			setHeader(HN_RETURN_MESSAGE, this.returnMsg);
		if (this.output != null)
		{
			OutputStream out = null;
			try
			{				
				out = this.response.getOutputStream();
				out.write(this.output.toByteArray());
			}
			finally
			{
				if (out != null)
					out.close();
			}
		}
	
	}
}
