package edu.berkeley.gamesman.server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import edu.berkeley.gamesman.server.db.DbModule;

public class TestDb {

	//convert long to byte[]
	public static void unmakeLong(long a, byte[] p_src)	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */		
		// assert(p_src.length == 8); //exception
		//note: using jacked db format for longs!
		//ack, figure this out!
		for (byte i=7;i>=0;i--){ // compiler better unroll this
			p_src[i]=(byte)(((char)a) & 0xFF);
			a >>= 8;			
		}		
	}


	protected static String readInput() throws IOException
	{
		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
		return reader.readLine().trim();
	}

public static void main(String[] args)
{
	try{
		System.out.println("Absolute file path:");
		String dir = readInput();
		System.out.println("Game:");
		String gamename = readInput();
		System.out.println("Game option:");
		String gameop = readInput();				
    	String fname = "m" + gamename + "_" + gameop + "_memdb.dat.gz";		
		DbModule db = new DbModule();
		//this only tests preloads
		String [] arg = new String[3];
		arg[0] = dir;
		arg[1] = Integer.toString(1);
		arg[2] = fname;
		db.initialize(arg);
		TestModuleRequest tm;
		TestModuleResponse tres;
		String [] headername = {"hash_length","game_name","game_option"};
		String [] headervalues = {"1",gamename,gameop};
		while (true){ //loop forever
			System.out.println("Give me an input:");
			String hash = readInput();
			long h = Long.parseLong(hash); //hash for long
			byte[] lb = new byte[8];
			unmakeLong(h,lb);			
			tm = new TestModuleRequest(lb,headername,headervalues);
			tres = new TestModuleResponse(2); //only 2 output byte
			//tm.
			db.handleRequest(tm, tres);
			System.out.println("length was " + tres.getHeadersWritten().get("hash_lengh"));
			lb = tres.getOutputWritten();
			short n = DbModule.makeShort(lb);
			System.out.println("value was " + n);			
			
		}
		
	}
	catch (Exception e){
		e.printStackTrace();
	}
}


}