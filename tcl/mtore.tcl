####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game

    global kGameName 
    set kGameName "Mu Torere"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 129
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to trap their opponent so their opponent can't move, WINS"
    set kMisereString "First player to not be able to move, LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

     global gMisereGame
     if {!$gMisereGame} {
   	SetToWinString "To Win: You must trap your opponent such that he/she can't move anymore."
     } else {
   	SetToWinString "To Win: You must trap yourself such that you can't move anymore."
     }
     SetToMoveString "To Move: You must move one of your pieces into a vacant space that is next to that piece. You may only move one piece each turn. In order to move from the edge to the center, your piece must be next to one of your opponent's pieces. You may move one of your pieces on the edge into an adjacent vacant edge."



     # Authors Info. Change if desired
     global kRootDir
     global kCAuthors kTclAuthors kGifAuthors
     set kCAuthors "Jeffrey Chou"
     set kTclAuthors "Jeffrey Chou, Geoffrey Kwan"
     set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}


#############################################################################
# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
#############################################################################
proc GS_NameOfPieces {} {

    return [list x o]

}


#############################################################################
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
#############################################################################
proc GS_ColorOfPlayers {} {

    return [list blue red]
    
}


#############################################################################
# GS_SetupRulesFrame sets up the rules frame;
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing
#############################################################################
proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame"]

    global kLabelFont
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


#############################################################################
# GS_GetOption gets the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
#############################################################################
proc GS_GetOption { } {
    # TODO: Needs to change with more variants
    global gMisereGame
    return $gMisereGame
}


#############################################################################
# GS_SetOption modifies the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
#############################################################################
proc GS_SetOption { option } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set gMisereGame $option
}


proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}


#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################


proc GS_Initialize { c } {

    global xCenter yCenter
    global smallCircleDiam largeCircleDiam pieceCircleDiam largeCircleRadius cornerOffset
    global gInitialPosition gPosition gFrameWidth gFrameHeight cWidth cHeight

    set cWidth [min $gFrameWidth $gFrameHeight]
    set cHeight $cWidth
    
    if {$cWidth < $cHeight} {
	set cSmallerDim $cWidth
    } else {
	set cSmallerDim $cHeight
    }


    set xCenter [expr $cWidth / 2]
    set yCenter [expr $cHeight / 2]

    set smallCircleDiam [expr $cSmallerDim * 0.16]
    set largeCircleDiam [expr $cSmallerDim * 0.7]
    set pieceCircleDiam [expr $cSmallerDim * 0.12]
    set largeCircleRadius [expr $largeCircleDiam / 2]

    set pi [expr {atan(1)} * 4]
    set cornerOffset [expr {cos([expr $pi / 4])} * $largeCircleRadius]


    ##### background circles
    
    ## large background circle
    drawOval $c $xCenter $yCenter [expr $largeCircleDiam * 1.24] "grey" "black" "bgGreyCircle"    

    ## large background circle
    drawOval $c $xCenter $yCenter $largeCircleDiam "black" "white" "bgCircle"

    ## center circle
    drawOval $c $xCenter $yCenter $smallCircleDiam "white" "black" "bgMiniCircle"



    ## top circle
    drawOval $c $xCenter [expr $yCenter - $largeCircleRadius] $smallCircleDiam "white" "black" "bgMiniCircle"

    ## bottom circle
    drawOval $c $xCenter [expr $yCenter + $largeCircleRadius] $smallCircleDiam "white" "black" "bgMiniCircle"

    ## left circle
    drawOval $c [expr $xCenter - $largeCircleRadius] $yCenter $smallCircleDiam "white" "black" "bgMiniCircle"

    ## right circle
    drawOval $c [expr $xCenter + $largeCircleRadius] $yCenter $smallCircleDiam "white" "black" "bgMiniCircle"



    ## upper left circle
    drawOval $c [expr $xCenter - $cornerOffset] [expr $yCenter - $cornerOffset] $smallCircleDiam "white" "black" "bgMiniCircle"

    ## lower left circle
    drawOval $c [expr $xCenter - $cornerOffset] [expr $yCenter + $cornerOffset] $smallCircleDiam "white" "black" "bgMiniCircle"

    ## upper right circle
    drawOval $c [expr $xCenter + $cornerOffset] [expr $yCenter - $cornerOffset] $smallCircleDiam "white" "black" "bgMiniCircle"

    ## lower right circle
    drawOval $c [expr $xCenter + $cornerOffset] [expr $yCenter + $cornerOffset] $smallCircleDiam "white" "black" "bgMiniCircle"






    ##### lines
    
    ## line from middle to top
    $c create line $xCenter $yCenter [expr $xCenter + $largeCircleRadius] $yCenter -fill white -tag line

    ## line from middle to bottom
    $c create line $xCenter $yCenter [expr $xCenter - $largeCircleRadius] $yCenter -fill white -tag line

    ## line from middle to left
    $c create line $xCenter $yCenter $xCenter [expr $yCenter - $largeCircleRadius] -fill white -tag line

    ## line from middle to right
    $c create line $xCenter $yCenter $xCenter [expr $yCenter + $largeCircleRadius] -fill white -tag line


    ## line from middle to upper left
    $c create line $xCenter $yCenter [expr $xCenter - $cornerOffset] [expr $yCenter - $cornerOffset] -fill white -tag line

    ## line from middle to lower left
    $c create line $xCenter $yCenter [expr $xCenter - $cornerOffset] [expr $yCenter + $cornerOffset] -fill white -tag line

    ## line from middle to upper right
    $c create line $xCenter $yCenter [expr $xCenter + $cornerOffset] [expr $yCenter - $cornerOffset] -fill white -tag line

    ## line from middle to lower right
    $c create line $xCenter $yCenter [expr $xCenter + $cornerOffset] [expr $yCenter + $cornerOffset] -fill white -tag line



    ##### board locations

    #    3
    # 2  |  4
    #  \   /
    #1-- 0 --5
    #  / | \
    # 8  |  6
    #    7


    for {set x 0} {$x < 9} {set x [expr $x + 1]} {
	drawPiece $c $x "blue"
	drawPiece $c $x "red"
	drawPiece $c $x "white"	
    }

#drawArrow $c $newBlankSpot $oldBlankSpot $color

    for {set y 1} {$y < 9} {set y [expr $y + 1]} {
	drawArrow $c 0 $y "cyan"
	drawArrow $c $y 0 "cyan"

	drawArrow $c 0 $y "yellow"
	drawArrow $c $y 0 "yellow"

	drawArrow $c 0 $y "green"
	drawArrow $c $y 0 "green"

	drawArrow $c 0 $y "red4"
	drawArrow $c $y 0 "red4"
    }


    for {set z 1} {$z < 9} {set z [expr $z + 1]} {
	set cwNeighbor [expr [expr $z + 1] % 8]
	set ccwNeighbor [expr [expr $z - 1] % 8]

	if {$cwNeighbor == 0} {
	    set cwNeighbor 8
	}

	if {$ccwNeighbor == 0} {
	    set ccwNeighbor 8
	}	


	drawArrow $c $z $cwNeighbor "cyan"
	drawArrow $c $z $ccwNeighbor "cyan"


	drawArrow $c $z $cwNeighbor "yellow"
	drawArrow $c $z $ccwNeighbor "yellow"


	drawArrow $c $z $cwNeighbor "green"
	drawArrow $c $z $ccwNeighbor "green"


	drawArrow $c $z $cwNeighbor "red4"
	drawArrow $c $z $ccwNeighbor "red4"


    }


    $c raise bgGreyCircle
    $c raise bgCircle
    $c raise line
    $c raise bgMiniCircle


    # draws the starting position with
    # 0000111100000
    # white0
    # blue1
    # blue2
    # blue3
    # blue4
    # red5
    # red6
    # red7
    # red8

    #GS_DrawPosition $c $gInitialPosition
#     $c raise blue1
#     $c raise blue2
#     $c raise blue3
#     $c raise blue4

#     $c raise red5
#     $c raise red6
#     $c raise red7
#     $c raise red8


    #$c create text 250 480 -text $gInitialPosition

}





    ##### board locations

    #    3
    # 2  |  4
    #  \   /
    #1-- 0 --5
    #  / | \
    # 8  |  6
    #    7

## getPiecePositionX takes in a board location (0-8) and returns the x-coordinate of the board location.
proc getPiecePositionX { boardLocation } {

    global xCenter yCenter
    global smallCircleDiam largeCircleDiam pieceCircleDiam largeCircleRadius cornerOffset


    if {$boardLocation == 0} {
	return $xCenter
    } elseif {$boardLocation == 1} {
	return [expr $xCenter - $largeCircleRadius]
    } elseif {$boardLocation == 2} {
	return [expr $xCenter - $cornerOffset]
    } elseif {$boardLocation == 3} {
	return $xCenter
    } elseif {$boardLocation == 4} {
	return [expr $xCenter + $cornerOffset]
    } elseif {$boardLocation == 5} {
	return [expr $xCenter + $largeCircleRadius]
    } elseif {$boardLocation == 6} {
	return [expr $xCenter + $cornerOffset]
    } elseif {$boardLocation == 7} {
	return $xCenter
    } elseif {$boardLocation == 8} {
	return [expr $xCenter - $cornerOffset]
    } else {
	#ERROR
    }
}



## getPIecePositionY takes in a board location (0-8) and returns the y-coordinate of the board location.
proc getPiecePositionY { boardLocation } {

    global xCenter yCenter
    global smallCircleDiam largeCircleDiam pieceCircleDiam largeCircleRadius cornerOffset


    if {$boardLocation == 0} {
	return $yCenter
    } elseif {$boardLocation == 1} {
	return $yCenter
    } elseif {$boardLocation == 2} {
	return [expr $yCenter - $cornerOffset]
    } elseif {$boardLocation == 3} {
	return [expr $yCenter - $largeCircleRadius]
    } elseif {$boardLocation == 4} {
	return [expr $yCenter - $cornerOffset]
    } elseif {$boardLocation == 5} {
	return $yCenter
    } elseif {$boardLocation == 6} {
	return [expr $yCenter + $cornerOffset]
    } elseif {$boardLocation == 7} {
	return [expr $yCenter + $largeCircleRadius]
    } elseif {$boardLocation == 8} {
	return [expr $yCenter + $cornerOffset]
    } else {
	#ERROR
    }
}



## drawOval draws an oval on the canvas given its x and y coordinates, size, fill and outline color and tag
proc drawOval { c x y size fill outline tag } {
    set radius [expr $size / 2]
    $c create oval [expr $x - $radius] [expr $y - $radius] [expr $x + $radius] [expr $y + $radius] -fill $fill -outline white -tag $tag
}


## drawPiece draws an piece on the canvas at the board location
proc drawPiece { c boardLocation color } {
    global smallCircleDiam largeCircleDiam pieceCircleDiam largeCircleRadius cornerOffset

    drawOval $c [getPiecePositionX $boardLocation] [getPiecePositionY $boardLocation] $pieceCircleDiam $color $color "$color$boardLocation"
}





#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}


#############################################################################
# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.
#
# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {
    
    $c raise bgGreyCircle
    $c raise bgCircle
    $c raise line
    $c raise bgMiniCircle

    ##### board locations

    #    3
    # 2  |  4
    #  \   /
    #1-- 0 --5
    #  / | \
    # 8  |  6
    #    7

    # hash on 13 digit binary
    # right most 9 digits specify board location and color
    # for the right most 9 digits 0 is blue, 1 is red
    # left most 4 digits specify blank spot (must equal a value 0-8)

    # example
    # 0011011001011
    # right most 9 = 011001011
    # from right to left location 0 is red
    #                             1 is red
    #                             2 is blue
    #                             3 is red
    #                             4 is blue
    #                             5 is blue
    #                             6 is red
    #                             7 is red
    #                             8 is blue
    # notice there are 5 red pieces which is incorrect so we must overwrite one
    # of those 5 pieces to be blank
    #
    # left most 4 digits = 0011 = 3 in base 10
    # so the blank space is 3 and should overwrite the red piece from above




    #set mask 1

    #set binaryPosition [Unhash $position]

#    $c create text 250 250 -text [C_GenericUnhash $position 9]
    #3uts "C_GenericUnhash"
    #puts [C_GenericUnhash $position 9]

    
#    $c create text 20 20 -text $binaryPosition

    set unhashedString [C_GenericUnhash $position 9]

    for {set x 0} {$x < 9} {set x [expr $x + 1]} {
	

	if {[string compare [string index $unhashedString $x] "x"] == 0} {
	    $c raise "blue$x"
	} elseif {[string compare [string index $unhashedString $x] "o"] == 0} {
	    $c raise "red$x"
	} elseif {[string compare [string index $unhashedString $x] "_"] == 0} {
	    #do nothing
	    $c raise "white$x"
	}
    }


#    set blankSpot [getBlankSpot $unhashedString]
#    $c raise "white$blankSpot"

}

#unhashes the position from the C code with C_GenericUnhash. Returns the unhashed position.
proc Unhash { position } {

    #set stringPosition ""
    set stringPosition [C_GenericUnhash $position 9]
    #set pieceString [string range [C_GenericUnhash $position 9] 0 8]
    #set stringPosition "_ooooxxxx"


#     for {set x 9} {$x >= 0} {set x [expr $x - 1]} {
# 	append stringPosition [string index $stringPositionR $x]
#     }



    set binaryPosition 0
    set positionMultiplier 1

    for {set n 0} {$n < 9} {set n [expr $n + 1]} {
	if {[string compare [string index $stringPosition $n] "x"] == 0} {
	    #The thing in the string is "x", which is red in our game. Do something in binary.
	    set binaryPosition [expr $binaryPosition + $positionMultiplier]
	} elseif {[string compare [string index $stringPosition $n] "o"] == 0} {
	    #The thing in the string is "o", which is blue in our game. Do something in binary.
	    #set binaryPosition [expr $binaryPosition + $positionMultiplier]
	} elseif {[string compare [string index $stringPosition $n] "_"] == 0} {
	    #the thing in the string is "_", which is the empty slot.  Do something in binary.
	    set binaryPosition [expr $binaryPosition + { 1024 * $n }]
	}

	set positionMultiplier [expr $positionMultiplier * 2]
    }



    return $binaryPosition
}



# Gets the blank location of a position
proc getBlankSpot { position } {
    
    set stringPosition [C_GenericUnhash $position 9]

    for {set x 0} {$x < 9} {set x [expr $x + 1]} {
	if {[string compare [string index $stringPosition $x] "_"] == 0} {
	    return $x
	}
    }
}





#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation

    #GS_DrawPosition $c $position
    #$c create text 250 5 -text $position
    $c delete gameover
    GS_DrawPosition $c $position
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    # Optional Procedure
    return ""
}


#############################################################################
# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"
#
# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.
#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {

    GS_DrawPosition $c $newPosition
    
}


#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {


    #$c create text 20 480 -text [C_GenerateMoves position]


    #$c create text 480 20 -text $position

    #set binaryPosition [Unhash $position]
    #set oldBlankSpot [getBlankSpot $binaryPosition]
    #set oldBlankSpot [getBlankSpot $position]

    #$c create text 250 250 -text $moveList
    #$c create text 250 20 -text [C_GenericUnhash $position 9]

    set oldBlankSpot [getBlankSpot $position]

    foreach item $moveList {
	set move [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	
	set newBlankSpot $move

	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } elseif {$value == "Win"} {
		set color red4
	    } else {
		#ERROR
	    }
	}

	raiseArrow $c $newBlankSpot $oldBlankSpot $position $move $color
    }
}



# draws an arrow and binds the mouse enter and exit to animate the big arrow when mouse is entered
proc drawArrow {c startLoc endLoc color} {
##### arrow
    $c create line [getPiecePositionX $startLoc] [getPiecePositionY $startLoc] [getPiecePositionX $endLoc] [getPiecePositionY $endLoc] -width 15 -arrow last -arrowshape {30 30 15} -fill $color -tag "arrow$color$startLoc$endLoc"

    $c create line [getPiecePositionX $startLoc] [getPiecePositionY $startLoc] [getPiecePositionX $endLoc] [getPiecePositionY $endLoc] -width 30 -arrow last -arrowshape {60 60 30} -fill $color -tag "bigArrow$color$startLoc$endLoc"


    $c bind "arrow$color$startLoc$endLoc" <Enter> "$c raise bigArrow$color$startLoc$endLoc"
    $c bind "bigArrow$color$startLoc$endLoc" <Leave> "$c lower bigArrow$color$startLoc$endLoc"



}


# raises an arrow and binds it to a button release that handles that move
proc raiseArrow {c startLoc endLoc oldPosition move color} {
    $c raise "arrow$color$startLoc$endLoc"

    $c bind "arrow$color$startLoc$endLoc" <ButtonRelease> "ReturnFromHumanMove $move"
    $c bind "bigArrow$color$startLoc$endLoc" <ButtonRelease> "ReturnFromHumanMove $move"
}




#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    set oldBlankSpot [getBlankSpot $position]

    foreach item $moveList {
	set move [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	set newBlankSpot [getBlankSpot $move]

	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } elseif {$value == "Win"} {
		set color red4
	    } else {
		#ERROR
	    }
	}

	$c lower "arrow$color$newBlankSpot$oldBlankSpot"
	$c lower "bigArrow$color$newBlankSpot$oldBlankSpot"
    }

}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {

    ### TODO if needed
    GS_DrawPosition $c $positionAfterUndo
}


#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {
}

#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
    
    global cWidth
    set size $cWidth
    set fontsize [expr int($size / 20)]
    
    # Tell us it's "Game Over!" and announce and winner
    $c create rectangle 0 [expr $size/2 - 50] $size [expr $size/2 + 50] -fill gray -width 1 -outline black -tag "gameover"
    $c create text [expr $size/2] [expr $size/2] -text "Game Over! $nameOfWinner Wins" -font "Arial $fontsize" -fill black -tag "gameover"
	
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is finished.
# This is provided so that you may undo the drawing you did in GS_GameOver if you 
# drew something.
# For instance, if you drew a line crossing out the winning row in tic tac toe, 
# this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

    # Delete "Game Over!" text
    $c delete gameover
}
