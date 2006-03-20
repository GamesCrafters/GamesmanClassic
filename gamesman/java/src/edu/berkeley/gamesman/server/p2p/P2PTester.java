package edu.berkeley.gamesman.server.p2p;
import edu.berkeley.gamesman.server.*;
import java.util.Map;




public class P2PTester {

	
	
	public static void hPrint(String s) {
		if(s!=null)
			System.out.println(s);
		else System.out.println("--null--");

	}
	
	public static void main(String[] args) throws Exception{
		final P2PModule mod = new P2PModule();
		P2PModule.registerNewGame("Player B", "Player A");
		System.out.println("Registration method completed...now in main again");
		final byte[] trash = new byte[0];
		
		final String type = "type";
		final String pA = "Player A";
		final String pB = "Player B";
		final String m1 = "move 1";
		final String m2 = "move 2";
		final String m3 = "move 3";
		
		final String[] hVals1 = {Const.SEND_MOVE, m1, pA, pB};
		final String[] hVals2 = {Const.SEND_MOVE, m2, pB, pA};
		final String[] hVals3 = {Const.SEND_MOVE, m3, pA, pB};	
		final String[] hVals0 = {Const.SEND_MOVE, null, pB, pA};
		final String[] stdReq = {type, Const.MOVE_VALUE, Const.DESTINATION_PLAYER, Const.SOURCE_PLAYER};
		final String[] gameOverReq = {type, Const.DESTINATION_PLAYER, Const.SOURCE_PLAYER};
		final String[] gameOverVals = {Const.END_OF_GAME, pB, pA};
		// Create the requests that will eventually be sent

		Thread threadA = new Thread(new Runnable () {
			// Client A
			public void run() {
				try {
					IModuleRequest fromAreq1 = new TestModuleRequest(trash, stdReq, hVals0);
					IModuleRequest fromAreq2 = new TestModuleRequest(trash, stdReq, hVals2);
					IModuleRequest fromAFinal = new TestModuleRequest(trash, gameOverReq, gameOverVals);
					
					IModuleResponse toAres1 = new TestModuleResponse(0);
					IModuleResponse toAres2 = new TestModuleResponse(0);
					IModuleResponse toAFinal = new TestModuleResponse(0);
					System.out.println("Okay! Thread A started.");
					
					System.out.println("Calling handleRequest of first request from A");
					mod.handleRequest(fromAreq1, toAres1);
					TestModuleResponse aRes1 = (TestModuleResponse) toAres1;
					Map res1Headers = aRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to A (should be 'move 1'): "+res1Headers.get(Const.MOVE_VALUE));
					
					System.out.println("Calling handleRequest of second request from A");
					mod.handleRequest(fromAreq2, toAres2);
					TestModuleResponse aRes2 = (TestModuleResponse) toAres2;
					Map res2Headers = aRes2.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to A (should be 'move 3'): "+res2Headers.get(Const.MOVE_VALUE));
					
					System.out.println("Calling handleRequest of 'game over' request from A");
					mod.handleRequest(fromAFinal, toAFinal);
					TestModuleResponse aRes3 = (TestModuleResponse) toAFinal;
					Map res3Headers = aRes3.getHeadersWritten();				
					System.out.println("@@@@@@This was sent back to A (should be 'null'): "+res2Headers.get(Const.MOVE_VALUE));
					
					
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					return;
				}
				
			}
		});
		threadA.start();
		Thread.sleep(100);
		Thread threadB = new Thread(new Runnable() {
			//Client B
			public void run() {
				try {
					IModuleRequest fromBreq1 = new TestModuleRequest(trash, stdReq, hVals1);		
					IModuleRequest fromBreq2 = new TestModuleRequest(trash, stdReq, hVals3);
					
					IModuleResponse toBres1 = new TestModuleResponse(0);
					IModuleResponse toBres2 = new TestModuleResponse(0);
					System.out.println("Okay! Thread B started!");
					
					System.out.println("Calling handleRequest of first request from B");
					mod.handleRequest(fromBreq1, toBres1);
					TestModuleResponse bRes1 = (TestModuleResponse) toBres1;
					Map bRes1Headers = bRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to B (should be 'move 2'): "+bRes1Headers.get(Const.MOVE_VALUE));
					
					System.out.println("Calling handleRequest of second request from B");
					mod.handleRequest(fromBreq2, toBres2);
					TestModuleResponse bRes2 = (TestModuleResponse) toBres2;
					Map bRes2Headers = bRes2.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to B (should be 'Ack end'): "+bRes2Headers.get("type"));
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					return;
				}
			}
		});
		threadB.start();
		
		

		while(Thread.activeCount() != 0) {
			try {
				Thread.sleep(100);
			} catch(InterruptedException e) {
				System.out.println("### Thread interrupted");
			}
		}
	}
}
