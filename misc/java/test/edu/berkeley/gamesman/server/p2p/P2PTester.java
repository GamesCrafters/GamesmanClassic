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
		P2PModule.registerNewGame("Player D", "Player C");
		P2PModule.registerNewGame("Player F", "Player E");
		final byte[] trash = new byte[0];
		
		final String type = IModuleRequest.HN_TYPE;
		final String pA = "Player A";
		final String pB = "Player B";
		final String pC = "Player C";
		final String pD = "Player D";
		final String pE = "Player E";
		final String pF = "Player F";
		final String abm1 = "AB move 1";
		final String abm2 = "AB move 2";
		final String abm3 = "AB move 3";
		final String cdm1 = "CD move 1";
		final String cdm2 = "CD move 2";
		final String cdm3 = "CD move 3";
		final String efm1 = "EF move 1";
		//final String efResignation = "Player E resigned!";
		
		final String[] efhVals0 = {RequestType.SEND_MOVE, null, pF, pE};
		final String[] efhVals1 = {RequestType.SEND_MOVE, efm1, pE, pF};
		
		
		
		final String[] cdhVals0 = {RequestType.SEND_MOVE, null, pD, pC};
		final String[] cdhVals1 = {RequestType.SEND_MOVE, cdm1, pC, pD};
		final String[] cdhVals2 = {RequestType.SEND_MOVE, cdm2, pD, pC};
		final String[] cdhVals3 = {RequestType.SEND_MOVE, cdm3, pC, pD};				
		
		final String[] abhVals1 = {RequestType.SEND_MOVE, abm1, pA, pB};
		final String[] abhVals2 = {RequestType.SEND_MOVE, abm2, pB, pA};
		final String[] abhVals3 = {RequestType.SEND_MOVE, abm3, pA, pB};	
		final String[] abhVals0 = {RequestType.SEND_MOVE, null, pB, pA};
		final String[] stdReq = {type, Const.HN_MOVE, Const.HN_DESTINATION_PLAYER, Const.HN_SOURCE_PLAYER};
		final String[] gameOverReq = {type, Const.HN_DESTINATION_PLAYER, Const.HN_SOURCE_PLAYER};
		final String[] resignationReq = {type, Const.HN_DESTINATION_PLAYER, Const.HN_SOURCE_PLAYER};
		
		
		final String[] abGameOverVals = {RequestType.GAME_OVER, pB, pA};
		final String[] cdGameOverVals = {RequestType.GAME_OVER, pD, pC};
		final String[] efResignationVals = {RequestType.RESIGN, pF,pE};
		// Create the requests that will eventually be sent
		
		Thread threadA = new Thread(new Runnable () {
			// Client A
			public void run() {
				try {
					IModuleRequest fromAreq1 = new TestModuleRequest(trash, stdReq, abhVals0);
					IModuleRequest fromAreq2 = new TestModuleRequest(trash, stdReq, abhVals2);
					IModuleRequest fromAFinal = new TestModuleRequest(trash, gameOverReq, abGameOverVals);
					
					IModuleResponse toAres1 = new TestModuleResponse(0);
					IModuleResponse toAres2 = new TestModuleResponse(0);
					IModuleResponse toAFinal = new TestModuleResponse(0);
					System.out.println("Okay! Thread A started.");
					
					System.out.println("Calling handleRequest of first request from A");
					mod.handleRequest(fromAreq1, toAres1);
					TestModuleResponse aRes1 = (TestModuleResponse) toAres1;
					Map res1Headers = aRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to A (should be 'move 1'): "+res1Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of second request from A");
					mod.handleRequest(fromAreq2, toAres2);
					TestModuleResponse aRes2 = (TestModuleResponse) toAres2;
					Map res2Headers = aRes2.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to A (should be 'move 3'): "+res2Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of 'game over' request from A");
					mod.handleRequest(fromAFinal, toAFinal);
					TestModuleResponse aRes3 = (TestModuleResponse) toAFinal;
					Map res3Headers = aRes3.getHeadersWritten();				
					System.out.println("@@@@@@This was TYPE of what was sent back to A (should be 'Ack end'): "+res3Headers.get("type"));
					
					
					
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					return;
				}
				
			}
		});
		threadA.start();
		//Thread.sleep(100);
		
		Thread threadC = new Thread(new Runnable() {
			public void run() {
				try {
					IModuleRequest fromCreq1 = new TestModuleRequest(trash, stdReq, cdhVals0);
					IModuleRequest fromCreq2 = new TestModuleRequest(trash, stdReq, cdhVals2);
					IModuleRequest fromCFinal = new TestModuleRequest(trash, gameOverReq, cdGameOverVals);
					
					IModuleResponse toCres1 = new TestModuleResponse(0);
					IModuleResponse toCres2 = new TestModuleResponse(0);
					IModuleResponse toCFinal = new TestModuleResponse(0);
					System.out.println("Okay! Thread C started!");
					

					System.out.println("Calling handleRequest of first request from C");
					mod.handleRequest(fromCreq1, toCres1);
					TestModuleResponse cRes1 = (TestModuleResponse) toCres1;
					Map res1Headers = cRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to C (should be 'move 1'): "+res1Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of second request from C");
					mod.handleRequest(fromCreq2, toCres2);
					TestModuleResponse cRes2 = (TestModuleResponse) toCres2;
					Map res2Headers = cRes2.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to C (should be 'move 3'): "+res2Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of 'game over' request from C");
					mod.handleRequest(fromCFinal, toCFinal);
					TestModuleResponse cRes3 = (TestModuleResponse) toCFinal;
					Map res3Headers = cRes3.getHeadersWritten();				
					System.out.println("@@@@@@This was TYPE of what was sent back to C (should be 'Ack end'): "+res3Headers.get("type"));
					
				} catch(ModuleException e) {
					System.out.println(e.getLocalizedMessage());
				}
			}
		});
		threadC.start();
		Thread threadE = new Thread(new Runnable() {
			public void run() {
				try {
					IModuleRequest fromEreq1 = new TestModuleRequest(trash, stdReq, efhVals0);
					IModuleRequest fromEreq2 = new TestModuleRequest(trash, resignationReq, efResignationVals);
					
					
					IModuleResponse toEres1 = new TestModuleResponse(0);
					IModuleResponse toEres2 = new TestModuleResponse(0);
					
					System.out.println("Okay! Thread E started!");

					System.out.println("Calling handleRequest of first request from E");
					mod.handleRequest(fromEreq1, toEres1);
					TestModuleResponse eRes1 = (TestModuleResponse) toEres1;
					Map res1Headers = eRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to E (should be 'move 1'): "+res1Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of resignation from E");
					mod.handleRequest(fromEreq2, toEres2);
					TestModuleResponse eRes2 = (TestModuleResponse) toEres2;
					Map res2Headers = eRes2.getHeadersWritten();
					System.out.println("@@@@@@This was TYPE of what was sent back to E (should be 'Ack resign'): "+res2Headers.get(type));
					
				} catch(ModuleException e) {
					System.out.println(e.getLocalizedMessage());
				}
			}
		});
		threadE.start();
		
		Thread threadB = new Thread(new Runnable() {
			//Client B
			public void run() {
				try {
					IModuleRequest fromBreq1 = new TestModuleRequest(trash, stdReq, abhVals1);		
					IModuleRequest fromBreq2 = new TestModuleRequest(trash, stdReq, abhVals3);
					
					IModuleResponse toBres1 = new TestModuleResponse(0);
					IModuleResponse toBres2 = new TestModuleResponse(0);
					System.out.println("Okay! Thread B started!");
					
					System.out.println("Calling handleRequest of first request from B");
					mod.handleRequest(fromBreq1, toBres1);
					TestModuleResponse bRes1 = (TestModuleResponse) toBres1;
					Map bRes1Headers = bRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to B (should be 'move 2'): "+bRes1Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of second request from B");
					mod.handleRequest(fromBreq2, toBres2);
					TestModuleResponse bRes2 = (TestModuleResponse) toBres2;
					Map bRes2Headers = bRes2.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to B (should be 'null'): "+bRes2Headers.get(Const.HN_MOVE));
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					return;
				}
			}
		});
		threadB.start();
		
		Thread threadD = new Thread(new Runnable() {
			//Client D
			public void run() {
				try {
					IModuleRequest fromDreq1 = new TestModuleRequest(trash, stdReq, cdhVals1);		
					IModuleRequest fromDreq2 = new TestModuleRequest(trash, stdReq, cdhVals3);
					
					IModuleResponse toDres1 = new TestModuleResponse(0);
					IModuleResponse toDres2 = new TestModuleResponse(0);
					System.out.println("Okay! Thread D started!");
					
					System.out.println("Calling handleRequest of first request from D");
					mod.handleRequest(fromDreq1, toDres1);
					TestModuleResponse dRes1 = (TestModuleResponse) toDres1;
					Map dRes1Headers = dRes1.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to D (should be 'move 2'): "+dRes1Headers.get(Const.HN_MOVE));
					
					System.out.println("Calling handleRequest of second request from B");
					mod.handleRequest(fromDreq2, toDres2);
					TestModuleResponse dRes2 = (TestModuleResponse) toDres2;
					Map dRes2Headers = dRes2.getHeadersWritten();
					System.out.println("@@@@@@This was sent back to D (should be 'null'): "+dRes2Headers.get(Const.HN_MOVE));
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					return;
				}
			}
		});
		threadD.start();

		Thread threadF = new Thread(new Runnable() {
			public void run() {
				try {
					IModuleRequest fromFreq1 = new TestModuleRequest(trash, stdReq, efhVals1);		

					IModuleResponse toFres1 = new TestModuleResponse(0);

					System.out.println("Okay! Thread F started!");
					
					System.out.println("Calling handleRequest of first request from F");
					mod.handleRequest(fromFreq1, toFres1);
					TestModuleResponse dRes1 = (TestModuleResponse) toFres1;
					Map dRes1Headers = dRes1.getHeadersWritten();
					System.out.println("@@@@@@This was TYPE of what was sent to F (should be 'resign'): "+dRes1Headers.get(type));
					
				} catch(ModuleException e) {
					System.out.println("### ModuleException: "+e.getLocalizedMessage());
					return;
				}
			}
		});
		threadF.start();
		
		while(Thread.activeCount() > 1) {
			
			try {
				//System.out.println("           There are still pending threads...");
				Thread.sleep(5000);
			} catch(InterruptedException e) {
				System.out.println("### Thread interrupted");
			}
		}
		System.out.println("Successful termination!");
	}
}
