####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

set canvasWidth  500; # 230 for first ever board
set canvasHeight 500

set dotExpandAmount 10



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
    set kGameName "(Four Field) Kono"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set boardHeight, boardWidth, boardSize
    global boardHeight boardWidth boardSize
    set boardHeight 4
    set boardWidth 4
    set boardSize 16

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "To capture all of your opponent's pieces or to prevent him from moving."
    set kMisereString "To have all your pieces captured or to not be able to move."

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: To capture all of your opponent's pieces or to prevent him or her from moving."
    } else {
	SetToWinString "To Win: To capture all of your opponent's pieces or to prevent him or her from moving."
    }
    SetToMoveString "To Move: Players take turns moving their pieces either horizontally or vertically. To capture the opponent's piece a player must jump over one of his or her pieces and land on the opponent's piece. Captures cannot be made diagonally and only one piece can be captured on a move."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Greg Bonin, Nathan Spindel"
    set kTclAuthors "Greg Bonin, Nathan Spindel"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

#    set gInitialPosition [C_InitialPosition]
#    puts $gInitialPosition
#    set pieceString [C_Generic_Unhash gInitialPosition 16]


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

    return [list blue red]

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

    set captureRule \
	[list \
	     "Should capture moves be mandatory?" \
	     "Yes" \
	     "No" \
	    ]

    set boardWidthRule \
	[list \
	     "Board width:" \
	     "1" \
	     "2" \
	     "3" \
	     "4" \
	     "5"
	    ]

    set boardHeightRule \
	[list \
	     "Board height:" \
	     "1" \
	     "2" \
	     "3" \
	     "4" \
	     "5"
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $captureRule $boardWidthRule $boardHeightRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gForcedCapture
    set gForcedCapture 0

    global gBoardWidth
    set gBoardWidth 3

    global gBoardHeight
    set gBoardHeight 3

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gForcedCapture" "gBoardWidth" "gBoardHeight"]

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
    global gMisereGame gForcedCapture gBoardWidth gBoardHeight

    set option 0
#    set option [expr $option + $gBoardWidth+1]
#    set option [expr $option + 6 + $gBoardHeight+1]
#    set option [expr $option + 12 + $gMisereGame+1]

    # for HWrap
#    set option [expr $option + 14 + 1]

    # for VWrap
#    set option [expr $option + 16 + 1]

#    set option [expr $option + 18 + $gForcedCapture + 1]

#    puts $option
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
#    global gMisereGame gForcedCapture gBoardWidth gBoardHeight

#    set gForcedCapture [expr $option - 18 - 1]
#    set option [expr $option - 2]

    # HWrap
#    set option [expr $option - 2]

    # VWrap
 #   set option [expr $option - 2]

  #  set gStandardGame [expr $option - 12 - 1]
  #  set option [expr $option - 2]

  #  set gBoardHeight [expr $option - 5 - 1]
  #  set option [expr $option - 2]

#    set gBoardWidth [expr $option - 1]

#    set option [expr $option - 1]
#    set gMisereGame [expr 1-($option%2)]
}


proc max { x y } {
    if {$x > $y} {
	return $x
    } else {
	return $y
    }
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
    global canvasWidth canvasHeight boardHeight boardWidth
    $c configure -width $canvasWidth -height $canvasHeight
    set base [$c create rectangle 0 0  [expr $canvasWidth - 1] [expr $canvasHeight - 1] -fill white -tag base]   

    set margin 75

    set boardMax [max $boardHeight $boardWidth]
    set cellSize [expr [expr $canvasWidth - [expr $margin * 2]] / [expr $boardMax - 1]]

    set horizontalStart [expr [expr $canvasWidth - [expr $cellSize * [expr $boardWidth - 1]]] / 2]
    set verticalStart [expr [expr $canvasHeight - [expr $cellSize * [expr $boardHeight - 1]]] / 2]

    for {set i 0} {$i < $boardHeight} {incr i} {
	set verticalOffset [expr $verticalStart + [expr $i * $cellSize]]
	$c create line $horizontalStart $verticalOffset [expr $canvasWidth - $horizontalStart] $verticalOffset -tag base
    }

    for {set i 0} {$i < $boardWidth} {incr i} {
	set horizontalOffset [expr $horizontalStart + [expr $i * $cellSize]]
	$c create line $horizontalOffset $verticalStart $horizontalOffset [expr $canvasHeight - $verticalStart] -tag base
    }

    set pieceSize [expr $cellSize / 4]
    set arrowLength [expr $cellSize - $pieceSize]
    set longArrowLength [expr [expr $cellSize * 2] - $pieceSize]
    set arrowVar1 [expr 32 / $boardMax ]
    set arrowVar2 [expr 64 / $boardMax ]

    for {set x 0} {$x < $boardWidth} {incr x} {
	set horizontalOffset [expr $horizontalStart + [expr $x * $cellSize]]
	for {set y 0} {$y < $boardHeight} {incr y} {
	    set pos [expr [expr $y * $boardWidth] + $x]
	    set verticalOffset [expr $verticalStart + [expr $y * $cellSize]]

	    $c create oval [expr $horizontalOffset - $pieceSize] [expr $verticalOffset - $pieceSize] \
		[expr $horizontalOffset + $pieceSize] [expr $verticalOffset + $pieceSize] -fill blue -width 2 -tag piece$x$y

#	    $c bind piece$x$y <Enter> "MouseOverExpand piece$x$y $c"
#	    $c bind piece$x$y <Leave> "MouseOutContract piece$x$y $c"

	    if {$pos >= $boardWidth} {
		set temp [expr $y -  1]
		$c create line $horizontalOffset $verticalOffset $horizontalOffset [expr $verticalOffset - $arrowLength] \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$x$temp arrow]
	    }

	    if {$pos >= [expr $boardWidth * 2]} {
		set temp [expr $y -  2]
		$c create line $horizontalOffset $verticalOffset $horizontalOffset [expr $verticalOffset - $longArrowLength] \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$x$temp arrow]
	    }
	    
	    if {$pos < [expr $boardWidth * [expr $boardHeight - 1]]} {
		set temp [expr $y + 1]
		$c create line $horizontalOffset $verticalOffset $horizontalOffset [expr $verticalOffset + $arrowLength] \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$x$temp arrow]
	    }
	    
	    if {$pos < [expr $boardWidth * [expr $boardHeight - 2]]} {
		set temp [expr $y +  2]
		$c create line $horizontalOffset $verticalOffset $horizontalOffset [expr $verticalOffset + $longArrowLength] \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$x$temp arrow]
	    }
	    
	    if {[expr $pos % $boardWidth] != 0} {
		set temp [expr $x -  1]
		$c create line $horizontalOffset $verticalOffset [expr $horizontalOffset - $arrowLength] $verticalOffset \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$temp$y arrow]
	    }
	    
	    if {[expr $pos % $boardWidth] > 1} {
		set temp [expr $x -  2]
		$c create line $horizontalOffset $verticalOffset [expr $horizontalOffset - $longArrowLength] $verticalOffset \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$temp$y arrow]
	    }
	    
	    if {[expr $pos % $boardWidth] < [expr $boardWidth - 1]} {
		set temp [expr $x +  1]
		$c create line $horizontalOffset $verticalOffset [expr $horizontalOffset + $arrowLength] $verticalOffset \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$temp$y arrow]
	    }
	    
	    if {[expr $pos % $boardWidth] < [expr $boardWidth - 2]} {
		set temp [expr $x +  2]
		$c create line $horizontalOffset $verticalOffset [expr $horizontalOffset + $longArrowLength] $verticalOffset \
		    -arrow last -width $arrowVar1 -arrowshape [list $arrowVar2 $arrowVar2 $arrowVar1] -tags [list arrow$x$y$temp$y arrow]
	    }
	}
    }

    $c raise base
}    

proc MovePiece { x1 y1 x2 y2 c} {
    $c lower piece$x1$y1
    $c itemconfig piece$x2$y2 -fill [$c itemcget piece$x1$y1 -fill]
    $c raise piece$x2$y2
}

proc MouseOverExpand { dot c } {
    global dotExpandAmount
#    $c itemconfig $dot -fill red
}

proc MouseOutContract { dot c } {
#    $c itemconfig $dot -fill blue
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
    global boardWidth boardHeight boardSize
    
    $c raise base all

    set pieceString [string range [C_GenericUnhash $position $boardSize] 0 [expr $boardSize-1]]
    set pieceNumber 0

    for {set i 0} {$i < $boardWidth} {set i [expr $i + 1]} {
	for {set j 0} {$j < $boardHeight} {set j [expr $j + 1]} {
	    if {[string compare [string index $pieceString $pieceNumber] "x"] == 0} {
		$c itemconfig piece$j$i -fill red		
		$c raise piece$j$i
	    } elseif {[string compare [string index $pieceString $pieceNumber] "o"] == 0} {
		$c itemconfig piece$j$i -fill blue		
		$c raise piece$j$i
	    } else {}

	    set pieceNumber [expr $pieceNumber + 1]
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
		set color red
	    }
	}
	
	set dest [GetDestFromMove $move]
	set source [GetSourceFromMove $move]

	$c raise arrow$source$dest
	$c itemconfig arrow$source$dest -fill $color
	$c bind arrow$source$dest <ButtonRelease-1> "ReturnFromHumanMove $move"
	$c bind arrow$source$dest <Enter> "$c itemconfig arrow$source$dest -fill black"
	$c bind arrow$source$dest <Leave> "$c itemconfig arrow$source$dest -fill $color"

#	$c raise piece$source
    }
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in
    
    $c lower arrow
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

	### TODO if needed
	
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

	### TODO if needed

}

proc GetDestFromMove {theMove} {
    global boardHeight boardWidth

    set dest [expr $theMove % 1000]

    set width [expr $dest % $boardWidth]
    set height [expr $dest / $boardWidth]

    return $width$height
}

proc GetSourceFromMove {theMove} {
    global boardHeight boardWidth

    set source [expr $theMove / 1000]

    set width [expr $source % $boardWidth]
    set height [expr $source / $boardWidth]

    return $width$height
}