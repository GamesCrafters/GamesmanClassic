###################################################
# this is a tcl module for the game of Shogi
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
# mhshogi-specific code by Alex Choy and Ann Chen
####################################################

set gBaseSpeed 100

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
    set kGameName "Hasami Shogi"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition

    ### Set maximum gBoardRows, gBoardCols, rowsOfPieces, gNumInALine
    global kMaxBoardRows kMaxBoardCols kMaxRowsOfPieces kMaxNumInALine kMaxCaptureVersions kCaptureAll
    set kMaxBoardRows 9
    set kMaxBoardCols 9
    set kMaxRowsOfPieces 2; # rows of pieces for each player
    set kMaxNumInALine 5; # number of pieces in a line to win
    set kMaxCaptureVersions 2; # corner capture or no corner capture
    set kCaptureAll 2

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "kStandardString not used"
    set kMisereString "kMisereString not used"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame \
	gWinCondition kVersionLine kVersionCapture kVersionBoth \
	gCaptureVersion kVersionNoCorner kVersionCorner \
	gNumInALine
    if {$gWinCondition == $kVersionLine} {
	if {!$gMisereGame} {
	    SetToWinString "To Win: Move your pieces into a connected chain of $gNumInALine pieces that is either vertical, horizontal, or diagonal. None of the pieces may be in your original starting rows."
	} else {
	    SetToWinString "To Lose: Move your pieces into a connected chain of $gNumInALine pieces that is either vertical, horizontal, or diagonal. None of the pieces may be in your original starting rows."
	}
    } elseif {$gWinCondition == $kVersionCapture} {
	if {!$gMisereGame} {
	    SetToWinString "To Win: Reduce enemy to 1 or 0 pieces."
	} else {
	    SetToWinString "To Lose: Reduce enemy to 1 or 0 pieces."
	}
    } elseif {$gWinCondition == $kVersionBoth} {
	if {!$gMisereGame} {
	    SetToWinString "To Win: Move your pieces into a connected chain of $gNumInALine pieces that is either vertical, horizontal, or diagonal. None of the pieces may be in your original starting rows. Or reduce enemy to 1 or 0 pieces."
	} else {
	    SetToWinString "To Lose: Move your pieces into a connected chain of $gNumInALine pieces that is either vertical, horizontal, or diagonal. None of the pieces may be in your original starting rows. Or reduce enemy to 1 or 0 pieces."
	}
    } else {}
    if {$gCaptureVersion == $kVersionNoCorner} {
	SetToMoveString "To Move: Verticallly or horizontally, slide in a straight line or jump over one piece of either player adjacent to the starting spot, landing at the space just beyond the jumped piece. Capture one or more enemy pieces connected in a line by making a sandwich so that you have two pieces at boths ends of the line. No corner captures are allowed."
    } elseif {$gCaptureVersion == $kVersionCorner} {
	SetToMoveString "To Move: Verticallly or horizontally, slide in a straight line or jump over one piece of either player adjacent to the starting spot, landing at the space just beyond the jumped piece. Capture one or more enemy pieces connected in a line by making a sandwich so that you have two pieces at boths ends of the line. Corner captures are allowed."
    } else {}

    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Chris Wilmore, Ann Chen"
    set kTclAuthors "Alex Choy, Ann Chen"
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

    ### Set version constant definitions
    global kVersionNoCorner kVersionCorner kVersionLine kVersionCapture kVersionBoth
    set kVersionNoCorner 1
    set kVersionCorner 0
    set kVersionLine 0
    set kVersionCapture 1
    set kVersionBoth 2

    # Declare and initialize rule globals
    global gMisereGame gBoardRows gBoardCols gRowsOfPieces gNumInALine gCaptureVersion gWinCondition \
	kVersionNoCorner kVersionCorner kVersionLine kVersionCapture kVersionBoth
    set gMisereGame 0
    set gBoardRows 4
    set gBoardCols 4
    set gRowsOfPieces 1; # 1 row of pieces for each player
    set gNumInALine 4; # 4 in a line to win
    set gCaptureVersion $kVersionCorner; # corner capture allowed
    set gWinCondition $kVersionBoth; # both in a line and capture all

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    set boardColsRule \
	[list \
	     "Board width:" \
	     "2" \
	     "3" \
	     "4" \
	     "5" \
	     "6" \
	     "7" \
	     "8" \
	     "9"
	    ]

    set boardRowsRule \
	[list \
	     "Board height:" \
	     "3" \
	     "4" \
	     "5" \
	     "6" \
	     "7" \
	     "8" \
	     "9"
	    ]

    set rowsOfPiecesRule \
	[list \
	     "Rows of pieces per player:" \
	     "1" \
	     "2"
	    ]

    set numInALineRule \
	[list \
	     "Number of pieces in a line to win:" \
	     "2" \
	     "3" \
	     "4" \
	     "5"
	    ]

    set captureVersionRule \
	[list \
	     "Should corner captures be allowed?" \
	     "Yes" \
	     "No" \
	    ]

    set winConditionRule \
	[list \
	     "Winning condition" \
	     "$gNumInALine pieces in a line" \
	     "Reduce enemy to 1 or 0 pieces" \
	     "Both"
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $boardColsRule $boardRowsRule $rowsOfPiecesRule $numInALineRule $captureVersionRule $winConditionRule]

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gBoardRows" "gBoardCols" "gRowsOfPieces" "gNumInALine" "gCaptureVersion" "gWinCondition"]

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
    global gMisereGame gBoardRows gBoardCols gRowsOfPieces gCaptureVersion gWinCondition gNumInALine \
	kMaxBoardRows kMaxBoardCols kMaxRowsOfPieces kMaxNumInALine kMaxCaptureVersions kCaptureAll \
	kVersionLine kVersionCapture kVersionBoth

    return [expr {$gMisereGame \
		      + 2 * $gBoardCols \
		      + 2 * $kMaxBoardCols * $gBoardRows \
		      + 2 * $kMaxBoardCols * $kMaxBoardRows * $gRowsOfPieces \
		      + 2 * $kMaxBoardCols * $kMaxBoardRows * $kMaxRowsOfPieces * $gCaptureVersion \
		      + 2 * $kMaxBoardCols * $kMaxBoardRows * $kMaxRowsOfPieces * $kMaxCaptureVersions * ($gWinCondition != $kVersionLine ? 1 : 0) \
		      + 2 * $kMaxBoardCols * $kMaxBoardRows * $kMaxRowsOfPieces * $kMaxCaptureVersions * $kCaptureAll * ($gWinCondition == $kVersionCapture ? 0 : $gNumInALine - 1)}]
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
    global gMisereGame gBoardRows gBoardCols gRowsOfPieces gCaptureVersion gWinCondition gNumInALine \
	kMaxBoardRows kMaxBoardCols kMaxRowsOfPieces kMaxNumInALine kMaxCaptureVersions kCaptureAll \
	kVersionLine kVersionCapture kVersionBoth
  
    set gBoardCols [expr $option % $kMaxBoardCols]
    set gBoardRows [expr ($option / $kMaxBoardCols) % $kMaxBoardRows]
    set gRowsOfPieces [expr ($option / $kMaxBoardCols / $kMaxBoardRows) % $kMaxRowsOfPieces]
    set gCaptureVersion [expr ($option / $kMaxBoardCols / $kMaxBoardRows / $kMaxRowsOfPieces) % $kMaxCaptureVersions]
    set captureAll [expr ($option / $kMaxBoardCols / $kMaxBoardRows / $kMaxRowsOfPieces / $kMaxCaptureVersions) % $kCaptureAll]
    set inALine [expr ($option / $kMaxBoardCols / $kMaxBoardRows / $kMaxRowsOfPieces / $kMaxCaptureVersions / $kCaptureAll) %  $kMaxNumInALine]

    if {$captureAll == 1 && $inALine != 0} {
	set gNumInALine [expr $inALine + 1]
	set gWinCondition [expr $kVersionBoth]
    } elseif {$captureAll == 1} {
	set gWinCondition $kVersionCapture
    } else {
	set gWinCondition $kVersionLine
	set gNumInRow [expr $inALine + 1]
    }
}


proc max { x y } {
    if {$x > $y} {
	return $x
    } else {
	return $y
    }
}


proc min { x y } {
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

proc DrawCircle { w slotX slotY theTag theColor } {

    global gCellSize

    set circleWidth [expr $gCellSize/10.0]
    set startCircle [expr $gCellSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$gCellSize]
    set cornerY     [expr $slotY*$gCellSize]
    set theCircle   [$w create oval $startCircle $startCircle \
			 $endCircle $endCircle \
			 -outline $theColor \
			 -fill $theColor \
			 -tag $theTag]

    $w move $theCircle $cornerX $cornerY
    $w addtag tagPieceCoord$slotX$slotY withtag $theCircle
    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle

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
    global gCanvasWidth gCanvasHeight gBoardRows gBoardCols gBoardSize
    global gFrameWidth gFrameHeight
    global gPieceSize gCellSize
    global gFontSize

    global gPosition

    set gBoardSize [expr $gBoardRows * $gBoardCols]

    set gCanvasWidth  [$c cget -width]
    set gCanvasHeight $gCanvasWidth
    set gMinFrameLength [min $gFrameWidth $gFrameHeight]
    #set boardMax [max $gBoardRows $gBoardCols]
    set gCellSize [expr [min $gFrameWidth $gFrameHeight] / 4]

    set horizontalStart [expr [expr $gCanvasWidth - [expr $gCellSize * [expr $gBoardCols - 1]]] / 2]
    set verticalStart [expr [expr $gCanvasHeight - [expr $gCellSize * [expr $gBoardRows - 1]]] / 2]

    set gPieceSize [expr $gCellSize / 4]
    set gFontSize [expr int($gPieceSize * 3 / 2)]
    #set xColor [lindex [GS_ColorOfPlayers] 0]
    #set oColor [lindex [GS_ColorOfPlayers] 1]
    set arrowWidth [expr $gCellSize / 15]
    set arrowWidth2 [expr $arrowWidth * 2]
    set offset [list [expr -1 * $gPieceSize * 2 / 3] 0 [expr $gPieceSize * 2 / 3]]
    #set offsetSource [expr $gPieceSize / 10] 
    set arrowLength [list [expr $gCellSize - $gPieceSize] [expr [expr $gCellSize * 2] - $gPieceSize] [expr [expr $gCellSize * 3] - $gPieceSize]]

    for {set x 0} {$x < $gBoardCols} {incr x} {
	for {set y 0} {$y < $gBoardRows} {incr y} {
	    $c create rectangle [toCanvasCoord $x] [toCanvasCoord $y] [toCanvasCoord [expr $x + 1]] [toCanvasCoord [expr $y + 1]] -fill white -outline black -tag base 
	}
    }

    for {set cols 0} {$cols < $gBoardCols} {incr cols} {
	DrawCircle $c $cols 0 pieces [lindex [GS_ColorOfPlayers] 0]
	DrawCircle $c $cols 3 pieces [lindex [GS_ColorOfPlayers] 1]
    }

    for {set x 0} {$x < $gBoardCols} {incr x} {
	set horizontalOffset [expr $horizontalStart + [toCanvasCoord $x]]
	for {set y 0} {$y < $gBoardRows} {incr y} {
	    set verticalOffset [expr $verticalStart + [toCanvasCoord $y]]
	    # arrows with length 1
	    if {($y >= 0) && ($y <= [expr $gBoardRows - 2])} {
		set temp [expr $y + 1]
		$c create line [expr $horizontalOffset + [lindex $offset 0]]  $verticalOffset [expr $horizontalOffset + [lindex $offset 0]] [expr $verticalOffset + [lindex $arrowLength 0]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength1 arrowDown arrow$x$y$x$temp arrow]
	    }
	    if {($y >= 1) && ($y <= [expr $gBoardRows - 1])} {
		set temp [expr $y - 1]
		$c create line [expr $horizontalOffset + [lindex $offset 2]] $verticalOffset [expr $horizontalOffset + [lindex $offset 2]] [expr $verticalOffset - [lindex $arrowLength 0]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength1 arrowUp arrow$x$y$x$temp arrow]
	    }
	    if {($x >= 0) && ($x <= [expr $gBoardCols - 2])} {
		set temp [expr $x + 1]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 0]] [expr $horizontalOffset + [lindex $arrowLength 0]] [expr $verticalOffset + [lindex $offset 0]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength1 arrowRight arrow$x$y$temp$y arrow]
	    }
	    if {($x >= 1) && ($x <= [expr $gBoardCols - 1])} {
		set temp [expr $x - 1]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 2]] [expr $horizontalOffset - [lindex $arrowLength 0]] [expr $verticalOffset + [lindex $offset 2]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength1 arrowLeft arrow$x$y$temp$y arrow]
	    }
	    # arrows with length 2 (side)
	    if {($y >= 1) && ($y <= [expr $gBoardRows - 3])} {
		set temp [expr $y + 2]
		$c create line [expr $horizontalOffset + [lindex $offset 2]] $verticalOffset [expr $horizontalOffset + [lindex $offset 2]] [expr $verticalOffset + [lindex $arrowLength 1]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowDown arrow$x$y$x$temp arrow]
	    }
	    if {($y >= 2) && ($y <= [expr $gBoardRows - 2])} {
		set temp [expr $y - 2]
		$c create line [expr $horizontalOffset + [lindex $offset 0]] $verticalOffset [expr $horizontalOffset + [lindex $offset 0]] [expr $verticalOffset - [lindex $arrowLength 1]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowUp arrow$x$y$x$temp arrow]
	    }
	    if {($x >= 1) && ($x <= [expr $gBoardCols - 3])} {
		set temp [expr $x + 2]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 2]] [expr $horizontalOffset + [lindex $arrowLength 1]] [expr $verticalOffset + [lindex $offset 2]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowRight arrow$x$y$temp$y arrow]
	    }
	    if {($x >= 2) && ($x <= [expr $gBoardCols - 2])} {
		set temp [expr $x - 2]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 0]] [expr $horizontalOffset - [lindex $arrowLength 1]] [expr $verticalOffset + [lindex $offset 0]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowLeft arrow$x$y$temp$y arrow]
	    }
	    # arrows with length 2 (middle)
	    if {$y == 0} {
		set temp [expr $y + 2]
		$c create line [expr $horizontalOffset + [lindex $offset 1]] $verticalOffset [expr $horizontalOffset + [lindex $offset 1]] [expr $verticalOffset + [lindex $arrowLength 1]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowDown arrow$x$y$x$temp arrow]
	    }
	    if {$y == [expr $gBoardRows - 1]} {
		set temp [expr $y - 2]
		$c create line [expr $horizontalOffset + [lindex $offset 1]] $verticalOffset [expr $horizontalOffset + [lindex $offset 1]] [expr $verticalOffset - [lindex $arrowLength 1]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowUp arrow$x$y$x$temp arrow]
	    }
	    if {$x == 0} {
		set temp [expr $x + 2]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 1]] [expr $horizontalOffset + [lindex $arrowLength 1]] [expr $verticalOffset + [lindex $offset 1]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowRight arrow$x$y$temp$y arrow]
	    }
	    if {$x == [expr $gBoardCols - 1]} {
		set temp [expr $x - 2]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 1]] [expr $horizontalOffset - [lindex $arrowLength 1]] [expr $verticalOffset + [lindex $offset 1]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength2 arrowLeft arrow$x$y$temp$y arrow]
	    }
	    # arrows with length 3
	    if {($y >= 0) && ($y <= [expr $gBoardRows - 4])} {
		set temp [expr $y + 3]
		$c create line [expr $horizontalOffset + [lindex $offset 2]] $verticalOffset [expr $horizontalOffset + [lindex $offset 2]] [expr $verticalOffset + [lindex $arrowLength 2]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength3 arrowDown arrow$x$y$x$temp arrow]
	    }
	    if {($y >= [expr $gBoardRows - 1]) && ($y <= 3)} {
		set temp [expr $y - 3]
		$c create line [expr $horizontalOffset + [lindex $offset 0]] $verticalOffset [expr $horizontalOffset + [lindex $offset 0]] [expr $verticalOffset - [lindex $arrowLength 2]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength3 arrowUp arrow$x$y$x$temp arrow]
	    }
	    if {($x >= 0) && ($x <= [expr $gBoardRows - 4])} {
		set temp [expr $x + 3]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 2]] [expr $horizontalOffset + [lindex $arrowLength 2]] [expr $verticalOffset + [lindex $offset 2]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength3 arrowRight arrow$x$y$temp$y arrow]
	    }
	    if {($x >= [expr $gBoardCols - 1]) && ($x <= 3)} {
		set temp [expr $x - 3]
		$c create line $horizontalOffset [expr $verticalOffset + [lindex $offset 0]] [expr $horizontalOffset - [lindex $arrowLength 2]] [expr $verticalOffset + [lindex $offset 0]] -arrow last -width $arrowWidth -arrowshape [list $arrowWidth2 $arrowWidth2 $arrowWidth] -tags [list arrowLength3 arrowLeft arrow$x$y$temp$y arrow]
	    }
	}
    }

    $c raise base
#    $c raise pieces
#    $c raise arrow
#    $c raise arrowDown
#    $c raise arrowUp
#    $c raise arrowLeft
#    $c raise arrowRight

    #global gInitialPosition
    #GS_DrawPosition $c $gInitialPosition

    set pieceString "oooo        xxxx"
    DrawBoardString $c $pieceString
    $c create rectangle 0 [expr $gCanvasWidth/2 - 80] $gMinFrameLength [expr $gCanvasWidth/2 + 80] -fill gray -width 1 -outline black
    $c create text [expr $gCanvasWidth/2] [expr $gCanvasWidth/2] -text "Welcome to Hasami Shogi!" -font "Arial $gFontSize" -anchor center -fill black -width $gCanvasWidth -justify center
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
    global gBoardSize
    set pieceString [yCoordConvert [string range [C_GenericUnhash $position $gBoardSize] 0 [expr $gBoardSize-1]]]
    return [DrawBoardString $c $pieceString]

}
proc DrawBoardString { c pieceString } {
    global gBoardCols gBoardRows
    
    $c raise base all
    $c delete pieces
    set pieceNumber 0

    for {set rows 0} {$rows < $gBoardRows} {set rows [expr $rows + 1]} {
	for {set cols 0} {$cols < $gBoardCols} {set cols [expr $cols + 1]} {
	    if {[string compare [string index $pieceString $pieceNumber] [lindex [GS_NameOfPieces] 0]] == 0} {
		DrawCircle $c $cols $rows pieces [lindex [GS_ColorOfPlayers] 0]
	    } elseif {[string compare [string index $pieceString $pieceNumber] [lindex [GS_NameOfPieces] 1]] == 0} {
		DrawCircle $c $cols $rows pieces [lindex [GS_ColorOfPlayers] 1]
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
proc GS_HandleMove { c oldPosition move newPosition } {

    global gBoardSize gBoardCols

    set source [GetSourceFromMove $move]
    set dest [GetDestFromMove $move]
    set sourceCol [lindex $source 0]
    set sourceRow [lindex $source 1]
    set sourceIndex [expr [expr $sourceRow * $gBoardCols] + $sourceCol]
    set oldBoard [yCoordConvert [C_GenericUnhash $oldPosition $gBoardSize]]
    set newBoard [yCoordConvert [C_GenericUnhash $newPosition $gBoardSize]]
    set movedpiece [string index $oldBoard $sourceIndex]
    if {[string compare $movedpiece [lindex [GS_NameOfPieces] 0]] == 0} {
	set color [lindex [GS_ColorOfPlayers] 0]
	set enemyColor [lindex [GS_ColorOfPlayers] 1]
    } elseif {[string compare $movedpiece [lindex [GS_NameOfPieces] 1]] == 0} {
	set color [lindex [GS_ColorOfPlayers] 1]
	set enemyColor [lindex [GS_ColorOfPlayers] 0]
    } else {}

    set oldBoard [string replace $oldBoard $sourceIndex $sourceIndex " "]

    for {set i 0} {$i < $gBoardSize} {incr i} {
	if { [string index $oldBoard $i] !=  " " && [string index $newBoard $i] == " " } {
	    lappend captured [DrawCircle $c [expr $i % $gBoardCols] [expr $i / $gBoardCols] captured $enemyColor]
	}
    }

    DrawBoardString $c $oldBoard
    set piece [DrawCircle $c $sourceCol $sourceRow piece $color]    
    animate $c $piece $source $dest 0
    GS_DrawPosition $c $newPosition
}

####################################################
# This animates a piece from source to destination #
####################################################
proc animate { c piece source destination undo } {

    global gCellSize gBaseSpeed gAnimationSpeed

    set x0 [lindex $source 0]
    set x1 [lindex $destination 0]
    set y0 [lindex $source 1]
    set y1 [lindex $destination 1]

    # Relative speed factor gotten from gAnimationSpeed
    # speed should equal the amount of ms we take to run this whole thing
    set speed [expr $gBaseSpeed / pow(2, $gAnimationSpeed)]
    
    # If things get too fast, just make it instant
    if {$speed < 10} {
	set speed 10
    }

    set dx [expr [toCanvasCoord [expr $x1 - $x0]] / $speed]
    set dy [expr [toCanvasCoord [expr $y1 - $y0]] / $speed]

    for {set i 0} {$i < $speed} {incr i} {
	$c move $piece $dx $dy
	
	after 1
	update idletasks
    }
    if {$undo} {
	$c raise uncaptured
    } else {
	$c delete captured
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
    global gBoardSize gBoardCols
    foreach item $moveList {
	set move [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	set boardString [yCoordConvert [C_GenericUnhash $position $gBoardSize]]
	
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red4
	    }
	}

	set source [GetSourceFromMove $move]
	set sourcex [lindex $source 0]
	set sourcey [lindex $source 1]
	set source $sourcex$sourcey
	set dest [GetDestFromMove $move]
	set destx [lindex $dest 0]
	set desty [lindex $dest 1]
	set dest $destx$desty
	$c raise arrow$source$dest
	$c itemconfig arrow$source$dest -fill $color
	if {[lindex [$c gettags arrow$source$dest] 0] == "arrowLength2"} {
	    switch [lindex [$c gettags arrow$source$dest] 1] {
		arrowLeft {
		    if {[string index $boardString [expr $sourcey * $gBoardCols + ($sourcex - 1)]] != " "} {
			$c addtag arrowJump withtag arrow$source$dest
		    }
		}
		arrowRight {
		    if {[string index $boardString [expr $sourcey * $gBoardCols + ($sourcex + 1)]] != " "} {
			$c addtag arrowJump withtag arrow$source$dest
		    }
		}
		arrowUp {
		    if {[string index $boardString [expr ($sourcey - 1) * $gBoardCols + $sourcex]] != " "} {
			$c addtag arrowJump withtag arrow$source$dest
		    }
		}
		arrowDown {
		    if {[string index $boardString [expr ($sourcey + 1) * $gBoardCols + $sourcex]] != " "} {
			$c addtag arrowJump withtag arrow$source$dest
		    }
		}
		default {}
	    }
	}
	$c bind arrow$source$dest <ButtonRelease-1> "ReturnFromHumanMove $move"
	$c bind arrow$source$dest <Enter> "$c itemconfig arrow$source$dest -fill black"
	$c bind arrow$source$dest <Leave> "$c itemconfig arrow$source$dest -fill $color"
    }
    $c raise pieces
    $c raise arrowJump
    $c dtag arrowJump arrowJump
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
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

    global gBoardSize gBoardCols

    set source [GetDestFromMove $theMoveToUndo]
    set dest [GetSourceFromMove $theMoveToUndo]
    set sourceCol [lindex $source 0]
    set sourceRow [lindex $source 1]
    set sourceIndex [expr [expr $sourceRow * $gBoardCols] + $sourceCol]
    set oldBoard [yCoordConvert [C_GenericUnhash $currentPosition $gBoardSize]]
    set newBoard [yCoordConvert [C_GenericUnhash $positionAfterUndo $gBoardSize]]
    set movedpiece [string index $oldBoard $sourceIndex]
    if {[string compare $movedpiece [lindex [GS_NameOfPieces] 0]] == 0} {
	set color [lindex [GS_ColorOfPlayers] 0]
	set enemyColor [lindex [GS_ColorOfPlayers] 1]
    } elseif {[string compare $movedpiece [lindex [GS_NameOfPieces] 1]] == 0} {
	set color [lindex [GS_ColorOfPlayers] 1]
	set enemyColor [lindex [GS_ColorOfPlayers] 0]
    } else {}

    set oldBoard [string replace $oldBoard $sourceIndex $sourceIndex " "]

    for {set i 0} {$i < $gBoardSize} {incr i} {
	if { [string index $oldBoard $i] ==  " " && [string index $newBoard $i] != " " } {
	    lappend uncaptured [DrawCircle $c [expr $i % $gBoardCols] [expr $i / $gBoardCols] uncaptured $enemyColor]
	}
    }

    DrawBoardString $c $oldBoard

    set piece [DrawCircle $c $sourceCol $sourceRow piece $color]

    animate $c $piece $source $dest 1

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
    global gCanvasWidth gCellSize gPieceSize gFontSize
    foreach coord [InARow $position $nameOfWinningPiece $lastMove] {
	set source [lindex $coord 0]
	set dest [lindex $coord 1]
	set sourcex [lindex $source 0]
	set sourcey [lindex $source 1]
	set destx [lindex $dest 0]
	set desty [lindex $dest 1]
	$c create line [expr [toCanvasCoord $sourcex] + ($gCellSize / 2)] [expr [toCanvasCoord $sourcey] + ($gCellSize / 2)] [expr [toCanvasCoord $destx] + ($gCellSize / 2)] [expr [toCanvasCoord $desty] + ($gCellSize / 2)] -fill gray70 -tags GameOverLine -width [expr $gPieceSize / 2] -capstyle round
    }

    $c create text [expr $gCanvasWidth/2] [expr $gCanvasWidth/2 - 40] -text "$nameOfWinner" -font "Arial $gFontSize" -fill orange -tags winner
    $c create text [expr $gCanvasWidth/2] [expr $gCanvasWidth/2 + 40] -text "WINS!"         -font "Arial $gFontSize" -fill orange -tags winner
}

proc toCanvasCoord {coord} {
    global gCellSize
    return [expr $coord * $gCellSize]
}

proc InARow { position nameOfWinningPiece lastMove } {

    global gBoardSize gBoardRows gBoardCols gNumInALine

    set boardList [yCoordConvert [C_GenericUnhash $position $gBoardSize]]
    set dest [GetDestFromMove $lastMove]
    set destCol [lindex [GetDestFromMove $lastMove] 0]
    set destRow [lindex [GetDestFromMove $lastMove] 1]
    set destIndex [expr [expr $destRow * $gBoardCols] + $destCol]
    if {$nameOfWinningPiece == [lindex [GS_NameOfPieces] 0]} {
	set badRow [expr $gBoardRows - 1]
    } else {
	set badRow 0
    }

    # check left
    set rows $destRow
    for {set cols $destCol} {$cols >= 0} {incr cols -1} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr cols
    set left [list $cols $rows]

    # check right
    set rows $destRow
    for {set cols $destCol} {$cols < $gBoardCols} {incr cols} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr cols -1
    set right [list $cols $rows]

    # check up
    set cols $destCol
    for {set rows $destRow} {$rows >= 0 && $rows != $badRow} {incr rows -1} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr rows
    set up [list $cols $rows]

    # check down
    set cols $destCol
    for {set rows $destRow} {$rows < $gBoardRows && $rows != $badRow} {incr rows} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr rows -1
    set down [list $cols $rows]

    # check diagonal left up
    for {set rows $destRow; set cols $destCol} {$rows >= 0 && $cols >= 0 && $rows != $badRow} {incr rows -1; incr cols -1} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr rows 1
    incr cols 1
    set leftUp [list $cols $rows]

    # check diagonal right up
    for {set rows $destRow; set cols $destCol} {$rows < $gBoardRows && $cols >= 0 && $rows != $badRow} {incr rows 1; incr cols -1} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr rows -1
    incr cols 1
    set rightUp [list $cols $rows]

    # check diagonal left down
    for {set rows $destRow; set cols $destCol} {$rows >= 0 && $cols < $gBoardCols && $rows != $badRow} {incr rows -1; incr cols 1} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr rows 1
    incr cols -1
    set leftDown [list $cols $rows]

    # check diagonal right down
    for {set rows $destRow; set cols $destCol} {$rows < $gBoardRows && $cols < $gBoardCols && $rows != $badRow} {incr rows 1; incr cols 1} {
	if {[string index $boardList [expr ($rows * $gBoardCols) + $cols]] != $nameOfWinningPiece} {
	    break
	}
    }
    incr rows -1
    incr cols -1
    set rightDown [list $cols $rows]

    set coordList {}
    # check horizontal
    if {[expr [lindex $right 0] - [lindex $left 0] + 1] >= $gNumInALine} {
	lappend coordList [list $left $right]
    }
    # check vertical
    if {[expr [lindex $down 1] - [lindex $up 1] + 1] >= $gNumInALine} {
	lappend coordList [list $up $down]
    }
    # check diagonal top left to bottom right
    if {[expr [lindex $rightDown 0] - [lindex $leftUp 0] + 1] >= $gNumInALine} {
	lappend coordList [list $leftUp $rightDown]
    }
    # check diagonal top right to bottom left
    if {[expr [lindex $rightUp 0] - [lindex $leftDown 0] + 1] >= $gNumInALine} {
	lappend coordList [list $rightUp $leftDown]
    }

    return $coordList
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

    $c delete GameOverLine
    $c delete winner

}


proc GetDestFromMove {theMove} {
    global gBoardRows gBoardCols
    set width [expr [expr $theMove >> 8] & 0xff]
    set height [expr [expr $gBoardRows - 1] - [expr $theMove & 0xff]]
    return [list $width $height]
}


proc GetSourceFromMove {theMove} {
    global gBoardRows gBoardCols
    set width [expr [expr $theMove >> 24] & 0xff]
    set height [expr [expr $gBoardRows - 1] - [expr [expr $theMove >> 16] & 0xff]]
    return [list $width $height]
}


proc yCoordConvert {pieceString} {
    global gBoardRows gBoardCols gBoardSize
    set newString ""
    for {set rows [expr $gBoardRows - 1]} {$rows >= 0} {set rows [expr $rows - 1]} {
	append newString [string range $pieceString [expr $rows * $gBoardCols] [expr [expr $rows * $gBoardCols] + [expr $gBoardCols - 1]]]
    }
    return $newString
}
