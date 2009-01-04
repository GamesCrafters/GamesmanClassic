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
    set kGameName "ShiftTacToe"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 767638
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to get 3-in-a-row WINS"
    set kMisereString "First player to get 3-in-a-row LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
#	SetToWinString "To Win: Be the first player to get three of your pieces\nin a row (horizontally, vertically or diagonally)."
	SetToWinString [concat "To Win: " [C_GetStandardObjString]]
    } else {
	#SetToWinString "To Win: Force your opponent into getting three of his pieces\nin a row (horizontally, vertically or diagonally) first."
	SetToWinString [concat "To Win: " [C_GetReverseObjString]]

    }
    SetToMoveString "To Move: Shift Tac Toe is similar to Tic Tac Toe, except it has the extra option of shifting one of the rows on your turn. Players drop a piece into the slot by clicking the arrow that corresponds to where they would like to move. To shift the board, select the arrow on the sides that correspond to row that the player wants to shift."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Ling Xiao"
    set kTclAuthors "Ling Xiao"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"


    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 9
    set gSlotsY 6
    ### Set animation basespeed 
    global basespeed
    set basespeed 70
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

     set flatRule \
	[list \
	     "How would you like your board to be oriented:" \
	     "Vertically" \
	     "Horizontally" \
	    ]

     set wrapAroundRule \
	[list \
	     "Would you like your pieces to wrap around:" \
	     "No" \
	     "Yes" \
	    ]
    # List of all rules, in some order
    set ruleset [list $standardRule $flatRule $wrapAroundRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gFlatRule
    set gFlatRule 0

    global gWrapAroundRule
    set gWrapAroundRule 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gFlatRule" "gWrapAroundRule"]

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
    global gMisereGame gFlatRule gWrapAroundRule

    if {$gMisereGame} {
	set option 1
    } else { 
	set option 2
    }
    if ($gWrapAroundRule) {
	set option [expr $option + (1*2*2)]
    }
    if ($gFlatRule) {
	set option [expr $option + (2*2*2*2)]
    }
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
    global gMisereGame gFlatRule gWrapAroundRule
    set option [expr $option-1]
    set standard [expr $option%2]
    if ($standard) {
	set gMisereGame 0
    } else {
	set gMisereGame 1
    }
    set gWrapAroundRule [expr $option/(2*2)%2]
    set flattest [expr $option/(2*2*2)%4]
    if ($flattest>=2) {
	set gFlatRule 1
    } else {
	set gFlatRule 0
    }
    
    
    #set gMisereGame [expr 1-($option%2)]
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
  #  $c configure -width 500 -height 500
    
 ### Set the size of the board

    global gSlotsX gSlotsY gSlotSize 
    global gFrameWidth gFrameHeight
    set mySize [Min $gFrameWidth $gFrameHeight]
    set boardMax [Max $gSlotsX $gSlotsY]
    set cellSize [expr $mySize / $boardMax]
    set gSlotSize $cellSize

    $c create rectangle [expr 2*$gSlotSize] [expr 1*$gSlotSize]  [expr 7*$gSlotSize] [expr 2*$gSlotSize]  -fill blue -outline blue -tag topbase
    $c create rectangle [expr 2*$gSlotSize] [expr 5*$gSlotSize]  [expr 7*$gSlotSize] [expr 6*$gSlotSize]  -fill blue -outline blue
    
   $c create rectangle 0 0 [expr 9*$gSlotSize] [expr 1*$gSlotSize]  -fill grey -outline grey
    $c create rectangle 0 0 [expr 2*$gSlotSize] [expr 6*$gSlotSize]  -fill grey -outline grey
   $c create rectangle [expr 7*$gSlotSize] [expr 1*$gSlotSize]  [expr 9*$gSlotSize] [expr 6*$gSlotSize]  -fill grey -outline grey

#draw the board
 for {set slotX 3} {$slotX < 6} {incr slotX} {
	for {set slotY 4} {$slotY > 1} {incr slotY -1} {
	    set board [ClearSlot $c $slotX $slotY "black"]
	    $c addtag base withtag $board
	}
 }


    DrawSlider $c 0 1
    DrawSlider $c 1 1
    DrawSlider $c 2 1


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


 ##I used theSlot to hold the configuration after the move
    set theSlot $position
    set boardPos [expr ($theSlot - ($theSlot % 3))/3]

    for {set x 3} {$x < 6} {incr x} {
	for {set y 4} {$y > 1} {incr y -1} {
	    
	    $c delete tagPieceOnCoord$x$y

	    set choice [expr ($boardPos%3)]
	    
		switch $choice {
		    1 { set temppiece [DrawPiece $c $x $y "X"] }
		    2 { set temppiece [DrawPiece $c $x $y "O"] }
		    0 {}
		    default { puts "ERROR" }
		}
	    if ($choice>0) {
		$c addtag PieceRow$y withtag $temppiece
	    }
	    set boardPos [expr ($boardPos - $choice)/3]
	}
    }
    
    for {set x 0} {$x < 3} {incr x} {
	set choice [expr ($boardPos%3)]
	DrawSlider $c $x $choice
	set boardPos [expr ($boardPos - $choice)/3]
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

	### TODO: Fill this in
    global gSlotsX gSlotSize
    global gFlatRule
    #Regular board #theMove#   FlatBoard
    #  0 1 2            
    #3(     ) 6                   9  (2 5 8) 12
    #4(     ) 7                   10 (1 4 7) 13
    #5(     ) 8                   11 (0 3 6) 14
    
    if ($gFlatRule) {
	if ($theMove<=8) { 
	} else {
	    set m [ConvertFlatToRegMove $theMove]
	    set theMoveArgAbsolute [ConvertToAbsoluteMove $m]
	    set theMoveBegin [lindex $theMoveArgAbsolute 0]
	    set slotYbegin [expr $theMoveBegin / $gSlotsX]
	    if ($theMove<=11) {
		animatemoveonesquare $c PieceRow$slotYbegin 1
		
	    } else {
		animatemoveonesquare $c PieceRow$slotYbegin 0
		
	    }
	}  
	GS_DrawPosition $c $newPosition
	
    } else {
	set theMoveArgAbsolute [ConvertToAbsoluteMove $theMove]
	set theMoveBegin [lindex $theMoveArgAbsolute 0]
	set slotXbegin [expr $theMoveBegin % $gSlotsX]
	set slotYbegin [expr $theMoveBegin / $gSlotsX]
	
	if {$slotYbegin == 0 } {
	    
	    set theSlot $newPosition
	    set boardPos [expr ($theSlot - ($theSlot % 3))/3]
	    for {set x 3} {$x < 6} {incr x} {
		for {set y 4} {$y > 1} {incr y -1} {
		    
		    set choice [expr ($boardPos%3)]
		    if {$choice == 1 && $x == $slotXbegin} {
			set slotYend $y
			set piece "X"
		    } elseif {$choice == 2 && $x == $slotXbegin} {
			set slotYend $y
			set piece "O"
		    } elseif {$choice == 0 && $x == $slotXbegin} {
		    }
		    
		    set boardPos [expr ($boardPos - $choice)/3]
		}
	    }
	    set slotXend $slotXbegin
	    set cornerXbegin [expr $slotXbegin   * $gSlotSize]
	    set cornerYbegin [expr $slotYbegin   * $gSlotSize]
	    set cornerXend [expr $slotXend   * $gSlotSize]
	    set cornerYend  [expr $slotYend   * $gSlotSize]
	    
	    set thePiece [DrawPiece $c $slotXbegin $slotYbegin $piece]
	    $c dtag $thePiece tagPieceOnCoord$slotXbegin$slotYbegin 
	    $c addtag tagPieceOnCoord$slotXend$slotYend withtag $thePiece
	    $c addtag PieceRow$slotYend withtag $thePiece
	    animatedrop $c $thePiece [list $cornerXbegin $cornerYbegin] \
		[list $cornerXend $cornerYend]

	} else {
	    if ($slotXbegin==0) {
		animatemoveonesquare $c PieceRow$slotYbegin 1
	    } elseif ($slotXbegin==8) {
		animatemoveonesquare $c PieceRow$slotYbegin 0
	    }
	    GS_DrawPosition $c $newPosition    
	}
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

    ### TODO: Fill this in
    global gSlotSize gSlotsX gSlotsY 
    global gFlatRule
    global tk_library kRootDir gStippleRootDir
    set gStippleRootDir "$kRootDir/../bitmaps/"
    set stipple12 @[file join $gStippleRootDir gray12.bmp] 
	set stipple25 @[file join $gStippleRootDir gray25.bmp]
	set stipple50 @[file join $gStippleRootDir gray50.bmp]
	set stipple75 @[file join $gStippleRootDir gray75.bmp]
    set stipple ""
    #Regular board    #theMove#     FlatBoard
    #  0 1 2            
    #3(     ) 6                   9  (2 5 8) 12
    #4(     ) 7                   10 (1 4 7) 13
    #5(     ) 8                   11 (0 3 6) 14
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set remoteness [lindex $item 2]
	set delta [lindex $item 3]
    set delta [expr $delta/2]
	set color cyan

	 if {$moveType == "value" || $moveType == "rm"} {
		if {$value == "Tie"} {
		    set color yellow
		} elseif {$value == "Lose"} {
		    set color green
		} else {
		    set color red4
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
	if ($gFlatRule&&$move<=8) {
	    #do stuff
	    set XY [ConvertMoveToXY $move]
	    set movetemp [CreateOvalMove $c $XY $color $stipple]
	    $c bind $movetemp <Enter> "$c itemconfig $movetemp -fill black"
	    $c bind $movetemp <Leave> "$c itemconfig $movetemp -fill $color"
	    $c bind $movetemp <ButtonRelease-1> "ReturnFromHumanMove $move"
	} else {
	    if ($gFlatRule) {
		set oldmove [ConvertFlatToRegMove $move]
		set theMoveArgAbsolute [ConvertToAbsoluteMove $oldmove]
	    } else {
		set theMoveArgAbsolute [ConvertToAbsoluteMove $move]
	    }
	    set movetemp [CreateArrow $c $theMoveArgAbsolute $gSlotSize $gSlotsX $gSlotsY $color $stipple]
	    $c bind $movetemp <Enter> "$c itemconfig $movetemp -fill black"
	    $c bind $movetemp <Leave> "$c itemconfig $movetemp -fill $color"
	    $c bind $movetemp <ButtonRelease-1> "ReturnFromHumanMove $move"
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

    ### TODO: Fill this in
    $c delete arrows
    $c delete dots

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

    ### TODO if needed
    global gFlatRule gSlotsX gSlotSize
    if (!$gFlatRule) {
	set theMoveArgAbsolute [ConvertToAbsoluteMove $theMoveToUndo]
	set theMoveBegin [lindex $theMoveArgAbsolute 0]
	set slotXbegin [expr $theMoveBegin % $gSlotsX]
	set slotYbegin [expr $theMoveBegin / $gSlotsX]
	
	if {$slotYbegin == 0 } {
	    
	    set theSlot $currentPosition
	    set boardPos [expr ($theSlot - ($theSlot % 3))/3]
	    for {set x 3} {$x < 6} {incr x} {
		for {set y 4} {$y > 1} {incr y -1} {
		    
		    set choice [expr ($boardPos%3)]
		    if {$choice == 1 && $x == $slotXbegin} {
			set slotYend $y
			set piece "X"
		    } elseif {$choice == 2 && $x == $slotXbegin} {
			set slotYend $y
			set piece "O"
		    } elseif {$choice == 0 && $x == $slotXbegin} {
		    }
		    
		    set boardPos [expr ($boardPos - $choice)/3]
		}
	    }
	    set slotXend $slotXbegin
	    set cornerXbegin [expr $slotXbegin   * $gSlotSize]
	    set cornerYbegin [expr $slotYbegin   * $gSlotSize]
	    set cornerXend [expr $slotXend   * $gSlotSize]
	    set cornerYend  [expr $slotYend   * $gSlotSize]
	    
	    set thePiece tagPieceOnCoord$slotXend$slotYend
	    animatedrop $c $thePiece \
		[list $cornerXend $cornerYend] [list $cornerXbegin $cornerYbegin]
	    $c delete tagPieceOnCoord$slotXend$slotYend
	} else {
	    GS_DrawPosition $c $positionAfterUndo
	}
    } else {
	GS_DrawPosition $c $positionAfterUndo
    }
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



#Helper Functions
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
## ClearSlot
##
## Game specific function that draws a grey slot
##
#############################################################################


proc ClearSlot { w slotX slotY theColor} {
    global gSlotSize
    
    set upLeftX [expr ($slotX * $gSlotSize)]
    set upLeftY [expr ($slotY * $gSlotSize)]
    set lowRightX [expr ($upLeftX + $gSlotSize)]
    set lowRightY [expr ($upLeftY + $gSlotSize)]


    return [$w create rectangle $upLeftX $upLeftY $lowRightX $lowRightY -fill grey -outline $theColor]


}


#############################################################################
##
## DrawSliderSquare
##
## Game specific function that draws one square of the slider bar
##
#############################################################################

proc DrawSliderSquare { w slotX slotY } {
    global gSlotSize
    
    set upLeftX [expr ($slotX * $gSlotSize)]
    set upLeftY [expr ($slotY * $gSlotSize)]
    set lowRightX [expr ($upLeftX + $gSlotSize)]
    set lowRightY [expr ($upLeftY + $gSlotSize)]

    #puts "$upLeftX, $upLeftY, $lowRightX, $lowRightY"

    $w create rectangle $upLeftX $upLeftY $lowRightX $lowRightY -fill white -outline black -tag sliders
   
    set upLeftX [expr (($slotX * $gSlotSize) + 10)]
    set upLeftY [expr (($slotY * $gSlotSize) + 10)]
    set lowRightX [expr (($upLeftX + $gSlotSize)-20)]
    set lowRightY [expr (($upLeftY + $gSlotSize)-20)]
    
    #puts "$upLeftX, $upLeftY, $lowRightX, $lowRightY"

    $w create oval $upLeftX $upLeftY $lowRightX $lowRightY -fill grey -outline grey -tag sliders

    #puts "Just painted rectangle"
}

#############################################################################
##
## DrawSlider
##
## Game specific instruction that draws the shift tac toe sliders
##
#############################################################################

proc DrawSlider { w row position } {

    set rowY [expr $row + 2]

    ClearSlot $w 1 $rowY "black"
    ClearSlot $w 2 $rowY "black"
    ClearSlot $w 6 $rowY "black"
    ClearSlot $w 7 $rowY "black"

   switch $position {
       0 {  DrawSliderSquare $w 1 $rowY 
	    DrawSliderSquare $w 2 $rowY }
	1 { DrawSliderSquare $w 2 $rowY 
	    DrawSliderSquare $w 6 $rowY }
	2 { DrawSliderSquare $w 6 $rowY 
	    DrawSliderSquare $w 7 $rowY }

	}

}






#############################################################################
##
## CreateArrow
##
## Create and return arrow at the specified slots.
##
#############################################################################

proc CreateArrow { w theMoveArgAbsolute theSlotSize theSlotsX theSlotsY color stipple } {

    global gSlotsX

    ### The gap from the line to the edge
    
    set theLineGap    [expr int($theSlotSize / 8)]
    
    ### The actual length of the line is the width of two slots
    ### minus the two gaps at the end.
    
    set theLineLength [expr $theSlotSize - (2 * $theLineGap)]
    
    set theMoveBegin [lindex $theMoveArgAbsolute 0]
    set theMoveEnd   [lindex $theMoveArgAbsolute 1]
    
    set isitHorizontal [expr abs($theMoveEnd - $theMoveBegin) == 1]
    set isitRightDown  [expr $theMoveEnd > $theMoveBegin]
    set isitDiagonal   [not [expr abs($theMoveEnd - $theMoveBegin) == $gSlotsX]]
    
    set theOffsetCenter [expr $isitRightDown ? 1 : -1]
    set intSlotSize [expr int($theSlotSize)]
    set theHalfSlot     [expr $intSlotSize >> 1]
    
    set slotXbegin [expr $theMoveBegin % $theSlotsX]
    set slotYbegin [expr $theMoveBegin / $theSlotsX]


    set theLineOffset 3
    
    if { $isitHorizontal } {
	set theLineX [expr $theLineLength * $theOffsetCenter]
	set theLineY 0
	set theLineOffsetX [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
	set theLineOffsetY $theHalfSlot
    } elseif { [expr 0 && $isitDiagonal] } {
	set theLineX [expr ($theLineLength + $theLineGap) * $theOffsetCenter]
	set theLineY $theLineX
	set theLineOffsetX [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
	set theLineOffsetY [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
    } else {
	set theLineX 0
	set theLineY [expr $theLineLength * $theOffsetCenter]
	set theLineOffsetX $theHalfSlot
	set theLineOffsetY [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
    }   
    
    ### Create the line
    
    set arrow1 [expr $theLineGap * 2]
    set arrow2 [expr $theLineGap * 2]
    set arrow3 [expr $theLineGap * 1]
    set theMove [$w create line 0 0 $theLineX $theLineY \
		     -width $theLineGap \
		     -arrow last \
		     -arrowshape [list $arrow1 $arrow2 $arrow3] \
		     -fill $color \
		     -tag arrows \
             -stipple $stipple]

    
    set cornerX [expr $slotXbegin   * $theSlotSize + $theLineOffsetX]
    set cornerY [expr $slotYbegin   * $theSlotSize + $theLineOffsetY]
    
    $w move $theMove $cornerX $cornerY
    
    return $theMove
}

#Only used for flat board
proc CreateOvalMove { c XY color stipple} {
    global gSlotSize
    
    set slotX [lindex $XY 0]
    set slotY [lindex $XY 1]
    
    set upLeftX [expr (($slotX * $gSlotSize) + 20)]
    set upLeftY [expr (($slotY * $gSlotSize)+ 20)]
    set lowRightX [expr (($upLeftX + $gSlotSize)-40)]
    set lowRightY [expr (($upLeftY + $gSlotSize)-40)]

    $c create oval $upLeftX $upLeftY $lowRightX $lowRightY -fill $color -outline grey -tag dots -stipple $stipple
}

#only used for flat board
proc ConvertMoveToXY { theMove } {
    switch $theMove {
	    0 { return [list 3 4] }
	    1 { return [list 3 3] }
	    2 { return [list 3 2] }
	    3 { return [list 4 4] }
	    4 { return [list 4 3] }
	    5 { return [list 4 2] }
	    6 { return [list 5 4] }
	    7 { return [list 5 3] }
	    8 { return [list 5 2] }
	    default { return [list 0 0]}
	}
}


proc ConvertToAbsoluteMove { theMove } {
    

    #puts "GS_ConvertToAbsoluteMove"

    ### Unfortunately shift tac toe does not have absolute moves

    switch $theMove {
	    0 { return [list 3 12] }
	    1 { return [list 4 13] }
	    2 { return [list 5 14] }
	    3 { return [list 18 19] }
	    4 { return [list 27 28] }
	    5 { return [list 36 37] }
	    6 { return [list 26 25] }
	    7 { return [list 35 34] }
	    8 { return [list 44 43] }
	    default { return [list 0 0]}
	}
   
}

proc ConvertFlatToRegMove { theMove } {
    #Regular board    #theMove#     FlatBoard
    #  0 1 2            
    #3(     ) 6                   9  (2 5 8) 12
    #4(     ) 7                   10 (1 4 7) 13
    #5(     ) 8                   11 (0 3 6) 14

    switch $theMove {
	    9 { return 3 }
	    10 { return 4 }
	    11 { return 5 }
	    12 { return 6 }
	    13 { return 7 }
	    14 { return 8 }
	default { return $theMove}
	}
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



#############################################################################
##
## DrawPiece
##
## Draw a piece at the slot specified by slotX and slotY
##
#############################################################################

proc DrawPiece { c slotX slotY thePiece } {
    if { $thePiece == "X" } {
	return [DrawCross  $c $slotX $slotY tagPiece blue 1]
    } elseif { $thePiece == "O" } { 
	return [DrawHollowCircle $c $slotX $slotY tagPiece red 1]
    } else {
	BadElse DrawPiece
    }
}



#############################################################################
##
## DrawCross
##
## Here we draw an X on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawCross { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize $gSlotSize

    set startCross [expr $theSlotSize/8.0]
    set halfCross  [expr $theSlotSize/2.0]
    set endCross   [expr $startCross*7.0]

    set cornerX    [expr $slotX*$theSlotSize]
    set cornerY    [expr $slotY*$theSlotSize]
    set thickness  [expr $theSlotSize/10.0]

    set x1 $startCross
    set x2 [expr $startCross + $thickness]
    set x3 [expr $halfCross - $thickness]
    set x4 $halfCross
    set x5 [expr $halfCross + $thickness]
    set x6 [expr $endCross - $thickness]
    set x7 $endCross

    set y1 $startCross
    set y2 [expr $startCross + $thickness]
    set y3 [expr $halfCross - $thickness]
    set y4 $halfCross
    set y5 [expr $halfCross + $thickness]
    set y6 [expr $endCross - $thickness]
    set y7 $endCross

    set theCross [$w create polygon \
		   $x2 $y1 $x4 $y3 $x6 $y1 $x7 $y2 $x5 $y4 \
		   $x7 $y6 $x6 $y7 \
		   $x4 $y5 $x2 $y7 $x1 $y6 $x3 $y4 $x1 $y2 \
		   -fill $theColor \
		   -tag $theTag]

    if { $drawBig } {
	$w move $theCross $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCross
    return $theCross
}


#############################################################################
##
## DrawHollowCircle
##
## Here we draw a hollow circle on (slotX,slotY) in window w with a tag of 
## theTag. If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawHollowCircle { w slotX slotY theTag theColor drawBig } {
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
		       -width $circleWidth \
		       -tag $theTag]

    if { $drawBig } {
	$w move $theCircle $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle

    return $theCircle
}





proc animatedrop { c piece origin destination } {


    global basespeed gAnimationSpeed
    
 #   $c raise board
    $c raise topbase
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

proc animatemoveonesquare { c piece direction } {
    #0: left 
    #1: right
    #2: down
    #3: up
    global basespeed gAnimationSpeed gSlotSize
    $c raise sliders
    set speed [expr $basespeed / pow(2, $gAnimationSpeed)]
    
    # If things get too fast, just make it instant
    if {$speed < 10} {
	set speed 10
    }
    if {$direction==0} {
	set dx [expr (0 - $gSlotSize) / $speed]
	set dy 0
    } elseif {$direction==1} {
	set dx [expr $gSlotSize / $speed]
	set dy 0
    } elseif {$direction==2} {
	set dx 0
	set dy [expr $gSlotSize / $speed]
    } elseif {$direction==3} {
	set dx 0
	set dy [expr (0-$gSlotSize) / $speed]
    }
    
    

    for {set i 0} {$i < $speed} {incr i} {
	$c move $piece $dx $dy
	
	after 1
	update idletasks
    }
}