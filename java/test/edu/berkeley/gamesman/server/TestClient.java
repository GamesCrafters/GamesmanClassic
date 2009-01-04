package edu.berkeley.gamesman.server;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;

/**
 * Simple test client that will send header values and character/binary
 * content to a specified url and print out the value of the response
 * (headers and content).
 * 
 * @author Matt Jacobsen
 *
 */
public class TestClient
{
	/** URL to open */
	protected URL url;
	
	/**
	 * Default constructor. Takes the url to open.
	 * 
	 * @param url URL to open a connection to
	 */
	public TestClient(URL url)
	{
		this.url = url;
	}
	
	/**
	 * Main entry point. Requires the full url to the Gamesman servlet.
	 * 
	 * @param args String array of all the command line arguments
	 */
	public static void main(String[] args)
	{
		try
		{
			if (args.length < 1)
			{
				System.out.println("required arguments: <url to gamesman servlet>");
			}
			else
			{
				TestClient client = new TestClient(new URL(args[0]));
				if (!client.url.getProtocol().toLowerCase().startsWith("http"))
					throw new MalformedURLException("Only http/https urls are supported.");
				client.newRequest();
			}
		}
		catch (Exception ex)
		{
			ex.printStackTrace();
		}
	}

	/**
	 * Starts a new request/response transaction with the current url. This
	 * runs in a loop until the user choose to quit.
	 * 
	 * @throws IOException
	 */
	protected void newRequest() throws IOException
	{
		while (true)
		{
			HttpURLConnection conn = (HttpURLConnection)url.openConnection();
			conn.setRequestMethod("POST");
			conn.setDoOutput(true);
			requestLoop(conn);
		}
	}
	
	/**
	 * Handles setting headers and body content for the current request. Loops
	 * until the user selects send, or body content is written. Writing body
	 * content requires that we send the request (i.e. open the connection)
	 * and *then* write data. So if headers need to be set, do it before adding
	 * body content.
	 * 
	 * @param conn HttpURLConnection to open
	 * @throws IOException
	 */
	protected void requestLoop(HttpURLConnection conn) throws IOException
	{
		outer:
		while (true)
		{
			System.out.println("1) Set type header.");
			System.out.println("2) Add header.");
			System.out.println("3) Add string body and send.");
			System.out.println("4) Add binary body of long hash values and send.");
			System.out.println("5) Add binary body and send.");
			System.out.println("6) Just send.");
			System.out.println("7) Quit");
			
			OutputStream os = null;
			String str = readInput();
			int selection = -1;
			try
			{
				selection = Integer.parseInt(str);
			}
			catch (NumberFormatException ex)
			{
				if (str.equalsIgnoreCase("q") || str.equalsIgnoreCase("quit") || 
						str.equalsIgnoreCase("exit") || str.equalsIgnoreCase("bye"))
					System.exit(0);
				System.out.println("Unrecognized menu selection. Try again.");
				continue;
			}
			switch (selection)
			{
			case 1:
				System.out.println("  Enter type: ");
				conn.setRequestProperty("TYPE", readInput());
				break;
			case 2:
				System.out.println("  Enter header name: ");
				String name = readInput();
				System.out.println("  Enter header value: ");		
				conn.setRequestProperty(name, readInput());
				break;
			case 3:
				System.out.println("  Enter value: ");				
				readRequest(conn);
				conn.connect();
				os = conn.getOutputStream();
				os.write(readInput().getBytes());
				os.close();
				readResponse(conn, -1);
				break outer;
			case 4:
				System.out.println("  Enter sequence of long(s): ex. 34L 127L 123445323L ");
				String longs = readInput();
				StringTokenizer tokenizer = new StringTokenizer(longs, " Ll", false);
				readRequest(conn);
				conn.connect();
				os = conn.getOutputStream();
				DataOutputStream dos = new DataOutputStream(os);
				int count = tokenizer.countTokens();
				while (tokenizer.hasMoreTokens())
					dos.writeLong(Long.parseLong(tokenizer.nextToken()));
				dos.close();				
				readResponse(conn, count);
				break outer;
			case 5:
				System.out.println("  Enter absolute path to file: ");
				FileInputStream fin = new FileInputStream(readInput());
				readRequest(conn);
				conn.connect();
				os = conn.getOutputStream();
				int read = -1;
				byte[] buf = new byte[512];
				while ((read = fin.read(buf, 0, buf.length)) != -1)
					os.write(buf, 0, read);
				fin.close();
				os.close();				
				readResponse(conn, -1);
				break outer;
			case 6:
				readRequest(conn);
				conn.connect();
				readResponse(conn, -1);
				break outer;
			case 7:
				System.exit(0);
				break;
			default:
				System.out.println("Unrecognized menu selection. Try again.");
				break;
			}
		}
	}
		
	/**
	 * Reads input from System.in (the keyboard). Returns the value of the typed
	 * line.
	 * 
	 * @return String value of the typed line.
	 * @throws IOException
	 */
	protected String readInput() throws IOException
	{
		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
		return reader.readLine().trim();
	}
	
	/**
	 * Reads the HTTP request headers prints them to System.out.
	 * 
	 * @param conn HttpURLConnection to send and read from.
	 * @throws IOException
	 */
	protected void readRequest(HttpURLConnection conn) throws IOException
	{
		System.out.println("-------------->>");
		Map headers = conn.getRequestProperties();
		Iterator itr = headers.keySet().iterator();
		while (itr.hasNext())
		{
			String key = (String)itr.next();
			System.out.print(key + " = ");
			Iterator itr2 = ((List)headers.get(key)).iterator();
			while (itr2.hasNext())
				System.out.println(itr2.next() + ", ");
		}
		System.out.println("-------------->>");
	}
	
	/**
	 * Reads the HTTP response from the server and prints it to System.out.
	 * 
	 * @param conn HttpURLConnection to send and read from.
	 * @param longCount if == -1, then interpret body as char data, otherwise interpret
	 * 			the body as the specified number of longs
	 * @throws IOException
	 */
	protected void readResponse(HttpURLConnection conn, int longCount) throws IOException
	{
		System.out.println("<<--------------");
		Map headers = conn.getHeaderFields();
		Iterator itr = headers.keySet().iterator();
		while (itr.hasNext())
		{
			String key = (String)itr.next();
			System.out.print(key + " = ");
			Iterator itr2 = ((List)headers.get(key)).iterator();
			while (itr2.hasNext())
				System.out.println(itr2.next() + ", ");
		}
		InputStream is = conn.getInputStream();
		if (longCount != -1)
		{
			DataInputStream dis = new DataInputStream(is);
			for (int i=0; i<longCount; i++)
				System.out.println(dis.readShort());
			is = dis;
		}
		else
		{
			int read = -1;
			byte[] buf = new byte[512];
			while ((read = is.read(buf, 0, buf.length)) != -1)
				System.out.write(buf, 0, read);
		}
		is.close();
		conn.disconnect();
		System.out.println("<<--------------");
	}
}
