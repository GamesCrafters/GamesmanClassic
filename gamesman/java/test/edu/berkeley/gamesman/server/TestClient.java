package edu.berkeley.gamesman.server;

import java.io.BufferedReader;
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
			System.out.println("4) Add binary body and send.");
			System.out.println("5) Just send.");
			System.out.println("6) Quit");
			
			OutputStream os = null;
			switch (Integer.parseInt(readInput()))
			{
			case 1:
				System.out.println("  Enter type: ");
				conn.addRequestProperty("type", readInput());
				break;
			case 2:
				System.out.println("  Enter header name: ");
				String name = readInput();
				System.out.println("  Enter header value: ");		
				conn.addRequestProperty(name, readInput());
				break;
			case 3:
				System.out.println("  Enter value: ");		
				conn.connect();
				os = conn.getOutputStream();
				os.write(readInput().getBytes());
				os.close();
				readResponse(conn);
				break outer;
			case 4:
				System.out.println("  Enter path to file: ");
				FileInputStream fin = new FileInputStream(readInput());
				conn.connect();
				os = conn.getOutputStream();
				int read = -1;
				byte[] buf = new byte[512];
				while ((read = fin.read(buf, 0, buf.length)) != -1)
					os.write(buf, 0, read);
				fin.close();
				os.close();				
				readResponse(conn);
				break outer;
			case 5:
				conn.connect();
				readResponse(conn);
				break outer;
			case 6:
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
	 * Reads the HTTP response from the server and prints it to System.out.
	 * 
	 * @param conn HttpURLConnection to send and read from.
	 * @throws IOException
	 */
	protected void readResponse(HttpURLConnection conn) throws IOException
	{
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
		int read = -1;
		byte[] buf = new byte[512];
		while ((read = is.read(buf, 0, buf.length)) != -1)
			System.out.write(buf, 0, read);
		is.close();
		conn.disconnect();
		System.out.println("");
	}
}
