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
    
    global kGameName
    set kGameName "DinoDodgem"
    
    ### Set boardWidth, boardSize
    global boardWidth boardSize gBoardSizeOp 
    set boardWidth [expr $gBoardSizeOp + 3]
    set boardSize [expr $boardWidth * $boardWidth]

    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
     if {$boardWidth == 3} {
	set gInitialPosition 2183
    } elseif {$boardWidth == 4} {
	set gInitialPosition 414871
    } elseif {$boardWidth == 5} {
	set gInitialPosition 125030429
    }
    set gPosition $gInitialPosition

  
    ### Variants
    global gToTrapIsToWin gForwardStart 
    global gOpponentsSpace gForbidden gBuckets
    set gForbidden 1
    set gBuckets 0    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to reach the goal WINS"
    set kMisereString "First player to reach the goal LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Be the first player to move both your pieces off of the board or prevent \
	    your opponent from moving"
    } else {
	SetToWinString "To Win: Be the last player to move your pieces off of the board or to be \
prevented from moving by your opponent's pieces"
    }
    SetToMoveString "To Move: Select the arrow of the direction that you want to move your piece."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Ming Can Chang and Desmond Cheung"
    set kTclAuthors "John Lo and Diana Fang"
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
    set boardsizeRule \
	[list \
	     "What size board would you like to play on?" \
	     "3x3" \
	     "4x4" \
	     "5x5"
	]
    set TrapToWinRule \
	[list \
	     "Should Trapping be a win?" \
	     "No" \
	     "Yes" 
	]
    set ForwardStartRule \
	[list \
	     "Should the pieces in the start area move forward first?" \
	     "No" \
	     "Yes"
	 ]
    set OpponentsSpaceRule \
	[list \
	     "Should you be able to go into your opponents starting space?" \
	     "No"\
	     "Yes"
	]

    # List of all rules, in some order
    set ruleset [list $standardRule $boardsizeRule $TrapToWinRule $ForwardStartRule $OpponentsSpaceRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0
    
    global gBoardSizeOp
    set gBoardSizeOp 2

    global gToTrapIsToWin gForwardStart 
    set gToTrapIsToWin 1
    set gForwardStart 1
    global gOpponentsSpace gForbidden gBuckets 
    set gOpponentsSpace 0
    set gForbidden 1
    set gBuckets 0
    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gBoardSizeOp" "gToTrapIsToWin" "gForwardStart" "gOpponentsSpace"]

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

    global gMisereGame gToTrapIsToWin gForwardStart 
    global gOpponentsSpace gForbidden gBuckets 
    global gBoardSizeOp
    set option 1
    set option [expr $option + $gMisereGame]
    set option [expr $option + (2 * $gToTrapIsToWin)]
    set option [expr $option + (2*2*$gForwardStart)]
    set option [expr $option + (2*2*2*$gOpponentsSpace)]
    set option [expr $option + (2*2*2*2*$gForbidden)]
    set option [expr $option + (2*2*2*2*2*$gBuckets)]
    set option [expr $option + (2*2*2*2*2*2*$gBoardSizeOp)]
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
   
    global gMisereGame gToTrapIsToWin gForwardStart 
    global gOpponentsSpace gForbidden gBuckets 
    global boardWidth boardSize gBoardSizeOp
    set option [expr $option - 1]
    set gMisereGame [expr $option%2]
    set gToTrapIsToWin [expr $option/2%2]
    set gForwardStart [expr $option/2/2%2] 
    set gOpponentsSpace [expr $option/2/2/2%2]
    set gForbidden [expr $option/2/2/2/2%2]
    set gBuckets [expr $option/2/2/2/2/2%2]

    set boardWidth [expr ($option/2/2/2/2/2/2) + 3]
    set boardSize [expr $boardWidth * $boardWidth]
    set gBoardSizeOp [expr $boardWidth - 3]
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

    global boardWidth boardSize xbmLightGrey
    global gFrameWidth gFrameHeight numPieces
    global cell amount mySize yOffset numBars
    global gForbidden
    set amount -1
    set winByBarSizePercent 0.90
    set mySize [expr [min $gFrameHeight $gFrameWidth] * $winByBarSizePercent] 
    set yOffset [expr [min $gFrameHeight $gFrameWidth] * (1 - $winByBarSizePercent)]
    set cellSize [expr $mySize / ($boardWidth+1)]

    # draw the win-by bar
    if { [expr $boardWidth > 4] && $gForbidden } { 
	set numBars [expr [expr $boardWidth - 2] * $boardWidth]
    } else { 
	set numBars [expr [expr $boardWidth - 1] * $boardWidth]
    }
    set numPieces [expr $numBars / $boardWidth]

    set cell [expr $mySize / $numBars]
    set i 0
    $c create oval [expr $mySize - $yOffset] 0 \
	[expr $mySize + $yOffset] [expr 2 * $yOffset] \
	-fill [lindex [GS_ColorOfPlayers] 0] \
	-outline black \
	-tag [list base bars bar$numBars]
    $c itemconfig bar$numPieces -fill black
    for {set x 0} {$x < $numBars} {incr x} {
	$c create rect [expr $x * $cell] 0 \
	    [expr [expr $x + 1] * $cell] $yOffset \
	    -fill [lindex [GS_ColorOfPlayers] 0] \
	    -outline black \
	    -tag [list base bars bar$i]
	set i [expr $i + 1]
    }
    set i [expr $numBars + 1]
    for {set y 0} {$y < $numBars} {incr y} {
	$c create rect $mySize [expr $y * $cell + $yOffset + 1] \
	    [expr $mySize + $yOffset] [expr [expr $y + 1] * $cell + $yOffset + 1] \
	    -fill [lindex [GS_ColorOfPlayers] 1] \
	    -outline black \
	    -tag [list base bars bar$i]
	set i [expr $i + 1]
    }

    # Drawing the base
    $c create rectangle 0 [expr 0 + $yOffset] $mySize [expr $mySize + $yOffset] \
	-outline black -width 3 -fill white -tag base
    # Squares labeled from bottom left corner starting left to right from 0
    for {set x 0} {$x < $boardWidth} {incr x} {
	for {set y 1} {$y < [expr $boardWidth+1]} {incr y} {
	    $c create rectangle [expr $x * $cellSize] [expr $y * $cellSize + $yOffset] \
		[expr ($x + 1) * $cellSize] [expr ($y + 1) * $cellSize + $yOffset] \
		-fill grey -outline black -tags [list base square[expr $x+((($boardWidth -1) - ($y - 1)) * $boardWidth)]] 
	}
    } 

    #Draw the goals
    font configure GOALtext -size [expr round($cellSize/6)]
	for {set x 1} {$x < $boardWidth} {incr x} {
	    set y 0
	    $c create oval [expr $x * $cellSize] [expr $y * $cellSize + $yOffset] \
		[expr ($x + 1) * $cellSize] [expr ($y + 1) * $cellSize + $yOffset] \
		-fill blue3 -outline black -tags [list base goal] \
		-stipple gray75 -width 3
	    $c create text  [expr (($x * $cellSize) + ($x + 1) * $cellSize)/2] \
		[expr ($y * $cellSize + ($y+1) *$cellSize)/2 + $yOffset] -text "GOAL" -justify center \
		-font GOALtext -tags [list base goaltext] 
	}
   
	for {set y 1} {$y < $boardWidth} {incr y} {
	    set x $boardWidth
	    $c create oval [expr $x * $cellSize] [expr $y * $cellSize + $yOffset] \
		[expr ($x + 1) * $cellSize] [expr ($y + 1) * $cellSize + $yOffset] \
		-fill red3 -outline black -tags [list base goal] -stipple gray75 -width 3
	    $c create text  [expr (($x * $cellSize) + ($x + 1) * $cellSize)/2] \
		[expr ($y * $cellSize + ($y+1) *$cellSize)/2 + $yOffset] -text "GOAL" -justify center \
		-font GOALtext -tags [list base goaltext]
	}
    $c itemconfig goaltext -fill white

#Drawing the pieces and arrows
    for {set x 0} {$x < $boardWidth} {incr x} {
	for {set y 1} {$y < [expr $boardWidth+1]} {incr y} {
	    drawPiece $c [expr (($x * $cellSize)+ ($x + 1) * $cellSize)/2] [expr (($y * $cellSize) + ($y+1) *$cellSize)/2 + $yOffset] $cellSize blue [expr $x+((($boardWidth -1) - ($y-1)) * $boardWidth)]
	    drawPiece $c [expr (($x * $cellSize)+ ($x + 1) * $cellSize)/2] [expr (($y * $cellSize) + ($y+1) *$cellSize)/2 + $yOffset] $cellSize red [expr $x+((($boardWidth -1) - ($y-1)) * $boardWidth)]
	    drawArrows $c [expr (($x * $cellSize)+ ($x + 1) * $cellSize)/2] [expr (($y * $cellSize) + ($y+1) *$cellSize)/2 + $yOffset] $cellSize [expr $x+((($boardWidth -1) - ($y-1)) * $boardWidth)] 
	 
	}
    }  
    #remove extra squares, circles and arrows
    $c delete square0 
    $c delete red0
    $c delete blue0
    $c delete arrow0
    if {$boardWidth == 5} {
	$c delete square1
	$c delete square5
	$c delete red1
	$c delete red5
	$c delete blue1
	$c delete blue5
	$c delete arrow1
	$c delete arrow5
    }
    $c lower arrows
    $c lower pieces
   
    #draw the legend
    set x [expr $boardWidth * $cellSize]
    set y [expr 0 + $yOffset]
    drawLegend $c $x $y $cellSize

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
    
    global boardWidth boardSize numPieces 
    global amount cell mySize yOffset
    global numBars whosTurnIndex

    set pieceString [string range [C_GenericUnhash $position $boardSize] 0 [expr $boardSize-1]]
    $c raise base

    # raises appropriate pieces
    for {set i 0} {$i < [expr $boardWidth * $boardWidth]} {set i [expr $i + 1]} {
	if {[string compare [string index $pieceString $i] "x"] == 0} {
	    $c raise blue$i
	} elseif {[string compare [string index $pieceString $i] "o"] == 0} {
	    $c raise red$i
	} else {}
    }

    # does the win-by bar
    set numx 0
    set numo 0
    set xcount 0
    set ocount 0
    for {set row 0} {$row < $boardWidth} {incr row} {
	for {set col 0} {$col < $boardWidth} {incr col} {
	    set i [expr $boardWidth * $row + $col]
	    if {[string compare [string index $pieceString $i] "x"] == 0} {
		set xcount [expr $xcount + $row]
		set numx [expr $numx + 1]
	    }
	    if {[string compare [string index $pieceString $i] "o"] == 0} {
		set ocount [expr $ocount + $col]
		set numo [expr $numo + 1]
	    }
	}
    }
    set xcount [expr $xcount + ($boardWidth * ($numPieces - $numx))]
    set ocount [expr $ocount + ($boardWidth * ($numPieces - $numo))]

#     puts "--"
#     puts $xcount
#     puts $ocount

    set ocount [expr $xcount - $ocount + $numBars]

#     puts $ocount
#     puts "--"

#     if {$ocount == 0} { set ocount [expr $ocount + $amount] }
#     if {$amount < 0} { set amount 1 } else { set amount -1 }
#     set ocount [expr $ocount + $numPieces * $boardWidth]
    for {set i 0} {$i < [expr 2 * $numPieces * $boardWidth + 1]} {incr i} {
	if {$i < $ocount} {
	    $c itemconfig bar$i -fill [lindex [GS_ColorOfPlayers] 0]
	} elseif {$i > $ocount} {
	    $c itemconfig bar$i -fill [lindex [GS_ColorOfPlayers] 1]
	} else {
	    $c itemconfig bar$i -fill [lindex [GS_ColorOfPlayers] $whosTurnIndex]
	}
# 	if {$i < [expr $numPieces * $boardWidth]} {
# 	    set x [expr $i * $cell]
# 	    set y 0
# 	} elseif {$i > [expr $numPieces * $boardWidth]} {
# 	    set x $mySize
# 	    set y [expr $yOffset + ($i - ($numPieces * $boardWidth)) * $cell]
# 	}
# 	if {$i < [expr 2 * $numPieces * $boardWidth + 1 - $ocount]} {
# 	    $c itemconfig bar$i -fill [lindex [GS_ColorOfPlayers] 0]
# 	} else {
# 	    $c itemconfig bar$i -fill [lindex [GS_ColorOfPlayers] 1]
# 	}
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
    global whosTurnIndex
    set whosTurnIndex 0
    puts $whosTurnIndex
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
    global whosTurnIndex
    set whosTurnIndex [toggle $whosTurnIndex]
    puts $whosTurnIndex
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
		set color red4
	    }
	}
   
	set from [Movefrom $move]
	set to [Moveto $move]
	set player [whosturn $move]
	set direction [getDirection $from $to $player]
	
	if {$direction == 0} {
	    $c raise arrowRIGHT$from base
	    $c itemconfig arrowRIGHT$from -fill $color
	    $c bind arrowRIGHT$from <Enter> "$c itemconfig arrowRIGHT$from -fill black"
	    $c bind arrowRIGHT$from <Leave> "$c itemconfig arrowRIGHT$from -fill $color"
	    $c bind arrowRIGHT$from <ButtonRelease-1> "ReturnFromHumanMove $move"
	}
	if {$direction == 1} {
	    $c raise arrowDOWN$from base
	    $c itemconfig arrowDOWN$from -fill $color
	    $c bind arrowDOWN$from <Enter> "$c itemconfig arrowDOWN$from -fill black"
	    $c bind arrowDOWN$from <Leave> "$c itemconfig arrowDOWN$from -fill $color"
	    $c bind arrowDOWN$from <ButtonRelease-1> "ReturnFromHumanMove $move"
	}
	if {$direction == 2} {
	    $c raise arrowLEFT$from base
	    $c itemconfig arrowLEFT$from -fill $color
	    $c bind arrowLEFT$from <Enter> "$c itemconfig arrowLEFT$from -fill black"
	    $c bind arrowLEFT$from <Leave> "$c itemconfig arrowLEFT$from -fill $color"
	    $c bind arrowLEFT$from <ButtonRelease-1> "ReturnFromHumanMove $move"
	}
	if {$direction == 3} {
	    $c raise arrowUP$from base
	    $c itemconfig arrowUP$from -fill $color
	    $c bind arrowUP$from <Enter> "$c itemconfig arrowUP$from -fill black"
	    $c bind arrowUP$from <Leave> "$c itemconfig arrowUP$from -fill $color"
	    $c bind arrowUP$from <ButtonRelease-1> "ReturnFromHumanMove $move"
	}
    }
    update idletasks

}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    $c lower arrows base
    update idletasks

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
    global whosTurnIndex
    set whosTurnIndex [toggle $whosTurnIndex]
    puts $whosTurnIndex
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
    global gFrameWidth gFrameHeight
    global whosTurnIndex

    puts $whosTurnIndex
    $c create text [expr $gFrameWidth / 2] [expr $gFrameHeight/2] -width [expr $gFrameWidth * .9] \
	-font {Helvetica 64 bold} -fill [lindex [GS_ColorOfPlayers] [toggle $whosTurnIndex]] -text "$nameOfWinner wins!" -tag winText
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
    $c delete winText
}



#############################################################################
proc min { x y } {
    if {$x < $y} {
	return $x
    } else {
	return $y
    }
}

proc drawPiece {c x0 y0 cellsize color num} {
    set circlesize [expr $cellsize/3]
    $c create oval [expr $x0 - $circlesize] [expr $y0 - $circlesize] \
	[expr $x0 + $circlesize] [expr $y0 + $circlesize] \
	-fill $color -tags [list $color$num pieces] -outline black

}

proc drawArrows {c x y cellsize piecenum} {
    # -7 so that the arrows do not touch each other
    set arrowLen [expr $cellsize - 7] 
    set arrowWidth [expr $cellsize/7]
    #up
    $c create line $x $y $x [expr $y - $arrowLen] \
	-width $arrowWidth -arrow last -arrowshape [list [expr $arrowWidth * 2]  [expr $arrowWidth * 2] $arrowWidth] -fill cyan \
	-tags [list arrowUP$piecenum arrow$piecenum arrows]

    #down
    $c create line $x $y $x [expr $y + $arrowLen] \
    	-width $arrowWidth -arrow last -arrowshape [list [expr $arrowWidth * 2]  [expr $arrowWidth * 2] $arrowWidth] -fill cyan \
	-tags [list arrowDOWN$piecenum arrow$piecenum arrows]

    #left
    $c create line $x $y [expr $x - $arrowLen] $y \
	-width $arrowWidth -arrow last -arrowshape [list [expr $arrowWidth * 2]  [expr $arrowWidth * 2] $arrowWidth] -fill cyan \
	-tags [list arrowLEFT$piecenum arrow$piecenum arrows]

    #right
    $c create line $x $y [expr $x + $arrowLen] $y \
	-width $arrowWidth -arrow last -arrowshape [list [expr $arrowWidth * 2]  [expr $arrowWidth * 2] $arrowWidth] -fill cyan \
	-tags [list arrowRIGHT$piecenum arrow$piecenum arrows]
   

}

proc drawLegend {c x y cellsize} {
    set legendcellsize [expr $cellsize/3]
    set legendArrowWidth [expr $legendcellsize/7]
    set legendArrowLen [expr $legendcellsize - 3]
    set bluex0 [expr $x + (($legendcellsize + 2*$legendcellsize)/2)]
    set bluey0 [expr $y + ((2*$legendcellsize + 3*$legendcellsize)/2)]
    set redx0 [expr $x + ($legendcellsize/2)]
    set redy0  [expr $y + (($legendcellsize + 2*$legendcellsize)/2)]
    #border
    $c create rectangle $x $y [expr $x + $cellsize] [expr $y + $cellsize] -width 3 -fill gray -tags [list legend base]
    #legend text
    font configure LEGENDtext -size [expr round($legendcellsize/3)] -weight bold
    $c create text  [expr $x + $legendcellsize + $legendcellsize/1.5]  [expr $y + $legendcellsize/2] \
	-text "LEGEND" -justify center -font LEGENDtext -tags [list legend base]
    #red arrows
    #up
      $c create line $redx0 $redy0 $redx0 [expr $redy0 - $legendArrowLen] \
	-width $legendArrowWidth -arrow last -arrowshape [list [expr $legendArrowWidth * 2]  [expr $legendArrowWidth * 2] $legendArrowWidth] -fill cyan -tags [list legend base]
    #down
      $c create line $redx0 $redy0 $redx0 [expr $redy0 + $legendArrowLen] \
	-width $legendArrowWidth -arrow last -arrowshape [list [expr $legendArrowWidth * 2]  [expr $legendArrowWidth * 2] $legendArrowWidth] -fill cyan -tags [list legend base]
    #right
    $c create line $redx0 $redy0 [expr $redx0 + $legendArrowLen] $redy0 \
	-width $legendArrowWidth -arrow last -arrowshape [list [expr $legendArrowWidth * 2]  [expr $legendArrowWidth * 2] $legendArrowWidth] -fill cyan -tags [list legend base]
    #blue arrows
    #up
     $c create line $bluex0 $bluey0 $bluex0 [expr $bluey0 - $legendArrowLen] \
	-width $legendArrowWidth -arrow last -arrowshape [list [expr $legendArrowWidth * 2]  [expr $legendArrowWidth * 2] $legendArrowWidth] -fill cyan  -tags [list legend base]
    
    #left
    $c create line $bluex0 $bluey0 [expr $bluex0 - $legendArrowLen] $bluey0 \
	-width $legendArrowWidth -arrow last -arrowshape [list [expr $legendArrowWidth * 2]  [expr $legendArrowWidth * 2] $legendArrowWidth] -fill cyan  -tags [list legend base]
    #right
    $c create line $bluex0 $bluey0 [expr $bluex0 + $legendArrowLen] $bluey0 \
	-width $legendArrowWidth -arrow last -arrowshape [list [expr $legendArrowWidth * 2]  [expr $legendArrowWidth * 2] $legendArrowWidth] -fill cyan -tags [list legend base]
      
    #pieces
    set legendcirclesize [expr $legendcellsize/3]
    $c create oval [expr $bluex0 - $legendcirclesize] [expr $bluey0 - $legendcirclesize] \
	[expr $bluex0 + $legendcirclesize] [expr $bluey0 + $legendcirclesize] \
	-fill blue -tags [list legend base] -outline black -width 3
    $c create oval [expr $redx0 - $legendcirclesize] [expr $redy0 - $legendcirclesize] \
	[expr $redx0 + $legendcirclesize] [expr $redy0 + $legendcirclesize] \
	-fill red -tags [list legend base] -outline black -width 3
}

#Move Stuff
#East: 0
#South: 1
#West: 2
#North: 3
proc getDirection {from to whosturn} {
    global boardSize boardWidth
    if {$to == $boardSize} {
	if {[expr ($from % $boardWidth)] == [expr ($boardWidth -1)]} {
	    if {[string compare $whosturn "1"] == 0} { 
		return 0;
	    }
	}
	if {[expr ($from / $boardWidth)] == [expr ($boardWidth-1)]} {
	    if {[string compare $whosturn "0"] == 0} {
		return 3;
	    }
	} else {}
    } 
    if {$to == [expr $from+1]} {
	return 0;
    }
    if {$to == [expr $from-1]} {
	return 2;
    }
    if {$to == [expr $from+$boardWidth]} {
	return 3;
    }
    if {$to == [expr $from-$boardWidth]} {
	return 1;
    }
}

# 255 is 8 1s
proc Moveto {theMove} {
    return [expr ($theMove >> 8) & 255]
}

proc Movefrom {theMove} {
    return [expr ($theMove >> 16) & 255]
}

# 1 represents o
# 0 represents x
proc whosturn {theMove} {
    return [expr $theMove & 255]
}

#Creating fonts for text
font create GOALtext -family arial
font create LEGENDtext -family Times

proc toggle { n } {
    if { $n == 0 } {
	return 1
    } elseif { $n == 1 } {
	return 0
    } else {
	BadElse "toggle: n != 0/1"
    }
}

