####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################
#####################################################
#
# NAME:        msnake.tcl
#
# DESCRIPTION: Snake
#
# AUTHORS:     Judy Chen, Eleen Chiang, Peter Foo
#              University of California at Berkeley
#              Copyright (C) 2003. All rights reserved.
#
# DATE:        05/07/03
######################################################

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
    set kGameName "Snake"

    # Authors Info
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Alice Chang, Judy Chen, Eleen Chiang, Peter Foo"
    set kTclAuthors "Judy Chen, Eleen Chiang, Peter Foo"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 8357
    set gPosition $gInitialPosition
}



# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {
    return [list head tail]    
}


# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"

proc GS_Initialize { c } {
    
    # you may want to start by setting the size of the canvas; this line isn't cecessary
    $c configure -width 500 -height 500
    set counter 0
    for {set j 0} {$j < 4} {set j [expr $j + 1]} {
	for {set i 0} {$i < 4} {set i [expr $i + 1]} {

	    $c create rect [expr $i * 125] [expr $j * 125] [expr ($i+1) * 125] [expr ($j+1) * 125] -fill white -tag [list base base$counter]
	    incr counter
	}
    }

    $c create line 125 0 125 500 -width 2 -tag base
    $c create line 250 0 250 500 -width 2 -tag base
    $c create line 375 0 375 500 -width 2 -tag base
    
    $c create line 0 125 500 125 -width 2 -tag base
    $c create line 0 250 500 250 -width 2 -tag base
    $c create line 0 375 500 375 -width 2 -tag base
    $c create line 0 125 500 125 -width 2 -tag base
    MakePieces $c 0
    MakeConnectors $c
   
    $c raise base
    #update idletasks
} 

# Makes the body pieces of the snake, but not the head or tail.
proc MakePieces { c num } {
    MakeBody $c [expr $num % 4] [expr $num / 4] $num
    if { $num != 15 } {
	MakePieces $c [expr $num + 1]
    }
}

# Makes the body pieces of the snake.  Maybe combine MakeBody and MakePieces?
proc MakeBody { c x y tag } {
    set x [expr $x * 125]
    set y [expr $y * 125]
    $c create rect $x $y [expr $x + 124] [expr $y + 124] -fill green3 -tag [list body body$tag]
    $c lower body$tag base 
}

# Makes the connecting pieces of the body. 
proc MakeConnectors { c } {
    for {set i 0} {$i < 16} {set i [expr $i + 1]} {
	MakeHorizontalConnectors $c $i $i+1	
    }
    for {set i 0} {$i < 4} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i $i+4
    }
    for {set i 4} {$i < 8} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i $i+4
    }
    for {set i 8} {$i < 12} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i $i+4
    }
    for {set i 12} {$i < 16} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i $i+4
    }
    $c lower vconnectors base
    $c lower hconnectors base
}

# Makes the horizontal connecting body pieces
proc MakeHorizontalConnectors { c from to } {
    if {$from == [expr $to - 1]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 95] [expr $y + 30] [expr $x + 155] [expr $y + 95] -fill green3 -tag [list hconnectors hcon$from$to]
    }
    if {$from == [expr $to + 1]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x - 25] [expr $y + 30] [expr $x + 35] [expr $y + 95] -fill green3 -tag [list hconnectors hcon$from$to]
    }
}

# Makes vertical connecting pieces
proc MakeVerticalConnectors { c from to} {
    if {$from == [expr $to + 4]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 30] [expr $y - 25 ] [expr $x + 95] [expr $y + 35] -fill green3 -tag [list vconnectors vcon$from$to]
    }
    if {$from == [expr $to - 4]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 30] [expr $y + 95] [expr $x + 95] [expr $y + 155] -fill green3 -tag [list vconnectors vcon$from$to]
    }
}





## Draws a head in square SLOT on the board.
proc MakeHead { c slot } {
    set x [expr 125 * [expr $slot % 4]]
    set y [expr 125 * [expr $slot / 4]]
    $c create oval [expr $x+15] [expr $y+15] [expr $x+110] [expr $y+110] -fill green -tag head
    $c create oval [expr $x+38] [expr $y+35] [expr $x+52] [expr $y+65] -fill red -tag [list head eyes]
    $c create oval [expr $x+68] [expr $y+35] [expr $x+82] [expr $y+65] -fill red -tag [list head eyes]
    $c create oval [expr $x+40] [expr $y+45] [expr $x+50] [expr $y+55] -fill black -tag [list head eyes pupils]
    $c create oval [expr $x+70] [expr $y+45] [expr $x+80] [expr $y+55] -fill black -tag [list head eyes pupils]
     $c create line [expr $x+40] [expr $y+85] [expr $x+80] [expr $y+85] -fill black -width 2 -tag [list head mouth] 
    #$c raise pupils eyes
}

# Makes the tail in SLOT
proc MakeTail {c slot } {
    set x [expr 125 * [expr $slot % 4]]
    set y [expr 125 * [expr $slot / 4]]
    $c create oval [expr $x+15] [expr $y+15] [expr $x+110] [expr $y+110] -fill blanchedalmond -tag tail
    $c create oval [expr $x+30] [expr $y+30] [expr $x+95] [expr $y+95] -fill burlywood -tag [list tail ring1]
    $c create oval [expr $x+45] [expr $y+45] [expr $x+80] [expr $y+80] -fill brown4 -tag [list tail ring1 ring2]
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

proc GS_DrawPosition { c position } {
    # BTW too: don't make any assumptions about the state of the board.
    # Clears the board
    $c raise base 
    # Get Board
    set board [unhash $position]
    
    # Place head and tail on board
    set whereHead [lsearch -exact $board "head"] 
    set whereTail [lsearch -exact $board "tail"]
    MakeHead $c $whereHead
    MakeTail $c $whereTail

    # Raise body pieces
    for {set i 0} {$i<16} {incr i} {
	if {[lindex $board $i] == "body"} { 
	    $c raise body$i
	}
    }

   # Make Connections
   #set x 0
    #set y 1
    #foreach {a b} $board {
	#if {{$a == "head"} || {$a == "tail"} || {$a == "body"}  #&& 
	#{$b == "head"} || {$b == "tail"} || {$b == "body"}} {
	#    $c raise body$x-$y
	#}
	#incr x 
	#incr y 
    #} 
}

proc unhash { position } {
    set findHead [expr $position & 15]
    set findTail [expr $position & 240]
    set findTail [expr $findTail >> 4]
    set findBody [expr $position >> 8]

    # Building the board sequentially
    for {set i 0} {$i<16} {incr i} {
	if {$i == $findHead} {
	    lappend board "head"
	} elseif {$i == $findTail} {
	    lappend board "tail"
	} elseif {[expr $findBody & 1]} {
	    lappend board "body" 
	    set findBody [expr $findBody >> 1]
	} else {
	    lappend board "blank"
	    set findBody [expr $findBody >> 1]
	}
    }

    return $board
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
    
    # Assume initial position is 8357 (for now). Draw the body connectors.
    MakeHorizontalConnectors .c 5 6
    MakeVerticalConnectors .c 6 10
    $c raise head
    $c raise tail   
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    set board [unhash $position]
    set count 0  
    for {set i 0} {$i<16} {incr i} {
        if {[lindex $board $i] == "body"} {
            set count [expr $count + 1]
        }
    }

    if {[expr $count % 2] == 0} {
        set who tail
    } else {
        set who head
    }
    return $who
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

    set from [lindex $theMove 0]
    set to [lindex $theMove 1]
    set piece head

    if { [GS_WhoseMove $oldPosition] == "tail"} {
	set piece tail
    }
    if {$to == [expr $from + 1]} {
	set direction  right
    }
    if {$to == [expr $from + 4]} {
	set direction  down
    }
    if {$to == [expr $from - 1]} {
	set direction  left
    }
    if {$to == [expr $from - 4]} {
	set direction  up
    }
    $c raise body$from 
    MovePiece $c $piece $direction
    if {$direction == "up"} {
	MakeVerticalConnectors .c $from $to
    } elseif {$direction == "down"} {
	MakeVerticalConnectors .c $from $to
    } elseif {$direction == "left"} {
	MakeHorizontalConnectors .c $from $to
    } elseif {$direction == "right"} {
	MakeHorizontalConnectors .c $from $to
    }
    $c raise head
}

# Moves the PIECE, which is either head or tail
proc MovePiece {c piece direction} {
    if {$direction == "left"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece -5 0
	    update idletasks
	}
    } elseif {$direction == "right"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece 5 0
	    update idletasks
	}
    } elseif {$direction == "up"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece 0 -5
	    update idletasks
	}
    } elseif {$direction == "down"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece 0 5
	    update idletasks
	}
    }
}


# finds the location of the piece
proc findPiece { piece position } {
    set board [unhash $position]
    for {set i 0} {$i < 16} {incr i} {
	if {[lindex $board $i] == $piece} {
	    return $i
	}
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
proc drawUpArrow { c from to color} {
    set x [expr $from % 4]
    set y [expr $to / 4]
    $c create poly [expr $x*125 + 25] [expr $y*125 + 110] [expr $x*125 + 62] [expr $y*125 + 90] [expr $x*125 + 100] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 110] -fill $color -tag upArrow
}

proc drawDownArrow { c from to color } {
    set x [expr $from % 4]
    set y [expr $to / 4 - 1]
    $c create poly [expr $x*125 + 50] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 140] [expr $x*125 + 100] [expr $y*125 + 140] [expr $x*125 + 62] [expr $y*125 + 160] [expr $x*125 + 25] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 140] -fill $color -tag downArrow
}

proc drawRightArrow {c from to color } {
    set x [expr $from % 4]
    set y [expr $to / 4 ]
    $c create poly  [expr $x*125 + 110] [expr $y*125 + 50] [expr $x*125 + 110] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 100] [expr $x*125 + 160] [expr $y*125 + 62] [expr $x*125 + 140] [expr $y*125 + 25] [expr $x*125 + 140] [expr $y*125 + 50] -fill $color -tag rightArrow
}    

proc drawLeftArrow { c from to color} {
    set x [expr $from % 4 - 1]
    set y [expr $to / 4]
    $c create poly [expr $x*125 + 110] [expr $y*125 + 25] [expr $x*125 + 90] [expr $y*125 + 62] [expr $x*125 + 110] [expr $y*125 + 100] [expr $x*125 + 110] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 50] [expr $x*125 + 110] [expr $y*125 + 50] -fill $color -tag leftArrow
}

proc GS_ShowMoves { c moveType position moveList } {
    puts "entered"
    set whoseTurn [GS_WhoseMove $position]
    set from [findPiece $whoseTurn $position]  
    puts "say what"
    foreach item $moveList {
	#	set color [MoveTypeToColor $moveType]
 	if {$moveType == "value"} {
 	    if {$value == "Tie"} {
 		set color yellow
 	    } elseif {$value == "Lose"} {
 		set color red
 	    } else {
 		set color green  #switched colors (green/red)
 	    }
	    puts "Got past color picker"
	    if {$item < $from} {
		set n [expr $item + 1]
		if {$n == $from} { #if item + 1 = from, then it must an left arrow
		    drawLeftArrow $c $from $item $color
		} else {
		    drawUpArrow $c $from $item $color
		}
	    } else {
		set n [expr $item - 1]
		if {$n == $from} {
		    drawRightArrow $c $from $item $color
		} else {
		    drawDownArrow $c $from $item $color
		}
	    }
	    #updateidletasks
	}
    }
}

# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
# Or I could have created all of the arrows in GS_Initialize and just raise/lower them in showing moves (just don't forget to change colors in GS_ShowMoves).
proc GS_HideMoves { c moveType position moveList} {
    set whoseTurn [GS_WhoseMove $position]
    set from [findPiece $whoseTurn $position]
    foreach item $moveList {
	if {$item < $from} {
	    set n [expr $item + 1]
	    if {$n == $from} { #if item + 1 = from, then it must an left arrow
		$c lower leftArrow base
	    } else {
		$c lower upArrow base
	    }
	} else {
	    set n [expr $item - 1]
	    if {$n == $from} {
		$c lower rightArrow base 
	    } else {
		$c lower downArrow base 
	    }
	}
	#updateidletasks
    }	    
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
    GS_DrawPosition c positionAfterUndo
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
