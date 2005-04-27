#############################################################################
##
## NAME:         mstt.tcl
##
## DESCRIPTION:  The source code for the Tcl component of ShiftTacToe
##               for the Master's project GAMESMAN
##
## AUTHOR:       Ling Xiao  -  University of California at Berkeley
##        
##
## DATE:         07-10-02
##
## UPDATE HIST:
##
## 07-10-02 1.0    : First release
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
    set kGameName "ShiftTacToe"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 50
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "3-in-a-row WINS"
    set kMisereString "3-in-a-row LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Shift Tac Toe is similar to Tic Tac Toe, except it has the extra option of shifting one of the rows on your turn. Players drop a piece into the slot by clicking the arrow that corresponds to where they would like to move. To shift the board, select the arrow on the sides that correspond to row that the player wants to shift."
    set kToWinStandard  "Be the first player to get three of your pieces\nin a row (horizontally, vertically or diagonally)."
    set kToWinMisere  "Force your opponent into getting three of his pieces\nin a row (horizontally, vertically or diagonally) first."

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 9
    set gSlotsY 6
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 767638
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
    set kCAuthors "Ling Xiao"
    set kTclAuthors "Ling Xiao"
    set kGifAuthors "$kRootDir/../bitmaps/lite3team.gif"

    
    ### Set the procedures that will draw the pieces
    
    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCross
    set kRightDrawProc DrawHollowCircle
    set kBothDrawProc  DrawCircle
    
    ### What type of interaction will it be, sir?

    global kInteractionType
    
    set kInteractionType Rearranger

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1

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

    ### Do nothing because there are no game-specific options yet
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

    ### In Shift Tac Toe
    
    set aMove [lindex $theMove 0]

    #puts "GS_ConvertInteractionToMoves $theMove \n"

    if {($aMove >= 3) && ($aMove <= 5)} {
	set actMove [expr $aMove -3]
    } else {
	switch $aMove {
	    18 { set actMove 3}
	    27 { set actMove 4}
	    36 { set actMove 5}
	    26 { set actMove 6}
	    35 { set actMove 7}
	    44 { set actMove 8}
	    default { set actMove -1 }
	}
    }

    #puts "Processed move $actMove"
    return $actMove;
    
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {

    ### In Shift Tac Toe 

    #puts "GS_ConvertMovesToInteraction $theMove \n"

    if {($theMove >= 3) && ($theMove <= 5)} {
	set actMove [expr $theMove -3]
    } else {
	switch $theMove {
	    18 { set actMove 3}
	    27 { set actMove 4}
	    36 { set actMove 5}
	    26 { set actMove 6}
	    35 { set actMove 7}
	    44 { set actMove 8}
	    default { set actMove -1 }
	}
    }

    #puts "Processed move $actMove"
    
    return $actMove;
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {

    ### We need to clear the surrounding slots, and create the basic blue and grey backdrop

    global gSlotSize

    $w create rectangle [expr 2*$gSlotSize] [expr 1*$gSlotSize]  [expr 7*$gSlotSize] [expr 2*$gSlotSize]  -fill blue -outline blue
    $w create rectangle [expr 2*$gSlotSize] [expr 5*$gSlotSize]  [expr 7*$gSlotSize] [expr 6*$gSlotSize]  -fill blue -outline blue
    
    $w create rectangle 0 0 [expr 9*$gSlotSize] [expr 1*$gSlotSize]  -fill grey -outline grey
    $w create rectangle 0 0 [expr 2*$gSlotSize] [expr 6*$gSlotSize]  -fill grey -outline grey
    $w create rectangle [expr 7*$gSlotSize] [expr 1*$gSlotSize]  [expr 9*$gSlotSize] [expr 6*$gSlotSize]  -fill grey -outline grey

    #$w create rectangle 0 0 900 100 -fill grey -outline grey
    #$w create rectangle 0 0 200 600 -fill grey -outline grey
    #$w create rectangle 700 100 900 600 -fill grey -outline grey

    DrawSlider $w 0 1
    DrawSlider $w 1 1
    DrawSlider $w 2 1

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

    #puts "GS_ConvertToAbsoluteMove"

    ### Unfortunately shift tac toe does not have absolute moves

    switch $theMove {
	    0 { return [list 3 12] }
	    1 { return [list 4 13] }
	    2 { return [list 5 14] }
	    3 { return [list 18 19] }
	    4 { return [list 27 28] }
	    5 { return [list 36 37] }
	    6 { return [list 26 25] }
	    7 { return [list 35 34] }
	    8 { return [list 44 43] }
	    default { return [list 0 0]}
	}
   
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

  ##I used theSlot to hold the configuration after the move
    global gHumanGoesFirst

    set boardPos [expr ($theSlot - ($theSlot % 3))/3]

    for {set x 3} {$x < 6} {incr x} {
	for {set y 4} {$y > 1} {incr y -1} {
	    
	    ClearSlot $w $x $y "black"

	    set choice [expr ($boardPos%3)]
	    
	    if { $gHumanGoesFirst} {
		switch $choice {
		    1 { DrawPiece $x $y "X" }
		    2 { DrawPiece $x $y "O" }
		    0 {}
		    default { puts "HandleEnablesDisables ERROR" }
		}
	    } else {
		switch $choice {
		    1 { DrawPiece $x $y "O" }
		    2 { DrawPiece $x $y "X" }
		    0 {}
		    default { puts "HandleEnablesDisables ERROR" }
		}
	    }
	    
	    set boardPos [expr ($boardPos - $choice)/3]
	}
    }
    
    for {set x 0} {$x < 3} {incr x} {
	set choice [expr ($boardPos%3)]
	DrawSlider $w $x $choice
	set boardPos [expr ($boardPos - $choice)/3]
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

    ### No slot embellishents for the ShiftTacToe meister.
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

    ### No slot embellishents for the ShiftTacToe meister.
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {

    ### Delete all pieces that are left around

    $w delete tagPiece

}




#############################################################################
##
## ClearSlot
##
## Game specific function that clears a slot, i.e. paints it grey
##
#############################################################################


proc ClearSlot { w slotX slotY theColor} {
    global gSlotSize
    
    set upLeftX [expr ($slotX * $gSlotSize)]
    set upLeftY [expr ($slotY * $gSlotSize)]
    set lowRightX [expr ($upLeftX + $gSlotSize)]
    set lowRightY [expr ($upLeftY + $gSlotSize)]

    #puts "$upLeftX, $upLeftY, $lowRightX, $lowRightY"

    $w create rectangle $upLeftX $upLeftY $lowRightX $lowRightY -fill grey -outline $theColor

    #puts "Just painted rectangle"
}


#############################################################################
##
## DrawSliderSquare
##
## Game specific function that draws one square of the slider bar
##
#############################################################################

proc DrawSliderSquare { w slotX slotY } {
    global gSlotSize
    
    set upLeftX [expr ($slotX * $gSlotSize)]
    set upLeftY [expr ($slotY * $gSlotSize)]
    set lowRightX [expr ($upLeftX + $gSlotSize)]
    set lowRightY [expr ($upLeftY + $gSlotSize)]

    #puts "$upLeftX, $upLeftY, $lowRightX, $lowRightY"

    $w create rectangle $upLeftX $upLeftY $lowRightX $lowRightY -fill white -outline black
   
    set upLeftX [expr (($slotX * $gSlotSize) + 10)]
    set upLeftY [expr (($slotY * $gSlotSize) + 10)]
    set lowRightX [expr (($upLeftX + $gSlotSize)-20)]
    set lowRightY [expr (($upLeftY + $gSlotSize)-20)]
    
    #puts "$upLeftX, $upLeftY, $lowRightX, $lowRightY"

    $w create oval $upLeftX $upLeftY $lowRightX $lowRightY -fill grey -outline grey

    #puts "Just painted rectangle"
}

#############################################################################
##
## DrawSlider
##
## Game specific instruction that draws the shift tac toe sliders
##
#############################################################################

proc DrawSlider { w row position } {

    set rowY [expr $row + 2]

    ClearSlot $w 1 $rowY "black"
    ClearSlot $w 2 $rowY "black"
    ClearSlot $w 6 $rowY "black"
    ClearSlot $w 7 $rowY "black"

   switch $position {
       0 {  DrawSliderSquare $w 1 $rowY 
	    DrawSliderSquare $w 2 $rowY }
	1 { DrawSliderSquare $w 2 $rowY 
	    DrawSliderSquare $w 6 $rowY }
	2 { DrawSliderSquare $w 6 $rowY 
	    DrawSliderSquare $w 7 $rowY }

	}

}










