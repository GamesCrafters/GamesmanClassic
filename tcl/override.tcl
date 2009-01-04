#############################################################################
##
## DoBoard
##
## We've started a game and set up a few preliminary variables. Let's play!
## I.e. let's create a couple of windows and set them up to play the game. 
##
#############################################################################

proc DoBoard {} {
    global gVarMoves varPredictions varObjective
    global gSlotsX gSlotsY gSlotSize kLabelFont kLabelColor
    global kGameName
    global kToMove kToWinStandard kToWinMisere
    global kMovesOnAllTheTime
    
    ### Since we can only play one game at a time...

    DisablePlayButtons

    ### The control window - create it, name it and place it.

    toplevel .winBoardControl
    wm title .winBoardControl "GAMESMAN $kGameName control"
    wm geometry .winBoardControl [GeometryRightOf .]
    
    frame .winBoardControl.f0 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f1 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f2 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f3
    frame .winBoardControl.f4 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f5 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f6 \
        -borderwidth 2 \
        -relief raised
    
    #### The "To Move" window and message
    
    label .winBoardControl.f0.labToMove \
        -font $kLabelFont \
        -text "To Move:" \
        -foreground $kLabelColor

    message .winBoardControl.f0.mesToMove \
        -font $kLabelFont \
        -text $kToMove \
	-justify center \
	-width 500
    
    #### The "To Win" window and message

    label .winBoardControl.f6.labToWin \
        -font $kLabelFont \
        -text "To Win:" \
        -foreground $kLabelColor

    if { $varObjective == "butStandard" } {
	set theToWinString $kToWinStandard
    } else {
	set theToWinString $kToWinMisere
    }

    message .winBoardControl.f6.mesToWin \
        -font $kLabelFont \
        -text $theToWinString \
	-justify center \
	-width 500
    
    #### The Whose-turn-is-it window and message
    
    label .winBoardControl.f5.labTurn \
        -font $kLabelFont \
        -text "Turn:" \
        -foreground $kLabelColor

    label .winBoardControl.f5.mesTurn \
        -font $kLabelFont \
        -text ""

    label .winBoardControl.f5.c
    
    #### The Predictions window and message

    label .winBoardControl.f4.labPredictions \
        -font $kLabelFont \
        -text "Prediction:" \
        -foreground $kLabelColor

    radiobutton .winBoardControl.f4.butOn \
        -text "On" \
	-font $kLabelFont \
        -command HandlePredictions \
        -variable varPredictions \
        -value butOn

    radiobutton .winBoardControl.f4.butOff \
        -text "Off" \
	-font $kLabelFont \
        -command HandlePredictions \
        -variable varPredictions \
	-value butOff

    label .winBoardControl.f4.predictions \
        -font $kLabelFont
    
    ### The Radio Buttons for Displaying Moves
    
    label .winBoardControl.f1.labShow \
        -font $kLabelFont \
        -text "Show:" -width 6\
        -foreground $kLabelColor
    
    radiobutton .winBoardControl.f1.noMoves \
        -text "Nothing" \
	-font $kLabelFont \
        -variable gVarMoves \
	-value noMoves \
        -command DeleteMoves

    radiobutton .winBoardControl.f1.validMoves \
        -text "Moves" \
	-font $kLabelFont \
        -variable gVarMoves \
	-value validMoves \
        -command ShowMoves 

    radiobutton .winBoardControl.f1.valueMoves  \
        -text "Value Moves" \
	-font $kLabelFont \
        -variable gVarMoves \
	-value valueMoves \
	-command ShowValueMoves
    
    ### The two bottom buttons, undo and abort
    
    button .winBoardControl.f2.undo \
        -text "Undo" \
	-font $kLabelFont \
	-state disabled

    button .winBoardControl.f2.redo \
        -text "Redo" \
	-font $kLabelFont \
	-state disabled
    
    button .winBoardControl.f2.abort \
	    -text "Abort" \
	    -font $kLabelFont \
	    -command { 
	EnablePlayButtons
	destroy .winBoardControl
	destroy .winBoard
	if { [winfo exists .winValueMoves] } {
	    destroy .winValueMoves
	}
	.f0.mesStatus config -text "Choose a Play Option\n"
    }
    
    button .winBoardControl.f2.newGame \
	    -text "New Game" \
	    -font $kLabelFont \
	    -command { 
	
	### Make the code a bit easier to read
	
	set w .winBoard.c
	
	### Clear the message field

	.f0.mesStatus config -text "Starting a new game\n"

	### Reset the position to the initial position

	set gPosition $gInitialPosition
	set gPlayerOneTurn $gHumanGoesFirst
	
	### Call the Game-specific New Game command

	GS_NewGame $w
	
	### Remove all Dead and Alive tags and reset them to what they were
	### when we reset the game.

	$w dtag tagDead
	$w dtag tagAlive
	$w addtag tagDead withtag tagNotInitial
	$w addtag tagAlive withtag tagInitial

	HandleTurn
	HandlePredictions

    }
    
    ### Packing it all in.
    
    pack append .winBoardControl.f0 \
	    .winBoardControl.f0.labToMove {left} \
	    .winBoardControl.f0.mesToMove {left expand fill}
         
    pack append .winBoardControl.f6 \
	    .winBoardControl.f6.labToWin {left} \
	    .winBoardControl.f6.mesToWin {left expand fill}
    
    if { $kMovesOnAllTheTime } {
	pack append .winBoardControl.f1 \
		.winBoardControl.f1.labShow {left} \
		.winBoardControl.f1.validMoves {left expand fill} \
		.winBoardControl.f1.valueMoves {left expand fill}
    } else {
	pack append .winBoardControl.f1 \
	    .winBoardControl.f1.labShow {left} \
		.winBoardControl.f1.noMoves {left expand fill} \
		.winBoardControl.f1.validMoves {left expand fill} \
		.winBoardControl.f1.valueMoves {left expand fill}
    }
 
 #        .winBoardControl.f2.undo {left expand fill}
 #        .winBoardControl.f2.redo {left expand fill}

    pack append .winBoardControl.f2 \
         .winBoardControl.f2.newGame {left expand fill} \
         .winBoardControl.f2.abort {left expand fill}
    
    pack append .winBoardControl.f4 \
         .winBoardControl.f4.labPredictions {left} \
         .winBoardControl.f4.predictions {right expand fill} \
         .winBoardControl.f4.butOn {top fill} \
         .winBoardControl.f4.butOff {top fill} 
         
    pack append .winBoardControl.f5 \
        .winBoardControl.f5.labTurn {left} \
	.winBoardControl.f5.c {left} \
        .winBoardControl.f5.mesTurn {left expand fill}
             
    pack append .winBoardControl \
	    .winBoardControl.f0 {top expand fill} \
	    .winBoardControl.f6 {top expand fill} \
	    .winBoardControl.f5 {top expand fill} \
	    .winBoardControl.f4 {top expand fill} \
	    .winBoardControl.f1 {top expand fill} \
	    .winBoardControl.f2 {top fill expand}

    ### This is completely a hack, but it looks good. When I change my font
    ### I'll have to find the right way to do this. Why do I keep saying that?

    foreach i { \
	    .winBoardControl.f0.labToMove \
	    .winBoardControl.f6.labToWin \
	    .winBoardControl.f5.labTurn \
	    .winBoardControl.f4.labPredictions \
	    .winBoardControl.f1.labShow \
	} {
	$i config -width 10 -anchor e
    }

    ### Set up the help strings

    SetupHelpStringsForWindow BoardWindow

    ### Update the size of the window so that I can place things around it.

    update

    ### Now that I've packed and updated .winBoardControl, I can pack and 
    ### place the board below it.

    ### The board window

    toplevel .winBoard
    wm title .winBoard "GAMESMAN $kGameName board"
    wm geometry .winBoard [GeometryBelow .winBoardControl]

    ### The Canvas on which to draw the board
    ### There's some assumption that full orthogonal boards will be used -
    ### this needs to be modified for other boards.

    canvas .winBoard.c \
        -width  [expr $gSlotsX*$gSlotSize] \
        -height [expr $gSlotsY*$gSlotSize] \
        -relief raised

    GS_LoadBoard .winBoard.c

 #   for {set i 0} {$i < $gSlotsX} {incr i} { 
 #	for {set j 0} {$j < $gSlotsY} {incr j} { 
 #           CreateSlot .winBoard.c $i $j  
 #	} 
 #   }

    GS_PostProcessBoard .winBoard.c

    pack append .winBoard .winBoard.c top

    ### Update the size of the window so that I can place things around it.

    update

    ### If the values of the radiobutton is set to anything other than the
    ### default, we need to call the routines to set the moves up.

    GS_NewGame .winBoard.c

 #   HandleMoves
}

#############################################################################
##
## ShowMoves
##
## This is a small procedure to show all available moves on the board.
## For every move returned by C_GetValueMoves, draw it.
##
#############################################################################

proc ShowMoves {} {
 #  set gVarMoves "validMoves"

    set c .winBoard.c

    $c itemconfig small -fill cyan
    $c itemconfig bigO -fill purple
    $c itemconfig neutral -fill purple
}


#############################################################################
##
## ShowValueMoves
##
## This is a small procedure to create all moves but color-code them based
## on their color-value. Also bring up an explanation of the colors.
##
#############################################################################

proc ShowValueMoves {} {
    global gPhase gValueMoves gFinalL gCurrentBlack gCurrentWhite
    
    set c .winBoard.c

    if {$gPhase == 1} {
	for {set i 0} {$i < [expr [llength $gValueMoves] / 13]} {incr i} {
	    set thisL [getNewL [lindex [lindex $gValueMoves [expr $i * 13]] 0]]
	    set thisColor red4
	    for {set j 0} {$j < 13} {incr j} {
		set theValue [lindex [lindex $gValueMoves [expr $i * 13 + $j]] 1]
		if {[string match Lose $theValue] == 1} {
		    set thisColor green
		    break
		}
		if {[string match Tie $theValue] == 1} {
		    set thisColor yellow
		}
	    }
	    $c itemconfig num$thisL -fill $thisColor 
	}
    } else {
	set specValueMoves [list]
	for {set i 0} {$i < [expr [llength $gValueMoves] / 13]} {incr i} {
	    set thisValueMove [lindex $gValueMoves [expr $i * 13]]
	    if {[getNewL [lindex $thisValueMove 0]] == $gFinalL} {
		for {set j 0} {$j < 13} {incr j} {
		    set specValueMoves [linsert $specValueMoves end [lindex $gValueMoves [expr $i * 13 + $j]]]
		}
	    }
	}
	for {set k 0} {$k < 13} {incr k} {
	    set thisMove [lindex $specValueMoves $k]
	    set thisOPiece [getOPiece [lindex $thisMove 0]]
	    set thisOPieceColor Black
	    if {$thisOPiece == 2} {
		set thisOPieceColor White
	    }
	    set thisOSquare [getNewO [lindex $thisMove 0]]
	    set thisOValue [lindex $thisMove 1]
	    if {$thisOValue == "Win"} {
		if {$k == 12} {
		    $c itemconfig bigBlack$gCurrentBlack -fill red4
		    $c itemconfig bigWhite$gCurrentWhite -fill red4
		}
		if {$k != 12} {
		    $c itemconfig neut$thisOSquare$thisOPieceColor -fill red4 
		}
	    }
	    if {$thisOValue == "Lose"} {
		if {$k == 12} {
		    $c itemconfig bigBlack$gCurrentBlack -fill green
		    $c itemconfig bigWhite$gCurrentWhite -fill green
		}
		if {$k != 12} {
		    $c itemconfig neut$thisOSquare$thisOPieceColor -fill green 
		}
	    }
	    if {$thisOValue == "Tie"} {
		if {$k == 12} {
		    $c itemconfig bigBlack$gCurrentBlack -fill yellow
		    $c itemconfig bigWhite$gCurrentWhite -fill yellow
		}
		if {$k != 12} {
		    $c itemconfig neut$thisOSquare$thisOPieceColor -fill yellow 
		}
	    }
	}
    }
    if { ![winfo exists .winValueMoves] } {
	DoValueMovesExplanation
    }
}

#############################################################################
##
## DoComputer
##
## The "Play Computer and the $theHumanGoesFirst button was clicked"
## It is assumed gHumanGoesFirst has been set already.
##
#############################################################################

proc DoComputer {} {
    global gAgainstComputer
    global gHumanGoesFirst
    global gPlayerOneTurn
    global gInitialPosition
    global gPosition

    .f0.mesStatus config -text "Playing against a\nperfect computer opponent"
    set gAgainstComputer 1
    set gPlayerOneTurn $gHumanGoesFirst
    set gPosition $gInitialPosition
    set playerOneTurn $gPlayerOneTurn

    ### Create the board, make the computers move if nec., update 
    ### the predictions and the turn.
    DoBoard
    HandleTurn
    HandlePredictions
}

#############################################################################
##
## DoComputersMove
##
## A new game has been requested and it's the computer's turn.
## It's assumed that the board exists already. We get the move and do it.
##
#############################################################################

proc DoComputersMove {} {
    global gPlayerOneTurn gPosition gCurrentBlack
    
    set c .winBoard.c

    ### Set the piece for the computer and get the move.
    set move  [C_GetComputersMove $gPosition]
    set L [getNewL $move]
    set OPiece [getOPiece $move]
    set OValue [getNewO $move]
    placeL $c num$L
    if {$OPiece == 0} {
	moveBigO $c bigBlack$gCurrentBlack
    } elseif {$OPiece == 1} {
	moveBigO $c bigBlack$OValue
    } else {
	moveBigO $c bigWhite$OValue
    }
}
