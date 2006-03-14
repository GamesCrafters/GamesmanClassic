package edu.berkeley.gamesman.server;

import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;

/**
 * The Servlet that handles the requests from the client
 * and calls specific modules to implement them
 *
 * @author Amit Matani
 */

public class GamesmanServlet extends HttpServlet {
	
	private IModule modulesArray[];
	
	//this is unneeded but kept just in case
	private int IModuleLength;
	
	/**
	 * Called only once when tomcat initializes the servlet.
	 * The ServletConfig holds the parameter values set in the web.xml file
	 * This will instantiate all of the modules and place them in the
	 * modules array.  The class throws a servlet exception because
	 * it deals with the web.xml file and not the clients
	 * 
	 * The web.xml file must have the following init-params:
	 * imodules.lenghth
	 * imodules.list
	 *
	 * @param config config object of the web.xml file
	 * @throws ServletException 
	 */
	public void init(ServletConfig config)
		throws ServletException 
	{
		String IModuleList;
		String IModuleNameArray[];
		
		try {
			IModuleLength = Integer.parseInt(
					config.getInitParameter("imodules.length"));
		} catch(Exception e) {
			throw new ServletException("Bad imodules.length value");
		}
		if ((IModuleList = 
			config.getInitParameter("imodules.list")) == null) {
			throw new ServletException("imodules.list is unspecified");
		}
		
		IModuleNameArray = IModuleList.split("\\s+");
		
		modulesArray = new IModule[IModuleNameArray.length]; 
		
		//Instantiate Modules and place in object array
		for (int i = 0; i < IModuleNameArray.length; i++) {
			try {
				Class newClass = Class.forName(IModuleNameArray[i]);
				modulesArray[i] = (IModule) newClass.newInstance();
			} catch (Exception e) {
				throw new ServletException("Bad Module Name");
			}	
		}
		
	}
	/**
	 * The doPost function will respond to a post request from the
	 * client.  The client will package the request and reponse in a
	 * ModuleRequest and ModuleResponse and calls the appropriate module
	 * to handle it.  It also handles servlet exceptions thrown from 
	 * any of the modules and throws them back to the client
	 * 
	 * @param config config object of the web.xml file
	 * @throws ServletException 
	 */
	public void doPost(HttpServletRequest request,
                      HttpServletResponse response)
        throws IOException, ServletException
    {
    		ModuleRequest mRequest = new ModuleRequest(request);
    		ModuleResponse mResponse = new ModuleResponse(response);
    		int counter = 0;
    		
    		while (counter < modulesArray.length && !modulesArray[counter].typeSupported(
    				mRequest.getHeader("type"))) {
    			counter++;
    		}
    		if (counter < modulesArray.length) {
    			try {
    				modulesArray[counter].handleRequest(mRequest, mResponse);
    			} catch (ModuleException e) {
    				//send back error code
    				mResponse.setHeader("error_code", String.valueOf(e.getCode()));
    				mResponse.setHeader("error_message", e.getMessage());
    			}
    		}
    		else {
    			mResponse.setHeader("error_code", 
    					String.valueOf(ModuleException.BAD_REQUEST_TYPE));
    			mResponse.setHeader("error_message", "Bad Request Type");
    		}
    }
	public void doGet (HttpServletRequest request,
					HttpServletResponse response)
		throws IOException, ServletException {
		
		return;
	}
}