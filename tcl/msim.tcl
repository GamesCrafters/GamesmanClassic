#########################################################################
##
## NAME:         msimtcl
##
## DESCRIPTION:  The source code for the Tcl component of sim
##               for the Master's project GAMESMAN
##
## AUTHOR:       Peterson Trethewey
##               Copyright (C) Peterson Trethewey, 2002. All rights reserved.
##
## DATE:         05-13-02
##
## UPDATE HIST:  
## 03-17-06 David Chan
##                 - converted to gamesman3 gui, code from mttt3.tcl adopted
##                   for this game
## 03-19-06 David Chan
##                 - added scaling, code shamelessly ripped from Eudean
##                   and game over screen, from Eudean
## 05-23-06 David Chan
##                 - comments added
#############################################################################


# global variables
# power of 3s used to unhash
set g3Array [list 1 3 9 27 81 243 729 2187 6561 19683 59049 177147 531441 1594323 4782969]

set edges 15
set bgColor black
set bgColor2  grey
set lineColor cyan
set p1Color blue
set p2Color red
set hColor orange
set vertices 6
set CANVAS_HEIGHT 1
set CANVAS_WIDTH 1
set aval(0) false
#not sure if I need some of these variables, will clean up later
#dont want to break code
set gameover true
set prevColor $hColor
set dashSize 1
set lastMove 0

##############################################################################
# given a board position
# unhashes the board
# places the correct pieces into an
# array to be handled
#
##############################################################################
proc unhash {pos array} {
    global edges
    global g3Array
    upvar $array a

    for { set i [expr $edges - 1] } { $i >= 0 } { set i [expr $i - 1] } {
	if { $pos >= [expr 2 * [lindex $g3Array $i]] } {
	   set a($i) x
	    set pos [expr $pos - [expr 2 * [lindex $g3Array $i]]]
	} elseif { $pos >= [expr 1 * [lindex $g3Array $i]] } {
	    set a($i) o
	    set pos [expr $pos - [expr 1 * [lindex $g3Array $i]]]
	} elseif { $pos >= [expr 0 * [lindex $g3Array $i]] } {
	    set a($i) -
	    set pos [expr $pos - [expr 0 * [lindex $g3Array $i]]]
	}
    }
	    
}

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
#############################################################################

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Sim"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    global CANVAS_WIDTH CANVAS_HEIGHT gFrameWidth gFrameHeight

    set CANVAS_WIDTH [min $gFrameWidth $gFrameHeight]
    set CANVAS_HEIGHT $CANVAS_WIDTH

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to complete a triangle  LOSES"
    set kMisereString "First player to complete a triangle wins"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Avoid making a triangle of your color."
    } else {
	SetToWinString "To Win: Complete a triangle of your color before your opponent."
    }
    SetToMoveString "To Move: Players take turns connecting 2 of the 6 corners with a line. A line cannot be drawn twice and lines of the same color cannot form a triangle if the end points are part of the outer corners. Click on the stick you want to color."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Dan Garcia & Sunil Ramesh, Peter Trethewey"
    set kTclAuthors "Peter Trethewey, David Chan"
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
    global p1Color p2Color
    return [list $p1Color $p2Color]
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

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    #$c configure -width 500 -height 500
    global bgColor
    global bgColor2
    global lineColor
    global edges
    global vertices
    global aval
    global gameover
    global hColor
    global dashSize

    global CANVAS_WIDTH CANVAS_HEIGHT

    $c create rect 0 0 $CANVAS_WIDTH $CANVAS_WIDTH -fill $bgColor -tag bkgnd
    $c create oval 0 0 $CANVAS_WIDTH $CANVAS_WIDTH -fill $bgColor2 -tag bkgnd2
    
    set i 0
    for {set x 0} {$x < $vertices} {incr x} {
	for {set y [expr {$x+1}]} {$y < $vertices} {incr y} {
	    set p 1.0
	    set q 1.0
	    set p [expr {$CANVAS_WIDTH/2.0}]
	    set q [expr {($CANVAS_WIDTH*0.9)/2.0}]
	    set pi 3.14159265

	    $c create line \
		[expr {$p+$q*cos($x*$pi/3)}] \
		[expr {$p+$q*sin($x*$pi/3)}] \
		[expr {$p+$q*cos($y*$pi/3)}] \
		[expr {$p+$q*sin($y*$pi/3)}] \
		-width 15 -disableddash $dashSize -activefill $hColor -fill $lineColor -capstyle round \
		-tag [list base base$i]

	    $c bind base$i <ButtonRelease-1> "ReturnFromHumanMove $i"
	    set aval($i) true
	    incr i
	}
    }
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
    global edges
    global lineColor
    global p1Color
    global p2Color
    global aval
    set a(0) 0
    unhash $position a

    for {set i 0} {$i < $edges} {incr i} {
	set aval($i) false

	if {$a($i) == "x"} {
	    $c itemconfigure base$i -fill $p1Color
      	} elseif {$a($i) == "o"} {
	    $c itemconfigure base$i -fill $p2Color
	} else {
	    $c itemconfigure base$i -fill $lineColor
	    set aval($i) true
	}
	$c raise base$i
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
    global gameover
    if { $gameover == true } {
	$c delete gameover
	set gameover false
    }
    GS_DrawPosition $c $position
}

#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################

proc GS_WhoseMove { position } {
    # code copied from mttt3.tcl
    global edges
    set a(0) 0

    unhash $position a
    set counter 0
    for {set i 0} {$i < $edges} {incr i} {
	if {$a($i) != "-"} {
	    incr counter
	}

	set val o
	if ( [expr $counter % 2] == 0 } {
	    set val x
	}

	return $val
}

#########################################################################
# used for animating lines, incomplete as of 5-22-06
#
#
########################################################################
proc animateLine { c  i } {
    global CANVAS_WIDTH
    global vertices
    global edges
    set temp 0

    for {set x 0} {$x < $vertices} {incr x} {
	for {set y [expr {$x+1}]} {$y < $vertices} {incr y} {
	    incr temp
	    if { $i == $temp } {
		break
	    }
	}
	
	if { $i == $temp } {
	  break
	}
    }

    puts "$x $y"
    set p 1.0
    set q 1.0
    set p [expr {$CANVAS_WIDTH/2.0}]
    set q [expr {($CANVAS_WIDTH*0.9)/2.0}]
    set pi 3.14159265
    
    set xDiff  [expr [expr {$p+$q*sin($x*$pi/3)}] - [expr {$p+$q*sin($y*$pi/3)}]]
    
    set yDiff [expr [expr {$p+$q*cos($y*$pi/3)}] -[expr {$p+$q*cos($x*$pi/3)}]] 
    puts "$xDiff $yDiff"
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
    global edges
    global p1Color
    global p2Color
    global lineColor
    global aval
    global lastMove
    global hColor

    set a(0) 0
    set b(0) 0
    unhash $oldPosition b
    unhash $newPosition a

    # hack used to show last move done
    $c itemconfigure base$lastMove -state normal
    set lastMove $theMove

    #iterate through the piece array setting the pieces
    for {set i 0} {$i < $edges} {incr i} {
	if { $a($i) != $b($i) }  {
	    set aval($i) false
	    if {$a($i) == "x"} {
		$c itemconfigure base$i -fill $p1Color -activefill $p1Color -state normal
		#animateLine $c $i
	    } elseif {$a($i) == "o"} {
		$c itemconfigure base$i -fill $p2Color -activefill $p2Color -state normal
	    } else {
		$c itemconfigure base$i -fill $lineColor -activefill $hColor -state normal
		set aval($i) true
	    }
	}
    }

    # toggle state of the recent move
    # if recent move is now a blank spot, we had an undo
    # else we highlight the move by enabling the disabled dash
    # note that on an undo we do not know what was the last made move
    # so we can not highlight it... fix if you know how
    # there has to be an array of moves that gamesman uses for undos
    # but I do not know what it is

    if {[$c itemcget base$lastMove -fill] == $lineColor} {
	$c itemconfigure base$lastMove -state normal
    } else {
	$c itemconfigure base$lastMove -state disabled
    }
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
		set color red4
	    }
	$c itemconfigure base$move -fill $color	
	}	
    }
}

#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################

proc GS_HideMoves { c moveType position moveList} {
    GS_DrawPosition $c $position
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
    GS_HandleMove $c $currentPosition $theMoveToUndo $positionAfterUndo
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
    global gameover

    set gameover true
    
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
    global gameover
    set gameover false
    $c delete gameover
}
