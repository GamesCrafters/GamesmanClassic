package edu.berkeley.gamesman.server.registration;
import java.util.Map;

import edu.berkeley.gamesman.server.ModuleException;
import junit.framework.TestCase;
import edu.berkeley.gamesman.server.*;

/**
 * 
 * @author Victor Perez
 *
 */
public class RegistrationModuleTest extends TestCase {
	private RegistrationModule regMod;
	private TestModuleRequest tReq;
	private TestModuleResponse tRes;
	private byte [] input;
	private int INPUT_BYTE_ARR_SIZE = 80;
	private int OUTPUT_SIZE = 1024;
	private int STRING_ARR_SIZE = 20;
	private String user = "user1";
	private String game = "mancala";
	private String TAB = "\t";
	
	/**
	 * 
	 * @param status
	 * @param msgs
	 * @return
	 */
	private void testStatus(int testNum, String description , String[] msgs, boolean[] status) {
		String statusStr;
		boolean overallStatus = true;
		//compute Test suite overall status
		for (int index = 0; index < status.length; index++) {
			overallStatus = overallStatus && status[index];
		}
		
		if (overallStatus) statusStr = "PASSED";
		else statusStr = "FAILED";
		
		System.out.println("Test Number "+ testNum + " " + statusStr);
		System.out.println("Description: " + description);
		System.out.println("Details:");
		for (int index = 0; index<msgs.length; index++) {
			if (status[index]) statusStr = "PASSED";
			else statusStr = "FAILED";
			System.out.println(TAB + statusStr + ": " + msgs[index]);
		}
	}
	
	/**
	 * Test Case 1: tests registerUser() method
	 * @throws ModuleException
	 * @return
	 */
	public void test01() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] headerNames  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues = {Macros.REG_MOD_REGISTER_USER, user, game};
		String [] testMsgs = new String[5];
		boolean [] testStatus = new boolean[5];
		int testNum = 1;
		Map resHeaders;
		String resHrdVal, description;
		
		description = "Testing registerUser()";
		
		//don't actually need to use input stream
		input = new byte[0];
		
		tReq = new TestModuleRequest(input,headerNames,headerValues);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		testMsgs[1] = "Response is ACK";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); 	
		
		testMsgs[2] = "Response has valid Secret Key";
		resHrdVal = (String) resHeaders.get(Macros.SECRET_KEY);
		testStatus[2] = (resHrdVal != null) && regMod.isValidUserKey(user, resHrdVal);
		
		//Now keep the request the same and get a new response when the same userName is re-registered
		//this should not work
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [3] = "Response is DENY if user already exists";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[3] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY);
		
		testMsgs[4] = "Error Code if user already exists is " + Macros.USER_ALREADY_EXISTS;
		resHrdVal = (String) resHeaders.get(Macros.ERROR_CODE);
		testStatus[4] = (resHrdVal != null) && resHrdVal.equals(Macros.USER_ALREADY_EXISTS.toString());
		//print the results
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * Test Case 2: tests getUsersOnline method
	 * @throws ModuleException
	 * @return
	 */
	public void test02() throws ModuleException{
		regMod = new RegistrationModule();
		
		/**
		 * Add three users who want to play mancala
		 */
		String [] headerNames_1  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues_1 = {Macros.REG_MOD_REGISTER_USER, "user1", game};
		String [] testMsgs = new String[3];
		boolean [] testStatus = new boolean[3];
		int testNum = 2;
		String description;
		byte[] outputBytes, expectedOutputBytes;
		char [] parsedBytes;
		description = "Testing getUsersOnline()";
		
		//don't actually need to use input stream
		input = new byte[0];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//user1 has been registed
		regMod.handleRequest(tReq, tRes);
		
		//register another user (user2)
		headerValues_1[1] = "user2";
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		
		//register a third user (user3), but for a different game (achi)
		headerValues_1[1] = "user3";
		headerValues_1[2] = "achi";
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		
		/** 
		 * Now request the list of online users intested in playing mancala
		 */
		String [] headerNames_2  = {Macros.TYPE, Macros.GAME};
		String [] headerValues_2 = {Macros.REG_MOD_GET_USERS_ONLINE, game};
		
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		
		//parse response output buffer from byts to chars
		outputBytes = tRes.getOutputWritten();
		//parsedBytes = parseByteArray(outputBytes);
		//System.out.println(parsedBytes);
		expectedOutputBytes = new String ("user2\n" + "user1\n").getBytes();
		testMsgs[0] = "Output buffer matches users online for mancala";
		// System.out.print("--\n" + new String(outputBytes) + "\n" + "---\n" + new String(expectedOutputBytes) + "\n---\n");
		testStatus[0] = bytesEquals(expectedOutputBytes, outputBytes);
		
		/**
		 * Request the list of users playing a game for which no users are online
		 */
		
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		/** request othello players */
		headerValues_2[1] = "othello";
		tReq = new TestModuleRequest(input, headerNames_2, headerValues_2);
		regMod.handleRequest(tReq,tRes);
		outputBytes = tRes.getOutputWritten();
		testMsgs[1] = "Output buffer empty for unsolicited game";
		testStatus[1] = outputBytes.length == 0;
		
		/**
		 * Request the list of users for achi
		 */
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		headerValues_2[1] = "achi";
		tReq = new TestModuleRequest(input, headerNames_2, headerValues_2);
		regMod.handleRequest(tReq,tRes);
		outputBytes = tRes.getOutputWritten();
		expectedOutputBytes = new String("user3\n").getBytes();
		testMsgs[2] = "Output buffer matches users online for achi";
		testStatus[2] = bytesEquals(outputBytes, expectedOutputBytes);
		
		//print the results
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * Test Case 3: tests registerNewGame method
	 * @throws ModuleException
	 */
	public void test03() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] testMsgs = new String[5];
		boolean [] testStatus = new boolean[5];
		int testNum = 3;
		String description, secretKey, resHrdVal;
		Map resHeaders;
		byte[] outputBytes, expectedOutputBytes;
		char [] parsedBytes;
		description = "Testing registerNewGame()";
		
		/**
		 * So we add a user, and have them register a game.  
		 */
		String [] headerNames_1  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues_1 = {Macros.REG_MOD_REGISTER_USER, "user1", game};
		
		input = new byte[0];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//Adding user1
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		secretKey = (String) resHeaders.get(Macros.SECRET_KEY);
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		//Registering the new game
		String [] headerNames_2  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME, Macros.VARIATION, Macros.GAME_MESSAGE};
		String [] headerValues_2 = {Macros.REG_MOD_REGISTER_NEW_GAME, "user1", secretKey, game, "1", "Looking for Data."};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[1] = "Game Register Response is ACK";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK);
		
		testMsgs[2] = "User's key is valid";
		testStatus[2] = regMod.isValidUserKey(user, secretKey); 
		
		//Now keep the request the same and get a new response when the same userName tries to register
		//another game. This shouldn't work in our current schema. 
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [3] = "Response is DENY if user already has a game registered";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[3] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY);
		
		testMsgs[4] = "Error Code if user already has a game is " + Macros.USER_ALREADY_HAS_OPEN_GAME;
		resHrdVal = (String) resHeaders.get(Macros.ERROR_CODE);
		testStatus[4] = (resHrdVal != null) && resHrdVal.equals(Macros.USER_ALREADY_HAS_OPEN_GAME.toString());
		//print the results
		//System.out.print(resHrdVal + "\n" + Macros.USER_ALREADY_HAS_OPEN_GAME.toString() + "\n"); 
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * Test Case 4: tests getOpenGames method
	 * @throws ModuleException
	 */
	public void test04() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] testMsgs = new String[5];
		boolean [] testStatus = new boolean[5];
		int TEST_LIMIT = 10; 
		String [] openGamesHosts = new String[TEST_LIMIT];
		String [] openGamesIds = new String[TEST_LIMIT];
		String [] openGamesVariations = new String[TEST_LIMIT];
		int testNum = 4, gameCount;
		String description, secretKey, resHrdVal;
		Map resHeaders;
		byte[] outputBytes, expectedOutputBytes;
		char [] parsedBytes;
		description = "Testing getOpenGames()";
		
		/**
		 * So we add a user, and have them register a game.  Again. 
		 */
		String [] headerNames_1  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues_1 = {Macros.REG_MOD_REGISTER_USER, "user1", game};
		
		input = new byte[0];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//Adding user1
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		secretKey = (String) resHeaders.get(Macros.SECRET_KEY);
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		//Registering the new game
		String TEST_VARIATION = "1"; 
		String [] headerNames_2  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME, Macros.VARIATION, Macros.GAME_MESSAGE};
		String [] headerValues_2 = {Macros.REG_MOD_REGISTER_NEW_GAME, "user1", secretKey, game, TEST_VARIATION, "Looking for Data."};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[1] = "Game Register Response is ACK";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK);
		
		testMsgs[2] = "User's key is valid";
		testStatus[2] = regMod.isValidUserKey(user, secretKey); 
		
		// Now we want to get a listing of open games that user1 can see
		String [] headerNames_3  = {Macros.TYPE, Macros.GAME};
		String [] headerValues_3 = {Macros.REG_MOD_GET_OPEN_GAMES, game};
		tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		gameCount = Integer.decode((String)resHeaders.get(Macros.GAME_SESSIONS_INDEX)).intValue();
		if (gameCount > TEST_LIMIT) {
			System.out.print("More open-games than test array size.\n");
			assertTrue(false); 
		}
		
		for (int x = 0; x < gameCount; x++) {
			//System.out.print("---\n");
			openGamesHosts[x] = (String) resHeaders.get(Macros.PROPERTY_HOST + x);
			//System.out.print(openGamesHosts[x] + "\n");
			openGamesIds[x] = (String) resHeaders.get(Macros.PROPERTY_GAME_ID + x); 
			//System.out.print(openGamesIds[x] + "\n");
			openGamesVariations[x] = (String) resHeaders.get(Macros.PROPERTY_VARIATION + x);
			//System.out.print(openGamesVariations[x] + "\n");
		}
		//System.out.print("---\n");
		
		String expectedGamesHosts [] = {"user1"};
		String expectedGamesIds [] = {"1"}; 
		String expectedGamesVariations [] = {TEST_VARIATION}; 
		
		testMsgs[3] = "Response Headers are as expected.";
		testStatus[3] = true; 
		for (int x = 0; x< gameCount; x++) 
			testStatus[3] = testStatus[3] && (openGamesHosts[x].equals(expectedGamesHosts[x])) && (openGamesIds[x].equals(expectedGamesIds[x])) && (openGamesVariations[x].equals(expectedGamesVariations[x])); 
		
		//Finally, we test to see that asking about a game no-one is registered for gives back an empty listing.
		String [] headerNames_4  = {Macros.TYPE, Macros.GAME};
		String [] headerValues_4 = {Macros.REG_MOD_GET_OPEN_GAMES, "ttt"};
		tReq = new TestModuleRequest(input,headerNames_4,headerValues_4);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		testMsgs[4] = "Asking about a game for which there exist no open-games returns 0 for GAMES_SESSIONS_INDEX.";
		testStatus[4] = (Integer.decode((String)resHeaders.get(Macros.GAME_SESSIONS_INDEX)).intValue() == 0); 
		
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * Test Case 5: tests unregisterGame method
	 * @throws ModuleException
	 */
	public void test05() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] testMsgs = new String[6];
		boolean [] testStatus = new boolean[6];
		int testNum = 5;
		String description, secretKey, user2SecretKey, resHrdVal;
		Map resHeaders;
		byte[] outputBytes, expectedOutputBytes;
		char [] parsedBytes;
		description = "Testing unregisterGame()";
		
		/**
		 * So we add a user, and have them register a game.  Yet Again. 
		 */
		String [] headerNames_1  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues_1 = {Macros.REG_MOD_REGISTER_USER, "user1", game};
		
		input = new byte[0];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//Adding user1
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		secretKey = (String) resHeaders.get(Macros.SECRET_KEY);
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		//Registering the new game
		String TEST_VARIATION = "1"; 
		String [] headerNames_2  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME, Macros.VARIATION, Macros.GAME_MESSAGE};
		String [] headerValues_2 = {Macros.REG_MOD_REGISTER_NEW_GAME, "user1", secretKey, game, TEST_VARIATION, "Looking for Data."};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[1] = "Game Register Response is ACK";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK);
		
		// Now we want to first try having someone else kill this game
		// The try is some random person kill the game. 
		String [] headerNames_1b  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues_1b = {Macros.REG_MOD_REGISTER_USER, "user2", game};
		
		input = new byte[0];
		tReq = new TestModuleRequest(input,headerNames_1b,headerValues_1b);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		user2SecretKey = (String) resHeaders.get(Macros.SECRET_KEY);
		
		String [] headerNames_3  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME};
		String [] headerValues_3 = {Macros.REG_MOD_UNREGISTER_GAME, "user2", user2SecretKey, game};
		tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
		tRes = new TestModuleResponse(OUTPUT_SIZE);		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [2] = "Random person cannot kill someone else's game";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[2] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY);
		
		// We double check that user1's game still exists
		String [] headerNames_4  = {Macros.TYPE, Macros.GAME};
		String [] headerValues_4 = {Macros.REG_MOD_GET_OPEN_GAMES, game};
		tReq = new TestModuleRequest(input,headerNames_4,headerValues_4);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		int gameCount = Integer.decode((String)resHeaders.get(Macros.GAME_SESSIONS_INDEX)).intValue();
		int TEST_LIMIT = 10; 
		String [] openGamesHosts = new String[TEST_LIMIT];
		String [] openGamesIds = new String[TEST_LIMIT];
		String [] openGamesVariations = new String[TEST_LIMIT];
		if (gameCount > TEST_LIMIT) {
			System.out.print("More open-games than test array size.\n");
			assertTrue(false); 
		}
		
		for (int x = 0; x < gameCount; x++) {
			//System.out.print("---\n");
			openGamesHosts[x] = (String) resHeaders.get(Macros.PROPERTY_HOST + x);
			//System.out.print(openGamesHosts[x] + "\n");
			openGamesIds[x] = (String) resHeaders.get(Macros.PROPERTY_GAME_ID + x); 
			//System.out.print(openGamesIds[x] + "\n");
			openGamesVariations[x] = (String) resHeaders.get(Macros.PROPERTY_VARIATION + x);
			//System.out.print(openGamesVariations[x] + "\n");
		}
		//System.out.print("---\n");
		
		String expectedGamesHosts [] = {"user1"};
		String expectedGamesIds [] = {"2"}; 
		// This is very odd, but it's the variation number assigned to it. 
		String expectedGamesVariations [] = {TEST_VARIATION}; 
		
		testMsgs[3] = "user1's game is unaffected.";
		testStatus[3] = true; 
		for (int x = 0; x< gameCount; x++) 
			testStatus[3] = testStatus[3] && (openGamesHosts[x].equals(expectedGamesHosts[x])) && (openGamesIds[x].equals(expectedGamesIds[x])) && (openGamesVariations[x].equals(expectedGamesVariations[x])); 
		
		// Now user1 gets to unregister his own game
		String [] headerNames_5  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME};
		String [] headerValues_5 = {Macros.REG_MOD_UNREGISTER_GAME, "user1", secretKey, game};
		tReq = new TestModuleRequest(input,headerNames_5,headerValues_5);
		tRes = new TestModuleResponse(OUTPUT_SIZE);		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[4] = "Response to user1 unregistering game is ACK";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[4] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK);
		
		//Finally we double check that user1's game is gone
		String [] headerNames_6  = {Macros.TYPE, Macros.GAME};
		String [] headerValues_6 = {Macros.REG_MOD_GET_OPEN_GAMES, game};
		tReq = new TestModuleRequest(input,headerNames_6,headerValues_6);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		gameCount = Integer.decode((String)resHeaders.get(Macros.GAME_SESSIONS_INDEX)).intValue();
		openGamesHosts = new String[TEST_LIMIT];
		openGamesIds = new String[TEST_LIMIT];
		openGamesVariations = new String[TEST_LIMIT];
		if (gameCount > TEST_LIMIT) {
			System.out.print("More open-games than test array size.\n");
			assertTrue(false); 
		}
		
		for (int x = 0; x < gameCount; x++) {
			//System.out.print("---\n");
			openGamesHosts[x] = (String) resHeaders.get(Macros.PROPERTY_HOST + x);
			//System.out.print(openGamesHosts[x] + "\n");
			openGamesIds[x] = (String) resHeaders.get(Macros.PROPERTY_GAME_ID + x); 
			//System.out.print(openGamesIds[x] + "\n");
			openGamesVariations[x] = (String) resHeaders.get(Macros.PROPERTY_VARIATION + x);
			//System.out.print(openGamesVariations[x] + "\n");
		}
		//System.out.print("---\n");
		
		expectedGamesHosts = new String [0];
		expectedGamesIds = new String [0];  
		expectedGamesVariations = new String [0]; 
		
		testMsgs[5] = "user1's game is no longer listed.";
		testStatus[5] = true; 
		for (int x = 0; x< gameCount; x++) 
			testStatus[5] = testStatus[5] && (openGamesHosts[x].equals(expectedGamesHosts[x])) && (openGamesIds[x].equals(expectedGamesIds[x])) && (openGamesVariations[x].equals(expectedGamesVariations[x])); 
		
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * Test Case 6: tests unregisterUser() method
	 * @throws ModuleException
	 */
	public void test06() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] testMsgs = new String[8];
		boolean [] testStatus = new boolean[8];
		int testNum = 6;
		Map resHeaders;
		String resHrdVal, description, secretKey;
		
		description = "Testing unregisterUser()";
		
		// First we add a user
		input = new byte[0];
		
		String [] headerNames_1  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues_1 = {Macros.REG_MOD_REGISTER_USER, user, game};
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		testMsgs[1] = "Response is ACK";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); 	
		
		testMsgs[2] = "Response has valid Secret Key";
		secretKey = (String) resHeaders.get(Macros.SECRET_KEY);
		testStatus[2] = (resHrdVal != null) && regMod.isValidUserKey(user, secretKey);
		
		// Next we try unregistering a random other user. 
		String [] headerNames_2  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME};
		String [] headerValues_2 = {Macros.REG_MOD_UNREGISTER_USER, "user2", "iamfake", game};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [3] = "Response is DENY if user is not registered (I.E. invalid key)";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[3] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY);
		
		testMsgs[4] = "Error Code if user fails to unregister is " + Macros.INVALID_KEY;
		resHrdVal = (String) resHeaders.get(Macros.ERROR_CODE);
		testStatus[4] = (resHrdVal != null) && resHrdVal.equals(Macros.INVALID_KEY.toString());
		
		testMsgs[5] = "user1 is still in Registration Module Table";
		testStatus[5] = regMod.isUserOnline(user);
		
		// Finally we let user1 unregister 
		String [] headerNames_3  = {Macros.TYPE, Macros.NAME, Macros.SECRET_KEY, Macros.GAME};
		String [] headerValues_3 = {Macros.REG_MOD_UNREGISTER_USER, user, secretKey, game};
		tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [6] = "Response is ACK for user1 to unregister";
		resHrdVal = (String) resHeaders.get(Macros.STATUS);
		testStatus[6] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK);
		
		testMsgs[7] = "user1 is no longer in Registration Module Table";
		testStatus[7] = !regMod.isUserOnline(user);
		
		//print the results
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * 
	 * @param byteArr1
	 * @param byteArr2
	 * @return
	 */
	private boolean bytesEquals(byte [] byteArr1, byte[] byteArr2) {
		if (byteArr1.length != byteArr2.length) return false;
		for (int index = 0; index < byteArr1.length; index++) {
			if (byteArr1[index] != byteArr2[index]) return false;
		}
		return true;
	}
	
	/**
	 * 
	 * @param byteArr
	 * @return
	 */
	private char[] parseByteArray(byte[] byteArr) {
		char [] charArr = new char[byteArr.length];
		for (int index = 0; index < byteArr.length; index++) {
			charArr[index] = (char) byteArr[index];
		}
		return charArr;
	}
}



