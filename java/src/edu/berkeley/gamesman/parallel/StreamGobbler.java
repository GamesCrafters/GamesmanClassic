package edu.berkeley.gamesman.parallel;

import java.io.*;

public class StreamGobbler implements Runnable
{
	String name;
	InputStream is;
	Thread thread;
	StringBuffer sb = new StringBuffer();
	

	public StreamGobbler(String name, InputStream is)
	{
		this.name = name;
		this.is = is;
	}

	public void start()
	{
		thread = new Thread(this);
		thread.start();
	}

	public void run()
	{
		try
		{
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader br = new BufferedReader(isr);

			while (true)
			{
				String s = br.readLine();
				sb.append(s + "\n");
				if (s == null)
					break;
			}

			is.close();

		}
		catch (Exception ex)
		{
			System.out.println("Problem reading stream " + name + "... :" + ex);
			ex.printStackTrace();
		}
	}
	
	public void dumpToFile(File f) throws FileNotFoundException
	{
		PrintWriter pw = new PrintWriter(f);
		pw.println(sb.toString());
		pw.flush();
		pw.close();
	}
}
