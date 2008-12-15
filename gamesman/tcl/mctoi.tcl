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
    set kGameName "Chung-Toi"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to get 3-in-a-row WINS"
    set kMisereString "First player to get 3-in-a-row LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Be the first player to get three of your pieces\nin a row (horizontally, vertically or diagonally)"
    } else {
	SetToWinString "To Win: Force your opponent into getting three of his pieces\nin a row (horizontally, vertically or diagonally) first"
    }
    SetToMoveString "To Move: The players begin by placing their game pieces on the board until all the pieces are played. Then the players continue by moving their pieces to other open spaces on the board indicated by arrows or a piece can just rotate in place to pass."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Farzad Eskafi and Erwin Vedar"
    set kTclAuthors "Farzad Eskafi and Erwin Vedar"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"


    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 80

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 3
    set gSlotsY 3

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/XPlus.xbm"
    set xbmRight "$kRootDir/../bitmaps/XPlus.xbm"
    set macLeft  X_cursor
    set macRight circle
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

    set spininplaceRule \
	[list \
	     "Would you like to allow rotation in place?" \
	     "No" \
	     "Yes" \
	    ]

    set onespacejumpRule \
	[list \
	     "Would you like to allow single-space jumps?" \
	     "No" \
	     "Yes"\
	    ]

    set twospacejumpRule \
	[list \
	     "Would you like to allow two-space jumps?" \
	     "No" \
	     "Yes" \
	    ]
    
    set difforientationRule \
	[list \
	     "Would you like to allow rotation upon landing?" \
	     "No" \
	     "Yes" \
	    ]

    set trappedplayerRule \
	[list \
	     "Does the trapped player win?" \
	     "No" \
	     "Yes" \
	    ]

    


    # List of all rules, in some order
    set ruleset [list $standardRule $spininplaceRule $onespacejumpRule $twospacejumpRule $difforientationRule $trappedplayerRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gRotateInPlaceOp
    set gRotateInPlaceOp 1

    global gHopOneOp
    set gHopOneOp 1

    global gHopTwoOp
    set gHopTwoOp 1

    global gRotateOnHopOp
    set gRotateOnHopOp 1

    global gStuckAWinOp
    set gStuckAWinOp 0
    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gRotateInPlaceOp" "gHopOneOp" "gHopTwoOp" "gRotateOnHopOp" "gStuckAWinOp"]

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
    global gMisereGame gRotateInPlaceOp gHopOneOp gHopTwoOp
    global gRotateOnHopOp gStuckAWinOp
    set option $gRotateInPlaceOp
    set option [expr $option << 1]
    set option [expr $option + $gHopOneOp]
    set option [expr $option << 1]
    set option [expr $option + $gHopTwoOp]
    set option [expr $option << 1]
    set option [expr $option + $gRotateOnHopOp]
    set option [expr $option << 1]
    set option [expr $option + $gStuckAWinOp]
    set option [expr $option << 1]
    if {$gMisereGame} {
	set option2 0
    } else { 
	set option2 1
    }
    set option [expr $option + $option2]
    set option [expr $option + 1]
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
    global gMisereGame gStuckAWinOp gRotateOnHopOp
    global gHopTwoOp gHopOneOp gRotateInPlaceOp
    set op [expr $option - 1]
    set temp [expr $op % 2]
    if {$temp} {set gMisereGame 0} {set gMisereGame 1}
    set op [expr $op/2]
    set gStuckAWinOp [expr $op % 2]
    set op [expr $op/2]
    set gRotateOnHopOp [expr $op % 2]
    set op [expr $op/2]
    set gHopTwoOp [expr $op % 2]
    set op [expr $op/2]
    set gHopOneOp [expr $op % 2] 
    set op [expr $op/2]
    set gRotateInPlaceOp [expr $op % 2]
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
    global gSlotsX gSlotsY gSlotSize
    global gFrameWidth gFrameHeight
    set mySize [Min $gFrameWidth $gFrameHeight]
    set boardMax [Max $gSlotsX $gSlotsY]
    set cellSize [expr $mySize / $boardMax]
    set gSlotSize $cellSize

    $c delete all; # Clear all objects

    #draw the board and tag it base
    for {set x 0} {$x < $gSlotsX} {incr x} {
	for {set y 0} {$y < $gSlotsY} {incr y} {
	    $c create rectangle [expr $x * $cellSize] [expr $y * $cellSize] [expr ($x + 1) * $cellSize] [expr ($y + 1) * $cellSize] -fill grey -outline black -tag base 
	}
    }
    
    #draw all the pieces and moves for placement phase 
    for {set x 0} {$x < $gSlotsX} {incr x} {
	for {set y 0} {$y < $gSlotsY} {incr y} {
	    DrawCross $c $x $y bluecross$x$y blue "" 1
	    DrawCross $c $x $y redcross$x$y red "" 1
	    DrawPlus $c $x $y blueplus$x$y blue "" 1
	    DrawPlus $c $x $y redplus$x$y red "" 1

	    DrawCross $c $x $y movecross$x$y cyan "" 1
	    DrawPlus $c $x $y moveplus$x$y cyan "" 1
	    $c addtag moves withtag movecross$x$y 
	    $c addtag moves withtag moveplus$x$y 
	}
    }
    #hide all the pieces
    $c raise base

    # Create welcome sign.
    set MinFrameLength [Min [expr $gFrameWidth - 1] [expr $gFrameHeight - 1]]
    $c create rectangle 0 [expr $gFrameWidth/2 - 80] $MinFrameLength [expr $gFrameWidth/2 + 80] -fill white -width 1 -outline black -tag welcome
    $c create text [expr $gFrameWidth/2] [expr $gFrameWidth/2] -text "Welcome to Chung Toi!" -font "Arial 50" -anchor center -fill black -width $gFrameWidth -justify center -tag welcome
    # Move welcome sign to the center of the frame.
    $c move welcome [expr ($gFrameWidth - 1 - $MinFrameLength) / 2] [expr ($gFrameHeight - 1 - $MinFrameLength) / 2]
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
    global gSlotsX gSlotsY gSlotSize
    $c raise base all
    set board [C_CustomUnhash $position]
    #puts $board
    
    set i 0
    for {set row 0} {$row < $gSlotsY} {incr row} {
	for {set col 0} {$col < $gSlotsX} {incr col} {
	    if {[string compare [string index $board $i] "-"] == 0} {
	    } elseif {[string compare [string index $board $i] "X"] == 0} {
		$c raise bluecross$col$row base
	    } elseif {[string compare [string index $board $i] "x"] == 0} {
		$c raise redcross$col$row base
	    } elseif {[string compare [string index $board $i] "T"] == 0} {
		$c raise blueplus$col$row base
	    } elseif {[string compare [string index $board $i] "t"] == 0} {
		$c raise redplus$col$row base
	    } else {}
	    incr i
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
    global gPlayerOneTurn
    GS_Deinitialize $c
    GS_Initialize $c
    $c delete welcome
    set gPlayerOneTurn 1
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

    global gSlotsX gSlotsY
    global gPlayerOneTurn 
    set theAbsoluteMoveArg [ConvertToAbsoluteMove $theMove]

    set from [lindex $theAbsoluteMoveArg 0]
    set to   [lindex $theAbsoluteMoveArg 1]
    set orientation [lindex $theAbsoluteMoveArg 2]
    
    set fromSlotX [expr $from % $gSlotsX]
    set fromSlotY [expr $from / $gSlotsX]
    set toSlotX   [expr $to % $gSlotsX]
    set toSlotY   [expr $to / $gSlotsX]

    if { $from == 9 } {
	#ChungToiPlacing
	if {$orientation == 1} {
	    if {$gPlayerOneTurn == 1} \
		{$c raise blueplus$toSlotX$toSlotY base} \
		{$c raise redplus$toSlotX$toSlotY base}
	} elseif {$orientation != 1} {
	    if {$gPlayerOneTurn == 1} \
		{$c raise bluecross$toSlotX$toSlotY base}\
		{$c raise redcross$toSlotX$toSlotY base}
	}
    } elseif { $from == $to} {
	#ChungToiTwisting
	if {$orientation == 1} {
	    if {$gPlayerOneTurn == 1} {
		$c raise blueplus$toSlotX$toSlotY base
		$c lower bluecross$toSlotX$toSlotY base
	    } else {
		$c raise redplus$toSlotX$toSlotY base
		$c lower redcross$toSlotX$toSlotY base
	    }

	} elseif {$orientation != 1} {
	    if {$gPlayerOneTurn == 1} {
		$c raise bluecross$toSlotX$toSlotY base
		$c lower blueplus$toSlotX$toSlotY base
	    } else {
		$c raise redcross$toSlotX$toSlotY base
		$c lower redplus$toSlotX$toSlotY base
	    }
	}
    } elseif {$from != $to} {
	#need to do animation
	$c lower blueplus$fromSlotX$fromSlotY base
	$c lower bluecross$fromSlotX$fromSlotY base
	$c lower redplus$fromSlotX$fromSlotY base
	$c lower redcross$fromSlotX$fromSlotY base
	if {$gPlayerOneTurn == 1} {set pcolor blue} {set pcolor red}
	animate $c $fromSlotX $fromSlotY $toSlotX $toSlotY $pcolor "" $orientation
	if {$orientation == 1} {
	    if {$gPlayerOneTurn == 1} {
		$c raise blueplus$toSlotX$toSlotY base
	    } else {

		$c raise redplus$toSlotX$toSlotY base
	    }

	} elseif {$orientation != 1} {
	    if {$gPlayerOneTurn == 1} {
		$c raise bluecross$toSlotX$toSlotY base
	    } else {
		$c raise redcross$toSlotX$toSlotY base
	    }
 
	}
    }	

    ### Swap turns
    if {$gPlayerOneTurn == 1} {set gPlayerOneTurn 0} {set gPlayerOneTurn 1}
 #   GS_DrawPosition $c $newPosition

    
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
    global gSlotsX gSlotsY gSlotSize
    global tk_library kRootDir gStippleRootDir
    set gStippleRootDir "$kRootDir/../bitmaps/"
    
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set remoteness [lindex $item 2]
	set delta [lindex $item 3]
	set color cyan

	 if {$moveType == "value" || $moveType == "rm"} {
		if {$value == "Tie"} {
		    set color yellow
		} elseif {$value == "Lose"} {
		    set color green
		} else {
		    set color red4
		}
	 }
	 set theAbsoluteMoveArg [ConvertToAbsoluteMove $move]
	 set from [lindex $theAbsoluteMoveArg 0]
	 set to   [lindex $theAbsoluteMoveArg 1]
	 set orientation [lindex $theAbsoluteMoveArg 2]

	 set fromSlotX [expr $from % $gSlotsX]
	 set fromSlotY [expr $from / $gSlotsX]
	 set toSlotX   [expr $to % $gSlotsX]
	 set toSlotY   [expr $to / $gSlotsX]	

     
     set stipple12 @[file join $gStippleRootDir gray12.bmp] 
	 set stipple25 @[file join $gStippleRootDir gray25.bmp]
	 set stipple50 @[file join $gStippleRootDir gray50.bmp]
	 set stipple75 @[file join $gStippleRootDir gray75.bmp]
     set stipple ""
     
     if {$moveType == "rm"} {
        set stipple $stipple50
        
        set delta [expr $delta / 2]
        
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
     
	 if { $from == 9 || $from == $to} { 
	     #DrawMoveChungToiPlacing or DrawMoveChungToiTwisting
	     if {$orientation == 1} {
		 $c raise moveplus$toSlotX$toSlotY base
		 $c itemconfig moveplus$toSlotX$toSlotY -fill $color -stipple $stipple
		 
		 $c bind moveplus$toSlotX$toSlotY  <Enter> "$c itemconfig moveplus$toSlotX$toSlotY -fill black"
		 $c bind moveplus$toSlotX$toSlotY <Leave> "$c itemconfig moveplus$toSlotX$toSlotY -fill $color"
		 $c bind moveplus$toSlotX$toSlotY <ButtonRelease-1> "ReturnFromHumanMove $move"
		 
	     } else {
		 $c raise movecross$toSlotX$toSlotY base
		 $c itemconfig movecross$toSlotX$toSlotY -fill $color -stipple $stipple
		
		 $c bind movecross$toSlotX$toSlotY  <Enter> "$c itemconfig movecross$toSlotX$toSlotY -fill black"
		 $c bind movecross$toSlotX$toSlotY <Leave> "$c itemconfig  movecross$toSlotX$toSlotY -fill $color"
		 $c bind movecross$toSlotX$toSlotY <ButtonRelease-1> "ReturnFromHumanMove $move"
	     }

	 } elseif { $from != $to } {
	  #   DrawMoveChungToiHopping
	     set movetemp [CreateHoppingArrow $c $theAbsoluteMoveArg $gSlotSize $gSlotsX $gSlotsY $color $stipple $move]
	     $c addtag moves withtag $movetemp
	     $c bind $movetemp <ButtonRelease-1> "ReturnFromHumanMove $move"

	 } else {
	     BadElse "DrawMoveChungToi"
	 }
#puts is printf
     }
  
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    $c lower moves base
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

    ### TODO if needed
    global gPlayerOneTurn
    if {$gPlayerOneTurn == 1} {set gPlayerOneTurn 0} {set gPlayerOneTurn 1}
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
	### TODO if needed
  $c create text [expr $gFrameWidth/2] [expr $gFrameWidth/2 - 40] -text "$nameOfWinner" -font Winner -fill orange -tags winner
    $c create text [expr $gFrameWidth/2] [expr $gFrameWidth/2 + 40] -text "WINS!"        -font Winner -fill orange -tags winner
	
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
    $c delete winner

}


#############################################################################
##
## DrawCross
##
## Here we draw an X on (slotX,slotY) in window w with a tag of theTag
##
#############################################################################
proc DrawCross { w slotX slotY theTag theColor stipple drawBig} {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 7.0)]

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
           -stipple $stipple \
		   -tag $theTag]

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCross
    $w move $theCross [expr $slotX*$theSlotSize] [expr $slotY*$theSlotSize]

    return $theCross
}

#############################################################################
##
## DrawPlus
##
## Here we draw a + on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################
proc DrawPlus { w slotX slotY theTag theColor stipple drawBig} {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 7.0)]
    set startPlus [expr $theSlotSize/2.0]
    set halfPlus  [expr $theSlotSize/2.0]
    set endPlus   [expr $startPlus*7.0]

    set thickness  [expr $theSlotSize/5.0  ]
    set x1 [expr $theSlotSize * 4 / 7 ]
    set x2 [expr $theSlotSize * 3 / 7 ]
    set x3 [expr $theSlotSize * 3 / 7 ]
    set x4 [expr $theSlotSize * 1 / 10 ]
    set x5 [expr $theSlotSize * 1 / 10 ]
    set x6 [expr $theSlotSize * 3 / 7 ]
    set x7 [expr $theSlotSize * 3 / 7 ]
    set x8 [expr $theSlotSize * 4 / 7 ]
    set x9 [expr $theSlotSize * 4 / 7 ]
    set x10 [expr $theSlotSize * 9 / 10]
    set x11 [expr $theSlotSize * 9 / 10]
    set x12 [expr $theSlotSize * 4 / 7 ]
    

    set y1 [expr $theSlotSize * 1 / 10]
    set y2 [expr $theSlotSize * 1 / 10]
    set y3 [expr $theSlotSize * 17 / 40] 
    set y4 [expr $theSlotSize * 17 / 40] 
    set y5 [expr $theSlotSize * 23 / 40]
    set y6 [expr $theSlotSize * 23 / 40]
    set y7 [expr $theSlotSize * 9 / 10]
    set y8 [expr $theSlotSize * 9 / 10]
    set y9 [expr $theSlotSize * 23 / 40]
    set y10 [expr $theSlotSize * 23 / 40]
    set y11 [expr $theSlotSize * 17 / 40]
    set y12 [expr $theSlotSize * 17 / 40]
    

    set thePlus [$w create polygon \
		   $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 \
		   $x7 $y7 $x8 $y8 $x9 $y9 $x10 $y10 $x11 $y11 \
		   $x12 $y12 \
		   -fill $theColor \
           -stipple $stipple \
		   -tag $theTag]

    $w addtag tagPieceOnCoord$slotX$slotY withtag $thePlus
    $w move $thePlus [expr $slotX*$theSlotSize] [expr $slotY*$theSlotSize]

    return $thePlus
}


#############################################################################
##
## CreateHoppingArrow
##
## Create and return arrow at the specified slots.
##
#############################################################################
proc CreateHoppingArrow { w theMoveArgAbsolute theSlotSize theSlotsX theSlotsY color stipple move} {

    global gSlotsX gSlotSize

    set theLineGap      [expr $gSlotSize / 7]

    set theLineWidth    [expr int($theSlotSize / 50)]
    
    set from [lindex $theMoveArgAbsolute 0]
    set to   [lindex $theMoveArgAbsolute 1]
    set orientation [lindex $theMoveArgAbsolute 2]


    set fromX [expr $from % $gSlotsX]
    set fromY [expr $from / $gSlotsX]
    set toX   [expr $to % $gSlotsX]
    set toY   [expr $to / $gSlotsX]
    
    set fromXOffset [expr ($fromX * $theSlotSize)]
    set fromYOffset [expr ($fromY * $theSlotSize)]
    set toXOffset [expr ($toX * $theSlotSize)] 
    set toYOffset [expr ($toY * $theSlotSize)]

    set isitCross [expr $orientation == 0]

      
    
    ######################################################################

    # Calculations to figure out where arrowheads, lines should go

    #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  
   
    if { [expr $from > $to] } {
	set largerSlot   $from
	set smallerSlot  $to
	set minYOffset [expr $toY * $gSlotSize]
	set minXOffset [expr $toX * $gSlotSize]
	set maxYOffset [expr $fromY * $gSlotSize]
	set maxXOffset [expr $fromX * $gSlotSize]
    } else {
	set largerSlot   $to
	set smallerSlot  $from
	set minYOffset [expr $fromY * $gSlotSize]
	set minXOffset [expr $fromX * $gSlotSize]
	set maxYOffset [expr $toY * $gSlotSize]
	set maxXOffset [expr $toX * $gSlotSize]
    }
    
    ### Figure out the dimensions and location of the line object, properly rotated. We always draw the line from the smaller-numbered slot to the lower one
    if { [expr ($largerSlot == 8 && (( $smallerSlot == 6) || ( $smallerSlot == 2))) || ($smallerSlot == 0 && (($largerSlot == 6 ) || ($largerSlot == 2) ) )  || (($largerSlot == 7) && ($smallerSlot == 1)) || (($largerSlot == 5) && ($smallerSlot == 3) ) ] } {
	# Long axis-aligned arrows 
	set theLineLength [expr $gSlotSize * 7 / 3 + (.75 *  $theLineGap)]
	if { [expr $largerSlot - $smallerSlot == 6] } {
	    # Vertical
	    set oX 0
	    set oY $theLineLength
	    set gutterH [expr $theLineGap / 2]  
	    set gutterV [expr $theLineGap * 2 ]
	    if { $isitCross } {
		set gutterH [expr $gutterH * 2]
	    } 
	    if { $largerSlot == 8 } {
		set slotPlaceX [expr $gSlotSize - $gutterH ]
	    } elseif { $largerSlot == 7 } {
		if { $isitCross } {
		    set slotPlaceX [expr ($gSlotSize / 2) - (.25 * $theLineGap)]
		} else {
		    set slotPlaceX [expr ($gSlotSize / 2) + (.25 * $theLineGap)] 
		}
		    
	    } else { 
		set slotPlaceX [expr $gutterH ]
	    }
	    set xStart [expr $slotPlaceX + $fromXOffset]
	    set yStart [expr $gutterV] 
	    set xEnd   [expr $xStart]
	    set yEnd   [expr $yStart + $theLineLength]
	
	} else {
	    # Horizontal
	    set oX $theLineLength
	    set oY 0
	    set gutterH [expr $theLineGap * 2] 
	    set gutterV [expr $theLineGap / 2]
	    if { $isitCross } {
		set gutterV [expr $gutterV * 2]
	    } 
	    if { $largerSlot == 8 } {
		set slotPlaceY [expr $gSlotSize - $gutterV ]
	    } elseif { $largerSlot == 5 } { 
		if { $isitCross } {
		    set slotPlaceY [expr ($gSlotSize / 2) - (.25 * $theLineGap)]  
		} else {
		    set slotPlaceY [expr ($gSlotSize / 2) + (.25 * $theLineGap)]
		}
	    } else { 
		set slotPlaceY $gutterV
	    }
	    set xStart $gutterH
	    set yStart [expr $slotPlaceY + $toYOffset]
	    set xEnd   [expr $xStart + $theLineLength]
	    set yEnd   [expr $yStart]

	}
    } elseif { [expr ($largerSlot - $smallerSlot == 3)] }  {
	# Short axis-aligned  vertical arrows
	set theLineLength [expr $gSlotSize * .5]
	set oX 0
	set oY $theLineLength
	set gutterH [expr $theLineGap * 1.5]
	set gutterV [expr $theSlotSize - (.5 * $theLineLength)]
	if { $isitCross } {
	    set gutterH [expr $gutterH + (.5 * $theLineGap)]
	}
	if { [expr (($largerSlot == 6) || ($largerSlot == 3))] } {
	    set slotPlaceX $gutterH 
	} elseif { [expr (($largerSlot == 8) || ($largerSlot == 5))] } {
	    set slotPlaceX [expr $gSlotSize - $gutterH]
	} else {
	    if { $isitCross } {
		set slotPlaceX [expr ($gSlotSize / 2) - (.75 * $theLineGap) ]  
	    } else {
		set slotPlaceX [expr ($gSlotSize / 2) + (.75 * $theLineGap)]
	    }
	}
	set xStart [expr $toXOffset + $slotPlaceX]
	set yStart [expr $gutterV  + $minYOffset]
	set xEnd   $xStart
	set yEnd   [expr $yStart + $theLineLength]
    } elseif { [expr (($largerSlot - $smallerSlot == 1) && !($largerSlot == 3 || $largerSlot == 6) ) ] } { 
	# Short axis-aligned horizontal arrows
	
	set theLineLength [expr $gSlotSize * .5]
	set oX $theLineLength
	set oY 0
	set gutterH [expr $theSlotSize - (.5 * $theLineLength)]

	set gutterV [expr $theLineGap * 1.5]
	if { $isitCross } {
	    set gutterV [expr $gutterV + (.5 * $theLineGap)]
	}
	if { [expr (($largerSlot == 2 ) || ($largerSlot == 1))] } {
	    set slotPlaceY $gutterV
	} elseif { [expr (($largerSlot == 8) || ($largerSlot == 7))] } {
	    set slotPlaceY [expr $gSlotSize - $gutterV]
	} else {
	    if { $isitCross } {
		set slotPlaceY [expr ($gSlotSize / 2) - (.75 * $theLineGap) ]  
	    } else {
		set slotPlaceY [expr ($gSlotSize / 2) + (.75 * $theLineGap)]
	    }
	}

	set xStart [expr $gutterH + $minXOffset]
	set yStart [expr $toYOffset + $slotPlaceY]
	set xEnd [expr $xStart + $theLineLength]
	set yEnd $yStart
    
    } elseif { [expr ((($largerSlot == 8) && ($smallerSlot == 0)) || (($largerSlot == 6) && $smallerSlot == 2)) ] } {
	# Long Diagonal arrows
	
	set gutterH  [expr $theSlotSize * .125]
	set gutterV  [expr $theSlotSize * .0675]
	if { $isitCross } { 
	    set gutterVOffset [expr $gutterV + (.25 * $theLineGap)]
	} else {
	    set gutterVOffset 0
	}
	if { [expr $largerSlot == 8] } {
	    set xStart [expr $gutterH]
	    set yStart [expr $gutterV + $gutterVOffset]
	    set xEnd [expr $maxXOffset + $gSlotSize - $gutterH]
	    set yEnd [expr $maxYOffset + $gSlotSize - (2.375 *  $gutterV) + $gutterVOffset]
	} else {
	    set xStart [expr $minXOffset + $gSlotSize - $gutterH]
	    set yStart [expr $gutterV + $gutterVOffset]
	    set xEnd [expr $gutterH]
	    set yEnd [expr $maxYOffset + $gSlotSize - (2.375 *  $gutterV) + $gutterVOffset]
	}

    } else {
	# short diagonal arrows
#	set theLineLength [expr $gSlotSize / 2]
	set gutterH [expr $gSlotSize / 3]
	set gutterV [expr $gSlotSize / 3]
	
	if { [expr ($largerSlot - $smallerSlot) == 4]  } {
	    # Right/Down arrows
	    if { $isitCross } {
		
		set xStart [expr $minXOffset + $gSlotSize - $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - 2 * $theLineGap]
		set xEnd [expr $maxXOffset + 2 * $theLineGap]
		set yEnd [expr $maxYOffset + $theLineGap]
	    } else {
		set xStart [expr $minXOffset + $gSlotSize - 2 * $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - $theLineGap]
		set xEnd  [expr $maxXOffset + $theLineGap]
		set yEnd  [expr $maxYOffset + 2 * $theLineGap] 
	    }
	} elseif { [expr ($largerSlot - $smallerSlot) == 2] } { 
	    # Left/Down arrows
	    if { $isitCross } {
		set xStart [expr $minXOffset + $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - 2 * $theLineGap]
		set xEnd [expr $maxXOffset + $gSlotSize - 2 * $theLineGap]
		set yEnd [expr $maxYOffset + $theLineGap]
	    } else {
		set xStart [expr $minXOffset + 2 * $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - $theLineGap]
		set xEnd [expr $maxXOffset +  $gSlotSize - $theLineGap]
		set yEnd [expr $maxYOffset + 2 * $theLineGap]
	    }
	} else {
	    BadElse "CreateHoppinArrow"
	
	}
	
    }

    ### Figure out the coordinates of the leftmost/topmost point
    
    
    #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  




    set lineStartX $xStart
    set lineStartY $yStart
    set lineEndX   $xEnd
    set lineEndY   $yEnd

    set endPointOffset [expr $theSlotSize / 8] 

    ######################################################################
    
    set theMove [$w create line $lineStartX $lineStartY $lineEndX $lineEndY \
	    -width $theLineWidth \
	    -fill $color \
        -stipple $stipple]
    
    # We draw from the smaller-numbered slot to the larger-numbered one
    if { [expr $smallerSlot == $from] } {
	set ArrowheadX [expr $lineEndX - .5 * $theLineGap]
	set ArrowheadY [expr $lineEndY -  .5 * $theLineGap]
    } else {
	set ArrowheadX [expr $lineStartX - .5 * $theLineGap]
	set ArrowheadY [expr $lineStartY  - .5 * $theLineGap]
    }

	
    if { $isitCross } {
	set theEndPoint [DrawCross $w 0 0 tagDummyForMoves $color $stipple 0]
    } else {
	set theEndPoint [DrawPlus  $w 0 0 tagDummyForMoves $color $stipple 0]
    }
    
    ####
    $w addtag arrows withtag $theEndPoint
    $w addtag arrows withtag $theMove
    ## 
    $w addtag tagMoves withtag $theEndPoint
    $w addtag tagEnd$to$from$orientation withtag $theMove
    $w addtag tagEnd$to$from$orientation withtag $theEndPoint
    $w bind tagEnd$to$from$orientation <Enter> "$w itemconfig $theEndPoint -fill black"
    $w bind tagEnd$to$from$orientation <Leave> "$w itemconfig $theEndPoint -fill $color"

    $w addtag tagLine$to$from$orientation withtag $theMove
    $w addtag tagLine$to$from$orientation withtag $theEndPoint
    $w bind tagLine$to$from$orientation <Enter> "$w itemconfig $theMove -fill black"
    $w bind tagLine$to$from$orientation <Leave> "$w itemconfig $theMove -fill $color"
    
    $w addtag tagRaiseLine$to$from$orientation withtag $theMove
    $w addtag tagRaiseLine$to$from$orientation withtag $theEndPoint
    $w bind tagRaiseLine$to$from$orientation <Enter> "$w raise tagRaiseLine$to$from$orientation all"



    
    $w bind $theEndPoint <ButtonRelease-1> "ReturnFromHumanMove $move"
    
    

    
    $w move $theEndPoint $ArrowheadX $ArrowheadY

    
    return $theMove
}



proc ConvertToAbsoluteMove { theMove } {
    global gPosition

    if { $theMove == "041" } {
	set theMove "41"
    }

    if { $theMove == "040"} {
	set theMove "40"
    }


    set fromSlot    [expr ($theMove /100)]
    set toSlot      [expr (($theMove % 100) / 10)]
    set orientation [expr ($theMove - ($fromSlot * 100) - ($toSlot * 10))] 

    set theMove [list $fromSlot $toSlot $orientation]

    return $theMove
}
# More useful move stuff
#	set from [lindex $theAbsoluteMoveArg 0]
#	set to   [lindex $theAbsoluteMoveArg 1]
#	set orientation [lindex $theAbsoluteMoveArg 2]
#	
#	set fromSlotX [expr $from % $gSlotsX]
#	set fromSlotY [expr $from / $gSlotsX]
#	set toSlotX   [expr $to % $gSlotsX]
#	set toSlotY   [expr $to / $gSlotsX]


proc animate { c x0 y0 x1 y1 color stipple orientation} {

    global gAnimationSpeed gSlotSize
    set BaseSpeed 100
    #draw temp piece
    if {$orientation == 1} {
	set piece [DrawPlus $c $x0 $y0 temppiece $color $stipple 1]
    } else {
	set piece [DrawCross $c $x0 $y0 temppiece $color $stipple 1]
    }
    # Get source and destination coordinates.
 #   set x0 [lindex $source 0]
  #  set x1 [lindex $destination 0]
  #  set y0 [lindex $source 1]
  #  set y1 [lindex $destination 1]

    # Relative speed factor gotten from gAnimationSpeed
    # speed should equal the amount of ms we take to run this whole thing


    set speed [expr $BaseSpeed / pow(2, $gAnimationSpeed)]

    # If things get too fast, just make it instant
    if {$speed < 5} {
	set speed 1
    }

    # Distance to be moved per unit of delay.
    set dx [expr [expr [expr $x1 - $x0] * $gSlotSize] / $speed]
    set dy [expr [expr [expr $y1 - $y0] * $gSlotSize] / $speed]

    # Animate the move.
    for {set i 1} {$i < [expr $speed +1]} {incr i} {
	$c move $piece $dx $dy
	
	after 1
	update idletasks
    }

    $c delete temppiece

}


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

font create Winner -family arial -size 50
