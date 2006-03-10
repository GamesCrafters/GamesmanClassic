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
