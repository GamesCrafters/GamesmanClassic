package edu.berkeley.gamesman.parallel;

import java.util.Map;

public interface TierEventListener {
	/**
	 * Called to initialize the thing.
	 * @param fullMap
	 * @param toSolveMap
	 */
	public void setup(Map<Long, TierTreeNode> fullMap, Map<Long, TierTreeNode> toSolveMap);
	
	/**
	 * Called when setup is complete and clients can start doing stuff.
	 */
	public void start();
	
	/**
	 * Called when a tier is ready to be solved. Gives the priority of that tier as well
	 * @param tier
	 */
	public void tierMoveToReady(long tier);
	
	/**
	 * Called when a tier is starting to solve.
	 * @param tier
	 * @param coreOn 
	 */
	public void tierStartSolve(long tier, int coreOn);
	
	/**
	 * Called when a tier is finished solving
	 * @param tier
	 * @param bad
	 * @param seconds
	 */
	public void tierFinishedSolve(long tier, boolean bad, double seconds);
	
	/**
	 * Called when solver is done.
	 * @param errors
	 */
	public void done(boolean errors);

}
