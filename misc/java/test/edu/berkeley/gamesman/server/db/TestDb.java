
package edu.berkeley.gamesman.server.db;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import edu.berkeley.gamesman.server.TestModuleRequest;
import edu.berkeley.gamesman.server.TestModuleResponse;
import edu.berkeley.gamesman.server.db.DbModule;

public class TestDb {

	//convert long to byte[]
	public static void unmakeLong(long a, byte[] p_src)	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */		
		// assert(p_src.length == 8); //exception
		
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
		
		//Use below 2 lines if preload

//		arg[1] = Integer.toString(1);
//		arg[2] = fname;
	
		//use below line if not preload
		arg[1] = Integer.toString(0);
		
		
		db.initialize(arg);
		TestModuleRequest tm;
		TestModuleResponse tres;
		String [] headername = {"length","game","variant"};
		String [] headervalues = {"1",gamename,gameop};
		while (true){ //loop forever
			byte[] req = new byte[8*20]; //allow 20 requests
			int rc=0;
			while (true){ //inner loop forever
				System.out.println("Give me an input (hit q when done):");
				String hash = readInput();
				if (hash.equalsIgnoreCase("q"))
					break;
				long h = Long.parseLong(hash); //hash for long
				byte[] lb = new byte[8];
				unmakeLong(h,lb);
				for (int b = 0; b < 8; b++){
					req[rc+b] = lb[b];
				}
				rc+=8;					
			}
			if (rc==0) //this is safe exit
				return;
			headervalues[0] = Integer.toString(rc/8); //set length
			tm = new TestModuleRequest(req,headername,headervalues);
			tres = new TestModuleResponse(rc/4); //as much output as input
			//tm.
			db.handleRequest(tm, tres);
			int f = Integer.parseInt((String)(tres.getHeadersWritten().get("length"))); 
			System.out.println("length was " + f);
			byte[] lb = tres.getOutputWritten();			
			byte[] qv = new byte[2]; //I hate java..
			f*=2; //change bound
			for (int q = 0;q<f;q+=2){	
				qv[0] = lb[q];
				qv[1] = lb[q+1];
				short n = DbModule.makeShort(qv);
				System.out.println("value #" + (q/2) + " was " + n);
			}
			
		}
		
	}
	catch (Exception e){
		e.printStackTrace();
	}
}


}