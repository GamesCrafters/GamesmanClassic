package main;

import game.*;

public class CInterface {
    static {
	System.out.println( "CInterface loading..." );

	if( Game.DBName == null || Game.DBName.equals( "" ) ) {
	    //throw new Exception( "Bad DBName" );
	    System.out.println( "Bad DB name" );
	    System.exit(1);
	}

	//hack I found to change the library path at runtime...	
	try {
	    String strLibPath = System.getProperty("java.library.path") + ":.:./lib";
	    System.setProperty( "java.library.path", strLibPath );

	    Class loaderClass = ClassLoader.class;
         
	    java.lang.reflect.Field userPaths = loaderClass.getDeclaredField( "sys_paths" );
	    userPaths.setAccessible( true );
	    userPaths.set( null, null );

	    //System.out.println( Game.DBName );


	    //System.loadLibrary( "CInterface" );
	    System.loadLibrary( Game.DBName );
	}
	catch( Exception e ) {
	    System.out.println( e.getMessage() );
	    System.out.println( e.getStackTrace() );
	    System.exit(1);
	}
    }

    public static native void InitializeGame();


    public static Position DoMove( Position thePosition, Move theMove ) {
	return DoMove( thePosition, theMove.GetHashedMove() );
    }
    public static native Position DoMove( Position thePosition, int hashedMove );


    public static native Position InitialPosition();

    public static native Value Primitive( Position thePosition );

    public static Value GetValueOfMove( Move theMove ) {
	return GetValueOfMove( theMove.GetPositionBeforeMove().GetHashedPosition(), theMove.GetHashedMove() );
    }
    public static native Value GetValueOfMove( long hashedPosition, int hashedMove );

    public static Move[] GenerateMoves( Position thePosition ) {
	int[] moveInts = GenerateMovesAsInts( thePosition );
	Move[] moves = new Move[moveInts.length];

	for( int i = 0; i < moveInts.length; ++i ) {
	    moves[i] = new Move( thePosition, moveInts[i], 
				 GetValueOfMove( thePosition.GetHashedPosition(), moveInts[i] ) );
	}
	return moves;
    }

    public static String Unhash( Position thePosition ) {
	return Unhash( thePosition.GetHashedPosition() ); }
    public static String Unhash( long hashedPosition ) {
	if( Game.UsingGenericHash ) {
	    return GenericUnhash( hashedPosition );
	} else {
	    return CustomUnhash( hashedPosition );
	}
    }
    public static native String GenericUnhash( long hashedPosition );
    public static native String CustomUnhash( long hashedPosition );

    public static native void Initialize();
    public static native void InitializeDatabases();

    public static native Move GetComputersMove( Position thePosition );
    public static native Value DetermineValue( Position thePosition );

    public static native int Remoteness( Position thePosition );
    public static native int Mex( Position thePosition );

    /** TODO:

       GetValueMoves(
       GetValueOfPosition(
  
       SetGameSpecificOptions(

       Random(
       ComputeC(
       GoAgain(
       GetPrediction(
       SetSmarterComputer(
       GetOption(
       SetOption(
       PercentDone(

       public static native int NumberOfOptions();
       public static native int getOption();
       public static native void setOption( int option );

    */

    public static void InitializeAll() {
	Initialize();
	InitializeDatabases();

	InitializeGame();
    }

    private static native int[] GenerateMovesAsInts( Position thePosition );


    public static void InitLibraries( String game ) {
    }

    public static void main(String[] args) {

	System.out.println( InitialPosition() );
    }
}

