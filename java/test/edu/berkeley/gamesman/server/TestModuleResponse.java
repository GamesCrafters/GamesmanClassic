package edu.berkeley.gamesman.server;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Hashtable;
import java.util.Map;

/**
 * Test class for creating stub IModuleResponse objects.
 * 
 * @author Matt Jacobsen
 *
 */
public class TestModuleResponse implements IModuleResponse
{
	protected ByteArrayOutputStream out = null;
	protected Map headers = null;
	protected boolean returnCodeSet = false;
	protected boolean returnMessageSet = false;
	
	/**
	 * Default constructor. Takes the size of the buffer to which the 
	 * OutputStream will write.
	 * 
	 * @param outputSize
	 */
	public TestModuleResponse(int outputSize)
	{
		this.out = new ByteArrayOutputStream(outputSize);
		headers = new Hashtable();
		this.returnCodeSet = false;
		this.returnMessageSet = false;
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
		headers.put(name, value);
	}

	/**
	 * Returns the headers Map for checking if headers have been
	 * written to the response.
	 * 
	 * @return headers Map
	 */
	public Map getHeadersWritten()
	{
		return headers;
	}
	
	/**
	 * Returns the OutputStream to write to for the response.
	 * 
	 * @return OutputStream for the response
	 * @throws IOException
	 */
	public OutputStream getOutputStream() throws IOException
	{
		return this.out;
	}

	/**
	 * Returns the bytes written to the OutputStream so they 
	 * can be checked.
	 * 
	 * @return bytes written to the OutputStream
	 */
	public byte[] getOutputWritten()
	{
		return this.out.toByteArray();
	}
}
