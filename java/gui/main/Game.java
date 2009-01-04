package main;

import org.xml.sax.*;
import org.xml.sax.helpers.*;
import java.io.IOException;

import java.util.HashMap;
import java.util.Vector;
import java.awt.Color;

import game.*;

public class Game {
    public static String Name = null;
    public static String DBName = null;
    public static boolean UsingGenericHash = true;
    public static GameInterface gameInterface = null;

    public static void main( String[] args ) throws Exception {
	try {
	    if( args.length != 1 ) {
		System.out.println( "Please supply an XML file to load..." );
		System.exit(1);
	    }

	    ReadXML( args[0] );

	    gameInterface = new CInterface();

	    //gameInterface.InitializeAll();
	    gameInterface.Initialize();
	    
	    Board b = new Board();
	    b.getPosition().print();

	    Position p = b.getPosition();
	    /*System.out.println( p.getLocationCoords( 0 ) );
	    System.out.println( p.getLocationCoords( 2 ) );
	    System.out.println( p.getLocationCoords( 3 ) );*/

	    GameDisplay disp = new GameDisplay(b,500,500);

	    disp.draw();
	    
	}
	catch( Exception e ) {
	    System.out.println( "Exception :(" );
	    System.out.println( e.getMessage() );
	    System.out.println( e.getStackTrace() );
	}
    }

    public static void ReadXML( String filename ) {
	try {
	    XMLReader reader = XMLReaderFactory.createXMLReader();
	    reader.setContentHandler( new GameXMLHandler() );

	    reader.parse( new InputSource( filename ) );
	}
	catch( SAXException e ) {
	    System.out.println( "Invalid XML" );
	    System.out.println( e.getMessage() );
	}
	catch( IOException e ) {
	    System.out.println( "Error opening file" );
	    System.out.println( e.getMessage() );
	}
    }
}
