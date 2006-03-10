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
	 * Sets the specified header for the response.
	 * 
	 * @param name name of the header
	 * @param value value for the header
	 */	
	public void setHeader(String name, String value)
	{
		this.response.addHeader(name, value);
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
