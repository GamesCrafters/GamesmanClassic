
# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

# global var used, for hopefully scaling later
# also to fix a graphical bug due to hardcoding
# 5-23-06 David Chan
# 
set CANVAS_HEIGHT 0
set CANVAS_WIDTH 0

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Nim"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    global backgroundImage
    global playareaImage
    set kCAuthors "Gamesman Spring 2003 Whole Team!"
    set kTclAuthors "Gamesman Spring 2003 Whole Team!"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
    set backgroundImage "$kRootDir/../bitmaps/mnim-background.gif"
    set playareaImage "$kRootDir/../bitmaps/mnim-table.gif"

    global valueMoveColorLose
    global valueMoveColorWin
    global valueMoveColorOff

    set valueMoveColorLose red4
    set valueMoveColorWin green
    set valueMoveColorOff cyan

    global gFrameWidth gFrameHeight CANVAS_WIDTH CANVAS_HEIGHT
    
    set CANVAS_WIDTH [min $gFrameWidth $gFrameHeight]
    set CANVAS_HEIGHT $CANVAS_WIDTH
    

    global kMinRows kMaxRows
    set kMinRows 1
    set kMaxRows 5

    # Set number of rows
    global gRows gRowsOption
    set gRows [expr $gRowsOption + $kMinRows]

    # Set toMove toWin
    global gMisereGame
    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    set toWin2 "Remove the last piece from the board"

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Players take turns removing any number of counters from a single row. Click on a piece to remove that piece and all the pieces above it in that row"
    
    ### Set the initial position of the board
    global gInitialPosition gPosition

    # 7858 is 1, 3, 5, 7-piece columns
    # set gInitialPosition [expr int((pow(2, $gRows*3) - 1)) * 2]
    set gInitialPosition 7858
    set gPosition $gInitialPosition
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
    return [list left right]
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
    return [list green green]
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

    set rowsRule \
	[list \
	     "How many rows?" \
	     "1" "2" "3" "4" "5"
	 ]

    # List of all rules, in some order
    set ruleset [list $standardRule $rowsRule]

    # Declare and initialize rule globals
    global gMisereGame gRowsOption
    set gMisereGame 0
    set gRowsOption 4

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gRowsOption"]

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
    global gMisereGame gRowsOption
    global kMinRows kMaxRows
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    set option [expr $option + (2*$gRowsOption)]
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
    global gMisereGame gRowsOption
    global kMaxRows kMinRows
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
    set gRowsOption [expr $option/2%($kMaxRows - $kMinRows + 1)]
}

proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}


# GS_Initialize draws the graphics for the game on the canvas c
# You could put an opening animation or just draw an empty board.
# This function is called after GS_InitGameSpecific

proc GS_Initialize { c } {
    global backgroundImage
    global playareaImage gFrameWidth gFrameHeight size gInitialPosition

    set size [min $gFrameWidth $gFrameHeight]

    $c create rect 0 0 $size $size -fill black -tag base
    $c create rect 0 [expr 0.6 * $size] $size $size -fill darkgrey -tag base
    
    global gRows

    for {set j 0} {$j<7} {incr j} {
	for {set i 0} {$i<$gRows} {incr i} {
	    
	    set x [expr 70*$i+($size/2)-35*($gRows-1)]
	    set y [expr (0.75*$size)-(30*$j+50)]
	    set h [expr $j+1]
	    set w [expr $i+1]
	    $c create oval [expr $x-25] [expr $y-20] [expr $x+25] [expr $y+20] \
		-outline black -fill SteelBlue2 -tag move-$w$h
	    $c bind move-$w$h <ButtonRelease-1> "MyReturnFromHumanMove $w $h"
	}
    }

    GS_DrawPosition $c $gInitialPosition
}

proc GS_Deinitialize { c } {
    $c delete all
}

proc MyReturnFromHumanMove {w h} {
    global gPosition
    set l [unhash $gPosition]
    set dummy [expr [lindex $l [expr $w-1]]-$h+1]
    ReturnFromHumanMove $w$dummy
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

proc unhash { position } {
    set position [expr $position/2]
    set retval []
    global gRows
    for {set i 1} {$i<=$gRows} {incr i} {
	lappend retval [expr $position%8]
	set position [expr $position/8]
    }
    
    return $retval
}

proc GS_DrawPosition { c position } {
    
    set l [unhash $position]
    global CANVAS_HEIGHT
    global gRows size
    for {set j 0} {$j<7} {incr j} {
	for {set i 0} {$i<$gRows} {incr i} {
	    
	    set x [expr 70*$i+($size/2)-35*($gRows-1)]
	    set y [expr 0.75*$size-(30*$j+50)]
	    set h [expr $j+1]
	    set w [expr $i+1]
	    $c coords move-$w$h [expr $x-25] [expr $y-20] [expr $x+25] [expr $y+20]
	    $c raise move-$w$h
	}
    }
    
    for {set i 1} {$i<=$gRows} {incr i} {
	set height [lindex $l [expr $i-1]]
	for {set j 7} {$j>$height} {incr j -1} {
	    #changed to fix the undo graphical bug
	    # used to be hardcoded value of -400
	    $c move move-$i$j 0 -$CANVAS_HEIGHT
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
    $c delete winner
    GS_DrawPosition $c $position
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    return [lindex {left right} [expr $position%2]]
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

# hard-coding the value -8 makes me really nervous here.  ideally, it should
# be calculated from the window height.  (c has this information in it's height
# tag, but i can't make it CONFESS!)
proc GS_HandleMove { c oldPosition theMove newPosition } {

    global size

    set oldl [unhash $oldPosition]
    set newl [unhash $newPosition]
    set i  [expr $theMove/10]
    
    set im1 [expr $i-1]
    #if the animation sliderbar is high, this value should be low
    set maxframes [ScaleDownAnimation 50]
    set dely [ScaleUpAnimation [expr -0.8*$size / 50]]
    
    for {set frame 0} {$frame < $maxframes} {incr frame} {
	
	for {set j [lindex $oldl $im1]} {$j>[lindex $newl $im1]} {incr j -1} {
	    $c move move-$i$j 0 $dely
	}
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
# We provide a procedure called MoveValueToColor that takes in moveType and a moveValue and returns the correct color.
# For example [MoveValueToColor "value" "Tie"] would return yellow and [MoveValueToColor "all" "anything"] would return cyan.

proc GS_ShowMoves { c moveType position moveList } {
    global valueMoveColorLose
    global valueMoveColorWin
    global valueMoveColorOff

    set l [unhash $position]
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set color $valueMoveColorOff
	
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color $valueMoveColorWin
	    } else {
		set color $valueMoveColorLose
	    }
	}
	set i [expr $move/10]
	set j [expr [lindex $l [expr $i-1]]-$move%10+1]
	$c itemconfigure move-$i$j -fill $color
    }
    update idletasks    
}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    
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
    GS_DrawPosition $c $positionAfterUndo
}



# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}



# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    global size
    
    set fontsize [expr int($size/15)]

    $c create text [expr $size/2] [expr $size/2 - 90] -text "$nameOfWinner" -font "Arial $fontsize" -fill orange -tags winner
    $c create text [expr $size/2] [expr $size/2 + 20] -text "WINS!"         -font "Arial $fontsize" -fill orange -tags winner
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
    $c delete winner
}
