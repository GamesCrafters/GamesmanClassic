#############################################################################
##
## NAME:         mtactix.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Tac Tix
##               for the Master's project GAMESMAN
##
## AUTHOR:       Dan Garcia  -  University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         05-12-95
##
## UPDATE HIST:
##
## 05-15-95 1.0    : Final release code for M.S.
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
    set kGameName "Tac Tix"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules
    
    global kStandardString kMisereString
    set kStandardString "Removing the last stone WINS"
    set kMisereString "Removing the last stone LOSES"
    
    ### Set the strings to tell the user how to move and what the goal is.
    
    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click on the line/circle that removes the pieces you wish"
    set kToWinStandard  "Remove the last stone from the board"
    set kToWinMisere  "Force your opponent into removing the last stone from the board."
    
    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 4
    set gSlotsY 4
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 1911
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/plus.xbm"
    set xbmRight "$kRootDir/../bitmaps/plus.xbm"
    set macLeft  cross
    set macRight cross

    ### Authors Names and Photo (by AP)
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Dan Garcia"
    set kTclAuthors "Dan Garcia"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCircle
    set kRightDrawProc DrawCircle
    set kBothDrawProc  DrawCircle

    ### What type of interaction will it be, sir?

    global kInteractionType
    set kInteractionType MultiplePieceRemoval

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 1
}

#############################################################################
##
## GS_AddGameSpecificGUIOptions
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_AddGameSpecificGUIOptions { w } {

    ### Do nothing because there are no game-specific options
}

#############################################################################
##
## GS_AddGameSpecificGUIOnTheFlyOptions
##
## This creates new frame(s) for GUI options that can be changed on the fly.
##
#############################################################################

proc GS_AddGameSpecificGUIOnTheFlyOptions { w } {
    ### Do nothing because there are no game-specific on-the-fly options
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

    return {}
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {

    global gPosition gSlotsX gSlotsY

    ### Now we put the pieces on the board

    set theIndex [SinglePieceRemovalCoupleMove $slotX $slotY]

    if { $gPosition & int(pow(2,$theIndex)) } {

	### Put a piece there

	DrawPiece $slotX $slotY "+"
    }
}

#############################################################################
##
## GS_ConvertInteractionToMove
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertInteractionToMove { theMove } {

    ### In TacTix, the move is just the slot itself.

    set theOutputMove 0

    foreach i $theMove {
	incr theOutputMove [expr int(pow(2,$i))]
    }

    return $theOutputMove
}


#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {

    ### In TacTix, the move is just the slot itself.

    return $theMove
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {
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
    global gPosition gSlotsX gSlotsY

    ### Fortunately, TacTix's moves are already absolute.

    set theSlots {}

    for {set i 0} {$i < ($gSlotsX * $gSlotsY)} {incr i} { 

	if { int(pow(2,$i)) & $theMove } {
	    lappend theSlots $i
	}
    }    

    return $theSlots
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
    global gSlotsX

    $w delete tagLastMove

    ### Here we remove all the pieces under the move just chosen.

    foreach i [GS_ConvertToAbsoluteMove $theMove] {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	set theGoner [DrawCircle $w $slotX $slotY tagDummy magenta 1]
	$w dtag $theGoner tagPieceCoord$slotX$slotY 
	$w lower tagPieceCoord$slotX$slotY all
	DeleteMoves

	### Animating the piece's demise doesn't work, so we just remove it
	### When I find the memory bug, put this back.

#	ShrinkDeleteTag $w $theGoner
	$w delete $theGoner

	DrawPiece $slotX $slotY "-"
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

    ### No slot embellishents for the TacTix meister.
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

    ### No slot embellishents for the TacTix meister.
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {

    ### Here we make all dead slots alive. The blank board contains a full
    ### board of live slots.

    $w raise tagPiece all

    #$w addtag tagAlive withtag tagDead
    #$w dtag tagDead
}

