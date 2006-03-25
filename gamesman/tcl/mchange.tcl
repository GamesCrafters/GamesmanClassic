####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################
#
# NAME:           mchange.tcl
#  
# DESCRIPTION:    GUI for Change!
# 
# AUTHOR:         Alice Chang, Bryon Ross - University of California at Berkeley
#                 Copyright (C) Alice Chang, Bryon Ross 2004.  All rights reserved.
#   
# DATE:           2004-05-02
#
# UPDATE HISTORY: 2004-05-03
#
####################################################

proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}


# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

proc GS_InitGameSpecific {} {
    
    # Author's info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors gFrameWidth gFrameHeight
    set kCAuthors "Alice Chang, Judy Tuan"
    set kTclAuthors "Alice Chang, Bryon Ross"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-31x232.gif"

    # Set the name of the game
    global kGameName
    set kGameName "Change!"

    # Canvas size
    global CanvasWidth CanvasHeight
    set CanvasWidth [min $gFrameWidth $gFrameHeight]
    set CanvasHeight $CanvasWidth

    # Set the initial position of the board
    global gInitialPosition gPosition
    set gInitialPosition 9388804
    set gPosition $gInitialPosition

    # Set toMove and toWin
    global gMisereGame gToTrapIsToWin

    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    if {[expr $gToTrapIsToWin ^ $gMisereGame]} {
	set toWin3 " or trap the opponent"
    } else {
	set toWin3 " or get trapped by the opponent"
    }
    
    set toWin2 "Occupy your opponent's initial spaces " 
    
    SetToWinString [concat $toWin1 $toWin2 $toWin3]
    
    SetToMoveString "To Move: Players alternate turns by sliding one of their pieces along the line towards the opponent's starting point. On your turn you may move either one or two spaces forward, but you cannot turn corners, jump or go backwards."
}


# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {
    return [list x o]
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
    return [list blue red]
}


# Setup the rules frame
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

proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule [list "What would you like your winning condition to be:" "Standard" "Misere"]
    set trapRule [list "What kind of strategy would you like:" "To trap is to lose" "To trap is to win"]
    
    # List of all rules, in some order
    set ruleset [list $standardRule $trapRule]

    # Declare and initialize rule globals
    global gMisereGame gToTrapIsToWin
    set gMisereGame 0
    set gToTrapIsToWin 1

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gToTrapIsToWin"]

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


# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code

proc GS_GetOption { } {
    global gMisereGame
    global gToTrapIsToWin
    set option 1
    set option [expr $option + $gMisereGame]
    set option [expr $option + $gToTrapIsToWin * 2]
    return $option
}


# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing

proc GS_SetOption { option } {
    global gMisereGame gToTrapIsToWin
    set option [expr $option - 1]
    set gMisereGame [expr $option % 2]
    set gToTrapIsToWin [expr $option / 2 % 2]
}


# GS_Initialize draws the graphics for the game on the canvas c
# You could put an opening animation in this function
# or just draw an empty board.
# This function is called after GS_InitGameSpecific

proc GS_Initialize { c } {

    # You may want to start by setting the size of the canvas; this line isn't necessary
    global CanvasWidth CanvasHeight
    $c configure -width $CanvasWidth -height $CanvasHeight
    $c delete all

    # Draw the board
    DrawBoard $c
}


# DrawBoard creates the lines that make up the board, and all the pieces and arrows. 
# This function is called by GS_Initialize.

proc DrawBoard { c } {
    
    # Draw base for hiding things
    global CanvasWidth CanvasHeight CoordsList r
    $c create rect 0 0 $CanvasWidth $CanvasHeight -fill white -outline white -tag base

    # Piece radius
    set r [expr $CanvasWidth / 20]
    # Arrow width
    set arrowwidth [expr $CanvasWidth / 50]
    # Arrow shape
    set arrowshape [list $arrowwidth $arrowwidth [expr $arrowwidth / 2]]
    
    # Measurements for finding the slots
    set SideGap [expr int($CanvasHeight / 10)]
    set xGap [expr int(($CanvasWidth - 2 * $SideGap) / 3)]
    set yGap [expr int(($CanvasHeight - 2 * $SideGap) / 7)]
    
    # The slots are labeled like this:
    #      7       
    #    /   \    
    # 11       3
    #    \   /   \
    #      8       0
    #    /   \   /
    # 12       4
    #    \   /   \ 
    #      9      1
    #    /   \   /
    # 13       5 
    #    \   /   \
    #     10       2
    #        \   /
    #          6
    # The slots are this way because the original board in mchange.c is rotated counterclockwise 90 degrees.
    
    # Coordinates of the 14 slots
    set x0 [expr $SideGap + 3 * $xGap]
    set x1 $x0
    set x2 $x0
    set x3 [expr $SideGap + 2 * $xGap]
    set x4 $x3
    set x5 $x3
    set x6 $x3
    set x7 [expr $SideGap + $xGap]
    set x8 $x7
    set x9 $x7
    set x10 $x7
    set x11 $SideGap
    set x12 $x11
    set x13 $x11
    set y0 [expr $SideGap + 2 * $yGap]
    set y1 [expr $SideGap + 4 * $yGap]
    set y2 [expr $SideGap + 6 * $yGap]
    set y3 [expr $SideGap + $yGap]
    set y4 [expr $SideGap + 3 * $yGap]
    set y5 [expr $SideGap + 5 * $yGap]
    set y6 [expr $SideGap + 7 * $yGap]
    set y7 $SideGap
    set y8 $y0
    set y9 $y1
    set y10 $y2
    set y11 $y3
    set y12 $y4
    set y13 $y5

    # Draw the lines slanting downwards from top left to bottom right
    $c create line $x7 $y7 $x0 $y0 -fill black -width 5 -tag base
    $c create line $x11 $y11 $x1 $y1 -fill black -width 5 -tag base
    $c create line $x12 $y12 $x2 $y2 -fill black -width 5 -tag base
    $c create line $x13 $y13 $x6 $y6 -fill black -width 5 -tag base
		      
    # Draw all the lines slanting upwards from bottom left to top right
    $c create line $x11 $y11 $x7 $y7 -fill black -width 5 -tag base
    $c create line $x12 $y12 $x3 $y3 -fill black -width 5 -tag base
    $c create line $x13 $y13 $x0 $y0 -fill black -width 5 -tag base
    $c create line $x10 $y10 $x1 $y1 -fill black -width 5 -tag base
    $c create line $x6 $y6 $x2 $y2 -fill black -width 5 -tag base

    # Put the coordinates for all 14 slots in a list
    # This makes drawing pieces and arrows easier
    set CoordsList [list $x0 $y0 $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 \
			$x7 $y7 $x8 $y8 $x9 $y9 $x10 $y10 $x11 $y11 $x12 $y12 $x13 $y13] 
    
    # Draw the slots
    for {set i 0} {$i < 27} {incr i 2} {
	set x [lindex $CoordsList $i]
	set y [lindex $CoordsList [expr $i + 1]]
	$c create oval [expr $x - 10] [expr $y - 10] [expr $x + 10] [expr $y + 10] \
	    -fill black -tag [list base slots]
    }

    # 2006-02-27 Change x's to o's as well, just of a different color (blue)
    for {set i 0} {$i < 27} {incr i 2} {
	set x [lindex $CoordsList $i]
    	set y [lindex $CoordsList [expr $i + 1]]
    	$c create oval [expr $x - $r] [expr $y - $r] [expr $x + $r] [expr $y + $r] \
    	    -fill blue -tag x[expr $i/2]
    } 

    # Draw the red o's
    for {set i 0} {$i < 27} {incr i 2} {
	set x [lindex $CoordsList $i]
	set y [lindex $CoordsList [expr $i + 1]]
	$c create oval [expr $x - $r] [expr $y - $r] [expr $x + $r] [expr $y + $r] \
	    -fill red -tag o[expr $i / 2]
    }
    
    # Draw the full and half arrows 
    for {set i 0} {$i < 27} {incr i 2} {
	set x [lindex $CoordsList $i]
	set y [lindex $CoordsList [expr $i + 1]]
	set Upx [expr $x - $xGap]
	set Upy [expr $y - $yGap]
	set HalfUpx [expr $x - $xGap / 2]
	set HalfUpy [expr $y - $yGap  / 2]
	set Downx $Upx
	set Downy [expr $y + $yGap]
	set HalfDownx $HalfUpx
	set HalfDowny [expr $y + $yGap / 2]
	set from [expr $i / 2]
	for {set j 0} {$j < 27} {incr j 2} {
	    set xTemp [lindex $CoordsList $j]
	    set yTemp [lindex $CoordsList [expr $j + 1]]
	    set to [expr $j / 2]
	    if {($xTemp == $Upx) && ($yTemp == $Upy)} {
		$c create line $x $y [expr $Upx + 10] [expr $Upy + 4] \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag fullarrow$from-$to
		$c create line $Upx $Upy [expr $x - 10] [expr $y - 4] \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag fullarrow$to-$from
		$c create line $HalfUpx $HalfUpy $Upx $Upy\
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag halfarrow$from-$to
		$c create line $HalfUpx $HalfUpy $x $y \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag halfarrow$to-$from
	    } elseif {($xTemp == $Downx) && ($yTemp == $Downy)} {
		$c create line $x $y [expr $Downx + 10] [expr $Downy - 4] \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag fullarrow$from-$to
		$c create line $Downx $Downy [expr $x - 10] [expr $y + 4] \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag fullarrow$to-$from
		$c create line $HalfDownx $HalfDowny $Downx $Downy \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag halfarrow$from-$to
		$c create line $HalfDownx $HalfDowny $x $y \
		    -arrow last -arrowshape $arrowshape -width $arrowwidth -fill cyan -tag halfarrow$to-$from
	    }
	}
    }    
    
    $c lower all
    $c raise base
}


proc GS_Deinitialize { c } {
    $c delete all
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

proc Unhash { position } {
    set TempPos $position
    set Board []
    for {set i 0} {$i < 14} {incr i} {
	lappend Board [expr $TempPos % 3]
	set TempPos [expr $TempPos / 3]
    }
    return $Board
}

proc GS_DrawPosition { c position } {
    $c raise base all
    set Board [Unhash $position]
    set i 0
    foreach item $Board {
        if {$item == 0} {
	    $c lower x$i
	    $c lower o$i
	} elseif {$item == 1} {
	    $c raise o$i
	} else {
	    $c raise x$i
	}
	incr i
    }
    update idletasks
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

}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {

    # Redraw board to remove arrows
    $c lower all
    $c raise base
    GS_DrawPosition $c $oldPosition

    global CoordsList r
    set start [expr $theMove % 15]
    set end [expr $theMove / 15]

    set x0 [lindex $CoordsList [expr 2 * $start]]
    set y0 [lindex $CoordsList [expr 2 * $start + 1]]
    set x1 [lindex $CoordsList [expr 2 * $end]]
    set y1 [lindex $CoordsList [expr 2 * $end + 1]]

    # Four steps:
    # 1) Draw dummy shape at x0, y0
    # 2) Hide actual piece at x0, y0
    # 3) Animate dummy from x0, y0 to x1, y1
    # 4) Kill dummy, draw new position

    set oldboard [Unhash $oldPosition]
    set piece [string index $oldboard [expr 2 * $start]]

    # 1 = red (o), 2 = blue (x)
    if { $piece == 1 } {
	set color "red"
	$c lower o$start
    } else {
	set color "blue"
	$c lower x$start
    }

    $c create oval [expr $x0 - $r] [expr $y0 - $r] [expr $x0 + $r] [expr $y0 + $r] -fill $color -tag move
    animate $c move [list $x0 $y0] [list $x1 $y1]
    $c delete move

    GS_DrawPosition $c $newPosition
}

####################################################
# This animates a piece from origin to destination #
####################################################
proc animate { c piece origin destination } {

    global gAnimationSpeed

    set x0 [lindex $origin 0]
    set x1 [lindex $destination 0]
    set y0 [lindex $origin 1]
    set y1 [lindex $destination 1]

    # Relative speed factor gotten from gAnimationSpeed
    # speed should equal the amount of ms we take to run this whole thing
    set speed [expr 70 / pow(2, $gAnimationSpeed)]
    
    # If things get too fast, just make it instant
    if {$speed < 10} {
	set speed 10
    }

    set dx [expr ($x1 - $x0) / $speed]
    set dy [expr ($y1 - $y0) / $speed]

    for {set i 0} {$i < $speed} {incr i} {
	$c move $piece $dx $dy
	
	after 1
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
    set i 0
    set Moves []
    foreach item $moveList {
	set i [lindex $item 0]
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
	
	set from [expr $i % 15]
	set to [expr $i / 15]
	if {$from > $to} {
	    set max $from
	    set min $to
	} else {
	    set max $to
	    set min $from
	}
	if {($min <= 2) && ($max >= 7) && ($max <= 10)} {
	    set startArrow [expr ($min + $max) / 2]
	} elseif {($max >= 11) && ($min >= 3) && ($min <= 6)} {
	    set startArrow [expr ($min + $max) / 2 + 1]
	} elseif {($max >= 11) && ($min <= 2)} {
	    if {($to == 11) || ($to == 12)} {
		set startArrow [expr $to - 3]
	    } elseif {($to == 1) || ($to == 2)} {
		set startArrow [expr $to + 3]
	    } elseif {$to == 0} {
		set startArrow 4
	    } else {
		set startArrow 9
	    }
	} else {
	    set startArrow $from
	}

	lappend Moves $i
	if {[lsearch $Moves [expr $from * 15 + $to]] == -1} {
	    $c raise fullarrow$startArrow-$to base
	    $c raise slots fullarrow$startArrow-$to
	    $c itemconfigure fullarrow$startArrow-$to -fill $color
	    $c bind fullarrow$startArrow-$to <Enter> "$c itemconfigure fullarrow$startArrow-$to -fill black"
	    $c bind fullarrow$startArrow-$to <Leave> "$c itemconfigure fullarrow$startArrow-$to -fill $color"
	    $c bind fullarrow$startArrow-$to <1> "ReturnFromHumanMove $i"
	} else {
	    $c lower fullarrow$to-$startArrow
	    $c raise halfarrow$startArrow-$to base
	    $c raise halfarrow$to-$startArrow base
	    $c raise slots halfarrow$startArrow-$to
	    $c itemconfigure halfarrow$startArrow-$to -fill $color
	    $c itemconfigure halfarrow$to-$startArrow -fill [$c itemcget fullarrow$to-$startArrow -fill]
	    $c bind halfarrow$startArrow-$to <Enter> "$c itemconfigure  halfarrow$startArrow-$to -fill black"
	    $c bind halfarrow$to-$startArrow <Enter> "$c itemconfigure  halfarrow$to-$startArrow -fill black"
	    $c bind halfarrow$startArrow-$to <Leave> "$c itemconfigure  halfarrow$startArrow-$to -fill $color"
	    $c bind halfarrow$to-$startArrow <Leave> "$c itemconfigure  halfarrow$to-$startArrow -fill [$c itemcget fullarrow$to-$startArrow -fill]"
	    $c bind halfarrow$startArrow-$to <1> "ReturnFromHumanMove $i"
	    $c bind halfarrow$to-$startArrow <1> "ReturnFromHumanMove $i"
	}
    }
    update idletasks
}


# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    $c lower arrows all
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

    # Redraw board to remove arrows
    $c lower all
    $c raise base
    GS_DrawPosition $c $currentPosition

    global CoordsList r
    set end [expr $theMoveToUndo % 15]
    set start [expr $theMoveToUndo / 15]

    set x0 [lindex $CoordsList [expr 2 * $start]]
    set y0 [lindex $CoordsList [expr 2 * $start + 1]]
    set x1 [lindex $CoordsList [expr 2 * $end]]
    set y1 [lindex $CoordsList [expr 2 * $end + 1]]

    set oldboard [Unhash $currentPosition]
    set piece [string index $oldboard [expr 2 * $start]]

    # 1 = red (o), 2 = blue (x)
    if { $piece == 1 } {
	set color "red"
	$c lower o$start
    } else {
	set color "blue"
	$c lower x$start
    }

    $c create oval [expr $x0 - $r] [expr $y0 - $r] [expr $x0 + $r] [expr $y0 + $r] -fill $color -tag move
    animate $c move [list $x0 $y0] [list $x1 $y1]
    $c delete move

    GS_DrawPosition $c $positionAfterUndo
}


# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}

#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
    
    global CanvasWidth
    set size $CanvasWidth
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
