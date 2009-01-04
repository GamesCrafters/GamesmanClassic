package game;

public class Value {
    private int value;

    public static int WIN = 0;
    public static int LOSE = 1;
    public static int TIE = 2;
    public static int UNDECIDED = 3;

    public Value( int value ) throws Exception {
	if( value < 0 || value > 3 )
	    throw new Exception( "Invalid value" );

	this.value = value;
    }

    public boolean isWin() {
	return value == WIN;
    }
    public boolean isLose() {
	return value == LOSE;
    }
    public boolean isTie() {
	return value == TIE;
    }
    public boolean isUndecided() {
	return value == UNDECIDED;
    }

    public int value() {
	return value;
    }

    public String toString() {
	if( isWin() ) {
	    return "Win";
	} else if( isLose() ) {
	    return "Lose";
	} else if( isTie() ) {
	    return "Tie";
	} else if( isUndecided() ) {
	    return "Undecided";
	} else {
	    return "Value not valid";
	}
    }
}
