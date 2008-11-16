package edu.berkeley.gamesman.parallel;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Reader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map;
import java.util.PriorityQueue;

import javax.swing.JOptionPane;


import edu.berkeley.gamesman.parallel.gui.UbigraphListener;

/**
 * This class manages the task of keeping the ThreadManager as busy as possible
 * by supplying it with tiers to solve. 
 * Also the entry point.  
 * 
 * @author John Ng
 *
 */
public class TierTreeManager
{
	
	
	public static final boolean FAKE = false;
	
	public static Reader generateReader(String game, String dir, int option) throws IOException
	{
		if (FAKE)
		{
			return new FileReader(JOptionPane.showInputDialog("what file", game + ".in"));
		}
		else
		{
			if (dir.endsWith("/")==false)
				dir += "/";
			File d = new File(dir);
			ProcessBuilder pb;
			if (option == -1)
				pb = new ProcessBuilder(dir + game, "--analyze", "--vtp");
			else
				pb = new ProcessBuilder(dir + game, "--analyze", "--vtp", "--option", ""+option);
			pb.directory(d);
			Process p = pb.start();
			return new InputStreamReader(p.getInputStream());
		}
	}
	
	public static void main(String[] args) throws IOException
	{
		TierTreeManager ttm = new TierTreeManager();
		if (args.length == 0)
		{
		//	ttm.initAuto();
		}
		//else
		{
			String usage = "Syntax:\n" +
					"java edu.berekley.gamesman.parallel.TierTreeManager numThreads gamePath game priorityAlg {option}\n" +
					"\n" +
					"numThreads		Number of threads (and therefore processes) to spawn (2,3,8).\n" +
					"gamePath		Path of the bin folder (usually ../../bin).\n" +
					"game			Game you want to solve (mago, m6mm, etc.).\n" +
					"priorityAlg		Algorithm to prioritize tiers. -1 means complete analysis, 0,1,2,... \n" +
					"			uses a huerestic, the larger, the more accurate. -1 is fine for anything\n" +
					"			except mancala; for that game, use a huerestic.\n" +
					"option			Variant or option to run the game under. Blank or -1 uses default option.";
			if (args.length < 4)
			{
				//print usage string
				System.out.println(usage);
				return;
			}
			else
			{
				int numThreads;
				String gamePath;
				String game;
				int priOp;
				int option = -1;
				try
				{
					numThreads = Integer.parseInt(args[0]);
					gamePath = args[1];
					game = args[2];
					priOp = Integer.parseInt(args[3]);
					if (args.length > 4)
					{
						option = Integer.parseInt(args[4]);
					}
				}
				catch (Exception e)
				{
					System.out.println(usage);
					return;
				}
				
				ttm.init(numThreads, gamePath, game, priOp, option);
			}
		}

	}
	
	TierThreadManager ttm;
	Map<Long, TierTreeNode> tierMap = new HashMap<Long, TierTreeNode>();
	Map<Long, TierTreeNode> fullMap = new HashMap<Long, TierTreeNode>(); //This one contians ALL tiers
	PriorityQueue<TierTreeNode> freeTiers = new PriorityQueue<TierTreeNode>();
	Map<Long, TierTreeNode> badTiers = new HashMap<Long, TierTreeNode>();
	Map<Long, TierTreeNode> beingSolved = new HashMap<Long, TierTreeNode>();
	TierEventListener[] listeners = new TierEventListener[]{new UbigraphListener(this)};
	
	int minCoresIdle;
	PrintWriter logOutput;
	int cores;
	
	void initAuto() throws IOException
	{

		//ask for data input
		int numThreads = Integer.parseInt(JOptionPane.showInputDialog("Number of threads to use max", "2"));

		String gamePath;
		String game;
		int priOp;
		int option;
		if (!FAKE)
		{
			gamePath = JOptionPane.showInputDialog("Directory of game to be solved?", "../bin");
			game = JOptionPane.showInputDialog("Game executable name?", "mago");
			priOp = Integer.parseInt(JOptionPane.showInputDialog("Priority algorithm? -1 for complete, 0+ for huerestic", "-1"));
			option  = Integer.parseInt(JOptionPane.showInputDialog("Option? -1 for default option", "-1"));
		}
		else
			
		{
			gamePath = "";
			game = JOptionPane.showInputDialog("Game executable name?", "m6mm");
			priOp = -1;
			option = -1;
		}
		
		init(numThreads, gamePath, game, priOp, option);
	}
	
	void init(int numThreads, String gamePath, String game, int priOp, int option) throws IOException
	{
		logOutput = new PrintWriter(new File("game" + game + "_" + option + ".log"));
		//make a new tierthreadmanager		
		ttm = new TierThreadManager(numThreads, this, gamePath, game, option);
		//now, start the fun.
		
		//generate the tiertree
		//vtp stuffs
		generateTierTree(priOp, gamePath, game, option);
		notifyListenersSetup();
		minCoresIdle = numThreads;
		cores = numThreads;
		spawnNewThreads();
	}
	
	void generateTierTree(int priOp, String gamePath, String game, int option) throws IOException
	{
		//for now, use a dummy file.
		BufferedReader br = new BufferedReader(generateReader(game, gamePath, option));
		String line;
		
		while ((line = br.readLine()).equals("STARTVTP") == false);

		System.out.println("Reading data in...");
		//ok. begin processing
		//for now, just process the new tiers coming up. Save
		//the dependencies for later.
		LinkedList<String> deps = new LinkedList<String>();
		boolean solved = false;
		while ((line = br.readLine()).equals("ENDALLVTP") == false)
		{
			if (line.startsWith("d"))
			{
				//it was a tier dep mapping
				//store it for later (strip off d)
				deps.add(line);
			}
			else if (line.startsWith("t"))
			{
				//this is a tier 'declaration' and its size

				long tier = Long.parseLong(line.substring(1, line.indexOf(":")));
				long size = Long.parseLong(line.substring(line.indexOf(":")+1));
				if (!solved)	//Only non-solved tiers in here
					tierMap.put(tier, new TierTreeNode(tier, size, solved));
				fullMap.put(tier, new TierTreeNode(tier, size, solved));
			}
			else if (line.equals("ENDVTP"))
			{
				//Switching to solved tiers.
				solved = true;
			}
		}
		//now, just read in the rest
		while ((line = br.readLine()) != null);
		br.close();
		System.out.println("Done reading data in, doing initial process...");
		
		//OK, now process the dependicie mappings
		Iterator<String> i = deps.iterator();
		while (i.hasNext())
		{
			String dep = i.next();
			//parse
			long lParent = Long.parseLong(dep.substring(1, dep.indexOf(":")));
			long lChild = Long.parseLong(dep.substring(dep.indexOf(":")+1));
			//Check if it's the same thing - loopy dependence is ok
			if (lParent != lChild)
			{
				//The parent depends on the child.
				
				//set up the 2 way mapping-ish thing
				TierTreeNode pttn = tierMap.get(lParent);
				TierTreeNode cttn = tierMap.get(lChild);
				
				//the child may not exist - already been solved, and wasn't included
				//on the vtp list. If so, ignore it. 
				//The parent under no circumstances shold be null, so don't check that
				if (cttn != null)
				{
					pttn.deps.add(cttn);
					cttn.parents.add(pttn);
					pttn.wasPrim = false;
				}
				
				//now, update the ubigraph links. None of these should be null
				pttn = fullMap.get(lParent);
				cttn = fullMap.get(lChild);
				pttn.deps.add(cttn);
				cttn.parents.add(pttn);
				pttn.wasPrim = false;
			}
			else
			{
				//set flag that this tier depends on itself...
				TierTreeNode ttn = tierMap.get(lParent);
				if (ttn != null)	//May not be in the to-solve map.
					ttn.selfDependent = true;
				//but has to be in the full map.
				ttn = fullMap.get(lParent);
				ttn.selfDependent = true;
				
			}
		}
		//Tier tree is generated!
		
		//now, parse through the tree and determine the priority of all the tiers
		//Find all the top tiers - tier without parents, and set their priorities to 1

		
		System.out.println("Generating priorities...");
		if (priOp < 0)
		{
			//Use the complete algorithm. 
			for (Iterator<TierTreeNode> i2 = tierMap.values().iterator();i2.hasNext();)
			{			
				TierTreeNode ttnCur = i2.next();
				//if it's a top tier, recurse down the tree doing dependencies.
				if (ttnCur.parents.size() == 0)
				{
					System.out.print(ttnCur.tierNum + ",");
					ttnCur.propagateDependencies(1.0);
				}
			}
		}
		else
		{
			//So for this algorithm, we assign the priproty based on 
			//the number of people that depend on it. We look up N levels.
			for (Iterator<TierTreeNode> i2 = tierMap.values().iterator();i2.hasNext();)
			{
				TierTreeNode ttnCur = i2.next();
				//process.
				ttnCur.priority = ttnCur.calculateHeuristic(priOp, ttnCur.tierNum);
			}
		}

		System.out.println("Determining free tiers...");
		//Now, put stuff into the freelist if they don't have dependencies.
		for (Iterator<TierTreeNode> i2 = tierMap.values().iterator();i2.hasNext();)
		{		
			//Put into freelist
			TierTreeNode ttnCur = i2.next();
			if (ttnCur.deps.size() == 0)
			{
				freeTiers.add(ttnCur);
			}
		}
		System.out.println("Done generating tier tree!");
		System.out.println("Found " + tierMap.size() + " tiers to solve");
		System.out.println("Found " + freeTiers.size() + " free tiers to start with.");
		
	}

	/**
	 * 
	 * @param t
	 * @param retCode
	 * @param seconds
	 */
	synchronized void finished(long t, int retCode, double seconds)
	{
		System.out.println(t + " is finished in " + seconds + " seconds");
		//Find this tier and remove it.
		//Remove from both lists
		TierTreeNode ttn = tierMap.get(t);
		beingSolved.remove(t);
		
		if (retCode != 0)
		{
			//something went wrong. Add to badtiers
			badTiers.put(t, ttn);
			System.out.println("!!!!!!" + t + " ended with retcode "+retCode+", and was added to badTiers list.");
			notifyListenersFinished(t, true, seconds);
		}
		else
		{
			//ok, remove from tierMap,.
			tierMap.remove(t);
			long size = ttn.size;
			boolean isPrimitiveTier = ttn.wasPrim;
			//log it - size/seconds = number of positions per second
			logOutput.println((isPrimitiveTier?"Primitive ":"NonPrimit ") + "Tier " + t + ": size=" + size + ", " + "time=" + seconds + ", pos/sec=" + size/seconds);
			logOutput.flush();
			
			//now, update its parents
			for (int a=0;a<ttn.parents.size();a++)
			{
				TierTreeNode ttnParent = ttn.parents.get(a); 
				ttnParent.deps.remove(ttn);
				if (ttnParent.deps.size() == 0)
				{
					//sweet, that was the last dependencie. Dump in freeList.
					freeTiers.add(ttnParent);
				}
			}
			//notify listeners
			notifyListenersFinished(t, false, seconds);
		}
		

		//now, try to spawn some new threads
		spawnNewThreads();

	}
	
	private int activeCoreCount = 0;
	private long totalEff;
	private long perfectEff;
	private long lastTime=0;
	
	synchronized void spawnNewThreads()
	{
		//update stats. Multiply the time times number of threads we recored
		//were active.
		if (lastTime != 0)
		{
			long time = System.currentTimeMillis() - lastTime;
			long addedEff = time * activeCoreCount;
			long perfAE = time * ttm.threads;
			totalEff += addedEff;
			perfectEff += perfAE;
		}
		
		
		//go through the goodList for a tier that can be solved. 
		//Don't spawn more than the number of threads avalibale.
		
		for (;ttm.hasMoreThreads() && freeTiers.size() > 0;)
		{
			TierTreeNode ttn = freeTiers.poll();
			//put in being solved list.
			beingSolved.put(ttn.tierNum, ttn);
			if (!ttm.solveTier(ttn.tierNum))
			{
				//something went wrong
				System.err.println("Failed to spawn tier thread " + ttn.tierNum);
				//add back into the queue
				freeTiers.add(ttn);
				return;
			}
			System.out.println("Spawned tier " + ttn.tierNum + " - " + ttn);
			notifyListenersStartSolve(ttn.tierNum);
		}
		
		//restart stats
		lastTime = System.currentTimeMillis();
		activeCoreCount = ttm.threads - ttm.availableThreads();
		
		System.out.println("***Idle=" + ttm.availableThreads() + "/"+ttm.threads+", Tiers ready="+freeTiers.size()+"/"+tierMap.size()+", " +
				"Core status: " + beingSolved.keySet() + ", Efficiency = " + (double)totalEff/perfectEff*cores);
		if (ttm.availableThreads() < minCoresIdle)
		{
			minCoresIdle = ttm.availableThreads();
		}
		
		
		//check if we're all done!
		if (tierMap.size() == 0 || (freeTiers.size() == 0 && ttm.availableThreads() == cores))
		{
			if (tierMap.size()== 0)
			{
				System.out.println("All done!");
			}
			else
			{
				System.out.println("Not all done... We had errors.");
			}
			System.out.println("Min cores idle - " + minCoresIdle);
			System.out.println("Core usage efficiency - " + (double)totalEff/perfectEff*cores);
			System.out.println("Bad tiers: " + badTiers.keySet());
			ttm.shutdown();
			logOutput.close();
		}
	}
	
	private void notifyListenersSetup()
	{
		for (int a=0;a<listeners.length;a++)
		{
			listeners[a].setup(fullMap, tierMap);
		}
	}
	
	private void notifyListenersStartSolve(long tier)
	{
		for (int a=0;a<listeners.length;a++)
			listeners[a].tierStartSolve(tier);
	}
	
	private void notifyListenersFinished(long tier, boolean bad, double seconds)
	{
		for (int a=0;a<listeners.length;a++)
			listeners[a].tierFinishedSolve(tier, bad, seconds);
	}
}



