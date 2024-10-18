#############################################################################
##
## NAME:         mjoust.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Joust
##               for the Master's project GAMESMAN
##
## AUTHOR:       Isaac Greenbride  -  University of California at Berkeley
##               
##
## DATE:         04/03/02
##
## UPDATE HIST:
##
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
    set kGameName "JOUST"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "Opponent can't move WINS"
    set kMisereString "You can't move LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Each player has a knight piece, which moves as in the game of Chess. Once the piece is moved, the square the piece previously occupied cannot be re-entered by either player."
    set kToWinStandard  "Set up a position that prevents your opponent from moving"
    set kToWinMisere  "Force your opponent to set up a position that prevents you from moving"

    ### Set the size of the board

    global gSlotsX gSlotsY BITSIZEMASK 
    set gSlotsX 4
    set gSlotsY 4
    set BITSIZE 4
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 16778208
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
    set kCAuthors "Isaac Greenbride"
    set kTclAuthors "Isaac Greenbride"
    set kGifAuthors "$kRootDir/../bitmaps/lite3team.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCross
    set kRightDrawProc DrawHollowCircle
    set kBothDrawProc  DrawCircle

    ### What type of interaction will it be, sir?

    global kInteractionType
    set kInteractionType SinglePieceRemoveal

    ### Will you be needing moves to be on all the time, sir?
     
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 0

    ### What are the default game-specific options, sir?
    global varGameSpecificOption1
    set varGameSpecificOption1 butYes

    ### What are the player's piece types, sir?
    global varGameSpecificOption2
    set varGameSpecificOption2 knight

    global varGameSpecificOption3
    set varGameSpecificOption3 knight
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
            -text "How would you like to play?" \
            -width 200 \
            -foreground $kLabelColor
    
    radiobutton $w.f1.butYes \
            -text "Trapping your opponent WINS" \
            -font $kLabelFont \
            -variable varGameSpecificOption1 \
            -value butYes
    
    radiobutton $w.f1.butNo \
            -text "Trapping your opponent LOSES (you want to avoid it)" \
            -font $kLabelFont \
            -variable varGameSpecificOption1 \
            -value butNo

    message $w.f1.choosePiece \
            -font $kLabelFont \
            -text "With what pieces will you and your opponent be playing?" \
            -width 200 \
            -foreground $kLabelColor

  
    ### Pack it all in
    
    pack append $w.f1 \
            $w.f1.labMoves {left} \
            $w.f1.butYes {top expand fill} \
            $w.f1.butNo {top expand fill} \
            $w.f1.choosePiece {left}
         
    ## player1 piece type buttons 
    foreach i {0, 1, 2, 3, 4} {
        radiobutton $w.f1.xMoveType$i \
                -text $i \
                -font $kLabelFont \
                -variable varGameSpecificOption3 \
                -value $i
        pack append $w.f1 \
                $w.f1.xMoveType$i {top expand fill} 
    }

    ## player2 piece type buttons
    foreach i {0, 1, 2, 3, 4} {
        radiobutton $w.f1.oMoveType$i \
                -text $i \
                -font $kLabelFont \
                -variable varGameSpecificOption4 \
                -value $i 
        pack append $w.f1 \
                $w.f1.oMoveType$i {top expand fill} 
    }
    
    pack append $w \
            $w.f1 {top expand fill}

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
    global varGameSpecificOption1
    global varGameSpecificOption2
    global varGameSpecificOption3
    
    
    return [list [expr {$varGameSpecificOption1 == "butYes"}] \
            [expr {$varGameSpecificOption1 == "butYes"}] \
            [expr {$varGameSpecificOption2 == "knight"}] \
            [expr {$varGameSpecificOption2 == "knight"}] \
            [expr {$varGameSpecificOption3 == "knight"}] \
            [expr {$varGameSpecificOption3 == "knight"}]]
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
    
    ###Isaac's attempts
    ##from dodgem
    global gPosition gSlotList gSlotsX gSlotsY xbmLightGrey
    #gSlotList is new
    global gAgainstComputer gHumanGoesFirst
    
    set thePosition $gPosition

    set trueX $slotX
    set trueY [expr $slotY - 1]
 
    set pos [expr (($gSlotsX - 1) * $trueY) + $trueX]
    ### Now we put the piece on the board
    if {$thePosition / int(pow(2, $i))} {
        if {$gAgainstComputer && !$gHumanGoesFirst } {
            [addMoveTags slotX slotY] 
        }
    }
    
    set theIndex [SinglePieceRemovalCoupleMove $slotX $slotY]
    
    set xPos [getXPos $gPosition]
    set oPos [getOPos $gPosition]
    
    
    $w addtag oPiece $slot [getXCoord oPos] [getYCoord oPos]
    $w addtag xPiece $slot [getXCoord xPos] [getYCoord xPos]
    $w addtag player withTag $oPiece
    $w addtag player withTag $xPiece
    
    ###Put a piece there
    DrawPiece $oPiece "O"
    DrawPiece $xPiece "X"
    ###end of Isaac's attempts

    ### Only the slots you can move to should be active upon "New Game"
   foreach theMoveValue [C_GetValueMoves $gPosition] {
       set theMove [lindex $theMoveValue 0]
       $w addtag tagInitial withtag $theMove
   }

    ### need to turn off the slots that are pieces
    $w delete tagInitial withtag $player
}


#############################################################################
##
## getXPos & getOPos
##
## Isaac: These extract the position of a piece from the board state
##
#############################################################################
proc getXPos {gPosition} {
    return [expr ($gPosition * int(pow(2, (32 - (2*BITSIZE + 1))) / int(pow(2, (BITSIZE + 1)))))]
}
  
proc getXPos {gPosition} {
    return [expr ($gPosition * int(pow(2, (32 - (2*BITSIZE + 1))) / int(pow(2, 1))))]
} 

#############################################################################
##
## getXCoord & getYCoord
##
## Isaac: These extract 1 of 2 dimensions of a piece's position from its 1-d position
##
#############################################################################
proc getYCoord {1DPos} {
 return [expr (1DPos / gSlotsY)] # - 1?
}

proc getXCoord {1DPos} {
 return [expr (1DPos % gSlotsX)]
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

    ### In Basic Joust, the move is just the slot itself.
    ### (position of slot) withtag xPiece
    ### theMove|XPiece Position
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
    if { [Xturn]} {
        DisableSlot withtag xPiece
        addtag tagLastMove withtag xPiece
        addtag xPiece $theSlot
    } else {
        DisableSlot withtag oPiece
        addtag tagLastMove withtag oPiece
        addtag oPiece $theSlot
    }

    DisableSlot withtag aMove
    #not sure if this should be handled here or earlier
    $w destroy withtag tagLastMove 
    #remove the previous slot so no one can move there
    
    #disable all the old possible moves (they still have their pMove tags)
    foreach oldMoves [$w find withtag pMove] {
        set oldMove [lindex $oldMoves 0]
        DisableSlot $w $oldMove 
    }

    delete oldMove withtag oldMove 
    #delete the oldMove tags

    #tag each of the possible moves with pMove and enable them so people can move to them!
    foreach theMoveValue [C_GetValueMoves $gPosition] {
        set aMove [lindex $theMoveValue 0]
        EnableSlot $w $aMove
        addtag pMove withtag $aMove
    }
    ###  DisableSlot $w $theSlot
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
