####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################
#
# Jesse Phillips
# Jennifer Lee
#
# 4/11/03  changed GS_InitGamespecific, GS_NameOfPieces, GS_Initialize
#          added functions Unhash and expt
# 4/19/03  changed GS_DrawPosition, ShowMoves, HideMoves, HandleMoves,
#          and unhash
# 4/25/03  modified GS_UndoPosition
#
#
#
####################################################

############   GLOBAL VARS #######################

global dotgap dotmid
global x1 x2 x3 y1 y2 y3
global pieceSize pieceOutline xColor oColor pieceOffset
global dotSize dotExpandAmount lineWidth lineColor baseColor base
global dotx1 dotx2 dotx3 doty1 doty2 doty3
global px1 px2 px3 py1 py2 py3
global diagArrows horizArrows vertArrows slideDelay goDelay animQuality
global canvasWidth canvasHeight

# Authors Info
global kRootDir
global kCAuthors kTclAuthors kGifAuthors
set kCAuthors "Jesse Phillips, Jennifer Lee"
set kTclAuthors "Jesse Phillips, Jennifer Lee"
set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

## IMPORTANT: These are variables used to change the board.
# board size
set canvasWidth  500; # 230 for first ever board
set canvasHeight 500
#coordinates:
set dotgap [expr $canvasWidth / 3] 
set dotmid [expr $dotgap / 2]
set firstXCoord $dotmid
set firstYCoord $dotmid
## pieces
set pieceSize [expr $dotmid - ($dotmid / 4)]
set pieceOutline [expr $pieceSize / 14]
set xColor blue4
set oColor red2
set pieceOffset $dotgap
## dots
set dotSize [expr $pieceSize.0 / 2.6]
set dotExpandAmount [expr 3 * ( $dotSize / 4 )]
## lines
set lineWidth [ expr 3 * ( $pieceOutline / 2 ) ]
set lineColor CadetBlue4
## base
set baseColor white
## arrow lists
set diagArrows  {list 15 51 26 62 48 84 59 95 24 42 35 53 57 75 68 86}
set horizArrows {list 12 21 23 32 45 54 56 65 78 87 89 98}
set vertArrows  {list 14 41 25 52 36 63 47 74 58 85 69 96}
## animation delay
set slideDelay 20000
set goDelay 3000000
set animQuality "low"
# x and y position numbers
set x1 $firstXCoord
set x2 [expr $x1 + $dotgap]
set x3 [expr $x2 + $dotgap]
set x4 $x1
set x5 $x2
set x6 $x3
set x7 $x1
set x8 $x2
set x9 $x3

set y1 $firstYCoord
set y2 $y1
set y3 $y1
set y4 [expr $y1 + $dotgap]
set y5 $y4
set y6 $y4
set y7 [expr $y4 + $dotgap]
set y8 $y7
set y9 $y7



set dotx1 [expr $x1 - [expr $dotSize / 2]];
set dotx2 [expr $x2 - [expr $dotSize / 2]];
set dotx3 [expr $x3 - [expr $dotSize / 2]];
set doty1 [expr $y1 - [expr $dotSize / 2]];
set doty2 [expr $y4 - [expr $dotSize / 2]];
set doty3 [expr $y7 - [expr $dotSize / 2]];

set px1 [expr $x1 - [expr $pieceSize / 2]];
set px2 [expr $x2 - [expr $pieceSize / 2]];
set px3 [expr $x3 - [expr $pieceSize / 2]];
set py1 [expr $y1 - [expr $pieceSize / 2]];
set py2 [expr $y4 - [expr $pieceSize / 2]];
set py3 [expr $y7 - [expr $pieceSize / 2]];



############## HELPER FUNCTIONS ######################


proc SetColour { c obj colour } {
    $c itemconfig $obj -fill $colour
}

#######################################################
#MouseOverExpand
#arg1: the dot which indicates the move
# causes the outline of this dot to expand when it is mousedOver
#
#######################################################
proc MouseOverExpand { dot c } {
    global dotExpandAmount
    $c itemconfig $dot -width $dotExpandAmount
}

#######################################################
#MouseOutContract
#arg1: the dot which indicates the move
# causes the outline of this dot to contract when it is mouseOut
#
#######################################################
proc MouseOutContract { dot c } {
    $c itemconfig $dot -width 0
}

#######################################################
#animationQualityQuery
#  asks the user whether they want low or high
#  quality animations
#######################################################
proc animationQualityQuery { c } {
    global animQuality
    set anim_choice [$c create text 120 20  -text "choose animation quality:" \
			 -fill NavyBlue \
			 -font {Helvetica -20} -tags {base anim_qual}]
    set high [$c create text 270 20 -text "HIGH" -font {Helvetica -24} \
		  -fill gray40 -tags {high anim_qual base}]
    set dash [$c create text 325 20 -text " or"  -font {Helvetica -24} \
		  -fill NavyBlue -tags {base anim_qual}]
    set low  [$c create text 370 20 -text "low"  -font {Helvetica -24} \
		  -fill gray40 -tags {low anim_qual base}]

    $c bind high <Enter> "$c itemconfig high -fill black -font {Helvetica -30}"
    $c bind high <Leave> "$c itemconfig high -fill gray40 -font {Helvetica -24}"

    $c bind low <Enter>  "$c itemconfig low -fill black -font {Helvetica -30}"
    $c bind low <Leave>  "$c itemconfig low -fill gray40 -font {Helvetica -24}"

    $c bind high <ButtonRelease-1> "set animQuality high"
    $c bind low  <ButtonRelease-1> "set animQuality low"
    
}

#######################################################
# animateMove
# arg1: whoseTurn
# arg2: the tag of the piece to be moved
# arg3; the baord position number from which the piece is being moved (1-9)
# arg4; the board position number to which to move the piece (1-9)
# arg5: teh canvas
# pieceToMove is on the board already, so we create a piece at the same spot
#   lower the previous piece, slide the created piece over, raise the 
#    piece at $whoseTurn-$to, and destroy the temporary piece
#
#######################################################
proc animateMove { whoseTurn pieceToMove from to c } {
    global oColor xColor pieceSize pieceOutline

    set xfrom [getXCoord $from]
    set yfrom [getYCoord $from]
    set color $oColor

    if {$whoseTurn == "x"} {
	set color $xColor
    }

    set temp [$c create oval [expr $xfrom - $pieceSize/2] [expr $yfrom - $pieceSize/2] \
		  [expr $xfrom + $pieceSize/2] [expr $yfrom + $pieceSize/2] \
		  -fill $color -tags temp -width $pieceOutline];
    $c raise temp base
    $c lower $pieceToMove base

    slideAnimation temp $from $to $c

    $c raise $whoseTurn-$to base
    $c delete temp

}

#slideAnimation:
# arg1: the tag of the piece to be moved
# arg2: the board position number from which the piece is being moved (1-9)
# arg3: the board position number to which to move the piece (1-9)
# arg4: c the canvas
#
# pieceToMove is on the board already, so we need to slide animate it, then put it 
# back under the board in the correct spot, and finally raise the piece at position
# $to, so that in the end we don't have two pieces of the same color at the same
# position.
#
# 1) find the distance needed to travel:
#   a. find the difference between the "from" x coord, and the "to" x coord  
#   b. do the same for the 'from' y and 'to' y
# 2) There are three cases:
#   a. the piece slides horizontally, so y is fixed, i.e. yDist == 0
#      1) the piece is sliding left so 
#   b. the piece slides vertically, so x is fixed, i.e. xDist == 0
#   c. the piece slides along a diagonal so xDist == yDist.
proc slideAnimation { pieceToMove from to c} {
    global slideDelay animQuality

    set xfrom [getXCoord $from]
    set xto   [getXCoord $to]
    set yfrom [getYCoord $from]
    set yto   [getYCoord $to]

    set xDist [expr $xto - $xfrom]
    set yDist [expr $yto - $yfrom]
    set diag  false
    set dist  0
    set xPlus 0
    set yPlus 0

    if { $animQuality == "low"  } {
	set move_size 30
    } else {
	set move_size 1
    }

    #puts "from $from to $to"

    #puts "xfrom $xfrom xto $xto"
    #puts "yfrom $yfrom yto $yto"

    #puts "xDist: $xDist yDist: $yDist" 

    if {$yDist == 0} {
	if {$xDist < 0} {
	    set dist [expr 0 - $xDist]
	    set xPlus [expr 0 - $move_size]
	} else {
	    set dist $xDist
	    set xPlus $move_size
	}
    } elseif { $xDist == 0} {
	if {$yDist < 0} {
	    set dist [expr 0 - $yDist]
	    set yPlus [expr 0 - $move_size]
	} else {
	    set dist $yDist
	    set yPlus $move_size
	}
    } elseif { $xDist == $yDist } {# then we are going 1-5, 5-1, 5-9,9-5
	if {$xDist < 0} {# 9-5 or 5-1
	    set dist [expr 0 - $xDist]
	    set xPlus [expr 0 - $move_size]
	    set yPlus [expr 0 - $move_size]
	} else {# 1-5 or 5-9
	    set dist $xDist
	    set xPlus $move_size
	    set yPlus $move_size
	}
    } elseif { $xDist == [expr 0 - $yDist] } {# then -x == y, so
	# we are going from 3-5,5-3, or 7-5, 5-7
	if {$xDist < 0} {# 3-5, or 5-7 
	    set dist [expr 0 - $xDist]
	    set xPlus [expr 0 - $move_size]
	    set yPlus $move_size
	} else { # 5-3, or 7-5
	    set dist $xDist
	    set xPlus $move_size
	    set yPlus [expr 0 - $move_size]
	}
    } else {
	#puts "BAD ELSE: slideAnimation $pieceToMove $from $to"
    }
    
    if {$animQuality == "high"} {

	set starting_offset 10000
	set offset_adjust 30
    
	for {set i 0} {$i < $dist} {incr i} {

	    $c move $pieceToMove $xPlus $yPlus
	    update idletasks

	    set time [clock clicks]
	    set new_time [clock clicks]
	    set delay [expr $time + $starting_offset]

	    while { $new_time < $delay } {
		set new_time [clock clicks]
	    }
	    
	    set starting_offset [expr $starting_offset - $offset_adjust]
	
	}
    } else {
	for {set i 0} {$i < $dist} {set i [expr $i + 30]} {

	    $c move $pieceToMove $xPlus $yPlus
	    update idletasks
	
	}

    }

}

proc getXCoord {num} {
    global x1 x2 x3 x4 x5 x6 x7 x8 x9
    if { $num == 1 } {
	return $x1
    } elseif { $num == 2 } {
	return $x2
    } elseif { $num == 3 } {
	return $x3
    } elseif { $num == 4 } {
	return $x4
    } elseif { $num == 5 } {
	return $x5
    } elseif { $num == 6 } {
	return $x6
    } elseif { $num == 7 } {
	return $x7
    } elseif { $num == 8 } {
	return $x8
    } elseif { $num == 9 } {
	return $x9
    } else {
	#puts {bad else: getXCoord}
    }
}

proc getYCoord {num} {
    global y1 y2 y3 y4 y5 y6 y7 y8 y9

    if { $num == 1 } {
	return $y1
    } elseif { $num == 2 } {
	return $y2
    } elseif { $num == 3 } {
	return $y3
    } elseif { $num == 4 } {
	return $y4
    } elseif { $num == 5 } {
	return $y5
    } elseif { $num == 6 } {
	return $y6
    } elseif { $num == 7 } {
	return $y7
    } elseif { $num == 8 } {
	return $y8
    } elseif { $num == 9 } {
	return $y9
    } else {
	#puts {bad else: getYCoord}
    }
}


proc expt { num1 exp } {
    set ans 1
    if {$num1 == 0} {
	return 0
    }
    for {set i 0} {$i < $exp} {set i [expr $i + 1]} {
	set ans [expr $ans * $num1]
    }
    return $ans
}


proc unhash { position } {
    set board {}
    if {$position >= 19683} {
	lappend board x
	set position [expr {$position - 19683}]
    } else {
	lappend board o
    }

    for {set i 8} {$i >= 0} {set i [expr $i - 1]} {
	set ifX [expr {2 * [expt 3 $i]}]
	set ifO [expr {1 * [expt 3 $i]}]
	if [expr {$position >= $ifX}] {
	    lappend board 2
	    set position [expr {$position - $ifX}]
	} elseif [expr {$position >= $ifO}] {
	    lappend board 1
	    set position [expr {$position - $ifO}]
	} else {
	    lappend board 0
	}
    }  

    #arrange board
    set orderedBoard [lindex $board 0]
    for {set i 9} {$i >= 1} {set i [expr $i - 1]} {
        lappend orderedBoard [lindex $board $i] 
    }    

    return $orderedBoard
}



# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Achi"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set toWin and toMove

    global gMisereGame
    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    set toWin2 "Connect 3 pieces in a row in any direction"

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Click on a dot to place a piece, click on an arrow to slide a piece"

}



# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
# The left player's piece should be the first item in the list.
# The right player's piece should be second.

proc GS_NameOfPieces {} {

    return [list o x]
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
    return [list red2 blue4]
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


# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code

proc GS_GetOption { } {
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
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
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}


# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"


################################    INITIALIZING THE BOARD  (LONG FUNCTION!!) ########## 

proc GS_Initialize { c } {
    global canvasWidth canvasHeight
    # you may want to start by setting the size of the canvas; this line isn't necessary
    $c configure -width $canvasWidth -height $canvasHeight

    makeBoard $c
    # create font for Winner text at end of game
    font create Winner -family arial -size 80
    #animationQualityQuery $c

    start_animation $c

}

proc makeBoard { c } {
    global dotgap dotmid
    global x1 x2 x3 y1 y4 y7
    global pieceSize pieceOutline xColor oColor pieceOffset
    global dotSize lineWidth lineColor baseColor base
    global dotx1 dotx2 dotx3 doty1 doty2 doty3
    global px1 px2 px3 py1 py2 py3
    global horizArrows diagArrows vertArrows
    global canvasWidth canvasHeight

    ### undo text
    $c create text 250 250 -text UNDO -font {Helvetica 80} -fill gray50 -tags undo


    set base [$c create rectangle 0 0  $canvasWidth $canvasHeight -fill $baseColor -tag base];

    # horizontal lines
    set topLeftHL  [$c create line $x1 $y1 $x2 $y1 -fill $lineColor -tags [list base line-12 line-21 line]];
    set topRightHL [$c create line $x2 $y1 $x3 $y1 -fill $lineColor -tags [list base line-23 line-32 line]];
    set midLeftHL  [$c create line $x1 $y4 $x2 $y4 -fill $lineColor -tags [list base line-45 line-54 line]];
    set midRightHL [$c create line $x2 $y4 $x3 $y4 -fill $lineColor -tags [list base line-56 line-65 line]];
    set botLeftHL  [$c create line $x1 $y7 $x2 $y7 -fill $lineColor -tags [list base line-78 line-87 line]];
    set botRightHL [$c create line $x2 $y7 $x3 $y7 -fill $lineColor -tags [list base line-89 line-98 line]];

    # horizontal arrows
    set arrow12 [$c create line $x1 $y1 [expr $x1 + 1.3 * $dotmid] $y1 -tags [list arrow-12 arrow]];
    set arrow21 [$c create line $x2 $y1 [expr $x2 - 1.3 * $dotmid] $y1 -tags [list arrow-21 arrow]];
    set arrow23 [$c create line $x2 $y1 [expr $x2 + 1.3 * $dotmid] $y1 -tags [list arrow-23 arrow]];
    set arrow32 [$c create line $x3 $y1 [expr $x3 - 1.3 * $dotmid] $y1 -tags [list arrow-32 arrow]];
    set arrow45 [$c create line $x1 $y4 [expr $x1 + 1.3 * $dotmid] $y4 -tags [list arrow-45 arrow]];
    set arrow54 [$c create line $x2 $y4 [expr $x2 - 1.3 * $dotmid] $y4 -tags [list arrow-54 arrow]];
    set arrow56 [$c create line $x2 $y4 [expr $x2 + 1.3 * $dotmid] $y4 -tags [list arrow-56 arrow]];
    set arrow65 [$c create line $x3 $y4 [expr $x3 - 1.3 * $dotmid] $y4 -tags [list arrow-65 arrow]];
    set arrow78 [$c create line $x1 $y7 [expr $x1 + 1.3 * $dotmid] $y7 -tags [list arrow-78 arrow]];
    set arrow87 [$c create line $x2 $y7 [expr $x2 - 1.3 * $dotmid] $y7 -tags [list arrow-87 arrow]];
    set arrow89 [$c create line $x2 $y7 [expr $x2 + 1.3 * $dotmid] $y7 -tags [list arrow-89 arrow]];
    set arrow98 [$c create line $x3 $y7 [expr $x3 - 1.3 * $dotmid] $y7 -tags [list arrow-98 arrow]];

    # vertical lines
    
    set topLeftVL  [$c create line $x1 $y1 $x1 $y4 -fill $lineColor -tags [list base line-14 line-41 line]];
    set topMidVL   [$c create line $x2 $y1 $x2 $y4 -fill $lineColor -tags [list base line-25 line-52 line]];
    set topRightVL [$c create line $x3 $y1 $x3 $y4 -fill $lineColor -tags [list base line-36 line-63 line]];
    set botLeftVL  [$c create line $x1 $y4 $x1 $y7 -fill $lineColor -tags [list base line-47 line-74 line]];
    set botMidVL   [$c create line $x2 $y4 $x2 $y7 -fill $lineColor -tags [list base line-58 line-85 line]];
    set botRightVL [$c create line $x3 $y4 $x3 $y7 -fill $lineColor -tags [list base line-69 line-96 line]];

    # vertical arrows

    set arrow14 [$c create line $x1 $y1 $x1 [expr $y1 + 1.3 * $dotmid] -tags [list arrow-14 arrow]];
    set arrow41 [$c create line $x1 $y4 $x1 [expr $y4 - 1.3 * $dotmid] -tags [list arrow-41 arrow]];
    set arrow25 [$c create line $x2 $y1 $x2 [expr $y1 + 1.3 * $dotmid] -tags [list arrow-25 arrow]];
    set arrow52 [$c create line $x2 $y4 $x2 [expr $y4 - 1.3 * $dotmid] -tags [list arrow-52 arrow]];    
    set arrow36 [$c create line $x3 $y1 $x3 [expr $y1 + 1.3 * $dotmid] -tags [list arrow-36 arrow]];
    set arrow63 [$c create line $x3 $y4 $x3 [expr $y4 - 1.3 * $dotmid] -tags [list arrow-63 arrow]];
    set arrow47 [$c create line $x1 $y4 $x1 [expr $y4 + 1.3 * $dotmid] -tags [list arrow-47 arrow]];
    set arrow74 [$c create line $x1 $y7 $x1 [expr $y7 - 1.3 * $dotmid] -tags [list arrow-74 arrow]];    
    set arrow58 [$c create line $x2 $y4 $x2 [expr $y4 + 1.3 * $dotmid] -tags [list arrow-58 arrow]];
    set arrow85 [$c create line $x2 $y7 $x2 [expr $y7 - 1.3 * $dotmid] -tags [list arrow-85 arrow]];
    set arrow69 [$c create line $x3 $y4 $x3 [expr $y4 + 1.3 * $dotmid] -tags [list arrow-69 arrow]];
    set arrow96 [$c create line $x3 $y7 $x3 [expr $y7 - 1.3 * $dotmid] -tags [list arrow-96 arrow]];

    # Diagonal Lines
    set topLeftLDL  [$c create line $x1 $y1 $x2 $y4 -fill $lineColor -tags [list base line line-15 line-51]];
#    set topLeftRDL  [$c create line $x2 $y1 $x1 $y4 -fill $lineColor -tags [list base line line-24 line-42]];
 #   set topRightLDL [$c create line $x2 $y1 $x3 $y4 -fill $lineColor -tags [list base line line-26 line-62]];
    set topRightRDL [$c create line $x3 $y1 $x2 $y4 -fill $lineColor -tags [list base line line-35 line-53]];
  #  set botLeftLDL  [$c create line $x1 $y4 $x2 $y7 -fill $lineColor -tags [list base line line-48 line-84]];
    set botLeftRDL  [$c create line $x2 $y4 $x1 $y7 -fill $lineColor -tags [list base line line-57 line-75]];
    set botRightLDL [$c create line $x2 $y4 $x3 $y7 -fill $lineColor -tags [list base line line-59 line-95]];
   # set botRightRDL [$c create line $x3 $y4 $x2 $y7 -fill $lineColor -tags [list base line line-68 line-86]];

    # diagonal arrows - forward slashes
    set arrow15 [$c create line $x1 $y1 [expr $x1 + $dotmid] [expr $y1 + $dotmid] -tags [list arrow-15 arrow]];
    set arrow51 [$c create line $x2 $y4 [expr $x2 - $dotmid] [expr $y4 - $dotmid] -tags [list arrow-51 arrow]];
    set arrow26 [$c create line $x2 $y1 [expr $x2 + $dotmid] [expr $y1 + $dotmid] -tags [list arrow-26 arrow]];
    set arrow62 [$c create line $x3 $y4 [expr $x3 - $dotmid] [expr $y4 - $dotmid] -tags [list arrow-62 arrow]];
    set arrow48 [$c create line $x1 $y4 [expr $x1 + $dotmid] [expr $y4 + $dotmid] -tags [list arrow-48 arrow]];
    set arrow84 [$c create line $x2 $y7 [expr $x2 - $dotmid] [expr $y7 - $dotmid] -tags [list arrow-84 arrow]];
    set arrow59 [$c create line $x2 $y4 [expr $x2 + $dotmid] [expr $y4 + $dotmid] -tags [list arrow-59 arrow]];
    set arrow95 [$c create line $x3 $y7 [expr $x3 - $dotmid] [expr $y7 - $dotmid] -tags [list arrow-95 arrow]];

    # diagonal arrows - back slashes
    set arrow24 [$c create line $x2 $y1 [expr $x2 - $dotmid] [expr $y1 + $dotmid] -tags [list arrow-24 arrow]];
    set arrow42 [$c create line $x1 $y4 [expr $x1 + $dotmid] [expr $y4 - $dotmid] -tags [list arrow-42 arrow]];
    set arrow35 [$c create line $x3 $y1 [expr $x3 - $dotmid] [expr $y1 + $dotmid] -tags [list arrow-35 arrow]];
    set arrow53 [$c create line $x2 $y4 [expr $x2 + $dotmid] [expr $y4 - $dotmid] -tags [list arrow-53 arrow]];
    set arrow57 [$c create line $x2 $y4 [expr $x2 - $dotmid] [expr $y4 + $dotmid] -tags [list arrow-57 arrow]];
    set arrow75 [$c create line $x1 $y7 [expr $x1 + $dotmid] [expr $y7 - $dotmid] -tags [list arrow-75 arrow]];
    set arrow68 [$c create line $x3 $y4 [expr $x3 - $dotmid] [expr $y4 + $dotmid] -tags [list arrow-68 arrow]];
    set arrow86 [$c create line $x2 $y7 [expr $x2 + $dotmid] [expr $y7 - $dotmid] -tags [list arrow-86 arrow]];    
    

    # Dots for place moves at intersections
    set placeMove1 [$c create oval $dotx1 $doty1 [expr $dotx1 + $dotSize] [expr $doty1 + $dotSize]  -tags [list placeDots place-1]];
    set placeMove2 [$c create oval $dotx2 $doty1 [expr $dotx2 + $dotSize] [expr $doty1 + $dotSize]  -tags [list placeDots place-2]];
    set placeMove3 [$c create oval $dotx3 $doty1 [expr $dotx3 + $dotSize] [expr $doty1 + $dotSize]  -tags [list placeDots place-3]];
    set placeMove4 [$c create oval $dotx1 $doty2 [expr $dotx1 + $dotSize] [expr $doty2 + $dotSize]  -tags [list placeDots place-4]];
    set placeMove5 [$c create oval $dotx2 $doty2 [expr $dotx2 + $dotSize] [expr $doty2 + $dotSize]  -tags [list placeDots place-5]];
    set placeMove6 [$c create oval $dotx3 $doty2 [expr $dotx3 + $dotSize] [expr $doty2 + $dotSize]  -tags [list placeDots place-6]];
    set placeMove7 [$c create oval $dotx1 $doty3 [expr $dotx1 + $dotSize] [expr $doty3 + $dotSize]  -tags [list placeDots place-7]];
    set placeMove8 [$c create oval $dotx2 $doty3 [expr $dotx2 + $dotSize] [expr $doty3 + $dotSize]  -tags [list placeDots place-8]];
    set placeMove9 [$c create oval $dotx3 $doty3 [expr $dotx3 + $dotSize] [expr $doty3 + $dotSize]  -tags [list placeDots place-9]];

    # Dots at intersections
    set topLeftDot  [$c create oval $dotx1 $doty1 [expr $dotx1 + $dotSize] [expr $doty1 + $dotSize]  -tags [list base dot-1]];
    set topMidDot   [$c create oval $dotx2 $doty1 [expr $dotx2 + $dotSize] [expr $doty1 + $dotSize]  -tags [list base dot-2]];
    set topRightDot [$c create oval $dotx3 $doty1 [expr $dotx3 + $dotSize] [expr $doty1 + $dotSize]  -tags [list base dot-3]];
    set midLeftDot  [$c create oval $dotx1 $doty2 [expr $dotx1 + $dotSize] [expr $doty2 + $dotSize]  -tags [list base dot-4]];
    set midMidDot   [$c create oval $dotx2 $doty2 [expr $dotx2 + $dotSize] [expr $doty2 + $dotSize]  -tags [list base dot-5]];
    set midRightDot [$c create oval $dotx3 $doty2 [expr $dotx3 + $dotSize] [expr $doty2 + $dotSize]  -tags [list base dot-6]];
    set botLeftDot  [$c create oval $dotx1 $doty3 [expr $dotx1 + $dotSize] [expr $doty3 + $dotSize]  -tags [list base dot-7]];
    set botMidDot   [$c create oval $dotx2 $doty3 [expr $dotx2 + $dotSize] [expr $doty3 + $dotSize]  -tags [list base dot-8]];
    set botRightDot [$c create oval $dotx3 $doty3 [expr $dotx3 + $dotSize] [expr $doty3 + $dotSize] -tags [list base dot-9]];
    
    # create bindings for dots:

    for {set i 1} {$i <= 9} {incr i} {
	$c itemconfig dot-$i -fill $lineColor
	$c itemconfig dot-$i -outline $lineColor -width 0
    }

    for {set i 1} {$i <= 9} {incr i} {
	$c itemconfig place-$i -fill $lineColor
	$c itemconfig place-$i -outline $lineColor -width 0
	$c bind place-$i <ButtonRelease-1> "ReturnFromHumanMove $i"
	$c bind place-$i <Enter> "MouseOverExpand place-$i $c"
	$c bind place-$i <Leave> "MouseOutContract place-$i $c"
    }

    # Pieces at intersections
    set piecex1 [$c create oval $px1 $py1 [expr $px1 + $pieceSize] [expr $py1 + $pieceSize] -fill $xColor -tags [list piece x-1]];
    set piecex2 [$c create oval $px2 $py1 [expr $px2 + $pieceSize] [expr $py1 + $pieceSize] -fill $xColor -tags [list piece x-2]];
    set piecex3 [$c create oval $px3 $py1 [expr $px3 + $pieceSize] [expr $py1 + $pieceSize] -fill $xColor -tags [list piece x-3]];
    set piecex4 [$c create oval $px1 $py2 [expr $px1 + $pieceSize] [expr $py2 + $pieceSize] -fill $xColor -tags [list piece x-4]];
    set piecex5 [$c create oval $px2 $py2 [expr $px2 + $pieceSize] [expr $py2 + $pieceSize] -fill $xColor -tags [list piece x-5]];
    set piecex6 [$c create oval $px3 $py2 [expr $px3 + $pieceSize] [expr $py2 + $pieceSize] -fill $xColor -tags [list piece x-6]];
    set piecex7 [$c create oval $px1 $py3 [expr $px1 + $pieceSize] [expr $py3 + $pieceSize] -fill $xColor -tags [list piece x-7]];
    set piecex8 [$c create oval $px2 $py3 [expr $px2 + $pieceSize] [expr $py3 + $pieceSize] -fill $xColor -tags [list piece x-8]];
    set piecex9 [$c create oval $px3 $py3 [expr $px3 + $pieceSize] [expr $py3 + $pieceSize] -fill $xColor -tags [list piece x-9]];
   
    set pieceo1 [$c create oval $px1 $py1 [expr $px1 + $pieceSize] [expr $py1 + $pieceSize] -fill $oColor -tags [list piece o-1]];
    set pieceo2 [$c create oval $px2 $py1 [expr $px2 + $pieceSize] [expr $py1 + $pieceSize] -fill $oColor -tags [list piece o-2]];
    set pieceo3 [$c create oval $px3 $py1 [expr $px3 + $pieceSize] [expr $py1 + $pieceSize] -fill $oColor -tags [list piece o-3]];
    set pieceo4 [$c create oval $px1 $py2 [expr $px1 + $pieceSize] [expr $py2 + $pieceSize] -fill $oColor -tags [list piece o-4]];
    set pieceo5 [$c create oval $px2 $py2 [expr $px2 + $pieceSize] [expr $py2 + $pieceSize] -fill $oColor -tags [list piece o-5]];
    set pieceo6 [$c create oval $px3 $py2 [expr $px3 + $pieceSize] [expr $py2 + $pieceSize] -fill $oColor -tags [list piece o-6]];
    set pieceo7 [$c create oval $px1 $py3 [expr $px1 + $pieceSize] [expr $py3 + $pieceSize] -fill $oColor -tags [list piece o-7]];
    set pieceo8 [$c create oval $px2 $py3 [expr $px2 + $pieceSize] [expr $py3 + $pieceSize] -fill $oColor -tags [list piece o-8]];
    set pieceo9 [$c create oval $px3 $py3 [expr $px3 + $pieceSize] [expr $py3 + $pieceSize] -fill $oColor -tags [list piece o-9]];
    
    ## change the widths of the lines to $lineWidth
    
    foreach item [$c find withtag line] {
	$c itemconfig $item -width $lineWidth
    }
    
    ## changing the outline widths of the pieces
    
    foreach item [$c find withtag piece] {
	$c itemconfig $item -width $pieceOutline
    }

    ## changes the arrows
    foreach item [$c find withtag arrow] {
	$c lower arrow base
	$c itemconfig $item -width [expr $lineWidth * 2] -arrow last \
	    -arrowshape {28 28 14}
    }

    ## bind the horizontal arrows
    foreach item $horizArrows {
	$c bind arrow-$item <ButtonRelease-1> "ReturnFromHumanMove $item"
	$c bind arrow-$item <Enter> "SetColour $c arrow-$item black"
    }

    ## bind the vertical arrows
    foreach item $vertArrows {
	$c bind arrow-$item <ButtonRelease-1> "ReturnFromHumanMove $item"
	$c bind arrow-$item <Enter> "SetColour $c arrow-$item black"
    }

    ## bind the diagonal arrows
    foreach item $diagArrows {
	$c bind arrow-$item <ButtonRelease-1> "ReturnFromHumanMove $item"
	$c bind arrow-$item <Enter> "SetColour $c arrow-$item black"
    }

    ###  raise the base over the pieces
    $c lower piece all
    $c lower arrow all
    $c raise base all


    
    #animationQualityQuery $c
} 

################################   END INITIALIZING THE BOARD  (LONG FUNCTION!!) ########## 

proc GS_Deinitialize { c } {
    $c delete all
    font delete Winner
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
    
    # BTW too: don't make any assumptions about the state of the board.
    set boardList [unhash $position]

    $c raise base all
    for {set i 1} {$i <= 9} {set i [expr $i + 1]} {
	set posi [lindex $boardList $i]
	if { $posi == 1 } {
	    $c raise o-$i base
	} elseif { $posi == 2 } {
	    $c raise x-$i base
	}
    }
}




# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    global canvasWidth canvasHeight goDelay
    GS_DrawPosition $c $position
    $c raise base all

    #animationQualityQuery $c

    set go [$c create text [expr $canvasWidth / 2] [expr $canvasHeight / 2] \
		-text "GO"  \
		-fill orange \
		-font {Arial -200}]
    update idletasks
    
    set end_time [expr [clock clicks] + 200000]
    while { $end_time > [clock clicks] } {
	# delay the animation
    }
    $c delete $go

}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    set boardList [unhash $position];
    set whoseTurn [lindex $boardList 0]
    return $whoseTurn    
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

# There are two cases in Achi: dartboard, and rearranger
# In the first case, 
# 1) if the move is only one number, then we simply raise the correct piece.  
# 2) If the move is a slide move, then we cause the piece (which has already
#    been raised, to slide to the correct spot.
# So we:
# 1. figure out whose turn it is for dartboard moves:
#   a. unhash the oldPosition
#   b. set whoseTurn to the first element in boardList (which will be x or o)
# 2. if theMove is less than 10, then we are in dart board moves so:
#   a. raise the appropriate piece at piece$whoseTurn$theMove
# 3. Else, theMove is a rearanger move so:
#   a. 
# 4. Also, theMove may be some invalid move, probably not, but just in case . . .

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global pieceSize pieceOutline slideDelay animQuality
    set boardList [unhash $oldPosition]
    set whoseTurn [lindex $boardList 0]
    set from 0
    set to 0

    if { [expr $theMove < 10 ] } {
	$c itemconfig $whoseTurn-$theMove -width [expr $pieceSize * 5]
	$c raise $whoseTurn-$theMove base
	set num_steps 5
	set startSize [expr $pieceSize * 5]

	if {$animQuality == "low"} {
	set num_steps 100
	}

	for {set i $startSize} {$i >= $pieceOutline} {set i [expr $i - $num_steps]} {
	    $c itemconfig $whoseTurn-$theMove -width $i
	    #puts " drop move width $i"
	    update idletasks
	}

	if { $animQuality == "high" } {
	    set start_offset 50000
	    set time [clock clicks]
	    set new_time [clock clicks]
	    while { $new_time < [expr $time + $start_offset] } {
		set new_time [clock clicks]
	    }
	}

	$c itemconfig $whoseTurn-$theMove -width $pieceOutline
	update idletasks

    } elseif { [expr $theMove > 9 && $theMove < 100] } {
	#puts {do a rearranger move}
	set from [expr $theMove / 10]
	set to   [expr $theMove % 10]
	# animate the old piece to the new position, after this is called
	# the pieces will be back in the correct spots, with the appropriatte
	# piece raised (That is, pieces will not be moved permanently).
	animateMove $whoseTurn $whoseTurn-$from $from $to $c
		
	
    } else {
	#puts {ERROR: badelse from GS_HandleMove machi.tcl}
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
# We provide a procedure called MoveValueToColor that takes in moveType and a moveValue and returns the correct color.
# For example [MoveValueToColor "value" "Tie"] would return yellow and [MoveValueToColor "all" "anything"] would return cyan.

proc GS_ShowMoves { c moveType position moveList } {
    #puts ">> ShowMoves, moveList : $moveList"
    global lineWidth
    foreach item $moveList {
	set move  [lindex $item 0]
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

	if {$move < 10} {
	    $c itemconfig place-$move -fill $color -outline $color
	    $c raise place-$move base
	} else {
	    $c itemconfig arrow-$move -fill $color
	    $c bind arrow-$move <Leave> "SetColour $c arrow-$move $color"
	    $c raise arrow-$move base
	}
	update idletasks
    }

}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    global lineWidth lineColor

    foreach item $moveList {
	set move  [lindex $item 0]

	if {$move < 10} {
	    $c lower placeDots base
	} else {
	    $c lower arrow base
	}
    }
    update idletasks
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
    #GS_DrawPosition $c $positionAfterUndo
   
    update idletasks

    set afterUndoBoard [unhash $positionAfterUndo]
    set whoseTurn [lindex $afterUndoBoard 0]
    set from 0
    set to 0
    set undoText $c 
    if { [expr $theMoveToUndo < 10] } {
	$c lower $whoseTurn-$theMoveToUndo base
    } elseif { [expr $theMoveToUndo > 9 && $theMoveToUndo < 100] } {
	#puts {do a rearranger move}
	set to [expr $theMoveToUndo / 10]
	set from   [expr $theMoveToUndo % 10]
	# animate the old piece to the new position, after this is called
	# the pieces will be back in the correct spots, with the appropriatte
	# piece raised (That is, pieces will not be moved permanently).
	animateMove $whoseTurn $whoseTurn-$from $from $to $c
		
	
    } else {
	#puts {ERROR: badelse from GS_HandleMove machi.tcl}
    }
   
}


# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    global pieceSize

    set board [unhash $position]
    if { $nameOfWinningPiece == "x" } {
	set winner 2
    } elseif { $nameOfWinningPiece == "o" } {
	set winner 1
    } else {
	#puts "BAD ELSE: GS_GameOver, nameOfWinningPiece != x or o"
    }

    set p {1 3}
    # win on the top row
    if {[lindex $board 1] == $winner && [lindex $board 2] == $winner && [lindex $board 3] == $winner} {
	set p {1 3}
    }
    # win on the middle row
    if {[lindex $board 4] == $winner && [lindex $board 5] == $winner && [lindex $board 6] == $winner} {
	set p {4 6}
    }
    # win on the bottom row
    if {[lindex $board 7] == $winner && [lindex $board 8] == $winner && [lindex $board 9] == $winner} {
	set p {7 9}
    }    
    # win on the first column
    if {[lindex $board 1] == $winner && [lindex $board 4] == $winner && [lindex $board 7] == $winner} {
	set p {1 7}
    }
    # win on the middle column
    if {[lindex $board 2] == $winner && [lindex $board 5] == $winner && [lindex $board 8] == $winner} {
	set p {2 8}
    }
    # win on the right column
    if {[lindex $board 3] == $winner && [lindex $board 6] == $winner && [lindex $board 9] == $winner} {
	set p {3 9}
    }
    # win on the left diagonal
    if {[lindex $board 1] == $winner && [lindex $board 5] == $winner && [lindex $board 9] == $winner} {
	set p {1 9}
    }
    # win on the right diagonal 
    if {[lindex $board 3] == $winner && [lindex $board 5] == $winner && [lindex $board 7] == $winner} {
	set p {3 7}
    }

    set startx [getXCoord [lindex $p 0]]
    set starty [getYCoord [lindex $p 0]] 
    set endx [getXCoord [lindex $p 1]] 
    set endy [getYCoord [lindex $p 1]]     

    $c create line $startx $starty $endx $endy -fill gray80 -tags GameOverLine -width [expr $pieceSize / 5] -capstyle round

    $c create text 250 160 -text "$nameOfWinner" -font Winner -fill orange -tags winner
    $c create text 250 340 -text "WINS!"         -font Winner -fill orange -tags winner

}

# proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner } {

#     $c create text 250 200 -text "$nameOfWinner" -font {Helvetica 90} -fill white -tags winner
#     $c create text 250 320 -text "WINS!"         -font {Helvetica 90} -fill white -tags winner
#     for {set i 0} {$i < 2} {incr i} {
# 	$c raise winner
# 	update idletasks
# 	set next_time [expr [clock clicks] + 500000]
# 	while {$next_time > [clock clicks]} {
# 	    # spin wait
# 	}
# 	$c lower winner base
# 	update idletasks
# 	set next_time [expr [clock clicks] + 500000]
# 	while {$next_time > [clock clicks]} {
# 	    # spin wait
# 	}
#     } 

#     $c raise winner
# }


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
    $c delete GameOverLine
    $c delete winner
}

proc start_animation { c } {
    global dotmid
    
    $c create text [expr 2 * $dotmid] [expr 2 * $dotmid] -text "A" -font {Helvetica 100} -fill orange -tags {a achi}
    $c create text [expr 4 * $dotmid] [expr 2 * $dotmid] -text "C" -font {Helvetica 100} -fill orange -tags {c achi}
    $c create text [expr 2 * $dotmid] [expr 4 * $dotmid] -text "H" -font {Helvetica 100} -fill orange -tags {h achi}
    $c create text [expr 4 * $dotmid] [expr 4 * $dotmid] -text "I" -font {Helvetica 100} -fill orange -tags {i achi}
    $c lower achi base
    set flash_time 90000

    $c raise a
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower a base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    
    $c raise c
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower c base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    
    $c raise h
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower h base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    
    $c raise i
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower i base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }

    $c raise achi

}

