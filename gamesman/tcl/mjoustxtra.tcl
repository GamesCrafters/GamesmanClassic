




#############################################################################
##
## HandleMove
##
## Ok, the user has just clicked to make a move. Now what do we do? Wehave
## to update the internals, change the board, and call for a computer move
## if needed.
##
#############################################################################

proc HandleMove { theMove } {
  global gPlayerOneTurn gPosition gSlotSize gAgainstComputer 
    gSlotList
    global gSlotsX gSlotsY kInteractionType
    
    set w .winBoard.c
    
    if { $kInteractionType == "SinglePieceRemoval" } {
	
	### Get the slot's id
	
	set slotX [expr $theMove % $gSlotsX]
	set slotY [expr $theMove / $gSlotsX]
	set theSlot $gSlotList($slotX,$slotY)
	
    } elseif { $kInteractionType == "MultiplePieceRemoval" } {
	
	### Get the slot's id
	
	set theSlot dummyArgNeverSeen
	
	$w itemconfig tagPiece -fill magenta
	
    } elseif { $kInteractionType == "Rearranger" } {
	
	### Get the slot's id
	
	set theSlot dummyArgNeverSeen
	
	#$w itemconfig tagPiece -fill magenta
	
    } elseif { $kInteractionType == "Joust" } {
	
	### Get the slot's id
	
	set slotX [expr $theMove % $gSlotsX]
	set slotY [expr $theMove / $gSlotsX]
	set theSlot $gSlotList($slotX,$slotY)
	
    } else {
	BadElse "HandleMove (kInteractionType)"
    }
    
    ### Enable or Disable slots as a result of the move
    
    GS_HandleEnablesDisables $w $theSlot [GS_ConvertInteractionToMove 
    $theMove]
    
    ### Set the piece depending on whose turn it is and draw it.
    
    if { $kInteractionType == "SinglePieceRemoval" } {
	
	if { $gPlayerOneTurn } { set thePiece "X" } { set thePiece "O" }
	
	DrawPiece $slotX $slotY $thePiece
	
    } elseif { $kInteractionType == "MultiplePieceRemoval" } {
	
	### Don't need to do anything here.
	
    } elseif { $kInteractionType == "Rearranger" } {
	
	### Don't need to do anything here.
    } elseif { $kInteractionType == "Joust" } {
	
	if { $gPlayerOneTurn } { set thePiece "X" } { set thePiece "O" }
	
	DrawPiece $slotX $slotY $thePiece
	
    } else {
	
	BadElse "HandleMove (kInteractionType)"
	
    }
    
    ### Swap turns
    
    set gPlayerOneTurn [not $gPlayerOneTurn]
    
    if { $kInteractionType == "SinglePieceRemoval" } {
	
	### Convert the slots clicked to a move
	
	set theMoveForTheCProc [GS_ConvertInteractionToMove \
		[SinglePieceRemovalCoupleMove $slotX $slotY]]
    } elseif { $kInteractionType == "Joust" } {
	
	### Convert the slots clicked to a move
	##ISAAC: could make a change to include the old spot for the burn 
	here!
	set theMoveForTheCProc [GS_ConvertInteractionToMove \
		[SinglePieceRemovalCoupleMove $slotX $slotY]]
    } else {
	set theMoveForTheCProc [GS_ConvertInteractionToMove $theMove]
    }
    
    ### Do the move and update the position
    
    set gPosition [C_DoMove $gPosition $theMoveForTheCProc]
    
    ### Get the new value
    
    set theValue [C_Primitive $gPosition]
    
    ### And, depending on the value of the game...
    
    if { $theValue != "Undecided" } { 

	HandleGameOver $w $theValue
	
    } elseif { [not $gAgainstComputer] } {
	
	### Do nothing. It was all taken care of for us already.
	### And it's now the next person's turn.
	
    } else { 
	
	### It's now time to do the computer's move.
	
	DoComputersMove
	
	### We return because 'DoComputersMove' does the three Handlers 
	already
	
	return
    }
    
    ### Handle the standard things after a move
    
    HandleMoves
    HandlePredictions
    HandleTurn
}


#############################################################################
##
## DrawMove
##
## Depending on the InteractionType, call different DrawMove routines
##
#############################################################################

proc DrawMove { w theMoveArg color drawBig } {
    
    global kInteractionType
    
    if { $kInteractionType == "SinglePieceRemoval" } {
	DrawMoveSinglePieceRemoval $w $theMoveArg $color $drawBig
    } elseif { $kInteractionType == "MultiplePieceRemoval" } {
	DrawMoveMultiplePieceRemoval $w $theMoveArg $color $drawBig
    } elseif { $kInteractionType == "Rearranger" } {
	DrawMoveRearranger $w $theMoveArg $color $drawBig
    } elseif { $kInteractionType == "Joust" } {
	DrawMoveJoust $w $theMoveArg $color $drawBig
    } else  {
	BadElse "DrawMove (kInteractionType)"
    }
}

#############################################################################
##
## DrawMoveJoust
##
## Here we show a user's move, graphically. The color might be generic or
## it might represent a value. If drawBig is true, we draw it the full size
## of the slot. Otherwise we draw it 1/5 of the size of the slot.
##
#############################################################################

proc DrawMoveJoust { w theMoveArg color drawBig } {
    global gSlotSize gSlotsX gSlotsY
    
    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]
    
    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]
    
    ### Ok, if it's going to go in the "Values" window, let it be a 
    circle
    
    if { !$drawBig } {
	set theMove [$w create oval 0 0 $circleSize $circleSize \
		-outline $color \
		-fill $color]
    } else {
	
	set theMoveArgAbsolute [GS_ConvertToAbsoluteMove $theMoveArg]
	
	set theMove [CreateArrow $w $theMoveArgAbsolute $gSlotSize $gSlotsX 
	$gSlotsY $color]
	
	$w addtag tagMoves withtag $theMove
	$w bind $theMove <1> "HandleMove \{ $theMoveArgAbsolute \}"
	$w bind $theMove <Enter> "$w itemconfig $theMove -fill black"
	$w bind $theMove <Leave> "$w itemconfig $theMove -fill $color"
    }
}

#############################################################################
##
## DrawPiece
##
## Draw a piece at the slot specified by slotX and slotY
##
#############################################################################

proc DrawPiece { slotX slotY thePiece } {
    global kBigPiece kLeftDrawProc kRightDrawProc kBothDrawProc
    
    if     { $thePiece == "X" } {
	
	##ISAAC: burn the old spot
	##$w itemconfig oldX -fill magenta
	##ISAAC: tag the new position for later burning
	##addtag oldX tagPieceONCoord$slotX$slotY 
	return [$kLeftDrawProc  .winBoard.c $slotX $slotY tagPiece blue 
	$kBigPiece]
    } elseif { $thePiece == "O" } {
	
	##ISAAC: burn the old spot
	##$w itemconfig oldO -fill magenta
	##ISAAC: tag the new position for later burning
	##addtag oldO tagPieceONCoord$slotX$slotY 
	return [$kRightDrawProc .winBoard.c $slotX $slotY tagPiece red 
	$kBigPiece]
	
    } elseif { $thePiece == "+" } { 
	return [$kBothDrawProc .winBoard.c $slotX $slotY tagPiece magenta 
	$kBigPiece]
    } elseif { $thePiece == "-" } { 
	return [DrawLastMove .winBoard.c $slotX $slotY tagPiece magenta 
	$kBigPiece]
    } else {
	BadElse DrawPiece
    }
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
    
    global gPlayerOneTurn gPosition gSlotSize gSlotList gSlotsX gSlotsY
    global kInteractionType
    set w .winBoard.c
    
    ### Set the piece for the computer and get the move.
    
    if { $gPlayerOneTurn } { set thePiece "X" } { set thePiece "O" }
    set theMove  [C_GetComputersMove $gPosition]
    set theBoardMove [GS_ConvertMoveToInteraction $theMove]
    set theMoveX [expr $theBoardMove % $gSlotsX]
    set theMoveY [expr $theBoardMove / $gSlotsX]
    
    if { $kInteractionType == "SinglePieceRemoval" } {
	
	### Get the id of the slot from the slotlist
	
	set theSlot $gSlotList($theMoveX,$theMoveY)
	
	### Draw a piece there.
	
	DrawPiece $theMoveX $theMoveY $thePiece
	
    } elseif { $kInteractionType == "MultiplePieceRemoval" } {
	set theSlot dummyArgNeverSeen
    } elseif { $kInteractionType == "Rearranger" } {
	set theSlot dummyArgNeverSeen
    } elseif { $kInteractionType == "Joust" } {
	
	### Get the id of the slot from the slotlist
	
	set theSlot $gSlotList($theMoveX,$theMoveY)
	
	### Draw a piece there.
	
	DrawPiece $theMoveX $theMoveY $thePiece
	
    } else {
	BadElse "DoComputersMove (kInteractionType)"
    }
    
    ### Enable or Disable slots as a result of the move
    
    GS_HandleEnablesDisables $w $theSlot $theMove
    
    ### Do the move and check out the position.
    
    set gPosition [C_DoMove $gPosition $theMove]
    set theValue [C_GetValueOfPosition $gPosition]
    
    ### Alternate turns
    
    set gPlayerOneTurn [not $gPlayerOneTurn]
    
    ### If the game is over (the primitive value is Undecided), handle 
    it
    
    if { [C_Primitive $gPosition] != "Undecided" } { 
	HandleGameOver $w $theValue
    }
    
    ### Update the standard fields (Predictions, Turns & Moves)
    
    HandlePredictions
    HandleTurn
    HandleMoves
}

##Isaac: Need to alter these!
##DrawMove #automatically called on the entire list of possible moves

##DoComputersMove

##HandleMove

##tagPieceONCoord$slotX$slotY returns slot that was tagged by DrawCircle




