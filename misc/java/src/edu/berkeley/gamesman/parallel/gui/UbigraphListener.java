package edu.berkeley.gamesman.parallel.gui;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.ubiety.ubigraph.UbigraphClient;

import edu.berkeley.gamesman.parallel.TierEventListener;
import edu.berkeley.gamesman.parallel.TierTreeManager;
import edu.berkeley.gamesman.parallel.TierTreeNode;

public class UbigraphListener implements TierEventListener {

	TierTreeManager ttm;
	Map<Long, Integer> tierToVIDMap = new HashMap<Long, Integer>();
	UbigraphClient graphClient;
	String server;
	
	
	public UbigraphListener(TierTreeManager ttm, String server)
	{
		this.ttm = ttm;
		this.server = server;
	}
	
	public void setup(Map<Long, TierTreeNode> fullMap, Map<Long, TierTreeNode> toSolveMap) 
	{
		initUbigraph(toSolveMap, fullMap);
	}

	public void start()
	{
		//no need to do antuhing ere.
	}
	
	public void tierMoveToReady(long tier)
	{
		//Ignored for now
	}

	public void tierFinishedSolve(long tier, boolean bad, double seconds) 
	{
		//set back to normal size and make it yellow/red
		String color = bad?"#FF0000":"#FFFF00";
		graphClient.setVertexAttribute(tierToVIDMap.get(tier), "color", color);
		graphClient.setVertexAttribute(tierToVIDMap.get(tier), "size", "1.0");
	}

	public void tierStartSolve(long tier, int coreOn) 
	{
		//lets make it bigger and cyan!
		graphClient.setVertexAttribute(tierToVIDMap.get(tier), "color", "#00FFFF");
		graphClient.setVertexAttribute(tierToVIDMap.get(tier), "size", "2.0");
	}
	
	public void done(boolean errors)
	{
		//Nothing needs to be done.
	}
	
	private void initUbigraph(Map<Long, TierTreeNode> tierMap, Map<Long, TierTreeNode> ubigraphMap)
	{
		//create localhost graph client
		System.out.println("Creating new client object connected to server "+server+"...");
		graphClient = new UbigraphClient(server);
		System.out.println("Clearing graph area...");
		graphClient.clear();
		
		int styleID = graphClient.newEdgeStyle(0);
		graphClient.setEdgeStyleAttribute(styleID, "arrow", "true");
		
		System.out.println("Creating verticies...");
		//create all the vertexies first
		for (Iterator<TierTreeNode> i = ubigraphMap.values().iterator();i.hasNext();)
		{			
			TierTreeNode ttnCur = i.next();
			int VID = graphClient.newVertex();
			graphClient.setVertexAttribute(VID, "label", "Tier " + ttnCur.tierNum);
			tierToVIDMap.put(ttnCur.tierNum, VID);
			if (tierMap.containsKey(ttnCur.tierNum) == false)
				graphClient.setVertexAttribute(VID, "color", "#FFFF00");
		}
		System.out.println("Creating edges...");
		//now, set up edges. set up on dependencies.
		for (Iterator<TierTreeNode> i = ubigraphMap.values().iterator();i.hasNext();)
		{
			TierTreeNode ttnCur = i.next();
			for (Iterator<TierTreeNode> i2 = ttnCur.deps.iterator();i2.hasNext();)
			{
				TierTreeNode dep = i2.next();
				int edge = graphClient.newEdge(tierToVIDMap.get(ttnCur.tierNum), tierToVIDMap.get(dep.tierNum));
				//graphClient.changeEdgeStyle(edge, styleID);
			}
			if (ttnCur.selfDependent)
			{
				graphClient.newEdge(tierToVIDMap.get(ttnCur.tierNum), tierToVIDMap.get(ttnCur.tierNum));
			}
		}
	}
}
