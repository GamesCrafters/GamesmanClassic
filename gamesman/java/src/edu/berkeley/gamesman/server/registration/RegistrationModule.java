package edu.berkeley.gamesman.server.registration;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleRequest;

/**
 * 
 * @author vperez
 *
 */
public class RegistrationModule implements IModule
{

	public RegistrationModule()
	{
		super();
		// TODO Auto-generated constructor stub
	}

	public boolean typeSupported(String requestTypeName)
	{
		// TODO Auto-generated method stub
		return false;
	}

	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		// TODO Auto-generated method stub
		String type;
		type = ((ModuleRequest) req).getHeader(Macros.TYPE);
		if (type == Macros.REG_MOD_REGISTER_USER) {
			registerUser(req, res);
		}
		else if (type == Macros.REG_MOD_GET_USERS_ONLINE) {
			//TODO
		}
		else if (type == Macros.REG_MOD_GET_OPEN_GAMES) {
			//TODO
		}
		else if (type == Macros.REG_MOD_REGISTER_NEW_GAME) {
			//TODO
		}
		else if (type == Macros.REG_MOD_UNREGISTER_GAME) {
			//TODO
		}
		else if (type == Macros.REG_MOD_JOIN_GAME_NUMBER) {
			//TODO
		}
		else if (type == Macros.REG_MOD_JOIN_GAME_USER) {
			//TODO
		}
		else if (type == Macros.REG_MOD_REFRESH_STATUS) {
			//TODO
		}
		else if (type == Macros.REG_MOD_ACCEPT_CHALLENGE) {
			//TODO
		}
		else if (type == Macros.REG_MOD_UNREGISTER_USER) {
			//TODO
		}
		else {
			//the request type cannot be handled
			throw new ModuleException (Macros.UNKNOWN_REQUEST_TYPE_CODE, Macros.UNKNOWN_REQUEST_TYPE_MSG);
		}
		

	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	void registerUser(IModuleRequest req, IModuleResponse res) {
		
	}
	
	/**
	 * 
	 * @param name
	 * @param gameName
	 * @return
	 */
	void registerUser(String name, String gameName) {
		int errorCode, secretKey;
		String status;
		if ((errorCode = isValidName(name)) == Macros.VALID) {
			addUser(name, gameName);
			status = Macros.ACK;
		}
		else {
			status = Macros.DENY;
		}
		secretKey = generateKey(name);
		return;
		//return Response.newRegisterResponse(status, secretKey, errorCode);
	}
	
	/**
	 * Make sure name is not duplicated or 
	 * "invalid" (characters, eventually include vulgarity)
	 * @param name
	 * @return
	 */
	private int isValidName(String name) {
		return 0;
	}
	
	/**
	 * 
	 * @param name
	 */
	private void addUser(String name, String game) {
		
	}
	
	/**
	 * 
	 * @param user
	 * @return
	 */
	private int generateKey(String user) {
		//for now just return the user String hashCode
		return user.hashCode();
	}

}
