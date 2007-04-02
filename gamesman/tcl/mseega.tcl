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

global initialized
set initialized false

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Seega"
    
    global Winner initialized
    if { $initialized == false } {
	font create Winner -family arial -size 50    
    }
    set initialized true
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ______ WINS"
    set kMisereString "First player to ______ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Kill oponant's pieces by trapping them with yours"
    } else {
	SetToWinString "To Win: Let oponant kill your pieces by trapping them with his/her pieces"
    }
    SetToMoveString "To Move: (Click on a dot to place a piece, click on an arrow to slide a piece)"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "(Fill this in)"
    set kTclAuthors "Emad Salman"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    global BOARDARRAYSIZE colorX colorO
    set colorX blue
    set colorO red
    
    global WIDTH HEIGHT BOARDSIZE counter
    set WIDTH 3
    set HEIGHT 3
    set BOARDSIZE 9
    set BOARDARRAYSIZE [expr $BOARDSIZE + 2]
    set counter 0
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

proc min { a b } {
    if {$a < $b} {
	return $a
    }
    return $b
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
    
    global blockSize WIDTH BOARDSIZE boardWidth boardHeight gFrameWidth gFrameHeight
    global firstHalfAlreadyAnimated
    global initialized

    set HEIGHT [expr $BOARDSIZE/$WIDTH]
    set firstHalfAlreadyAnimated 0

    set preferredHeight [expr int([min $gFrameWidth $gFrameHeight])]
    set preferredWidth [expr int([min $gFrameWidth $gFrameHeight])]
    
    # The label has been removed due to an error in GAMESMAN
    # set labelSize 50.0
    set labelSize 0.0

    set widthLimit [expr $preferredWidth/$WIDTH]
    set heightLimit [expr ($preferredHeight-$labelSize)/$HEIGHT]

    if {$heightLimit < $widthLimit} {
	set blockSize $heightLimit
    } else {
	set blockSize $widthLimit
    }

    set boardWidth [expr $blockSize * $WIDTH]
    set boardHeight [expr $blockSize * $HEIGHT]

    $c delete all

    # draw base
    $c configure -width [expr $blockSize*$WIDTH] -height [expr $blockSize*$HEIGHT+$labelSize]
    $c create line 0 [expr ($blockSize*$HEIGHT+$labelSize)/2] [expr $blockSize*$WIDTH] [expr ($blockSize*$HEIGHT+$labelSize)/2] -width [expr $blockSize*$HEIGHT+$labelSize] -fill grey50 -tag base

    # draw vertical gridlines
    for {set i 1} {$i <= $WIDTH} {incr i} {
	$c create line [expr $boardWidth*$i/$WIDTH] 0 [expr $boardWidth*$i/$WIDTH] $boardHeight -width 2 -tag base
    }

    # draw horizontal gridlines
    for {set i 1} {$i <= $HEIGHT} {incr i} {
	$c create line 0 [expr $boardHeight*$i/$HEIGHT] $boardWidth [expr $boardHeight*$i/$HEIGHT] -width 2 -tag base
    }

    # board outline
    $c create line 0 0 $boardWidth 0 -width 2 -tag base 
    $c create line 0 0 0 $boardHeight -width 2 -tag base 
    $c create line 0 $boardHeight $boardWidth $boardHeight -width 2 -tag base 
    $c create line $boardWidth 0 $boardWidth $boardHeight -width 2 -tag base 

    #MakePieces $c 0
    #MakeArrows $c 0
    #$c raise base all

    # The winner indicator does not function correctly due to a bug in GAMESMAN 
    # font create ourArial -family arial -size 35 
    # $c create line 0 [expr $boardHeight+25] $boardWidth [expr $boardHeight+25] -width 50 -fill grey50 -tag base
    # $c create line 0 $boardHeight 0 [expr $boardHeight*50] -width 2 -tag base
    # $c create line $boardWidth $boardHeight $boardWidth [expr $boardHeight+50] -width 2 -tag base
    # $c create line 0 [expr $boardHeight+50] $boardWidth [expr $boardHeight+50] -width 2 -tag base
    # $c create text [expr $boardWidth/2] [expr $boardHeight+25] -font ourArial -text "Rubik's Magic" -tag labelrubix

    #global firstHalfAlreadyAnimated
    #set firstHalfAlreadyAnimated 0

    update idletasks


    ### TODO: fill this in

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

    global BOARDARRAYSIZE BOARDSIZE colorX colorO gInitialPosition counter

    $c raise base all
    # $c raise labelrubix base
    set a(0) 0
    set counter [expr $counter + 1] 
#puts positions
#puts $gInitialPosition
#puts $position
    set pieceString [string range [C_GenericUnhash $position $BOARDARRAYSIZE] 0 [expr $BOARDARRAYSIZE - 1]]
    #set a $pieceString
    
    for {set i 0} {$i < $BOARDSIZE} {incr i} {
	if {[string compare [string index $pieceString $i] "x"] == 0 } {
	    #puts x
	    #puts $i
	    make_horizOval $c [calc_x $i] [calc_y $i] $colorX $counter    
	} elseif {[string compare [string index $pieceString $i] "o"] == 0} {
	    make_VertOval $c [calc_x $i] [calc_y $i] $colorO $counter
	}
	    #$c raise [subst $a($i)]b-$i base
	    #$c raise [subst $a($i)]c-$i [subst $a($i)]b-$i
	
    }
    update idletasks
    ### TODO: Fill this in

}


proc calc_x {index} {
global blockSize WIDTH
    #set xVal [expr floor ([expr $index / $WIDTH)]
    set xVal [expr $index % $WIDTH]
    return [expr [expr $xVal * $blockSize] + [expr $blockSize / 2]]
    
}

proc calc_y {index} {
global blockSize WIDTH
    #set yVal [expr $index % $HEIGHT]
    set yVal [expr floor ([expr $index / $WIDTH])]
    return [expr [expr $yVal * $blockSize] + [expr $blockSize / 2]]
}

proc make_horizOval {c x y color tag} {
    $c create oval [expr $x - 30] [expr $y - 20] [expr $x + 30] [expr $y + 20] -fill $color -tag piece$tag
} 

proc make_VertOval {c x y color tag} {
    $c create oval [expr $x - 20] [expr $y - 30] [expr $x + 20] [expr $y + 30] -fill $color -tag piece$tag
}

proc drawDot {c x y color tag} {
    $c create oval [expr $x - 18] [expr $y - 18] [expr $x + 18] [expr $y + 18] -fill $color -tag dot$tag

 $c bind "dot$tag" <Enter> "$c itemconfig dot$tag -fill black"
    $c bind "dot$tag" <Leave> "$c itemconfig dot$tag -fill $color"
    $c bind "dot$tag" <ButtonRelease> "ReturnFromHumanMove $tag"
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
	### TODO: Fill this in
    
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
#set lineWidth [ expr 3 * ( $pieceOutline / 2 ) ]
global BOARDARRAYSIZE BOARDSIZE counter
set pieceString [string range [C_GenericUnhash $position $BOARDARRAYSIZE] 0 [expr $BOARDARRAYSIZE - 1]]

 foreach item $moveList {

	set move  [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} { # lose for opponent, win for you
		set color green
	    } else {
		set color red4
	    }
	}

	if {[string compare [string index $pieceString $BOARDSIZE] "x"] == 0 } {
	    drawDot $c [calc_x $move] [calc_y $move] $color $move
	} else {
	    set from [fromIndex $move]
	    set to [toIndex $move]
	    drawArrow $c [calc_x $from] [calc_y $from] [calc_x $to] [calc_y $to]  $move $color  
       
	}
     }
 $c raise piece$counter all
update idletasks
    
    ### TODO: Fill this in
}

proc fromIndex {move} {
    return [expr $move >> 16] 
}

proc toIndex {move} {
    return [expr $move & 0x0000FFFF]
}

proc  drawArrow {c x1 y1 x2 y2 move color} {
##### arrow
    $c create line $x1 $y1 $x2 $y2 -width 15 -arrow last -arrowshape {30 30 15} -fill $color -tag "arrow$move"

   # $c create line [getPiecePositionX $startLoc] [getPiecePositionY $startLoc] [getPiecePositionX $endLoc] [getPiecePositionY $endLoc] -width 30 -arrow last -arrowshape {60 60 30} -fill $color -tag "bigArrow$color$startLoc$endLoc"


    $c bind "arrow$move" <Enter> "$c itemconfig arrow$move -fill black"
    $c bind "arrow$move" <Leave> "$c itemconfig arrow$move -fill $color"
    #lkll$c bind "arrow$move" <ButtonRelease> "checkmove $move"
    $c bind "arrow$move" <ButtonRelease> "ReturnFromHumanMove $move"
    #$c itemconfig $obj -fill $colour

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
    global boardHeight Winner

    set x [expr [calc_x 1] + 0] 
    for {set y 0} {$y <= [expr $boardHeight / 2]} {set y [expr $y + 10]} {
	$c lower text
	#update idletasks
	$c create text $x $y -text "$nameOfWinner Wins!" -font Winner -fill orange -tag text
	#$c create text  340 -text "WINS!"         -font Winner -fill orange -tags winner
	update idletasks
	#$c lower text
	#Tcl_Sleep(10)
	### TODO if needed
    }

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
    #update idletasks
	### TODO if needed

}
