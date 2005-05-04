#############################################################################
##
## NAME:         m1210.tcl
##
## DESCRIPTION:  The source code for the Tcl component of the game 1,2,...,10
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
    set kGameName "1,2,...,10"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 60
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to reach 10 WINS"
    set kMisereString "First player to reach 10 LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "On your turn, move one or two spaces on the board. Select the dot that corresponds to the desired move."
    set kToWinStandard  "Be the first player to reach the top slot (10)"
    set kToWinMisere    "Force your opponent to reach the top slot (10) first"

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 1
    set gSlotsY 10
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/circle.xbm"
    set xbmRight "$kRootDir/../bitmaps/circle.xbm"
    set macLeft  dot
    set macRight dot

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
    set kInteractionType SinglePieceRemoval

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 0

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 0
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
    global gSlotsY gSlotList gSlotSize kLabelFont xbmGrey varObjective

    ### We add a numeric label to each slot

    set theText [$w create text 7 [expr ($slotY+1)*$gSlotSize - 5] \
	    -text "[expr $gSlotsY - $slotY]" \
	    -anchor sw \
	    -font $kLabelFont \
	    -fill  white \
	    -tag tagText]

    $w addtag tagTextSlot$slot withtag $theText
    #puts "theText = $theText"

    ### for 1210, we'd like to highlight the winning slots as well as add
    ### numbers to the slots. Make it nice for the people.
    ### Remember the slots are numbered from 0 from the top.

    if { $slotY == 0 } {

	if { $varObjective == "butStandard" } {
	    set theMessage "WIN"
	} else {
	    set theMessage "LOSE"
	}
	    

	$w itemconfig $slot -outline yellow
	$w itemconfig $slot -stipple @$xbmGrey
	set theText [$w create text [expr $gSlotSize/2] 5 \
		-text $theMessage \
		-anchor n \
		-font $kLabelFont \
		-fill  white \
		-tag tagText]

	$w addtag tagTextSlot$slot withtag $theText
	$w bind tagTextSlot$slot <1> { HandleMove [SinglePieceRemovalCoupleMove [expr %x/$gSlotSize] [expr %y/$gSlotSize]] }
	$w bind tagTextSlot$slot <Enter> { .winBoard.c itemconfig $gSlotList([expr %x/$gSlotSize],[expr %y/$gSlotSize]) -fill black }

    } 

    if { $slotY == ($gSlotsY-2) || $slotY == ($gSlotsY-1) } {

	### Enable the first two slots

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
    global gSlotsY gPosition

    ### In 1210, we have to be a bit clever. There are gSlotsY rows, but
    ### they are ordered from top to bottom instead of the other way around.
    ### So we need to deal with that. Plus the move is just 1 or 2 (the 
    ### difference between the physical move and the position).

    return [expr $gSlotsY - $theMove - $gPosition]
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {
    global gSlotsY gPosition

    ### In 1210, we have to be a bit clever. There are gSlotsY rows, but
    ### they are ordered from top to bottom instead of the other way around.
    ### So we need to deal with that. Plus the move is just 1 or 2 (the 
    ### difference between the physical move and the position).

    return [expr $gSlotsY - $theMove - $gPosition]
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {

    ### Here we need to raise the text items because they need raising,
    ### otherwise they'll be under the slots the whole time...
    
    $w raise tagText tagSlot
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
    global gPosition gSlotsY

    return [expr $gSlotsY - ($theMove + $gPosition)]
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
    global gSlotList gPosition gSlotsY

    if { ($gSlotsY - ($gPosition+$theMove+1)) >= 0 } {
	#puts "GS slot = $theSlot, move = $theMove, gPosition = $gPosition, slot = $gSlotList(0,[expr $gSlotsY - ($gPosition+$theMove+1)])"
    }

    DisableSlot $w $theSlot

    ### If they skipped one, we need to disable it
    ### We have NOT done the move yet. This is kind of a hack.
    ### But the slots are listed from bottom to top (stoopid)

    if { $theMove == 2 } {
	DisableSlot $w $gSlotList(0,[expr $gSlotsY - ($gPosition + 1)])
    }

    ### The if statement is to prevent running over the end

    if { ($gSlotsY - ($gPosition+$theMove+1)) >= 0 } {
	EnableSlot $w $gSlotList(0,[expr $gSlotsY - ($gPosition+$theMove+1)])
    }
    if { ($gSlotsY - ($gPosition+$theMove+2)) >= 0 } {
	EnableSlot $w $gSlotList(0,[expr $gSlotsY - ($gPosition+$theMove+2)])
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
    global gSlotSize

#    puts "enablingSlotEmb $theSlot"

    $w bind tagTextSlot$theSlot <1> { HandleMove [SinglePieceRemovalCoupleMove [expr %x/$gSlotSize] [expr %y/$gSlotSize]] }
    $w bind tagTextSlot$theSlot <Enter> { .winBoard.c itemconfig $gSlotList([expr %x/$gSlotSize],[expr %y/$gSlotSize]) -fill black }

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

#    puts "disblingSlotEmb $theSlot"

    $w bind tagTextSlot$theSlot <1> { }
    $w bind tagTextSlot$theSlot <Enter> { }
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {
    global gSlotList gPosition gSlotsY

    ### Delete all pieces that are left around

    $w delete tagPiece

    GS_EnableSlotEmbellishments $w $gSlotList(0,[expr $gSlotsY - 1])
    GS_EnableSlotEmbellishments $w $gSlotList(0,[expr $gSlotsY - 2])
}

