#############################################################################
##
## NAME:         mlite3.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Mlite3
##
## AUTHOR:       Dan Garcia
##               Alex Perelman
##
## DATE:         04-03-2002
##
##
#############################################################################

#############################################################################
##
## GS_InitGameSpecific
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_InitGameSpecific {} {

    ### Set the name of the game

    global kGameName
    set kGameName "Lite-3"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "Game Objective WINS"
    set kMisereString "Game Objective  LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click with the left button\non the slot you want"
    set kToWinStandard  "Be the first player to get three of your pieces\nin a row (horizontally, vertically or diagonally)"
    set kToWinMisere  "Force your opponent into getting three of his pieces\nin a row (horizontally, vertically or diagonally) first"

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 3
    set gSlotsY 3
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition
 
    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/X.xbm"
    set xbmRight "$kRootDir/../bitmaps/O.xbm"
    set macLeft  X_cursor
    set macRight circle

    ### Authors Names and Photo (by AP)
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Alex Perelman and Babak Hamadani"
    set kTclAuthors "Alex Perelman"
    set kGifAuthors "$kRootDir/../bitmaps/lite3team.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCross
    set kRightDrawProc DrawHollowCircle
    set kBothDrawProc  DrawCircle

    ### What type of interaction will it be, sir?

    global kInteractionType
    set kInteractionType SinglePieceRemoval

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 0

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 0

    ### What are the default game-specific options, sir?
    global varGameSpecificOption1
    set varGameSpecificOption1 0

    ### Color of the numbers (to show order)
    global numberColor
    set numberColor grey

    global varXone varXtwo varXthree varOone varOtwo varOthree
    set varXone -1
    set varXtwo -1
    set varXthree -1
    set varOone -1
    set varOtwo -1
    set varOthree -1
}

#############################################################################
##
## GS_AddGameSpecificGUIOptions
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_AddGameSpecificGUIOptions { w } {

    global kLabelFont kLabelColor varGameSpecificOption1

    frame $w.f1 \
	    -borderwidth 2 \
	    -relief raised
    
    message $w.f1.labMoves \
	    -font $kLabelFont \
	    -text "What would you like the game objective to be:" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f1.butThree \
	    -text "Three In A Row (Standard)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0
    
    radiobutton $w.f1.butSurround \
	    -text "Surround (Trap opponent's piece) " \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 1

    radiobutton $w.f1.butBoth \
	    -text "Either Three In A Row or Surround " \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 2

    ### Pack it all in
    
    pack append $w.f1 \
	    $w.f1.labMoves {left} \
	    $w.f1.butThree {top expand fill} \
	    $w.f1.butSurround {top expand fill} \
	    $w.f1.butBoth {top expand fill}
    
    pack append $w \
	    $w.f1 {top expand fill}
}


#############################################################################
##
## GS_AddGameSpecificGUIOnTheFlyOptions
##
## This creates new frame(s) for GUI options that can be changed on the fly.
##
#############################################################################

proc GS_AddGameSpecificGUIOnTheFlyOptions { w } {

    global kLabelFont kLabelColor

    ### Another option

    frame $w.f9 \
	    -borderwidth 2 \
	    -relief raised
    
    label $w.f9.labNumbers \
	    -font $kLabelFont \
	    -text "Order:" \
	    -foreground $kLabelColor
    
    radiobutton $w.f9.butYes \
	    -text "Show" \
	    -font $kLabelFont \
	    -variable numberColor \
	    -value white \
	    -command {

	# turn on the numbers

	.winBoard.c itemconfig tagText -fill white
    }
    
    radiobutton $w.f9.butNo \
	    -text "Hide" \
	    -font $kLabelFont \
	    -variable numberColor \
	    -value grey \
	    -command {

	# turn off the numbers
	
	.winBoard.c itemconfig tagText -fill grey
	
    }

    ### Pack it all in
    
    pack append $w.f9 \
	    $w.f9.labNumbers {left} \
	    $w.f9.butYes {left expand fill} \
	    $w.f9.butNo {left expand fill} 
    
    pack append $w \
	    $w.f9 {top expand fill}

    $w.f9.labNumbers config -width 12 -anchor e
}


#############################################################################
##
## GS_GetGameSpecificOptions
##
## If you don't have any cool game specific options, then just return 
## an empty list. Otherwise, return a list of 1s or 0s...
##
#############################################################################

proc GS_GetGameSpecificOptions {} {

    global varGameSpecificOption1 kToWinStandard kToWinMisere

    ## Set the description strings according
    ## to the chosen game winning conditions.
    ## In here because this is called right 
    ## before the game is solved.

    if { $varGameSpecificOption1 == 0 } {
	set kToWinStandard  "Be the first player to get three of your pieces\nin a row (horizontally, vertically or diagonally)"
	set kToWinMisere  "Force your opponent into getting three of his pieces\nin a row (horizontally, vertically or diagonally) first"
    } elseif { $varGameSpecificOption1 == 1 } {
	set kToWinStandard "Be the first player to trap your opponent's piece,\ni.e. completely surround it with your three pieces"
	set kToWinMisere "Force your opponent into trapping one of your pieces,\ni.e. completely surrounding it with his three pieces"
    } elseif { $varGameSpecificOption1 == 2 } {
	set kToWinStandard "Be the first player to get three of your pieces\nin a row or trap your opponent's piece"
	set kToWinMisere "Force your opponent into getting three of his pieces\nin a row or trapping one of your pieces"
    }

    return [list $varGameSpecificOption1]
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {

    global gSlotSize kLabelFont

    ### for TicTacToe, we enable all initial slots

    EnableSlot $w $slot
    
    ### And all the slots should be active upon "New Game"

    $w addtag tagInitial withtag $slot
}

#############################################################################
##
## GS_ConvertInteractionToMove
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertInteractionToMove { theMove } {

    ### In TicTacToe, the move is just the slot itself.

    return [expr $theMove + 1]
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {

    ### In TicTacToe, the move is just the slot itself.

    return [expr $theMove - 1]
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {

    ### In TicTacToe, we need no post-processing
}

#############################################################################
##
## GS_ConvertToAbsoluteMove
##
## Sometimes the move handed back by our C code is a relative move. We need
## to convert this to an absolute move to indicate on the board.
##
#############################################################################

proc GS_ConvertToAbsoluteMove { theMove } {
    global gPosition

    ### Fortunately, TicTacToe's moves are already absolute.

    return [expr $theMove - 1]
}

#############################################################################
##
## GS_HandleEnablesDisables
##
## At this point a move has been registered and we have to handle the 
## enabling and disabling of slots
##
#############################################################################

proc GS_HandleEnablesDisables { w theSlot theMove } {

    global varXone varXtwo varXthree varOone varOtwo varOthree gPlayerOneTurn gSlotsX gSlotSize kLabelFont gSlotList numberColor

    DisableSlot $w $theSlot

    set theBoardMove [GS_ConvertMoveToInteraction $theMove]
    set moveSlotX   [expr $theBoardMove % $gSlotsX]
    set moveSlotY   [expr $theBoardMove / $gSlotsX]
    set textNum "1"

    if { $gPlayerOneTurn } {

	# must have been o's turn

	if { $varOone == -1 } {
	    set varOone $theBoardMove
	    set textNum "1"
	} elseif { $varOtwo == -1 } {
	    set varOtwo $theBoardMove
	    set textNum "2"
	} elseif { $varOthree == -1 } {
	    set varOthree $theBoardMove
	    set textNum "3"
	} else {
	    set slotX   [expr $varOone % $gSlotsX]
	    set slotY   [expr $varOone / $gSlotsX]

	    $w delete tagPieceOnCoord$slotX$slotY
	    EnableSlot $w $gSlotList($slotX,$slotY)
	    
	    set varOone $varOtwo
	    set varOtwo $varOthree
	    set varOthree $theBoardMove

	    set textNum "3"

	    $w delete tagTextNumO1

	    $w itemconfig tagTextNumO3 -text "2"
	    $w addtag num3 withtag tagTextNumO3

	    $w itemconfig tagTextNumO2 -text "1"
	    $w addtag num2 withtag tagTextNumO2

	    $w dtag tagTextNumO3
	    $w dtag tagTextNumO2
	    $w dtag tagTextNumO1

	    $w addtag tagTextNumO2 withtag num3
	    $w addtag tagTextNumO1 withtag num2

	    $w dtag num3
	    $w dtag num2
	}

	set theText [$w create text [expr ($moveSlotX * $gSlotSize) + 4] [expr ($moveSlotY + 1)*$gSlotSize - 4] \
		-text $textNum \
		-anchor sw \
		-font $kLabelFont \
		-fill  $numberColor \
		-tag tagText]

	$w addtag tagTextNumO$textNum withtag $theText

	#$w itemconfig tagTextNumO1 -fill white
	#$w itemconfig tagTextNumO2 -fill white
	#$w itemconfig tagTextNumO3 -fill white

    } else {

	#must have been x's turn

	if { $varXone < 0 } {
	    set varXone $theBoardMove
	    set textNum "1"
	} elseif { $varXtwo < 0 } {
	    set varXtwo $theBoardMove
	    set textNum "2"
	} elseif { $varXthree < 0 } {
	    set varXthree $theBoardMove
	    set textNum "3"
	} else {
	    set slotX   [expr $varXone % $gSlotsX]
	    set slotY   [expr $varXone / $gSlotsX]

	    $w delete tagPieceOnCoord$slotX$slotY
	    EnableSlot $w $gSlotList($slotX,$slotY)
	    
	    set varXone $varXtwo
	    set varXtwo $varXthree
	    set varXthree $theBoardMove

	    set textNum "3"

	    $w delete tagTextNumX1

	    $w itemconfig tagTextNumX3 -text "2"
	    $w addtag num3 withtag tagTextNumX3
	    
	    $w itemconfig tagTextNumX2 -text "1"
	    $w addtag num2 withtag tagTextNumX2

	    $w dtag tagTextNumX3
	    $w dtag tagTextNumX2
	    $w dtag tagTextNumX1

	    $w addtag tagTextNumX2 withtag num3
	    $w addtag tagTextNumX1 withtag num2

	    $w dtag num3
	    $w dtag num2
	}

	 ### We add a numeric label to each slot

	 
	set theText [$w create text [expr ($moveSlotX * $gSlotSize) + 4] [expr ($moveSlotY + 1)*$gSlotSize - 4] \
		-text $textNum \
		-anchor sw \
		-font $kLabelFont \
		-fill  $numberColor \
		-tag tagText]

	$w addtag tagTextNumX$textNum withtag $theText
	
		#    $w itemconfig tagTextNumX3 -fill white
    }

}

#############################################################################
##
## GS_EnableSlotEmbellishments
##
## If there are any slot embellishments that need to be enabled, now is the
## time to do it.
##
#############################################################################

proc GS_EnableSlotEmbellishments { w theSlot } {

    ### No slot embellishents for the TicTacToe meister.
}

#############################################################################
##
## GS_DisbleSlotEmbellishments
##
## If there are any slot embellishments that need to be enabled, now is the
## time to do it.
##
#############################################################################

proc GS_DisableSlotEmbellishments { w theSlot } {

    ### No slot embellishents for the TicTacToe meister.
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {

    global varXone varXtwo varXthree varOone varOtwo varOthree

    ### Delete all pieces that are left around

    $w delete tagPiece

    set varXone -1
    set varXtwo -1
    set varXthree -1
    set varOone -1
    set varOtwo -1
    set varOthree -1

    $w delete tagText
}

