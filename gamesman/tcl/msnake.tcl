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
#              03/24/04 Added bindings, SendMove, Options code, changed GS_Initialize
######################################################

# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.

proc GS_InitGameSpecific {} {
    puts "** GS_InitGameSpecific"
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Snake"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Alice Chang, Judy Chen, Eleen Chiang, Peter Foo"
    set kTclAuthors "Judy Chen, Eleen Chiang, Peter Foo"
    #set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 8357
    set gPosition $gInitialPosition


    global kToMove kToWin

    set kToMove "\n- Click on a thin vertical or horizontal bar to place a vertical or horizontal piece in the square\n- Click on a thick piece to flip it from horizontal to vertical or vice versa.\n"

    set kToWin "\nFirst player to get any 3 vertical or horizontal pieces in a row WINS!"

    # 4x4 board
    global BOARDSIZE
    set BOARDSIZE 16

    global BOARDWIDTH
    set BOARDWIDTH 4
}

# Setup the rules frame
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables. This procedure should not modify global variables
# that affect initialization or game play. Such actions should
# occur in GS_ImplementOption. 
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing
proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    global gMisereGame
    set gMisereGame 0

    set ruleSettingGlobalNames [list "gMisereGame"]

    global kLabelFont
    set ruleset [list $standardRule]
    set ruleNum 0
    foreach rule $ruleset {
	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
	pack $rulesFrame.rule$ruleNum.label -side left
	set rulePartNum 0
	foreach rulePart [lrange $rule 1 end] {
	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
	    incr rulePartNum
	}
	incr ruleNum
    } 
    
}


# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
proc GS_GetOption { } {
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]

    # diagonals and tie loses
    set option [expr $option + 2*2 + 2] 
    return $option
}


# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option. This procedure should not modify any global 
# variables that affect initialization or game play. Such actions 
# should occur in GS_ImplementOption. 
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
proc GS_SetOption { option } {
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}


# Implement the given game option
# Modifies the global variables used to initialize and play the game 
# to match the given option. This can include the To Win and To Move 
# strings if any option modifies them. 
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used during initialization and game play
# Returns: nothing
proc GS_ImplementOption { option } {
    set option [expr $option - 1]
    set standardOption [expr $option%2]
    
    if { $standardOption == "1" } {
	set toWin1 "To Win: "
    } elseif { $standardOption == "0" } {
	set toWin1 "To Lose: "
    }

    global Dimension
    set toWin2  "Connect in a row in any direction" 

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Click on an arrow to place a piece"
}


proc GS_GetDefaultRules {} {
    global Dimension
    set kToMove
    set kToWin

    return [list]
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {
    puts "GS_NameOfPieces"
    return [list head tail]    
}


# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"

proc GS_Initialize { c } {

    puts "** GS_Initialize"
    
    # you may want to start by setting the size of the canvas; this line isn't cecessary
    $c configure -width 500 -height 500
    set counter 0
    for {set j 0} {$j < 4} {set j [expr $j + 1]} {
	for {set i 0} {$i < 4} {set i [expr $i + 1]} {

	    $c create rect [expr $i * 125] [expr $j * 125] [expr ($i+1) * 125] [expr ($j+1) * 125] -fill white -outline black -width 2 -tag [list base base$counter]
	    incr counter
	}
    }

    # counter is the NUMBER OF TILES, base$i refers to each tile i, 0<i<counter
    #puts "COUNTER:$counter"

# dont need to draw these lines since using the black outlines of the tiles
#     $c create line 125 0 125 500 -width 2 -tag base
#     $c create line 250 0 250 500 -width 2 -tag base
#     $c create line 375 0 375 500 -width 2 -tag base
    
#     $c create line 0 125 500 125 -width 2 -tag base
#     $c create line 0 250 500 250 -width 2 -tag base
#     $c create line 0 375 500 375 -width 2 -tag base
#     $c create line 0 125 500 125 -width 2 -tag base

    MakePieces $c 0
    MakeConnectors $c
    MakeArrows $c

    # bind the tiles
    for {set i 0} {$i < $counter} {incr i} {
	$c bind base$i <Enter> "TileEnter $c $i"
	$c bind base$i <Leave> "TileLeave $c $i"
	$c bind base$i <ButtonRelease-1> "$c itemconfig base$i -fill black; SendMove $i"
    }
   
    $c raise base
    #update idletasks
} 

proc SendMove { square } {
    set theMove $square

    ReturnFromHumanMove $theMove
    puts $theMove
}

proc TileEnter { c tileNum } {
    $c raise base$tileNum base
    $c itemconfig base$tileNum -outline cyan -width 4
}

proc TileLeave {c tileNum } {
    $c itemconfig base$tileNum -outline black -width 2
}

# April
# Make all the arrows (four arrows for every tile, excepting the corners and borders)
proc MakeArrows { c } {
    # ie 4x4, BOARDSIZE is 16
    global BOARDSIZE
    global BOARDWIDTH

    # remember, the make arrow functions written by us take in external move reps (from, to slots)
    for {set i 0} {$i < $BOARDSIZE} {set i [expr $i + 1]} {
	# in a tile
	
	# the tile is, in 0,1,2,...,boardsize-1 indexing
	set theTile $i

	# theTile is the "from", now generate the "to"'s
	set upTo [expr $theTile - $BOARDWIDTH]
	set downTo [expr $theTile + $BOARDWIDTH]
	set leftTo [expr $theTile - 1]
	set rightTo [expr $theTile + 1]

	if {$upTo >= 0} {
	    #puts "in upTo section"
	    drawUpArrow $c $theTile $upTo cyan

	    # prepare internal move representation for binding
	    set internalMove [SlotsToMove $theTile $upTo]

	    # now bind the arrow; this depends on the draw<Dir>Arrow procs to tag the arrows appropriately! the draw<dir>arrow procs tag with 0,1,2,...,boardsize-1 indexing
	    $c bind arrow$theTile$upTo <Enter> "ArrowEnter $c arrow$theTile$upTo"
	    $c bind arrow$theTile$upTo <Leave> "ArrowLeave $c arrow$theTile$upTo"
	    $c bind arrow$theTile$upTo <ButtonRelease-1> "$c itemconfig arrow$theTile$upTo -fill black; SendMove $internalMove"
	}
	# here assuming that the board length is same as board width (ie board is square). this is what  msnake.c  does, so i'm just following the msnake.c convention
	if {$downTo < $BOARDSIZE} {
	    #puts "in downTo section"
	    drawDownArrow $c $theTile $downTo cyan
	    set internalMove [SlotsToMove $theTile $downTo]
	    $c bind arrow$theTile$downTo <Enter> "ArrowEnter $c arrow$theTile$downTo"
	    $c bind arrow$theTile$downTo <Leave> "ArrowLeave $c arrow$theTile$downTo"
	    $c bind arrow$theTile$downTo <ButtonRelease-1> "$c itemconfig arrow$theTile$downTo -fill black; SendMove $internalMove"
	}
	if {[expr $theTile % $BOARDWIDTH] != 0 } {
	    #puts "in leftTo section"
	    #puts "TILE: $theTile"
	    puts [expr $i % $BOARDWIDTH]
	    # drawLeftArrow appears to take internal move rep? dunno, didnt write that proc
	    drawLeftArrow $c $theTile $leftTo cyan
	    set internalMove [SlotsToMove $theTile $leftTo]
	    $c bind arrow$theTile$leftTo <Enter> "ArrowEnter $c arrow$theTile$leftTo"
	    $c bind arrow$theTile$leftTo <Leave> "ArrowLeave $c arrow$theTile$leftTo"    
	    $c bind arrow$theTile$leftTo <ButtonRelease-1> "puts arrow$theTile$leftTo; $c itemconfig arrow$theTile$leftTo -fill black; SendMove $internalMove"
	}
	if {[expr ($theTile+1) % $BOARDWIDTH] != 0 } {
	    #puts "in rightTo section"
	    drawRightArrow $c $theTile $rightTo cyan
	    set internalMove [SlotsToMove $theTile $rightTo]
	    $c bind arrow$theTile$rightTo <Enter> "ArrowEnter $c arrow$theTile$rightTo"
	    $c bind arrow$theTile$rightTo <Leave> "ArrowLeave $c arrow$theTile$rightTo"
	    $c bind arrow$theTile$rightTo <ButtonRelease-1> "$c itemconfig arrow$theTile$rightTo -fill black; SendMove $internalMove"
	}
    }

    $c lower arrows all
}


proc ArrowEnter { c theTag } {
    global prevColour
    
    set prevColour [$c itemcget $theTag -fill]

    $c itemconfig $theTag -fill black
}

proc ArrowLeave { c theTag } {
    global prevColour

    $c itemconfig $theTag -fill $prevColour
}
    

proc SlotsToMove { fromSlot toSlot } {
    global BOARDSIZE

    return [expr $toSlot*($BOARDSIZE+1) + $fromSlot]
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
	MakeHorizontalConnectors $c $i [expr $i+1]	
	#puts "in makehoriz"
    }
    for {set i 0} {$i < 4} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    for {set i 4} {$i < 8} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    for {set i 8} {$i < 12} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    for {set i 12} {$i < 16} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    $c lower vconnectors base
    $c lower hconnectors base
}

# Makes the horizontal connecting body pieces
proc MakeHorizontalConnectors { c from to } {
    if {$from == [expr $to - 1]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 95] [expr $y + 30] [expr $x + 155] [expr $y + 95] -fill green3 -tag [list connectors hconnectors hcon$from$to]
    }
    if {$from == [expr $to + 1]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x - 25] [expr $y + 30] [expr $x + 35] [expr $y + 95] -fill green3 -tag [list connectors hconnectors hcon$from$to]
	#puts $from
	#puts $to
    }
    puts "**** hcon$from$to"
}

# Makes vertical connecting pieces
proc MakeVerticalConnectors { c from to} {
    if {$from == [expr $to + 4]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 30] [expr $y - 25 ] [expr $x + 95] [expr $y + 35] -fill green3 -tag [list connectors vconnectors vcon$from$to]
    }
    if {$from == [expr $to - 4]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 30] [expr $y + 95] [expr $x + 95] [expr $y + 155] -fill green3 -tag [list connectors vconnectors vcon$from$to]
	puts "---- vcon$from$to"
	#puts $from
	#puts $to
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
    puts "** GS_DrawPosition"
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
    global BOARDSIZE

    set findHead [expr $position & 15]
    set findTail [expr $position >> 4]
    set findTail [expr $findTail & 15]
    set findBody [expr $position >> 8]

# originals
#    set findTail [expr $position & 240]
#    set findTail [expr $findTail >> 4]

    # Building the board sequentially

# april - i started changing here

    for {set i 0} {$i<$BOARDSIZE} {incr i} {
	lappend board "blank"
    }

    lset board $findHead "head"
    lset board $findTail "tail"

    for {set i 0} {$i<$BOARDSIZE} {incr i} {
	if {[lindex $board $i]=="blank"} {
	    if {[expr $findBody & 1]} {
		lset board $i "body"
		set findBody [expr $findBody >> 1]
	    } else {
		lset board $i "blank"
		set findBody [expr $findBody >> 1]
	    }
	}
    }

#     #original
#     for {set i 0} {$i<$BOARDSIZE} {incr i} {
# 	if {$i == $findHead} {
# 	    lappend board "head"
# 	} elseif {$i == $findTail} {
# 	    lappend board "tail"
# 	} elseif {[expr $findBody & 1]} {
# 	    lappend board "body" 
# 	    set findBody [expr $findBody >> 1]
# 	} else {
# 	    lappend board "blank"
# 	    set findBody [expr $findBody >> 1]
# 	}
#     }

    return $board
}

# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    puts "** GS_NewGame"
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position

    # Assume initial position is 8357 (for now). Draw the body connectors.
    MakeHorizontalConnectors $c 5 6
    MakeVerticalConnectors $c 6 10
    $c raise head
    $c raise tail   
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    global BOARDSIZE
    puts "** GS_WhoseMove"
    set board [unhash $position]
    set count 0  
    for {set i 0} {$i<$BOARDSIZE} {incr i} {
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
    global BOARDSIZE
    puts "** GS_HandleMove"

    set oldl [unhash $oldPosition]
    set newl [unhash $newPosition]

    #theMove is in internal rep format, gotta convert to external (from, slot) format, from slot format is in 0,1,2,...,boardsize-1 indexing form

    set from [expr $theMove % ($BOARDSIZE+1)]
    set to [expr $theMove / ($BOARDSIZE+1)]
    set piece head

    puts "theMove: $theMove"
    puts "from: $from"
    puts "to: $to"

    puts [GS_WhoseMove $oldPosition]

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
    #temp, cause my temp random "positions" seem to cause none of above direction if's to be hit
    #set direction right
    puts "---- direction: $direction"
    puts "---- piece: $piece"
    $c raise body$from 
    MovePiece $c $piece $direction
    if {$direction == "up"} {
	$c raise vcon$from$to all
    } elseif {$direction == "down"} {
	$c raise vcon$from$to all
#	MakeVerticalConnectors $c $from $to
    } elseif {$direction == "left"} {
	$c raise hcon$from$to all
	#MakeHorizontalConnectors $c $from $to
    } elseif {$direction == "right"} {
	$c raise hcon$from$to all
	#MakeHorizontalConnectors $c $from $to
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

# all draw<dir>Arrow procs take 0,1,2,..,boardsize-1 indexing
proc drawUpArrow { c from to color} {
    set x [expr $from % 4]
    set y [expr $to / 4]
    $c create poly [expr $x*125 + 25] [expr $y*125 + 110] [expr $x*125 + 62] [expr $y*125 + 90] [expr $x*125 + 100] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 110] -fill $color -tags [list arrows upArrow upArrow$from$to arrow$from$to]
}

proc drawDownArrow { c from to color } {
    set x [expr $from % 4]
    set y [expr $to / 4 - 1]
    $c create poly [expr $x*125 + 50] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 140] [expr $x*125 + 100] [expr $y*125 + 140] [expr $x*125 + 62] [expr $y*125 + 160] [expr $x*125 + 25] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 140] -fill $color -tags [list arrows downArrow downArrow$from$to arrow$from$to]
}

proc drawRightArrow {c from to color } {
    set x [expr $from % 4]
    set y [expr $to / 4 ]
    $c create poly  [expr $x*125 + 110] [expr $y*125 + 50] [expr $x*125 + 110] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 100] [expr $x*125 + 160] [expr $y*125 + 62] [expr $x*125 + 140] [expr $y*125 + 25] [expr $x*125 + 140] [expr $y*125 + 50] -fill $color -tags [list arrows rightArrow rightArrow$from$to arrow$from$to]
}    

proc drawLeftArrow { c from to color} {
    set x [expr $from % 4 - 1]
    set y [expr $to / 4]
    $c create poly [expr $x*125 + 110] [expr $y*125 + 25] [expr $x*125 + 90] [expr $y*125 + 62] [expr $x*125 + 110] [expr $y*125 + 100] [expr $x*125 + 110] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 50] [expr $x*125 + 110] [expr $y*125 + 50] -fill $color -tags [list arrows leftArrow leftArrow$from$to arrow$from$to]
}

proc GS_ShowMoves { c moveType position moveList } {
    global BOARDSIZE

    puts "** GS_ShowMoves"

    puts "entered"
    set whoseTurn [GS_WhoseMove $position]
    set from [findPiece $whoseTurn $position]  
    puts $from
    puts "say what"
    foreach item $moveList {
	#	set color [MoveTypeToColor $moveType]
	puts "in GS_ShowMoves: $item"

	# this theMove is in internal rep format
	set theMove [lindex $item 0]
	# convert to external move format (ie from, to)
	set fromSlot [expr $theMove % ($BOARDSIZE+1)]
	set toSlot [expr $theMove / ($BOARDSIZE+1)]

	set value [lindex $item 1]
	set color cyan

 	if {$moveType == "value"} {
 	    if {$value == "Tie"} {
 		set color yellow
 	    } elseif {$value == "Lose"} {
 		set color red
 	    } else {
 		set color green  
		#switched colors (green/red)
 	    }
	}
	puts "Got past color picker"

	$c itemconfig arrow$fromSlot$toSlot -fill $color
	$c raise arrow$fromSlot$toSlot all


# 	# april - replaced $item with $toSlot
# 	if {$toSlot < $from} {
# 	    set n [expr $toSlot + 1]
# 	    if {$n == $from} { 
# 		#if item + 1 = from, then it must an left arrow
# 		drawLeftArrow $c $from $toSlot $color
# 	    } else {
# 		drawUpArrow $c $from $toSlot $color
# 	    }
# 	} else {
# 	    set n [expr $toSlot - 1]
# 	    if {$n == $from} {
# 		drawRightArrow $c $from $toSlot $color
# 	    } else {
# 		drawDownArrow $c $from $toSlot $color
# 	    }
# 	}
	update idletasks
    }
}

# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
# Or I could have created all of the arrows in GS_Initialize and just raise/lower them in showing moves (just don't forget to change colors in GS_ShowMoves).
proc GS_HideMoves { c moveType position moveList} {
    global BOARDSIZE

    puts "GS_HideMoves"

    set whoseTurn [GS_WhoseMove $position]
    set from [findPiece $whoseTurn $position]
    puts $moveList
    foreach item $moveList {
	set item [lindex $item 0]

	# this theMove is in internal rep format
	set theMove [lindex $item 0]
	# convert to external move format (ie from, to)
	set fromSlot [expr $theMove % ($BOARDSIZE+1)]
	set toSlot [expr $theMove / ($BOARDSIZE+1)]

	$c lower arrow$fromSlot$toSlot all

	# if {$item < $from} {
# 	    #puts $from
# 	    #puts $item
# 	    set n [expr $item + 1]
# 	    if {$n == $from} { 
# 		#if item + 1 = from, then it must an left arrow
# 		$c lower leftArrow base
# 	    } else {
# 		$c lower upArrow base
# 	    }
# 	} else {
# 	    set n [expr $item - 1]
# 	    if {$n == $from} {
# 		$c lower rightArrow base 
# 	    } else {
# 		$c lower downArrow base 
# 	    }
# 	}
	update idletasks
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
    puts "** GS_HandleUndo"
    GS_DrawPosition $c positionAfterUndo
}





# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}

# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.

proc GS_ColorOfPlayers {} {
    puts "** GS_ColorOfPlayers"

    return [list black green]
}


# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    puts "** GS_GameOver"
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}
