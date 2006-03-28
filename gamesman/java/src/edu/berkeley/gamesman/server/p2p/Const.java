package edu.berkeley.gamesman.server.p2p;

public final class Const {
	
	// Error codes
	public static final int NO_SUCH_GAME = 1;
	public static final int THREAD_INTERRUPTED = 2;
	
	// Obsolete error codes (these now only give warnings)
	public static final int INVALID_START_OF_GAME = 3;
	public static final int GAME_ALREADY_INITIALIZED = 4;
	public static final int WRONG_PLAYER_TURN = 5;
	
	// Header fields
	public static final String TYPE = "type";
	public static final String SOURCE_PLAYER = "SrcPlayer";
	public static final String DESTINATION_PLAYER = "DestPlayer";
	public static final String MOVE_VALUE = "Move";
	
	// Header types
	public static final String SEND_MOVE = "SendMove";
	public static final String END_OF_GAME = "GameOver";
	public static final String ACKNOWLEDGE_END = "Ack end";
	public static final String SEND_RESIGNATION = "Resign";
	public static final String ACKNOWLEDGE_RESIGNATION = "Ack resign";
	
	// Set either to true to print statements of that type while running
	public static final boolean PRINT_DEBUGGING = false;
	public static final boolean PRINT_WARNINGS = true;
}