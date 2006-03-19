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



