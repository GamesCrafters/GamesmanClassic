// bugs: gzip reading is broken at 2134
// this gives 772; should be 1026
// caused by byte alignment being broken
// broken at least at 1600
// help: I do not understand why this is happening!

package edu.berkeley.gamesman.server.db;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;
import edu.berkeley.gamesman.server.RequestType;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.zip.GZIPInputStream;

public class DbModule implements IModule
{
	/** Header names */
	// test
	public static final String HN_LENGTH = "Length";
	public static final String HN_GAME_NAME = "GameName";
	public static final String HN_GAME_VARIANT = "GameVariant";

	private HashMap loadedData; // hash of loaded data (maps to shorts!)
	private String baseDir; // base directory of database files
	private short FILEVER = 1; // db file version

	// private HashMap versions; //game versions table

	/* Because java sucks with data type conversion */
	/* Assumes p_src is Big Endian (network byte order) */
	/* This is slow :( */
	public static short makeShort(byte[] p_src)
	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */
		short ret = 0;
		for (byte i = 0; i < 2; i++)
		{ // compiler better unroll this
			ret <<= 8;
			ret |= (((char) p_src[i]) & 0xFF);

		}
		return ret;
	}

	/* generate int (which numpositions is stored as() */
	/* Note: using broken db format! */
	public static int makeInt(byte[] p_src)
	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */
		int ret = 0;
		for (byte i = 0; i < 4; i++)
		{ // compiler better unroll this
			ret <<= 8;
			ret = ret | (((char) p_src[i]) & 0xFF);
			// System.out.println("p_src " + i + " = " + p_src[i] + "ret is " + ret);
		}
		return ret;
	}

	/* positions passed as 64 bit */
	public static long makeLong(byte[] p_src)
	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */
		long ret = 0;
		// assert(p_src.length == 8); //exception
		for (byte i = 0; i < 8; i++)
		{ // compiler better unroll this
			ret = ret << 8;
			ret = ret | (((char) p_src[i]) & 0xFF);
			// System.out.println("p_src " + i + " = " + p_src[i] + "ret is " + ret);
		}
		return ret;
	}

	/**
	 * The configArgs String[] contains database file names in the working directory that
	 * should be pre-loaded.
	 */
	public void initialize(String workingDir, String[] configArgs)
			throws ModuleInitializationException
	{
		File f;
		FileInputStream fis;
		GZIPInputStream gz;
		baseDir = workingDir + "db/";
		int len = configArgs.length;
		short[] buf = null; // injected memory buffer
		byte[] sbuf = new byte[2]; // memory buffer for reading in short
		short s;
		byte[] lbuf = new byte[8]; // memory buffer for reading in long
		byte[] ibuf = new byte[4]; // memory buffer for reading in int
		int num_pos = 0; // this really should be long; let's hope we don't have
		// 16 gb databases anytime soon
		// if (pl.length!=0)
		// always load
		loadedData = new HashMap();

		for (int i = 2; i < len; i++)
		{
			try
			{
				f = new File(baseDir + configArgs[i]);
				if (!f.exists())
				{
					System.out.println("File does not exist: " + baseDir + configArgs[i]);
					continue;
				}

				fis = new FileInputStream(f);
				gz = new GZIPInputStream(fis);
			}
			catch (Exception e)
			{
				// just log error and continue
				continue;
			}

			try
			{
				// available can be used to read stuff
				readgz(gz, sbuf, 2);

				s = makeShort(sbuf);

				if (s != FILEVER)
				{
					// error and continue;
					continue;
				}

				readgz(gz, ibuf, ibuf.length);

				// FIXME: I need a version number here!!
				// actually we are ignoring this for now
				// num_pos = (int)makeLong(lbuf); // this is the size (why this is long i
				// don't know)
				num_pos = makeInt(ibuf);

				// only supports reading 32 bit files - who cares
				// System.out.println("num pos is " + num_pos);
				gz.skip(4); // 4 bytes of 0's in broken format
				buf = new short[num_pos];
				// now load in the entire system
				for (int j = 0; j < num_pos; j++)
				{
					readgz(gz, sbuf, 2);

					s = makeShort(sbuf); // this is the position
					// if (j>=13400&&j<13410)
					/*
					 * if (j>=2130&&j<2140){
					 * 
					 * System.out.println(j + " pos is " + s); System.out.println("high
					 * byte is " + sbuf[0] + "low is " + sbuf[1]); }
					 */
					buf[j] = s;
				}
			}

			catch (Exception e)
			{
				System.out.println("Initialization warning: Could not load " + baseDir
						+ configArgs[i]);
				continue;
			}
			// check if can save!
			loadedData.put(configArgs[i], buf); // associate file with datastream
		}
	}

	public DbModule() throws ModuleException
	{
		// this doesn't do anything
	}

	public boolean typeSupported(String requestTypeName)
	{
		return (requestTypeName.equalsIgnoreCase(RequestType.GET_VALUE_OF_POSITIONS) || 
				requestTypeName.equalsIgnoreCase(RequestType.INIT_DATABASE));
	}

	// helper function
	// Throws module exception if fails
	void readgz(GZIPInputStream gz, byte[] data, int len) throws ModuleException
	{
		try
		{
			int num_read;
			int cnt = 0;
			while (len > 0)
			{
				num_read = gz.read(data, cnt, len);
				if (num_read == -1)
					throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
							ErrorCode.Msg.INTERNAL_DB_ERROR);
				len -= num_read;
				cnt += num_read;
			}

		}
		catch (IOException E)
		{
			throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR, ErrorCode.Msg.INTERNAL_DB_ERROR);
		}
	}

	/**
	 * Handles the request types for this IModule.
	 */
	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		if (req.getType().equalsIgnoreCase(RequestType.GET_VALUE_OF_POSITIONS))
			handleGetValueOfPositionsRequest(req, res);
		else if (req.getType().equalsIgnoreCase(RequestType.INIT_DATABASE))
			handleInitDatabaseRequest(req, res);
		else
			throw new ModuleException(ErrorCode.BAD_REQUEST_TYPE, ErrorCode.Msg.BAD_REQUEST_TYPE);
	}	
	
	/**
	 * Handles INIT_DATABASE requests. Basically makes sure the database exists and can be read.
	 * 
	 * @param req request to read data from
	 * @param res response to write data to
	 * @throws ModuleException
	 */
	protected void handleInitDatabaseRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		String gamename = req.getHeader(HN_GAME_NAME);
		String gameop = req.getHeader(HN_GAME_VARIANT);
		if (gamename == null)
			throw new ModuleException(ErrorCode.MISSING_GAME, ErrorCode.Msg.MISSING_GAME);
		if (gameop == null)
			throw new ModuleException(ErrorCode.MISSING_VARIANT, ErrorCode.Msg.MISSING_VARIANT);
		
		// Generate filename
		String fname = genFileName(gamename, gameop);

		if (!loadedData.containsKey(fname))
		{
			GZIPInputStream gz = null;
			try
			{
				try
				{
					// Just see if we can open it.
					gz = openDb(gamename, gameop);
				}
				finally
				{
					if (gz != null)
						gz.close();
				}
			}
			catch (ModuleException ex)
			{
				throw ex;
			}
			catch (IOException ex)
			{
				throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
						ErrorCode.Msg.INTERNAL_DB_ERROR, ex);
			}
		}		
	}

	/**
	 * Handles GET_VALUE_OF_POSITIONS requests. Opens the db file and reads the values of the positions out
	 * of the file in order and returns them as data to the response.
	 * 
	 * @param req request to read data from
	 * @param res response to write data to
	 * @throws ModuleException
	 */
	protected void handleGetValueOfPositionsRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		String hlen = req.getHeader(HN_LENGTH);
		String gamename = req.getHeader(HN_GAME_NAME);
		String gameop = req.getHeader(HN_GAME_VARIANT);
		if (hlen == null)
			throw new ModuleException(ErrorCode.MISSING_LENGTH, ErrorCode.Msg.MISSING_LENGTH);
		if (gamename == null)
			throw new ModuleException(ErrorCode.MISSING_GAME, ErrorCode.Msg.MISSING_GAME);
		if (gameop == null)
			throw new ModuleException(ErrorCode.MISSING_VARIANT, ErrorCode.Msg.MISSING_VARIANT);

		short len = Short.decode(hlen).shortValue(); // number of batched requests
		// note: using short as we don't want to kill server w/ endless requests!
		if (len <= 0)
		{ // don't allow even 0
			// malformed request error
			throw new ModuleException(ErrorCode.INVALID_LENGTH, ErrorCode.Msg.INVALID_LENGTH);
		}

		// Generate filename
		String fname = genFileName(gamename, gameop);

		if (loadedData.containsKey(fname))
		{
			// Read it out of the cache
			short[] htable = (short[]) loadedData.get(fname);
			try
			{
				DataInputStream w = new DataInputStream(req.getInputStream());
				for (int i = 0; i < len; i++)
				{
					long l = w.readLong(); // 64 bit, though we can only use 32
					// System.out.println("accessing " + (int)l);
					// int l = (int)makeLong(lbuf); //only support 32 bit
					if (l >= htable.length)
						throw new ModuleException(ErrorCode.INVALID_POSITION,
								ErrorCode.Msg.INVALID_POSITION);
					short s = htable[(int) l];
					// write in Big Endian to stream
					// System.out.println("Got " + s);
					res.getOutputStream().write((s >> 8) & 0xFF);
					res.getOutputStream().write(s & 0xFF);
				}
			}
			catch (IOException e)
			{
				// throw module exception
				throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
						ErrorCode.Msg.INTERNAL_DB_ERROR);
			}
		}
		else
		{ 
			// Open the db and read it
			GZIPInputStream gz = null;
			try
			{
				try
				{
					// Open the db. Note: first 2 bytes have been read and file version has been checked.
					gz = openDb(gamename, gameop);
					
					// Get legal number of positions. Next 4 bytes represent the number of
					// positions.
					byte[] ibuf = new byte[4];
					readgz(gz, ibuf, ibuf.length);
					int num_pos = makeInt(ibuf);
	
					// Next 4 bytes are bad (I guess??)
					gz.skip(4); // skip bad data
	
					// Build a SortedMap of positions for which we'd like to get a value.
					SortedMap<Long, Integer> insort = new TreeMap<Long, Integer>();
					byte[] lbuf = new byte[8];
					for (int i = 0; i < len; i++)
					{
						// Must be aligned
						if (req.getInputStream().read(lbuf, 0, 8) == -1)
							throw new ModuleException(ErrorCode.INVALID_LENGTH,
									ErrorCode.Msg.INVALID_LENGTH);
	
						// Assign
						long l = makeLong(lbuf); // only support 32 bit
						insort.put(new Long(l), new Integer(i));
	
						// Check that the greatest position isn't greater than the max number
						// of positions in the db
						if (l >= num_pos)
							throw new ModuleException(ErrorCode.INVALID_POSITION,
									ErrorCode.Msg.INVALID_POSITION);
					}
	
					// Now iterate thru the positions in ascending order to grab correct data!
					long lastpos = 0;
					short[] outbuf = new short[len]; // again: only 32 bit (long limits)
					Iterator<Map.Entry<Long, Integer>> itr = insort.entrySet().iterator();
					while (itr.hasNext())
					{
						// Get the values
						Map.Entry<Long, Integer> entry = itr.next();
						long l = entry.getKey().longValue();
						int ndx = entry.getValue().intValue();
	
						// Skip over entries
						gz.skip(2 * l - lastpos); // short array
						lastpos = 2 * l + 2;
	
						// Read the 2 bytes of data representing the position's value and 
						// use the original index value to store.
						byte[] sbuf = new byte[2];
						readgz(gz, sbuf, 2);
						outbuf[ndx] = makeShort(sbuf);
					}
	
					// Dump out to stream (correctly sorted!)
					for (int i = 0; i < outbuf.length; i++)
					{
						// write in Big Endian to stream
						res.getOutputStream().write(outbuf[i] >> 8);
						res.getOutputStream().write(outbuf[i] & 0xFF);
					}
				}
				finally
				{
					if (gz != null)
						gz.close();
				}
			}
			catch (IOException E)
			{
				throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
						ErrorCode.Msg.INTERNAL_DB_ERROR);
			}
		}
		
		// Add header to response
		res.setHeader(HN_LENGTH, hlen); // client gets back what it sent
	}

	/**
	 * Generates the db file name from the gamename and game option.
	 * 
	 * @param gamename game name
	 * @param gameop game option
	 * @return db file name
	 */
	protected String genFileName(String gamename, String gameop)
	{
		return "m" + gamename + "_" + gameop + "_memdb.dat.gz";
	}
	
	/**
	 * Opens the specified database and reads the file version from it. Returns a GZIPInputStream
	 * representing the database. Because the file version has been read, the GZIPInputStream is 
	 * already 2 bytes into the data.
	 * 
	 * @param gamename game name
	 * @param gameop game option number
	 * @return GZIPInputStream representing the db file, 2 bytes already read
	 * @throws ModuleException
	 * @throws IOException
	 */
	protected GZIPInputStream openDb(String gamename, String gameop) throws ModuleException, IOException
	{
		// Generate the db file name.
		String fname = "m" + gamename + "_" + gameop + "_memdb.dat.gz";
		File f = new File(baseDir + fname);
		if (!f.exists())
		{
			throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
					ErrorCode.Msg.NO_DB + ": Database for game: " + gamename + " with option: " + gameop + " not found");
		}
		else if (!f.canRead())
		{
			throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
					ErrorCode.Msg.INTERNAL_DB_ERROR + ": Cannot read database");					
		}
		else
		{
			// Try to open it
			FileInputStream fis = new FileInputStream(f);
			GZIPInputStream gz = new GZIPInputStream(fis);

			// Check file version. First 2 bytes are the db version number.
			byte[] sbuf = new byte[2];
			readgz(gz, sbuf, sbuf.length);
			if (makeShort(sbuf) != FILEVER)
			{
				throw new ModuleException(ErrorCode.INTERNAL_DB_ERROR,
						ErrorCode.Msg.INTERNAL_DB_ERROR + ": Invalid database version found");
			}
			
			return gz;
		}
	}
	
}
