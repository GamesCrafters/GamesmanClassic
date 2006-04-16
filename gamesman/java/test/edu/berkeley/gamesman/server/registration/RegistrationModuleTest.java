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
	private int INPUT_BYTE_ARR_SIZE = 0;
	private int OUTPUT_SIZE = 1024;
	
	/** deafault userName and gameName **/
	private String user = "user1";
	private String game = "mancala";
	private String variation = "1";
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
		System.out.println();
	}
	
	/**
	 * Test Case 1: tests registerUser() method
	 * @throws ModuleException
	 * @return
	 */
	public void test01() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues = {RequestType.REGISTER_USER, user, game};
		String [] testMsgs = new String[4];
		boolean [] testStatus = new boolean[4];
		int testNum = 1;
		Map resHeaders;
		String resHrdVal, description;
		
		description = "Testing registerUser()";
		
		//don't actually need to use input stream
		input = new byte[INPUT_BYTE_ARR_SIZE];
		
		tReq = new TestModuleRequest(input,headerNames,headerValues);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		testMsgs[1] = "Response is ACK";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[1] = (resHrdVal == null);
		
		
		testMsgs[2] = "Response has valid Secret Key";
		resHrdVal = (String) resHeaders.get(Macros.HN_SECRET_KEY);
		testStatus[2] = (resHrdVal != null) && regMod.isValidUserKey(user, resHrdVal);
		
		//Now keep the request the same and get a new response when the same userName is re-registered
		//this should not work
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		//testMsgs [3] = "Response is DENY if user already exists";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[3] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY); // Convert to use responseCode/responseMessage
		
		//resHrdVal = (String) resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		//testStatus[3] = (resHrdVal != null) && Integer.parseInt(resHrdVal) == ErrorCode.USER_ALREADY_EXISTS;
		
		
		testMsgs[3] = "Error Code if user already exists is " + ErrorCode.USER_ALREADY_EXISTS;
		resHrdVal = (String) resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[3] = (resHrdVal != null) && resHrdVal.equals(String.valueOf(ErrorCode.USER_ALREADY_EXISTS));
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
		String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues_1 = {RequestType.REGISTER_USER, "user1", game};
		String [] testMsgs = new String[3];
		boolean [] testStatus = new boolean[3];
		int testNum = 2;
		String description;
		byte[] outputBytes, expectedOutputBytes;
		description = "Testing getUsersOnline()";
		
		//don't actually need to use input stream
		input = new byte[INPUT_BYTE_ARR_SIZE];
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
		String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_GAME};
		String [] headerValues_2 = {RequestType.GET_USERS, game};
		
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
		
		String [] testMsgs = new String[4];
		boolean [] testStatus = new boolean[4];
		int testNum = 3;
		String description, secretKey, resHrdVal;
		Map resHeaders;
		description = "Testing registerNewGame()";
		
		/**
		 * So we add a user, and have them register a game.  
		 */
		String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues_1 = {RequestType.REGISTER_USER, "user1", game};
		
		input = new byte[INPUT_BYTE_ARR_SIZE];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//Adding user1
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		secretKey = (String) resHeaders.get(Macros.HN_SECRET_KEY);
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		//Registering the new game
		String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME, Macros.HN_VARIANT, Macros.HN_GAME_MESSAGE};
		String [] headerValues_2 = {RequestType.REGISTER_GAME, "user1", secretKey, game, "1", "Looking for Data."};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[1] = "Game Register Response is ACK";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[1] = (resHrdVal == null);
		
		
		testMsgs[2] = "User's key is valid";
		testStatus[2] = regMod.isValidUserKey(user, secretKey); 
		
		//Now keep the request the same and get a new response when the same userName tries to register
		//another game. This shouldn't work in our current schema. 
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		//testMsgs [3] = "Response is DENY if user already has a game registered";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[3] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY); // Convert to use responseCode/responseMessage
		
		testMsgs[3] = "Error Code if user already has a game is " + ErrorCode.USER_ALREADY_HAS_OPEN_GAME;
		resHrdVal = (String) resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[3] = (resHrdVal != null) && resHrdVal.equals(String.valueOf(ErrorCode.USER_ALREADY_HAS_OPEN_GAME));
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
		description = "Testing getOpenGames()";
		
		/**
		 * So we add a user, and have them register a game.  Again. 
		 */
		String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues_1 = {RequestType.REGISTER_USER, "user1", game};
		
		input = new byte[INPUT_BYTE_ARR_SIZE];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//Adding user1
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		secretKey = (String) resHeaders.get(Macros.HN_SECRET_KEY);
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		//Registering the new game
		String TEST_VARIATION = "1"; 
		String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME, Macros.HN_VARIANT, Macros.HN_GAME_MESSAGE};
		String [] headerValues_2 = {RequestType.REGISTER_GAME, "user1", secretKey, game, TEST_VARIATION, "Looking for Data."};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[1] = "Game Register Response is ACK";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[1] = (resHrdVal == null);
		
		
		testMsgs[2] = "User's key is valid";
		testStatus[2] = regMod.isValidUserKey(user, secretKey); 
		
		// Now we want to get a listing of open games that user1 can see
		String [] headerNames_3  = {IModuleRequest.HN_TYPE, Macros.HN_GAME};
		String [] headerValues_3 = {RequestType.GET_GAMES, game};
		tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		gameCount = Integer.decode((String)resHeaders.get(Macros.HN_GAME_SESSIONS_INDEX)).intValue();
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
		String [] headerNames_4  = {IModuleRequest.HN_TYPE, Macros.HN_GAME};
		String [] headerValues_4 = {RequestType.GET_GAMES, "ttt"};
		tReq = new TestModuleRequest(input,headerNames_4,headerValues_4);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		testMsgs[4] = "Asking about a game for which there exist no open-games returns 0 for GAMES_SESSIONS_INDEX.";
		testStatus[4] = (Integer.decode((String)resHeaders.get(Macros.HN_GAME_SESSIONS_INDEX)).intValue() == 0); 
		
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
		description = "Testing unregisterGame()";
		
		/**
		 * So we add a user, and have them register a game.  Yet Again. 
		 */
		String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues_1 = {RequestType.REGISTER_USER, "user1", game};
		
		input = new byte[INPUT_BYTE_ARR_SIZE];
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		//Adding user1
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		secretKey = (String) resHeaders.get(Macros.HN_SECRET_KEY);
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		//Registering the new game
		String TEST_VARIATION = "1"; 
		String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME, Macros.HN_VARIANT, Macros.HN_GAME_MESSAGE};
		String [] headerValues_2 = {RequestType.REGISTER_GAME, "user1", secretKey, game, TEST_VARIATION, "Looking for Data."};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[1] = "Game Register Response is ACK";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[1] = (resHrdVal == null);
		
		
		//System.out.println("user1 registers game");
		// Now we want to first try having someone else kill this game
		// The try is some random person kill the game. 
		String [] headerNames_1b  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues_1b = {RequestType.REGISTER_USER, "user2", game};
		
		input = new byte[INPUT_BYTE_ARR_SIZE];
		tReq = new TestModuleRequest(input,headerNames_1b,headerValues_1b);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		user2SecretKey = (String) resHeaders.get(Macros.HN_SECRET_KEY);
		
		String [] headerNames_3  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME};
		String [] headerValues_3 = {RequestType.UNREGISTER_GAME, "user2", user2SecretKey, game};
		tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
		tRes = new TestModuleResponse(OUTPUT_SIZE);		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [2] = "Random person cannot kill someone else's game";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[2] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[2] = (resHrdVal != null) && resHrdVal.equals(String.valueOf(ErrorCode.GENERIC_ERROR));
		
		
		// We double check that user1's game still exists
		String [] headerNames_4  = {IModuleRequest.HN_TYPE, Macros.HN_GAME};
		String [] headerValues_4 = {RequestType.GET_GAMES, game};
		tReq = new TestModuleRequest(input,headerNames_4,headerValues_4);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		int gameCount = Integer.decode((String)resHeaders.get(Macros.HN_GAME_SESSIONS_INDEX)).intValue();
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
		String [] headerNames_5  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME};
		String [] headerValues_5 = {RequestType.UNREGISTER_GAME, "user1", secretKey, game};
		tReq = new TestModuleRequest(input,headerNames_5,headerValues_5);
		tRes = new TestModuleResponse(OUTPUT_SIZE);		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs[4] = "Response to user1 unregistering game is ACK";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[4] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[4] = (resHrdVal == null);
		
		
		//Finally we double check that user1's game is gone
		String [] headerNames_6  = {IModuleRequest.HN_TYPE, Macros.HN_GAME};
		String [] headerValues_6 = {RequestType.GET_GAMES, game};
		tReq = new TestModuleRequest(input,headerNames_6,headerValues_6);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		gameCount = Integer.decode((String)resHeaders.get(Macros.HN_GAME_SESSIONS_INDEX)).intValue();
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
		
		String [] testMsgs = new String[7];
		boolean [] testStatus = new boolean[7];
		int testNum = 6;
		Map resHeaders;
		String resHrdVal, description, secretKey;
		
		description = "Testing unregisterUser()";
		
		// First we add a user
		input = new byte[INPUT_BYTE_ARR_SIZE];
		
		String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
		String [] headerValues_1 = {RequestType.REGISTER_USER, user, game};
		tReq = new TestModuleRequest(input,headerNames_1,headerValues_1);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		
		testMsgs[0] = "User name added to Registration Module Table";
		testStatus[0] = regMod.isUserOnline(user);
		
		testMsgs[1] = "Response is ACK";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[1] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); 	 // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[1] = (resHrdVal == null);
		
		
		testMsgs[2] = "Response has valid Secret Key";
		secretKey = (String) resHeaders.get(Macros.HN_SECRET_KEY);
		testStatus[2] = secretKey!= null && regMod.isValidUserKey(user, secretKey);
		
		// Next we try unregistering a random other user. 
		String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME};
		String [] headerValues_2 = {RequestType.UNREGISTER_USER, "user2", "iamfake", game};
		tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		//testMsgs [3] = "Response is DENY if user is not registered (I.E. invalid key)";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[3] = (resHrdVal != null) && resHrdVal.equals(Macros.DENY); // Convert to use responseCode/responseMessage
		
		testMsgs[3] = "Error Code if user fails to unregister is " + ErrorCode.INVALID_KEY;
		resHrdVal = (String) resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[3] = (resHrdVal != null) && resHrdVal.equals(String.valueOf(ErrorCode.INVALID_KEY));
		
		testMsgs[4] = "user1 is still in Registration Module Table";
		testStatus[4] = regMod.isUserOnline(user);
		
		// Finally we let user1 unregister 
		String [] headerNames_3  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME};
		String [] headerValues_3 = {RequestType.UNREGISTER_USER, user, secretKey, game};
		tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.handleRequest(tReq, tRes);
		resHeaders = tRes.getHeadersWritten();
		
		testMsgs [5] = "Response is ACK for user1 to unregister";
		//resHrdVal = (String) resHeaders.get(Macros.HN_STATUS);
		//testStatus[5] = (resHrdVal != null) && resHrdVal.equals(Macros.ACK); // Convert to use responseCode/responseMessage
		resHrdVal = (String)resHeaders.get(IModuleResponse.HN_RETURN_CODE);
		testStatus[5] = (resHrdVal == null);
		
		
		testMsgs[6] = "user1 is no longer in Registration Module Table";
		testStatus[6] = !regMod.isUserOnline(user);
		
		//print the results
		testStatus(testNum, description, testMsgs, testStatus);
	}
	
	/**
	 * Tests joinGameNumber method in isolation through the use of simulated accept/deny responses
	 * @throws Module Exception
	 */
	public synchronized void test07() throws ModuleException {
		regMod = new RegistrationModule();
		Thread tr1 = new Thread( new Runnable() {
			public void run () {
				try {
					String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
					String [] headerValues = {RequestType.REGISTER_USER, user, game};
					String [] testMsgs = new String[4];
					boolean [] testStatus = new boolean[4];
					int testNum = 7;
					Map resHeaders;
					String description, secretKey1, secretKey2;
					
					description = "Testing joinGameNumber()";
					
					//don't actually need to use input stream
					input = new byte[INPUT_BYTE_ARR_SIZE];
					tReq = new TestModuleRequest(input,headerNames,headerValues);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					
					//user1 has been registed to play mancala
					regMod.handleRequest(tReq, tRes);
					secretKey1 = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
					
					
					//register user2 to also play the same game
					headerValues[1] = "user2";
					tReq = new TestModuleRequest(input,headerNames,headerValues);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					secretKey2 = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
					
					//let user1 register a new game of mancala
					//Registering the new game
					String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME, Macros.HN_VARIANT, Macros.HN_GAME_MESSAGE};
					String [] headerValues_2 = {RequestType.REGISTER_GAME, "user1", secretKey1, game, variation, "I challenge anyone to play"};
					tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					
					//Now user2 will attempt to join this game, which should have a gameID of 0
					//but with an incorrect secret key
					String [] headerNames_3  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME_ID};
					String [] headerValues_3 = {RequestType.JOIN_GAME_NUMBER, "user1", "WRONG_KEY", new Integer(3).toString()};
					tReq = new TestModuleRequest(input, headerNames_3, headerValues_3);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					resHeaders = tRes.getHeadersWritten();
					testMsgs[0] = "User1 attempting to join a game with an invalid secret key";
					testStatus[0] = resHeaders.get(Macros.HN_STATUS).equals(Macros.DENY) && // Convert to use responseCode/responseMessage
									resHeaders.get(IModuleResponse.HN_RETURN_CODE).equals(String.valueOf(ErrorCode.INVALID_KEY));
					
					/**
					 * User will now attempt to join the same game with the correct secret key by with the wrong
					 * gameID choose a random game ID
					 */
					headerValues_3[3] = new Integer(52).toString();
					headerValues_3[2] = secretKey1;
					tReq = new TestModuleRequest(input, headerNames_3, headerValues_3);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					resHeaders = tRes.getHeadersWritten();
					testMsgs[1] = "User1 attempting to join a game providing an incorrect gameID";
					//testStatus[1] = resHeaders.get(Macros.HN_STATUS).equals(Macros.DENY) && // Convert to use responseCode/responseMessage
					//				resHeaders.get(IModuleResponse.HN_RETURN_CODE).equals(String.valueOf(ErrorCode.INVALID_GAME_NUMBER));
					testStatus[1] = resHeaders.get(IModuleResponse.HN_RETURN_CODE).equals(String.valueOf(ErrorCode.INVALID_GAME_NUMBER));
					
					
					/**
					 * The same user will now make a valid request and a denial will be simulated
					 */
					headerValues_3[3] = new Integer(3).toString();
					tReq = new TestModuleRequest(input, headerNames_3, headerValues_3);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					resHeaders = tRes.getHeadersWritten();
					testMsgs[2] = "Simulating user 1 trying to join, but host rejecting";
					//testStatus[2] = resHeaders.get(Macros.HN_STATUS).equals(Macros.DENY) && // Convert to use responseCode/responseMessage
					//				resHeaders.get(IModuleResponse.HN_RETURN_CODE).equals(String.valueOf(ErrorCode.HOST_DECLINED));
					testStatus[2] = resHeaders.get(IModuleResponse.HN_RETURN_CODE).equals(String.valueOf(ErrorCode.HOST_DECLINED));
					
					
					/**
					 * Now simulate user2 requesting to join the game with a correct request, and receiving
					 * an ACK
					 */
					headerValues_3[1] = "user2";
					headerValues_3[2] = secretKey2;
					tReq = new TestModuleRequest(input, headerNames_3, headerValues_3);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					resHeaders = tRes.getHeadersWritten();
					testMsgs[3] = "Simulating user 2 trying to join, and host accepting the challenge";
					//testStatus[3] = resHeaders.get(Macros.HN_STATUS).equals(Macros.ACK); // Convert to use responseCode/responseMessage
					testStatus[3] = resHeaders.get(IModuleResponse.HN_RETURN_CODE) == null;
					
					testStatus(testNum, description, testMsgs, testStatus);
				}
				catch (ModuleException me) {
					//do something
					me.printStackTrace();
				}
			}
		}
		);
		//begin thread execution
		tr1.start();
		
		while (tr1.isAlive()){
			try {
				wait(1);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			regMod.acceptUser("user1", Macros.HOST_DECLINE);
			regMod.acceptUser("user2", Macros.HOST_ACCEPT);
			regMod.wakeThreads();
		}
	}
	
	/**
	 * 
	 *
	 */
	public synchronized void test08() throws ModuleException{
		//testStatus(testNum, description, testMsgs, testStatus);
		
		regMod = new RegistrationModule();
		Thread tr1 = new Thread( new Runnable() {
			public void run () {
				try {
					String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
					String [] headerValues = {RequestType.REGISTER_USER, user, game};
					String [] testMsgs = new String[1];
					boolean [] testStatus = new boolean[1];
					int testNum = 8;
					Map resHeaders;
					String description, secretKey1, secretKey2;
					
					description = "Testing acceptChallenge() & refreshHostStatus()";
					
					//don't actually need to use input stream
					input = new byte[INPUT_BYTE_ARR_SIZE];
					tReq = new TestModuleRequest(input,headerNames,headerValues);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					
					//user1 has been registed to play mancala
					regMod.handleRequest(tReq, tRes);
					secretKey1 = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
					
					
					//register user2 to also play the same game
					headerValues[1] = "user2";
					tReq = new TestModuleRequest(input,headerNames,headerValues);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					secretKey2 = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
					
					//let user1 register a new game of mancala
					//Registering the new game
					String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME, Macros.HN_VARIANT, Macros.HN_GAME_MESSAGE};
					String [] headerValues_2 = {RequestType.REGISTER_GAME, "user1", secretKey1, game, variation, "I challenge anyone to play"};
					tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					
					//Now user2 will attempt to join this game, which should have a gameID of 4
					//but with an incorrect secret key
					String [] headerNames_3  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME_ID};
					String [] headerValues_3 = {RequestType.JOIN_GAME_NUMBER, "user2", secretKey2, new Integer(4).toString()};
					tReq = new TestModuleRequest(input, headerNames_3, headerValues_3);
					tRes = new TestModuleResponse(OUTPUT_SIZE);
					regMod.handleRequest(tReq, tRes);
					resHeaders = tRes.getHeadersWritten();
					testMsgs[0] = "User2 attempting to join a game with an valid secret key";
					//testStatus[0] = resHeaders.get(Macros.HN_STATUS).equals(Macros.ACK); // Convert to use responseCode/responseMessage
					testStatus[0] = resHeaders.get(IModuleResponse.HN_RETURN_CODE) == null;
					
					testStatus(testNum, description, testMsgs, testStatus);
				}
				catch (ModuleException me) {
					//do something
					me.printStackTrace();
				}
			}
		}
		);
		//begin thread execution
		tr1.start();
		String secretKey;
		String [] headerNames4 = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY};
		String [] headerValues4 = {RequestType.REFRESH_STATUS, "user1", secretKey = RegistrationModule.generateKeyString("user1")};
		input = new byte[INPUT_BYTE_ARR_SIZE];
		TestModuleRequest tReq1 = new TestModuleRequest(input,headerNames4,headerValues4);
		TestModuleResponse tRes1 = new TestModuleResponse(OUTPUT_SIZE);
		String opponent = Macros.DUMMY_USER;
		boolean accept = false;
		while (tr1.isAlive()){
			try {
				wait(1);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			if (opponent == Macros.DUMMY_USER) {
				regMod.handleRequest(tReq1,tRes1);
				opponent = (String)tRes1.getHeadersWritten().get(Macros.HN_OPPONENT_USERNAME);
			}
			else if (!accept) {
				accept = true;
				String [] headerNames1 = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_CHALLENGE_ACCEPTED};
				String [] headerValues1 = {RequestType.ACCEPT_CHALLENGE, "user1", secretKey, Macros.ACCEPTED};
				tReq1 = new TestModuleRequest(input,headerNames1,headerValues1);
				tRes1 = new TestModuleResponse(OUTPUT_SIZE);
				regMod.handleRequest(tReq1,tRes1);
			}
			
		}
	}
	
	/** Test 9 Fields */
	String [] testMsgs_test9 = new String [4];
	boolean [] testStatus_test9 = new boolean [4];
	int testNum9 = 9;
	String test9Description = "Scalar inter method test (joinGameNumber(), refreshHostStatus(), acceptChallenge())";
	
	public synchronized void test09() throws ModuleException, InterruptedException {
		regMod = new RegistrationModule();
		
		/**
		 * User 1 execution thread
		 */
		Thread user1 = new Thread( new Runnable() {
			public void run () {
				try {
				String userName = "user1";
				int userIndex = 0;
				String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
				String [] headerValues = {RequestType.REGISTER_USER, userName, game};
				String secretKey;
				Map resHeaders;
				
				TestModuleRequest tReq;
				TestModuleResponse tRes;
				
				//don't actually need to use input stream
				input = new byte[INPUT_BYTE_ARR_SIZE];
				tReq = new TestModuleRequest(input,headerNames,headerValues);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				
				//user1 has been registed to play mancala
				regMod.handleRequest(tReq, tRes);
				secretKey = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
				
				//request to join the game of mancala, note that another thread must already be hosting the game
				//Current id should be 5
				String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME_ID};
				String [] headerValues_1 = {RequestType.JOIN_GAME_NUMBER, userName, secretKey, new Integer(5).toString()};
				tReq = new TestModuleRequest(input, headerNames_1, headerValues_1);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				regMod.handleRequest(tReq, tRes);
				resHeaders = tRes.getHeadersWritten();
				testMsgs_test9[userIndex] = userName + " attempting to join a game with an valid secret key...host DECLINED";
				//testStatus_test9[userIndex] = resHeaders.get(Macros.HN_STATUS).equals(Macros.DENY); // Convert to use responseCode/responseMessage
				testStatus_test9[userIndex] = resHeaders.get(IModuleResponse.HN_RETURN_CODE).equals(String.valueOf(ErrorCode.HOST_DECLINED));
				}
				catch (Exception e) {
					e.printStackTrace();	
				}
			}
		});
		
		/**
		 * User 2 execution thread
		 * This is the lucky user who will be chosen to play
		 */
		Thread user2 = new Thread( new Runnable() {
			public void run () {
				try {
				String userName = "user2";
				int userIndex = 1;
				String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
				String [] headerValues = {RequestType.REGISTER_USER, userName, game};
				String secretKey;
				Map resHeaders;
				
				TestModuleRequest tReq;
				TestModuleResponse tRes;
				
				//don't actually need to use input stream
				input = new byte[INPUT_BYTE_ARR_SIZE];
				tReq = new TestModuleRequest(input,headerNames,headerValues);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				
				//user1 has been registed to play mancala
				regMod.handleRequest(tReq, tRes);
				secretKey = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
				
				//request to join the game of mancala, note that another thread must already be hosting the game
				//Current id should be 5
				String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME_ID};
				String [] headerValues_1 = {RequestType.JOIN_GAME_NUMBER, "user2", secretKey, new Integer(5).toString()};
				tReq = new TestModuleRequest(input, headerNames_1, headerValues_1);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				regMod.handleRequest(tReq, tRes);
				resHeaders = tRes.getHeadersWritten();
				testMsgs_test9[userIndex] = userName + " attempting to join a game with an valid secret key...host ACCEPTED";
				//testStatus_test9[userIndex] = resHeaders.get(Macros.HN_STATUS).equals(Macros.ACK); // Convert to use responseCode/responseMessage
				testStatus_test9[userIndex] = resHeaders.get(IModuleResponse.HN_RETURN_CODE) == null;
				
				}
				catch (Exception e) {
					e.printStackTrace();	
				}
			}
		});
		
		/**
		 * User 3 execution thread
		 */
		Thread user3 = new Thread( new Runnable() {
			public void run () {
				try {
				String userName = "user3";
				int userIndex = 2;
				String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
				String [] headerValues = {RequestType.REGISTER_USER, userName, game};
				String secretKey;
				Map resHeaders;
				
				TestModuleRequest tReq;
				TestModuleResponse tRes;
				
				//don't actually need to use input stream
				input = new byte[INPUT_BYTE_ARR_SIZE];
				tReq = new TestModuleRequest(input,headerNames,headerValues);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				
				//user1 has been registed to play mancala
				regMod.handleRequest(tReq, tRes);
				secretKey = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
				
				//request to join the game of mancala, note that another thread must already be hosting the game
				//Current id should be 5
				String [] headerNames_1  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME_ID};
				String [] headerValues_1 = {RequestType.JOIN_GAME_NUMBER, "user2", secretKey, new Integer(5).toString()};
				tReq = new TestModuleRequest(input, headerNames_1, headerValues_1);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				regMod.handleRequest(tReq, tRes);
				resHeaders = tRes.getHeadersWritten();
				testMsgs_test9[userIndex] = userName + " attempting to join a game with an valid secret key...host DECLINED";
				testStatus_test9[userIndex] = resHeaders.get(Macros.HN_STATUS).equals(Macros.DENY); // Convert to use responseCode/responseMessage
				
				}
				catch (Exception e) {
					e.printStackTrace();	
				}
			}
		});
		
		/**
		 * User 4 Mancala Game Host
		 */
		Thread user4 = new Thread( new Runnable() {
			public void run () {
				try {
				String userName = "user4";
				int userIndex = 3;
				String [] headerNames  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_GAME};
				String [] headerValues = {RequestType.REGISTER_USER, userName, game};
				String secretKey;
				
				TestModuleRequest tReq;
				TestModuleResponse tRes;
				
				//don't actually need to use input stream
				input = new byte[INPUT_BYTE_ARR_SIZE];
				tReq = new TestModuleRequest(input,headerNames,headerValues);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				
				//user1 has been registed to play mancala
				regMod.handleRequest(tReq, tRes);
				secretKey = (String)tRes.getHeadersWritten().get(Macros.HN_SECRET_KEY);
				
				//let user4 register a new game of mancala
				//Registering the new game
				String [] headerNames_2  = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_GAME, Macros.HN_VARIANT, Macros.HN_GAME_MESSAGE};
				String [] headerValues_2 = {RequestType.REGISTER_GAME, userName, secretKey, game, variation, "I challenge anyone to play"};
				tReq = new TestModuleRequest(input,headerNames_2,headerValues_2);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				regMod.handleRequest(tReq, tRes);
				
				
				String [] headerNames_3 = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY};
				String [] headerValues_3 = {RequestType.REFRESH_STATUS, userName, secretKey};
				input = new byte[INPUT_BYTE_ARR_SIZE];
				tReq = new TestModuleRequest(input,headerNames_3,headerValues_3);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				
				String opponent = Macros.DUMMY_USER;
				while (!opponent.equals("user2")) {
					synchronized (this) {
						wait(10);
					}
					regMod.handleRequest(tReq,tRes);
					opponent = (String)tRes.getHeadersWritten().get(Macros.HN_OPPONENT_USERNAME);
				}
				
				//When we are presented with user2 accept the challenge and inform the other users that
				//they will not be able to join this game session
				String [] headerNames_4 = {IModuleRequest.HN_TYPE, Macros.HN_NAME, Macros.HN_SECRET_KEY, Macros.HN_CHALLENGE_ACCEPTED};
				String [] headerValues_4 = {RequestType.ACCEPT_CHALLENGE, userName, secretKey, Macros.ACCEPTED};
				tReq = new TestModuleRequest(input,headerNames_4,headerValues_4);
				tRes = new TestModuleResponse(OUTPUT_SIZE);
				regMod.handleRequest(tReq,tRes);
				
				testMsgs_test9[userIndex] = "Host accepted user 2 response ACK";
				//testStatus_test9[userIndex] = 	tRes.getHeadersWritten().get(Macros.HN_STATUS).equals(Macros.ACK); // Convert to use responseCode/responseMessage
				testStatus_test9[userIndex] = tRes.getHeadersWritten().get(IModuleResponse.HN_RETURN_CODE) == null;
				}
				catch (Exception e) {
					e.printStackTrace();	
				}
			}
		});
		
		/*
		user4.join();
		user3.join();
		user2.join();
		user1.join();
		*/
		
		user4.start();
		//Give the host time to set up the game
		wait(10);
		
		//Start all remaining threads
		user3.start();
		user2.start();
		user1.start();
		
		while (user1.isAlive() || user2.isAlive() || user3.isAlive() || user4.isAlive()) {
			//wait until thread execution is over
			wait(10);
		}
		
		testStatus(testNum9, test9Description, testMsgs_test9, testStatus_test9);
	}
	
	/**
	 * 
	 * @param byteArr1
	 * @param byteArr2
	 * @return
	 */
	private static boolean bytesEquals(byte [] byteArr1, byte[] byteArr2) {
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
	/*
	private static char[] parseByteArray(byte[] byteArr) {
		char [] charArr = new char[byteArr.length];
		for (int index = 0; index < byteArr.length; index++) {
			charArr[index] = (char) byteArr[index];
		}
		return charArr;
	}
	*/
}