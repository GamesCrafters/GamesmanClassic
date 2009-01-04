####################################################
# this is a tcl module for the game of Dao
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
# Dao-specific code by Dan Garcia and GamesCrafters2005Fa
#
# LAST CHANGE: $Id: mdao.tcl,v 1.8 2008-12-15 23:18:15 koolswim88 Exp $
#
####################################################

global NUM_OF_DIRS
set NUM_OF_DIRS 8

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
    set kGameName "Dao"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 189388
    set gPosition $gInitialPosition

    ### Set boardRows, boardCols, boardSize

    global boardRows boardCols boardSize
    set boardRows 4
    set boardCols 4
    set boardSize [expr $boardRows * $boardCols]

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "I will be a monkeys uncle"
    set kMisereString "Not used ever"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "A player wins any one of three ways: (1) Forming a straight line with all 4 of their pieces (diagonally doesn't count), (2) Occupying the 4 corners of the board, or (3) Forming their pieces into a 2x2 square. BUT a player loses by using just 3 of their pieces to trap a single opponent piece in the corner of the board."
    } else {
	SetToWinString "A player loses any one of three ways: (1) Forming a straight line with all 4 of their pieces (diagonally doesn't count), (2) Occupying the 4 corners of the board, or (3) Forming their pieces into a 2x2 square. BUT a player wins by using just 3 of their pieces to trap a single opponent piece in the corner of the board."
    }
    SetToMoveString "To Move: You take turns moving your pieces in a straight line in any 'open' direction including diagonally. You may not jump other pieces or move to a spot already occupied. The key to Dao is that you must move your piece as far as possible each turn (until it reaches the end of teh board or another piece). In other words, during each turn you may not move only 1 space if 2 or 3 are open in the same direction - or 2 if 3 are open." 
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors   "Dan Garcia and GamesCrafters 2005Fa"
    set kTclAuthors "Dan Garcia and GamesCrafters 2005Fa"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    set gInitialPosition [C_InitialPosition]
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

    set boardColsRule \
	[list \
	     "Board width:" \
	     "1" \
	     "2" \
	     "3" \
	     "4" \
	     "5"
	    ]

    set boardRowsRule \
	[list \
	     "Board height:" \
	     "1" \
	     "2" \
	     "3" \
	     "4" \
	     "5"
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $captureRule $boardColsRule $boardRowsRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gForcedCapture
    set gForcedCapture 0

    global gBoardWidth
    set gBoardWidth 4

    global gBoardHeight
    set gBoardHeight 4

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
}


proc Max { x y } {
    if {$x > $y} {
	return $x
    } else {
	return $y
    }
}

proc Min { x y } {
    if {$x < $y} {
	return $x
    } else {
	return $y
    }
}

#############################################################################
##
## DrawCircle
##
## Here we draw a circle on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawCircle { c slotSize slotX slotY theTag theColor } {

    set circleWidth [expr $slotSize/10.0]
    set startCircle [expr $slotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$slotSize]
    set cornerY     [expr $slotY*$slotSize]
    set theCircle [$c create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline $theColor \
		       -fill $theColor \
		       -tag $theTag]

    $c move $theCircle $cornerX $cornerY

    return $theCircle
}

proc MoveCircle { c slotSize slotX slotY theId } {

    set circleWidth [expr $slotSize/10.0]
    set startCircle [expr $slotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$slotSize]
    set cornerY     [expr $slotY*$slotSize]
    
    set theCoords [$c coords $theId]
    set botLeftX [lindex $theCoords 0]
    set botLeftY [lindex $theCoords 1]
    $c move $theId [expr $cornerX + $startCircle - $botLeftX] \
	[expr $cornerY + $startCircle - $botLeftY]
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
    global boardRows boardCols
    global gFrameWidth gFrameHeight
    set mySize [Min $gFrameWidth $gFrameHeight]
    set boardMax [Max $boardRows $boardCols]
    set cellSize [expr $mySize / $boardMax]

    $c delete all; # Clear all objects

    for {set x 0} {$x < $boardCols} {incr x} {
	for {set y 0} {$y < $boardRows} {incr y} {
	    $c create rectangle [expr $x * $cellSize] [expr $y * $cellSize] [expr ($x + 1) * $cellSize] [expr ($y + 1) * $cellSize] -fill grey -outline black -tag base 
	}
    }

    set pieceNum 0
    for {set col 0} {$col < $boardCols} {incr col} {
	set x [DrawCircle $c $cellSize $col $col pieces [lindex [GS_ColorOfPlayers] 0]]
	set o [DrawCircle $c $cellSize $col [expr $boardCols - 1 - $col] pieces [lindex [GS_ColorOfPlayers] 1]]
	$c addtag x$pieceNum withtag $x
	$c addtag o$pieceNum withtag $o
	$c addtag pieces withtag $x
	$c addtag pieces withtag $o
	incr pieceNum
    }

    # 5 6 7  { row col }
    # 3   4
    # 0 1 2

    set dels { { -1   1 }   { 0   1 }   { 1   1 }  
               { -1   0 }               { 1   0 }  
      	       { -1  -1 }   { 0  -1 }   { 1  -1 } }

    for {set col 0} {$col < $boardCols} {incr col} {
	for {set row 0} {$row < $boardRows} {incr row} {
	    set index [expr $row * $boardCols + $col]
	    for { set dir 0 } {$dir < 8} {incr dir} {
		set toX [expr $col + [lindex [lindex $dels $dir] 1]]; # col
		set toY [expr $row + [lindex [lindex $dels $dir] 0]]; # row
		if { ($toX >= 0) && ($toY >= 0) && ($toX <= ($boardCols-1)) && ($toY <= ($boardRows-1)) } {
		    set a [CreateArrow $c $col $row $toX $toY $boardCols $boardRows $cellSize cyan]
		    $c addtag arrow$index$dir withtag $a
		    $c addtag arrows withtag $a
		}
	    }
	}
    }

    $c raise base
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
    global boardCols boardRows boardSize
    global gFrameWidth gFrameHeight
    set mySize [Min $gFrameWidth $gFrameHeight]
    set boardMax [Max $boardRows $boardCols]
    set cellSize [expr $mySize / $boardMax]
    
    $c raise base all
    $c raise pieces

    set board [C_GenericUnhash $position]
    set pieceNumber 0

    set pieceNumX 0
    set pieceNumO 0
    set i 0
    for {set row 0} {$row < $boardRows} {incr row} {
	for {set col 0} {$col < $boardCols} {incr col} {
	    if {[string compare [string index $board $i] "X"] == 0} {
		MoveCircle $c $cellSize $col $row x$pieceNumX
		incr pieceNumX
	    } elseif {[string compare [string index $board $i] "O"] == 0} {
		MoveCircle $c $cellSize $col $row o$pieceNumO
		incr pieceNumO
	    } else {}
	    incr i
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
    global tk_library kRootDir gStippleRootDir
    set gStippleRootDir "$kRootDir/../bitmaps/"
    set stipple12 @[file join $gStippleRootDir gray12.bmp] 
	set stipple25 @[file join $gStippleRootDir gray25.bmp]
	set stipple50 @[file join $gStippleRootDir gray50.bmp]
	set stipple75 @[file join $gStippleRootDir gray75.bmp]
    set stipple ""
     
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set remoteness [lindex $item 2]
	set delta [lindex $item 3]
	set color cyan

	#h4x
	puts "move:$move, value:$value, remoteness: $remoteness, delta:$delta"

	if {$moveType == "value" || $moveType == "rm"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red4
	    }
	}
	
    if {$moveType == "rm"} {
        set stipple $stipple50
        
        if {$delta == 0} {
            set stipple ""
        } elseif {$delta == 1} {
            set stipple $stipple75
        } elseif {$delta == 2} {
            set stipple $stipple50
        } elseif {$delta == 3} {
            set stipple $stipple25
        } else {
            set stipple $stipple12
        }
     }
     
	set index [Unhasher_Index     $move]
	set dir   [Unhasher_Direction $move]

	$c raise arrow$index$dir

	$c itemconfig arrow$index$dir -fill $color -stipple $stipple

	$c bind arrow$index$dir <ButtonRelease-1> "ReturnFromHumanMove $move"
	$c bind arrow$index$dir <Enter> "$c itemconfig arrow$index$dir -fill black"
	$c bind arrow$index$dir <Leave> "$c itemconfig arrow$index$dir -fill $color"
    }
    $c raise pieces
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    $c lower arrows
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

proc Unhasher_Index { hashed_move } {
    global NUM_OF_DIRS
    return [expr $hashed_move / $NUM_OF_DIRS];
}

proc Unhasher_Direction { hashed_move } { 
    global NUM_OF_DIRS
    return [expr $hashed_move % $NUM_OF_DIRS];
}

proc Row { index } {
    global boardCols
    return [expr $index / $boardCols]
}

proc Column { index } {
    global boardCols
    return [expr $index % $boardCols]
}

#############################################################################
##
## CreateArrow
##
## Create and return arrow at the specified slots.
##
#############################################################################

proc CreateArrow { c fromX fromY toX toY slotsX slotsY slotSize color } {
    ### Set up the constants
    set delX [expr $toX - $fromX]
    set delY [expr $toY - $fromY]
    set lineGap    [expr int($slotSize/8)]
    set lineLength [expr int($slotSize*3/4)]
    set endX [expr $delX * $lineLength]
    set endY [expr $delY * $lineLength]
    set offX [expr $delX * $lineGap + int($slotSize/2)]
    set offY [expr $delY * $lineGap + int($slotSize/2)]

    ### Create the line starting from the origin
    set arrow1 [expr $lineGap * 2]
    set arrow2 [expr $lineGap * 2]
    set arrow3 [expr $lineGap * 1]
    # set theMove [$c create line 0 0 $endX $endY \
# 	    -width $lineGap \
# 	    -arrow last \
# 	    -arrowshape [list $arrow1 $arrow2 $arrow3] \
# 	    -fill $color]

    #h4x
    set theMove [PolyArrow $c 0 0 $endX $endY $lineGap $arrow1 $arrow2 $arrow3]
    
    
    ### Shift the line to put it in the right place
    set cornerX [expr $fromX * $slotSize + $offX]
    set cornerY [expr $fromY * $slotSize + $offY]
    $c move $theMove $cornerX $cornerY
    
    #h4x
    #set theMove [PolyArrow $c $fromX $fromY $toX $toY $lineGap $arrow1 $arrow2 $arrow3 []]
    #$c itemconfig $theMove -fill $color
    #$c raise $theMove
    
        
    return $theMove
}
