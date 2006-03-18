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
	
	private String [] headerNames, headerValues;
	private int STRING_ARR_SIZE = 20;
	
	private String user = "user";
	private String game = "mancala";
	private String TAB = "\t";
	
	private void initialize() {
		input = new byte[INPUT_BYTE_ARR_SIZE];
		headerNames = new String[STRING_ARR_SIZE];
		headerValues = new String[STRING_ARR_SIZE];
	}
	
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
		
		if (overallStatus) statusStr = "passed";
		else statusStr = "failed";
		
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
	 * 
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
		
		regMod.registerUser(tReq, tRes);
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
		regMod.registerUser(tReq, tRes);
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
	
	public void test02() {
		//System.out.println("another test");
	}
}
