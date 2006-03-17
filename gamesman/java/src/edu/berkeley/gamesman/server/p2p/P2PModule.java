package edu.berkeley.gamesman.server.p2p;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import java.util.Hashtable;
import java.util.LinkedList;

/**
 * Handles messages passed between two gamesman clients.
 * 
 * @author Ramesh Sridharan
 */
public class P2PModule implements IModule
{
	public static final boolean DEBUGGING = false;
	private static Hashtable theGames;
		
	/**
	 * Wrapper class holding 2 strings.
	 * 
	 * @author ramesh
	 */
	static class ActiveGame {
		private String player1;
		private String player2;
		//LinkedList moduleQueue; // stores 2 active instances for this game
		ActiveGame(String p1, String p2) {
			player1=p1;
			player2=p2;
			//moduleQueue = new LinkedList();
		}
		
		public boolean equals(Object o) {
			if(o.getClass() != this.getClass())  {
				return false;
			} else {
				ActiveGame otherGame = (ActiveGame) o;
				boolean sameOrder = this.player1.equals(otherGame.player1) &&
									this.player2.equals(otherGame.player2);
				boolean oppositeOrder = this.player1.equals(otherGame.player2) &&
										this.player2.equals(otherGame.player1);
				return sameOrder || oppositeOrder;
				
			}
		}
		
		public String toString() {
			return player1+"==with=="+player2;
		}
		
		public int hashCode() {
			return this.toString().hashCode();
		}
	}
		
	static void debugprint(String s) {
		if(DEBUGGING) {
			System.out.println(s);
		}
	}
	
	/**
	 * Creates a game between user1 and user2 and stores it.
	 * 
	 * @param user1 First player
	 * @param user2 Second player
	 */
	static void registerNewGame(String user1, String user2) 
	{
		LinkedList gameQueue = new LinkedList();
		theGames.put(new ActiveGame(user1, user2), gameQueue);
		debugprint("Added a game between "+user1+" and "+user2);
		return;
	}
	
	private IModuleResponse pendingPlayerResponse;
	
	public P2PModule() 
	{
		super();
		
		// TODO Fill this in if necessary?
	}
	
	
	public boolean typeSupported(String requestTypeName)
	{
		// TODO Fill in any other possible types?
		return (requestTypeName == "SendMove");
	}

	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException {
		String sourcePlayer=req.getHeader("SourcePlayerName");
		String destPlayer = req.getHeader("DestPlayerName");
		ActiveGame theirGame = new ActiveGame(sourcePlayer, destPlayer);
		
		// TODO add cases for handling start and end of game
		if(req.getHeader("Move")==null) 
			;
		
		if(!theGames.containsKey(theirGame)) {
			throw new ModuleException(P2PMacros.NO_SUCH_GAME, "No game between these players exists");
			
		} else {
			//if(req.getHeader("Move")!=null) {
				// Extract the player to send the info to
				LinkedList gameQueue = (LinkedList) theGames.get(theirGame);
				P2PModule pendingModule = (P2PModule) gameQueue.removeLast();
				IModuleResponse pendingResponse = pendingModule.pendingPlayerResponse;
			
			// Set the response headers and send the info
			pendingResponse.setHeader("SourcePlayerName", sourcePlayer);
			pendingResponse.setHeader("DestPlayerName", destPlayer);
			pendingResponse.setHeader("Move", req.getHeader("Move"));
			debugprint("Send headers have been set...notifying module");
			pendingModule.notify();
			
			// store this game, then wait until the next player is ready
			gameQueue.addFirst(this);
			this.pendingPlayerResponse=res;
			try {
				this.wait();
			} catch(Exception e) {
				throw new ModuleException(P2PMacros.THREAD_INTERRUPTED, "Game was interrupted", e);
			}
			
			// Headers/etc have already been set, so terminate 
		}
	}
	
	static void finishGame(ActiveGame theGame)
	{
		theGames.remove(theGame);
		debugprint("Game between "+theGame.player1+" and "+theGame.player2+" finished.");
	}
}