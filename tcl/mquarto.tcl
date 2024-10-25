#############################################################################
##
## NAME:			Xmquarto
##
## DESCRIPTION: 	Quarto
## AUTHOR:       	Amy Hsueh, Yanpei Chen, Mario Tanev
## DATE:         	2005-11-21
## UPDATE HISTORY:
##
##
## 26 Nov 2005 Amy: file created 
## 28 Nov 2005 Amy: attempted to bind moves, don't think i can do anything else 
## without writing unhash first
#############################################################################


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
    set kGameName "Quarto"
    
    ### Set default gameDimension, currently constant and 3 dimensions
    global gameDimension
    set gameDimension 2
    
    ### Set HAND slot
    global hand
    set hand 0
    
    ### Set NumPieces, number of pieces total
    global numPieces
    set numPieces [expr int(pow(2,$gameDimension))]
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0 
    # initial position is the empty board by default
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ______ WINS" 
    set kMisereString "First player to ______ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Line up $gameDimension pieces in a row that all have at least one characteristic in common. The rows can be vertical, horizontal, or diagonal."
    } else {
	SetToWinString "To Win: (fill in)" # not used
    }
    SetToMoveString "To Move: On your turn, if it is the first move, choose a piece to place in the hand slot. If it is not the first move, place the piece in the hand slot into an empty position on the board, and then choose another piece to put in the hand slot if there are more pieces available." 
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Yanpei CHEN, Amy HSUEH, Mario TANEV"
    set kTclAuthors "Yanpei CHEN, Amy HSUEH, Mario TANEV"
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

    ### FILL THIS IN
    return [list black white]
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

    global bColor wColor
   	set bColor black
   	set wColor white
   	
   	# quarto is impartial
    return [list white white]
    
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


    # List of all rules, in some order
    set ruleset [list $standardRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0
    
    
    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" ]
    
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
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
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
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
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

    ########### graphics constants ##########################################
    global canvasWidth canvasHeight

	global gameDimension
	# Set canvas size based on canvas given to us
    set canvasWidth [$c cget -width]
    set canvasHeight [$c cget -height]
    
    # size of the board within the canvas
    global boardWidth canvasWidth
    set boardWidth [expr $canvasWidth/($gameDimension+1)*($gameDimension)]
    set boardHeight [expr $canvasHeight/($gameDimension+1)*($gameDimension)]
    
    global slotSize cellPadding megaCellPadding arrowWidth
    set slotSize(w) [expr $boardWidth / $gameDimension]
    set slotSize(h) [expr $boardHeight / $gameDimension]
    
    global dotExpandAmount dotSize
    set dotExpandAmount [expr ($slotSize(h)+$slotSize(w))/18]
    set dotSize(w) [expr $slotSize(w)/9]
    set dotSize(h) [expr $slotSize(h)/9]
    
    #set cellPadding 10
    #set megaCellPadding [expr 3 * $cellPadding]
    #set arrowWidth [expr $slotSize(w) / 8]
    
    # border for unused pieces slot
    set border [expr ($slotSize(w)+$slotSize(h))/100]
    # font for label
    font create labelFont -family Helvetica -size [expr int(floor($slotSize(h)/7))] 
    ############################## board #######################################
    
    # draw the board and lines
    
    # board
    set board [$c create rectangle 0 0 $boardWidth $boardHeight -fill gray -tag board]
    # grids
    for {set i 1} {$i < $gameDimension} {incr i} {
	$c create line \
	    [expr $i * $slotSize(w)] 0 \
	    [expr $i * $slotSize(w)] $boardHeight \
	    -tags lines
    }
    for {set j 1} {$j < $gameDimension} {incr j} {
	$c create line \
	    0 [expr $j * $slotSize(h)]  \
	    $boardWidth [expr $j * $slotSize(h)] \
	    -tags lines
    }
    
    # Hand slot
    set hand [$c create rectangle $boardWidth 0 [expr $slotSize(w) + $boardWidth] $slotSize(h) -fill pink -tag base] 
    set handLabel [$c create text [expr $slotSize(w)/2 + $boardWidth] [expr $slotSize(h)/6] -font labelFont -text "Hand:" -tag base] 
    
    # Unused Pieces Container
    set x1 [expr $slotSize(w)/2 + $border]
    set y1 $boardHeight
    set x2 [expr $slotSize(w) + $boardWidth + $border]
    set y2 [expr $boardHeight + $slotSize(h)]
    set unusedContainer [$c create oval $x1 $y1 $x2 $y2	-fill lightGreen -width $border]
   	set ContainerLabel [$c create text [expr ($slotSize(w)/2 + $slotSize(w) + $boardWidth)/2] \
   					   [expr $boardHeight + $slotSize(h)/6] \
   					   -font labelFont -text "Pieces Left:"] 
   	# Unused Pieces				   
 	global unusedPieces numPieces
 	# font for pieces
 	global unusedPiecesFont unusedPiecesFontExpand
 	font create unusedPiecesFont -family Helvetica -size [expr int(floor($slotSize(h)/9))] 
 	font create unusedPiecesFontExpand -family Helvetica -size [expr int(floor($slotSize(h)/5))] 
   
 	# the first (2^gamedimension)/2 = [expr pow(2,$gameDimension-1)]
 	# for 4-d, i iterates 0-7
 	
 	for {set i 0} {$i < [expr $numPieces/2]} {incr i 1} {
 		set xPos [expr int($slotSize(w)) + $i*$slotSize(w)/7*3]
 		drawUnusedPiece $i $xPos [expr int($y1 + $slotSize(h)/3)] $c
 	}
 	# 8-16
 	for {set i [expr $numPieces/2]} {$i < $numPieces} {incr i 1} {
 		set xPos [expr int($slotSize(w)) + ($i-$numPieces/2)*$slotSize(w)/7*3]
 		drawUnusedPiece $i $xPos [expr int($y1 + $slotSize(h)/3*2)] $c
 	}
 	# bind these pieces to command
 	for {set i 0} {$i < $numPieces} {incr i 1} {
 		$c bind $unusedPieces($i) <Enter> "MouseOverExpandText $unusedPieces($i) $c"
 		$c bind $unusedPieces($i) <Leave> "MouseOutContractText $unusedPieces($i) $c"
 		# drag
		$c bind $unusedPieces($i) <Button-1> {CanvasMark %x %y %W}
		$c bind $unusedPieces($i) <B1-Motion> {CanvasDrag %x %y %W}
 	}
 	
 	global dots
 	# draw dots
 	for {set i 0} {$i < $gameDimension} {incr i 1} {
 		for {set j 0} {$j < $gameDimension} {incr j 1} {
 			set dots($i,$j) [drawOval $c \
 							[expr ($slotSize(w)/2 + $i*$slotSize(w))] \
 							[expr ($slotSize(h)/2 + $j*$slotSize(h))] \
 							$dotSize(w) $dotSize(h) \
 							dots pink pink]
 			$c bind $dots($i,$j) <Enter> "MouseOverExpand $dots($i,$j) $c"
 			$c bind $dots($i,$j) <Leave> "MouseOutContract $dots($i,$j) $c"
 		}
 	}
 	$c lower dots all
 	#$c lower unusedPieces all
    #$c raise lines all
  
}	
#############################################################
################### HELPER FUNCTIONS ########################
# the built in create oval function is HORRIBLE! HORRIBLE!!
# forgot what the radiousX and radiusY are called...
proc drawOval { c x y radiusX radiusY theTag theColor outlineColor} {
	set x1 [expr $x - $radiusX]
	set y1 [expr $y - $radiusY]
	set x2 [expr $x + $radiusX]
	set y2 [expr $y + $radiusY]
	return [$c create oval $x1 $y1 $x2 $y2 -fill $theColor -outline $outlineColor -tag $theTag]
}
# pieces in quarto works like: piece is a short with 4 bits,
# rightmost bit is trait 0, then 1, 2, 3... etc
proc drawUnusedPiece { piece x y c } {
	# an array for unusedPieces handles
	global unusedPieces
 	global unusedPiecesFont
 	
	set bits [intToBits $piece]
	set unusedPieces($piece) [$c create text $x $y -text $bits -font unusedPiecesFont -tags unusedPieces]
}
# int to bits
# returns a bit string given int
proc intToBits {x} {
	global gameDimension
	set ans ""
	# loops through traits
	for {set i 0} {$i < $gameDimension} { incr i 1} {
		set tmp [cconcat $ans [expr ($x >> $i) & 1]]
		set ans $tmp
	}
	return $ans
}
proc cconcat args {join $args ""}
proc CanvasMark {x y w} {
	global canvas
	set canvas($w,obj) [$w find closest $x $y]
	set canvas($w,x) $x
	set canvas($w,y) $y
}
proc CanvasDrag {x y w} {
	global canvas
	set dx [expr $x - $canvas($w,x)]
	set dy [expr $y - $canvas($w,y)]
	$w move $canvas($w,obj) $dx $dy
	
    global unusedPiecesFontExpand
    $w itemconfigure $canvas($w,obj) -font unusedPiecesFontExpand
	set canvas($w,x) $x
	set canvas($w,y) $y
}
#### MouseOverExpandText
proc MouseOverExpandText { obj c } {
    global unusedPiecesFontExpand
    $c itemconfigure $obj -font unusedPiecesFontExpand
}

#### MouseOutContractText
proc MouseOutContractText { obj c } {
	global unusedPiecesFont
    $c itemconfigure $obj -font unusedPiecesFont
}
#### MouseOverExpand
proc MouseOverExpand { obj c } {
    global dotExpandAmount
    $c itemconfig $obj -width $dotExpandAmount
}

#### MouseOutContract
proc MouseOutContract { obj c } {
    $c itemconfigure $obj -width 0
}

#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
    #delete fonts??
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
    
    ### TODO: Fill this in

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

	global unusedPieces hand moves
	#$c raise base all
	### TODO: Fill this in
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
						set color red
					}
				}
		set piece [getPiece $move]
		set slot [getSlot $move]
		
		if { $slot == $hand } {
			$c bind $unusedPieces($piece) <ButtonRelease-1> "putIntoHand $c $piece"
		}
		
		#$c raise UnusedPieces($piece) all
		set moves($piece,$slot) $move
		
	}
	global currentMove
	set currentMove(piece) -1
	set currentMove(slot) -1
	#$c raise unusedPieces all
	#$c raise base all 
	#$c raise lines all
}
############################## MOVE ##########################################
### how to get pieces and places from MOVE
# the C code:
#/* Creates move given slot and piece */
#MOVE CreateMove( MOVE slot, MOVE piece ) {	
#    return slot + ( piece << ( GAMEDIMENSION + 1 ) );
#}
# slot is from 0 to NUMPIECES
### GetSlot
proc getSlot { move } {
	global numPieces
	return [expr $move & $numPieces]
}
### GetPiece
proc getPiece { move } {
	global gameDimension
	set piece [expr $move >> ($gameDimension+1)]
	return $piece
}
### createMove
### the C code equivalent, makes Move from slot and piece
### not used
#proc createMove { slot piece } {
#	global gameDimension
#	return [expr $slot + ( $piece << ($gameDimension+1))]
#}
########################### ReturnFromHumanMove ################################
### markSlot

### markPiece
### gets 
### putIntoHand puts the piece into the hand
### currentMove stores part of the current move
proc putIntoHand { c piece } {
	global currentMove unusedPieces boardWidth slotSize hand
	drawHandSlotPiece $c $piece
	$c lower unusedPieces all
	set currentMove(slot) $hand
	set currentMove(piece) $piece
	$c create text 20 20 -text $hand$piece
	returnMoveIfNeeded 
}
proc drawHandSlotPiece { c piece } {
	global inHand slotSize boardWidth
	set bits [intToBits $piece]
	set inHand($piece) [$c create text [expr $boardWidth + $slotSize(w)/2] [expr $slotSize(h)/2] \
										 -text $bits -font unusedPiecesFont -tags inHand]
}
proc returnMoveIfNeeded {} {
	global currentMove moves
	if {[expr ($currentMove(slot) != -1) && ($currentMove(piece) != -1)]} {
		bell
		ReturnFromHumanMove $moves($currentMove(piece),$currentMove(slot))
	} else {
		return 
	}
}	
	
#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
	$c lower inHand all
    ### TODO: Fill this in

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
