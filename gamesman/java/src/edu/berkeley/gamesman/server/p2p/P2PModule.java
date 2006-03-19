package edu.berkeley.gamesman.server.p2p;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import java.util.Hashtable;


/**
 * Handles messages passed between two GAMESMAN clients.
 *
 * @author Ramesh Sridharan
 */
public class P2PModule implements IModule {
	
	
	static void debugPrint(String s) {
		if(P2PMacros.DEBUGGING) {
			System.out.println(s);			
		}
	}
	// hashes ActiveGames (see below) to current active associated IModuleResponse
	static private Hashtable theGames = new Hashtable();
	
	static class ActiveGame {
		// improvement: instead of wasting extra string, just make order count
		private String player1;
		private String player2;
		private String whoseTurn;
		/*
		private IModuleResponse pendingResponse;
		
		public void setaCurrentResponse(IModuleResponse r) {
			pendingResponse = r;
		}
		
		public IModuleResponse getandUpdateRes(IModuleResponse r) {
			IModuleResponse old = this.pendingResponse;
			this.pendingResponse = r;
			return old;
		}
		*/
		public String getPlayer1() {
			return player1;
		}
		
		public String getPlayer2() {
			return player2;
		}
		
		public String getWhoseTurn() {
			return whoseTurn;
		}
		
		/*
		public IModuleResponse getaPendingResponse() {
			return pendingResponse;
		}
		*/
		public String toString() {
			if(this.whoseTurn == this.player1) {
				return (player1 + "===with===" + player2);
			} else return (player2 + "===with===" + player1);
			
		}
		
		public int hashCode() {
			debugPrint("Hashing string: "+this.toString());
			return this.toString().hashCode();
		}
		
		public boolean equals(Object o) {
			if(this.getClass() != o.getClass()) {
				debugPrint("Class mismatch");
				return false;
			} else {
				debugPrint("Comparing 2 ActiveGames in format player1, player2, whoseTurn");
				debugPrint("First : "+this.player1+", "+this.player2+", "+this.whoseTurn);
				ActiveGame otherGame = (ActiveGame) o;
				debugPrint("Second: "+otherGame.player1+", "+otherGame.player2+", "+otherGame.whoseTurn);
				
				// order doesn't matter, so check both possibilities
				if(this.whoseTurn == otherGame.whoseTurn) {
					return ((this.player1==otherGame.player1 && this.player2==otherGame.player2) ||
							(this.player2==otherGame.player1 && this.player1==otherGame.player2));
				} else return false;
			}
		}
		
		public ActiveGame(String p1, String p2, String currentTurn) {
			player1 = p1;
			player2 = p2;
			whoseTurn = currentTurn;
			//pendingResponse = null;
			debugPrint("Creating new active game between "+p1+","+p2+". It's "+currentTurn+"'s turn.");
		}
		
		public static ActiveGame switchTurn(ActiveGame existingGame) {
			
			if (existingGame.whoseTurn == existingGame.player1) {
				return new ActiveGame(existingGame.player1, existingGame.player2, existingGame.player2);
			} else {
				return new ActiveGame(existingGame.player1, existingGame.player2, existingGame.player1);
			}
		}
	}
	
	static void registerNewGame(String p1, String p2) {
		synchronized(theGames) {
			theGames.put(new ActiveGame(p1, p2, p1), new Integer(0));
		}
		debugPrint("Registered new game between"+p1+","+p2);
	}
	
	public P2PModule() { }
	
	public void initialize(String[] initStrings) { }
	
	public boolean typeSupported(String requestTypeName) {
		return (requestTypeName == P2PMacros.SEND_MOVE || requestTypeName == P2PMacros.END_OF_GAME);
	}

	public void handleRequest(IModuleRequest req, IModuleResponse res)
	throws ModuleException {
		
		String incomingMove = req.getHeader(P2PMacros.MOVE_VALUE);
		String destPlayer = req.getHeader(P2PMacros.DESTINATION_PLAYER);
		String srcPlayer = req.getHeader(P2PMacros.SOURCE_PLAYER);
		debugPrint("Move, source, and destination: "+incomingMove+", "+srcPlayer+", "+destPlayer);
		ActiveGame game = new ActiveGame(destPlayer, srcPlayer, srcPlayer); // it's always the sender's turn
		if(incomingMove == null) {
			// first move
			ActiveGame actualGame = ActiveGame.switchTurn(game); // since the player who sent the null is second
			Object shouldBeZero = theGames.remove(actualGame);
			debugPrint(shouldBeZero.toString());
			theGames.put(actualGame, res);
			debugPrint("Initialized the game with a valid response in the hashtable");
			return;
		}
		
		debugPrint("Going to handle move from"+srcPlayer+" to "+destPlayer);
		synchronized(theGames) {
			if(!theGames.containsKey(game)) {
				throw new ModuleException(P2PMacros.NO_SUCH_GAME, "No game exists between "+destPlayer+" and "+srcPlayer);
			}
		}
		synchronized(theGames) {
			// update the game's pending response
			IModuleResponse waitingPlayerResponse = (IModuleResponse) theGames.remove(game); 
			waitingPlayerResponse.setHeader(P2PMacros.MOVE_VALUE, incomingMove);
			waitingPlayerResponse.setHeader(P2PMacros.DESTINATION_PLAYER, destPlayer);
			waitingPlayerResponse.setHeader(P2PMacros.SOURCE_PLAYER, srcPlayer);
			theGames.put(ActiveGame.switchTurn(game), res);
		}

			
		// everything after this doesn't really work
		synchronized(this) {
			notify();
			/*
		}
		synchronized(this) { */
			try {
				debugPrint("Trying to wait...");
				wait();
			} catch(InterruptedException e) {
				throw new ModuleException(P2PMacros.THREAD_INTERRUPTED, "Thread interrupted", e);
			}
		}
		debugPrint("Thread from "+srcPlayer+" to "+destPlayer+" woke up.");
		
	}
	
}
