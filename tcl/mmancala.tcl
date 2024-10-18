####################################################
# mmancala.tcl
# by Spencer Ray and Keith Ho
####################################################

# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

proc GS_InitGameSpecific {} {

   # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Alice Chang, Judy Chen, Eleen Chiang, Peter Foo"
    set kTclAuthors "Judy Chen, Eleen Chiang, Peter Foo"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"    

    ### Set the name of the game
    
    global kGameName
    set kGameName "Mancala"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 2230173
    set gPosition $gInitialPosition

    ### Set the board size and number of pieces
    global boardSize
    set boardSize 8
    global numOfPieces 
    set numOfPieces 24

    ### Set etc
    global turnOffset numberOfPositions 
    set turnOffset 2629575
    set numberOfPositions [expr $turnOffset * 2]

    global gHashMinMax gHashBoardSize gNCR
    set gHashMinMax [] 
    set gHashBoardSize 0

    global binPieces binPiecesOffsets
    set binPieces []
    global pieceSize

    global colorMove
    set colorMove 1

    rearranger_hash_init [expr $boardSize + $numOfPieces - 1] $numOfPieces [expr $boardSize - 1]

    ### Set toMove and toWin
    global gMisereGame
    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    set toWin2  "Play until all the stones are contained in the two mancalas and no more moves are possible. The player whose mancala contains more stones wins." 

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: The first player begins by picking up his pieces in any of the holes on his side. Moving counter-clockwise, the player deposits one of his pieces into each hole until his pieces run out. The player starts by placing a seed into the hole next to the one he or she just emptied."
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
    set option [expr $option + $gMisereGame]

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
    set gMisereGame [expr $option%2]
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {

    return [list 0 1]

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

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    global boardSize binPieces pieceSize gFrameWidth gFrameHeight
    set canvasSize [min $gFrameWidth $gFrameHeight]
    set fontSize [expr int($canvasSize / 25)]
    set boardYOffset [expr ($canvasSize / 3) / 2]
    set binSize [expr $canvasSize / (($boardSize / 2) + 2)]
    set x [expr $canvasSize / 150]

    $c create rect 0 $boardYOffset $canvasSize [expr $canvasSize - $boardYOffset] \
	-fill #b98e60 -tag board
    
    set counter 0
    for {set j [expr ($canvasSize / 3)]} {$j <= 2 * ($canvasSize / 3) || abs([expr $j - 2 * ($canvasSize / 3)]) < 1 } \
	{set j [expr $j + ($canvasSize / 3)]} { 
	if { $counter == [expr ($boardSize / 2) - 1] } {
	    set counter $boardSize
	}
	#the '+ 0.01' was added because cygwin has a rounding error and exits out of the loop after only 2 columns
	for {set i [expr $binSize * 2]} {$i <= [expr $binSize * ($boardSize / 2) + 0.01]} \
	    {set i [expr $i + $binSize]} {
	    if { $counter < [expr $boardSize / 2] } {
		incr counter  
	    } elseif { $counter > [expr $boardSize / 2] } {
		set counter [expr $counter - 1]
	    }
		
	    $c create oval [expr $i - ($binSize / 2) + $x] [expr $j - ($binSize / 2) + $x] \
		[expr $i + ($binSize / 2) - $x] [expr $j + ($binSize / 2) - $x] \
		-fill white -tag bin$counter

	    $c create text $i [expr $j - ($binSize / 3.3)] \
		-text "0" -font [list Times $fontSize] -tag counter$counter

	    $c bind bin$counter <ButtonRelease-1> "ReturnFromHumanMove $counter"
	    $c bind counter$counter <ButtonRelease-1> "ReturnFromHumanMove $counter"
	    lappend binPieces 0
	}
    }

    $c create oval [expr $binSize / 2] [expr ($canvasSize / 3) - ($binSize / 2)] \
	[expr ($binSize / 2) + $binSize] [expr ($canvasSize / 3) * 2 + ($binSize / 2)] \
	-fill white -tag bin0
    $c create text $binSize [expr $canvasSize / 3.6] \
	-text "0" -font [list Times $fontSize] -tag counter0
    lappend binPieces 0

    $c create oval [expr $canvasSize - $binSize - ($binSize / 2)] \
	[expr ($canvasSize / 3) - ($binSize / 2)] \
	[expr $canvasSize - $binSize + ($binSize / 2)] \
	[expr ($canvasSize / 3) * 2 + ($binSize / 2)] -fill white -tag bin[expr $boardSize / 2]
    $c create text [expr $canvasSize - $binSize] \
	[expr $canvasSize / 3.6] \
	-text "0" -font [list Times $fontSize] -tag counter[expr $boardSize / 2]
    lappend binPieces 0

    $c lower board    
    set pieceSize [expr $binSize / 9]
    makeOffsets [expr 1.5 * $pieceSize] $pieceSize

    update idletasks
} 

# creates a piece in BIN. 
proc drawPiece { c bin } {
    global binPieces pieceSize binPiecesOffsets

    set x [lindex $binPieces $bin]

    set xo [lindex $binPiecesOffsets [expr $x * 2]]
    set yo [lindex $binPiecesOffsets [expr ($x * 2) + 1]]
    set cx [expr [expr [expr [expr [lindex [$c coords bin$bin] 0] + [lindex [$c coords bin$bin] 2]] / 2] + $xo] - ( ($pieceSize * 1.5) / 2 )]
    set cy [expr [expr [expr [expr [lindex [$c coords bin$bin] 1] + [lindex [$c coords bin$bin] 3]] / 2] + $yo] - ( $pieceSize / 2)]

    $c create oval $cx $cy [expr $cx + (1.5 * $pieceSize)] [expr $cy + $pieceSize] \
	-fill gray -tag piece$bin,[lindex $binPieces $bin]
    $c bind piece$bin,[lindex $binPieces $bin] <ButtonRelease-1> "ReturnFromHumanMove $bin"
    set binPieces [lreplace $binPieces $bin $bin [expr [lindex $binPieces $bin] + 1]]
    

}

# Deletes a piece from BIN. Assumes there is > 0 pieces in BIN. 
proc deletePiece { c bin } {
    global binPieces
    set binPieces [lreplace $binPieces $bin $bin [expr [lindex $binPieces $bin] - 1]]
    $c delete piece$bin,[lindex $binPieces $bin]
}

# Creates a list of offsets from the center of a bin with which to draw pieces.
proc makeOffsets { x y } {
    global binPiecesOffsets
    set binPiecesOffsets []
    lappend binPiecesOffsets [expr $x * -.75] 0 0 [expr $y * -.75] [expr $x * .75] 0 \
	0 [expr $y * .75] 0 0 [expr $x * -.75] [expr $y * -.75] [expr $x * -.75] [expr $y * .75] \
	[expr $x * .75] [expr $y * -.75] [expr $x *.75] [expr $y * .75] [expr $x * -.75 * 2] 0 \
	[expr $x * .75 * 2] 0 [expr $y * -.75 * 2] 0 [expr $y * .75 * 2] 0 \
	[expr $x * -.75 * 1.5] [expr $y * -.75 * 1.5] [expr $x * .75 * 1.5] [expr $y * -.75 * 1.5] \
	[expr $x * -.75 * 1.5] [expr $y * .75 * 1.5] [expr $x * .75 * 1.5] [expr $y * .75 * 1.5] \
	0 [expr $y * -.75 * 2] 0 [expr $y * .75 * 2] [expr $x * -.75 * 1.2] [expr $y * -.75 * 1.8] \
	[expr $x * .75 * 1.2] [expr $y * -.75 * 1.8] [expr $x * -.75 * 1.2] [expr $y * .75 * 1.8] \
	[expr $x * .75 * 1.2] [expr $y * .75 * 1.8] [expr $x * .75 * 2] [expr $y * .75 * 2]
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
    global boardSize binPieces binPiecesOffsets

    if { [GS_WhoseMove $position] == 0 } {
	for {set i 1} {$i < [expr $boardSize / 2]} {incr i} {
	    $c lower moveI$i 
	    $c raise moveI[expr $boardSize - $i]
	}
    } else {
	for {set i 1} {$i < [expr $boardSize / 2]} {incr i} {
	    $c raise moveI$i 
	    $c lower moveI[expr $boardSize - $i]
	}
    }

    set board [unhash $position]

    for {set i 0} {$i < $boardSize} {incr i} {
	while { [lindex $binPieces $i] > 0 } {
	    deletePiece $c $i
	}
    }
    
    for {set i 0} {$i < $boardSize} {incr i} {
	$c itemconfigure counter$i -text "[lindex $board $i]"
	for {set j 0} {$j < [lindex $board $i]} {incr j} {	    	  
	    drawPiece $c $i
	}
    }

}

proc GS_ColorOfPlayers {} {
    return [list red blue]
}

# Unhashes POSITION and returns a list of length BOARDSIZE, with elt0 = bin0,
# elt1 = bin1, ..., eltBORDSIZE = binBOARDSIZE
proc unhash { position } {
    global boardSize numOfPieces turnOffset

    if { $position > $turnOffset } {
	set position [expr $position - $turnOffset]
    }

    set unhashed [rearranger_unhash $position]

    set result []
    set j 0
    set temp 0
    for {set i 0} {$i < [expr $boardSize + $numOfPieces]} {incr i} {
	if { [lindex $unhashed $i] == "o" } {
	    set temp [expr $temp + 1] 
	}
	if { [lindex $unhashed $i] == "x" || $i == [expr $boardSize + $numOfPieces - 1]} {
	    lappend result $temp
	    set j [expr $j + 1]
	    set temp 0
	}
    }
    return $result
}

# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
      GS_DrawPosition $c $position
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    global turnOffset
    if { $position < $turnOffset } {
	return 0
    } else { return 1 }
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global boardSize binPieces colorMove
    set boardList [unhash $oldPosition]
    set k [lindex $binPieces $theMove]
    if {[GS_WhoseMove $oldPosition] == 0} {
	set s 0
    } else { set s [expr $boardSize / 2] }

    for {set i [expr $theMove - 1]} {$k > 0} {set i [expr $i - 1]} {
	set k [expr $k - 1]
	if {$i == $s} {
	    set i [expr $i - 1]
	} 
	if {$i < 0} {
	    set i [expr $boardSize - 1]
	} 
	deletePiece $c $theMove
	drawPiece $c $i
	
	$c itemconfigure counter$theMove -text "[lindex $binPieces $theMove]"
	$c itemconfigure counter$i -text "[lindex $binPieces $i]"
	if {$colorMove == 1} { 	   
	    $c itemconfigure bin$i -fill cyan
	    $c itemconfigure bin$theMove -fill cyan
	}
	update idletasks
	after 200
	if {$colorMove == 1} { 	 
	    $c itemconfigure bin$i -fill white
	    $c itemconfigure bin$theMove -fill white
	}
    }
    GS_DrawPosition $c $newPosition
    update idletasks
    after 500
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

proc GS_ShowMoves { c moveType position moveList } {
    global binPieces
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		if {[lindex $binPieces $move] > 0} {
		set color green
		} else { set color cyan }
	    } else {
		set color red4
	    }
	}
	$c itemconfigure bin$move -fill $color
    }
    update idletasks
}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    global boardSize
    for {set i 0} {$i < $boardSize} {incr i} {
	$c itemconfigure bin$i -fill white
    }
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
    global boardSize binPieces colorMove
    set oldBoardList [unhash $currentPosition]
    set newBoardList [unhash $positionAfterUndo]
    set k [lindex $newBoardList $theMoveToUndo]

    if {[GS_WhoseMove $positionAfterUndo] == 0} {
	set s 0
    } else { set s [expr $boardSize / 2] }

    ### Redo Move
    GS_DrawPosition $c $positionAfterUndo

    for {set i [expr $theMoveToUndo - 1]} {$k > 0} {set i [expr $i - 1]} {
	set k [expr $k - 1]
	if {$i == $s} {
	    set i [expr $i - 1]
	} 
	if {$i < 0} {
	    set i [expr $boardSize - 1]
	} 
	deletePiece $c $theMoveToUndo
	drawPiece $c $i

	$c itemconfigure counter$theMoveToUndo -text "[lindex $binPieces $theMoveToUndo]"
	$c itemconfigure counter$i -text "[lindex $binPieces $i]"
    }
    update idletasks
    after 200

    # Figure out where the move ended
    set k [lindex $newBoardList $theMoveToUndo]

    for {set i [expr $theMoveToUndo - 1]} {$k > 0} {set i [expr $i - 1]} {
	set k [expr $k - 1]
	if {$i == $s} {
	    set i [expr $i - 1]
	} 
	if {$i < 0} {
	    set i [expr $boardSize - 1]
	} 
    }

    set k [lindex $newBoardList $theMoveToUndo]
    # Do it in reverse
    for {set i [expr $i + 1]} {$k > 0} {set i [expr $i + 1]} {
	set k [expr $k - 1]
	if {$i == $s} {
	    set i [expr $i + 1]
	} 
	if {$i == $boardSize} {
	    set i 0
	} 
	deletePiece $c $i
	drawPiece $c $theMoveToUndo
	
	$c itemconfigure counter$theMoveToUndo -text "[lindex $binPieces $theMoveToUndo]"
	$c itemconfigure counter$i -text "[lindex $binPieces $i]"
	if {$colorMove == 1} { 	   
	    $c itemconfigure bin$i -fill cyan
	    $c itemconfigure bin$theMoveToUndo -fill cyan
	}
	update idletasks
	after 200
	if {$colorMove == 1} {
	    $c itemconfigure bin$i -fill white
	    $c itemconfigure bin$theMoveToUndo -fill white
	}
	
    }

    GS_DrawPosition $c $positionAfterUndo
    update idletasks
    after 500
}





# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}




# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    global boardSize
    set result [unhash $position]
    if {[lindex $result 0] > [lindex $result [expr $boardSize / 2]]} {
	for {set i 0} {$i < 3} {incr i} {
	    $c itemconfigure bin0 -fill green
	    update idletasks
	    after 500
	    $c itemconfigure bin0 -fill white
	    update idletasks
	    after 500
	}
    } elseif {[lindex $result 0] < [lindex $result [expr $boardSize / 2]]} {
	for {set i 0} {$i < 3} {incr i} {
	    $c itemconfigure bin[expr $boardSize / 2] -fill green
	    update idletasks
	    after 500
	    $c itemconfigure bin[expr $boardSize / 2] -fill white
	    update idletasks
	    after 500
	}
    } else {
	for {set i 0} {$i < 3} {incr i} {
	    $c itemconfigure bin0 -fill yellow
	    $c itemconfigure bin[expr $boardSize / 2] -fill yellow
	    update idletasks
	    after 500
	    $c itemconfigure bin0 -fill white
	    $c itemconfigure bin[expr $boardSize / 2] -fill white
	    update idletasks
	    after 500
	}	
    }
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}

proc GS_Deinitialize { c } {   
}

proc rearranger_hash_init { boardsize num0s numXs } {
    global gHashBoardSize gHashMinMax
    lappend gHashMinMax $num0s
    lappend gHashMinMax $num0s
    lappend gHashMinMax $numXs
    lappend gHashMinMax $numXs
    set gHashBoardSize $boardsize
    nCr_init $boardsize
}

proc rearranger_unhash { hashed } {
    set dest []
    global gHashBoardSize gHashMinMax
    set j 0
    set boardsize $gHashBoardSize
    set numxs [lindex $gHashMinMax 3]
    set numos [lindex $gHashMinMax 1]
    for {set i 0} {$i < $gHashBoardSize} {incr i} {
	set t1 [nCr [expr $numxs + $numos] $numos]
	set t2 [nCr [expr $boardsize - 1] [expr $numxs + $numos]]
	set temp [expr $t1 * $t2]
	if { [expr $numxs + $numos] <= 0 } { 
	    lappend dest "b"
	} elseif { [expr $temp + [nCr [expr $numxs + $numos - 1] $numxs] * \
		 [nCr [expr $boardsize - 1] [expr $numxs + $numos - 1]]] \
		 <= $hashed } {
	    set hashed [expr $hashed - ($temp + [nCr [expr $numxs + $numos - 1] $numxs] * \
		     [nCr [expr $boardsize - 1] [expr $numxs + $numos - 1]])]
	    lappend dest "x"
	    set numxs [expr $numxs - 1]
	} elseif { $temp <= $hashed } {
	    set hashed [expr $hashed - $temp]
	    lappend dest "o"
	    set numos [expr $numos - 1]
	} else {
	    lappend dest "b"
	}
	set boardsize [expr $boardsize - 1]
    }
    return $dest
}

proc nCr_init { boardsize } {
    global gNCR
    set gNCR []
    for { set i 0 } { $i <= [expr ($boardsize + 1) * ($boardsize + 1)] } { incr i } {
	lappend gNCR 0
    }

    for { set i 0 } { $i <= $boardsize } { incr i } {
	set t1 [expr $i * ($boardsize + 1)]
	set t2 [expr $i * ($boardsize + 1) + $i]
	set gNCR [lreplace $gNCR $t1 $t1 1]
	set gNCR [lreplace $gNCR $t2 $t2 1]
    }

    for { set i 1 } { $i <= $boardsize } {incr i } {
	for { set j 1 } { $j < $i } { incr j } {
	    set t1 [expr ($i - 1) * ($boardsize + 1) + $j - 1]
	    set t2 [expr ($i - 1) * ($boardsize + 1) + $j]
	    set t3 [lindex $gNCR $t1]
	    set t4 [lindex $gNCR $t2]
	    set t5 [expr $t4 + $t3]
	    set gNCR [lreplace $gNCR [expr $i * ($boardsize + 1) + $j] \
			  [expr $i * ($boardsize + 1) + $j] $t5]
	}
    }
}

proc nCr { n r } {
    global gNCR gHashBoardSize
    if { [expr $n*($gHashBoardSize+1)+$r] < 0 } {
	puts "Yikes!"
    }
    set result [lindex $gNCR [expr $n * ($gHashBoardSize + 1) + $r]]
    return $result
}
