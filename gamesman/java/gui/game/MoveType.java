package game;

import patterns.*;

public class MoveType implements Comparable {

    // These values are sorted.  Changing the order might result in broken code elsewhere.

    public static final int TYPE_SLIDE = 0;
    public static final int TYPE_POSITION = 1;
    public static final int TYPE_PLACE = 2;
    public static final int TYPE_REMOVE = 3;
    public static final int TYPE_REPLACE = 4;


    public static final int TARGET_MULTIPLE = 0;
    public static final int TARGET_SINGLE = 1;
    //current not used:
    //public static final int TARGET_RANK = 2;
    //public static final int TARGET_FILE = 3;

    public static final int OBJECT_EXISTING = 0;
    public static final int OBJECT_EMPTY = 1;

    private int type;
    private int target;
    private int object;

    public static int getTypeFromString( String s ) {
	s = s.toLowerCase();
	if( s.equals("position") )
	    return TYPE_POSITION;
	else if( s.equals("slide") )
	    return TYPE_SLIDE;
	else if( s.equals("place") )
	    return TYPE_PLACE;
	else if( s.equals("remove") )
	    return TYPE_REMOVE;
	else if( s.equals("replace") )
	    return TYPE_REPLACE;
	else
	    return -1;
    }

    public static int getTargetFromString( String s ) {
	s = s.toLowerCase();
	if( s.equals("single") )
	    return TARGET_SINGLE;
	else if( s.equals("multiple") )
	    return TARGET_MULTIPLE;
	else 
	    return -1;
    }

    public static int getObjectFromString( String s ) {
	s = s.toLowerCase();
	if( s.equals("existing") )
	    return OBJECT_EXISTING;
	else if( s.equals("empty") )
	    return OBJECT_EMPTY;
	else 
	    return -1;
    }




    public MoveType( String typeStr, String targetStr, String objectStr ) throws Exception {
	SetValues( getTypeFromString( typeStr ),
		   getTargetFromString( targetStr ),
		   getObjectFromString( objectStr ) );
    }
    public MoveType( int type, int target, int object ) throws Exception {
	SetValues( type, target, object );
    }

    private void SetValues( int type, int target, int object ) throws Exception {
	if( type != TYPE_SLIDE && type != TYPE_POSITION )
	    throw new Exception( "Bad Type" );

	if( target != TARGET_SINGLE && target != TARGET_MULTIPLE )
	    throw new Exception( "Bad Target" );

	if( object != OBJECT_EXISTING && object != OBJECT_EMPTY )
	    throw new Exception( "Bad Object" );

	this.type = type;
	this.target = target;
	this.object = object;
    }

    public int getType() {
	return type;
    }

    public int getTarget() {
	return target;
    }

    public int getObject() {
	return object;
    }

    public int compareTo( Object o ) {
	if( o instanceof MoveType ) {
	    MoveType m = (MoveType) o;

	    if( getType() == m.getType() ) {
		if( getTarget() == m.getTarget() ) {
		    if( getObject() == m.getObject() ) {
			return 0;		    
		    } else {
			return getObject() < m.getObject() ? -1 : 1;
		    }
		} else {
		    return getTarget() < m.getTarget() ? -1 : 1;
		}
	    } else {
		return getType() < m.getType() ? -1 : 1;
	    }
	} else {
	    return 0;
	}
    }

    public String toString() {
	StringBuffer sb = new StringBuffer();

	switch( type ) {
	    case TYPE_POSITION: sb.append( "po" ); break;
	    case TYPE_SLIDE: sb.append( "sl" );	break;	    
	    case TYPE_PLACE: sb.append( "pl" ); break;
	    case TYPE_REMOVE: sb.append( "rm" ); break;
	    case TYPE_REPLACE: sb.append( "rp" ); break;
	    default: sb.append( "INV" ); break;
	}
	switch( target ) {
	    case TARGET_SINGLE: sb.append( "si" ); break;
	    case TARGET_MULTIPLE: sb.append( "mu" ); break;
	    default: sb.append("INV"); break;
	}
	switch( object ) {
	    case OBJECT_EXISTING: sb.append( "ex" ); break;
	    case OBJECT_EMPTY: sb.append( "em" ); break;
	    default: sb.append("INV"); break;
	}

	return sb.toString();
    }

    public SimplePattern getSimplePattern() {
	if( type == TYPE_SLIDE && target == TARGET_SINGLE && object == OBJECT_EXISTING ) {
	    return new Slide();

	} else if( type == TYPE_POSITION && target == TARGET_SINGLE && object == OBJECT_EMPTY ) {
	    return new Place();

	} else if( type == TYPE_REMOVE && target == TARGET_SINGLE && object == OBJECT_EXISTING ) {
	    return new Remove();

	} else if( type == TYPE_REPLACE && target == TARGET_SINGLE && object == OBJECT_EXISTING ) {
	    return new Replace();

        } else {
	    return null;
	}

    }
}
