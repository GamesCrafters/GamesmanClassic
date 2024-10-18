####################################################
# Othello
#
# Based upon the template for tcl module creation
# and Xmnim
#
# Authors: Keaton Mowery and Victor Perez
# Written Fall 2005
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
    set kGameName "Othello"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition

    #These are set in GS_SetupRulesFrame
    global gRowsOption gColsOption

    global gMinRows gMinCols
    set gMinRows 3
    set gMinCols 3

    ### Set gBoardRows, gBoardCols, boardSize
    global gBoardRows gBoardCols boardSize
    set gBoardRows [expr $gRowsOption + $gMinRows]
    set gBoardCols [expr $gColsOption + $gMinCols]
    set boardSize [expr $gBoardRows * $gBoardCols]

    global gForceCapture
    set gForceCapture 0

    global pi
    set pi 3.14159265

    #Percentage of the canvas that should be used for the score bar

    global gScoreBarHeight
    set gScoreBarHeight .025

    ### Set the strings to be used in the Edit Rules

    global gMisereGame
    set gMisereGame 0
    if {!$gMisereGame} {
	SetToWinString "To Win: Have the most number of pieces of your color when the board is filled."
    } else {
	SetToWinString "To Win: Have the least number of pieces of your color when the board is filled."
    }
    SetToMoveString "To Move: Players take turns placing a piece on the board.  A capture is made when a piece is placed so that it and another piece of the same color sandwich one or more oppositely colored pieces.  All of the sandwiched pieces then switch to the player's color.  Captures can be made vertically, horizontally, and diagonally."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Michael Chen, Robert Liao"
    set kTclAuthors "Keaton Mowery, Victor Perez"
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

    return [list black white]
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

    return [list black white]
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

    set captureRule \
	[list \
	     "Should capture moves be mandatory?" \
	     "Yes" \
	     "No" \
	    ]

    set autoPassRule \
	[list \
	     "Should you pass automatically if there are no moves available?" \
	     "Yes" \
	     "No" \
	    ]

    set gBoardColsRule \
	[list \
	     "Board Columns:" \
	     "3" \
	     "4" \
	    ]

    set gBoardRowsRule \
	[list \
	     "Board Rows:" \
	     "3" \
	     "4" \
	    ]


    # List of all rules, in some order
    set ruleset [list $captureRule $autoPassRule $gBoardColsRule $gBoardRowsRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gForcedCapture
    set gForcedCapture 0

    global gNoAutoPass
    set gNoAutoPass 1

    global gBoardCols gBoardRows

    global gRowsOption gColsOption
    set gRowsOption 1
    set gColsOption 1

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gForcedCapture" "gNoAutoPass" "gColsOption" "gRowsOption"]

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
    global gMisereGame gForcedCapture
    global gBoardCols gBoardRows
    global gMinCols gMinRows
    global gColsOption gRowsOption

    set gBoardCols [expr $gMinCols + $gColsOption]
    set gBoardRows [expr $gMinRows + $gRowsOption]

    set option [expr [expr $gBoardCols << 5] + [expr $gBoardRows << 1] + $gForcedCapture]

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

    global gForcedCapture gBoardCols gBoardRows

    set gForcedCapture [expr $option & 0x1]
    set gBoardRows [expr [expr $option >> 0x01] & 0x0f]
    set gBoardCols [expr $option  >> 5]

}


#############################################################################
# DrawCircle draws a piece-sied circle on (slotX,slotY) in window w 
# with a tag of theTag.
#############################################################################
proc DrawCircle { w slotSize slotX slotY theTag theColor } {
    global xBoardOffset yBoardOffset

    set startCircle [expr $slotSize / 8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$slotSize + $xBoardOffset]
    set cornerY     [expr $slotY*$slotSize + $yBoardOffset] 
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -fill $theColor \
		       -outline $theColor \
		       -tag $theTag]


    $w move $theCircle $cornerX $cornerY

    $w addtag piece$slotX$slotY withtag $theCircle

    return $theCircle
}

#############################################################################
# DrawMoveCircle draws a small circle that represents a move on (slotX,slotY)
#  in the window w and adds the tag theTag
#############################################################################

proc DrawMoveCircle { w slotSize slotX slotY theTag theColor } {
    global xBoardOffset yBoardOffset
    
    set startCircle [expr $slotSize / 3.0]
    set endCircle   [expr $startCircle*2.0]
    set cornerX     [expr $slotX*$slotSize + $xBoardOffset]
    set cornerY     [expr $slotY*$slotSize + $yBoardOffset]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -fill $theColor \
		       -outline $theColor \
		       -tag $theTag]

    $w move $theCircle $cornerX $cornerY

    $w addtag movedot$slotX$slotY withtag $theCircle

    return $theCircle
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
    global gBoardRows gBoardCols
    global gFrameWidth gFrameHeight
    global cellSize
    global xBoardOffset yBoardOffset

    #Save the background so we don't delete it later...
    $c addtag background all

    set cellSize [GetCellSize]

    set xBoardOffset [GetBoardXOffset]
    set yBoardOffset [GetBoardYOffset]

    for {set x 0} {$x < $gBoardCols} {incr x} {
	for {set y 0} {$y < $gBoardRows} {incr y} {
	    
	    $c create rectangle [expr $x * $cellSize + $xBoardOffset] [expr $y * $cellSize + $yBoardOffset] \
		[expr ($x + 1) * $cellSize + $xBoardOffset] [expr ($y + 1) * $cellSize + $yBoardOffset] \
		-fill darkgreen -outline black -width 2 -tag base 
	}
    }

    for {set x 0} {$x < $gBoardCols} {incr x} {
	for {set y 0} {$y < $gBoardRows} {incr y} {

	    DrawCircle $c $cellSize $x $y pieces [lindex [GS_ColorOfPlayers] 0]
	    DrawMoveCircle $c $cellSize $x $y moves cyan
	}
    }

    $c create text [expr $gFrameWidth / 2] [expr $gFrameHeight/2] -width [expr $gFrameWidth * .9] \
	-font {Helvetica 32 bold} -fill blue -text "No moves available.  Click here to pass." \
	-tag NoMovesText

    DrawScoreBar $c 2 2

    $c raise base
    $c raise scorebar
}    


#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete {!background}
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
    global gBoardCols gBoardRows boardSize

    $c lower pieces
    $c lower moves
    $c raise base

    set pieceString [string range [C_GenericUnhash $position $boardSize] 0 [expr $boardSize-1]]
    set pieceNumber 0

    set whitePieces 0
    set blackPieces 0

    for {set i 0} {$i < $gBoardRows} {set i [expr $i + 1]} {
	for {set j 0} {$j < $gBoardCols} {set j [expr $j + 1]} {

	    if {[string compare [string index $pieceString $pieceNumber] "W"] == 0} {
		$c itemconfig piece$j$i -fill white -outline white
		$c raise piece$j$i
		
		set whitePieces [expr $whitePieces + 1]
	    } elseif {[string compare [string index $pieceString $pieceNumber] "B"] == 0} {
		$c itemconfig piece$j$i -fill black -outline black
		$c raise piece$j$i

		set blackPieces [expr $blackPieces + 1]
	    } else {}

	    set pieceNumber [expr $pieceNumber + 1]
	}
    }

    DrawScoreBar $c $blackPieces $whitePieces
    $c raise scorebar
}


#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    global gBoardRows gBoardCols

    GS_Deinitialize $c
    GS_Initialize $c

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
    
    AnimateMove $c $oldPosition $newPosition
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
    global gNoAutoPass

    $c lower moves

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
		set color darkred
	    }
	}
	
	if { $move != -1 } {
	    #we get -1 when there are no moves available

	    set movetag movedot[GetXYFromMove $item]
	    
	    $c raise $movetag
	    $c itemconfig $movetag -fill $color -outline $color
	    
	    $c bind $movetag <ButtonRelease-1> "ReturnFromHumanMove $move"
	    $c bind $movetag <Enter> "$c itemconfig $movetag -fill black -outline black"
	    $c bind $movetag <Leave> "$c itemconfig $movetag -fill $color -outline $color"

	} else {
	    if { $gNoAutoPass } {
		# If we want to show the pass text, raise and bind it
		$c raise NoMovesText		

		$c bind NoMovesText <ButtonRelease-1> "ReturnFromHumanMove $move"
		$c bind NoMovesText <Enter> "$c itemconfig NoMovesText -fill darkblue"
		$c bind NoMovesText <Leave> "$c itemconfig NoMovesText -fill blue"
	    } else {
		# Otherwise, just move on
		ReturnFromHumanMove $move
	    }

	}
    }
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    $c lower moves
    $c lower NoMovesText
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

    AnimateMove $c $currentPosition $positionAfterUndo
}


#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {

}


#############################################################################
# GS_GameOver is called the moment the game is finished (won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
    global gFrameWidth gFrameHeight

    $c create text [expr $gFrameWidth / 2] [expr $gFrameHeight/2] -width [expr $gFrameWidth * .9] \
	-font {Helvetica 64 bold} -fill blue -text "$nameOfWinner wins!" \
	-tag winText

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

    $c delete winText
}


#############################################################################
# GetCellSize gets the correct size of the board locations for this canvas 
#and board size.
#############################################################################
proc GetCellSize {} {
    global gBoardRows gBoardCols
    global gFrameWidth gFrameHeight
    global gScoreBarHeight

    set boardCanvasSize [min $gFrameWidth [expr $gFrameHeight * (1-$gScoreBarHeight)]]
    
    return [min [expr $boardCanvasSize / $gBoardRows] [expr $boardCanvasSize / $gBoardCols]]
}

#############################################################################
# GetBoardYOffset returns the length in pixels from the top of the canvas to
# where the top of the board should be drawn.
#############################################################################
proc GetBoardYOffset {} {
    global gBoardRows gBoardCols
    global gFrameWidth gFrameHeight
    global cellSize

    if { $gBoardCols > $gBoardRows } {
	return [expr int( ($gBoardCols - $gBoardRows) * $cellSize / 2)]
    } else {
	return 0
    }
}

#############################################################################
# GetBoardXOffset returns the length in pixels from the leftmost edge of the
# canvas to where the left side of the board should be drawn.
#############################################################################
proc GetBoardXOffset {} {
    global gBoardRows gBoardCols
    global gFrameWidth gFrameHeight
    global cellSize

    return [expr int( ($gFrameWidth - ($cellSize * $gBoardCols)) /2 )]
}


#############################################################################
# AnimateMove animates any board change, changing from oldPosition to newPosition.
#############################################################################
proc AnimateMove { c oldPosition newPosition } {
    global gBoardRows gBoardCols boardSize
    
    set oldBoard [string range [C_GenericUnhash $oldPosition $boardSize] 0 [expr $boardSize-1]]
    set newBoard [string range [C_GenericUnhash $newPosition $boardSize] 0 [expr $boardSize-1]]

    set whitePieces 0
    set blackPieces 0

    #check for new and removed pieces
    for {set y 0} {$y < $gBoardRows} {set y [expr $y + 1]} {
	for {set x 0} {$x < $gBoardCols} {set x [expr $x + 1]} {
	    
	    set old [string index $oldBoard [Index $x $y]]
	    set new [string index $newBoard [Index $x $y]]

	    if { $old ne $new } {
		
		if { $old eq " " } {
		    
		    if { $new eq "W" } {
			set color [lindex [GS_ColorOfPlayers] 1]
		    } else {
			set color [lindex [GS_ColorOfPlayers] 0]
		    }
		    
		    FadePiece $c $x $y $color fadein
		}

		if { $new eq " " } {
		    FadePiece $c $x $y black fadeout
		}
	    }

	    #Count the pieces.  This has nothing to do with modifying the board, we just need it later.
	    if { $new eq "W" } {
		set whitePieces [expr $whitePieces+1]
	    } elseif { $new eq "B" } {
		set blackPieces [expr $blackPieces+1]
	    }
	}
    }

    #check for changed pieces
    for {set y 0} {$y < $gBoardRows} {set y [expr $y + 1]} {
	for {set x 0} {$x < $gBoardCols} {set x [expr $x + 1]} {
	    
	    set old [string index $oldBoard [Index $x $y]]
	    set new [string index $newBoard [Index $x $y]]

	    if { $old ne $new && $old ne " " && $new ne " " } {

		if { $new eq "W" } {
		    set color [lindex [GS_ColorOfPlayers] 1]
		} else {
		    set color [lindex [GS_ColorOfPlayers] 0]
		}

		FadePiece $c $x $y $color flip
	    }
	}
    }

    AnimateScoreBar $c $blackPieces $whitePieces
}

#############################################################################
# FadePiece animates a piece flipping.  x and y are the column and row of
# the piece, respectively, while newColor is the color that the piece
# should be when the animation is finished.
#
# type should be one of three things, "flip", "fadein", or "fadeout".
#
# flip should be used when a piece is already showing in this location.
# fadein should be used when the spot is empty, and we're creating a new
#  piece.
# fadeout should be used when the spot was previously full, and should be
#  empty when we're done.
#############################################################################
proc FadePiece { c x y newColor type } {
    global gBoardRows gBoardCols boardSize
    global gAnimationSpeed
    global cellSize
    global pi
    global xBoardOffset yBoardOffset

    if { $type != "flip" && $type != "fadein" && $type != "fadeout" } {
	error type must be flip or fadein
    }

    set speed [expr 60 - $gAnimationSpeed*10]
    
    set startCircle [expr $cellSize / 8.0]
    set endCircle   [expr $startCircle*7.0]
    set width       [expr $endCircle - $startCircle]
    
    set cornerX     [expr $x*$cellSize + $xBoardOffset]
    set cornerY     [expr $y*$cellSize + $yBoardOffset] 

    if { $type == "fadein" } {
	#we want to only do the last half if we're fading in (zero to full size)
	set loopStart [expr $speed/2]
	set loopEnd $speed
    } else { 
	if { $type == "fadeout" } {
	    #go from the start (entire piece)
	    #and go halfway (piece width 0)
	    set loopStart 0
	    set loopEnd [expr $speed/2]
	    
	} else {
	    #otherwise, do the whole thing
	    set loopStart 0
	    set loopEnd $speed
	}
    }

    $c raise piece$x$y

    for {set i $loopStart} {$i < $loopEnd} {incr i} {

	if { $i == [expr $speed/2] } {
	    $c itemconfig piece$x$y -fill $newColor -outline $newColor
	}

	set size [expr cos([expr $pi*$i/$speed])]
	set offsetX [expr $startCircle + (1-$size)*$width/2]
	set pieceWidth [expr $size*$width]
	
	$c coords piece$x$y [expr $cornerX + $offsetX] \
	    [expr $cornerY + $startCircle ] \
	    [expr $cornerX + $offsetX + $pieceWidth] \
	    [expr $cornerY + $endCircle ]
		
	after 1
	update idletasks
    }
    
    if { $type == "fadeout" } {
	$c lower piece$x$y
	$c coords piece$x$y [expr $cornerX + $startCircle] \
	    [expr $cornerY + $startCircle] \
	    [expr $cornerX + $startCircle + $width] \
	    [expr $cornerY + $startCircle + $width]
    }

}


#############################################################################
# DrawScoreBar draws the relative score bar at the top of the canvas c.
# It is constant length, and sets each color according to the ratio of the
# number of that color to the total number of pieces.
#############################################################################
proc DrawScoreBar { c numBlack numWhite } {
    global gFrameWidth gFrameHeight
    global gScoreBarHeight
    global xBoardOffset yBoardOffset

    global gLastNumBlack gLastNumWhite
    global cellSize
    global gBoardRows gBoardCols

    $c delete scorebar

    set boardWidth [expr $gFrameWidth - ($xBoardOffset * 2) ]
    set cutoff [expr int( $boardWidth * double($numBlack) / ($numBlack + $numWhite)) ]
    set barHeight [expr int( $gScoreBarHeight * $gFrameHeight) ]

    set offsetY [expr $yBoardOffset + $cellSize * $gBoardRows]

    set firstColor [lindex [GS_ColorOfPlayers] 0]
    set secondColor [lindex [GS_ColorOfPlayers] 1]

    $c create rectangle $xBoardOffset $offsetY \
	[expr $cutoff + $xBoardOffset] [expr $offsetY + $barHeight] \
	-fill $firstColor -outline $firstColor -width 2 -tags [list blackScore scorebar]

    $c create rectangle [expr $cutoff + $xBoardOffset] $offsetY \
	[expr $boardWidth + $xBoardOffset] [expr $offsetY + $barHeight] \
	-fill $secondColor -outline $secondColor -width 2 -tags [list whiteScore scorebar]

    if { $numBlack == 0 } {
	$c itemconfig blackScore -outline [lindex [GS_ColorOfPlayers] 1]
    } elseif { $numWhite == 0 } {
	$c itemconfig whiteScore -outline [lindex [GS_ColorOfPlayers] 0]
    }

    set gLastNumBlack $numBlack
    set gLastNumWhite $numWhite
}

#############################################################################
# AnimteScoreBar animates a change in the scroll bar.  It needs a canvas,
# the number of black pieces, and the number of white pieces. 
#
# DrawScoreBar must be run before AnimateScrollBar is called!
#############################################################################
proc AnimateScoreBar { c numBlack numWhite } {
    global gScoreBarHeight
    global gFrameWidth gFrameHeight
    global xBoardOffset yBoardOffset
    global gBoardRows gBoardCols
    global gAnimationSpeed
    global cellSize
    global pi

    global gLastNumBlack gLastNumWhite
    
    set oldratio [expr double( $gLastNumBlack ) / ( $gLastNumBlack + $gLastNumWhite) ]
    set newratio [expr double( $numBlack ) / ($numBlack + $numWhite) ]

    set boardWidth [expr $gFrameWidth - ($xBoardOffset * 2) ]
    set barHeight [expr int( $gScoreBarHeight * $gFrameHeight ) ]

    set animLength [expr 60 - $gAnimationSpeed*10]

    set offsetY [expr $yBoardOffset + $cellSize * $gBoardRows]

    #make sure the outlines are the right color
    if { $numBlack != 0 } {
	$c itemconfig blackScore -outline [lindex [GS_ColorOfPlayers] 0]
    }
    if { $numWhite != 0 } {
	$c itemconfig whiteScore -outline [lindex [GS_ColorOfPlayers] 1]
    }

    #loop through the animation
    for {set i 0} {$i < $animLength} {incr i} {
	
	set currentratio [expr $oldratio + ($newratio-$oldratio)*sin($pi*.5*double($i)/$animLength) ]

	set cutoff [expr int( $boardWidth * $currentratio)]

	$c coords blackScore $xBoardOffset $offsetY \
	    [expr $cutoff + $xBoardOffset] [expr $offsetY + $barHeight]
	
	$c coords whiteScore [expr $cutoff + $xBoardOffset] $offsetY \
	    [expr $boardWidth + $xBoardOffset] [expr $offsetY + $barHeight]

	after 1
	update idletasks
    }

    #if we've obliterated someone, change their outline color
    if { $numBlack == 0 } {
	$c itemconfig blackScore -outline [lindex [GS_ColorOfPlayers] 1]
    } elseif { $numWhite == 0 } {
	$c itemconfig whiteScore -outline [lindex [GS_ColorOfPlayers] 0]
    }

    set gLastNumBlack $numBlack
    set gLastNumWhite $numWhite
}


#############################################################################
# GetXYFromMove takes a move as passed in to GS_ShowMoves.  It then returns
# the x location and y location, concatenated together. "00" would be the
# return value for the first column and first row, while "12" would be the
# return value for the second column and third row.
#############################################################################
proc GetXYFromMove {theMove} {

    set pos [lindex $theMove 0]
    return [Column $pos][Row $pos]
}

#############################################################################
# Row takes an index on the board and returns the row where this index
# resides.
#############################################################################
proc Row { index } {
    global gBoardCols

    return [expr $index / $gBoardCols]
}

#############################################################################
# Column takes an index on the board and returns the column where this index
# resides.
#############################################################################
proc Column { index } {
    global gBoardCols

    return [expr $index % $gBoardCols]
}

#############################################################################
# Index takes a column and a row and returns the index of the place.
#############################################################################
proc Index { col row } {
    global gBoardCols

    return [expr [expr $row * $gBoardCols] + $col]
}

#############################################################################
# max simply returns the largest value passed to it.
#############################################################################
proc max { x y } {
    if {$x > $y} {
	return $x
    } else {
	return $y
    }
}

#############################################################################
# min returns the smaller value passed to it.
#############################################################################
proc min { x y } {
    if {$x < $y} {
	return $x
    } else {
	return $y
    }
}
