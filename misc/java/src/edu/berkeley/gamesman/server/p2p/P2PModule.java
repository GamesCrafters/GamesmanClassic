package edu.berkeley.gamesman.server.p2p;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;
import edu.berkeley.gamesman.server.RequestType;
import edu.berkeley.gamesman.server.registration.RegistrationModule;
import edu.berkeley.gamesman.server.registration.UserNode;

import java.util.Date;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

/**
 * IModule that handles message passing between two clients.
 * 
 * @author Ramesh Sridharan
 * 
 */

public class P2PModule implements IModule
{
	public static final String HN_USERNAME = "Username";
	public static final String HN_SECRET_KEY = "SecretKey";
	public static final String HN_GAME_ID = "GameId";
	public static final String HN_MOVE = "Move";
	public static final String HN_LAST_MOVE = "LastMove";
	
	private static final long TIMEOUT = 15 * 60 * 1000; // 15 mins
	private static Hashtable<String, ActiveGame> games = new Hashtable<String, ActiveGame>(200);
	private static Timer reaper = null;
	
	static 
	{
		reaper = new Timer(true);
		long period = 2 * 60 * 1000; // 2 mins
		TimerTask task = new TimerTask()
		{
			public void run()
			{
				long now = new Date().getTime();
				synchronized (games)
				{		
					Set<String> keys = new HashSet<String>(games.keySet());
					Iterator<String> e = keys.iterator();
					while (e.hasNext())
					{
						String key = e.next();
						ActiveGame game = games.get(key);						
						if ((game.getLastActiveDate().getTime() + TIMEOUT) < now)
						{
							games.remove(key);
							game.getPlayer1().setPlayingGame(false);
							game.getPlayer2().setPlayingGame(false);
						}
					}
				}
			}
		};
		// Run every "period" seconds
		reaper.scheduleAtFixedRate(task, period, period);
	}
	
	/**
	 * Registers a game between two players. Should be called by Registration module to
	 * register a game between players.
	 * 
	 * @param firstPlayer - Player who will move first
	 * @param secondPlayer - Player who will move second
	 */
	public static void startGame(String gameId, UserNode player1, UserNode player2)
	{
		ActiveGame game = new ActiveGame(gameId, player1, player2);
		synchronized (games)
		{
			games.put(gameId, game);
		}
	}
	
	
	public void initialize(String workingDir, String[] configArgs) throws ModuleInitializationException
	{
	}
	
	public boolean typeSupported(String requestTypeName)
	{
		return (requestTypeName.equalsIgnoreCase(RequestType.GAME_OVER) ||
				requestTypeName.equalsIgnoreCase(RequestType.SEND_MOVE) ||
				requestTypeName.equalsIgnoreCase(RequestType.GET_LAST_MOVE) || 				
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
		String type = req.getType();
		if (type.equalsIgnoreCase(RequestType.SEND_MOVE)) {
			sendMove(req, res);
		}
		else if (type.equalsIgnoreCase(RequestType.GET_LAST_MOVE)) {
			getLastMove(req, res);
		}				
		else if (type.equalsIgnoreCase(RequestType.GAME_OVER)) {
			gameOver(req, res); 
		}			
		else if (type.equalsIgnoreCase(RequestType.RESIGN)) {
			resign(req, res); 
		}			
		else {
			//the request type cannot be handled
			throw new ModuleException (ErrorCode.UNKNOWN_REQUEST_TYPE, ErrorCode.Msg.UNKNOWN_REQUEST_TYPE);
		}		
	}
	
	protected void getLastMove(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		// Get the ActiveGame
		ActiveGame game = getGame(req, res, true, true);
		if (game == null)
		{
			return;
		}
		else if (game.isPlayer1Abandoned() || game.isPlayer2Abandoned())
		{
			res.setReturnCode(ErrorCode.GAME_ABANDONED);
			res.setReturnMessage(ErrorCode.Msg.GAME_ABANDONED);			
		}

		String username = req.getHeader(HN_USERNAME);

		synchronized (game)
		{
			// See whose turn it is
			if ((game.getPlayer1().getUsername().equalsIgnoreCase(username) && game.isPlayer1Turn()) ||
					(game.getPlayer2().getUsername().equalsIgnoreCase(username) && !game.isPlayer1Turn()))
			{
				// It is our turn to move. So send the last move made by the other player.
				res.setHeader(HN_LAST_MOVE, (game.getLastMove() == null?"":game.getLastMove()));
			}
			else
			{
				// It is not our turn to move. Which means the last move made was by us and we're waiting 
				// for the other player to make a move. Send nothing.
			}
		}
	}
	
	protected void sendMove(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		// Get the ActiveGame
		ActiveGame game = getGame(req, res, true, true);
		if (game == null)
		{
			return;
		}
		else if (game.isPlayer1Abandoned() || game.isPlayer2Abandoned())
		{
			res.setReturnCode(ErrorCode.GAME_ABANDONED);
			res.setReturnMessage(ErrorCode.Msg.GAME_ABANDONED);			
		}

		String username = req.getHeader(HN_USERNAME);
		String currMove = req.getHeader(HN_MOVE);

		// Validate the move info
		if (currMove == null || currMove.trim().equals(""))
		{
			res.setReturnCode(ErrorCode.INVALID_MOVE_INFO);
			res.setReturnMessage(ErrorCode.Msg.INVALID_MOVE_INFO);
		}
		else
		{
			synchronized (game)
			{
				// See whose turn it is
				if ((game.getPlayer1().getUsername().equalsIgnoreCase(username) && game.isPlayer1Turn()) ||
						(game.getPlayer2().getUsername().equalsIgnoreCase(username) && !game.isPlayer1Turn()))
				{
					// It is our turn to move.
					game.setLastMove(currMove);
					game.switchTurn();					
				}
				else
				{
					// It is not our turn to move. We might be re-trasmitting the same move over again.
					if ((currMove == null && game.getLastMove() != null) || (currMove != null && game.getLastMove() == null) ||
							(currMove != null && game.getLastMove() != null && !currMove.equals(game.getLastMove())))
					{
						// Not a re-trasmission of the last move. This new move is different. Problem
						res.setReturnCode(ErrorCode.MOVE_OUT_OF_TURN);
						res.setReturnMessage(ErrorCode.Msg.MOVE_OUT_OF_TURN);
					}
				}
			}
		}		
	}
	
	protected void gameOver(IModuleRequest req, IModuleResponse res) throws ModuleException {	
		
		// Get the ActiveGame
		ActiveGame game = getGame(req, res, true, true);
		if (game == null)
			return;
		
		synchronized (games)
		{
			// Clear out the game
			games.remove(game.getId());
			game.getPlayer1().setPlayingGame(false);
			game.getPlayer2().setPlayingGame(false);
		}
	}
	
	protected void resign(IModuleRequest req, IModuleResponse res) throws ModuleException {
		
		// Get the ActiveGame
		ActiveGame game = getGame(req, res, true, true);
		if (game == null)
			return;

		// Abandon the game
		String username = req.getHeader(HN_USERNAME);
		if (game.getPlayer1().getUsername().equalsIgnoreCase(username))
			game.setPlayer1Abandoned(true);
		else
			game.setPlayer2Abandoned(true);
			
		if (game.isPlayer1Abandoned() && game.isPlayer2Abandoned())
		{
			synchronized (games)
			{
				// Clear out the game if both players abandoned
				games.remove(game.getId());
				game.getPlayer1().setPlayingGame(false);
				game.getPlayer2().setPlayingGame(false);
			}
		}
	}
		
	protected static ActiveGame getGame(IModuleRequest req, IModuleResponse res, 
			boolean respondErrorIfMissing, boolean updateActiveIfFound) {

		// Get the user
		UserNode node = RegistrationModule.getCredentials(req, res, respondErrorIfMissing, updateActiveIfFound);
		if (node == null)
			return null;
		
		// Look for the game
		String gameId = req.getHeader(HN_GAME_ID);
		ActiveGame game = null;
		synchronized (games)
		{
			game = games.get(gameId);
			if (game != null)
			{
				// Verify that this game involves this user
				if (game.getPlayer1() == node || game.getPlayer2() == node)
				{
					// Update the last active date
					if (updateActiveIfFound)
						game.updateLastActiveDate();
				}
				else
				{
					if (respondErrorIfMissing)
					{
						// No game found
						res.setReturnCode(ErrorCode.NO_GAME_FOUND);
						res.setReturnMessage(ErrorCode.Msg.NO_GAME_FOUND);
					}
				}					
			}
			else
			{
				if (respondErrorIfMissing)
				{
					// No game found
					res.setReturnCode(ErrorCode.NO_GAME_FOUND);
					res.setReturnMessage(ErrorCode.Msg.NO_GAME_FOUND);
				}
			}
		}
		return game;
	}
	
}
