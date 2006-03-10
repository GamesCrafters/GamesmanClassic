package edu.berkeley.gamesman.server;

import java.io.IOException;
import java.io.InputStream;

import javax.servlet.http.HttpServletRequest;

/**
 * Implementation of an IModuleRequest that wraps the HttpServletRequest.
 * 
 * @author Matt Jacobsen
 *
 */
public class ModuleRequest implements IModuleRequest
{
	protected HttpServletRequest request = null;

	/**
	 * Default constructor. Takes the HttpServletRequest to wrap.
	 * 
	 * @param req HttpServletRequest to wrap
	 */
	public ModuleRequest(HttpServletRequest req)
	{
		this.request = req;
	}

	/**
	 * Returns the value of the specified request header or null if none 
	 * exists with the specified name.
	 * 
	 * @param name name of the header to retrieve
	 * @return value of the named header
	 */
	public String getHeader(String name)
	{
		return this.request.getHeader(name);
	}

	/**
	 * Returns the InputStream representing the body of the request.
	 * 
	 * @return InputStream representing the body of the request.
	 * @throws IOException 
	 */
	public InputStream getInputStream() throws IOException
	{
		return this.request.getInputStream();
	}

}
