package main;

import org.xml.sax.Attributes;
import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;
import org.xml.sax.Locator;

import java.util.Stack;
import java.awt.Color;

import game.*;

public class GameXMLHandler extends DefaultHandler {
    Stack<String> tree;
    String currentNode;

    char bin_charID;
    Color bin_pieceColor;
    int bin_pieceShape;
    
    public GameXMLHandler() {
    }


    public void characters( char[] ch, int start, int length ) {
	try {
	    String s = new String( ch, start, length );
	    s = s.trim();

	    if( s.equals( "" ) )
		return;

	    if( tree.contains("INFO" ) ) {
		if( currentNode.equals("NAME") ) {
		    Game.Name  = s;
		    System.out.println( "Game name set to \"" + s + "\"." );
		    return;
		}

		if( currentNode.equals("DBNAME") ) {
		    Game.DBName = s;
		    System.out.println( "Game dbname set to \"" + s + "\"." );
		    return;
		}
	    }

	    if( tree.contains("BOARD") ) {
		if( currentNode.equals( "TYPE" ) ) {
		    //grid, hex, etc
		    return;
		} else if( currentNode.equals( "WIDTH" ) ) {
		    int width = Integer.parseInt( s );
		    Board.setWidth( width );
		    System.out.println( "Board width set to " + width + "." );
		    return;
		} else if( currentNode.equals( "HEIGHT" ) ) {
		    int height = Integer.parseInt(s);
		    Board.setHeight( height );
		    System.out.println( "Board height set to " + height + "." );
		    return;
		} else if( currentNode.equals( "BGCOLOR" ) ) {
		    int color = Integer.parseInt( s.toUpperCase(), 16 );
		    Board.setColor( new Color(color) );
		}
	    }

	    if( currentNode.equals( "HASH" ) ) {

		if( s.toUpperCase().equals( "GENERIC" ) ) {
		    Game.UsingGenericHash = true;
		    System.out.println( "Using generic hash." );
		    return;
		} else {
		    Game.UsingGenericHash = false;
		    System.out.println( "Using custom hash." );
		    return;
		}
	    }

	    if( tree.contains( "PIECES" ) ) {
		if( tree.contains( "PIECE" ) ) {
		    if( currentNode.equals("CHARID") ) {
			if( s.toUpperCase().equals( "SPACE" ) )
			    bin_charID = ' ';
			else
			    bin_charID = s.charAt(0);
		    } else if( currentNode.equals( "SHAPE" ) ) {

			if( s.toUpperCase().equals( "EMPTY" ) )
			    bin_pieceShape = Bin.SHAPE_EMPTY;
			else if ( s.toUpperCase().equals( "X" ) )
			    bin_pieceShape = Bin.SHAPE_X;
			else if ( s.toUpperCase().equals( "O" ) )
			    bin_pieceShape = Bin.SHAPE_O;
			else if ( s.toUpperCase().equals( "CIRCLE" ) )
			    bin_pieceShape = Bin.SHAPE_CIRCLE;
			else if ( s.toUpperCase().equals( "PLUS" ) )
			    bin_pieceShape = Bin.SHAPE_PLUS;
			else if ( s.toUpperCase().equals( "CUSTOM" ) )
			    bin_pieceShape = Bin.SHAPE_CUSTOM;
			else
			    System.out.println( "invalid piece shape" );

		    } else if( currentNode.equals( "COLOR" ) ){
			int hexvalue = Integer.parseInt( s.toUpperCase(), 16 );
			bin_pieceColor = new Color( hexvalue );
		    }
		}
	    }

	    
	}
	catch( NumberFormatException e ) {
	    System.out.println( "Error reading number" );
	    System.out.println( e.getMessage() );
	}
    }

    public void endDocument() {
    }

    public void startDocument() {
	tree = new Stack<String>();
	currentNode = "";
    }

    public void startElement( String uri, String localName, String qName, Attributes attributes ) {
	if( !currentNode.equals( "" ) ) {
	    tree.push( currentNode );
	}

	currentNode = localName.toUpperCase();;
    }

    public void endElement( String uri, String localName, String qName ) {
	if( ! currentNode.equals( localName.toUpperCase() ) ) {
	    System.out.println( "\"" + currentNode + "\" and \"" 
				+ localName.toUpperCase() + "\" don't match..." );
	}

	if( currentNode.equals( "PIECE" ) ) {
	    Bin.addArchetype( new Bin( bin_charID, bin_pieceColor, bin_pieceShape ) );
	}


	if( tree.empty() )
	    currentNode = "";
	else
	    currentNode = tree.pop();
    }


    /*public void startPrefixMapping( String prefix, String uri ) {
	System.out.println( "startPrefixMapping" );
	}*/

    /*public void endPrefixMapping( String prefix ) {
	System.out.println( "endPrefixMapping" );
    }

    public void ignorableWhitespace( char[] ch, int start, int length ) throws SAXException {
	System.out.println( "whitespace" );
	}


    public void processingInstruction( String target, String data ) {
	System.out.println( "processingInstruction" );
    }

    public void setDocumentLocator( Locator locator ) {
	System.out.println( "setDocumentLocator" );
    }

    public void skippedEntity(String name) {
	System.out.println( "skippedEntity" );
	}*/


}
