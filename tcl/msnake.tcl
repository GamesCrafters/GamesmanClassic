####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################
#####################################################
#
# NAME:        msnake.tcl
#
# DESCRIPTION: Snake
#
# AUTHORS:     Judy Chen, Eleen Chiang, Peter Foo, Rach Liu
#              University of California at Berkeley
#              Copyright (C) 2003. All rights reserved.
# 
# DATE:        05/07/03
#              03/24/04 Added bindings, SendMove, Options code, changed GS_Initialize
######################################################

# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

proc GS_InitGameSpecific {} {
    #puts ">> GS_InitGameSpecific"
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Snake"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Alice Chang, Judy Chen, Eleen Chiang, Peter Foo"
    set kTclAuthors "Judy Chen, Eleen Chiang, Peter Foo, Rach Liu"
    #set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 8357
    set gPosition $gInitialPosition


    global kToMove kToWin

    set kToMove "Players alternate turns moving one space up, right or left into an adjacent open square. Once the player has moved to an open slot, it bcomes occupied and neither player is allowed to move into that space. The game ends when there is no room for one player to move."

    set kToWin "Force your opponent into a spot where on the next turn he or she will hit the wall or the snake's body."

    # 4x4 board
    global BOARDSIZE
    set BOARDSIZE 16

    global BOARDWIDTH
    set BOARDWIDTH 4

    # HEAD and TAIL tracker variables -- they hold the tile that the head or tail is currently in
    # default is whatever is default in the C code version
    global HEADinTILE
    global TAILinTILE
    set HEADinTILE 5
    set TAILinTILE 10

    # game-specific options flags, the defaults will match the c code defaults
    
    # headonly==1 ->  player can only move head, computer can only move tail
    # headonly==0 ->  either player can move either the head or the tail

    # not implemented yet, but to implement in the future, just put in an if statement in GS_ShowMoves that filters out tail moves if its the players turn, or filters out head moves when its the computer's turn. Use above head and tail tracker variables (HEADinTILE and TAILinTILE to determine which moves to filter, easiest if first convert moves to external rep format
    global HEADONLY
    set HEADONLY 1

    #puts "<< exit GS_InitGameSpecific"

    # Set toMove and toWin
    global gMisereGame
    global gMoveTail
    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    set toWin2  "Force your opponent into a spot where on the next turn he or she will hit the wall or the snake's body."

    SetToWinString [concat $toWin1 $toWin2]

    set toMove1 "To Move: Players alternate turns moving one space up, right or left into an adjacent open square. Once the player has moved to an open slot, it bcomes occupied and neither player is allowed to move into that space. Select an arrow to move"
    if { $gMoveTail } {
	set toMove2 "the head or tail "
    } else {
	set toMove2 "the head (Player 1) or tail (Player 2) "
    }
    set toMove3 "to an empty adjacent space."

    SetToMoveString [concat $toMove1 $toMove2 $toMove3]
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

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

     set tailRule \
	[list \
	     "Movable parts are:" \
	     "Head for P1, Tail for P2" \
	     "Head and Tail" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $tailRule]
    
    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0
    global gMoveTail
    set gMoveTail 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gMoveTail"]

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
    global gMisereGame gMoveTail
    set option 1
    set option [expr $option + $gMisereGame]
    set option [expr $option + 2*$gMoveTail]

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
    global gMisereGame gToTrapIsToLose gMoveTail
    set option [expr $option - 1]
    set gMisereGame [expr $option%2]
    set gMoveTail [expr $option/2%2]
}


# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {
    #puts ">> GS_NameOfPieces"
    #puts "<< exit GS_NameOfPieces"
    return [list head tail]    
}


# GS_Initialize draws the graphics for the game on the canvas c
# You could put an opening animation or just draw an empty board.
# This function is called after GS_InitGameSpecific

proc GS_Initialize { c } {

    #puts ">> GS_Initialize"
    
    # you may want to start by setting the size of the canvas; this line isn't cecessary
    $c configure -width 500 -height 500
    set counter 0
    for {set j 0} {$j < 4} {set j [expr $j + 1]} {
	for {set i 0} {$i < 4} {set i [expr $i + 1]} {

	    $c create rect [expr $i * 125] [expr $j * 125] [expr ($i+1) * 125] [expr ($j+1) * 125] -fill white -outline black -width 2 -tag [list base base$counter]
	    incr counter
	}
    }

    # counter is the NUMBER OF TILES, base$i refers to each tile i, 0<i<counter
    #puts "COUNTER:$counter"

# dont need to draw these lines since using the black outlines of the tiles
#     $c create line 125 0 125 500 -width 2 -tag base
#     $c create line 250 0 250 500 -width 2 -tag base
#     $c create line 375 0 375 500 -width 2 -tag base
    
#     $c create line 0 125 500 125 -width 2 -tag base
#     $c create line 0 250 500 250 -width 2 -tag base
#     $c create line 0 375 500 375 -width 2 -tag base
#     $c create line 0 125 500 125 -width 2 -tag base

    MakePieces $c 0
    MakeConnectors $c
    MakeArrows $c

    # bind the tiles
    for {set i 0} {$i < $counter} {incr i} {
	$c bind base$i <Enter> "TileEnter $c $i"
	$c bind base$i <Leave> "TileLeave $c $i"
	$c bind base$i <ButtonRelease-1> "TileClick $c $i"
#	$c bind base$i <ButtonRelease-1> "SendMove $i"
    }
   
    $c raise base
    #update idletasks
    
    #puts "<< exit GS_Initialize"
} 

proc GS_Deinitialize { c } {   
    $c delete all
}

proc SendMove { square } {
    set theMove $square

    ReturnFromHumanMove $theMove
    #puts $theMove
}

proc makeMouse { c tileNum } {

    set x [expr 125 * [expr $tileNum % 4]]
    set y [expr 125 * [expr $tileNum / 4]]
    $c create oval [expr $x+45] [expr $y+45] [expr $x+85] [expr $y+85] -fill gray55 -tag [list mousey mousehead]
    $c create oval [expr $x+55] [expr $y+55] [expr $x+60] [expr $y+65] -fill white -tag [list mousey mousehead eye eyeL]
    $c create oval [expr $x+70] [expr $y+55] [expr $x+75] [expr $y+65] -fill white -tag [list mousey mousehead eye eyeR]
    $c create oval [expr $x+45] [expr $y+35] [expr $x+55] [expr $y+53] -fill gray65 -tag [list mousey mousehead ear earL]
    $c create oval [expr $x+75] [expr $y+35] [expr $x+85] [expr $y+53] -fill gray65 -tag [list mousey mousehead ear earR]
    $c create oval [expr $x+48] [expr $y+38] [expr $x+52] [expr $y+47] -fill pink3 -tag [list mousey mousehead earInner earInnerL]
    $c create oval [expr $x+78] [expr $y+38] [expr $x+82] [expr $y+47] -fill pink3 -tag [list mousey mousehead earInner earInnerR]
    $c create line [expr $x+58] [expr $y+75] [expr $x+72] [expr $y+75] -fill black -width 2 -tag [list mousey mousehead mousemouth]
    
    # put the Mouse lower than the head and tail so that the tail and head cover it when they move onto the tile
    $c lower mousey head
    $c lower mousey tail
    $c raise mousey base
}

proc TileClick { c tileNum } {

    makeMouse $c $tileNum

}

proc TileEnter { c tileNum } {
    global BOARDWIDTH
    global HEADinTILE
    global TAILinTILE
    
    $c raise base$tileNum base
    $c itemconfig base$tileNum -outline cyan -width 4


#---------- THIS SECTION HANDLES PUPILS (FUNCTIONIZE THIS SECTION?) ----------
    # NOTE: in future, implement option flag so can skip this if not selected
    # move pupils appropriately
    # mod head tile by BOARDWIDTH, mod current tile (tileNum) by BOARDWIDTH, then compare
    set headTile [expr $HEADinTILE % $BOARDWIDTH]
    set curTile [expr $tileNum % $BOARDWIDTH]
    # handle left-right pupil displacement
    if {$curTile > $headTile} {
	# case where curTile is RIGHT of the headTile
	movePupils $c right
    } elseif {$curTile < $headTile} {
	# case where curTile IS left of the headTile
	movePupils $c left
    } else {
	# case where curTile IS the headTile
	movePupils $c resetHoriz
    }
    # handle up-down pupil displacement
    # get the leftest tile number of the row the HEAD is on:
    set leftestTile [expr $HEADinTILE - ($HEADinTILE % $BOARDWIDTH)]
    if {$tileNum < $leftestTile} {
	# tileNum is vertically above head
	movePupils $c up
    } elseif { $tileNum >= [expr $leftestTile + $BOARDWIDTH] } {
	# tileNum is vertically below head
	movePupils $c down
    } else {
	# tileNum (current tile the mouse is in) is in the same row as head
	movePupils $c resetVert
    }
}

proc movePupils { c command } {
    #puts "---- $command"

    # get coords of eyes
    set confine_coords [$c coords confinement]
    set x_confinecenter [lindex $confine_coords 0]
    set y_confinecenter [lindex $confine_coords 1]

    # get current coords of pupils
    set pupils_center_coords [$c coords pupils]
    set xcenter [lindex $pupils_center_coords 0]
    set ycenter [lindex $pupils_center_coords 1]

    # amount to move by each time, radius of horizontal movement
    set X 1 
    # same as above, for vertical
    set Y 2

    # SCRATCH THIS COMMENTreset commands will raise back the original pupils, and delete the copies, all others hides the originals and makes copies
    if {$command == "right" && ($xcenter <= [expr $x_confinecenter + $X]) } {
	$c move pupils $X 0
	
    }
    if {$command == "left" && ($xcenter >= [expr $x_confinecenter - $X]) } {
	$c move pupils [expr -1 * $X] 0
    }
    if {$command == "up" && ($ycenter >= [expr $y_confinecenter - $Y]) } {
	$c move pupils 0 [expr -1 * $Y]
    }
    if { $command == "down" && ($ycenter <= [expr $y_confinecenter + $Y]) } {
	$c move pupils 0 $Y
    }
    if { $command == "resetHoriz" } {

    }
    if { $command == "resetVert" } {
	
    }     
    
}

proc TileLeave {c tileNum } {
    $c itemconfig base$tileNum -outline black -width 2
}

# April
# Make all the arrows (four arrows for every tile, excepting the corners and borders)
proc MakeArrows { c } {
    # ie 4x4, BOARDSIZE is 16
    global BOARDSIZE
    global BOARDWIDTH

    # remember, the make arrow functions written by us take in external move reps (from, to slots)
    for {set i 0} {$i < $BOARDSIZE} {set i [expr $i + 1]} {
	# in a tile
	
	# the tile is, in 0,1,2,...,boardsize-1 indexing
	set theTile $i

	# theTile is the "from", now generate the "to"'s
	set upTo [expr $theTile - $BOARDWIDTH]
	set downTo [expr $theTile + $BOARDWIDTH]
	set leftTo [expr $theTile - 1]
	set rightTo [expr $theTile + 1]

	if {$upTo >= 0} {
	    #puts "in upTo section"
	    drawUpArrow $c $theTile $upTo cyan

	    # prepare internal move representation for binding
	    set internalMove [SlotsToMove $theTile $upTo]

	    # now bind the arrow; this depends on the draw<Dir>Arrow procs to tag the arrows appropriately! the draw<dir>arrow procs tag with 0,1,2,...,boardsize-1 indexing
	    $c bind arrow$theTile$upTo <Enter> "ArrowEnter $c arrow$theTile$upTo"
	    $c bind arrow$theTile$upTo <Leave> "ArrowLeave $c arrow$theTile$upTo"
	    $c bind arrow$theTile$upTo <ButtonRelease-1> "$c itemconfig arrow$theTile$upTo -fill black; SendMove $internalMove"
	}
	# here assuming that the board length is same as board width (ie board is square). this is what  msnake.c  does, so i'm just following the msnake.c convention
	if {$downTo < $BOARDSIZE} {
	    #puts "in downTo section"
	    drawDownArrow $c $theTile $downTo cyan
	    set internalMove [SlotsToMove $theTile $downTo]
	    $c bind arrow$theTile$downTo <Enter> "ArrowEnter $c arrow$theTile$downTo"
	    $c bind arrow$theTile$downTo <Leave> "ArrowLeave $c arrow$theTile$downTo"
	    $c bind arrow$theTile$downTo <ButtonRelease-1> "$c itemconfig arrow$theTile$downTo -fill black; SendMove $internalMove"
	}
	if {[expr $theTile % $BOARDWIDTH] != 0 } {
	    #puts "in leftTo section"
	    #puts "TILE: $theTile"
	    #puts [expr $i % $BOARDWIDTH]
	    # drawLeftArrow appears to take internal move rep? dunno, didnt write that proc
	    drawLeftArrow $c $theTile $leftTo cyan
	    set internalMove [SlotsToMove $theTile $leftTo]
	    $c bind arrow$theTile$leftTo <Enter> "ArrowEnter $c arrow$theTile$leftTo"
	    $c bind arrow$theTile$leftTo <Leave> "ArrowLeave $c arrow$theTile$leftTo"    
#	    $c bind arrow$theTile$leftTo <ButtonRelease-1> "puts arrow$theTile$leftTo; $c itemconfig arrow$theTile$leftTo -fill black; SendMove $internalMove"
	    $c bind arrow$theTile$leftTo <ButtonRelease-1> "$c itemconfig arrow$theTile$leftTo -fill black; SendMove $internalMove"
	}
	if {[expr ($theTile+1) % $BOARDWIDTH] != 0 } {
	    #puts "in rightTo section"
	    drawRightArrow $c $theTile $rightTo cyan
	    set internalMove [SlotsToMove $theTile $rightTo]
	    $c bind arrow$theTile$rightTo <Enter> "ArrowEnter $c arrow$theTile$rightTo"
	    $c bind arrow$theTile$rightTo <Leave> "ArrowLeave $c arrow$theTile$rightTo"
	    $c bind arrow$theTile$rightTo <ButtonRelease-1> "$c itemconfig arrow$theTile$rightTo -fill black; SendMove $internalMove"
	}
    }

    $c lower arrows all
}


proc ArrowEnter { c theTag } {
    global prevColour
    
    set prevColour [$c itemcget $theTag -fill]

    $c itemconfig $theTag -fill black
}

proc ArrowLeave { c theTag } {
    global prevColour

    $c itemconfig $theTag -fill $prevColour
}
    

proc SlotsToMove { fromSlot toSlot } {
    global BOARDSIZE

    return [expr $toSlot*($BOARDSIZE+1) + $fromSlot]
}    

# Makes the body pieces of the snake, but not the head or tail.
proc MakePieces { c num } {
    MakeBody $c [expr $num % 4] [expr $num / 4] $num
    if { $num != 15 } {
	MakePieces $c [expr $num + 1]
    }
}

# Makes the body pieces of the snake.  Maybe combine MakeBody and MakePieces?
proc MakeBody { c x y tag } {
    set x [expr $x * 125]
    set y [expr $y * 125]
    $c create rect $x $y [expr $x + 124] [expr $y + 124] -fill green3 -tag [list body body$tag]
    $c lower body$tag base 
}

# Makes the connecting pieces of the body. 
proc MakeConnectors { c } {
    for {set i 0} {$i < 16} {set i [expr $i + 1]} {
	MakeHorizontalConnectors $c $i [expr $i+1]	
	#puts "in makehoriz"
    }
    for {set i 0} {$i < 4} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    for {set i 4} {$i < 8} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    for {set i 8} {$i < 12} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    for {set i 12} {$i < 16} {set i [expr $i + 1]} {
	MakeVerticalConnectors $c $i [expr $i+4]
    }
    $c lower vconnectors base
    $c lower hconnectors base
}

# Makes the horizontal connecting body pieces
proc MakeHorizontalConnectors { c from to } {
    if {$from == [expr $to - 1]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 95] [expr $y + 30] [expr $x + 155] [expr $y + 95] -fill green3 -tag [list connectors hconnectors hcon$from$to]
    }
    if {$from == [expr $to + 1]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x - 25] [expr $y + 30] [expr $x + 35] [expr $y + 95] -fill green3 -tag [list connectors hconnectors hcon$from$to]
	#puts $from
	#puts $to
    }
    #puts "---- hcon$from$to"
}

# Makes vertical connecting pieces
proc MakeVerticalConnectors { c from to} {
    if {$from == [expr $to + 4]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 30] [expr $y - 25 ] [expr $x + 95] [expr $y + 35] -fill green3 -tag [list connectors vconnectors vcon$from$to]
    }
    if {$from == [expr $to - 4]} {
	set x [expr [expr $from % 4] * 125]
	set y [expr [expr $from / 4] * 125]
	$c create rect [expr $x + 30] [expr $y + 95] [expr $x + 95] [expr $y + 155] -fill green3 -tag [list connectors vconnectors vcon$from$to]
	#puts "---- vcon$from$to"
	#puts $from
	#puts $to
    }
}





## Draws a head in square SLOT on the board.
proc MakeHead { c slot } {
    set x [expr 125 * [expr $slot % 4]]
    set y [expr 125 * [expr $slot / 4]]
    $c create oval [expr $x+15] [expr $y+15] [expr $x+110] [expr $y+110] -fill green -tag head
    $c create oval [expr $x+38] [expr $y+35] [expr $x+52] [expr $y+65] -fill red -tag [list head eyes]
    $c create oval [expr $x+68] [expr $y+35] [expr $x+82] [expr $y+65] -fill red -tag [list head eyes]
    $c create oval [expr $x+40] [expr $y+45] [expr $x+50] [expr $y+55] -fill black -tag [list head eyes pupils]
    $c create oval [expr $x+70] [expr $y+45] [expr $x+80] [expr $y+55] -fill black -tag [list head eyes pupils]

    # create the confinement circles for moving the eyes, make it the same colour as eyes so as to hide it (unless there is a "hide" option, this is OK)
    set X 1
    set Y 2
    $c create oval [expr $x+40-$X] [expr $y+45-$Y] [expr $x+50+$X] [expr $y+55+$Y+1] -fill red -width 0 -tag [list head eyes confinement confineL]
    $c create oval [expr $x+70-$X] [expr $y+45-$Y] [expr $x+80+$X] [expr $y+55+$Y+1] -fill red -width 0 -tag [list head eyes confinement confineR]

    $c create line [expr $x+40] [expr $y+85] [expr $x+80] [expr $y+85] -fill black -width 2 -tag [list head mouth] 
    #$c raise pupils eyes

    #MakeTongue $c

    $c lower confinement all

    $c bind head <Enter> "EnterHead $c $slot"
}

proc MakeTongue { c tongueLen} {

    #create the tongue
    #set tongueLen 30
    set tongueWidth 3
    set tongueColour red3

    set mouthCoords [$c coords mouth]
    # the mouth coords are the very left coords of the mouth, ie  => ______
    set x1 [lindex $mouthCoords 0]
    set y1 [lindex $mouthCoords 1]
    set x2 [lindex $mouthCoords 2]
    set y2 [lindex $mouthCoords 3]
    set xCenter [expr $x1+($x2-$x1)/2]
    $c create line $xCenter $y1 $xCenter [expr $y1 + $tongueLen] -fill $tongueColour -width $tongueWidth -tag [list head tongue]

    # create the forks on the tongue
    # horiz displacement of fork from tongue center
    set forkX 4
    set forkY 3
    set forkWidth 2
    set yEnd [expr $y1 + $tongueLen]
    $c create line [expr $xCenter-$forkX] [expr $yEnd+$forkY] $xCenter $yEnd -fill $tongueColour -width $forkWidth -tag [list head tongue fork forkL]
    $c create line [expr $xCenter+$forkX] [expr $yEnd+$forkY] $xCenter $yEnd -fill $tongueColour -width $forkWidth -tag [list head tongue fork forkR]

}

proc EnterHead { c slot } {

    #puts " ---- [$c coords mouth]"

    set MaxTongueLen 30
    set MinTongueLen 6

    for {set i 0} {$i < 3} {set i [expr $i+1]} {

	set temp [clock seconds]
	set temp [expr $temp % $MaxTongueLen + $MinTongueLen]

	TongueShoot $c $temp
    }
}

proc TongueShoot { c maxlengthExtend} {
    
    #extend the tongue
    for {set i 0} {$i < $maxlengthExtend} {set i [expr $i+1]} {
	MakeTongue $c $i
	update idletasks
	$c delete tongue
    }
    #retract the tongue
    for {} {$i >= 0} {set i [expr $i-1]} {
	MakeTongue $c $i
	update idletasks
	$c delete tongue
    }
}

# Makes the tail in SLOT
proc MakeTail {c slot } {
    set x [expr 125 * [expr $slot % 4]]
    set y [expr 125 * [expr $slot / 4]]
    $c create oval [expr $x+15] [expr $y+15] [expr $x+110] [expr $y+110] -fill blanchedalmond -tag tail
    $c create oval [expr $x+30] [expr $y+30] [expr $x+95] [expr $y+95] -fill burlywood -tag [list tail ring1]
    $c create oval [expr $x+45] [expr $y+45] [expr $x+80] [expr $y+80] -fill brown4 -tag [list tail ring1 ring2]
    
    # create a ring for moving around to simulate rattle
    $c create oval [expr $x+45] [expr $y+45] [expr $x+80] [expr $y+80] -fill brown2 -tag [list tail ringAnim]

    # create a confinement oval for rattle ring
    set X 5
    set Y 5
    $c create oval [expr $x+45-$X] [expr $y+45-$Y] [expr $x+80+$X] [expr $y+80+$Y] -fill brown4 -width 0 -tag [list tail ring1 ring2]

    #hide the extra ring and confinement ring
    $c lower ringAnim all
    $c lower confineRing all

    $c bind tail <Enter> "EnterTail $c $slot"
}

proc EnterTail { c slot } {
    
    # rattle the tail x times, where x is random
    set MaxNum 35

    # store current position of tail
    set tail_coords [$c coords tail]

    set temp [clock seconds]
    set temp [expr $temp % $MaxNum]
    rattleTail $c $temp

    $c coords tail $tail_coords
}

proc spinwait {k} {
    for {set j 0} {$j < $k} {set j [expr $j+1]} {
    }
}

proc rattleTail { c numRattles} {
    # get coords of confine ring
    set confine_coords [$c coords confineRing]
    set x_confinecenter [lindex $confine_coords 0]
    set y_confinecenter [lindex $confine_coords 1]

    # constants taken from MakeTail
    set X 20
    set Y 20

    # get current coords of ringAnim
    set ringAnim_center_coords [$c coords ringAnim]
    set xcenter [lindex $ringAnim_center_coords 0]
    set ycenter [lindex $ringAnim_center_coords 1]

    $c raise ringAnim

    set Yspeed 3
    set Xspeed 3

    set curWidth [$c itemcget ringAnim -width]

    for {set i 0} {$i < $numRattles} {set i [expr $i + 1]} {

 	set temp [clock seconds]
 	set temp [expr $temp % 3]

 	$c itemconfig ringAnim -width [expr $curWidth +$i*$temp]
	
# 	$c move tail $temp $temp
# 	spinwait 1000
# 	$c move tail 0 [expr -1*$temp]
# 	spinwait 1000
# 	$c move tail 0 [expr -1*$temp]
# 	spinwait 1000
# 	$c move tail [expr -1*$temp] 0
# 	$c move tail [expr -1*$temp] 0
# 	spinwait 1000
# 	$c move tail 0 $temp
# 	$c move tail 0 $temp
# 	spinwait 1000
# 	$c move tail $temp 0
# 	spinwait 1000
# 	$c move tail 0 [expr -1*$temp]

# 	# random rattling around in confinement ring not working
#  	set temp [clock seconds]
#  	set temp [expr $temp % 4]
#  	if {$temp == 0 && ($ycenter >= [expr $y_confinecenter - $Y]) } {
#  	    #move Up
#  	    $c move ringAnim 0 [expr -1 * $Yspeed]
#  	} elseif {$temp == 1 && ($ycenter <= [expr $y_confinecenter + $Y]) } {
#  	    # move Down
#  	    $c move ringAnim 0 $Yspeed
#  	} elseif {$temp == 2 && ($xcenter <= [expr $x_confinecenter + $X]) } {
#  	    # move Right
#  	    $c move ringAnim $Xspeed 0
#  	} elseif {$temp == 3 && ($xcenter >= [expr $x_confinecenter - $X]) } {
#  	    # move Left
#  	    $c move ringAnim [expr -1*$Xspeed] 0
#  	}

	update idletasks
    }

    # reset back to orig size, hide the ringAnim
    $c itemconfig ringAnim -width $curWidth
    $c lower ringAnim
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

proc GS_DrawPosition { c position } {
    #puts ">> GS_DrawPosition"
    # BTW too: don't make any assumptions about the state of the board.
    # Clears the board
    $c raise base 
    # Get Board
    set board [unhash $position]
    
    # Place head and tail on board
    set whereHead [lsearch -exact $board "head"] 
    set whereTail [lsearch -exact $board "tail"]
    MakeHead $c $whereHead
    MakeTail $c $whereTail

    # Raise body pieces
    for {set i 0} {$i<16} {incr i} {
	if {[lindex $board $i] == "body"} { 
	    $c raise body$i
	}
    }

   # Make Connections
   #set x 0
    #set y 1
    #foreach {a b} $board {
	#if {{$a == "head"} || {$a == "tail"} || {$a == "body"}  #&& 
	#{$b == "head"} || {$b == "tail"} || {$b == "body"}} {
	#    $c raise body$x-$y
	#}
	#incr x 
	#incr y 
    #} 

    #puts "<< exit GS_DrawPosition"
}

proc unhash { position } {
    global BOARDSIZE

    set findHead [expr $position & 15]
    set findTail [expr $position >> 4]
    set findTail [expr $findTail & 15]
    set findBody [expr $position >> 8]

# originals
#    set findTail [expr $position & 240]
#    set findTail [expr $findTail >> 4]

    # Building the board sequentially

# april - i started changing here

    for {set i 0} {$i<$BOARDSIZE} {incr i} {
	lappend board "blank"
    }

    lset board $findHead "head"
    lset board $findTail "tail"

    for {set i 0} {$i<$BOARDSIZE} {incr i} {
	if {[lindex $board $i]=="blank"} {
	    if {[expr $findBody & 1]} {
		lset board $i "body"
		set findBody [expr $findBody >> 1]
	    } else {
		lset board $i "blank"
		set findBody [expr $findBody >> 1]
	    }
	}
    }

#     #original
#     for {set i 0} {$i<$BOARDSIZE} {incr i} {
# 	if {$i == $findHead} {
# 	    lappend board "head"
# 	} elseif {$i == $findTail} {
# 	    lappend board "tail"
# 	} elseif {[expr $findBody & 1]} {
# 	    lappend board "body" 
# 	    set findBody [expr $findBody >> 1]
# 	} else {
# 	    lappend board "blank"
# 	    set findBody [expr $findBody >> 1]
# 	}
#     }

    return $board
}

# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # NOTE: REPLACE ALL CONSTANTS (5, 10) WITH APPROPRIATE GLOBAL VARS, ie 5, 10 replace with constants that store the beginning positions of head and tail
    #puts ">> GS_NewGame"

    global HEADinTILE
    global TAILinTILE

    # delete previous head and tail and just make a new one, since don't want to move the old ones
    $c delete head
    $c delete tail

    MakeHead $c 5
    MakeTail $c 10


    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position

    # Assume initial position is 8357 (for now). Raise the already drawn body connectors.
    $c raise hcon56 all
    $c raise vcon610 all


    # reset the HEAD and TAIL tracker vars; NOTE: REPLACE CONSTANTS with global variables that store the starting position for head and tail
    set HEADinTILE 5
    set TAILinTILE 10

    $c raise head 
    $c raise tail   
    
    #puts "<< exit GS_NewGame"
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    global BOARDSIZE
    #puts ">> GS_WhoseMove"
    set board [unhash $position]
    set count 0  
    for {set i 0} {$i<$BOARDSIZE} {incr i} {
        if {[lindex $board $i] == "body"} {
            set count [expr $count + 1]
        }
    }

    if {[expr $count % 2] == 0} {
        set who tail
    } else {
        set who head
    }

    #puts "<< exit GS_WhoseMove"

    return $who
}

# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global BOARDSIZE
    global HEADinTILE
    global TAILinTILE
    #puts ">> GS_HandleMove"

    set oldl [unhash $oldPosition]
    set newl [unhash $newPosition]

    #theMove is in internal rep format, gotta convert to external (from, slot) format, from slot format is in 0,1,2,...,boardsize-1 indexing form

    set from [expr $theMove % ($BOARDSIZE+1)]
    set to [expr $theMove / ($BOARDSIZE+1)]
    set piece head


    #puts " ---- theMove: $theMove"
    #puts " ---- from: $from"
    #puts " ---- to: $to"

    #puts " ---- [GS_WhoseMove $oldPosition]"
    

   # determine if the head was moved, or if the tail was moved
    if {$from == $HEADinTILE} {
	set piece head
    }
    if {$from == $TAILinTILE} {
	set piece tail
    }


    # major bug-- assumes that player and comp alternate between heads and tails, and that player only picks heads, comp only picks tails; when the HEADONLY is false (ie player and comp can choose heads OR tails for ANY move), must determine if the head or the tail is picked in some OTHER way, rather than using [GS_WhoseMove $oldPosition], since [GS_WhoseMove $oldPosition] becomes an indicator of whether the player (inappropriately in this case called "head"), or the comp (inappropriately in this case called "tail") moved last
#     if { [GS_WhoseMove $oldPosition] == "tail"} {
# 	set piece tail
#     }

    if {$to == [expr $from + 1]} {
	set direction  right
    }
    if {$to == [expr $from + 4]} {
	set direction  down
    }
    if {$to == [expr $from - 1]} {
	set direction  left
    }
    if {$to == [expr $from - 4]} {
	set direction  up
    }

    # update head or tail tracker variables accordingly
    if {$piece == "head"} {
	if {$direction == "right"} {
	    set HEADinTILE [expr $HEADinTILE + 1]
	} elseif {$direction == "left"} {
	    set HEADinTILE [expr $HEADinTILE - 1]
	} elseif {$direction == "down"} {
	    set HEADinTILE [expr $HEADinTILE + 4]
	} elseif {$direction == "up"} {
	    set HEADinTILE [expr $HEADinTILE - 4]
	}
    } elseif {$piece == "tail"} {
	if {$direction == "right"} {
	    set TAILinTILE [expr $TAILinTILE + 1]
	} elseif {$direction == "left"} {
	    set TAILinTILE [expr $TAILinTILE - 1]
	} elseif {$direction == "down"} {
	    set TAILinTILE [expr $TAILinTILE + 4]
	} elseif {$direction == "up"} {
	    set TAILinTILE [expr $TAILinTILE - 4]
	}
    }


    #temp, cause my temp random "positions" seem to cause none of above direction if's to be hit
    #set direction right
    #puts "---- direction: $direction"
    #puts "---- piece: $piece"
    $c raise body$from 
    MovePiece $c $piece $direction
    if {$direction == "up"} {
	$c raise vcon$from$to all
    } elseif {$direction == "down"} {
	$c raise vcon$from$to all
#	MakeVerticalConnectors $c $from $to
    } elseif {$direction == "left"} {
	$c raise hcon$from$to all
	#MakeHorizontalConnectors $c $from $to
    } elseif {$direction == "right"} {
	$c raise hcon$from$to all
	#MakeHorizontalConnectors $c $from $to
    }
    $c raise head

    #puts "<< exit GS_HandleMove"
}

# Moves the PIECE, which is either head or tail
proc MovePiece {c piece direction} {
    if {$direction == "left"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece -5 0
	    update idletasks
	}
    } elseif {$direction == "right"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece 5 0
	    update idletasks
	}
    } elseif {$direction == "up"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece 0 -5
	    update idletasks
	}
    } elseif {$direction == "down"} {
	for {set frame 0} {$frame < 25} {incr frame} {
	    $c move $piece 0 5
	    update idletasks
	}
    }
}


# finds the location of the piece
proc findPiece { piece position } {
    set board [unhash $position]
    for {set i 0} {$i < 16} {incr i} {
	if {[lindex $board $i] == $piece} {
	    return $i
	}
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

# all draw<dir>Arrow procs take 0,1,2,..,boardsize-1 indexing
proc drawUpArrow { c from to color} {
    set x [expr $from % 4]
    set y [expr $to / 4]
    $c create poly [expr $x*125 + 25] [expr $y*125 + 110] [expr $x*125 + 62] [expr $y*125 + 90] [expr $x*125 + 100] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 110] -fill $color -tags [list arrows upArrow upArrow$from$to arrow$from$to]
}

proc drawDownArrow { c from to color } {
    set x [expr $from % 4]
    set y [expr $to / 4 - 1]
    $c create poly [expr $x*125 + 50] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 110] [expr $x*125 + 75] [expr $y*125 + 140] [expr $x*125 + 100] [expr $y*125 + 140] [expr $x*125 + 62] [expr $y*125 + 160] [expr $x*125 + 25] [expr $y*125 + 140] [expr $x*125 + 50] [expr $y*125 + 140] -fill $color -tags [list arrows downArrow downArrow$from$to arrow$from$to]
}

proc drawRightArrow {c from to color } {
    set x [expr $from % 4]
    set y [expr $to / 4 ]
    $c create poly  [expr $x*125 + 110] [expr $y*125 + 50] [expr $x*125 + 110] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 100] [expr $x*125 + 160] [expr $y*125 + 62] [expr $x*125 + 140] [expr $y*125 + 25] [expr $x*125 + 140] [expr $y*125 + 50] -fill $color -tags [list arrows rightArrow rightArrow$from$to arrow$from$to]
}    

proc drawLeftArrow { c from to color} {
    set x [expr $from % 4 - 1]
    set y [expr $to / 4]
    $c create poly [expr $x*125 + 110] [expr $y*125 + 25] [expr $x*125 + 90] [expr $y*125 + 62] [expr $x*125 + 110] [expr $y*125 + 100] [expr $x*125 + 110] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 75] [expr $x*125 + 140] [expr $y*125 + 50] [expr $x*125 + 110] [expr $y*125 + 50] -fill $color -tags [list arrows leftArrow leftArrow$from$to arrow$from$to]
}

proc GS_ShowMoves { c moveType position moveList } {
    global BOARDSIZE

    #puts ">> GS_ShowMoves"

    set whoseTurn [GS_WhoseMove $position]
    set from [findPiece $whoseTurn $position]  
    #puts $from

    #puts "---- moveList: $moveList"
    foreach item $moveList {
	#	set color [MoveTypeToColor $moveType]
	#puts "---- item: $item"

	# this theMove is in internal rep format
	set theMove [lindex $item 0]
	# convert to external move format (ie from, to)
	set fromSlot [expr $theMove % ($BOARDSIZE+1)]
	set toSlot [expr $theMove / ($BOARDSIZE+1)]

	set value [lindex $item 1]

	#puts "---- <$fromSlot , $toSlot> $value"

        set color [MoveValueToColor $moveType $value]

	#puts "---- Got past color picker"

	$c itemconfig arrow$fromSlot$toSlot -fill $color
	$c raise arrow$fromSlot$toSlot all


# 	# april - replaced $item with $toSlot
# 	if {$toSlot < $from} {
# 	    set n [expr $toSlot + 1]
# 	    if {$n == $from} { 
# 		#if item + 1 = from, then it must an left arrow
# 		drawLeftArrow $c $from $toSlot $color
# 	    } else {
# 		drawUpArrow $c $from $toSlot $color
# 	    }
# 	} else {
# 	    set n [expr $toSlot - 1]
# 	    if {$n == $from} {
# 		drawRightArrow $c $from $toSlot $color
# 	    } else {
# 		drawDownArrow $c $from $toSlot $color
# 	    }
# 	}
	update idletasks
    }

    #puts "<< exit GS_ShowMoves"
}

# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
# Or I could have created all of the arrows in GS_Initialize and just raise/lower them in showing moves (just don't forget to change colors in GS_ShowMoves).
proc GS_HideMoves { c moveType position moveList} {
    global BOARDSIZE

    #puts ">> GS_HideMoves"

    set whoseTurn [GS_WhoseMove $position]
    set from [findPiece $whoseTurn $position]
    #puts $moveList
    foreach item $moveList {
	set item [lindex $item 0]

	# this theMove is in internal rep format
	set theMove [lindex $item 0]
	# convert to external move format (ie from, to)
	set fromSlot [expr $theMove % ($BOARDSIZE+1)]
	set toSlot [expr $theMove / ($BOARDSIZE+1)]

	$c lower arrow$fromSlot$toSlot all

	# if {$item < $from} {
# 	    #puts $from
# 	    #puts $item
# 	    set n [expr $item + 1]
# 	    if {$n == $from} { 
# 		#if item + 1 = from, then it must an left arrow
# 		$c lower leftArrow base
# 	    } else {
# 		$c lower upArrow base
# 	    }
# 	} else {
# 	    set n [expr $item - 1]
# 	    if {$n == $from} {
# 		$c lower rightArrow base 
# 	    } else {
# 		$c lower downArrow base 
# 	    }
# 	}
	update idletasks
    }	    
    # puts "<< exit GS_HideMoves"
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
    #puts ">> GS_HandleUndo"
    GS_DrawPosition $c $positionAfterUndo

    #puts "<< exit GS_HandleUndo"
}





# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
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
    #puts ">> GS_ColorOfPlayers"

    #puts "<< exit GS_ColorOfPlayers"

    return [list black green]
}


# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    #puts ">> GS_GameOver"
    #puts "<< GS_GameOver"
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}
