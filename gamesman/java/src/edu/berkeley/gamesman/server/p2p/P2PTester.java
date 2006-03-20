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
		// Create the requests that will eventually be sent

		Thread threadA = new Thread(new Runnable () {
			// Client A
			public void run() {
				try {
					IModuleRequest fromAreq1 = new TestModuleRequest(trash, stdReq, hVals0);
					IModuleRequest fromAreq2 = new TestModuleRequest(trash, stdReq, hVals2);
					IModuleResponse toAres1 = new TestModuleResponse(0);
					IModuleResponse toAres2 = new TestModuleResponse(0);
					
					mod.handleRequest(fromAreq1, toAres1);
					TestModuleResponse aRes1 = (TestModuleResponse) toAres1;
					Map res1Headers = aRes1.getHeadersWritten();
					System.out.println("***This was sent back to A (should be 'first move'): "+res1Headers.get(Const.MOVE_VALUE));
					
					mod.handleRequest(fromAreq2, toAres2);
					TestModuleResponse aRes2 = (TestModuleResponse) toAres2;
					Map res2Headers = aRes2.getHeadersWritten();
					System.out.println("***This was sent back to A (should be 'third move'): "+res2Headers.get(Const.MOVE_VALUE));
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					int x = 1/0;
				}
				
			}
		});
		threadA.start();
		Thread threadB = new Thread(new Runnable() {
			//Client B
			public void run() {
				try {
					IModuleRequest fromBreq1 = new TestModuleRequest(trash, stdReq, hVals1);		
					IModuleRequest fromBreq2 = new TestModuleRequest(trash, stdReq, hVals3);
					
					IModuleResponse toBres1 = new TestModuleResponse(0);
					IModuleResponse toBres2 = new TestModuleResponse(0);
					
					mod.handleRequest(fromBreq1, toBres1);
					TestModuleResponse bRes1 = (TestModuleResponse) toBres1;
					Map bRes1Headers = bRes1.getHeadersWritten();
					System.out.println("***This was sent back to B (should be 'second move'): "+bRes1Headers.get(Const.MOVE_VALUE));
					
					mod.handleRequest(fromBreq2, toBres2);
					TestModuleResponse bRes2 = (TestModuleResponse) toBres2;
					Map bRes2Headers = bRes2.getHeadersWritten();
					System.out.println("***This was sent back to B (should be 'fourth move'): "+bRes2Headers.get(Const.MOVE_VALUE));
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					int x = 5/0;
				}
			}
		});
		threadB.start();
		
		
		/* bad testing
		// setup the module and the game
		P2PModule theModule = new P2PModule();
		P2PModule.registerNewGame("Player B", "Player A");
		hPrint("Yay, registered!");
		ClientA a = new ClientA();
		ClientB b = new ClientB();
		Thread aThread = new Thread(a);
		Thread bThread = new Thread(b);
		// action!
		try {
			
			theModule.handleRequest(fromAreq1, toBres1);
			
			hPrint("First request 'handled'");
			
			
			TestModuleResponse test1 = (TestModuleResponse) toBres1;
			Map toBres1Map = test1.getHeadersWritten();
			hPrint("Should say null");
			hPrint((String) toBres1Map.get(Const.MOVE_VALUE));
			System.out.println();
			
			theModule.handleRequest(fromBreq1, toAres1);
			toBres1Map = test1.getHeadersWritten();
			hPrint("should say: first move of game");
		} catch(ModuleException e) {
			System.out.println("!! Exception: "+e.getMessage());
		}
		*/
		while(Thread.activeCount() != 0) {
			try {
				Thread.sleep(100);
			} catch(InterruptedException e) {
				System.out.println("### Thread interrupted");
			}
		}
	}
}
