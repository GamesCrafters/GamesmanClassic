#############################################################################
## 
## NAME:         mttt.tcl
##
## DESCRIPTION:  The source code for the Tcl component of TicTacToe
##               for the Master's project GAMESMAN
##
## AUTHOR:       Dan Garcia  -  University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:        05-12-95
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
    set kGameName "TicTacToe"

    global kCAuthors kTclAuthors kGifAuthors kRootDir
    set kCAuthors "Dan Garcia"
    set kTclAuthors "GamesCrafters"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "3-in-a-row WINS"
    set kMisereString "3-in-a-row LOSES"

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
    
    ### Set the procedures that will draw the pieces
    
    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCross
    set kRightDrawProc DrawHollowCircle
    set kBothDrawProc  DrawCircle
    
    ### What type of interaction
    
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

    return $theMove
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

    return $theMove
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

    DisableSlot $w $theSlot

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

    ### Delete all pieces that are left around

    $w delete tagPiece

}

