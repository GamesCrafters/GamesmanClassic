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
    
    global kGameName basespeed
    set kGameName "Dodgem"
    set basespeed 50
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 17524
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to move both of his or her pieces off of the board into his or her goal area first and avoid trapping his or her opponent so that he or she may not move WINS"
    set kMisereString "First player to move both of his or her pieces off of the board into his or her goal area first and avoid trapping his or her opponent so that he or she may not move LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Move both of your pieces off of the board into your goal area first and avoid trapping your opponent so that he may not move."
    } else {
	SetToWinString "To Win: Avoid moving both of your pieces off the board into your goal area first or trap your opponent so that he may not move."
    }
    SetToMoveString "To Move: The pieces are intially arranged at a right angle. Players alternate turns moving their pieces to an adjacent open space. The player with pieces lined up vertically may move north, east, or south. The player with his or her pieces lind up horizontally may move north, east or west. Click on the arrow that moves the piece in the direction you wish to move. Note that you can only move to an adjacent UNOCCUPIED square and you may never move away from your goal"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Dan Garcia"
    set kTclAuthors "Eudean Sun"
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
    global gMisereGame gToTrapIsToWin
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

proc min { x y } {
    if { $x < $y } {
	return $x
    }
    return $y
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

    global gFrameWidth gFrameHeight size r gInitialPosition

    set size [min $gFrameWidth $gFrameHeight]
    set r [expr 0.4 * ($size / 4)]
    set goalfontsize [expr int(0.03 * $size)]
    set smallr [expr 0.12 * ($size / 4)]

    # Draw a gray background
    $c create rect 0 0 [expr $size-1] [expr $size-1] -fill gray

    # Draw a 4x4 grid
    for { set i 0 } { $i < 3 } { incr i } {
	$c create line 0 [expr ($i + 1) * ($size / 4)] $size [expr ($i + 1) * ($size / 4)] -width 3 -fill "\#333333"
	$c create line [expr ($i + 1) * ($size / 4)] 0 [expr ($i + 1) * ($size / 4)] $size -width 3 -fill "\#333333"
    }

    # Draw the goals
    for { set i 0 } { $i < 3 } { incr i } {
	$c create oval [expr (2*$i + 1) * ($size / 8) - $r] [expr $size / 8 - $r] [expr (2*$i + 1) * ($size / 8) + $r] [expr $size / 8 + $r] -fill "\#FF8888"
	$c create oval [expr $size * 7/8 - $r] [expr (2*$i + 3) * ($size / 8) - $r] [expr $size * 7/8 + $r] [expr (2*$i + 3) * ($size / 8) + $r] -fill lightblue
    }

    # Write "The Red Goal" and "The Blue Goal"
    $c create text [expr $size / 8] [expr $size / 8] -text "The" -fill "White" -font "Arial $goalfontsize"
    $c create text [expr $size * 3/8] [expr $size / 8] -text "Red" -fill "White" -font "Arial $goalfontsize"
    $c create text [expr $size * 5/8] [expr $size / 8] -text "Goal" -fill "White" -font "Arial $goalfontsize"
    $c create text [expr $size * 7/8] [expr $size * 3/8] -text "The" -fill "White" -font "Arial $goalfontsize"
    $c create text [expr $size * 7/8] [expr $size * 5/8] -text "Blue" -fill "White" -font "Arial $goalfontsize"
    $c create text [expr $size * 7/8] [expr $size * 7/8] -text "Goal" -fill "White" -font "Arial $goalfontsize"

    # Block out upper right square
    $c create rect [expr $size * 3/4] 0 $size [expr $size * 1/4] -fill "\#555555"

    # Legend
    set redx [expr $size * 51/64]
    set redy [expr $size / 8]
    set bluex [expr $size * 7/8]
    set bluey [expr $size * 13/64]
    set arrowwidth [expr $smallr * 0.4]
    set arrowhead [list [expr 2*$arrowwidth] [expr 2*$arrowwidth] $arrowwidth]
    set legendfontsize [expr int(0.03 * $size)]
    $c create line $redx $redy $redx [expr $redy + $size/13] -arrow last -width $arrowwidth -arrowshape $arrowhead -fill cyan
    $c create line $redx $redy $redx [expr $redy - $size/13] -arrow last -width $arrowwidth -arrowshape $arrowhead -fill cyan
    $c create line $redx $redy [expr $redx + $size/13] $redy -arrow last -width $arrowwidth -arrowshape $arrowhead -fill cyan
    $c create line $bluex $bluey $bluex [expr $bluey - $size/13] -arrow last -width $arrowwidth -arrowshape $arrowhead -fill cyan
    $c create line $bluex $bluey [expr $bluex + $size/13] $bluey -arrow last -width $arrowwidth -arrowshape $arrowhead -fill cyan
    $c create line $bluex $bluey [expr $bluex - $size/13] $bluey -arrow last -width $arrowwidth -arrowshape $arrowhead -fill cyan
    $c create oval [expr $redx - $smallr] [expr $redy - $smallr] [expr $redx + $smallr] [expr $redy + $smallr] -fill red
    $c create oval [expr $bluex - $smallr] [expr $bluey - $smallr] [expr $bluex + $smallr] [expr $bluey + $smallr] -fill blue
    $c create text $bluex [expr $size * 3/64] -text "Legend" -fill "White" -font "Arial $legendfontsize"

    # Draw the Initial Position
    GS_DrawPosition $c $gInitialPosition

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
    
    global size r

    set board [unhash $position]

    DrawBoard $c $board

}

proc DrawBoard { c board } {

    global size r

    # Clear the board of pieces
    $c delete pieces

    # Draw in the new pieces, iterating over board
    for { set i 0 } { $i < 9 } { incr i } {
	set piece [string index $board $i]
	set row [expr int($i / 3)]
	set col [expr $i % 3]
	set x [expr (2*$col + 1) * ($size / 8)]
	set y [expr (2*$row + 3) * ($size / 8)]
	if { [string equal $piece "x"] } {
	    $c create oval [expr $x-$r] [expr $y-$r] [expr $x+$r] [expr $y+$r] -fill red -tag pieces
	} elseif { [string equal $piece "o"] } {
	    $c create oval [expr $x-$r] [expr $y-$r] [expr $x+$r] [expr $y+$r] -fill blue -tag pieces
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

    global turn r size

    set move [unhashmove $theMove]
    set from [lindex $move 0]
    set to [lindex $move 1]
    set rowi [expr $from / 3]
    set coli [expr $from % 3]
    set rowf [expr $to / 3]
    set colf [expr $to % 3]
    set xi [expr (2*$coli + 1) * ($size / 8)]
    set yi [expr (2*$rowi + 3) * ($size / 8)]
    set xf [expr (2*$colf + 1) * ($size / 8)]
    set yf [expr (2*$rowf + 3) * ($size / 8)]

    # Again, we must handle the special case of to "9"
    if { $to == 9 } {
	if { [string equal $turn "x"] } {
	    set yf [expr $yi - $size/4]
	    set xf $xi
	} else {
	    set xf [expr $xi + $size/4]
	    set yf $yi
	}
    }

    # Clear board, draw oldPosition with a blank at "from"
    $c delete pieces
    set oldboard [unhash $oldPosition]
    set oldboard [string replace $oldboard $from $from "-"]
    DrawBoard $c $oldboard

    # Need the color of the piece
    if { [string equal $turn "x"] } {
	set color red
    } else {
	set color blue
    }

    # Draw the dummy to animate, then animate it
    set piece [$c create oval [expr $xi-$r] [expr $yi-$r] [expr $xi+$r] [expr $yi+$r] -fill $color -tag dummy]

    animate $c $piece [list $xi $yi] [list $xf $yf]

    # Draw new position
    GS_DrawPosition $c $newPosition
    
    # Kill dummy
    $c delete dummy
}

####################################################
# This animates a piece from origin to destination #
####################################################
proc animate { c piece origin destination } {

    global basespeed gAnimationSpeed

    set x0 [lindex $origin 0]
    set x1 [lindex $destination 0]
    set y0 [lindex $origin 1]
    set y1 [lindex $destination 1]

    # Relative speed factor gotten from gAnimationSpeed
    # speed should equal the amount of ms we take to run this whole thing
    set speed [expr $basespeed / pow(2, $gAnimationSpeed)]
    
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
proc GS_ShowMoves { c moveType position moveList} {

    $c delete arrows

	
    foreach m $moveList {

	if { [string equal $moveType "value"] } {
	    set value [lindex $m 1]
		set remoteness "off"
	} elseif { [string equal $moveType "rm"] } {
		set value [lindex $m 1]
		set remoteness [lindex $m 3]
	} else {
	    set value "off"
		set remoteness "off"
	}
	
	#if { $delta } {
	#    set remoteness [lindex $m 3]
	#} else {
	#    set remoteness "off"
	#}
	
	DrawArrow $c $m $value $remoteness
    }

    $c raise pieces
}

proc DrawArrow { c m value remoteness } {

    global size r turn gMisereGame
	global tk_library kRootDir gStippleRootDir
	
	set gStippleRootDir "$kRootDir/../bitmaps/"

    set move [unhashmove [lindex $m 0]]
    set from [lindex $move 0]
    set to [lindex $move 1]
    set arrowwidth [expr 0.25 * $r]
    set arrowhead [list [expr 2*$arrowwidth] [expr 2*$arrowwidth] $arrowwidth]
	set maxRemoteness [expr 0.25 * $r]
	set factor [expr $maxRemoteness/5]
	set stipple12 @[file join $gStippleRootDir gray12.bmp] 
	set stipple25 @[file join $gStippleRootDir gray25.bmp]
	set stipple50 @[file join $gStippleRootDir gray50.bmp]
	set stipple75 @[file join $gStippleRootDir gray75.bmp]
	
	#if {$remoteness < [expr $factor*1] } {
	#	set stipple ""
	#} elseif {$remoteness < [expr $factor*2] } {
	#	set stipple $stipple75
	#} elseif {$remoteness < [expr $factor*3] } {
	#	set stipple $stipple50
	#} elseif {$remoteness < [expr $factor*4] } {
	#	set stipple $stipple25
	#} else {
	#	set stipple $stipple12
	#}
    if { $remoteness != "off" } {
        set delta [expr $remoteness / 2]
    } else {
        set delta 0
    }
    
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
	
	if {$remoteness == 0} {
	    set width $maxRemoteness
	} elseif {$remoteness > $maxRemoteness} {
	    set width 1
	} else {
	    set width [expr $maxRemoteness - $remoteness]
	}
	
    if { !$gMisereGame } {
	switch $value {
	    Win { set color darkred }
	    Lose { set color green }
	    Tie { set color yellow }
	    default { set color cyan }
	}
    } else {
	switch $value {
	    Win { set color green }
	    Lose { set color darkred }
	    Tie { set color yellow }
	    default { set color cyan }
	}
    }

    set rowi [expr $from / 3]
    set coli [expr $from % 3]
    set rowf [expr $to / 3]
    set colf [expr $to % 3]
    set xi [expr (2*$coli + 1) * ($size / 8)]
    set yi [expr (2*$rowi + 3) * ($size / 8)]
    set xf [expr (2*$colf + 1) * ($size / 8)]
    set yf [expr (2*$rowf + 3) * ($size / 8)]

    # We have a special case if we're moving to "9", or off the board
    # If blue's turn, draw arrow from "from" vertically up
    # If red's turn, draw arrow from "from" horizontally right

    if { $to == 9 } {
	if { [string equal $turn "x"] } {
	    set yf [expr $yi - $size/4]
	    set xf $xi
	} else {
	    set xf [expr $xi + $size/4]
	    set yf $yi
	}
    }

   	if { $remoteness == "off" } {
		set arrow [$c create line $xi $yi $xf $yf -width $arrowwidth -fill $color -arrowshape $arrowhead -arrow last -stipple "" -tag arrows]
	} else {
		set arrow [$c create line $xi $yi $xf $yf -width $arrowwidth -fill $color -arrowshape $arrowhead -arrow last -stipple $stipple -tag arrows]
	    #set arrow [$c create line $xi $yi $xf $yf -width $width -fill $color -arrowshape $arrowhead -arrow last -tag arrows]
	}
	
    $c bind $arrow <Enter> "$c itemconfigure $arrow -fill black"
    $c bind $arrow <Leave> "$c itemconfigure $arrow -fill $color"
    $c bind $arrow <ButtonRelease-1> "ReturnFromHumanMove [lindex $m 0]"

}

#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    $c delete arrows

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

proc unhash { position } {

    global turn

    set offset 19683
    set x 2
    set o 1
    set b 0
    set board "---------"

    for { set i 0 } { $i < 9 } { incr i } {
	set g3Array($i) [expr int(pow(3, $i))]
    }

    if { $position >= $offset } {
	set position [expr $position - $offset]
	set turn "x"
    } else {
	set turn "o"
    }

    for { set i 8 } { $i >= 0 } { incr i -1 } {
	if { $position >= $x * $g3Array($i) } {
	    set board [string replace $board $i $i "x"]
	    set position [expr $position - $x * $g3Array($i)]
	} elseif { $position >= $o * $g3Array($i) } {
	    set board [string replace $board $i $i "o"]
	    set position [expr $position - $o * $g3Array($i)]
	} else {
	    set board [string replace $board $i $i "-"]
	    set position [expr $position - $b * $g3Array($i)]
	}
    }

    return $board
}

proc unhashmove { move } {
    # Returns list (from to)
    return [list [expr $move % 10] [expr $move / 10]]
}
