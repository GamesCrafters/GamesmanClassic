#############################################################################
##
## NAME:         mwin4.tcl
##
## DESCRIPTION:  The source code for the Tcl component of the game Connect-4
##               for the Master's project GAMESMAN
##
## AUTHOR:       Michael Thon  -  University of California at Berkeley
##               most parts taken over from mttt.tcl
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         11-12-02
##
## UPDATE HIST:
##
## 11-12-02 1.0: Modified the mttt.tcl-code for connect-4.
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

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 4
    set gSlotsY 4

    ### Set the name of the game

    global kGameName
    set kGameName "Connect-4"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 60
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to get 4 in a row WINS"
    set kMisereString "First player to get 4 in a row LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click with the left button on a slot slot"
    set kToWinStandard  "Be the first player to get 4 in a row"
    set kToWinMisere    "Force your opponent to get 4 in a row"
   
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    GS_MyInitialPosition
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/circle.xbm"
    set xbmRight "$kRootDir/../bitmaps/circle.xbm"
    set macLeft  dot
    set macRight dot

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCircle
    set kRightDrawProc DrawCircle
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

    ### For the tcl-interface I avoid using the position representation
    ### from mwin4.c and store some position information instead:

    global mySlots
    GS_ResetMySlots
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
## GS_GetGameSpecificOptions
##
## If you don't have any cool game specific options, then just return 
## an empty list. Otherwise, return a list of 1s or 0s...
##
#############################################################################

proc GS_GetGameSpecificOptions {} {

    return {}
}

proc GS_AddGameSpecificGUIOnTheFlyOptions { w } {
    ### Do nothing because there are no game-specific on-the-fly options
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {
    global gSlotsY
    
    ### Reset my stored position information
    GS_ResetMySlots

    if { $slotY == ($gSlotsY-1) } {

	### Enable the first slot

	EnableSlot $w $slot
	$w addtag tagInitial withtag $slot

    } else {

	### Disable all other slots

	DisableSlot $w $slot
	$w addtag tagNotInitial withtag $slot
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
    global gSlotsX

    return [expr $theMove % $gSlotsX]
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {
    global mySlots gSlotsX 

    return [expr ($mySlots($theMove))*$gSlotsX + $theMove]
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
    global mySlots gSlotsX 

    return [expr ($mySlots($theMove))*$gSlotsX + $theMove]
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
    global gSlotList gPosition gSlotsY mySlots

    DisableSlot $w $gSlotList($theMove,$mySlots($theMove))
    set mySlots($theMove) [expr $mySlots($theMove)-1]

    if { ($mySlots($theMove)) >= 0 } {
	EnableSlot $w $gSlotList($theMove,$mySlots($theMove))
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

    ### Don't understand the purpose of this at all. Have I not done
    ### everything in the previous function?

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

    ### Don't understand the purpose of this either. 

}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {
    global gSlotList gSlotsX gSlotsY mySlots

    ### Delete all pieces that are left around

    $w delete tagPiece

    ### Reset my stored position information
    GS_ResetMySlots
}

#############################################################################
##
## GS_ResetMySlots
##
## We need to reset mySlots (information about how full each slot is)
##
#############################################################################

proc GS_ResetMySlots { } {
    global mySlots gSlotsX gSlotsY

    for {set i 0} {$i < $gSlotsX} {incr i} {
	set mySlots($i) [expr $gSlotsY-1]
    }
}

#############################################################################
##
## GS_MyInitialPosition
##
## Calculates the initial position which depends on the height and width of
## the board (i.e. this code can also handle other board sizes :-)
##
#############################################################################

proc GS_MyInitialPosition { } {
    global gInitialPosition gSlotsX gSlotsY
    
    set gInitialPosition 1
    for {set i 1} {$i < $gSlotsX} {incr i} {
	for {set j 0} {$j <= $gSlotsY} {incr j} {
	    set gInitialPosition [expr 2*$gInitialPosition]
	}
	set gInitialPosition [expr 1+$gInitialPosition] 
    }
}
