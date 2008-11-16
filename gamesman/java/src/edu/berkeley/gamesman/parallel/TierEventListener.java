package edu.berkeley.gamesman.parallel;

import java.util.Map;

public interface TierEventListener {
	public void setup(Map<Long, TierTreeNode> fullMap, Map<Long, TierTreeNode> toSolveMap);
	public void tierStartSolve(long tier);
	public void tierFinishedSolve(long tier, boolean bad, double seconds);
	public void done(boolean errors);
}
