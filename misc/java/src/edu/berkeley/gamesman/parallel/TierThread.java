//TierThread.java

//Assuming # of threads = # of cores on the solving computer	
//Threads should not have sleep time.

package edu.berkeley.gamesman.parallel;
import java.io.File;


class TierThread extends Thread
{
	
	private long TierID;		//Hash value of a particular Tier
					//Essentially identifies a specific Tier
	private String gamePath;
	private String gameName;
	TierThreadManager ttm;

	//Constructor of a TierThread
	volatile boolean running = false;
	public volatile boolean shutdown = false;
	private int option;
	private static int TID_COUNT = 0;
	public int threadID = TID_COUNT++;
	
	
	public TierThread (TierThreadManager ttm, String path, String name, int option) {
		gamePath = path;
		gameName = name;
		this.ttm = ttm;
		this.option = option;
		if (gamePath.endsWith("/")==false)
			gamePath += "/";
	}

	//Get TierID
	public long getID(){
		return this.TierID;
	}

	//Execution of a thread is called by run()
	public void run() 
	{
		try
		{
			while (!shutdown)
			{
				int retCode = -1;
				long time = System.currentTimeMillis();
				try 
				{
					if (TierTreeManager.FAKE)
					{
						//sleep for like 100 ms
						Thread.sleep((long) (1000 + Math.random()*1000));
						retCode = 0;
					}
					else
					{
						ProcessBuilder pb;
						if (option == -1)
							pb = new ProcessBuilder(this.gamePath + this.gameName, "--analyze", "--onlytier", "" + TierID);
						else
							pb = new ProcessBuilder(this.gamePath + this.gameName, "--analyze", "--onlytier", "" + TierID, "--option", "" + option);
						pb.directory(new File(gamePath));
						Process p = pb.start();
						
						//Gobble up both streams
						StreamGobbler s1 = new StreamGobbler (TierID + "stdout", p.getInputStream ());
						StreamGobbler s2 = new StreamGobbler (TierID + "stderr", p.getErrorStream ());
						s1.start ();
						s2.start ();
						
						//wait for the code
						retCode = p.waitFor();
						//if the recode is't 0, dump the streams.
						if (retCode != 0)
						{
							System.out.println("return code for tier " + TierID+" wasn't zero, dumping output streams.");
							s1.dumpToFile(new File(TierID + "_stdout.txt"));
							s2.dumpToFile(new File(TierID + "_stderr.txt"));
						}
					}
					
				} catch (Exception ex) {
					ex.printStackTrace();
				}
				time = System.currentTimeMillis() - time;
				//We are DONE. Call done method
				System.out.println("Tier " + TierID + " is done, calling done.");
				done(TierID, retCode, time/1000.0);
			}
		}
		catch (InterruptedException e)
		{
			shutdown = true;
			e.printStackTrace();
			System.out.println("A thread has died unexpectedly.");
		}
	}

	
	/*
So what's happening:

A thread finishes. Goes into done, goes into finished, and goes into spawn new threads

Meanwhile, another thread finishes. It goes into done (SYNCHED), and tries to go 
into finished but can't since it's locked by the first thread

The first thread, in spawnNewTiers, tries to give itself a new job. OK. Tries to 
give the other thread a new thread. DEADLOCK.

We can solve this problem by having another seperate thread run the finisher. This way
done always enters into the wait phase since newjob can't enter and make running true

Then, if the finisher was waiting to enter newjob, it does, and notifies.
The thread wakes up and everything is ok.




new solution - unsynch (part of) done. That way, it doesn't block if it's waiting on 
finished and finish calls newjob
	 */
	
	public synchronized boolean newJob(long tier)
	{
		//we're running again.
		running = true;
		TierID = tier;
		if (isAlive())
		{
			//notify if necessary
			notify();
		}
		else if (!shutdown)
		{
			//inital start of thread
			start();
		}
		else
		{
			return false;	//can't do antyoing
		}
		return true;
	}
	public void done(long tid, int retCode, double seconds) throws InterruptedException
	{
		//No threads can enter the newjob right now, so we're safe
		//Now, call finished 
		//Finished atomically sets running to false for us. Why didn't i think of that before?
		ttm.ttm.finished(tid, retCode, seconds, this); 


		
		//This is impossible since we are sycned, and so we have 'complete'
		//control over running. And of course, the finished thread can't
		//come in early and notify because they are both sycned.
		synchronized (this)
		{
			if (shutdown)
			{
				return; //shutdown, don't wait
			}
			
			
			if (running)
			{
				//cool, we just jump right back into the game;
			}
			else 
			{
				//wait for someone to wake us up (The notify in newJob())
				wait();
			}
		}
	}
	
	public synchronized void shutdownThread()
	{
		shutdown = true;
		notify();
	}

}
