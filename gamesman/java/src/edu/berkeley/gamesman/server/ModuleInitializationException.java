package edu.berkeley.gamesman.server;

/**
 * Exception type for use when a problem arises during IModule initialization.
 * 
 * @author Matt Jacobsen
 *
 */
public class ModuleInitializationException extends Exception
{
	/**
	 * Default constructor. Takes a description of the problem.
	 * 
	 * @param arg0 description of the problem
	 */
	public ModuleInitializationException(String arg0)
	{
		super(arg0);
	}

	/**
	 * Alternate constructor. Takes a description of the problem and the 
	 * underlying Throwable that caused it.
	 * 
	 * @param arg0 description of the problem
	 * @param arg1 underlying Throwable that caused the problem
	 */
	public ModuleInitializationException(String arg0, Throwable arg1)
	{
		super(arg0, arg1);
	}

}
