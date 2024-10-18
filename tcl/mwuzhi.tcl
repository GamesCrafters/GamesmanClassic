
####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################
# Name:     mwuzhi.tcl
# Authors:  Diana Fang, Dachuan Yan
# Update History:
# 2005-3-25 -added animation for move in HandleMoves
#           -added movePiece procedure
#           -fixed "red" and "blue" duplicate arrow problem
#           -ungrouped pieces and arrows
#           -fixed off center board by adding "offset" variable
#           -fixed arrows appearing when "values" is checked after game ends
#            by adding "gGameover" global variable and if-statement in ShowMoves
#           -added Diagonals option
# 2005-3-26 -changed boardsize to gBoardSize
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
    set kGameName "Wuzhi"

    global gBoardSize gBoardSizeOp
    set gBoardSize [expr $gBoardSizeOp + 3]

    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition

    if {$gBoardSize == 3} {
	set gInitialPosition 19
    } elseif {$gBoardSize == 4} {
	set gInitialPosition 494
    } elseif {$gBoardSize == 5} {
	set gInitialPosition 15503
    }

    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to capture all but one of their opponents pieces WINS"
    set kMisereString "First player to capture all but one of their opponents pieces LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gDiagonalsOption diagonals
    set diagonals $gDiagonalsOption

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Capture all but one of your opponenet's piece or if your opponent has no moves left. A capture occurs when you move two of your pieces inline with an opponent's piece"
    } else {
	SetToWinString "To Win: The Person with one piece left or the person with no moves left wins."
    }
    SetToMoveString "To Move: click on the arrow of the piece that you want 
to move in the direction that you want to move."

    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Diana Fang and Dan Yan"
    set kTclAuthors "Dan Yan and Diana Fang"
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

    return [list w b]

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

    set diagonalsRule \
	[list \
	     "Would you like to play with diagonal movement?" \
	     "No Diagonals" \
	     "Diagonals"
	 ]

    set boardsizeRule \
	[list \
	     "What size board would you like to play on?" \
	     "3x3" \
	     "4x4" \
	     "5x5"
	]

    # List of all rules, in some order
    set ruleset [list $standardRule $diagonalsRule $boardsizeRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gDiagonalsOption
    set gDiagonalsOption 0

    global gBoardSizeOp
    set gBoardSizeOp 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gDiagonalsOption" "gBoardSizeOp"]

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
    global gMisereGame gDiagonalsOption gBoardSizeOp
    if {$gMisereGame} {
	set option 0
    } else { 
	set option 1
    }
    set option [expr $option + ($gDiagonalsOption << 1)]
    set option [expr $option + ($gBoardSizeOp << 2)]

    return $option
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
    global gMisereGame gDiagonalsOption diagonals gBoardSize
    set gMisereGame [expr $option & 1]
    if {$gMisereGame} {
	set gMisereGame 0
    } else { 
	set gMisereGame 1
    }
    set diagonals gDiagonalsOption
    set gBoardSize [expr ($option >> 2) + 3]
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

    global gBoardSize gGameover dist gDiagonalsOption gFrameWidth gFrameHeight r arrowshape arrowwidth canvasWidth
    set gGameover 0
    set numofBlue 0
    set numofRed 0
    set numofArrowSet 0
    set canvasWidth [min $gFrameWidth $gFrameHeight]
    set margin [expr 0.2 * $canvasWidth]
    set dist [expr ($canvasWidth - 2 * $margin) / [expr $gBoardSize - 1]]
    set r [expr 0.25 * $dist]
    set offset 0
    set boardwidth [expr $gBoardSize * $dist]
    set dot [expr 0.25 * $r]
    set arrowwidth [expr 1.5 * $dot]
    set arrowshape [list [expr 2 * $arrowwidth] [expr 2 * $arrowwidth] $arrowwidth]
    set linewidth [expr 0.5 * $dot]
    pack $c

    # creates a base which to hide the arrows
    $c create rectangle 0 0 $canvasWidth $canvasWidth -fill white -tags base

    # board grid
    for {set i 0} {$i < $gBoardSize} {incr i} {
	$c create line $margin [expr $i * $dist + $margin] [expr $canvasWidth - $margin] [expr $i * $dist + $margin] -width $linewidth -fill black -tags base
	$c create line [expr $i * $dist + $margin] $margin [expr $i * $dist + $margin] [expr $canvasWidth - $margin] -width $linewidth -fill black -tags base
    }

    # diagonals
    for {set i 0} {$i < [expr $gBoardSize - 1]} {incr i} {
	for {set j 0} {$j < [expr $gBoardSize - 1]} {incr j} {
	    $c create line [expr $margin + $j * $dist] [expr $margin + $i * $dist] [expr $margin + ($j + 1) * $dist] [expr $margin + ($i + 1) * $dist] -width $linewidth -tags [list base diag]
	    $c create line [expr $margin + ($j + 1) * $dist] [expr $margin + $i * $dist] [expr $margin + $j * $dist] [expr $margin + ($i + 1) * $dist] -width $linewidth -tags [list base diag]
	}
    }

    # lower diagonals
    if ($gDiagonalsOption) {
	$c raise diag base
    } else {
	$c lower diag
    }

    # draws all possible pieces and arrows and lowers them
    for {set j 0} {$j< $gBoardSize} {incr j} {
	for {set i 0} {$i < $gBoardSize} {incr i} {
	    # draws dots at vertices of board
	    $c create oval [expr $margin + $i * $dist - $dot] [expr $margin + $j * $dist - $dot] \
		[expr $margin + $i * $dist + $dot] [expr $margin + $j * $dist + $dot] -fill black -tags base

	    # draws all possible arrows
	    drawArrows $c [expr $margin + $i * $dist] [expr $margin + $j * $dist] $numofArrowSet
	    set numofArrowSet [expr $numofArrowSet + 1]

	    # draws all possible blue pieces
	    drawPiece $c [expr $margin + $i * $dist] [expr $margin + $j * $dist] blue $numofBlue
	    $c lower blue$numofBlue
	    set numofBlue [expr $numofBlue + 1]

	    # draws all possible red pieces
	    drawPiece $c [expr $margin + $i * $dist] [expr $margin + $j * $dist] red $numofRed
	    $c lower red$numofRed
	    set numofRed [expr $numofRed + 1]
	}
    }

    # lowers all the arrows
    $c lower arrow

    # raise starting red pieces
    for {set r 0} {$r < $gBoardSize} {set r [expr $r + 1]} {
	$c raise red$r
    }

    # raise starting blue pieces
    for {set b [expr $gBoardSize*$gBoardSize - $gBoardSize]} {$b < [expr $gBoardSize*$gBoardSize]} {set b [expr $b + 1]} {
	$c raise blue$b
    }

    #set settingup 0
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

    global gBoardSize pieceString
    set pieceString [string range [C_GenericUnhash $position [expr $gBoardSize * $gBoardSize]] 0 [expr $gBoardSize*$gBoardSize-1]]

    # resets board
    $c raise base

    # raises appropriate pieces
    for {set i 0} {$i < [expr $gBoardSize * $gBoardSize]} {set i [expr $i + 1]} {
	if {[string compare [string index $pieceString $i] "w"] == 0} {
	    $c raise blue$i
	} elseif {[string compare [string index $pieceString $i] "b"] == 0} {
	    $c raise red$i
	} else {}
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

    # TODO: The default behavior of this funciton is just to draw the xposition
    # but if you want you can add a special behaivior here like an animation

    global gGameover
    set gGameover 0

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
    # incr determines the animation speed
    global incr gAnimationSpeed dist

    set speed [expr 100 / pow(2, $gAnimationSpeed)]

    # If things get too fast, just make it instant
    if {$speed < 10} {
	set speed 10
    }

    set incr [expr $dist / $speed]
    #figure out which piece to move and which direction
    set arrayNum [getArraynum [GetXCoord $theMove] [GetYCoord $theMove]]
    set dir [GetDirection $theMove]
    #figure out whose piece it is
    global gBoardSize pieceString
    set pieceString [string range [C_GenericUnhash $oldPosition [expr $gBoardSize * $gBoardSize]] 0 [expr $gBoardSize*$gBoardSize-1]]

    if {[string compare [string index $pieceString $arrayNum] "w"] == 0} {
	set pieceToMove blue$arrayNum
    } elseif {[string compare [string index $pieceString $arrayNum] "b"] == 0} {
	set pieceToMove red$arrayNum
    }

    #start moving the piece in the direction
    movePiece $c $pieceToMove $dir $incr

    #execute the move which takes care of any pieces that are captured
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

    global gBoardSize boardstring
    global gGameover

    # do not show moves if game is over, 0 means not over
    if {$gGameover == 0} {

	foreach item $moveList {
	    set move [lindex $item 0]
	    set value [lindex $item 1]
	    set color cyan

	    if {$moveType == "value"} {
		if {$value == "Tie"} {
		    set color yellow
		} elseif {$value == "Lose"} {
		    set color green
		} else {
		    set color red4
		}
	    }

	    set arrayNum [getArraynum [GetXCoord $move] [GetYCoord $move]]
	    set dir [GetDirection $move]

	    switch $dir {
		0 {
		    $c raise arrowUP$arrayNum base
		    $c itemconfig arrowUP$arrayNum -fill $color
		    $c bind arrowUP$arrayNum <Enter> "$c itemconfig arrowUP$arrayNum -fill black"
		    $c bind arrowUP$arrayNum <Leave> "$c itemconfig arrowUP$arrayNum -fill $color"
		    $c bind arrowUP$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"

		}
		1 {
		    $c raise arrowRIGHT$arrayNum base
		    $c itemconfig arrowRIGHT$arrayNum -fill $color
		    $c bind arrowRIGHT$arrayNum <Enter> "$c itemconfig arrowRIGHT$arrayNum -fill black"
		    $c bind arrowRIGHT$arrayNum <Leave> "$c itemconfig arrowRIGHT$arrayNum -fill $color"
		    $c bind arrowRIGHT$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		2 {
		    $c raise arrowDOWN$arrayNum base
		    $c itemconfig arrowDOWN$arrayNum -fill $color
		    $c bind arrowDOWN$arrayNum <Enter> "$c itemconfig arrowDOWN$arrayNum -fill black"
		    $c bind arrowDOWN$arrayNum <Leave> "$c itemconfig arrowDOWN$arrayNum -fill $color"
		    $c bind arrowDOWN$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		3 {
		    $c raise arrowLEFT$arrayNum base
		    $c itemconfig arrowLEFT$arrayNum -fill $color
		    $c bind arrowLEFT$arrayNum <Enter> "$c itemconfig arrowLEFT$arrayNum -fill black"
		    $c bind arrowLEFT$arrayNum <Leave> "$c itemconfig arrowLEFT$arrayNum -fill $color"
		    $c bind arrowLEFT$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		4 {
		    $c raise arrowUPLEFT$arrayNum base
		    $c itemconfig arrowUPLEFT$arrayNum -fill $color
		    $c bind arrowUPLEFT$arrayNum <Enter> "$c itemconfig arrowUPLEFT$arrayNum -fill black"
		    $c bind arrowUPLEFT$arrayNum <Leave> "$c itemconfig arrowUPLEFT$arrayNum -fill $color"
		    $c bind arrowUPLEFT$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		5 {
		    $c raise arrowUPRIGHT$arrayNum base
		    $c itemconfig arrowUPRIGHT$arrayNum -fill $color
		    $c bind arrowUPRIGHT$arrayNum <Enter> "$c itemconfig arrowUPRIGHT$arrayNum -fill black"
		    $c bind arrowUPRIGHT$arrayNum <Leave> "$c itemconfig arrowUPRIGHT$arrayNum -fill $color"
		    $c bind arrowUPRIGHT$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		6 {
		    $c raise arrowDOWNRIGHT$arrayNum base
		    $c itemconfig arrowDOWNRIGHT$arrayNum -fill $color
		    $c bind arrowDOWNRIGHT$arrayNum <Enter> "$c itemconfig arrowDOWNRIGHT$arrayNum -fill black"
		    $c bind arrowDOWNRIGHT$arrayNum <Leave> "$c itemconfig arrowDOWNRIGHT$arrayNum -fill $color"
		    $c bind arrowDOWNRIGHT$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		7 {
		    $c raise arrowDOWNLEFT$arrayNum base
		    $c itemconfig arrowDOWNLEFT$arrayNum -fill $color
		    $c bind arrowDOWNLEFT$arrayNum <Enter> "$c itemconfig arrowDOWNLEFT$arrayNum -fill black"
		    $c bind arrowDOWNLEFT$arrayNum <Leave> "$c itemconfig arrowDOWNLEFT$arrayNum -fill $color"
		    $c bind arrowDOWNLEFT$arrayNum <ButtonRelease-1> "ReturnFromHumanMove $move"
		}
		default {}
	    }
	}
    update idletasks
    }
}



#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
    $c lower arrow base
    update idletasks

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

    # incr determines the animation speed
    global incr gAnimationSpeed dist gBoardSize

    set speed [expr 100 / pow(2, $gAnimationSpeed)]

    # If things get too fast, just make it instant
    if {$speed < 10} {
	set speed 10
    }

    set incr [expr $dist / $speed]

    # Let's find where the piece went to (our new starting point). Compare
    # currentPosition and positionAfterUndo to see where there is a value in
    # currentPosition that isn't there in positionAfterUndo.
    set currentboard [string range [C_GenericUnhash $currentPosition [expr $gBoardSize * $gBoardSize]] 0 [expr $gBoardSize * $gBoardSize - 1]]
    set boardafter [string range [C_GenericUnhash $positionAfterUndo [expr $gBoardSize * $gBoardSize]] 0 [expr $gBoardSize * $gBoardSize - 1]]

    for { set i 0 } { $i < [expr $gBoardSize * $gBoardSize] } { incr i } {
	set temp [string index $currentboard $i]

	if { [string compare $temp "w"] == 0 && [string compare [string index $boardafter $i] " "] == 0 } {
	    set pieceToMove blue$i
	}
	if { [string compare $temp "b"] == 0 && [string compare [string index $boardafter $i] " "] == 0 } {
	    set pieceToMove red$i
	}
    }

    set dir [GetDirection $theMoveToUndo]

    # Flip dir to the opposite direction
    switch $dir {
	0 { set dir 2 }
	1 { set dir 3 }
	2 { set dir 0 }
	3 { set dir 1 }
	4 { set dir 6 }
	5 { set dir 7 }
	6 { set dir 4 }
	7 { set dir 5 }
    }
    
    #start moving the piece in the direction
    movePiece $c $pieceToMove $dir $incr

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
    
    global canvasWidth
    set size $canvasWidth
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



######################Helper Stuff###########################
proc drawPiece {c x0 y0 color num} {

    global r

    $c create oval [expr $x0 - $r] [expr $y0 - $r] \
	[expr $x0 + $r] [expr $y0 + $r] \
	-fill $color -tags $color$num -width 4

}
proc drawArrows {c x y piece} {

    global dist arrowshape arrowwidth

    set arrowLen [expr 0.7 * $dist]
    set arrowLenD [expr 0.7 * $dist]
    #up
    $c create line $x $y $x [expr $y - $arrowLen] \
    -width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
    -tags [list arrowUP$piece arrow$piece arrow]

    #down
    $c create line $x $y $x [expr $y + $arrowLen] \
    -width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
    -tags [list arrowDOWN$piece arrow$piece arrow]

    #left
    $c create line $x $y [expr $x - $arrowLen] $y \
    -width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
    -tags [list arrowLEFT$piece arrow$piece arrow]

    #right
    $c create line $x $y [expr $x + $arrowLen] $y \
    -width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
    -tags [list arrowRIGHT$piece arrow$piece arrow]

    #diagonal arrows
    #upleft
    $c create line $x $y [expr $x - $arrowLenD] [expr $y - $arrowLenD] \
	-width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
	-tags [list arrowUPLEFT$piece arrow$piece arrow diagArrow]
    #upright
    $c create line $x $y [expr $x + $arrowLenD] [expr $y - $arrowLenD] \
	-width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
	-tags [list arrowUPRIGHT$piece arrow$piece arrow diagArrow]
    #downright
    $c create line $x $y [expr $x + $arrowLenD] [expr $y + $arrowLenD] \
	-width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
	-tags [list arrowDOWNRIGHT$piece arrow$piece arrow diagArrow]
    #downleft
    $c create line $x $y [expr $x - $arrowLenD] [expr $y + $arrowLenD] \
	-width $arrowwidth -arrow last -arrowshape $arrowshape -fill lightblue \
	-tags [list arrowDOWNLEFT$piece arrow$piece arrow diagArrow]

}


# Move Stuff (Hashers, Unhashers, etc)
proc GetDirection {theMove} {
    return [expr $theMove >> 6]
}

proc GetXCoord {theMove} {
    return [expr ($theMove >> 3) & 0x7]
}

proc GetYCoord {theMove} {
    return [expr $theMove & 0x7]
}


proc getArraynum {xcoord ycoord} {
global gBoardSize
    return [expr ($gBoardSize *($gBoardSize - $ycoord)) - ($gBoardSize - $xcoord)]
}

# animates movement of piece
# @c           : canvas
# @pieceToMove : piece to be moved
# @dir         : direction of movement
# @incr        : controls move speed / move amount per while loop
proc movePiece { c pieceToMove dir incr } {
    global dist
    set tmpDist 0
    set origcoords [$c coords $pieceToMove]

    switch $dir {
	0 {
	    set xAmount 0
	    set yAmount [expr 0 - $incr]
	}
	1 {
	    set xAmount $incr
	    set yAmount 0
	}
	2 {
	    set xAmount 0
	    set yAmount $incr
	}
	3 {
	    set xAmount [expr 0 - $incr]
	    set yAmount 0
	}
	4 {
	    set xAmount [expr 0 - $incr]
	    set yAmount [expr 0 - $incr]
	}
	5 {
	    set xAmount [expr 0 + $incr]
	    set yAmount [expr 0 - $incr]
	}
	6 {
	    set xAmount [expr 0 + $incr]
	    set yAmount [expr 0 + $incr]
	}
	7 {
	    set xAmount [expr 0 - $incr]
	    set yAmount [expr 0 + $incr]
	}
	default {}
    }

    while {$tmpDist < $dist} {
	# move piece by incr amount
	$c move $pieceToMove $xAmount $yAmount
	# increment tmpDist by incr amount
	set tmpDist [expr $tmpDist + $incr]
	update idletasks
    }

    # reset piece
    $c lower $pieceToMove

    $c coords $pieceToMove $origcoords

}



