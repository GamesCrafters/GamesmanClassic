package edu.berkeley.gamesman.server.registration;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Hashtable;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleRequest;
import edu.berkeley.gamesman.server.ModuleResponse;

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
		usersOnline = new Hashtable();
		secretKeys =  new Hashtable();
		openGames = new Hashtable();
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
		ModuleRequest mreq = (ModuleRequest) req;
		ModuleResponse mres = (ModuleResponse) res;
		type = mreq.getHeader(Macros.TYPE);
		if (type == Macros.REG_MOD_REGISTER_USER) {
			registerUser(mreq, mres);
		}
		else if (type == Macros.REG_MOD_GET_USERS_ONLINE) {
			getUsersOnline(mreq, mres);
		}
		else if (type == Macros.REG_MOD_GET_OPEN_GAMES) {
			getOpenGames(mreq, mres);
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
	 * Register the current user with the gameName he/she request
	 * Respond with the status of this request
	 * 		If sucessful return a secretKey that will be used for the duration of the session
	 * 		else if the request is denied return the corresponding error code
	 * @param req
	 * @param res
	 */
	void registerUser(ModuleRequest req, ModuleResponse res) {
		String userName, gameName, status, secretKey;
		int errorCode;
		
		//get userName and gameName from the request object
		userName = req.getHeader(Macros.NAME);
		gameName = req.getHeader(Macros.GAME);
		
		if ((errorCode = isValidName(userName)) == Macros.VALID) {
			addUser(userName, gameName);
			status = Macros.ACK;
			secretKey = generateKeyString(userName);
			
			//set response headers
			res.setHeader(Macros.SECRET_KEY, secretKey);
			secretKeys.put(userName, secretKey);
			res.setHeader(Macros.STATUS, status);
		}
		else {
			status = Macros.DENY;
			res.setHeader(Macros.STATUS, status);
			res.setHeader(Macros.ERROR_CODE, errorCodeToString(errorCode));
		}
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	void getUsersOnline(ModuleRequest req, ModuleResponse res) throws ModuleException {
		String gameName, onlineUser, onlineGame;
		OutputStream outStream;
		Enumeration users;
		byte [] byteArr;
		try {
			outStream = res.getOutputStream();
		}
		catch (IOException ioe) {
			throw new ModuleException(Macros.IO_EXCEPTION_CODE, Macros.IO_EXCEPTION_TYPE_MSG);
		}
		
		//Get Name of game being requested and write each user to the output stream
		//delimit with a newline
		gameName = req.getHeader(Macros.GAME);
		for (users = usersOnline.keys(); users.hasMoreElements();) {
			onlineUser = (String)users.nextElement();
			onlineGame = (String)usersOnline.get(onlineUser);
			if (onlineGame.equals(gameName)) {
				try {
					onlineUser += "\n";
					byteArr = onlineUser.getBytes();
					outStream.write(byteArr);
				}
				catch (IOException ioe) {
					throw new ModuleException(Macros.IO_EXCEPTION_CODE, Macros.IO_EXCEPTION_TYPE_MSG);
				}
			}
		}
		
		
	}
	
	/**
	 * 
	 * @param req
	 * @param res
	 */
	private void getOpenGames(ModuleRequest req, ModuleResponse res) {
		
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
	 * Add a mapping with the given user and game
	 * @param name
	 * @param game
	 */
	private void addUser(String userName, String gameName) {
		usersOnline.put(userName, gameName);
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
	
	/**
	 * 
	 * @param user
	 * @return
	 */
	private String generateKeyString(String user) {
		return (new Integer(generateKey(user))).toString();
	}
	
	/**
	 * 
	 * @param errorCode
	 * @return
	 */
	private String errorCodeToString(int errorCode) {
		return (new Integer(errorCode)).toString();
	}
	
	private Hashtable usersOnline, secretKeys, openGames;

}
