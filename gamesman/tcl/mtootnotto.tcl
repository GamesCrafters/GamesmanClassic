#############################################################################
#			GUI FOR MTOOTNOTTO
# Author: Zach Wasserman
#
# Changes:
# 2006-11-8 	Basic functions working correctly.
# 2006-12-6		New interface with arrows implemented.
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
    global basespeed
    set basespeed 70
    set kGameName "Toot and Otto"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition boardwidth boardheight
    set gInitialPosition 37782561
    set gPosition $gInitialPosition
    set boardwidth 4
    set boardheight 4

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player spells TOOT, second Player spells OTTO"
    set kMisereString "First player spells OTTO, second player spells TOOT"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Spell your name, first player OTTO, second player TOOT"
    } else {
	SetToWinString "To Win: Spell your name, first player TOOT, second player OTTO"
    }
    SetToMoveString "To Move: The players take turns dropping pieces into one of the four slots. The pieces fall to the bottom of the board or stack on top of each other.  Select an open slot to drop your piece. Choose whether to drop a T or an O"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Zach Wasserman & Kyler Murlas"
    set kTclAuthors "Zach Wasserman"
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

    return [list O T]

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

    return [list blue blue]
    
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

#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################
proc GS_Initialize { c } {

    global gFrameWidth gFrameHeight boardwidth boardheight size r linewidth fontsize piecespace movespace
    global p1t p1o p2t p2o
    set size [min $gFrameWidth $gFrameHeight]
    set linewidth 2
    set piecespace 100
    set r [expr 0.4 * ($size - $piecespace) / [max $boardwidth $boardheight]]
    set fontsize [expr int($size / 25)]
	set rmove [expr 0.025 * $size]
	set movespace [expr (($size - $piecespace) / (2 * [max $boardwidth $boardheight]) - (0.025 * $size)) / 4 + $rmove * 2]
	
	#Give Players Pieces
	set p1t [expr (($boardwidth * $boardheight) / 4) + (($boardwidth * $boardheight) % 4)]
	set p1o $p1t
	set p2t $p1t
	set p2o $p1t

    $c addtag background all
    DrawBoard $c

    set board "1221211221121221"
    DrawPieces $board $c

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
    
    DrawBoard $c
    set board [unhash $position]
    DrawPieces $board $c
	drawPlayerPieces $c
}

#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
	global p1t p1o p2t p2o boardwidth boardheight
	
	#Give Players Pieces
	set p1t [expr (($boardwidth * $boardheight) / 4) + (($boardwidth * $boardheight) % 4)]
	set p1o $p1t
	set p2t $p1t
	set p2o $p1t
	
    GS_DrawPosition $c $position
}

#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    global p1t p1o p2t p2o
    set total1 [expr $p1t + $p1o]
    set total2 [expr $p2t + $p2o]
    
    if {total2 < total1} {
    	return 2
    } else {
    	return 1
    }
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

    global size boardwidth boardheight r
    global p1t p1o p2t p2o

    # Need to figure out origin and destination coordinates of piece
    # Origin: Find x coord from theMove, y coord = -r
    # Destination: Same x coord as origin, y coord = coordinate of new piece
    
    
    set p1total [expr $p1t + $p1o]
    set p2total [expr $p2t + $p2o]
    
    if {$theMove > 10} {
            set TO T
            set col [expr $theMove - 10]
            if {$p1total == $p2total} {
            	incr p1t -1
            } else {
            	incr p2t -1
            }
    } else {
            set TO O
            set col $theMove
            if {$p1total == $p2total} {
	    	incr p1o -1
	    } else {
	        incr p2o -1
            }
    }
	
	GS_DrawPosition $c $oldPosition
    
    set m [expr 4 - $col]
    set oldboard [unhash $oldPosition]
    set newboard [unhash $newPosition]
			
    set origin [list [expr (2 * $m + 1) * $size / (2 * [max $boardwidth $boardheight])] [expr -$size / (2 * [max $boardwidth $boardheight])]]
    
    
    # To find coordinates of new piece, compare oldboard and newboard
    for { set i 0 } { $i < [string length $oldboard] } { incr i } {
	if { [string index $oldboard $i] != [string index $newboard $i] } {
	    break
	}
    }
    

    # i contains the index of the different piece, so getting coordinates is simple
    set destination [coords [lindex [coord $i] 0] [lindex [coord $i] 1]]

    set color blue
    
    set piece [$c create oval [expr (2 * $m + 1) * $size / (2 * [max $boardwidth $boardheight]) - $r] \
		   [expr -$size / (2 * [max $boardwidth $boardheight]) - $r] \
		   [expr (2 * $m + 1) * $size / (2 * [max $boardwidth $boardheight]) + $r] \
		   [expr -$size / (2 * [max $boardwidth $boardheight]) + $r] -fill $color]
		   
    set x [expr (2 * (4 - $col) + 1) * $size / (2 * [max $boardwidth $boardheight])]
    set y [expr -$size / (2 * [max $boardwidth $boardheight])]
		   
    set text [$c create text  [expr $x + 1] [expr $y + 1] \
    	     -font "Arial [expr int($r * 1)]" -text $TO -anchor center -fill red -justify center]
    	     

    animate $c $piece $text $origin $destination

    GS_DrawPosition $c $newPosition
}

proc animate { c piece text origin destination } {


    global basespeed gAnimationSpeed
    
    $c raise board

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
	$c move $text $dx $dy
	
	after 1
	update idletasks
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
# moveType = a string which is either value, moves or best according to which radio button is down		IMPORTANT!!!
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {
    
    foreach move $moveList {
	drawmove $c [lindex $move 0] [lindex $move 1] $moveType $position
    }
    
}

proc drawmove { c move value moveType position } {
    global boardwidth boardheight size piecespace movespace
    set rmove [expr 0.025 * $size]

    switch $moveType {
	value {

	    # If the move leads to a win, it is a losing move (RED)
	    # If the move leads to a losing position, it is a winning move (GREEN)
	    # If the move leads to a tieing position, it is a tieing move (YELLOW)
	    switch $value {
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
	
    
    ## set y [expr (($size - $piecespace) / (2 * [max $boardwidth $boardheight]) - $rmove) / 5]
	set y [expr $movespace / 2 - $rmove - 1]
    set r [expr $rmove]
    
    ##puts "move = $move	y = $y	r = $r"
    
    if {$move > 10} {
        	set piece T
        	set col [expr $move - 10]
        	set x [expr (2 * (4 - $col) + 1) * $size / (2 * [max $boardwidth $boardheight]) - $rmove * 3]
        } else {
        	set piece O
        	set col $move
        	set x [expr (2 * (4 - $col) + 1) * $size / (2 * [max $boardwidth $boardheight])]
    }
    
    ##puts "piece = $piece	move = $move	col = $col	x = $x"
    
    set m [$c create oval $x $y [expr $x + $rmove * 2] [expr $y + $rmove *2] -fill $color]
    
    if {$move > 10 } {
    	set t [$c create text [expr $x + $rmove] [expr $y + $rmove + 2]  -font "Arial [expr int($r * 1.5)]" -text $piece -anchor center -fill red -justify center]
    } else {
    	set t [$c create text [expr $x + $rmove + 1] [expr $y + $rmove + 1]  -font "Arial [expr int($r * 1.5)]" -text $piece -anchor center -fill red -justify center]
    }
	
	##Find which row the piece will drop to:
    
    for { set i [expr $col * ($boardwidth + 1) - 1] } { $i >= [expr $col * ($boardwidth) - $boardheight] } { incr i -1} {
    	##puts "i = $i index i = [expr $position & (1 << $i)]"
    	if { [expr $position & (1 << $i)] != 0 } {
    		break
    	}
    }
    
    set row [expr $boardheight - ($i - ($col - 1) * ($boardwidth + 1))]
    
    ##puts "pos = $position i = $i col = $col row = $row"
    
    set linX [expr (2 * (4 - $col) + 1) * $size / (2 * [max $boardwidth $boardheight]) - .5 * $rmove + 1]
    set linY [expr $movespace + 1]
    set endY [expr $movespace + ($row - 1) * (($size - ($piecespace + $movespace)) / $boardheight) + .5 * (($size - ($piecespace + $movespace)) / $boardheight)]
	set arrowwidth [expr 1.5 * $rmove]
	set arrowshape [list [expr 2 * $arrowwidth] [expr 2 * $arrowwidth] $arrowwidth]
	
    set a [$c create line $linX $linY $linX $endY -fill darkgray -width $arrowwidth -arrow last -arrowshape $arrowshape]
    
    ##Make lines show up above arrows
    $c raise board
    
	##if the mouse interacts with oval
    $c bind $m <Enter> "$c itemconfigure $m -fill black"
    $c bind $m <Leave> "$c itemconfigure $m -fill $color"
    $c bind $m <ButtonRelease-1> "ReturnFromHumanMove $move"
	
	##if the mouse interacts with the text
    $c bind $t <Enter> "$c itemconfigure $m -fill black"
    $c bind $t <Leave> "$c itemconfigure $m -fill $color"
    $c bind $t <ButtonRelease-1> "ReturnFromHumanMove $move"
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    GS_DrawPosition $c $position

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

    global size boardwidth boardheight r
	global p1t p1o p2t p2o

    GS_DrawPosition $c $positionAfterUndo
	
	set p1total [expr $p1t + $p1o]
    set p2total [expr $p2t + $p2o]
	
	if {$theMoveToUndo > 10} {
            set TO T
            set col [expr $theMoveToUndo - 10]
            if {$p1total == $p2total} {
            	incr p2t 1
            } else {
            	incr p1t 1
            }
    } else {
            set TO O
            set col $theMoveToUndo
            if {$p1total == $p2total} {
	    	incr p2o 1
	    } else {
	        incr p1o 1
            }
    }

    set m [expr 4 - $col]
    set oldboard [unhash $positionAfterUndo]
    set newboard [unhash $currentPosition]
			
    set destination [list [expr (2 * $m + 1) * $size / (2 * [max $boardwidth $boardheight])] [expr -$size / (2 * [max $boardwidth $boardheight])]]
    
    # To find coordinates of new piece, compare oldboard and newboard
    for { set i 0 } { $i < [string length $oldboard] } { incr i } {
	if { [string index $oldboard $i] != [string index $newboard $i] } {
	    break
	}
    }

    # i contains the index of the different piece, so getting coordinates is simple
    set origin [coords [lindex [coord $i] 0] [lindex [coord $i] 1]]

    set piece [$c create oval [expr [lindex $origin 0] - $r] \
		   [expr [lindex $origin 1] - $r] \
		   [expr [lindex $origin 0] + $r] \
		   [expr [lindex $origin 1] + $r] -fill blue]
		   
	##set x [expr (2 * (4 - $col) + 1) * $size / (2 * [max $boardwidth $boardheight])]
	set x [lindex $origin 0]
    ##set y [expr -$size / (2 * [max $boardwidth $boardheight])]
	set y [lindex $origin 1]
		   
    set text [$c create text  [expr $x + 1] [expr $y + 1] \
    	     -font "Arial [expr int($r * 1)]" -text $TO -anchor center -fill red -justify center]

    animate $c $piece $text $origin $destination

    GS_DrawPosition $c $positionAfterUndo
}


#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
    
    global size fontsize piecespace
    
    # Tell us it's "Game Over!" and announce and winner
    $c create rectangle 0 [expr ($size - $piecespace)/2 - 10] $size [expr ($size - $piecespace)/2 + 20] -fill gray -width 1 -outline black -tag "gameover"
    $c create text [expr $size/2] [expr ($size - $piecespace)/2 + 5] -text "Game Over! $nameOfWinner Wins" -font "Arial $fontsize" -fill black -tag "gameover"
	
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

    # Delete "Game Over!" text
    $c delete gameover
}


proc unhash { position } {

    global boardwidth boardheight

    set board [string repeat "0" [expr $boardwidth * $boardheight]]

    for { set i 0 } { $i < $boardwidth } { incr i } {
	set j [expr $boardheight - 1]
	
	for { set h [expr $i * ($boardheight + 1) + $boardheight] } { [expr $position & (1 << $h)] == 0 } { incr h -1 } {
	    set board [string replace $board [index $i $j] [index $i $j] 0]
	    incr j -1
	}
	incr h -1

	while { $j >= 0 } {
	    if { [expr $position & (1 << $h)] != 0 } {
		set board [string replace $board [index $i $j] [index $i $j] 1]
	    } else {
		set board [string replace $board [index $i $j] [index $i $j] 2]
	    }
	    incr j -1
	    incr h -1
	}
    }

    return [reverse $board]
}
		
proc reverse s {
	set res {}
	for {set i 0} {$i < [string length $s]} {incr i} {
		append res [string index $s end-$i]
	}
	
	return $res
}



proc DrawBoard { c } {
    global boardwidth boardheight linewidth size piecespace movespace

    $c delete {!background}

    ##originally: $c create rect 0 0 $size $size -fill lightgray -width 1 -outline black	
    $c create rect 0 0 $size $size -fill lightgray -width 1 -outline black

    # Create grid of size boardwidth vs. boardheight
    for { set i 1 } { $i <= $boardwidth } { incr i } {
		set x [expr $i * $size / [max $boardwidth $boardheight]]
		$c create line $x 0 $x [expr $size - $piecespace] -width $linewidth -tag board
    }
    for { set j 0 } { $j <= $boardheight } {incr j } {
		set x [expr $j * ($size - $piecespace - $movespace) / [max $boardwidth $boardheight] + $movespace]
		$c create line 0 $x $size $x -width $linewidth -tag board
    } 
	
}

proc DrawPieces { board c } {
    global boardwidth boardheight

    for { set i 0 } { $i < [expr $boardwidth * $boardheight] } { incr i } {
	set ij [coord $i]
	set xy [coords [lindex $ij 0] [lindex $ij 1]]
	draw [string index $board $i] [lindex $xy 0] [lindex $xy 1] $c
    }
}

proc draw { piece x y c } {
    global r boardwidth boardheight
    set color blue
    
	if { $piece != 0} {
		$c create oval [expr $x - $r] [expr $y - $r] [expr $x + $r] [expr $y + $r] -fill $color
		
		set TO O
		if { $piece == 2}  {
			set TO T
		}
		
		$c create text [expr $x] [expr $y] -text $TO -font "Arial [expr int($r)]" -anchor center -fill red -width [expr 2 * $r] -justify center
	}
}

proc drawPlayerPieces { c } {


	global p1t p1o p2t p2o piecespace r size
	set smallR [expr int($r / 2)]
	set color black
	
	
	#########################  PLAYER 1  ##############################
	
	$c create text [expr $size / 4] [expr $size - $piecespace / 2] -text "1st Player" -font "Arial 16" -anchor center -fill black -width [expr $size / 4]
	
	##Player 1 T
	set x [ expr int($size / 8) - $p1t ]
	set y [ expr $size - $piecespace + $smallR + 2]
	set TO T
	for { set placed 0 } { $placed < $p1t } { incr placed } {
		$c create oval [expr $x - $smallR] [expr $y - $smallR] [expr $x + $smallR] [expr $y + $smallR] -fill $color
		$c create text [expr $x] [expr $y] -text $TO -font "Arial [expr int($smallR)]" -anchor center -fill red -width [expr 2 * $smallR] -justify center
		incr x [expr $smallR * 2 + 1]
	}
	
	##Player 1 O
	set x [ expr int($size / 8) - $p1o ]
	set y [expr $size - $smallR - 2]
	set TO O
	for { set placed 0 } { $placed < $p1o } { incr placed } {
		$c create oval [expr $x - $smallR] [expr $y - $smallR] [expr $x + $smallR] [expr $y + $smallR] -fill $color
		$c create text [expr $x] [expr $y] -text $TO -font "Arial [expr int($smallR)]" -anchor center -fill red -width [expr 2 * $smallR] -justify center
		incr x [expr $smallR * 2 + 1]
	}
	
	
	
	#########################  PLAYER 2  ##############################
	
	$c create text [expr $size / 4 * 3] [expr $size - $piecespace / 2] -text "2nd Player" -font "Arial 16" -anchor center -fill black -width [expr $size / 4]
	
	##Player 2 T
	set x [ expr int($size / 8) * 5 - $p2t ]
	set y [ expr $size - $piecespace + $smallR + 2]
	set TO T
	for { set placed 0 } { $placed < $p2t } { incr placed } {
		$c create oval [expr $x - $smallR] [expr $y - $smallR] [expr $x + $smallR] [expr $y + $smallR] -fill $color
		$c create text [expr $x] [expr $y] -text $TO -font "Arial [expr int($smallR)]" -anchor center -fill red -width [expr 2 * $smallR] -justify center
		incr x [expr $smallR * 2 + 1]
	}
	
	##Player 2 O
	set x [ expr int($size / 8) * 5 - $p2o ]
	set y [expr $size - $smallR - 2]
	set TO O
	for { set placed 0 } { $placed < $p2o } { incr placed } {
		$c create oval [expr $x - $smallR] [expr $y - $smallR] [expr $x + $smallR] [expr $y + $smallR] -fill $color
		$c create text [expr $x] [expr $y] -text $TO -font "Arial [expr int($smallR)]" -anchor center -fill red -width [expr 2 * $smallR] -justify center
		incr x [expr $smallR * 2 + 1]
	}
}
	
proc coord { index } {
    global boardwidth

    return [list [expr int($index / $boardwidth)] [expr $index % $boardwidth]]
}


proc coords { i j } {
    global size boardwidth boardheight piecespace movespace
    return [list [expr ($i + 0.5) * $size / [max $boardwidth $boardheight]] \
		[expr (($j + 0.5) * ($size - $piecespace - $movespace) / [max $boardwidth $boardheight]) + $movespace]]
}

proc index { i j } {
    global boardwidth

    return [expr $boardwidth * $i + $j]
}

proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}


proc max { a b } {
    if { $a > $b } {
	return $a
    }
    return $b
}


