####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
##
## NAME:         mtactix.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Tac Tix
##               for the Master's project GAMESMAN
##
## AUTHOR:       Dan Garcia  -  University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         05-12-95
##
## UPDATE HIST:
##
## 05-15-95 1.0    : Final release code for M.S.
##
## GOLD UPDATE:  Jeffrey Chiang (2005-01-10)
#############################################################################
#############################################################################
# Some Notes:
#   o This tcl code is hardcoded for the 4x4 game.  To change it, you really
#     only have to change the boardsize.  In fact, there's a GUI written
#     (mtactixsub.tcl) that provides support for picking an initial position
#   o There is some not so nice abstraction going on in this code.
#     Essentially, we pass a number representation of the move to 
#     ReturnFromHumanMove (since that's what it expects; the move
#     in C is a number, not a list of the slots.  So stuff was changed
#     around depending on when the move is needed and in what form.
#############################################################################

proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}

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
    
    global g2List
    ### Blah - Need this to convert stuff to moves, I think
    ### /* Powers of 2 - this is the way I encode the position, as an integer */
    set g2List [list 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536]

    ### Set the name of the game
    
    global kGameName
    set kGameName "Tac Tix"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 65535
    set gPosition $gInitialPosition

    ### Set the size of the canvas (can be changed)

    global CANVAS_WIDTH CANVAS_HEIGHT gFrameWidth gFrameHeight

    set CANVAS_WIDTH [min $gFrameWidth $gFrameHeight]
    set CANVAS_HEIGHT $CANVAS_WIDTH

    ### Set the size of the board

    global gBoardSize
    set gBoardSize 16

    global gSlotsX gSlotsY 
    set gSlotsX 4
    set gSlotsY 4

    ### Calculate and set size of a slot
    ### Allegedly it should be multiple of 80, but 100 is ok
    ### Hmmm, I think X is horizontal and Y is vertical...
    global gSlotSize

    set widthLimit [expr $CANVAS_WIDTH/$gSlotsX]
    set heightLimit [expr $CANVAS_HEIGHT/$gSlotsY]

    if {$heightLimit < $widthLimit} {
        set gSlotSize $heightLimit
    } else {
        set gSlotSize $widthLimit
    }

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCircle
    set kRightDrawProc DrawCircle
    set kBothDrawProc  DrawCircle

    ### These are used in the routine that draws the pieces - we want a small
    ### piece used as a label but a larger piece used in the playing board.

    global kBigPiece
    set kBigPiece 1

    global kSmallPiece
    set kSmallPiece 0

    ### Set the color and font of the labels

    global kLabelFont
    set kLabelFont { Helvetica 12 bold }

    global kLabelColor
    set kLabelColor grey40

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString

    set kStandardString "Removing the last stone WINS"
    set kMisereString "Removing the last stone LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Remove the last stone from the board."
    } else {
	SetToWinString "To Win: Force your opponent into removing the last stone from the board."
    }
    SetToMoveString "To Move: On a player's turn, he or she may either remove one piece or remove adjacent pieces from a single row or column. The pieces must be touching in order to be removed together.  If you want to remove a single piece select the circle. If you would like to remove a row or column of pieces, select the line corresponding to the pieces you want to remove."

    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Dan Garcia"
    set kTclAuthors "Dan Garcia"
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

    return [list magenta magenta]
    
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
    set option 1
    set option [expr $option + (1-$gMisereGame)]
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
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
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
    global CANVAS_WIDTH CANVAS_HEIGHT
    global gSlotsX gSlotsY

    $c configure -width $CANVAS_WIDTH -height $CANVAS_HEIGHT

    for {set i 0} {$i < $gSlotsX} {incr i} { 
        for {set j 0} {$j < $gSlotsY} {incr j} { 
            CreateSlot $c $i $j  
	} 
    }
} 


#############################################################################
##
## CreateSlot
##
## Here we create a new slot on the board defined by window w. The value
## of the slot in (slotX,slotY) tells us where on the board to be.
##
#############################################################################

proc CreateSlot {w slotX slotY} {
    global gSlotSize gSlotList

    set slot [$w create rect \
        [expr $gSlotSize*$slotX] \
        [expr $gSlotSize*$slotY] \
        [expr $gSlotSize*[expr $slotX+1]] \
        [expr $gSlotSize*[expr $slotY+1]] \
        -outline black \
        -width 1 \
        -fill grey \
        -tag tagSlot]

    ### Remember what the id of the slot was in our array

    set gSlotList($slotX,$slotY) $slot
}


#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot position} {

    global gSlotsX gSlotsY

    ### Now we put the pieces on the board

    set theIndex [SinglePieceRemovalCoupleMove $slotX $slotY]

    if { $position & int(pow(2,$theIndex)) } {

	### Put a piece there

	DrawPiece $slotX $slotY "+" $w
    }
}


#############################################################################
##
## SinglePieceRemovalCoupleMove
##
## With SinglePieceRemoval, sometimes the move is in (x,y) and we have to 
## convert it to a single move again.
##
#############################################################################

proc SinglePieceRemovalCoupleMove { slotX slotY } {
    global gSlotsX

    ### Row major format. The number of rows times the pieces per row + the # of columns
    return [expr ($gSlotsX * $slotY) + $slotX]
}


#############################################################################
##
## DrawPiece
##
## Draw a piece at the slot specified by slotX and slotY
##
#############################################################################

proc DrawPiece { slotX slotY thePiece c} {
    global kBigPiece kLeftDrawProc kRightDrawProc kBothDrawProc

    if     { $thePiece == "X" } {
	return [$kLeftDrawProc  $c $slotX $slotY tagPiece blue $kBigPiece]
    } elseif { $thePiece == "O" } { 
	return [$kRightDrawProc $c $slotX $slotY tagPiece red $kBigPiece]
    } elseif { $thePiece == "+" } { 
	return [$kBothDrawProc $c $slotX $slotY tagPiece magenta $kBigPiece]
    } elseif { $thePiece == "-" } { 
	return [DrawLastMove $c $slotX $slotY tagPiece magenta $kBigPiece]
    } else {
	BadElse DrawPiece
    }
}


#############################################################################
##
## DrawLastMove
##
## Here we draw a faint sketch of the last person to move.
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawLastMove { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set circleWidth [expr $theSlotSize/10.0]
    set startCircle [expr $theSlotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$theSlotSize]
    set cornerY     [expr $slotY*$theSlotSize]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline $theColor \
		       -tag $theTag]

    if { $drawBig } {
	$w move $theCircle $cornerX $cornerY
	$w addtag tagLastMove withtag $theCircle
    }

    return $theCircle
}


#############################################################################
##
## DrawCircle
##
## Here we draw a circle on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawCircle { w slotX slotY theTag theColor drawBig } {
    global gSlotSize
    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set circleWidth [expr $theSlotSize/10.0]
    set startCircle [expr $theSlotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$theSlotSize]
    set cornerY     [expr $slotY*$theSlotSize]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline $theColor \
		       -fill $theColor \
		       -tag $theTag]

    if { $drawBig } {
	$w move $theCircle $cornerX $cornerY
	$w addtag tagPieceCoord$slotX$slotY withtag $theCircle
	$w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle
    }

    return $theCircle
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
    
    global gSlotList
    
    global gSlotsX gSlotsY

    for {set i 0} {$i < $gSlotsX} {incr i} { 
        for {set j 0} {$j < $gSlotsY} {incr j} { 
	    ### Remember what the id of the slot was in our array

	    set slot gSlotList($i,$j)

	    GS_EmbellishSlot $c $i $j $slot $position
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
    global gSlotList
    
    global gPlayerOneTurn
    set gPlayerOneTurn 1

    global CANVAS_WIDTH CANVAS_HEIGHT
    global gSlotsX gSlotsY

    $c configure -width $CANVAS_WIDTH -height $CANVAS_HEIGHT
    
    GS_DrawPosition $c $position
    $c delete gameover
    $c raise tagPiece all
    
    # Anoto pen support - start new game
    C_SetGameSpecificOptions 1 1
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
    global gPlayerOneTurn gPosition gSlotSize gSlotList
    global gSlotsX gSlotsY 
    set w $c
    
    ### Get the slot's id
	
    set theSlot dummyArgNeverSeen

    $w itemconfig tagPiece -fill magenta

    ### Enable or Disable slots as a result of the move
    
    GS_HandleEnablesDisables $w $theSlot [GS_ConvertInteractionToMove $theMove]

    ### Swap turns

    set gPlayerOneTurn [not $gPlayerOneTurn]

}


#############################################################################
##
## GS_HandleEnablesDisables
##
## At this point a move has been registered and we have to handle the 
## enabling and disabling of slots
##
#############################################################################

proc GS_HandleEnablesDisables { w theSlot theMove } {
    global gSlotsX

    $w delete tagLastMove

    ### Here we remove all the pieces under the move just chosen.

    foreach i $theMove {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	set theGoner [DrawCircle $w $slotX $slotY tagDummy magenta 1]
	$w dtag $theGoner tagPieceCoord$slotX$slotY 
	$w lower tagPieceCoord$slotX$slotY all

	### Animating the piece's demise doesn't work, so we just remove it
	### When I find the memory bug, put this back.

#	ShrinkDeleteTag $w $theGoner
	$w delete $theGoner

	DrawPiece $slotX $slotY "-" $w
    }
}


#############################################################################
##
## GS_ConvertToAbsoluteMove
##
## Sometimes the move handed back by our C code is a relative move. We need
## to convert this to an absolute move to indicate on the board.
##
#############################################################################

proc GS_ConvertToAbsoluteMove { theMove } {
    global gPosition gSlotsX gSlotsY

    ### Fortunately, TacTix's moves are already absolute.

    set theSlots {}

    for {set i 0} {$i < ($gSlotsX * $gSlotsY)} {incr i} { 
	if { int(pow(2,$i)) & $theMove } {
	    lappend theSlots $i
	}
    }    

    return $theSlots
}


#############################################################################
##
## GS_ConvertInteractionToMove
##
## This will now unhash the move, actually.  
##
#############################################################################

proc GS_ConvertInteractionToMove { theMove } {
    global gBoardSize
    set moveList {}

    set j 1
    for {set i 0} {$i < $gBoardSize} {incr i} {
	if { [expr $j & $theMove] } {
	    lappend moveList $i
	}
	set j [expr $j * 2]
    }
    return $moveList
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
#
# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveTypeToColor that takes in moveType and
# returns the correct color.
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {
  global kBigPiece

    if {$moveType == "value"} {

	foreach item $moveList {
	    set theMove [lindex $item 0]
	    set value [lindex $item 1]
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} { 
		set color green
	    } else {
		set color red4
	    }

	    DrawMoveMultiplePieceRemoval $c $theMove $color $kBigPiece
	}

    } else {

	foreach item $moveList {
	    set theMove [lindex $item 0]
	    set color cyan
	    DrawMoveMultiplePieceRemoval $c $theMove $color $kBigPiece
	}


    }
    
    update idletasks
}


#############################################################################
##
## DrawMoveMultiplePieceRemoval
##
## Here we show a user's move, graphically. The color might be generic or
## it might represent a value. If drawBig is true, we draw it the full size
## of the slot. Otherwise we draw it 1/5 of the size of the slot.
##
#############################################################################

proc DrawMoveMultiplePieceRemoval { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]

    ### Ok, if it's going to go in the "Values" window, let it be a circle
    
    if { !$drawBig } {
	set theMove [$w create oval 0 0 $circleSize $circleSize \
		-outline $color \
		-fill $color]
  } else {

	set theMoveArg [GS_ConvertToAbsoluteMove $theMoveArg]

	if { [llength $theMoveArg] == 1 } {
	    
	    ### If the move is a single piece, draw a circle
	    
	    set theMoveArg [lindex $theMoveArg 0]
	    
	    set theMove [$w create oval 0 0 $circleSize $circleSize \
		    -outline $color \
		    -fill $color]
	    
	    ### Put it in the usual place away from the edge.
	    
	    set slotX   [expr $theMoveArg % $gSlotsX]
	    set slotY   [expr $theMoveArg / $gSlotsX]
	    set cornerX [expr $slotX   * $gSlotSize + $circleOffset]
	    set cornerY [expr $slotY   * $gSlotSize + $circleOffset]
	    	} else {
	    
	    ### If the move is a multiple move, draw a line.
	    
	    ### The gap from the line to the edge
	    
	    set theLineGap    [expr int($gSlotSize / 25)]
	    
	    ### The number of slots it occupies (= length)
	    
	    set theLineSlots  [llength $theMoveArg]
	    
	    ### The actual length of the line is the width of two slots
	    ### minus the two gaps at the end.
	    
	    set theLineLength [expr ($gSlotSize * $theLineSlots) - (2 * $theLineGap)]
	    
	    set theMoveBegin [lindex $theMoveArg 0]
	    set theMoveEnd   [lindex $theMoveArg [expr [llength $theMoveArg] - 1]]
	    
	    set isitHorizontal [expr ($theMoveEnd - $theMoveBegin) < $gSlotsX]
	    
	    set slotXbegin [expr $theMoveBegin % $gSlotsX]
	    set slotYbegin [expr $theMoveBegin / $gSlotsX]
	    
	    if { $theLineSlots == 2 } {
		set theLineOffset [expr (($isitHorizontal ? $slotXbegin : $slotYbegin) % 2) ? 11 : 3]
	    } elseif { $theLineSlots == 3 } {
		set theLineOffset [expr (($isitHorizontal ? $slotXbegin : $slotYbegin) % 2) ? 9 : 5]
	    } elseif { $theLineSlots == 4 } {
		set theLineOffset 7
	    } else {
		BadElse "theLineSlots in DrawMove"
	    }
	    
	    if { $isitHorizontal } {
		set theLineX $theLineLength
		set theLineY 0
		set theLineOffsetX $theLineGap
		set theLineOffsetY [expr $theLineGap * $theLineOffset + $theLineGap/2]
	    } else {
		set theLineX 0
		set theLineY $theLineLength
		set theLineOffsetX [expr $theLineGap * $theLineOffset + $theLineGap/2]
		set theLineOffsetY $theLineGap
	    }
	    
	    ### Create the line
	    
	    set theMove [$w create line 0 0 $theLineX $theLineY \
		    -width $theLineGap \
		    -capstyle round \
		    -fill $color]
	    
	    set cornerX [expr $slotXbegin   * $gSlotSize + $theLineOffsetX]
	    set cornerY [expr $slotYbegin   * $gSlotSize + $theLineOffsetY]
	}

	$w move $theMove $cornerX $cornerY

	$w addtag tagMoves withtag $theMove
	$w bind $theMove <1> "SendMove \{$theMoveArg\}"
	$w bind $theMove <Enter> "HandleEnterMultiplePieceRemoval $w \{$theMoveArg\}"
	$w bind $theMove <Leave> "HandleLeaveMultiplePieceRemoval $w \{$theMoveArg\}"
    }
}


proc SendMove { theMove } {
    ReturnFromHumanMove [ConvertInputToMove $theMove]
}


#############################################################################
##
## ConvertInputToMove
##
## Since the moves are actually hashed later on, we need to hash them too
##
#############################################################################
proc ConvertInputToMove { theMove } {
    global g2List

    set numPieces [llength $theMove]
    set tmpMove 0
    for {set i 0} {$i < $numPieces} {incr i} { 
	set somePiece [lindex $theMove $i]
	set someNum [lindex $g2List $somePiece]
	set tmpMove [expr $tmpMove + $someNum]
    }
    return $tmpMove
}
 


#############################################################################
##
## HandleEnterMultiplePieceRemoval
##
## Ok, the user has just entered a move. Now what to do?
##
#############################################################################

proc HandleEnterMultiplePieceRemoval { w theMove } {
    global gSlotsX

    ### Here we color all pieces to-be-chosen as black.

    foreach i $theMove {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	$w itemconfig tagPieceCoord$slotX$slotY -fill black
    }
}


#############################################################################
##
## HandleLeaveMultiplePieceRemoval
##
## Ok, the user has just left a move. Now what to do?
##
#############################################################################

proc HandleLeaveMultiplePieceRemoval { w theMove } {
    global gSlotsX

    ### Here we color all pieces to-be-chosen as white.

    foreach i $theMove {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	$w itemconfig tagPieceCoord$slotX$slotY -fill magenta
    }
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
    $c delete tagMoves
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
    global gPlayerOneTurn
    global gSlotsX
    global gMovesSoFar
    global gInitialPosition

    ### TODO if needed
    GS_DrawPosition $c $positionAfterUndo
    
    if {$positionAfterUndo == $gInitialPosition} {
	set thePreviousPreviousMove {} 
    } else {
	set thePreviousPreviousMove [GS_ConvertInteractionToMove [TacTix_peek2 $gMovesSoFar]] }

    foreach i $thePreviousPreviousMove {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	set theGoner [DrawCircle $c $slotX $slotY tagDummy magenta 1]
	$c dtag $theGoner tagPieceCoord$slotX$slotY 
	$c lower tagPieceCoord$slotX$slotY all

	$c delete $theGoner

	DrawPiece $slotX $slotY "-" $c
    }

    if { $gPlayerOneTurn == 1} { set gPlayerOneTurn 0 } { set gPlayerOneTurn 1}
}

proc TacTix_peek2 {l} {
    return [lindex $l 1]
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
    
    global CANVAS_WIDTH
    set size $CANVAS_WIDTH
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


#############################################################################
##
## not
##
## easier than [expr !]
##
#############################################################################

proc not { x } {
    if { $x } { return 0 } { return 1 }
}