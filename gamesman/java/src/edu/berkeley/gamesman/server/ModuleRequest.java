package edu.berkeley.gamesman.server;

import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;

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
	 * Returns the value of request type specified when the client made the request.
	 * 
	 * @return value of the request type specified when the client made the request
	 */
	public String getType()
	{
		return getHeader(HN_TYPE);
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
		Enumeration e = this.request.getHeaderNames();
		while (e.hasMoreElements())
		{
			String n = ((String)e.nextElement()).trim();
			String val = n;
			if (val.endsWith(","))
				val = val.substring(0, val.length()-1);
			if (val.equalsIgnoreCase(name))
				return this.request.getHeader(n);
		}
		return null;
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
