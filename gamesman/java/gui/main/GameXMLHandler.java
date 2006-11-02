package main;

import org.xml.sax.Attributes;
import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;
import org.xml.sax.Locator;

import java.util.Stack;
import java.awt.Color;

import patterns.*;
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
	/*try {
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
	    }*/
    }

    public void endDocument() {
    }

    public void startDocument() {
	tree = new Stack<String>();
	currentNode = "";
    }

    public void startElement( String uri, String localName, String qName, Attributes attributes ) {
	String value;
	if( !currentNode.equals( "" ) ) {
	    tree.push( currentNode );
	}

	try {

	    if( localName.equals( "game" ) ) {
		Game.Name = getAttribute( localName, "name", attributes, true );
		Game.DBName = getAttribute( localName, "dbname", attributes, true );

		value = getAttribute( localName, "hashtype", attributes, true );
		Game.UsingGenericHash = value.equals( "generic" );

	    } else if( localName.equals( "board" ) ) {
		value = getAttribute( localName, "type", attributes, true );
		// do something with this in the future

		value = getAttribute( localName, "width", attributes, true );
		Board.setWidth( Integer.parseInt( value ) );

		value = getAttribute( localName, "height", attributes, true );
		Board.setHeight( Integer.parseInt( value ) );

		value = getAttribute( localName, "bgcolor", attributes, false );
		if( value != null ) {
		    int color = Integer.parseInt( value, 16 );
		    Board.setColor( new Color( color ) );
		}
	    
	    } else if( localName.equals( "piece" ) ) {
		char charID;
		int pieceShape;
		int raw_color;
		Color color;

		value = getAttribute( localName, "charID", attributes, true );
		if( value.toLowerCase().equals( "space" ) ) 
		    charID = ' ';
		else
		    charID = value.charAt(0);

		value = getAttribute( localName, "shape", attributes, true );
		if( value.toUpperCase().equals( "EMPTY" ) )
		    pieceShape = Bin.SHAPE_EMPTY;
		else if ( value.toUpperCase().equals( "X" ) )
		    pieceShape = Bin.SHAPE_X;
		else if ( value.toUpperCase().equals( "O" ) )
		    pieceShape = Bin.SHAPE_O;
		else if ( value.toUpperCase().equals( "CIRCLE" ) )
		    pieceShape = Bin.SHAPE_CIRCLE;
		else if ( value.toUpperCase().equals( "PLUS" ) )
		    pieceShape = Bin.SHAPE_PLUS;
		else if ( value.toUpperCase().equals( "CUSTOM" ) )
		    pieceShape = Bin.SHAPE_CUSTOM;
		else {
		    System.out.println( "Invalid piece shape: \"" + value + "\"" );
		    pieceShape = Bin.SHAPE_EMPTY;
		}

		value = getAttribute( localName, "color", attributes, true );
		raw_color = Integer.parseInt( value, 16 );
		color = new Color( raw_color );

		Bin.addArchetype( new Bin( charID, color, pieceShape ) );
		

	    } else if( localName.equals( "move" ) ) {
		String typeStr = getAttribute( localName, "type", attributes, true );
		String targetStr = getAttribute( localName, "target", attributes, true );
		String objectStr = getAttribute( localName, "object", attributes, true );

		SimplePattern.addType( new MoveType( typeStr, targetStr, objectStr ) );
		//SimplePattern.printActivePatterns();
		    
		

		// ignore this for now
		//	    value = getAttribute( localName, "type", attributes, true );

	    }
	} catch( NumberFormatException e ) {
	    System.out.println( "Error parsing number" );
	    System.out.println( e.getMessage() );
	}
	catch( Exception e ) {
	    System.out.println( "Unknown exception" );
	    System.out.println( e.getMessage() );
	}

	currentNode = localName.toUpperCase();
	//	currentNode = localName.toLowerCase();
    }

    public void endElement( String uri, String localName, String qName ) {
	if( ! currentNode.toLowerCase().equals( localName.toLowerCase() ) ) {
	    System.out.println( "\"" + currentNode + "\" and \"" 
				+ localName.toLowerCase() + "\" don't match..." );
	}

	/*	if( currentNode.equals( "PIECE" ) ) {
	    Bin.addArchetype( new Bin( bin_charID, bin_pieceColor, bin_pieceShape ) );
	    }*/


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


    private String getAttribute( String node, String attributeName, Attributes attributes, boolean required ) {
	String s = attributes.getValue( attributeName  );

	if( s == null && required ) {
	    System.out.println( "Required attribute not found." );
	    System.out.println( "Node name:  \"" + node + "\"" );
	    System.out.println( "Attribute name:  \"" + attributeName + "\"" );
	    System.exit( 1 );
	}

	return s;
    }
}
