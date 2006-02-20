
####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################


# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.

proc GS_InitGameSpecific {} {
    
    #global constants
    global width height boardWidth boardHeight
    set boardWidth 500 
    set boardHeight 500
    set width 4
    set height 4

    #slotSize: the dimensions of each square on the board
    #cellPadding: the size of the gap between the edge of a piece and the edge of the slot
    #megaCellPadding: the size of the gap between the edge of one of the "place move" dots and the edge of the slot
    #arrowWidth: the width of the "slide move" arrows
    #mouseOverColor: the color of anything being moused over
    global slotSize cellPadding megaCellPadding arrowWidth mouseOverColor passButtonHeight passButtonWidth
    set slotSize(w) [expr $boardWidth / $width]
    set slotSize(h) [expr $boardHeight / $height]
    set cellPadding 10
    set megaCellPadding [expr 5 * $cellPadding]
    set arrowWidth [expr $slotSize(w) / 8]
    set mouseOverColor black
    set passButtonHeight [expr $slotSize(h) / 2]
    set passButtonWidth $slotSize(w)

    global xColor oColor
    set xColor blue
    set oColor red

    ### Set the name of the game
    
    global kGameName
    set kGameName "Queensland"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString ""
    set kMisereString ""

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: (fill in)"
    } else {
	SetToWinString "To Win: (fill in)"
    }
    
    ### Edit this string

    SetToMoveString "To Move: (fill in)"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Gamescrafters Team!"
    set kTclAuthors "(Fill this in)"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}

proc UnhashPosition {position} {
	global boardWidth boardHeight
	return [C_GenericUnhash $position [expr $boardWidth * $boardHeight]]

}

# Calls orderArrowsFrom every space on the board
proc orderAllArrows {c} {
    global width height
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
		orderArrowsFrom $c $i $j	
	}
    }
}

# Looks at all arrows originating from (i,j) and orders them so that the shortest is raised to the top
# and the longest is at the bottom.  Only considers arrows that represent "active" moves; that is,
# moves that are currently selectable by the player.
proc orderArrowsFrom {c i j} {
	global width height	
	set maxtag [expr ($width-1)*($width-1)+($height-1)*($height-1)]
	for {set p $maxtag} {$p >= 1} {set p [expr $p - 1]} {
		$c raise "i $i j $j p $p && active"
	}
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {

    return [list x o]

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
    global xColor oColor
    set xColor blue
    set oColor red
    return [list $xColor $oColor]
    
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

proc GS_Initialize { c } {

    global width height boardWidth boardHeight
    global slotSize cellPadding megaCellPadding arrowWidth mouseOverColor passButtonWidth passButtonHeight
    global xColor oColor
    global xPieces oPieces
    global placeMoves
    global slideStartLocs arrows
    global passButton
    global background

    #Everything starts out filled with dummy colors, but should be refilled before being used.
    set dummyDotColor pink
    set dummyArrowColor pink

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    $c configure -width $boardWidth -height [expr $boardHeight + $passButtonHeight]

    #draw the pieces
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} { $j < $height} {incr j} {
	    set xPieces($i,$j) [$c create oval \
				    [expr $i * $slotSize(w) + $cellPadding] \
				    [expr ($j+1) * $slotSize(h) - $cellPadding] \
				    [expr ($i+1) * $slotSize(w) - $cellPadding] \
				    [expr $j * $slotSize(h) + $cellPadding] \
				    -fill $xColor -tags [list xPieces]]
	    set oPieces($i,$j) [$c create oval \
				    [expr $i * $slotSize(w) + $cellPadding] \
				    [expr ($j+1) * $slotSize(h) - $cellPadding] \
				    [expr ($i+1) * $slotSize(w) - $cellPadding] \
				    [expr $j * $slotSize(h) + $cellPadding] \
				    -fill $oColor -tags [list oPieces]]
	}
    }

    #draw small circles for place moves
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
	    set placeMoves($i,$j) [$c create oval \
				       [expr $i * $slotSize(w) + $megaCellPadding] \
				       [expr ($j+1) * $slotSize(h) - $megaCellPadding] \
				       [expr ($i+1) * $slotSize(w) - $megaCellPadding] \
				       [expr $j * $slotSize(h) + $megaCellPadding] \
				       -fill $dummyDotColor]
	}
    } 
   
    #draw small circles for slide start locations and 
    #arrows for slide direction on slide moves
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
	    set slideStartLocs($i,$j) [$c create oval \
				       [expr $i * $slotSize(w) + $megaCellPadding] \
				       [expr ($j+1) * $slotSize(h) - $megaCellPadding] \
				       [expr ($i+1) * $slotSize(w) - $megaCellPadding] \
				       [expr $j * $slotSize(h) + $megaCellPadding] \
				       -fill $dummyDotColor]
	}
    } 
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
	    for {set k 0} {$k < $width} {incr k} {
		for {set l 0} {$l < $height} {incr l} {
		    if {([expr $k - $i] == 0 && [expr $l - $j] != 0) || \
			    ([expr $k - $i] != 0 && [expr $l - $j] == 0) || \
			    ([expr abs ([expr $k - $i])] == [expr abs ([expr $l - $j])] && [expr $k - $i] != 0)} {
			set arrows($i,$j,$k,$l) [$c create line \
						     [expr $i * $slotSize(w) + $slotSize(w) / 2] \
						     [expr $j * $slotSize(h) + $slotSize(h) / 2] \
						     [expr $k * $slotSize(w) + $slotSize(w) / 2] \
						     [expr $l * $slotSize(h) + $slotSize(h) / 2] \
						     -width $arrowWidth -arrow last \
						     -arrowshape [list [expr 2 * $arrowWidth] [expr 2 * $arrowWidth] $arrowWidth] \
						     -fill $dummyArrowColor -tags [list "i $i j $j p [expr ($i-$k)*($i-$k) + ($j-$l)*($j-$l)]"]]
		    }
		}
	    }
	}
    }

    #draw the pass button at the bottom
    set passButton [$c create rectangle [expr ([$c cget -width] / 2) - ($passButtonWidth / 2)] [expr [$c cget -height] - $passButtonHeight] \
	[expr ([$c cget -width] / 2) + ($passButtonWidth / 2)] [$c cget -height] -fill $dummyDotColor]

    #draw the background board and lines
    set background [$c create rectangle 0 0 [$c cget -width] [$c cget -height] -fill gray]
    for {set i 1} {$i <= $width} {incr i} {
	$c create line \
	    [expr $i * $slotSize(w)] 0 \
	    [expr $i * $slotSize(w)] $boardHeight \
	    -tags [list lines]
    }
    for {set j 1} {$j <= $height} {incr j} {
	$c create line \
	    0 [expr $j * $slotSize(h)]  \
	    $boardWidth [expr $j * $slotSize(h)] \
	    -tags [list lines]
    }
    
    #raise the backround so that the pieces and moves aren't visible
    $c raise $background
    $c raise lines
}

proc GS_Deinitialize { c } {
    $c delete all
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
    
    global xPieces oPieces width background
    set board [UnhashPosition $position]

    #raise the background to hide everything else
    $c raise $background
    $c raise lines

    for {set i 0} {$i < [string length $board]} {incr i} {
	set w [expr $i % $width]
	set h [expr $i / $width]
	if {[string index $board $i] == "X"} {
		$c raise $xPieces($w,$h)
	} elseif {[string index $board $i] == "O"} {
		$c raise $oPieces($w,$h)
	}
    }
}

# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}

# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
# Optional Procedure

proc GS_WhoseMove { position } {
	set X 0
	set O 0
	set board [UnhashPosition $position]
	for {set i 0} {$i < [string length $board]} {incr i} {
		if {[string index $board $i] == "X"} {
			incr X
		} elseif {[string index $board $i] == "O"} {
			incr O
		}
	}
	if {$X == $O} {
		return "x"
	} else {
		return "o"
	}
}

# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {

	### TODO: Fill this in
	GS_DrawPosition $c $newPosition
    
}

# ****************************************************************************
# This part of Queensland is a bit cryptic.  Essentially, the problem is that 
# every move has both a "slide" and a "place" component, but Gamesman isn't
# equipped to deal with multi-part moves.  In C, this problem is dealt with
# by representing the move as the triplet [source dest place].  This doesn't
# work so well in tcl/tk, because it is difficult for the player to visualize
# a move as a single arrow or dot.  Therefore, the move is divided up into
# two clicks.  This is accomplished by a sort of hack:
# 1) First, the arrows are drawn by DrawSlideMove
# 2) Each arrow is bound to an event that calls SetSlideComponent with
#    parameters specific to the arrow clicked
# 3) SetSlideComponent hides all the arrows and draws all of the dots with
#    DrawPlaceMoves.
# 4) Each dot is bound to a complete move (i.e. [source dest place]).  This
#    is possible because when each dot is drawn, it "remembers" the slide move
#    that was selected by the player.
# ****************************************************************************

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
	global width height passButton mouseOverColor
	set foundSlideMove false
	set currentPlayer [GS_WhoseMove $position]
	for {set i 0} {$i < [expr $width * $height]} {incr i} {
		for {set j 0} {$j < [expr $width * $height]} {incr j} {
			set slideMoveVals($i,$j) "Lose"
		}
	}
	# Set the values of each slide component.  Since each slide component corresponds to a handful of
	# actual moves (many moves can have the same slide component but different place components), the
	# value that each slide component is given is the BEST of all the possible moves it can lead to.
	# Think of this as a go-again, even though that's not how this is implemented is C.
	foreach move $moveList {
		set theMove [UnhashMove [lindex $move 0]]
		switch [lindex $move 1] {
			"Win" {
				set slideMoveVals([lindex $theMove 0],[lindex $theMove 1]) "Win"
			}
			"Tie" {
				if {$slideMoveVals([lindex $theMove 0],[lindex $theMove 1]) != "Win"} {
					set slideMoveVals([lindex $theMove 0],[lindex $theMove 1]) "Tie"
				}
			}
		}
		# 0,0 represents the player's choice not to slide a piece.  If a move starts with 0,0
		# then we need to somehow give the player the ability to signify that he doesn't want
		# to slide.  The "passButton" is used to represent a "null slide."
		if {[lindex $theMove 0] != 0 || [lindex $theMove 1] != 0} {
			DrawSlideMove $c $theMove $slideMoveVals([lindex $theMove 0],[lindex $theMove 1]) $moveType $moveList $currentPlayer $position
			set foundSlideMove true
		} else {
			$c raise $passButton
			$c itemconfig $passButton -fill [MoveValueToColor $moveType $slideMoveVals(0,0)]
			$c bind $passButton <ButtonRelease-1> "SetSlideComponent $c 0 0 $moveType [list $moveList] $currentPlayer $position"
			$c bind $passButton <Enter> "$c itemconfigure $passButton -fill $mouseOverColor"
			$c bind $passButton <Leave> "$c itemconfigure $passButton -fill [MoveValueToColor $moveType $slideMoveVals(0,0)]"
		}
	}
	# Raise shorter arrows on top of longer arrows to make them easier to click on.
	orderAllArrows $c
	# If there are absolutely no possible slide moves (for example, the beginning of the game when
	# you don't have any pieces on the board yet) then skip straight to the part where you get
	# to pick the place move, without having to first click on the passButton
	if {!$foundSlideMove} {
		SetSlideComponent $c 0 0 $moveType $moveList $currentPlayer $position
	}
	
}
#Unhashes a move.  The input is some number and the output is [source dest place].
proc UnhashMove { move } {
	return [list [GetMoveSource $move] [GetMoveDest $move] [GetMovePlace $move]]
}
proc GetMoveSource { move } {
	global width height
	return [expr $move / [expr $width * $width * $height * $height]]
}
proc GetMoveDest { move } {
	global width height
	return [expr [expr $move % [expr $width * $width * $height * $height]] / [expr $width * $height]]
}
proc GetMovePlace { move } {
	global width height
	return [expr $move % [expr $width * $height]]
}
proc DrawSlideMove { c theMove value moveType moveList currentPlayer position} {
	global width slideStartLocs arrows mouseOverColor
	set fromrow [expr [lindex $theMove 0] % $width]
	set fromcol [expr [lindex $theMove 0] / $width]
	set torow [expr [lindex $theMove 1] % $width]
	set tocol [expr [lindex $theMove 1] / $width]
	#$c raise slideStartLocs($fromrow,$fromcol)
	$c raise $arrows($fromrow,$fromcol,$torow,$tocol)
	$c itemconfig $arrows($fromrow,$fromcol,$torow,$tocol) -fill [MoveValueToColor $moveType $value]
	$c bind $arrows($fromrow,$fromcol,$torow,$tocol) <ButtonRelease-1> \
		"SetSlideComponent $c [lindex $theMove 0] [lindex $theMove 1] \
		$moveType [list $moveList] $currentPlayer $position"
	$c bind $arrows($fromrow,$fromcol,$torow,$tocol) <Enter> \
		"$c itemconfigure $arrows($fromrow,$fromcol,$torow,$tocol) -fill $mouseOverColor; \
		$c raise $arrows($fromrow,$fromcol,$torow,$tocol)"
	$c bind $arrows($fromrow,$fromcol,$torow,$tocol) <Leave> \
		"$c itemconfigure $arrows($fromrow,$fromcol,$torow,$tocol) -fill [MoveValueToColor $moveType $value]; \
		orderArrowsFrom $c $fromrow $fromcol"
	$c addtag "active" withtag $arrows($fromrow,$fromcol,$torow,$tocol)
}
proc SetSlideComponent {c source dest moveType moveList currentPlayer position} {
	global xPieces oPieces width
	GS_HideMoves $c $moveType $position $moveList
	if {$currentPlayer == "x" && ($source != 0 || $dest != 0)} {
		$c lower $xPieces([expr $source % $width],[expr $source / $width])
		$c raise $xPieces([expr $dest % $width],[expr $dest / $width])
	} elseif {$currentPlayer == "o" && ($source != 0 || $dest != 0)} {
		$c lower $oPieces([expr $source % $width],[expr $source / $width])
		$c raise $oPieces([expr $dest % $width],[expr $dest / $width])
	}
	foreach move $moveList {
		set theMove [UnhashMove [lindex $move 0]]
		if {[lindex $theMove 0] == $source && [lindex $theMove 1] == $dest} {
			DrawPlaceMove $c $theMove [lindex $move 1] $moveType $source $dest
		}
	}
}
proc DrawPlaceMove { c theMove value moveType source dest } {
	global width placeMoves mouseOverColor
	set row [expr [lindex $theMove 2] % $width]
	set col [expr [lindex $theMove 2] / $width]
	$c raise $placeMoves($row,$col)
	$c itemconfig $placeMoves($row,$col) -fill [MoveValueToColor $moveType $value]
	$c bind $placeMoves($row,$col) <Enter> "$c itemconfigure $placeMoves($row,$col) -fill $mouseOverColor"
	$c bind $placeMoves($row,$col) <Leave> "$c itemconfigure $placeMoves($row,$col) -fill [MoveValueToColor $moveType $value]"
	$c bind $placeMoves($row,$col) <ButtonRelease-1> "SetPlaceComponent $source $dest [expr $row + ($col * $width)]"
}
proc SetPlaceComponent {source dest place} {
	global width height
	set move [expr $source * $width * $width * $height * $height]
	set move [expr $move + ($dest * $width * $height)]
	set move [expr $move + $place]
	ReturnFromHumanMove $move
}

# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {

    $c dtag "all" "active"
    # DrawPosition raises the board (therefore hiding all the moves) and raises all the pieces on it.
    GS_DrawPosition $c $position

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

	### TODO if needed
    GS_DrawPosition $c positionAfterUndo
}

# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}

# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {

	### TODO if needed
	
}

# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {

	### TODO if needed

}

