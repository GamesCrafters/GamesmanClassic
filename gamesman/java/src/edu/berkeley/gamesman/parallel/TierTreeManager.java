package edu.berkeley.gamesman.parallel;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Reader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map;
import java.util.PriorityQueue;

import javax.swing.JOptionPane;

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
	PriorityQueue<TierTreeNode> freeTiers = new PriorityQueue<TierTreeNode>();
	Map<Long, TierTreeNode> badTiers = new HashMap<Long, TierTreeNode>();
	Map<Long, TierTreeNode> beingSolved = new HashMap<Long, TierTreeNode>();
	
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
		while ((line = br.readLine()).equals("ENDVTP") == false)
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
				tierMap.put(tier, new TierTreeNode(tier, size));
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
	public synchronized void finished(long t, int retCode, double seconds)
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
		}
		

		//now, try to spawn some new threads
		spawnNewThreads();

	}
	
	private int activeCoreCount = 0;
	private long totalEff;
	private long perfectEff;
	private long lastTime=0;
	
	public synchronized void spawnNewThreads()
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
	
	
}

class TierTreeNode implements Comparable<TierTreeNode>
{
	public boolean wasPrim=true;
	public long tierNum, size;
	public ArrayList<TierTreeNode> deps = new ArrayList<TierTreeNode>();
	public ArrayList<TierTreeNode> parents = new ArrayList<TierTreeNode>();
	public double priority = 0;
	
	
	public TierTreeNode(long tierNum, long size)
	{
		this.tierNum = tierNum;
		this.size = size;
	}
	
	public String toString()
	{
		return "(" + tierNum + " - " + priority + ")";
	}
	
	public boolean equals(Object o)
	{
		return (o instanceof TierTreeNode)?((TierTreeNode)o).tierNum==tierNum:false;
	}
	
	public void propagateDependencies(double amt)
	{
		//add to my own priority
		this.priority += amt;
		
		//split this one's dependencies into the number of children and propagate
		//use amt not priority - becuyase if we don't it messes up big time
		if (deps.size() > 0)
		{
			double eachPri = amt/deps.size();
			for (int a=0;a<deps.size();a++)
			{
				TierTreeNode ttn = deps.get(a);
				ttn.propagateDependencies(eachPri);
			}
		}
	}


	public int compareTo(TierTreeNode ttno)
	{
		//We have to reverse it since priorityqueues use
		//the least element at the top
		if (priority > ttno.priority)
		{
			return -1;
		}
		else if (priority == ttno.priority)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	

	public long marker = -1; //This is for the priority huerestic use only.
	public int calculateHeuristic(int levels, long tierNum)
	{
		int count = 0;
		if (marker != tierNum)
		{
			count = 1;
			//if this is a top tier, make it value of 2
			if (parents.size() == 0)
				count++;
		}
		marker = tierNum;	//don't count this one again.
		
		if (levels != 0)
		{
			//check all the parents
			for (int a=0;a<parents.size();a++)
			{
				count += parents.get(a).calculateHeuristic(levels-1, tierNum);
			}
		}
		return count;
	}
	
}


