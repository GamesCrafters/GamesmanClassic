package edu.berkeley.gamesman.server.p2p;

public final class Const {
	
	// Header fields
	public static final String HN_SOURCE_PLAYER = "SrcPlayer";
	public static final String HN_DESTINATION_PLAYER = "DestPlayer";
	public static final String HN_MOVE = "Move";
	// P2P is probably the only module that needs to set "type" in the response
	public static final String HN_TYPE = "Type"; 
	
	// Set either to true to print statements of that type while running
	public static final boolean PRINT_DEBUGGING = true;
	public static final boolean PRINT_WARNINGS = true;
}
