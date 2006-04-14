package game;

public class MoveType {

    public static int TYPE_SLIDE = 0;
    public static int TYPE_POSITION = 1;

    public static int TARGET_SINGLE = 0;
    public static int TARGET_MULTIPLE = 1;
    //current not used:
    public static int TARGET_RANK = 2;
    public static int TARGET_FILE = 3;

    public static int OBJECT_EXISTING = 0;
    public static int OBJECT_EMPTY = 1;

    

    private int type;
    private int target;
    private int object;

    public MoveType( int type, int target, int object ) throws Exception {
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
}
