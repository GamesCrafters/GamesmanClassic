package edu.berkeley.gamesman.server.p2p;
import edu.berkeley.gamesman.server.*;
import java.util.Map;

public class P2PTester {
	
	public static void hPrint(String s) {
		if(s!=null)
			System.out.println(s);
		else System.out.println("--null--");

	}
	
	public static void main(String[] args) {
		

		
		String[] stdReq = new String[4];
		stdReq[0] = "type";
		stdReq[1] = P2PMacros.MOVE_VALUE;
		stdReq[2] =  P2PMacros.DESTINATION_PLAYER;
		stdReq[3] = P2PMacros.SOURCE_PLAYER;
		byte[] trash = {1,2};
		
		String pA = "Player A";
		String pB = "Player B";
		String m1 = "move 1";
		String m2 = "move 2";
		String m3 = "move 3";
		
		String[] hVals1 = {P2PMacros.SEND_MOVE, m1, pA, pB};
		String[] hVals2 = {P2PMacros.SEND_MOVE, m2, pB, pA};
		String[] hVals3 = {P2PMacros.SEND_MOVE, m3, pA, pB};
		
		String[] hVals0 = {P2PMacros.SEND_MOVE, null, pB, pA};
		
		// Create the requests that will eventually be sent
		
		IModuleRequest fromAreq1 = new TestModuleRequest(trash, stdReq, hVals0);
		
		IModuleRequest fromBreq1 = new TestModuleRequest(trash, stdReq, hVals1);
		
		IModuleRequest fromAreq2 = new TestModuleRequest(trash, stdReq, hVals2);
		
		IModuleRequest fromBreq2 = new TestModuleRequest(trash, stdReq, hVals3);
		// create the responses
		IModuleResponse toBres1 = new TestModuleResponse(0);
		IModuleResponse toAres1 = new TestModuleResponse(0);
		IModuleResponse toBres2 = new TestModuleResponse(0);
		IModuleResponse toAres2 = new TestModuleResponse(0);
		
		
		// setup the module and the game
		P2PModule theModule = new P2PModule();
		P2PModule.registerNewGame("Player B", "Player A");
		hPrint("Yay, registered!");
		// action!
		try {
		theModule.handleRequest(fromAreq1, toBres1);
		
		hPrint("First request 'handled'");
		
		
		TestModuleResponse test1 = (TestModuleResponse) toBres1;
		Map toBres1Map = test1.getHeadersWritten();
		hPrint("Should say null");
		hPrint((String) toBres1Map.get(P2PMacros.MOVE_VALUE));
		System.out.println();
		
		theModule.handleRequest(fromBreq1, toAres1);
		toBres1Map = test1.getHeadersWritten();
		hPrint("should say: first move of game");
		} catch(ModuleException e) {
			System.out.println("!! Exception: "+e.getMessage());
		}
		
		
		
		
		
		
	}
}
