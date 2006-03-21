package edu.berkeley.gamesman.server.p2p;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;
import java.util.HashMap;
import java.util.Hashtable;


public class P2PModule implements IModule {
	
	static Hashtable theGames = new Hashtable(200);
	
	/**
	 * Registers a game between two players. 
	 * 
	 * @param firstPlayer - Player who will move first
	 * @param secondPlayer - Player who will move second
	 */
	public static void registerNewGame(String firstPlayer, String secondPlayer) {
		int x = 0;
		x++;
		ActiveGame game = new ActiveGame(firstPlayer, secondPlayer);
		GameInfoContainer info = new GameInfoContainer(secondPlayer);
		debugPrint("Going to add the game and its info to the hashtable");
		synchronized(theGames) {
			theGames.put(game, info);
			//new ActiveGame(firstPlayer, secondPlayer), new GameInfoContainer(secondPlayer));
		}
		debugPrint("Registering new game");
	}
	
	
	/**
	 * Simple method that prints if the debugging flag is true.
	 * 
	 * @param o
	 */
	public static void debugPrint(Object o) {
		if(Const.DEBUGGING)  System.out.println("*** "+o.toString());
	}
	
	/**
	 * Class representing a game between two players. Holds two Strings representing players.
	 * @author ramesh
	 *
	 */
	static class ActiveGame {
		protected String player1;
		protected String player2;
		
		public String getPlayer1() {
			return player1;
		}
		
		public String getPlayer2() {
			return player2;
		}
		
		public ActiveGame(String p1, String p2) {
			debugPrint("Creating new active object between "+p1+" and "+p2);
			player1=p1;
			player2=p2;
			
		}
		
		public String toString() {
			
			String theString;
			if(player1.compareTo(player2) > 0) {
				theString =  (player2+"===with==="+player1);
			} else {
				theString =  (player1+"===with==="+player2);
			}
			//debugPrint("  Calling toString: "+theString);
			return theString;
		}
		
		public int hashCode() {
			//debugPrint("Hashing string: "+this.toString());
			return this.toString().hashCode();
		}
		
		public boolean equals(Object o) {			
			if(this.getClass() != o.getClass()) {
				return false;
			} else {
				ActiveGame that = (ActiveGame) o;
				return(this.toString().equals(that.toString()));
				//return ((this.player1 == that.player1 && this.player2 == that.player2) ||
				//		(this.player2 == that.player1 && this.player1 == that.player2));
			}
		}
	}
	/**
	 * Class holding information about a game. Contains a string representing
	 * player who has control of the game, and a pending response object to be 
	 * sent to the player waiting for a move to be made.
	 * 
	 * @author Ramesh Sridharan
	 *
	 */
	static class GameInfoContainer {
		protected String whoseTurn;
		protected IModuleResponse pendingResponse;
		
		public String getWhoseTurn() {
			return whoseTurn;
		}
		public GameInfoContainer(String whoseTurn) {
			debugPrint("Creating a new GameInfoContainer object");
			this.whoseTurn = whoseTurn;
			this.pendingResponse = null;
			debugPrint("Okay, created");
		}
		
		/**
		 * Changes whose turn it is, assuming the game is between players specified
		 * in the argument.
		 * 
		 * @param theGame
		 */
		public void switchTurn(ActiveGame theGame) {
			if(this.whoseTurn == theGame.getPlayer1()) {
				this.whoseTurn = theGame.getPlayer2();
			} else {
				this.whoseTurn = theGame.getPlayer1();
			}
		}
		
		/**
		 * Sets a new pending response, and returns the response object for the 
		 * currently waiting player.
		 * 
		 * @param newResponse
		 * @return
		 */
		public IModuleResponse changePendingResponse(IModuleResponse newResponse) {
			IModuleResponse old = this.pendingResponse;
			this.pendingResponse = newResponse;
			if(old == null) debugPrint("This should only print on the first move...");
			return old;
		}
	}
	
	public void initialize(String[] configArgs)
	throws ModuleInitializationException {}
	
	public boolean typeSupported(String requestTypeName) {
		return (requestTypeName == Const.END_OF_GAME || requestTypeName == Const.SEND_MOVE);
	}
	
	
	
	
	/**
	 * Handles requests passed between two players. 
	 * @param req
	 * @param res
	 */
	public void handleRequest(IModuleRequest req, IModuleResponse res)
	throws ModuleException {
		String incomingMove = null;
		if(req.getHeader("type") != Const.END_OF_GAME) {
			incomingMove	= req.getHeader(Const.MOVE_VALUE);
		}
		String destPlayer	= req.getHeader(Const.DESTINATION_PLAYER);
		String srcPlayer	= req.getHeader(Const.SOURCE_PLAYER);
		
		ActiveGame theGame = new ActiveGame(destPlayer, srcPlayer);
		GameInfoContainer gameStatus;
		if(!theGames.containsKey(theGame)) {
			throw new ModuleException(Const.NO_SUCH_GAME, "No game has been registered between "+srcPlayer+" and "+destPlayer);
		}
		
		gameStatus = (GameInfoContainer) theGames.get(theGame);
		IModuleResponse waitingResponse = gameStatus.changePendingResponse(res);
		debugPrint("");
		if(req.getHeader("type") == Const.END_OF_GAME) {
			debugPrint("Game over!");
			waitingResponse.setHeader(Const.TYPE, Const.SEND_MOVE);
			waitingResponse.setHeader(Const.MOVE_VALUE, "");
			waitingResponse.setHeader(Const.DESTINATION_PLAYER, destPlayer);
			waitingResponse.setHeader(Const.SOURCE_PLAYER, srcPlayer);
			res.setHeader("type", Const.ACKNOWLEDGE_END);
			synchronized(this) {
				notify();
			}
			return;
		}
		// Error-checking
		if((waitingResponse == null) && (incomingMove != null)) {
			throw new ModuleException(Const.INVALID_START_OF_GAME, "Game between "+srcPlayer+" and "+destPlayer+" has not yet been initialized");
		} else if(waitingResponse !=null && incomingMove == null) {
			throw new ModuleException(Const.GAME_ALREADY_INITIALIZED, "Game between "+srcPlayer+" and "+destPlayer+" has already been initialized");
		}
		if(gameStatus.getWhoseTurn() != srcPlayer) {
			throw new ModuleException(Const.WRONG_PLAYER_TURN, "It isn't "+srcPlayer+"'s turn.");
		}
		debugPrint("Okay! There were no errors detected.");
		gameStatus.switchTurn(theGame);
		if(incomingMove != null) {
			updateResponse(req, waitingResponse);
		}
		debugPrint("Going to notify and then wait...");
		synchronized(this) {
			this.notify();
			try {
				this.wait();
			} catch(InterruptedException e) {
				throw new ModuleException(Const.THREAD_INTERRUPTED, "Thread interrupted", e);
				
			}
		}
		
		
		
		
		
	}
	public void updateResponse(IModuleRequest data, IModuleResponse toBeUpdated) {
		String move = data.getHeader(Const.MOVE_VALUE);
		String src = data.getHeader(Const.SOURCE_PLAYER);
		String dest = data.getHeader(Const.DESTINATION_PLAYER);
		
		toBeUpdated.setHeader(Const.TYPE, Const.SEND_MOVE);
		toBeUpdated.setHeader(Const.MOVE_VALUE, move);
		toBeUpdated.setHeader(Const.DESTINATION_PLAYER, dest);
		toBeUpdated.setHeader(Const.SOURCE_PLAYER, src);
	}
	
}
