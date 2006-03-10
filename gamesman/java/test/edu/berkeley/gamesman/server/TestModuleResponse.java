package edu.berkeley.gamesman.server;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
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
	
	/**
	 * Default constructor. Takes the size of the buffer to which the 
	 * OutputStream will write.
	 * 
	 * @param outputSize
	 */
	public TestModuleResponse(int outputSize)
	{
		this.out = new ByteArrayOutputStream(outputSize);
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
