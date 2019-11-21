package main;

import game.*;

public interface GameInterface {

    public void Initialize();

    public Position DoMove( Position thePosition, Move theMove );
    public Position DoMove( Position thePosition, int hashedMove );

    public Position InitialPosition();

    public Value Primitive( Position thePosition );

    public Value GetValueOfMove( Move theMove );

    public Move[] GenerateMoves( Position thePosition );

    public String Unhash( Position thePosition );
    public String Unhash( long hashedPosition );

    public Move GetComputersMove( Position thePosition );
    public Value DetermineValue( Position thePosition );

    public int Remoteness( Position thePosition );
    public int Mex( Position thePosition );

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

    /*public void InitializeAll();
    public void InitializeDatabases();
    public void InitializeGame();*/
}

