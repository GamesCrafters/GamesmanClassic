#######################################################################
##
## NAME:         gamesmanmsim.tcl
##
## DESCRIPTION:  Sim override of gamesman.tcl
##
#############################################################################

#############################################################################
##
## DeleteMoves
##
## This is a small procedure to delete all moves on a board.
## Note that if there are no moves on the board, that's ok.
##
#############################################################################

proc DeleteMoves {} {
    global gPosition kBigPiece gSlotsX gSlotsY
    
    .winBoard.c delete tagMoves
    
    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
        DrawMove .winBoard.c $theMove gray $kBigPiece
    } 

    global varMoves
    if { $varMoves == "noMoves" && [winfo exists .winValueMoves] } {
	destroy .winValueMoves
    }
}

#############################################################################
##
## ShowMoves
##
## This is a small procedure to show all available moves on the board.
## For every move returned by C_GetValueMoves, draw it.
##
#############################################################################

proc ShowMoves {} {
    global gPosition kBigPiece gSlotsX gSlotsY
        
    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
        DrawMove .winBoard.c $theMove cyan $kBigPiece
    } 

    if { [winfo exists .winValueMoves] } {
	destroy .winValueMoves
    }
}

#############################################################################
##
## DrawMoveCustom
##
## A new game has been requested and it's the computer's turn.
## It's assumed that the board exists already. We get the move and do it.
##
#############################################################################

proc DrawMoveCustom { w theMoveArg color drawBig } {

    set moveList [$w find withtag tagIndicator-$theMoveArg]

    foreach item $moveList {
	$w itemconfig $item -fill $color
    }

    if { [llength $moveList]==0 } {
	global gSlotSize
	set circleSize [expr $gSlotSize*.2]
	$w create oval 0 0 $circleSize $circleSize \
	     -outline $color \
	     -fill $color
    }
}

#############################################################################
##
## HandleGameOver
##
## The game was just over and we have to clean up.
##
#############################################################################

proc HandleGameOver { w theValue } {

    global gPlayerOneTurn
    global gLeftPlayerType gRightPlayerType
    global gGameDelay
    global gPosition gInitialPosition
    global gAgainstComputer gHumanGoesFirst

    ### Update the status box.
    
    .f0.mesStatus config \
	    -text "[.f3.entPlayer[expr !$gPlayerOneTurn + 1] get] [PastTenseValue $theValue] [.f3.entPlayer[expr $gPlayerOneTurn + 1] get]\nClick 'New Game' for more!"
    
    ### Make all the other slots inactive
    
    $w addtag tagDead withtag tagAlive
    for { set x 0 } { $x<15 } { incr x} {
	$w addtag tagUnclickable-$x withtag tagClickable-$x
	$w dtag tagClickable-$x
    }

    ##by AP - play a new game if comp vs comp
    if { ($gLeftPlayerType && $gRightPlayerType) } {
	update
	after [expr int($gGameDelay * 1000)]
	
	### Make the code a bit easier to read
	
	set w .winBoard.c
	
	### Clear the message field

	.f0.mesStatus config -text "Starting a new game\n"

	### Reset the position to the initial position

	set gPosition $gInitialPosition
	set gPlayerOneTurn 1
	set gAgainstTheComputer 1
	set gHumanGoesFirst 0

	### Call the Game-specific New Game command
	
	DeleteMoves
	
	GS_NewGame $w
	
	global varMoves
	if { $varMoves == "validMoves" } {
	    ShowMoves
	} elseif { $varMoves == "valueMoves" } {
	    ShowValueMoves
	}

	### Remove all Dead and Alive tags and reset them to what they were
	### when we reset the game.

	$w dtag tagDead
	$w dtag tagAlive
	$w addtag tagDead withtag tagNotInitial
	$w addtag tagAlive withtag tagInitial

    }
}
