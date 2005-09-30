####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

global canvasWidth canvasHeight
set canvasWidth  500
set canvasHeight 500


# Board dimensions

global height width
set height 5
set width 3

global numPositions numSlots numPieces

#################################
#
# Variables for drawing the board
#
#################################

global base

global pieceGap vertGap horizGap 
global tileSize
set pieceGap 5
set vertGap 50
set tileSize [expr ($canvasHeight - 2*$vertGap)/$height]
set horizGap [expr ($canvasWidth - $width*$tileSize)/2]

global arrowLength lineWidth
set arrowLength 20
set lineWidth 2

global xColor oColor lineColor baseColor
set xColor red
set oColor blue
set lineColor CadetBlue4
set baseColor lightgrey

# Arrow types

global leftArrow rightArrow upArrow downArrow
set leftArrow 0
set rightArrow 1
set upArrow 2
set downArrow 3

global X O
set X 1
set O 0


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
    global height width
    global numSlots numPieces
    global numPositions
   
    ### Set the name of the game
    
    global kGameName
    set kGameName "Lewthwaite's Game"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set stdStr "To Win: have your opponent be "
    set misStr "To Win: be "
    set gameEnd "the first player who has no moves "
    set kStandardString [concat $stdStr $gameEnd]
    set kMisereString [concat $misStr $gameEnd]

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
   
    if {!$gMisereGame} {
	SetToWinString $kStandardString
    } else {
	SetToWinString $kMisereString
    }
    SetToMoveString "To Move: Choose one of your pieces to the left, right, \
                     top, or bottom of the empty space to move into the \
                     space. (With standard rules, you can only move one \
                     piece at a time.)"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Yuliya Sarkisyan, Cindy Song"
    set kTclAuthors "Yuliya Sarkisyan, Cindy Song"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    set numSlots $width*$height
    set numPieces [expr ($numSlots - 1)/2]
    set numPositions [expr 2 * [factorial $numSlots]\
			  /[factorial $numPieces]\
			  /[factorial $numPieces]\
			  /[factorial [expr $numSlots - 2*$numPieces]]]

    set numPositions 102960
    set gInitialPosition 31193
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
    return [list X O]
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

    set multipleMoves \
	[list \
	     "Would you like to be able to slide multiple pieces:" \
	     "No" \
	     "Yes" \
	     ]

    # List of all rules, in some order
    set ruleset [list $standardRule $multipleMoves]

    # Declare and initialize rule globals
    global gMisereGame multiplePieceMoves
    set gMisereGame 0
    set multiplePieceMoves 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "multiplePieceMoves"]

    global kLabelFont
    set ruleNum 0
    foreach rule $ruleset {
	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] \
	    -font $kLabelFont
	pack $rulesFrame.rule$ruleNum.label -side left
	set rulePartNum 0
	foreach rulePart [lrange $rule 1 end] {
	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum \
		-text $rulePart \
		-variable [lindex $ruleSettingGlobalNames $ruleNum] \
		-value $rulePartNum -highlightthickness 0 -font $kLabelFont
	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left \
		-expand 1 -fill both
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
    global gMisereGame multiplePieceMoves
    set option 0
    set option [expr $option + $gMisereGame + 2*$multiplePieceMoves]
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
    global gMisereGame multiplePieceMoves
    set option 0
    set gMisereGame [expr $option%2]
    set multiplePieceMoves [expr ($option/2)%2]
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
    
    global canvasWidth canvasHeight
    $c configure -width $canvasWidth -height $canvasHeight
    makeBoard $c
 
    # create font for Winner text at end of game
    # font create Winner -family arial -size 50
   
} 





#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why 
# this is here, so whoever put this here should update this.  -Jeff
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
# BY THE WAY: Before you go any further, I recommend writing a tcl function 
# that UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {
    
    global height width numSlots
    
    set boardList [C_GenericUnhash $position $numSlots]
    set o 'O'
    set x 'X'


    $c raise base all
    for {set i 0} {$i < $height} {incr i} {
	for {set j 0} {$j < $width} {incr j} {
	    set piece [string index $boardList [expr $i*$width + $j]]
	    if { [string compare $piece "O"] == 0 } {
		$c raise o-$i-$j base
	    } elseif { [string compare $piece "X"] == 0} {
		$c raise x-$i-$j base
	    }
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
    GS_DrawPosition $c $position
    global O
    animateMove $O $O-1-1 [list 1 1] [list 1 2] $c
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
    
}


#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever 
# the player clicks to make the move)  It is also the function that handles 
# coloring of the moves according to value. It is called by gamesman just 
# before the player is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which 
#    radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

    ### TODO: Fill this in
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in

}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to 
# position B then an undo is called
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
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner \
		       lastMove} {

	### TODO if needed
	
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is 
# finished.  This is provided so that you may undo the drawing you did in 
# GS_GameOver if you drew something. For instance, if you drew a line crossing 
# out the winning row in tic tac toe, this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end 
# of the game, so IF you choose to do nothing in GS_GameOver, you needn't do 
# anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

	### TODO if needed

}



############################## HELPER FUNCTIONS #############################

###################
#
# DRAWING THE BOARD
#
###################


#############################################################################
# Draws all of the pieces, arrows, etc. on the board and hides them under the 
# base.
#############################################################################
proc makeBoard { c } {
    global lineWidth lineColor baseColor base
    global canvasWidth canvasHeight

    global height width 
    global vertGap horizGap 
    global tileSize
    global leftArrow rightArrow upArrow downArrow
    global X O

    set base \
	[$c create rectangle \
	     0 \
	     0 \
	     [expr $canvasWidth - 1] \
	     [expr $canvasHeight - 1] \
	     -fill $baseColor -tag base];


    # create horizontal lines
    for {set i 0} {$i <= $height } {incr i} {
	set horiz-$i  \
	    [$c create line \
		 $horizGap \
		 [expr $vertGap + $i * $tileSize] \
		 [expr $canvasWidth - $horizGap] \
		 [expr $vertGap + $i * $tileSize] \
		 -fill $lineColor -width $lineWidth -tags [list base line]]; 

	
    }

    # create vertical lines
    for {set i 0} {$i <= $width} {incr i} {
	set vert-$i  \
	    [$c create line \
		 [expr $horizGap + $i * $tileSize] \
		 $vertGap \
		 [expr $horizGap + $i * $tileSize] \
		 [expr $canvasHeight - $vertGap] \
		 -fill $lineColor -width $lineWidth -tags [list base line]];
	
    }

    # create pieces
    for {set i 0} {$i < $height} {incr i} {
	for {set j 0} {$j < $width} {incr j} {
	    createPiece $c $X $i $j piecex-$i-$j
	    createPiece $c $O $i $j pieceo-$i-$j
	}
    }

    # create arrows

    for {set i 0} {$i < $height} {incr i} {
	for {set j 0} {$j < $width} {incr j} {
	    if { $i > 0 } { drawArrow $c $downArrow $i $j down-$i-$j }
	    if { $i < $height - 1 } { drawArrow $c $upArrow $i $j up-$i-$j }
	    if { $j < $width - 1 } {drawArrow $c $rightArrow $i $j right-$i-$j}
	    if { $j > 0 } { drawArrow $c $leftArrow $i $j left-$i-$j }
	}
    }	 

    ###  raise the base over the pieces
    #$c lower piece all
    #$c lower arrow all
    #$c raise base all

} 


#############################################################################
# Draws an arrows on canvas c in slot (i,j). Type is one of leftArrow, 
# rightArrow, upArrow, downArrow.
#############################################################################
proc drawArrow { c type i j name} {
    global height width 
    global vertGap horizGap 
    global tileSize
    global arrowLength leftArrow rightArrow upArrow downArrow
    global lineWidth

    if { $type == $leftArrow } {
	set $name [$c create line \
		       [expr $horizGap + $j * $tileSize] \
		       [expr $vertGap + $i * $tileSize + $tileSize/2] \
		       [expr $horizGap + $j * $tileSize + $arrowLength] \
		       [expr $vertGap + $i * $tileSize + $tileSize/2] \
		       -tags [list $name arrow]];

    } 
    if { $type == $rightArrow } {
	set $name [$c create line \
		       [expr $horizGap + ($j+1) * $tileSize] \
		       [expr $vertGap + $i * $tileSize + $tileSize/2] \
		       [expr $horizGap + ($j+1) * $tileSize - $arrowLength] \
		       [expr $vertGap + $i * $tileSize + $tileSize/2] \
		       -tags [list $name arrow]];
    }
    if { $type == $upArrow } {
	set $name [$c create line \
		       [expr $horizGap + $j * $tileSize + $tileSize/2] \
		       [expr $vertGap + ($i+1) * $tileSize] \
		       [expr $horizGap + $j * $tileSize + $tileSize/2] \
		       [expr $vertGap + ($i+1) * $tileSize - $arrowLength] \
		       -tags [list $name arrow]];
    }
    if { $type == $downArrow } {
	set $name [$c create line \
		       [expr $horizGap + $j * $tileSize + $tileSize/2] \
		       [expr $vertGap + $i * $tileSize] \
		       [expr $horizGap + $j * $tileSize + $tileSize/2] \
		       [expr $vertGap + $i * $tileSize + $arrowLength] \
		       -tags [list $name arrow]];
    }
   
    $c itemconfig $name -width [expr $lineWidth * 1] -arrow last \
	-arrowshape {20 20 20}
    set movebind [expr 1 + 11]
    $c bind $name <ButtonRelease-1> "ReturnFromHumanMove $movebind"
    $c bind $name <Enter> "SetColour $c $name black"
}


#############################################################################
# Draws a piece on canvas c in slot (i,j). Whose is one of X, O.
#############################################################################
proc createPiece {c whose i j name} {
    global X O
    global horizGap vertGap pieceGap tileSize
    global xColor oColor
    
    if { $whose == $X } {
	set $name \
	    [$c create rect \
		 [expr $horizGap + $j * $tileSize + $pieceGap] \
		 [expr $vertGap + $i * $tileSize + $pieceGap] \
		 [expr $horizGap + ($j+1) * $tileSize - $pieceGap] \
		 [expr $vertGap + ($i+1) * $tileSize - $pieceGap] \
		 -fill $xColor -tags [list piece x-$i-$j $X-$i-$j $name]];
    }
    
    if { $whose == $O } {
	set $name \
	    [$c create rect \
		 [expr $horizGap + $j * $tileSize + $pieceGap] \
		 [expr $vertGap + $i * $tileSize + $pieceGap] \
		 [expr $horizGap + ($j+1) * $tileSize - $pieceGap] \
		 [expr $vertGap + ($i+1) * $tileSize - $pieceGap] \
		 -fill $oColor -tags [list piece o-$i-$j $O-$i-$j $name]];
    }

    return $name
}
###########
#
# ANIMATION
#
###########


#######################################################
# animateMove
# arg1: whoseTurn
# arg2: the tag of the piece to be moved
# arg3; the board position number from which the piece is being moved (1-9)
# arg4; the board position number to which to move the piece (1-9)
# arg5: the canvas
# pieceToMove is on the board already, so we create a piece at the same spot
#   lower the previous piece, slide the created piece over, raise the 
#    piece at $whoseTurn-$to, and destroy the temporary piece
#
#######################################################
proc animateMove { whoseTurn pieceToMove from to c } {
    global oColor xColor pieceSize pieceOutline base

    #$c lower piece all
    
    createPiece $c $whoseTurn [lindex $from 0] [lindex $from 1] temp
   
    $c lower $pieceToMove base
    $c raise temp base

    slideAnimation temp $from $to $c

    $c raise $whoseTurn-[lindex $to 0]-[lindex $to 1] base
    $c delete temp

}


#slideAnimation:
# pieceToMove  the tag of the piece to be moved
# from         the board position number from which the piece is being moved 
#              (1-9)
# to           the board position number to which to move the piece (1-9)
# c            c the canvas
#
# pieceToMove is on the board already, so we need to slide animate it, then 
# put it back under the board in the correct spot, and finally raise the piece 
# at position $to, so that in the end we don't have two pieces of the same 
# color at the same position.
#
# 1) find the distance needed to travel:
#   a. find the difference between the "from" x coord, and the "to" x coord  
#   b. do the same for the 'from' y and 'to' y
# 2) There are two cases:
#   a. the piece slides horizontally, so y is fixed, i.e. yDist == 0
#      1) the piece is sliding left so 
#   b. the piece slides vertically, so x is fixed, i.e. xDist == 0


#this implementation assumes can move things with float values
# fortunately, you can.
# unfortunately, i'm not sure if you can set the locations of pieces, so that
# might be a problem.  first design:
# determine actual time of animation, have global setting how many frames per
# second.  paint a frame, and wait until the clock is greater than or equal to
# the number of the frame you should be at. (okay to be slower)
proc slideAnimation { pieceToMove from to c} {
    global tileSize gAnimationSpeed
    #i'm itching to make this system-independent, but to do that i *need* to
    #know how many clicks per second the clock goes through!
    #I'll assume for now it's like java and [clock clicks] returns the system
    #time in milliseconds. (i'll adjust it later)

    #clicks of animation time. 1000 is the default.
    set animDuration [ScaleDownAnimation 1000]
    #how many frames to paint in this time? 15 is the default
    #(re-using AdjustedTimeOfAnimation is a hack, but heuristically I want the
    #same thing to happen to the number of frames that happened to
    #animDuration.  i.e. if the time of animation went up, so should the number
    #of frames.)
    set numFrames [ScaleDownAnimation 15]
    #how much time in between painting?
    set clicksPerFrame [expr $animDuration / $numFrames]

    #starting and ending screen coordinates
    set xDist [expr ([lindex $to 0] - [lindex $from 0]) * $tileSize]
    set yDist [expr ([lindex $to 1] - [lindex $from 1]) * $tileSize]
    set xPlus [expr $xDist / $numFrames]
    set yPlus [expr $yDist / $numFrames]

    #store time so can determine what frame we should be on.  this is only
    #relevant in that we can use it to determine how long the thread should
    #wait after painting a frame
    #EX: after [expr int($gMoveDelay * 1000)]
    set currentTime [clock clicks]
    set endTime [expr $currentTime + $animDuration - $clicksPerFrame]

    #algorithm: move the piece by xPlus/yPlus, then wait the number of clicks
    #until this frame expires
    for {} {$currentTime < $endTime} \
	{set currentTime [expr $currentTime + $clicksPerFrame]} {
	#in java, i'd increment elapsed clicks by the actual amount of time
	#that passed, but then i'd be able to actually set the piece's
	#location, so can't do that here. (at least, not with my meager
	#knowledge of Tcl)
	$c move $pieceToMove $xPlus $yPlus
	update idletasks

	#check if must wait until ready for next frame
	set waitClicks [expr $currentTime + $clicksPerFrame - [clock clicks]]
 	if {$waitClicks > 0} {
	    after $waitClicks
	}
    }
}


###############
#
# OTHER HELPERS
#
###############

proc SetColour { c obj colour } {
    $c itemconfig $obj -fill $colour
}

proc factorial { n } {
    if { $n == 0 || $n == 1 } {
	return 1
    }
    return [expr $n * [factorial [expr $n - 1]]]
} 