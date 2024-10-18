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
		    Board.setBackgroundColor( new Color( color ) );
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

	    } else if( localName.equals( "grid" ) ) {
		String sColor = getAttribute( localName, "color", attributes, true );
		Board.setGridColor( new Color( Integer.parseInt( sColor, 16 )));

	    } else if ( localName.equals( "line" ) || localName.equals( "circle" ) ) {
		String sOrientation = getAttribute( localName, "orientation", attributes, false );
		String sRepeat = getAttribute( localName, "repeat-every", attributes, false );
		String sOffset = getAttribute( localName, "offset", attributes, false );
		String sSize = getAttribute( localName, "size", attributes, false );
		String sColor = getAttribute( localName, "color", attributes, false );



		//Fix values if they aren't given
		int repeat = (sRepeat == null) ? 1 : Integer.parseInt( sRepeat );
		int offset = (sOffset == null) ? 0 : Integer.parseInt( sOffset );
		double size = (sSize == null) ? .1 : Integer.parseInt( sSize.replace("%", "") )/100.0;
		Color color = sColor == null ? Color.BLACK : new Color( Integer.parseInt( sColor, 16 ));
		sOrientation = (sOrientation == null) ? "NONE" : sOrientation;

		Board.Decoration decoration;

		if( localName.equals("line" ) )
		    decoration = new Board.Decoration.Line( color, sOrientation, repeat, offset, size );
		else if( localName.equals("circle") ) {
		    decoration = new Board.Decoration.Circle( color, repeat, offset, size );
		} else
		    return;

		Board.addDecoration( decoration );

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
