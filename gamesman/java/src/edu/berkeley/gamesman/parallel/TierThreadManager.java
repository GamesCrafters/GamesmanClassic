package edu.berkeley.gamesman.parallel;

public class TierThreadManager
{
	TierTreeManager ttm;
	int threads;
	int option;
	
	String gamePath, game;
	volatile boolean down = false;
	
	TierThread[] threadArr;
	
	public TierThreadManager(int numThreads, TierTreeManager ttm,String gamePath, String game, int option)
	{
		this.ttm  = ttm;
		threads = numThreads;
		this.game = game;
		this.gamePath = gamePath;
		this.option = option;
		
		
		//create the threads.
		threadArr = new TierThread[threads];
		for (int a=0;a<threadArr.length;a++)
		{
			threadArr[a] = new TierThread(this, gamePath, game, option);
		}
	}
	
	public boolean solveTier(long t)
	{
		if (down)
		{
			System.out.println("This is shut down.");
			return false;
		}
		
		//See if there is an avalibale thread
		TierThread tt = getAThread();
		if (tt == null)
		{
			return false; //Coudln't find an avaible thread.
		}
		//start this job up on this thread
		return tt.newJob(t);		
	}
	
	private TierThread getAThread()
	{
		for (int a=0;a<threadArr.length;a++)
		{
			if (threadArr[a].running == false)
				return threadArr[a];
		}
		return null;
	}
	
	
	public int availableThreads()
	{
		int t = 0;
		for (int a=0;a<threadArr.length;a++)
		{
			if (threadArr[a].running == false)
				t++;
		}
		return t;
	}
	
	public boolean hasMoreThreads()
	{
		return getAThread() != null;
	}
	
	void shutdown()
	{
		down = true;
		for (int a=0;a<threadArr.length;a++)
		{
			threadArr[a].shutdownThread();
		}
	}

}
