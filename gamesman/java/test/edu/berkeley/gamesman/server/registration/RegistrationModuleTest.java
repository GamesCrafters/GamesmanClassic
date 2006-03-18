package edu.berkeley.gamesman.server.registration;
import edu.berkeley.gamesman.server.ModuleException;
import junit.framework.TestCase;
import edu.berkeley.gamesman.server.*;

/**
 * 
 * @author vperez
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
	
	private void initialize() {
		input = new byte[INPUT_BYTE_ARR_SIZE];
		headerNames = new String[STRING_ARR_SIZE];
		headerValues = new String[STRING_ARR_SIZE];
	}
	
	/**
	 * 
	 * @throws ModuleException
	 */
	public void test01() throws ModuleException {
		regMod = new RegistrationModule();
		
		String [] headerNames  = {Macros.TYPE, Macros.NAME, Macros.GAME};
		String [] headerValues = {Macros.REG_MOD_REGISTER_USER, user, game};
		//don't actually need to use input stream
		input = new byte[0];
		
		tReq = new TestModuleRequest(input,headerNames,headerValues);
		tRes = new TestModuleResponse(OUTPUT_SIZE);
		
		regMod.registerUser(tReq, tRes);
		
		
	}
	
	public void test02() {
		System.out.println("another test");
	}
}
