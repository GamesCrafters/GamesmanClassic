
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "The FABULOUS game of Nim"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 65534
    set gPosition $gInitialPosition

    ### tomove and towin (write these)
    global kToMove kToWin
    set kToMove ""
    set kToWin "" 
}



# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {
    return [list left right]
}


# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"

proc GS_Initialize { c } {

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    $c configure -width 400 -height 400
    
    $c create rect 0 0 400 400 -fill white -outline white -tag base
    $c create rect 0 300 400 400 -fill brown -outline black -tag base
    
    for {set j 0} {$j<7} {incr j} {
	for {set i 0} {$i<5} {incr i} {
	    
	    set x [expr 70*$i+60]
	    set y [expr 400-(30*$j+50)]
	    set h [expr $j+1]
	    set w [expr $i+1]
	    $c create oval [expr $x-25] [expr $y-20] [expr $x+25] [expr $y+20] \
		-outline black -fill SteelBlue2 -tag move-$w$h
	    $c bind move-$w$h <ButtonRelease-1> "MyReturnFromHumanMove $w $h"
	}
    }
    
}

proc GS_Deinitialize { c } {
}

proc MyReturnFromHumanMove {w h} {
    global gPosition
    set l [unhash $gPosition]
    set dummy [expr [lindex $l [expr $w-1]]-$h+1]
    ReturnFromHumanMove $w$dummy
}



# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.

# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.

proc unhash { position } {
    set position [expr $position/2]
    set retval []
    foreach i {1 2 3 4 5} {
	lappend retval [expr $position%8]
	set position [expr $position/8]
    }
    
    return $retval
}

proc GS_DrawPosition { c position } {
    
    set l [unhash $position]
    
    for {set j 0} {$j<7} {incr j} {
	for {set i 0} {$i<5} {incr i} {
	    
	    set x [expr 70*$i+60]
	    set y [expr 400-(30*$j+50)]
	    set h [expr $j+1]
	    set w [expr $i+1]
	    $c coords move-$w$h [expr $x-25] [expr $y-20] [expr $x+25] [expr $y+20]
	    $c raise move-$w$h
	}
    }
    
    for {set i 1} {$i<6} {incr i} {
	set height [lindex $l [expr $i-1]]
	for {set j 7} {$j>$height} {incr j -1} {
	    $c move move-$i$j 0 -400
	}
    }
}


# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    return [lindex {left right} [expr $position%2]]
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
    set oldl [unhash $oldPosition]
    set newl [unhash $newPosition]
    set i  [expr $theMove/10]
    
    set im1 [expr $i-1]
    
    for {set frame 0} {$frame < 50} {incr frame} {
	
	for {set j [lindex $oldl $im1]} {$j>[lindex $newl $im1]} {incr j -1} {
	    $c move move-$i$j 0 -8
	}
	update idletasks
    }
}


# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.

# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of available moves to the player.  These moves are represented as numbers (same as in C)

# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveTypeToColor that takes in moveType and
# returns the correct color.

proc GS_ShowMoves { c moveType position moveList } {
    set l [unhash $position]
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set color cyan
	
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red
	    }
	}
	set i [expr $move/10]
	set j [expr [lindex $l [expr $i-1]]-$move%10+1]
	$c itemconfigure move-$i$j -fill $color
    }
    update idletasks    
}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    
}



# GS_HandleUndo draws a move undoing itself sortof.
# the names of the arguments explain themsleves but just to clarify...
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A

# By default this function just calls GS_DrawPosition, but you certainly don't need to keep that.

proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
    GS_DrawPosition $c $positionAfterUndo
}



# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}



# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner } {
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}
