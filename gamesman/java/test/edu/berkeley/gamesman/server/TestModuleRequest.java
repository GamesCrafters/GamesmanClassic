package edu.berkeley.gamesman.server;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * Test class for creating stub IModuleRequest objects.
 * 
 * @author Matt Jacobsen
 *
 */
public class TestModuleRequest implements IModuleRequest
{
	protected ByteArrayInputStream in = null;
	protected Map headers = null;
	
	/**
	 * Default constructor. Takes a byte array as the stub InputStream, a String
	 * array of header names, and a String array of header values. Note, the 
	 * headerNames and headerValues arrays should be of the same length, and 
	 * neither should be null.
	 * 
	 * @param input contents of the InputStream 
	 * @param headerNames names of headers
	 * @param headerValues values of headers
	 */
	public TestModuleRequest(byte[] input, String[] headerNames, String[] headerValues)
	{
		this.in = new ByteArrayInputStream(input);
		this.headers = new HashMap(headerNames.length);
		for (int i=0; i<headerNames.length; i++)
			this.headers.put(headerNames[i], headerValues[i]);
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
		return (String)headers.get(name);
	}

	/**
	 * Returns the InputStream representing the body of the request.
	 * 
	 * @return InputStream representing the body of the request.
	 * @throws IOException 
	 */
	public InputStream getInputStream() throws IOException
	{
		return in;
	}
}
