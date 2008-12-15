####################################################
#
# masalto.tcl
#
# Asalto GUI, Tck/Tk
#
# C authored by Robert Liao and Michael Chen
# Tcl/Tk authored by Eudean Sun
#
####################################################

# Game parameters
# Note: Other parameters in DrawBoard

# t = line thickness
set t 5
# dot = dot radius (at intersections)
set dot 10
# color = color of the board
set color "black"
# outlineColor = color of piece outlines
set outlineColor "black"
# outline = size of piece outlines
set outline 3
# player1 = color of player 1's pieces (geese)
set player1 "blue"
# player2 = color of player 2's pieces (foxes)
set player2 "red"
# arrowlength = relative length of arrows (100% would be completely across a square)
set arrowlength 0.7
# arrowcolor = default color of arrows showing moves
set arrowcolor cyan
# basespeed = base speed of the animations in milliseconds (values 10-1000, larger is faster)
set basespeed 100

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
    set kGameName "Asalto"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ______ WINS"
    set kMisereString "First player to ______ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: As the foxes, your goal is to capture one of the geese before they occupy the castle area. As the geese, your goal is to occupy the castle area before being captured. The geese may also trap a fox to win."
    } else {
	SetToWinString "To Win: As the foxes, your goal is to avoid capturing one of the geese and to attempt to be trapped by the geese. As the geese, your goal is to attempt to be captured, avoid filling the castle area, and avoid trapping a fox."
    }
    SetToMoveString "To Move: Possible moves are indicated by arrows around your pieces. Click on one of these arrows to move in the direction indicated."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Robert Liao, Michael Chen"
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

    return [list Foxes Geese]

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
    
    global player1 player2

    return [list $player1 $player2]
    
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
    
    global side margin size color outlineColor outline t dot r player1 player2 fontsize
    
    # Save the GC background skin
    $c addtag background all

    set board "GGG               F F"
    
    # Draw the default board with default piece configuration
    DrawBoard $c
    DrawPieces $c $board

    # Finally, draw a title
    $c create rectangle 0 [expr $size/2 - 50] $size [expr $size/2 + 50] -fill gray -width 1 -outline black
    $c create text [expr $size/2] [expr $size/2] -text "Welcome to Asalto!" -font "Arial $fontsize" -anchor center -fill black -width [expr 4*$side] -justify center

} 


#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}

proc max { a b } {
    if {$a > $b} {
	return $a
    }
    return $b
}

proc min { a b } {
    if {$a < $b} {
	return $a
    }
    return $b
}

##################################################
# DrawBoard - Draws the board layout on canvas c #
##################################################
proc DrawBoard { c } {

    global side margin size color outlineColor outline t dot r player1 player2 gFrameWidth gFrameHeight fontsize arrowwidth arrowhead

    # If we don't clean the canvas, things pile up and get slow
    # We need to keep the background, though (otherwise we get ugly borders around game on some resolutions)
    $c delete {!background}
    
    set size [expr int([min $gFrameWidth $gFrameHeight])]
    set margin [expr 0.1*$size]
    set side [expr ($size-2*$margin)/4]
    set r [expr 0.33*$side]
    set fontsize [expr int($size / 20)]
    set arrowwidth [expr 0.4 * $r]
    set arrowhead [list [expr 2 * $arrowwidth] [expr 2 * $arrowwidth] $arrowwidth]

    # Make a gray background
    $c create rect 0 0 $size $size -fill darkgray -width 1 -outline black

    # Draw the default board

    # Draw horizontal board lines (top to bottom)
    $c create line [expr $margin+$side] [expr $margin] [expr $margin+3*$side] [expr $margin] -width $t -fill $color
    $c create line [expr $margin] [expr $margin+$side] [expr $margin+4*$side] [expr $margin+$side] -width $t -fill $color
    $c create line [expr $margin] [expr $margin+2*$side] [expr $margin+4*$side] [expr $margin+2*$side] -width $t -fill $color
    $c create line [expr $margin] [expr $margin+3*$side] [expr $margin+4*$side] [expr $margin+3*$side] -width $t -fill $color
    $c create line [expr $margin+$side] [expr $margin+4*$side] [expr $margin+3*$side] [expr $margin+4*$side] -width $t -fill $color
    
    # Draw vertical board lines (left to right)
    $c create line [expr $margin] [expr $margin+$side] [expr $margin] [expr $margin+3*$side] -width $t -fill $color
    $c create line [expr $margin+$side] [expr $margin] [expr $margin+$side] [expr $margin+4*$side] -width $t -fill $color
    $c create line [expr $margin+2*$side] [expr $margin] [expr $margin+2*$side] [expr $margin+4*$side] -width $t -fill $color
    $c create line [expr $margin+3*$side] [expr $margin] [expr $margin+3*$side] [expr $margin+4*$side] -width $t -fill $color
    $c create line [expr $margin+4*$side] [expr $margin+$side] [expr $margin+4*$side] [expr $margin+3*$side] -width $t -fill $color
    
    # Draw diagonal board lines
    # top to right
    $c create line [expr $margin+2*$side] [expr $margin] [expr $margin+4*$side] [expr $margin+2*$side] -width $t -fill $color
    # top to left
    $c create line [expr $margin+2*$side] [expr $margin] [expr $margin] [expr $margin+2*$side] -width $t -fill $color
    # bot to right
    $c create line [expr $margin+2*$side] [expr $margin+4*$side] [expr $margin+4*$side] [expr $margin+2*$side] -width $t -fill $color
    # bot to left
    $c create line [expr $margin+2*$side] [expr $margin+4*$side] [expr $margin] [expr $margin+2*$side] -width $t -fill $color
    # top-left to bot-right
    $c create line [expr $margin+$side] [expr $margin+$side] [expr $margin+3*$side] [expr $margin+3*$side] -width $t -fill $color
    # bot-left to top-right
    $c create line [expr $margin+$side] [expr $margin+3*$side] [expr $margin+3*$side] [expr $margin+$side] -width $t -fill $color
    
    # Draw dots at intersections of board lines
    # Top and bottom row
    for {set i 1} {$i < 4} {incr i} {
	$c create oval [expr $margin+$i*$side-$dot] [expr $margin-$dot] [expr $margin+$i*$side+$dot] [expr $margin+$dot] -fill $color -width 0
	$c create oval [expr $margin+$i*$side-$dot] [expr $margin+4*$side-$dot] [expr $margin+$i*$side+$dot] [expr $margin+4*$side+$dot] -fill $color -width 0
    }
    # Middle rows
    for {set i 0} {$i < 5} {incr i} {
	$c create oval [expr $margin+$i*$side-$dot] [expr $margin+$side-$dot] [expr $margin+$i*$side+$dot] [expr $margin+$side+$dot] -fill $color -width 0
	$c create oval [expr $margin+$i*$side-$dot] [expr $margin+2*$side-$dot] [expr $margin+$i*$side+$dot] [expr $margin+2*$side+$dot] -fill $color -width 0
	$c create oval [expr $margin+$i*$side-$dot] [expr $margin+3*$side-$dot] [expr $margin+$i*$side+$dot] [expr $margin+3*$side+$dot] -fill $color -width 0
    }
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
    
    ### TODO: Fill this in
    set board [unhash $position]

    DrawBoard $c
    DrawPieces $c $board

}

####################################
# Unhashes a position into a board #
####################################
proc unhash { position } {
    return [C_GenericUnhash [expr $position >> 2] 21]
}

#############################
# Draws pieces on the board #
#############################
proc DrawPieces { c board } {
    for {set i 0} {$i < 21} {incr i} {
	draw [string index $board $i] $i $c
    }
}

######################################
# Draws a piece at index on canvas c #
######################################
proc draw { piece index c } {
    global r player1 player2 outline

    set location [coords $index]

    switch $piece {
	G { $c create oval [expr [lindex $location 0] - $r] [expr [lindex $location 1] - $r] [expr [lindex $location 0] + $r] [expr [lindex $location 1] + $r] -fill $player1 -width $outline}
	F { $c create oval [expr [lindex $location 0] - $r] [expr [lindex $location 1] - $r] [expr [lindex $location 0] + $r] [expr [lindex $location 1] + $r] -fill $player2 -width $outline}
	default {}
    }

}

########################################
# Returns coordinates of a board index #
########################################
proc coords { index } {
    global margin side
    set index [incr index]
    if { $index <= 3 } {
	return [list [expr $margin+$index*$side] $margin]
    } elseif { $index <= 8 } {
	return [list [expr $margin+($index-4)*$side] [expr $margin+$side]]
    } elseif { $index <= 13 } {
	return [list [expr $margin+($index-9)*$side] [expr $margin+2*$side]]
    } elseif { $index <= 18 } {
	return [list [expr $margin+($index-14)*$side] [expr $margin+3*$side]]
    } else {
	return [list [expr $margin+($index-18)*$side] [expr $margin+4*$side]]
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
    # but if you want you can add a special behavior here like an animation
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

    global r outline
    
    # Unhashes the move
    set origposition [expr $theMove & 0x3F]
    set origin [coords $origposition]
    set destination [coords [expr $theMove >> 6]]

    # Draw oldPosition with piece at origin gone
    DrawBoard $c
    set oldBoard [unhash $oldPosition]
    set movedpiece [string index $oldBoard $origposition]

    switch $movedpiece {
	F { set color red  }
	G { set color blue }
    }

    set oldBoard [string replace $oldBoard $origposition $origposition " "]
    set captured 0

    set newBoard [unhash $newPosition]
    for {set i 0} {$i < 21} {incr i} {
	if { [string index $oldBoard $i] == "G" && [string index $newBoard $i] == " " } {
	    set oldBoard [string replace $oldBoard $i $i " "]
	    set captured [coords $i]
	}
    }

    if {$captured != 0} {
	set captured [$c create oval \
			[expr [lindex $captured 0] - $r] \
			[expr [lindex $captured 1] - $r] \
			[expr [lindex $captured 0] + $r] \
			[expr [lindex $captured 1] + $r] \
			-fill blue -width $outline]
    }

    DrawPieces $c $oldBoard

    set piece [$c create oval [expr [lindex $origin 0] - $r] [expr [lindex $origin 1] - $r] [expr [lindex $origin 0] + $r] [expr [lindex $origin 1] + $r] -fill $color -width $outline]	       

    animate $c $piece $origin $destination $captured

    GS_DrawPosition $c $newPosition 
}

####################################################
# This animates a piece from origin to destination #
####################################################
proc animate { c piece origin destination captured } {

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
	
	if {$captured != 0} {
	    if {$i == [expr floor($speed / 2)]} {
		$c delete $captured
	    }
	}
	after 1
	update idletasks
    }
}

########################################################################################
# This animates an undo (similar to animate, but reverses the capture if there is one) #
########################################################################################
proc undo { c piece origin destination uncaptured } {

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
	
	if {$uncaptured != 0} {
	    if {$i == [expr $speed / 2]} {
		$c raise $uncaptured
		$c raise $piece
	    }
	}
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
proc GS_ShowMoves { c moveType position moveList } {

    foreach move $moveList {
	drawmove $c $move $moveType $position
    }

}

################################
# Returns a list with the move #
################################
proc unhashmove { move } {
    return [list [expr $move & 0x3F] [expr $move >> 6]]
}

proc diagonal { origin destination } {
    if { [lindex $origin 0] != [lindex $destination 0] && [lindex $origin 1] != [lindex $destination 1] } {
	return 1
    }
    return 0
}

######################
# Draws a move arrow #
######################
proc drawmove { c move moveType position } {
    # 1. Unhash the move into indices
    # 2. Convert indices into coordinates
    # 3. Draw arrow from origin to destination coordinates

    global t arrowhead arrowwidth arrowlength arrowcolor r
    
    global tk_library kRootDir gStippleRootDir
    set gStippleRootDir "$kRootDir/../bitmaps/"
    set stipple12 @[file join $gStippleRootDir gray12.bmp] 
	set stipple25 @[file join $gStippleRootDir gray25.bmp]
	set stipple50 @[file join $gStippleRootDir gray50.bmp]
	set stipple75 @[file join $gStippleRootDir gray75.bmp]
       
     set color $arrowcolor
     set stipple ""

     
    set m [unhashmove [lindex $move 0]]
    set origin [coords [lindex $m 0]]
    set destination [coords [lindex $m 1]]
    set board [unhash $position]
    set delta [lindex $move 3]
    set delta [expr $delta/2]
    
    if {$moveType == "value" || $moveType == "rm"} {

	    # If the move leads to a win, it is a losing move (RED)
	    # If the move leads to a losing position, it is a winning move (GREEN)
	    # If the move leads to a tieing position, it is a tieing move (YELLOW)
	    switch [lindex $move 1] {
		Win { set color darkred }
		Lose { set color green }
		Tie { set color yellow }
		default { set color $arrowcolor }
	    }
        
        if {$moveType == "rm"} {
        
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
	}
	
    
    
    # This expression draws the arrows from origin to destination. However, since
    # we want some space between the end of the arrow and the destination (otherwise
    # we get some ugly overlap), we set our destination to be the actual destination
    # minus some factor multiplied by the distance from origin to destination.

    set arrow [$c create line \
		   [expr [lindex $origin 0]] \
		   [expr [lindex $origin 1]] \
		   [expr [lindex $destination 0] - (1 - $arrowlength)*([lindex $destination 0] - [lindex $origin 0])] \
		   [expr [lindex $destination 1] - (1 - $arrowlength)*([lindex $destination 1] - [lindex $origin 1])] \
		   -width $arrowwidth -fill $color -arrow last -arrowshape $arrowhead -stipple $stipple]

    # Now, in order to make sure the arrows show up under the pieces they are on, we have to draw the pieces again.
    # However, we cannot draw all of them, only the ones that have moves on them. This is to ensure that the arrows
    # go over pieces that are to be captured (which must be a subset of already drawn pieces).

    set piece [string index $board [lindex $m 0]]

    draw $piece [lindex $m 0] $c
	       
    $c bind $arrow <Enter> "$c itemconfigure $arrow -fill black"
    $c bind $arrow <Leave> "$c itemconfigure $arrow -fill $color"
    $c bind $arrow <ButtonRelease-1> "ReturnFromHumanMove [lindex $move 0]"
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

    global r outline
    
    set origposition [expr $theMoveToUndo >> 6]
    set origin [coords $origposition]
    set destination [coords [expr $theMoveToUndo & 0x3F]]

    # Draw currentPosition with piece at origin gone
    DrawBoard $c
    set oldBoard [unhash $currentPosition]
    set movedpiece [string index $oldBoard $origposition]

    switch $movedpiece {
	F { set color red  }
	G { set color blue }
    }

    set oldBoard [string replace $oldBoard $origposition $origposition " "]
    set uncaptured 0

    set newBoard [unhash $positionAfterUndo]

    # Capture condition: newBoard has a goose where oldBoard does not, AND a fox has moved
    # (i.e. there is a spot where newBoard has a fox and oldBoard has a blank)
    set foxmoved 0
    for {set i 0} {$i < 21} {incr i} {
	if { [string index $oldBoard $i] == " " && [string index $newBoard $i] == "F"} {
	    set foxmoved 1
	}
    }

    for {set i 0} {$i < 21} {incr i} {
	if { [string index $oldBoard $i] == " " && [string index $newBoard $i] == "G" && $foxmoved} {
	    set oldBoard [string replace $oldBoard $i $i " "]
	    set uncaptured [coords $i]
	}
    }

    if {$uncaptured != 0} {
	set uncaptured [$c create oval \
			[expr [lindex $uncaptured 0] - $r] \
			[expr [lindex $uncaptured 1] - $r] \
			[expr [lindex $uncaptured 0] + $r] \
			[expr [lindex $uncaptured 1] + $r] \
			-fill blue -width $outline]
	$c lower $uncaptured
    }

    DrawPieces $c $oldBoard

    set piece [$c create oval [expr [lindex $origin 0] - $r] [expr [lindex $origin 1] - $r] [expr [lindex $origin 0] + $r] [expr [lindex $origin 1] + $r] -fill $color -width $outline]

    undo $c $piece $origin $destination $uncaptured

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
    
    global size fontsize
    
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
