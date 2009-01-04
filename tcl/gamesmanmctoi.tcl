#############################################################################
##
## DrawMove
## all different DrawMove routines
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
    } elseif { $kInteractionType == "ChungToi" } {
	DrawMoveChungToi $w $theMoveArg $color $drawBig
    } else {
	BadElse "DrawMove (kInteractionType)"
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
    global kInteractionType gAnimationSpeed
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
    } elseif { $kInteractionType == "ChungToi" } {
	set theSlot dummyArgNeverSeen
	
	set from      [expr $theMove / 100]
	set to        [expr ($theMove - (100 * $from)) / 10]
	set fromSlotX [expr $from % $gSlotsX]
	set fromSlotY [expr $from / $gSlotsX]
	set toSlotX   [expr $to % $gSlotsX]
	set toSlotY   [expr $to / $gSlotsX]
	set orientation [expr ($theMove - (100 * $from) - (10 * $to))]
	
	if {$from == 9} {
	    if { $gPlayerOneTurn} {
		if { $orientation == 1} {
		    set thePiece "L+" 
		} else { 
		    set thePiece "LX"
		}
	    } elseif { [not $gPlayerOneTurn] } {
		if { $orientation == 1} {
		    set thePiece "R+" 
		} else { 
		    set thePiece "RX"
		}
	    } else {
		BadElse "HandleMove (theMove)"
	    }
	    set slotX [expr (($theMove - 900) / 10)  % $gSlotsX]
	    set slotY [expr (($theMove - 900) / 10)  / $gSlotsX]
	    DrawPiece $slotX $slotY $thePiece
	} elseif { $from == $to } {
	    set slotX $fromSlotX
	    set slotY $fromSlotY
	    if { $gPlayerOneTurn} {
		if { $orientation == 1} {
		    set thePiece "L+" 
		} else { 
		    set thePiece "LX"
		}
	    } elseif { [not $gPlayerOneTurn] } {
		if { $orientation == 1} {
		    set thePiece "R+" 
		} else { 
		    set thePiece "RX"
		}
	    } else {
		BadElse "HandleMove (theMove)"
	    }
	    $w delete tagPieceOnCoord$slotX$slotY
	    DrawPiece $slotX $slotY $thePiece

	} elseif {[not [expr $from == $to]]} {
	    
	   
	   
	    ### Try a little animation
	    
	    set timeSlices [expr int(($gSlotSize + 0.0) / ($gAnimationSpeed + 0.0))]
	    
	    set theIncrementalMoveX [expr ($toSlotX - $fromSlotX) * (($gSlotSize + 0.0) / $timeSlices)]
	    set theIncrementalMoveY [expr ($toSlotY - $fromSlotY) * (($gSlotSize + 0.0) / $timeSlices)]
	    set theOverallMoveX [expr $timeSlices * $theIncrementalMoveX]
	    set theOverallMoveY [expr $timeSlices * $theIncrementalMoveY]
	    
	    DeleteMoves


	    
	    for {set i 1} { $i <= $timeSlices } {incr i} {
		$w move tagPieceOnCoord$fromSlotX$fromSlotY \
			$theIncrementalMoveX $theIncrementalMoveY
		update idletasks
	    }

	    
	    ### Update our internal markers
	    
	    if { $gPlayerOneTurn} {
		if { $orientation == 1} {
		    set thePiece "L+" 
		} else { 
		    set thePiece "LX"
		}
	    } elseif { [not $gPlayerOneTurn] } {
		if { $orientation == 1} {
		    set thePiece "R+" 
		} else { 
		    set thePiece "RX"
		}
	    } else {
		BadElse "HandleMove (theMove)"
	    }
	    $w delete tagPieceOnCoord$fromSlotX$fromSlotY
	    DrawPiece $toSlotX $toSlotY $thePiece


	}


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

    ### If the game is over (the primitive value is Undecided), handle it

    if { [C_Primitive $gPosition] != "Undecided" } { 
	HandleGameOver $w $theValue
    }

    ### Update the standard fields (Predictions, Turns & Moves)

    HandlePredictions
    HandleTurn
    HandleMoves
}

#############################################################################
##
## HandleMove
##
## Ok, the user has just clicked to make a move. Now what do we do? We have
## to update the internals, change the board, and call for a computer move
## if needed.
##
#############################################################################

proc HandleMove { theMove } {
    global gPlayerOneTurn gPosition gSlotSize gAgainstComputer gSlotList
    global gSlotsX gSlotsY kInteractionType gAnimationSpeed

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
    
    } elseif { $kInteractionType == "ChungToi" } {

	### Get the slot's id

	
	
	set theSlot dummyArgNeverSeen

	#$w itemconfig tagPiece -fill magenta

    } else {
	BadElse "HandleMove (kInteractionType)"
    }

    ### Enable or Disable slots as a result of the move
    
    GS_HandleEnablesDisables $w $theSlot [GS_ConvertInteractionToMove $theMove]

    ### Set the piece depending on whose turn it is and draw it.

    if { $kInteractionType == "SinglePieceRemoval" } {

	if { $gPlayerOneTurn } { set thePiece "X" } { set thePiece "O" }

	DrawPiece $slotX $slotY $thePiece

    } elseif { $kInteractionType == "MultiplePieceRemoval" } {

	### Don't need to do anything here.

    } elseif { $kInteractionType == "Rearranger" } {

	### Don't need to do anything here.

    } elseif { $kInteractionType == "ChungToi" } {
	
	if { $theMove == "041" } {
	    set theMove "41"
	}
	
	if { $theMove == "040"} {
	    set theMove "40"
	}
	set theAbsoluteMoveArg [GS_ConvertToAbsoluteMove $theMove]
	
	set from [lindex $theAbsoluteMoveArg 0]
	set to   [lindex $theAbsoluteMoveArg 1]
	set orientation [lindex $theAbsoluteMoveArg 2]
	
	set fromSlotX [expr $from % $gSlotsX]
	set fromSlotY [expr $from / $gSlotsX]
	set toSlotX   [expr $to % $gSlotsX]
	set toSlotY   [expr $to / $gSlotsX]


	if {$from == 9} {
	    if { $gPlayerOneTurn} {
		if { $orientation == 1} {
		    set thePiece "L+" 
		} else { 
		    set thePiece "LX"
		}
	    } elseif { [not $gPlayerOneTurn] } {
		if { $orientation == 1} {
		    set thePiece "R+" 
		} else { 
		    set thePiece "RX"
		}
	    } else {
		BadElse "HandleMove (theMove)"
	    }
	    set slotX [expr (($theMove - 900) / 10)  % $gSlotsX]
	    set slotY [expr (($theMove - 900) / 10)  / $gSlotsX]
	    DrawPiece $slotX $slotY $thePiece
	} elseif { $from == $to } {
	    set slotX $fromSlotX
	    set slotY $fromSlotY
	    if { $gPlayerOneTurn} {
		if { $orientation == 1} {
		    set thePiece "L+" 
		} else { 
		    set thePiece "LX"
		}
	    } elseif { [not $gPlayerOneTurn] } {
		if { $orientation == 1} {
		    set thePiece "R+" 
		} else { 
		    set thePiece "RX"
		}
	    } else {
		BadElse "HandleMove (theMove)"
	    }
	    $w delete tagPieceOnCoord$slotX$slotY
	    DrawPiece $slotX $slotY $thePiece

	} elseif {[not [expr $from == $to]]} {
	    
	   
	   
	    ### Try a little animation


	    
	    set timeSlices [expr int(($gSlotSize + 0.0) / ($gAnimationSpeed + 0.0))]
	    
	    set theIncrementalMoveX [expr ($toSlotX - $fromSlotX) * (($gSlotSize + 0.0) / $timeSlices)]
	    set theIncrementalMoveY [expr ($toSlotY - $fromSlotY) * (($gSlotSize + 0.0) / $timeSlices)]
	    set theOverallMoveX [expr $timeSlices * $theIncrementalMoveX]
	    set theOverallMoveY [expr $timeSlices * $theIncrementalMoveY]
	    
	    DeleteMoves
	    
	    for {set i 1} { $i <= $timeSlices } {incr i} {
		$w move tagPieceOnCoord$fromSlotX$fromSlotY \
			$theIncrementalMoveX $theIncrementalMoveY
		update idletasks
	    }

	    
	    ### Update our internal markers
	    

	    if { $gPlayerOneTurn} {
		if { $orientation == 1} {
		    set thePiece "L+" 
		} else { 
		    set thePiece "LX"
		}
	    } elseif { [not $gPlayerOneTurn] } {
		if { $orientation == 1} {
		    set thePiece "R+" 
		} else { 
		    set thePiece "RX"
		}
	    } else {
		BadElse "HandleMove (theMove)"
	    }
	    $w delete tagPieceOnCoord$fromSlotX$fromSlotY
	    DrawPiece $toSlotX $toSlotY $thePiece


	}
    } else {

	BadElse "HandleMove (kInteractionType)"

    }

    ### Swap turns

    set gPlayerOneTurn [not $gPlayerOneTurn]

    if { $kInteractionType == "SinglePieceRemoval" } {

	### Convert the slots clicked to a move
	
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
	
	### We return because 'DoComputersMove' does the three Handlers already

	return
    }

    ### Handle the standard things after a move

    HandleMoves
    HandlePredictions
    HandleTurn
}

#############################################################################
##
## DrawMoveChungtoi
##
## 
## 
## 
##
#############################################################################

proc DrawMoveChungToi { w theMoveArg color drawBig } {

    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    
    
    if {$drawBig} {
	set theAbsoluteMoveArg [GS_ConvertToAbsoluteMove $theMoveArg]
	set fromSlot [lindex $theAbsoluteMoveArg 0]
	set toSlot   [lindex $theAbsoluteMoveArg 1]
	set orientation [lindex $theAbsoluteMoveArg 2]
    }
    if {[not $drawBig]} {
	DrawMoveChungToiSmall $w $theMoveArg $color $drawBig
    } elseif { $fromSlot == 9 } {
	DrawMoveChungToiPlacing $w $theMoveArg $color $drawBig
    } elseif { $fromSlot == $toSlot } {
	DrawMoveChungToiTwisting $w $theMoveArg $color $drawBig
    } elseif { $fromSlot != $toSlot } {
	DrawMoveChungToiHopping $w $theMoveArg $color $drawBig
    } else {
	BadElse "DrawMoveChungToi"
    }
}


#############################################################################
##
## DrawMoveChungToiSmall
##
## 
## 
## 
##
#############################################################################
proc DrawMoveChungToiSmall { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]



    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]

    set theMove [$w create oval 0 0 $circleSize $circleSize \
	    -outline $color \
	    -fill $color]
}

#############################################################################
##
## DrawMoveChungToiPlacing
##
## 
## 
## 
##
#############################################################################
proc DrawMoveChungToiPlacing { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set theAbsoluteMoveArg [GS_ConvertToAbsoluteMove $theMoveArg] 
    
    set to      [expr [lindex $theAbsoluteMoveArg 1]]
    set slotX   [expr ($to % $gSlotsX)]
    set slotY   [expr ($to / $gSlotsX)]
    set orientation [expr [lindex $theAbsoluteMoveArg 2]]
    
    if { $orientation == 1 } {
	set theMove [DrawPlus $w $slotX $slotY tagDummyForMoves $color $drawBig]
    } else {
	set theMove [DrawCross $w $slotX $slotY tagDummyForMoves $color $drawBig]
    }
	
    

    
    
    $w addtag tagMoves withtag $theMove
    
    $w addtag tagMoveAlive withtag $theMove
    $w bind $theMove <1> "HandleMove \{$theMoveArg\}"
    
    $w bind tagMoveAlive <Enter> { .winBoard.c itemconfig current -fill black }
    $w bind $theMove <Leave> "$w itemconfig $theMove -fill $color"

}


#############################################################################
##
## DrawMoveChungToiHopping
##
## 
## 
## 
##
#############################################################################   
proc DrawMoveChungToiHopping { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set theAbsoluteMoveArg [GS_ConvertToAbsoluteMove $theMoveArg] 



    set theMove [CreateHoppingArrow $w $theAbsoluteMoveArg $gSlotSize $gSlotsX $gSlotsY $color]

    $w addtag tagMoves withtag $theMove
    
    $w addtag tagMoveAlive withtag $theMove
    $w bind $theMove <1> "HandleMove \{$theMoveArg\}"


}

#############################################################################
##
## DrawMoveChungToiTwisting
##
## 
## 
## 
##
#############################################################################   
proc DrawMoveChungToiTwisting { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set theAbsoluteMoveArg [GS_ConvertToAbsoluteMove $theMoveArg] 

    set slotX   [expr [lindex $theAbsoluteMoveArg 1] % $gSlotsX]
    set slotY   [expr [lindex $theAbsoluteMoveArg 1] / $gSlotsX]
    set orientation [expr [lindex $theAbsoluteMoveArg 2]]
    
    if { $orientation == 1 } {
	set theMove [DrawPlus $w $slotX $slotY tagDummyForMoves $color $drawBig]
    } else {
	set theMove [DrawCross $w $slotX $slotY tagDummyForMoves $color $drawBig]
    }
    

    $w addtag tagMoves withtag $theMove
    
    $w addtag tagMoveAlive withtag $theMove
    $w bind $theMove <1> "HandleMove \{$theMoveArg\}"
    
    $w bind tagMoveAlive <Enter> { .winBoard.c itemconfig current -fill black}
    
    $w bind $theMove <Leave> "$w itemconfig $theMove -fill $color"
}

#############################################################################
##
## DrawPlus
##
## Here we draw a + on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################
proc DrawPlus { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 7.0)]

    set startPlus [expr $theSlotSize/2.0]
    set halfPlus  [expr $theSlotSize/2.0]
    set endPlus   [expr $startPlus*7.0]

    set thickness  [expr $theSlotSize/5.0  ]
    set x1 [expr $theSlotSize * 4 / 7 ]
    set x2 [expr $theSlotSize * 3 / 7 ]
    set x3 [expr $theSlotSize * 3 / 7 ]
    set x4 [expr $theSlotSize * 1 / 10 ]
    set x5 [expr $theSlotSize * 1 / 10 ]
    set x6 [expr $theSlotSize * 3 / 7 ]
    set x7 [expr $theSlotSize * 3 / 7 ]
    set x8 [expr $theSlotSize * 4 / 7 ]
    set x9 [expr $theSlotSize * 4 / 7 ]
    set x10 [expr $theSlotSize * 9 / 10]
    set x11 [expr $theSlotSize * 9 / 10]
    set x12 [expr $theSlotSize * 4 / 7 ]
    

    set y1 [expr $theSlotSize * 1 / 10]
    set y2 [expr $theSlotSize * 1 / 10]
    set y3 [expr $theSlotSize * 17 / 40] 
    set y4 [expr $theSlotSize * 17 / 40] 
    set y5 [expr $theSlotSize * 23 / 40]
    set y6 [expr $theSlotSize * 23 / 40]
    set y7 [expr $theSlotSize * 9 / 10]
    set y8 [expr $theSlotSize * 9 / 10]
    set y9 [expr $theSlotSize * 23 / 40]
    set y10 [expr $theSlotSize * 23 / 40]
    set y11 [expr $theSlotSize * 17 / 40]
    set y12 [expr $theSlotSize * 17 / 40]
    

    set thePlus [$w create polygon \
		   $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 \
		   $x7 $y7 $x8 $y8 $x9 $y9 $x10 $y10 $x11 $y11 \
		   $x12 $y12 \
		   -fill $theColor \
		   -tag $theTag]

     if { $drawBig } {
	 
	 set cornerX    [expr $slotX*$theSlotSize]
	 set cornerY    [expr $slotY*$theSlotSize]
	 $w move $thePlus $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $thePlus

    return $thePlus
}

#############################################################################
##
## DrawPiece
##
## Draw a piece at the slot specified by slotX and slotY
##
#############################################################################
proc DrawPiece { slotX slotY thePiece } {
    global kBigPiece kLeftDrawProc kRightDrawProc kBothDrawProc kLeftCrossDrawProc kLeftPlusDrawProc kRightCrossDrawProc kRightPlusDrawProc

    if     { $thePiece == "X" } {
	return [$kLeftDrawProc  .winBoard.c $slotX $slotY tagPiece blue $kBigPiece]
    } elseif { $thePiece == "O" } { 
	return [$kRightDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece]
    } elseif { $thePiece == "+" } { 
	return [$kBothDrawProc .winBoard.c $slotX $slotY tagPiece magenta $kBigPiece]
    } elseif { $thePiece == "-" } { 
	return [DrawLastMove .winBoard.c $slotX $slotY tagPiece magenta $kBigPiece]
    } elseif { $thePiece == "LX" } { 
	return [$kLeftCrossDrawProc .winBoard.c $slotX $slotY tagPiece blue $kBigPiece]
    } elseif { $thePiece == "L+" } { 
	return [$kLeftPlusDrawProc .winBoard.c $slotX $slotY tagPiece blue $kBigPiece]
    } elseif { $thePiece == "RX" } { 
	return [$kRightCrossDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece]
    } elseif { $thePiece == "R+" } { 
	return [$kRightPlusDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece]
    } else {
	BadElse DrawPiece
    }
}


#############################################################################
##
## CreateHoppingArrow
##
## Create and return arrow at the specified slots.
##
#############################################################################

proc CreateHoppingArrow { w theMoveArgAbsolute theSlotSize theSlotsX theSlotsY color } {

    global gSlotsX gSlotSize

    set theLineGap      [expr $gSlotSize / 7]

    set theLineWidth    [expr int($theSlotSize / 50)]
    
    set from [lindex $theMoveArgAbsolute 0]
    set to   [lindex $theMoveArgAbsolute 1]
    set orientation [lindex $theMoveArgAbsolute 2]


    set fromX [expr $from % $gSlotsX]
    set fromY [expr $from / $gSlotsX]
    set toX   [expr $to % $gSlotsX]
    set toY   [expr $to / $gSlotsX]
    
    set fromXOffset [expr ($fromX * $theSlotSize)]
    set fromYOffset [expr ($fromY * $theSlotSize)]
    set toXOffset [expr ($toX * $theSlotSize)] 
    set toYOffset [expr ($toY * $theSlotSize)]

    set isitCross [expr $orientation == 0]

      
    
    ######################################################################

    # Calculations to figure out where arrowheads, lines should go

    #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  
   
    if { [expr $from > $to] } {
	set largerSlot   $from
	set smallerSlot  $to
	set minYOffset [expr $toY * $gSlotSize]
	set minXOffset [expr $toX * $gSlotSize]
	set maxYOffset [expr $fromY * $gSlotSize]
	set maxXOffset [expr $fromX * $gSlotSize]
    } else {
	set largerSlot   $to
	set smallerSlot  $from
	set minYOffset [expr $fromY * $gSlotSize]
	set minXOffset [expr $fromX * $gSlotSize]
	set maxYOffset [expr $toY * $gSlotSize]
	set maxXOffset [expr $toX * $gSlotSize]
    }
    
    ### Figure out the dimensions and location of the line object, properly rotated. We always draw the line from the smaller-numbered slot to the lower one
    if { [expr ($largerSlot == 8 && (( $smallerSlot == 6) || ( $smallerSlot == 2))) || ($smallerSlot == 0 && (($largerSlot == 6 ) || ($largerSlot == 2) ) )  || (($largerSlot == 7) && ($smallerSlot == 1)) || (($largerSlot == 5) && ($smallerSlot == 3) ) ] } {
	# Long axis-aligned arrows 
	set theLineLength [expr $gSlotSize * 7 / 3 + (.75 *  $theLineGap)]
	if { [expr $largerSlot - $smallerSlot == 6] } {
	    # Vertical
	    set oX 0
	    set oY $theLineLength
	    set gutterH [expr $theLineGap / 2]  
	    set gutterV [expr $theLineGap * 2 ]
	    if { $isitCross } {
		set gutterH [expr $gutterH * 2]
	    } 
	    if { $largerSlot == 8 } {
		set slotPlaceX [expr $gSlotSize - $gutterH ]
	    } elseif { $largerSlot == 7 } {
		if { $isitCross } {
		    set slotPlaceX [expr ($gSlotSize / 2) - (.25 * $theLineGap)]
		} else {
		    set slotPlaceX [expr ($gSlotSize / 2) + (.25 * $theLineGap)] 
		}
		    
	    } else { 
		set slotPlaceX [expr $gutterH ]
	    }
	    set xStart [expr $slotPlaceX + $fromXOffset]
	    set yStart [expr $gutterV] 
	    set xEnd   [expr $xStart]
	    set yEnd   [expr $yStart + $theLineLength]
	
	} else {
	    # Horizontal
	    set oX $theLineLength
	    set oY 0
	    set gutterH [expr $theLineGap * 2] 
	    set gutterV [expr $theLineGap / 2]
	    if { $isitCross } {
		set gutterV [expr $gutterV * 2]
	    } 
	    if { $largerSlot == 8 } {
		set slotPlaceY [expr $gSlotSize - $gutterV ]
	    } elseif { $largerSlot == 5 } { 
		if { $isitCross } {
		    set slotPlaceY [expr ($gSlotSize / 2) - (.25 * $theLineGap)]  
		} else {
		    set slotPlaceY [expr ($gSlotSize / 2) + (.25 * $theLineGap)]
		}
	    } else { 
		set slotPlaceY $gutterV
	    }
	    set xStart $gutterH
	    set yStart [expr $slotPlaceY + $toYOffset]
	    set xEnd   [expr $xStart + $theLineLength]
	    set yEnd   [expr $yStart]

	}
    } elseif { [expr ($largerSlot - $smallerSlot == 3)] }  {
	# Short axis-aligned  vertical arrows
	set theLineLength [expr $gSlotSize * .5]
	set oX 0
	set oY $theLineLength
	set gutterH [expr $theLineGap * 1.5]
	set gutterV [expr $theSlotSize - (.5 * $theLineLength)]
	if { $isitCross } {
	    set gutterH [expr $gutterH + (.5 * $theLineGap)]
	}
	if { [expr (($largerSlot == 6) || ($largerSlot == 3))] } {
	    set slotPlaceX $gutterH 
	} elseif { [expr (($largerSlot == 8) || ($largerSlot == 5))] } {
	    set slotPlaceX [expr $gSlotSize - $gutterH]
	} else {
	    if { $isitCross } {
		set slotPlaceX [expr ($gSlotSize / 2) - (.75 * $theLineGap) ]  
	    } else {
		set slotPlaceX [expr ($gSlotSize / 2) + (.75 * $theLineGap)]
	    }
	}
	set xStart [expr $toXOffset + $slotPlaceX]
	set yStart [expr $gutterV  + $minYOffset]
	set xEnd   $xStart
	set yEnd   [expr $yStart + $theLineLength]
    } elseif { [expr (($largerSlot - $smallerSlot == 1) && !($largerSlot == 3 || $largerSlot == 6) ) ] } { 
	# Short axis-aligned horizontal arrows
	
	set theLineLength [expr $gSlotSize * .5]
	set oX $theLineLength
	set oY 0
	set gutterH [expr $theSlotSize - (.5 * $theLineLength)]

	set gutterV [expr $theLineGap * 1.5]
	if { $isitCross } {
	    set gutterV [expr $gutterV + (.5 * $theLineGap)]
	}
	if { [expr (($largerSlot == 2 ) || ($largerSlot == 1))] } {
	    set slotPlaceY $gutterV
	} elseif { [expr (($largerSlot == 8) || ($largerSlot == 7))] } {
	    set slotPlaceY [expr $gSlotSize - $gutterV]
	} else {
	    if { $isitCross } {
		set slotPlaceY [expr ($gSlotSize / 2) - (.75 * $theLineGap) ]  
	    } else {
		set slotPlaceY [expr ($gSlotSize / 2) + (.75 * $theLineGap)]
	    }
	}

	set xStart [expr $gutterH + $minXOffset]
	set yStart [expr $toYOffset + $slotPlaceY]
	set xEnd [expr $xStart + $theLineLength]
	set yEnd $yStart
    
    } elseif { [expr ((($largerSlot == 8) && ($smallerSlot == 0)) || (($largerSlot == 6) && $smallerSlot == 2)) ] } {
	# Long Diagonal arrows
	
	set gutterH  [expr $theSlotSize * .125]
	set gutterV  [expr $theSlotSize * .0675]
	if { $isitCross } { 
	    set gutterVOffset [expr $gutterV + (.25 * $theLineGap)]
	} else {
	    set gutterVOffset 0
	}
	if { [expr $largerSlot == 8] } {
	    set xStart [expr $gutterH]
	    set yStart [expr $gutterV + $gutterVOffset]
	    set xEnd [expr $maxXOffset + $gSlotSize - $gutterH]
	    set yEnd [expr $maxYOffset + $gSlotSize - (2.375 *  $gutterV) + $gutterVOffset]
	} else {
	    set xStart [expr $minXOffset + $gSlotSize - $gutterH]
	    set yStart [expr $gutterV + $gutterVOffset]
	    set xEnd [expr $gutterH]
	    set yEnd [expr $maxYOffset + $gSlotSize - (2.375 *  $gutterV) + $gutterVOffset]
	}

    } else {
	# short diagonal arrows
#	set theLineLength [expr $gSlotSize / 2]
	set gutterH [expr $gSlotSize / 3]
	set gutterV [expr $gSlotSize / 3]
	
	if { [expr ($largerSlot - $smallerSlot) == 4]  } {
	    # Right/Down arrows
	    if { $isitCross } {
		
		set xStart [expr $minXOffset + $gSlotSize - $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - 2 * $theLineGap]
		set xEnd [expr $maxXOffset + 2 * $theLineGap]
		set yEnd [expr $maxYOffset + $theLineGap]
	    } else {
		set xStart [expr $minXOffset + $gSlotSize - 2 * $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - $theLineGap]
		set xEnd  [expr $maxXOffset + $theLineGap]
		set yEnd  [expr $maxYOffset + 2 * $theLineGap] 
	    }
	} elseif { [expr ($largerSlot - $smallerSlot) == 2] } { 
	    # Left/Down arrows
	    if { $isitCross } {
		set xStart [expr $minXOffset + $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - 2 * $theLineGap]
		set xEnd [expr $maxXOffset + $gSlotSize - 2 * $theLineGap]
		set yEnd [expr $maxYOffset + $theLineGap]
	    } else {
		set xStart [expr $minXOffset + 2 * $theLineGap]
		set yStart [expr $minYOffset + $gSlotSize - $theLineGap]
		set xEnd [expr $maxXOffset +  $gSlotSize - $theLineGap]
		set yEnd [expr $maxYOffset + 2 * $theLineGap]
	    }
	} else {
	    BadElse "CreateHoppinArrow"
	
	}
	
    }

    ### Figure out the coordinates of the leftmost/topmost point
    
    
    #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  #  




    set lineStartX $xStart
    set lineStartY $yStart
    set lineEndX   $xEnd
    set lineEndY   $yEnd

    set endPointOffset [expr $theSlotSize / 8] 

    ######################################################################
    
    set theMove [$w create line $lineStartX $lineStartY $lineEndX $lineEndY \
	    -width $theLineWidth \
	    -fill $color]
    
    # We draw from the smaller-numbered slot to the larger-numbered one
    if { [expr $smallerSlot == $from] } {
	set ArrowheadX [expr $lineEndX - .5 * $theLineGap]
	set ArrowheadY [expr $lineEndY -  .5 * $theLineGap]
    } else {
	set ArrowheadX [expr $lineStartX - .5 * $theLineGap]
	set ArrowheadY [expr $lineStartY  - .5 * $theLineGap]
    }

	
    if { $isitCross } {
	set theEndPoint [DrawCross $w 0 0 tagDummyForMoves $color 0]
    } else {
	set theEndPoint [DrawPlus  $w 0 0 tagDummyForMoves $color 0]
    }

    $w addtag tagMoves withtag $theEndPoint
    $w addtag tagEnd$to$from$orientation withtag $theMove
    $w addtag tagEnd$to$from$orientation withtag $theEndPoint
    $w bind tagEnd$to$from$orientation <Enter> "$w itemconfig $theEndPoint -fill black"
    $w bind tagEnd$to$from$orientation <Leave> "$w itemconfig $theEndPoint -fill $color"

    $w addtag tagLine$to$from$orientation withtag $theMove
    $w addtag tagLine$to$from$orientation withtag $theEndPoint
    $w bind tagLine$to$from$orientation <Enter> "$w itemconfig $theMove -fill black"
    $w bind tagLine$to$from$orientation <Leave> "$w itemconfig $theMove -fill $color"
    
    $w addtag tagRaiseLine$to$from$orientation withtag $theMove
    $w addtag tagRaiseLine$to$from$orientation withtag $theEndPoint
    $w bind tagRaiseLine$to$from$orientation <Enter> "$w raise tagRaiseLine$to$from$orientation all"



    
    $w bind $theEndPoint <1> "HandleMove \{$from$to$orientation\}"
    
    

    
    $w move $theEndPoint $ArrowheadX $ArrowheadY

    
    return $theMove
}



#############################################################################
##
## DoValueMovesExplanation
##
## So we've just clicked the 'ValueMoves' button and it'd sure be nice to
## display a window to show what each colored moved looked like. 
##
#############################################################################

proc DoValueMovesExplanation {} {
    global kLabelFont kLabelColor gSlotSize kSmallPiece

    ### Create a new window, name it, and place it to the right of the board.

    toplevel .winValueMoves
    wm title .winValueMoves "Values"
    wm geometry .winValueMoves [GeometryRightOf .winBoard]
    
    ### Scale the move to be 1/5 the size of the slot

    set theMoveSize [expr $gSlotSize * .2]

    ### For each of the three values, display a move colored appropriately.

    set theValueList { Losing Tieing Winning  }

    for {set i 0} {$i < 3} {incr i} {
	set theValue [lindex $theValueList $i]
	set notValue [lindex $theValueList [expr 2-$i]]
	set theColor [ValueToColor $i]

	frame .winValueMoves.f$i -borderwidth 2 -relief raised
	canvas .winValueMoves.f$i.c -width $theMoveSize -height $theMoveSize
	DrawMove .winValueMoves.f$i.c dummy_arg $theColor $kSmallPiece

	label .winValueMoves.f$i.lab \
	    -font $kLabelFont \
	    -text "$theValue Move" \
	    -foreground $kLabelColor

	pack append .winValueMoves.f$i \
	    .winValueMoves.f$i.c {left} \
	    .winValueMoves.f$i.lab {left expand fill}
	pack append .winValueMoves .winValueMoves.f$i {top expand fill}

	### Normally we'd have put this in SetupHelpStringsForWindow,
	### but as you can see, there's a variable to set (theValue and i)
	### so we're stuck and forced to have it here.

	SetupHelp .winValueMoves.f$i.c \
		"$theValue move representation" \
		"picture" \
		"The graphical representation of a $theValue move. Selecting a move of this type will result in handing your opponent a $notValue position."
	SetupHelp .winValueMoves.f$i.lab \
		"$theValue move representation" \
		"static text" \
		"The graphical representation of a $theValue move. Selecting a move of this type will result in handing your opponent a $notValue position."
    }
}


#############################################################################
##
## DrawCross
##
## Here we draw an X on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawCross { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 7.0)]

    set startCross [expr $theSlotSize/8.0]
    set halfCross  [expr $theSlotSize/2.0]
    set endCross   [expr $startCross*7.0]

    set cornerX    [expr $slotX*$theSlotSize]
    set cornerY    [expr $slotY*$theSlotSize]
    set thickness  [expr $theSlotSize/10.0]

    set x1 $startCross
    set x2 [expr $startCross + $thickness]
    set x3 [expr $halfCross - $thickness]
    set x4 $halfCross
    set x5 [expr $halfCross + $thickness]
    set x6 [expr $endCross - $thickness]
    set x7 $endCross

    set y1 $startCross
    set y2 [expr $startCross + $thickness]
    set y3 [expr $halfCross - $thickness]
    set y4 $halfCross
    set y5 [expr $halfCross + $thickness]
    set y6 [expr $endCross - $thickness]
    set y7 $endCross

    set theCross [$w create polygon \
		   $x2 $y1 $x4 $y3 $x6 $y1 $x7 $y2 $x5 $y4 \
		   $x7 $y6 $x6 $y7 \
		   $x4 $y5 $x2 $y7 $x1 $y6 $x3 $y4 $x1 $y2 \
		   -fill $theColor \
		   -tag $theTag]

    if { $drawBig } {
	$w move $theCross $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCross

    return $theCross
}
