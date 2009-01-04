####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

set boardHeight 2
set boardWidth  2
set margin      100
set maxWidth    3
set maxHeight   3
set r           5
set basespeed   50
set turn        0

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
    set kGameName "Dots and Boxes"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition boardHeight boardWidth
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ______ WINS"
    set kMisereString "First player to ______ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Score more boxes than your opponent. Boxes are denoted by an X or an O."
    } else {
	SetToWinString "To Win: Force your opponent to draw the most boxes."
    }
    SetToMoveString "To Move: Select the line you want to begin drawing a square. You may make square even if it contains an opponent's line. Select the line you want to draw on the board."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "(Fill this in)"
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
    ### FILL THIS IN

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
	### FILL THIS IN
    
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

    set widthRule \
	[list \
	     "Select a board width:" \
	     "1" \
	     "2" \
	     "3" \
	    ]

    set heightRule \
	[list \
	     "Select a board height:" \
	     "1" \
	     "2" \
	     "3" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $widthRule $heightRule]

    # Declare and initialize rule globals
    global gMisereGame boardWidth boardHeight boardWidthOpt boardHeightOpt
    set gMisereGame    0
    set boardWidthOpt  1
    set boardHeightOpt 1

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "boardWidthOpt" "boardHeightOpt"]

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
    global gMisereGame boardWidth boardHeight maxWidth maxHeight boardWidthOpt boardHeightOpt
    set boardWidth [expr $boardWidthOpt + 1]
    set boardHeight [expr $boardHeightOpt + 1]

    set option 1
    set option [expr $option + $gMisereGame]
    set option [expr $option + 2 * ($boardWidth - 1)]
    set option [expr $option + 2 * $maxWidth * ($boardHeight - 1)]

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
    global gMisereGame boardWidth boardHeight maxWidth maxHeight

    incr option -1
    set gMisereGame [expr $option%2]
    set boardWidth  [expr $option/2 % $maxWidth + 1]
    set boardHeight [expr $option/(2*$maxWidth) % $maxHeight + 1]
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

    global gFrameWidth gFrameHeight boardHeight boardWidth margin r gInitialPosition

    DrawBoard $c
    DrawPieces $c $gInitialPosition

}

proc DrawBoard { c } {
    global gFrameWidth gFrameHeight boardHeight boardWidth margin r square

    # We'd like to have either the rows or cols (whichever we have more of) fill the board.
    # We'll set $square to the size of the squares we want to create that will achieve this.
    if {$boardHeight > $boardWidth} {
        set square [expr ($gFrameHeight-2*$margin) / $boardHeight]
    } else {
        set square [expr ($gFrameWidth-2*$margin) / $boardWidth]
    }
    
    $c create rect -1 -1 $gFrameWidth $gFrameHeight -fill white
    $c create line [expr $margin-10] 0 [expr $margin-10] $gFrameHeight -fill red -width 1

    for {set i [expr $margin-10]} {$i < $gFrameHeight} {set i [expr $i+20]} {
	$c create line 0 $i $gFrameWidth $i -fill blue -width 1
    }

    # Need to create a $row+1, $col+1 grid of dots separated by $square
    for {set i 0} {$i < $boardHeight+1} {incr i} {
	for {set j 0} {$j < $boardWidth+1} {incr j} {
	    $c create oval [expr ($margin+$j*$square)-$r] [expr ($margin+$i*$square)-$r] [expr ($margin+$j*$square)+$r] [expr ($margin+$i*$square)+$r] -fill black
	}
    }
}

proc DrawPiece { c index } {
    global margin square boardWidth boardHeight

    # Draw line at index
    # If 0 <= index <= 6, horizontal line
    # If 7 <= index <= 12, vertical line
    if {$index < [expr ($boardHeight+1)*$boardWidth]} {
	$c create line \
	    [expr $margin+($index%$boardWidth)*$square] \
	    [expr $margin+($index/$boardWidth)*$square] \
	    [expr $margin+($index%$boardWidth+1)*$square] \
	    [expr $margin+($index/$boardWidth)*$square] \
	    -fill black -width 4 -tag "pieces"
    } else {
	set index [expr $index-($boardHeight+1)*$boardWidth]
	$c create line \
	    [expr $margin+($index/$boardHeight)*$square] \
	    [expr $margin+($index%$boardHeight)*$square] \
	    [expr $margin+($index/$boardHeight)*$square] \
	    [expr $margin+($index%$boardHeight+1)*$square] \
	    -fill black -width 4 -tag "pieces"
    }
}

proc DrawPieces { c position } {
    global boardWidth boardHeight
    set board [unhash $position]
    for {set i 0} {$i < [expr $boardWidth*($boardHeight+1)+$boardHeight*($boardWidth+1)]} {incr i} {
	if {[string index $board $i] == 1} {
	    DrawPiece $c $i
	}
    }
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
    
    global boardWidth boardHeight

    $c delete pieces
    DrawPieces $c $position

}

proc LabelBox { c i j flag } {
    global margin square
    if {$flag == -1} {
	return
    } elseif {$flag == 0} {
	set color "blue"
    } else {
	set color "red"
    }
    
    # Draw a diagonal line through the box with the appropriate color
    $c create line [expr $margin+$j*($square)] [expr $margin+$i*($square)] [expr $margin+($j+1)*($square)] [expr $margin+($i+1)*($square)] -width 4 -fill $color -tags [list label "l$i$j"]
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
    set turn 0
    $c delete label
    GS_DrawPosition $c $position

    # Anoto pen support - start new game
    global boardWidth boardHeight
    C_SetGameSpecificOptions 1 $boardWidth $boardHeight
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

    global boardWidth boardHeight turn
    
    AnimateMove $c $theMove
    GS_DrawPosition $c $newPosition

    # Here, we implement tracking of box owners
    # Namely, if theMove completes a box, note who won that box in a list
    # Will have to keep track of who's move it is as well
    set count 0
    for {set i 0} {$i < $boardWidth * $boardHeight} {incr i} {
	set box [BoxCompleted [unhash $newPosition] $i $theMove]

	# Check if we've completed a box
	if { $box != -1 } {
	    # We have completed a box
	    # turn does not change
	    # mark the box we completed according to whose turn it is
	    LabelBox $c [lindex $box 0] [lindex $box 1] $turn
	    incr count
	    if {$count == 2} {
		break
	    }
	}
    }

    # We went through and didn't find any new completed boxes
    if { $count == 0 } {
	ChangeTurn
    }
    

}

proc BoxCompleted { board index move } {
    global boardWidth boardHeight
    # we have boardWidth * boardHeight boxes
    # for the nth box, check the following sides:
    #
    # row = n / boardWidth
    # col = n % boardWidth
    # half = boardWidth * (boardHeight + 1)
    #
    # sides are:
    # top: row * boardWidth + col
    # bot: (row+1) * boardWidth + col
    # lef: half + col * boardHeight + row
    # rig: half + (col+1) * boardHeight + row
    set row [expr int($index/$boardWidth)]
    set col [expr $index%$boardWidth]
    set half [expr $boardWidth * ($boardHeight+1)]

    set top [expr int($row * $boardWidth + $col)]
    set bot [expr int(($row+1) * $boardWidth + $col)]
    set lef [expr int($half + $col * $boardHeight + $row)]
    set rig [expr int($half + ($col+1) * $boardHeight + $row)]

    # Is the move related to the box?
    if { $top != $move && $bot != $move && $lef != $move && $rig != $move } {
	return -1
    }
    if { [string index $board $top] == 1 && [string index $board $bot] == 1 && [string index $board $lef] == 1 && [string index $board $rig] == 1 } {
	# We've completed a box! Return i, j
	return [list $row $col]
    }
    return -1
}

proc ChangeTurn {} {
    global turn
    if { $turn == 0 } {
	set turn 1
    } else {
	set turn 0
    }
}

proc AnimateMove { c move } {
    global square basespeed gAnimationSpeed margin boardHeight boardWidth
    set speed [expr $basespeed / pow(2, $gAnimationSpeed)]
    set delta [expr $square / $speed]

    set dir 0
    set col [expr $move%$boardWidth]
    set row [expr floor($move/$boardWidth)]
    if {$move >= [expr ($boardHeight+1)*$boardWidth]} {
	set move [expr ($move-($boardHeight+1)*$boardWidth)]
	set row [expr $move%$boardHeight]
	set col [expr floor($move/$boardHeight)]
	set dir 1
    }

    for {set i 0} {$i < $square} {set i [expr $i+$delta]} {
	if { $dir == 0 } {
	    $c create line [expr $margin+$col*($square)] [expr $margin+$row*($square)] [expr $margin+$col*($square)+$i] [expr $margin+$row*($square)] -width 4 -fill black -tag tmp
	} else {
	    $c create line [expr $margin+$col*($square)] [expr $margin+$row*($square)] [expr $margin+$col*($square)] [expr $margin+$row*($square)+$i] -width 4 -fill black -tag tmp
	}
	after 1
	update idletasks
	$c delete tmp
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
    $c delete moves
    foreach move $moveList {
	DrawMove $c [lindex $move 0] $moveType [lindex $move 1]
    }
}

proc DrawMove { c move moveType type } {

    global margin square boardWidth boardHeight r

    switch $moveType {
	value {

	    # If the move leads to a win, it is a losing move (RED)
	    # If the move leads to a losing position, it is a winning move (GREEN)
	    # If the move leads to a tieing position, it is a tieing move (YELLOW)
	    switch $type {
		Win { set color darkred }
		Lose { set color green }
		Tie { set color yellow }
		default { set color cyan }
	    }
	}
	default {
	    set color cyan
	}
    }

    # Draw move at index
    # If 0 <= index <= 6, horizontal line
    # If 7 <= index <= 12, vertical line
    if {$move < [expr ($boardHeight+1)*$boardWidth]} {
	set tmp [$c create line \
		     [expr $margin+($move%$boardWidth)*$square+$r] \
		     [expr $margin+($move/$boardWidth)*$square] \
		     [expr $margin+($move%$boardWidth+1)*$square-$r] \
		     [expr $margin+($move/$boardWidth)*$square] \
		     -fill $color -width $r -tag "moves"]
    } else {
	set tmp [$c create line \
		     [expr $margin+(($move-($boardHeight+1)*$boardWidth)/$boardHeight)*$square] \
		     [expr $margin+(($move-($boardHeight+1)*$boardWidth)%$boardHeight)*$square+$r] \
		     [expr $margin+(($move-($boardHeight+1)*$boardWidth)/$boardHeight)*$square] \
		     [expr $margin+(($move-($boardHeight+1)*$boardWidth)%$boardHeight+1)*$square-$r] \
		     -fill $color -width $r -tag "moves"]
    }

    $c bind $tmp <Enter> "$c itemconfigure $tmp -fill black"
    $c bind $tmp <Leave> "$c itemconfigure $tmp -fill $color"
    $c bind $tmp <ButtonRelease-1> "ReturnFromHumanMove $move"
}

#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in
    $c delete moves
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

    global boardWidth boardHeight

    set count 0
    for {set i 0} {$i < $boardWidth * $boardHeight} {incr i} {
	set box [BoxCompleted [unhash $currentPosition] $i $theMoveToUndo]

	# Check if we've completed a box
	if { $box != -1 } {
	    # We have completed a box
	    # turn does not change
	    # mark the box we completed according to whose turn it is
	    $c delete "l[lindex $box 0][lindex $box 1]"
	    incr count
	    if {$count == 2} {
		break
	    }
	}
    }

    # We went through and didn't find any new completed boxes
    if { $count == 0 } {
	ChangeTurn
    }

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
    global boardWidth boardHeight
    for {set i 0} {$i < [expr $boardWidth*($boardHeight+1)+$boardHeight*($boardWidth+1)]} {incr i} {
	if {[expr (1<<$i) & $position] != 0} {
	    append board 1
	} else {
	    append board 0
	}
    }
    return $board
}