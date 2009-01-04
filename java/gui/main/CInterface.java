package main;

import game.*;

public class CInterface implements GameInterface {
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
	    //System.out.println( System.getProperty("java.library.path") );
	    System.loadLibrary( Game.DBName );
	}
	catch( Exception e ) {
	    System.out.println( e.getMessage() );
	    System.out.println( e.getStackTrace() );
	    System.exit(1);
	}
    }


    CInterface() {}


    /* Entry functions.
       These just serve as wrappers for the native and workhorse methods below. */
    
    public Position InitialPosition() {
	return NativeInitialPosition();
    }

    public Value Primitive( Position thePosition ) {
	return NativePrimitive( thePosition );
    }

    public Move GetComputersMove( Position thePosition ) {
	return NativeGetComputersMove( thePosition );
    }
    public Value DetermineValue( Position thePosition ) {
	return NativeDetermineValue( thePosition );
    }

    public int Remoteness( Position thePosition ) {
	return NativeRemoteness( thePosition );
    }
    public int Mex( Position thePosition ) {
	return NativeMex( thePosition );
    }

    public Position DoMove( Position thePosition, Move theMove ) {
	return NativeDoMove( thePosition, theMove.GetHashedMove() );
    }
    public Position DoMove( Position thePosition, int hashedMove ) {
	return NativeDoMove( thePosition, hashedMove );
    }

    public Value GetValueOfMove( Move theMove ) {
	return NativeGetValueOfMove( theMove.GetPositionBeforeMove().GetHashedPosition(), theMove.GetHashedMove() );
    }

    public String Unhash( Position thePosition ) {
	return Unhash( thePosition.GetHashedPosition() );
    }
    public String Unhash( long hashedPosition ) {
	if( Game.UsingGenericHash ) {
	    return NativeGenericUnhash( hashedPosition );
	} else {
	    return NativeCustomUnhash( hashedPosition );
	}
    }

    /*    public void InitializeGame() {
	NativeInitializeGame();
    }
    public void Initialize() {
	NativeInitialize();
    }
    public void InitializeDatabases() {
	NativeInitializeDatabases();
	}*/


    //    public void InitializeAll() {
    public void Initialize() {
	NativeInitialize();
	NativeInitializeDatabases();
	NativeInitializeGame();
    }




    /** Code that actually does stuff. **/

    public Move[] GenerateMoves( Position thePosition ) {
	int[] moveInts = NativeGenerateMovesAsInts( thePosition );
	Move[] moves = new Move[moveInts.length];

	for( int i = 0; i < moveInts.length; ++i ) {
	    moves[i] = new Move( thePosition, moveInts[i], 
				 NativeGetValueOfMove( thePosition.GetHashedPosition(), moveInts[i] ) );
	}
	return moves;
    }

    public static native void NativeInitializeGame();

    public static native Position NativeDoMove( Position thePosition, int hashedMove );

    private static native int[] NativeGenerateMovesAsInts( Position thePosition );

    public static native Position NativeInitialPosition();

    public static native Value NativePrimitive( Position thePosition );

    public static native Value NativeGetValueOfMove( long hashedPosition, int hashedMove );



    public static native String NativeGenericUnhash( long hashedPosition );
    public static native String NativeCustomUnhash( long hashedPosition );

    public static native void NativeInitialize();
    public static native void NativeInitializeDatabases();

    public static native Move NativeGetComputersMove( Position thePosition );
    public static native Value NativeDetermineValue( Position thePosition );

    public static native int NativeRemoteness( Position thePosition );
    public static native int NativeMex( Position thePosition );

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


    //public static void InitLibraries( String game ) {
    //}

    /*    public static void main(String[] args) {
	System.out.println( InitialPosition() );
	}*/
}

