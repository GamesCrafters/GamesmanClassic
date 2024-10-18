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
	
	public synchronized int solveTier(long t)
	{
		if (down)
		{
			System.out.println("This is shut down.");
			return -1;
		}
		
		//See if there is an avalibale thread
		TierThread tt = getAThread();
		if (tt == null)
		{
			return -1; //Coudln't find an avaible thread.
		}
		//start this job up on this thread
		boolean ret = tt.newJob(t);
		if (ret)
		{
			return tt.threadID;
		}
		return -1;
		
	}
	
	private synchronized TierThread getAThread()
	{
		for (int a=0;a<threadArr.length;a++)
		{
			if (threadArr[a].running == false)
				return threadArr[a];
		}
		return null;
	}
	
	
	public synchronized int availableThreads()
	{
		int t = 0;
		for (int a=0;a<threadArr.length;a++)
		{
			if (threadArr[a].running == false && threadArr[a].shutdown == false)
				t++;
		}
		return t;
	}
	
	public boolean hasMoreThreads()
	{
		return getAThread() != null;
	}
	
	synchronized void shutdown()
	{
		down = true;
		for (int a=0;a<threadArr.length;a++)
		{
			threadArr[a].shutdownThread();
		}
	}

	public synchronized void removeFromThreadPool(TierThread thread)
	{
		for (int a=0;a<threadArr.length;a++)
		{
			if (threadArr[a] == thread)
			{
				threads--;
				TierThread[] newArr = new TierThread[threads];
				System.arraycopy(threadArr, 0, newArr, 0, a);
				System.arraycopy(threadArr, a+1, newArr, a, threads-a);
				threadArr = newArr;
				return;
			}
		}
	}

	public boolean killReadyThread()
	{
		TierThread tt = getAThread();
		if (tt == null)
			return false;
		tt.shutdownThread();
		removeFromThreadPool(tt);
		System.out.println("Shutdown thread " + tt.threadID);
		return true;
	}
	
	public synchronized void spawnNewThread()
	{
		TierThread tt = new TierThread(this, gamePath, game, option);
		threads++;
		TierThread[] newArr = new TierThread[threads];
		System.arraycopy(threadArr, 0, newArr, 0, threads-1);
		newArr[threads-1] = tt;
		threadArr = newArr;
		
		
	}


}
