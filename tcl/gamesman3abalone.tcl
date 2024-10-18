
############################################################################
##
## Source The Utility Functions
##
##
##
#############################################################################

proc stack {} {
    return [list]
}

proc push {l a} {
    return [linsert $l 0 $a]
}

proc pop {l} {
    return [lreplace $l 0 0]
}

proc peek {l} {
    return [lindex $l 0]
}

## returns whether or not the list l contains the key a
proc containskey {a l} {
    set retval false

    foreach item $l {
        if {[lindex $item 0] == $a} {  
            set retval true
        } 
    }
    return $retval
}

##### other utility function

proc MoveValueToColor { moveType value } {
    set color cyan
    if {$moveType == "value"} {
	if {$value == "Tie"} {
	    set color yellow
	} elseif {$value == "Lose"} {
	    set color green
	} else {
	    set color red4
	}
    } elseif {$moveType == "all"} {
	set color cyan
    }
    return $color
}


#############################################################################
##
## InitGlobals
##
## This procedure is used to reserve some keywords. Constants begin with
## 'k' to remember they're constants to make the code easier to read. 
## Constants never change. Nice to know somethings don't change around here.
##
#############################################################################

proc InitGlobals {} {

    ### These are used in the routine that draws the pieces - we want a small
    ### piece used as a label but a larger piece used in the playing board.

    global kBigPiece
    set kBigPiece 1
    
    global kSmallPiece
    set kSmallPiece 0

    ### Set the color and font of the labels (moved to InitWindow.tcl)

    global kLabelColor
    set kLabelColor grey40

    ### Set the animation speed

    global gAnimationSpeed
    set gAnimationSpeed 8
    
    global gMoveType
    set gMoveType all
    
    global gMoveRadioStation
    set gMoveRadioStation all

    global gGameSolved 
    set gGameSolved false

    ### Smarter Computer

    global gSmartness gSmartnessScale
    set gSmartness Perfectly
    set gSmartnessScale 100

    ### ToMove & ToWin
    global gToMove gToWin
    set gToMove "To Move:"
    set gToWin "To Win:"
}


proc SetupGamePieces {} {

    global gLeftPiece gRightPiece gLeftHumanOrComputer gRightHumanOrComputer
    
    global gLeftName gRightName
    global gPiecesPlayersName
    set alist [GS_NameOfPieces]
    
    set gLeftPiece [lindex $alist 0]
    set gRightPiece [lindex $alist 1]
    set gLeftHumanOrComputer Human
    set gRightHumanOrComputer Human
    set gLeftName Player
    set gRightName Hal9000
    
    set gPiecesPlayersName($gLeftPiece) $gLeftName
    set gPiecesPlayersName($gRightPiece) $gRightName

    global gLeftColor gRightColor
    set alist [GS_ColorOfPlayers]
    set gLeftColor [lindex $alist 0]
    set gRightColor [lindex $alist 1]

}



#############################################################################
##
## New Game
##
## This is what we do when the user clicks the 'New Game' button.
##
## Here we set up the globals for the new game and call the game-specific 
## function GS_NewGame to draw the initial screen
##
## Args: None
## 
## Requires: GS_Initialize has been called
##
#############################################################################

proc NewGame { } {

    global gGameSoFar gPosition gInitialPosition gMovesSoFar
    global gLeftName gRightName gWhoseTurn gPlaysFirst
    .middle.f1.cMLeft itemconfigure LeftName \
	-text [format "Left:\n%s" $gLeftName]
    .middle.f3.cMRight itemconfigure RightName \
	-text [format "Right:\n%s" $gRightName]	
    .middle.f1.cMLeft raise LeftName
    .middle.f3.cMRight raise RightName
    update
    set gPosition $gInitialPosition
    if { $gPlaysFirst == 0 } {
        set gWhoseTurn "Left"
    } else {
	set gWhoseTurn "Right"
    }
    set gGameSoFar [list $gInitialPosition]
    set gMovesSoFar [list]
    GS_NewGame .middle.f2.cMain $gPosition
    EnableMoves
    DriverLoop
}

#############################################################################
##
## DriverLoop
##
## Here is where we decide whether to get a move from the database or give
## up control to the user and allow for a human move.  
##
## Args: None
##
## Requires: New Game has been called 
##
#############################################################################

proc DriverLoop { } {
   
    ## retrieve global variables
    global gGameSoFar gMovesSoFar gPosition gWaitingForHuman
    global gMoveDelay gGameDelay gMoveType gGameSolved

    if { !$gGameSolved } {
	return
    }

    set gWaitingForHuman false

    while { [expr !$gWaitingForHuman] } {

	set primitive [C_Primitive $gPosition]

	## Game's over if the position is primitive
	if { $primitive != "Undecided" } {
	
	    set gWaitingForHuman true
	    GameOver $gPosition $primitive [peek $gMovesSoFar]
	    
	} else {

	    global gWhoseTurn gLeftName gRightName
	    .middle.f3.cMRight itemconfigure WhoseTurn \
		-text [format "It's %s's Turn" [subst $[subst g[subst $gWhoseTurn]Name]]]
		update idletasks

	    ## Handle Prediction
	    global gPredString
	    GetPredictions
	    .middle.f3.cMRight itemconfigure Predictions \
		-text [format "Predictions: %s" $gPredString] 
	    update idletasks
       	    
	    if { [PlayerIsComputer] } {
		GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
		after [expr int($gMoveDelay * 1000)]
		GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
		DoComputerMove
		set gWaitingForHuman false
		update
	    } else {
		DoHumanMove
		set gWaitingForHuman true
	    }
	}
    }
}

#############################################################################
##
## SwitchWhoseTurn
##
## Switches from the left player's turn to the right player's and vice-versa
## 
## Args: none
##
## Requires: NewGame has been called
##
#############################################################################

proc SwitchWhoseTurn {} {
    global gWhoseTurn

    if { $gWhoseTurn == "Left" } {
	set gWhoseTurn "Right"
    } else {
	set gWhoseTurn "Left"
    }
}
    
#############################################################################
##
## DoComputerMove
##
## This function gets the computer's move from the database and makes it
## 
## Args: none
##
## Requires: The state of the board is such that there are no moves being
##           shown at this time
##
#############################################################################

proc DoComputerMove { } {

    global gPosition gGameSoFar gMovesSoFar

    set theMove [C_GetComputersMove $gPosition]    

    set oldPosition $gPosition

    set gPosition [C_DoMove $gPosition $theMove]

    set gGameSoFar [push $gGameSoFar $gPosition]

    set gMovesSoFar [push $gMovesSoFar $theMove]

    HandleComputersMove .middle.f2.cMain $oldPosition $theMove $gPosition

    if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	SwitchWhoseTurn
    }

}

#############################################################################
##
## HandleComputersMove
##
## This function handles the  move for a computer player
## This should be overwritten for modules which support n-phase moves,
## with n > 1
##
#############################################################################

proc HandleComputersMove { c oldPos theMove Position } {

  GS_HandleMove $c $oldPos $theMove $Position

}


#############################################################################
##
## DoHumanMove
##
## This function shows all the moves and relinquishes control so that the
## user can input the move
## 
## Args: none
##
## Requires: The state of the board is such that there are no moves being
##           shown at this time
##
#############################################################################

proc DoHumanMove { } {

    global gPosition gMoveType

    GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]

}

#############################################################################
##
## ReturnFromHumanMove
##
## This function is called from the Game Specific Functions and gives
## control back to gamesman
## 
## Args: the Move
##
## Requires: The Moves on the Game Board are still being shown
##
#############################################################################

proc ReturnFromHumanMove { theMove } {
    global gGamePlayable
    if {$gGamePlayable && ![PlayerIsComputer]} {
        ReturnFromHumanMoveHelper $theMove
    }
}

proc ReturnFromHumanMoveHelper { theMove } {
        
    global gPosition gGameSoFar gMovesSoFar gMoveType

    set primitive [C_Primitive $gPosition]

    set PositionValueList [C_GetValueMoves $gPosition]

    if { $primitive == "Undecided" &&
         [containskey $theMove $PositionValueList] } {
        
        GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
        
        set oldPosition $gPosition
                
        set gPosition [C_DoMove $gPosition $theMove]
                
        set gGameSoFar [push $gGameSoFar $gPosition]
        
        set gMovesSoFar [push $gMovesSoFar $theMove]
                
        GS_HandleMove .middle.f2.cMain $oldPosition $theMove $gPosition

	if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	    SwitchWhoseTurn
	}

        DriverLoop

    }

}


#############################################################################
##
## GameOver
##
## This function is called from the driver loop to signify a game is over.
## 
## Args: the position, the Game Value (win, lose or tie)
##
## Requires: The Moves on the Game Board are not shown
##
#############################################################################

proc GameOver { position gameValue lastMove } {

    global gPosition gGameSoFar gWhoseTurn gLeftName gRightName
    global gLeftPiece gRightPiece
    global gGameDelay gLeftHumanOrComputer gRightHumanOrComputer

    set previousPos [peek [pop $gGameSoFar]]

    set WhoWon Nobody

    set WhichPieceWon Nobody

    if { $gWhoseTurn == "Right" } {

        if { $gameValue == "Win" } {
            
            set WhoWon $gRightName
	    set WhoLost $gLeftName
            set WhichPieceWon $gRightPiece

        } elseif { $gameValue == "Lose" } {

            set WhoWon $gLeftName
	    set WhoLost $gRightName
            set WhichPieceWon $gLeftPiece

        }

    } else {
        
        if { $gameValue == "Win" } {
            
            set WhoWon $gLeftName
	    set WhoLost $gRightName
            set WhichPieceWon $gLeftPiece

        } elseif { $gameValue == "Lose" } {

            set WhoWon $gRightName
	    set WhoLost $gLeftName
            set WhichPieceWon $gRightPiece

        }
    }

    if { $gameValue == "Tie" } {
        set message [concat GAME OVER: It's a TIE!]
        SendMessage $message
	set loseMessage "Nobody wins!"
    } else {
        set message [concat GAME OVER: $WhoWon wins!]
	SendMessage $message
	set loseMessage [format "%s loses!" $WhoLost]
    }

    .middle.f3.cMRight itemconfigure WhoseTurn \
	-text [format "%s" $message]
    
    .middle.f3.cMRight itemconfigure Predictions \
	-text [format "%s" $loseMessage] 
    update idletasks

    GS_GameOver .middle.f2.cMain $gPosition $gameValue $WhichPieceWon $WhoWon $lastMove
    update idletasks

    DisableMoves

    if { $gLeftHumanOrComputer == "Computer" && $gRightHumanOrComputer == "Computer" } {
	after [expr int($gGameDelay * 1000)]
	TBaction1
    }
}


#############################################################################
##
## DisableMoves/EnableMoves
##
## Disables or Enables the radiobuttons which show All/Value/Best Moves
## 
## Args: None
##
## Requires: Nothing
##
#############################################################################

proc DisableMoves {} {

#   .winPlayOptions.fRadio.butAllMoves configure -state disabled
#   .winPlayOptions.fRadio.butValueMoves configure -state disabled
#   .winPlayOptions.fRadio.butBestMoves configure -state disabled

}

proc EnableMoves {} {

#    .winPlayOptions.fRadio.butAllMoves configure -state normal
#    .winPlayOptions.fRadio.butValueMoves configure -state normal
#    .winPlayOptions.fRadio.butBestMoves configure -state normal

}

#############################################################################
##
## ToggleMoves
##
## Called When Changing Between All/Value/Best Moves
##
## Args:  A string corresponding to all, value, or best
##
## Requires: Moves are currently shown and Game is not over
##
#############################################################################

proc ToggleMoves { moveType } {

    global gMoveType gPosition

    ChangeMoveType $gMoveType $moveType $gPosition [C_GetValueMoves $gPosition]

    set gMoveType $moveType

}

#############################################################################
##
## ChangeMoveType
##
## Called when Changing Between All/Value/Best Moves
## 
## Args: 
##
## Requires: Moves are currently shown
##
#############################################################################

proc ChangeMoveType { fromMoveType toMoveType position moveList } {

    global gWaitingForHuman

    GS_HideMoves .middle.f2.cMain $fromMoveType $position $moveList

    GS_ShowMoves .middle.f2.cMain $toMoveType $position $moveList

}

#############################################################################
##
## Undo
##
## Calls Game Specific Undo to undo the last move
## 
## Args: none
##
## Requires: Moves are currently shown
##
#############################################################################

proc Undo { } {
    
    UndoHelper
    global gWhoseTurn gLeftName gRightName
    .middle.f3.cMRight itemconfigure WhoseTurn \
	-text [format "It's %s's Turn" [subst $[subst g[subst $gWhoseTurn]Name]]]
    GetPredictions
    global gPredString
    .middle.f3.cMRight itemconfigure Predictions \
	    -text [format "Predictions: %s" $gPredString] 
    update idletasks
    
    DriverLoop
}

proc UndoHelper { } {
    
    global gPosition gMovesSoFar gGameSoFar gMoveType
    
    if { [llength $gGameSoFar] != 1 } {
        
        set primitive [C_Primitive $gPosition]
        
        if { $primitive == "Undecided" } {
            
            GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
            
        } else {

            GS_UndoGameOver .middle.f2.cMain $gPosition

        }
        
        set undoOnce [pop $gGameSoFar]
        
        GS_HandleUndo .middle.f2.cMain [peek $gGameSoFar] [peek $gMovesSoFar] [peek $undoOnce]
        
        set gGameSoFar $undoOnce
        
        set gPosition [peek $gGameSoFar]
        
	set undoneMove [peek $gMovesSoFar]
        set gMovesSoFar [pop $gMovesSoFar]

	if { [expr ![C_GoAgain $gPosition $undoneMove]] } {
	    SwitchWhoseTurn
	}
        
        if { [PlayerIsComputer] } {
            
            UndoHelper

	}
        
    }

}

#############################################################################
##
## PlayerIsComputer
##
## Returns true or false if the player whose turn it is currently is controlled
## by a computer
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc PlayerIsComputer { } {

    global gWhoseTurn gRightHumanOrComputer gLeftHumanOrComputer

    if { $gWhoseTurn == "Left" } {
        return [expr { $gLeftHumanOrComputer == "Computer"}]
    } else {
        return [expr { $gRightHumanOrComputer == "Computer"}]
    }
}

proc ReturnToGameSpecificOptions {} {
    GS_Deinitialize .middle.f2.cMain
    global gGameSolved
    set gGameSolved false
}

#############################################################################
##
## SendMessage
##
## Displays whose turn it is or who's won
## 
## Args: string
##
## Requires: Nothing
##
#############################################################################

proc SendMessage { arg } {
    
}

#############################################################################
##
## GetPredictions
##
## Get predictions from the C code
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc GetPredictions {} {

    global gPosition gPredString gWhoseTurn
    global gLeftName gRightName

    if { [C_Primitive $gPosition] != "Undecided" } {
	set gPredString ""
    } else {

	### Get the value, the player and set the prediction
	set theValue      [C_GetValueOfPosition $gPosition]
	set theRemoteness [C_Remoteness $gPosition]
	set theMex        [C_Mex $gPosition]
	
	if { $gWhoseTurn == "Left" } {
	    set playersName $gLeftName
	} else {
	    set playersName $gRightName
	}
	
	set prediction [format "%s should " $playersName]
	
	if { $theValue == "Tie" && $theRemoteness == 255 } {
	    set prediction [concat $prediction "Draw"]
	} else {
	    set prediction [concat $prediction [format "%s in %s" $theValue $theRemoteness]]
	}

	set prediction [concat $prediction " " $theMex]

	### And place it in the field if the button is on.
	set gPredString $prediction
    }
}

# argv etc
proc main {kRootDir} {
    # Initialize the C backend
    C_Initialize
    C_InitializeDatabases

    # Initialize generic top-level window
    source "$kRootDir/../tcl/InitWindow.tcl"
    InitGlobals

    global gSkinsDir
    if {[catch {set fileptr [open skin.txt r]}]} {
	set gSkinsDir OxySkin_HiRes/
    } else {
	gets $fileptr gSkinsDir
	if {![file isdirectory "$kRootDir/../tcl/skins/$gSkinsDir"]} {
	    set gSkinsDir OxySkin_HiRes/
	}
	close $fileptr
    }

    InitWindow $kRootDir ppm

    # Initialize game-specific globals and frame
    GS_InitGameSpecific
    GS_Initialize .middle.f2.cMain

    # Set the window title
    global kGameName
    wm title . "$kGameName - GAMESMAN"

    # Generate game-specific About, Help, and Skins frames
    global gFrameWidth
    SetupAboutFrame .middle.f2.fAbout.content $gFrameWidth
    SetupHelpFrame .middle.f2.fHelp.content $gFrameWidth
    SetupSkinsFrame .middle.f2.fSkins.content $gFrameWidth
}

