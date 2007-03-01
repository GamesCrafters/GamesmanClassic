package edu.berkeley.gamesman.server.p2p;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;
import edu.berkeley.gamesman.server.RequestType;
import edu.berkeley.gamesman.server.registration.*;
import java.util.Hashtable;

/**
 * IModule that handles message passing between two clients.
 * 
 * @author Ramesh Sridharan
 * 
 */

/** TODO
 * secret key check - just use func in regmod
 * hash on serial number as well - includeded in header
 * document everything
 */
public class P2PModule implements IModule
{
	
	/* A hash table mapping ActiveGames to GameInfoContainers. */
	static Hashtable theGames = new Hashtable(200);
	
	/**
	 * Registers a game between two players. Should be called by Registration module to
	 * register a game between players.
	 * 
	 * @param firstPlayer - Player who will move first
	 * @param secondPlayer - Player who will move second
	 */
	public static void registerNewGame(String firstPlayer, String secondPlayer, int gameID)
	{
		ActiveGame game = new ActiveGame(firstPlayer, secondPlayer, gameID);
		GameInfoContainer info = new GameInfoContainer(secondPlayer);
		debugPrint("Going to add the game and its info to the hashtable");
		synchronized (theGames)
		{
			theGames.put(game, info);
		}
		debugPrint("Registered new game!");
	}
	
	/**
	 * Simple method that prints if the debugging flag is true.
	 * 
	 * @param o
	 */
	private static void debugPrint(Object o)
	{
		if (Const.PRINT_DEBUGGING)
			System.out.println("*** " + o.toString());
	}
	
	private static void warn(Object o)
	{
		if (Const.PRINT_WARNINGS)
		{
			System.err.println("WARNING: " + o.toString());
		}
	}
	/**
	 * Class representing a game between two players. Holds two Strings representing
	 * players and a serial game ID representing the game they're playing.
	 * 
	 * @author Ramesh Sridharan
	 * 
	 */
	protected static class ActiveGame
	{
		protected String player1;
		protected String player2;
		protected int gameID;
		
		public String getPlayer1()
		{
			return player1;
		}
		
		public String getPlayer2()
		{
			return player2;
		}
		
		public int getGameID() {
			return gameID;
		}
		
		public ActiveGame(String p1, String p2, int gameID)
		{
			debugPrint("Creating new active object between " + p1 + " and " + p2+", playing #"+gameID);
			player1 = p1;
			player2 = p2;
			this.gameID = gameID;
		}
		
		public String toString()
		{
			String theString;
			if (player1.compareTo(player2) > 0)
			{
				theString = (player2 + "===with===" + player1);
			}
			else
			{
				theString = (player1 + "===with===" + player2);
			}
			theString+= " playing"+gameID;
			return theString;
		}
		
		public int hashCode()
		{
			return this.toString().hashCode();
		}
		
		public boolean equals(Object o)
		{
			if (this.getClass() != o.getClass())
			{
				return false;
			}
			else
			{
				ActiveGame that = (ActiveGame) o;
				return (this.toString().equals(that.toString()));
			}
		}
	}
	/**
	 * Class holding information about a game. Contains a string representing player who
	 * has control of the game, and a pending response object to be sent to the player
	 * waiting for a move to be made.
	 * 
	 * @author Ramesh Sridharan
	 * 
	 */
	protected static class GameInfoContainer
	{
		protected String whoseTurn;
		protected IModuleResponse pendingResponse;
		
		public String getWhoseTurn()
		{
			return whoseTurn;
		}
		
		public GameInfoContainer(String whoseTurn)
		{
			debugPrint("Creating a new GameInfoContainer object");
			this.whoseTurn = whoseTurn;
			this.pendingResponse = null;
			debugPrint("Okay, created");
		}
		
		/**
		 * Changes whose turn it is, assuming the game is between players specified in the
		 * argument.
		 * 
		 * @param theGame
		 */
		public void switchTurn(ActiveGame theGame)
		{
			if (this.whoseTurn.equalsIgnoreCase(theGame.getPlayer1()))
			{
				this.whoseTurn = theGame.getPlayer2();
			}
			else
			{
				this.whoseTurn = theGame.getPlayer1();
			}
		}
		
		/**
		 * Sets a new pending response, and returns the response object for the currently
		 * waiting player.
		 * 
		 * @param newResponse
		 * @return
		 */
		public IModuleResponse changePendingResponse(IModuleResponse newResponse)
		{
			IModuleResponse old = this.pendingResponse;
			this.pendingResponse = newResponse;
			if (old == null)
				debugPrint("This should only print on the first move...");
			return old;
		}
	}
	
	public void initialize(String workingDir, String[] configArgs) throws ModuleInitializationException
	{
	}
	
	public boolean typeSupported(String requestTypeName)
	{
		return (requestTypeName.equalsIgnoreCase(RequestType.GAME_OVER) ||
				requestTypeName.equalsIgnoreCase(RequestType.INIT_GAME) || 
				requestTypeName.equalsIgnoreCase(RequestType.SEND_MOVE) || 
				requestTypeName.equalsIgnoreCase(RequestType.RESIGN));
	}
	
	/**
	 * Handles requests passed between two players.
	 * 
	 * @param req
	 * @param res
	 */
	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		String type = req.getHeader("type");
		String incomingMove = null;
		String destPlayer = req.getHeader(Const.HN_DESTINATION_PLAYER);
		String srcPlayer = req.getHeader(Const.HN_SOURCE_PLAYER);
		String srcPlayerKey = req.getHeader(Const.HN_SOURCE_PLAYER_SECRET_KEY);
		String gameIDString = req.getHeader(Const.HN_GAME_ID);
		int gameID = Integer.parseInt(gameIDString);
		
		if(!RegistrationModule.isValidUserKey(srcPlayer, srcPlayerKey)) {
			// invalid key!
			warn("Incorrect secret key!");
			return;
		}
		
		ActiveGame theGame = new ActiveGame(destPlayer, srcPlayer, gameID);
		GameInfoContainer gameStatus;
		if (!theGames.containsKey(theGame))
		{
			throw new ModuleException(ErrorCode.NO_SUCH_GAME, "No game has been registered between "
					+ srcPlayer + " and " + destPlayer);
		}
		
		gameStatus = (GameInfoContainer) theGames.get(theGame);
		IModuleResponse waitingResponse = gameStatus.changePendingResponse(res);
		
		// possibly clean this up to avoid code duplication
		if (type.equalsIgnoreCase(RequestType.SEND_MOVE))
		{
			incomingMove = req.getHeader(Const.HN_MOVE);
		}
		else if (type.equalsIgnoreCase(RequestType.INIT_GAME))
		{
			// Do nothing
		}
		else if (type.equalsIgnoreCase(RequestType.GAME_OVER))
		{
			debugPrint("Game over!");
			waitingResponse.setHeader(Const.HN_TYPE, RequestType.GAME_OVER);
			waitingResponse.setHeader(Const.HN_MOVE, "null");
			waitingResponse.setHeader(Const.HN_DESTINATION_PLAYER, destPlayer);
			waitingResponse.setHeader(Const.HN_SOURCE_PLAYER, srcPlayer);
			res.setHeader("type", Const.ACK); // Still needed?
			res.setHeader(Const.HN_DESTINATION_PLAYER, destPlayer);
			res.setHeader(Const.HN_SOURCE_PLAYER, srcPlayer);
			synchronized (gameStatus)
			{
				gameStatus.notifyAll();
			}
			theGames.remove(theGame);
			return;
		}
		else if (type.equalsIgnoreCase(RequestType.RESIGN))
		{
			waitingResponse.setHeader(Const.HN_TYPE, RequestType.RESIGN);
			waitingResponse.setHeader(Const.HN_SOURCE_PLAYER, srcPlayer);
			waitingResponse.setHeader(Const.HN_DESTINATION_PLAYER, destPlayer);
			res.setHeader("type", Const.ACK); // Still needed?
			res.setHeader(Const.HN_DESTINATION_PLAYER, destPlayer);
			res.setHeader(Const.HN_SOURCE_PLAYER, srcPlayer);
			
			synchronized (gameStatus)
			{
				gameStatus.notifyAll();
			}
			theGames.remove(theGame);
			return;
		}
		// Error-checking
		if ((waitingResponse == null) && (incomingMove != null))
		{
			warn("Game between " + srcPlayer + " and " + destPlayer
					+ " has not yet been initialized");
			return;
			// throw new ModuleException(Const.INVALID_START_OF_GAME, "Game between
			// "+srcPlayer+" and "+destPlayer+" has not yet been initialized");
		}
		else if (waitingResponse != null && incomingMove == null)
		{
			warn("Game between " + srcPlayer + " and " + destPlayer
					+ " has already been initialized");
			return;
			// throw new ModuleException(Const.GAME_ALREADY_INITIALIZED, "Game between
			// "+srcPlayer+" and "+destPlayer+" has already been initialized");
		}
		if (gameStatus.getWhoseTurn().equalsIgnoreCase(srcPlayer))
		{
			warn("It isn't " + srcPlayer + "'s turn. It's " + gameStatus.getWhoseTurn() + "'s turn.");
			return;
			// throw new ModuleException(Const.WRONG_PLAYER_TURN, "It isn't
			// "+srcPlayer+"'s turn.");
		}
		debugPrint("Okay! There were no errors detected.");
		gameStatus.switchTurn(theGame);
		if (incomingMove != null)
		{
			updateResponse(req, waitingResponse);
		}
		debugPrint("Going to notify and then wait...");
		synchronized (gameStatus)
		{
			gameStatus.notify();
			try
			{
				gameStatus.wait();
			}
			catch (InterruptedException e)
			{
				throw new ModuleException(ErrorCode.THREAD_INTERRUPTED, "Thread interrupted", e);
				
			}
		}
	}
	
	private void updateResponse(IModuleRequest data, IModuleResponse toBeUpdated)
	{
		String move = data.getHeader(Const.HN_MOVE);
		String src = data.getHeader(Const.HN_SOURCE_PLAYER);
		String dest = data.getHeader(Const.HN_DESTINATION_PLAYER);
		String gameIDString = data.getHeader(Const.HN_GAME_ID);
		
		toBeUpdated.setHeader(Const.HN_TYPE, RequestType.SEND_MOVE);
		
		toBeUpdated.setHeader(Const.HN_MOVE, move);
		toBeUpdated.setHeader(Const.HN_DESTINATION_PLAYER, dest);
		toBeUpdated.setHeader(Const.HN_SOURCE_PLAYER, src);
		toBeUpdated.setHeader(Const.HN_GAME_ID, gameIDString);
	}
	
}
