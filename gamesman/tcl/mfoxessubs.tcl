#############################################################################
##
## TagToColor
##
## Depending whether the tag is Alive or Dead, we return different colors
##
#############################################################################

proc TagToColor { theTag } {
    if { $theTag == "tagEditBoardDead" } {
	return grey
    } elseif { $theTag == "tagEditBoardFox" } {
	return red
    } elseif { $theTag == "tagEditBoardGoose" } {
	return blue
    } else {
	return grey
    }
}

############################################################################
##
## DrawInitialBoard
##
## Here we actually draw the board depending what the position already is.
## This allows us to have a position partially saved, then go in and load
## a new one up immediately.
##
############################################################################

proc DrawInitialBoard { w } {

    global gSlotSize gPosition gSlotsX gSlotsY gAgainstComputer

    #puts $gSlotsY
    set gAgainstComputer 0

    $w config \
        -width  [expr $gSlotsX*$gSlotSize] \
        -height [expr $gSlotsY*$gSlotSize]


#    foreach slot [$w find withtag tagEditBoardFox] {
#	$w itemconfigure $slot -fill grey
#    }

#    foreach slot [$w find withtag tagEditBoardGoose] {
#	$w itemconfigure $slot -fill grey
#    }
    
    for {set i 0} {$i < $gSlotsX} {incr i} { 
        for {set j 0} {$j < $gSlotsY} {incr j} { 
		CreateEditSlot $w $i $j tagEditBoardDead
	} 
    }
#    for {set i 0} {$i < $gSlotsX} {incr i} { 
#        for {set j 0} {$j < $gSlotsY} {incr j} { 
#	    puts "x=$i, y=$j"
#	    if {$j==0 && $i==5} {
#		puts "hello"
#		CreateEditSlot $w $i $j tagEditBoardFox		
#	    } elseif {$j==[expr $gSlotsY-1]} {
#		CreateEditSlot $w $i $j tagEditBoardGoose
#	    }
#	}
#    }
    #FillInitialPos $w
DrawDiffInitialBoard $w
}

proc DrawDiffInitialBoard { w } {
    global gSlotSize gPosition gSlotsX gSlotsY gAgainstComputer
    
    #puts $gSlotsY
    set gAgainstComputer 0
    
    $w config \
	    -width  [expr $gSlotsX*$gSlotSize] \
	    -height [expr $gSlotsY*$gSlotSize]
    
    foreach slot [$w find withtag tagEditBoardFox] {
	$w itemconfigure $slot -fill grey
	$w dtag $slot tagEditBoardFox
	$w addtag tagEditBoardDead withtag $slot

    }
    foreach slot [$w find withtag tagEditBoardGoose] {
	$w itemconfigure $slot -fill grey
	$w dtag $slot tagEditBoardGoose
	$w addtag tagEditBoardDead withtag $slot
    }
    
    for {set i 0} {$i < $gSlotsX} {incr i} { 
        for {set j 0} {$j < $gSlotsY} {incr j} { 
	    if {$j==0 && $i==5} {
		set slot [$w find withtag [list $i $j]]
		$w itemconfigure $slot -fill red
		$w dtag $slot tagEditBoardDead
		$w addtag tagEditBoardFox withtag $slot
	
	    } elseif {$j==[expr $gSlotsY-1]} {
		set slot [$w find withtag [list $i $j]]
		$w itemconfigure $slot -fill blue
		$w dtag $slot tagEditBoardDead
		$w addtag tagEditBoardGoose withtag $slot
	    } 
	} 
    }    
}

proc CreateEditSlot { w slotX slotY theTag } {
    global gSlotSize
    
    set circleWidth [expr $gSlotSize/10]
    set startCircle [expr $gSlotSize/8]
    set endCircle   [expr $startCircle*7]
    set cornerX     [expr $slotX*$gSlotSize]
    set cornerY     [expr $slotY*$gSlotSize]
    set slot [$w find withtag [list $slotX $slotY]]
    
    #puts stdout "slot: $slot, null?: [string compare $slot {}]"


    if { [string compare $slot {}] == 0 } {
	
	if {[expr ($slotX+$slotY)%2==0]} {
	    
	    set theBackground [$w create rect \
		    [expr $gSlotSize*$slotX] \
		    [expr $gSlotSize*$slotY] \
		    [expr [expr $gSlotSize*[expr $slotX+1]]+1] \
		    [expr [expr $gSlotSize*[expr $slotY+1]]+1] \
		    -outline black \
		    -width 1 \
		    -fill #a88]
	} else {
	    #puts stdout "CHANGING THE COLOR"
	    set theBackground [$w create rect \
		    [expr $gSlotSize*$slotX] \
		    [expr $gSlotSize*$slotY] \
		    [expr [expr $gSlotSize*[expr $slotX+1]]+1] \
		    [expr [expr $gSlotSize*[expr $slotY+1]]+1] \
		    -outline black \
		    -width 1 \
		    -fill #111]
	    
	    set slot [$w create oval $startCircle $startCircle \
		    $endCircle $endCircle \
		    -outline black \
		    -width 1 \
		    -fill [TagToColor $theTag] \
		    -tag tagSlot]
	    
	    $w move $slot $cornerX $cornerY
	    $w raise $slot $theBackground
	    $w addtag [list $slotX $slotY] withtag $slot	    
	    $w addtag $theTag withtag $slot 

	    #puts stdout "slotX:$slotX, slotY:$slotY"
	    
	    
	    $w bind tagEditBoardDead <1> "HandleDtoFClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "	
	    $w bind tagEditBoardFox <1> "HandleFtoGClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
	    $w bind tagEditBoardGoose <1> "HandleGtoDClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
	    
	}
    } else {
	#puts stdout "IN THE ESLE"
    }
}

proc HandleDtoFClick { w x y } {
    set theTags [$w gettags current]
    
    $w dtag current tagEditBoardDead
    $w addtag tagEditBoardFox withtag current

    $w itemconfig current -fill red
}

proc HandleFtoGClick { w x y } {
    set theTags [$w gettags current]
    
    $w dtag current tagEditBoardFox
    $w addtag tagEditBoardGoose withtag current

    $w itemconfig current -fill blue
}

proc HandleGtoDClick { w x y } {
    set theTags [$w gettags current]
    
    $w dtag current tagEditBoardGoose
    $w addtag tagEditBoardDead withtag current

    $w itemconfig current -fill grey
}


############################################################################
##
## DoEditInitialPosition
##
## The main procedure called by the "Edit Initial Positions" button. Set
## up the board, put it to the right of the current board, and then fill
## it with the current position.
##
############################################################################

proc DoEditInitialPosition { } {
    global kLabelFont
    global gSlotsX gSlotsY
    global gPosition gInitialPosition
    global thePositionItCameInWith
    global theGSlotsYItCameInWith

    set thePositionItCameInWith $gInitialPosition
    set theGSlotsYItCameInWith $gSlotsY

    .f5.butInitialPosition config -state disabled

    toplevel .winEditBoard
    wm title .winEditBoard "Edit your own Foxes and Geese  position"
    wm geometry .winEditBoard [GeometryRightOf .]

    canvas .winEditBoard.c -relief raised


    frame .winEditBoard.f1 \
        -borderwidth 2 \
        -relief raised

    label .winEditBoard.f1.labValue \
        -font $kLabelFont \
        -anchor w \
        -text "" \
        -foreground red

    ChooseHowManyRows winEditBoard
    
    DrawInitialBoard .winEditBoard.c
    
    #puts "done with doeditinitial"

#    bind .winEditBoard.f1.mes <3> { TheHelp "This is your status window, telling you which editing mode you are in, either toggle-mode, drag-select-mode, or drag-unselect-mode (or pie ala mode)."}
#    bind .winEditBoard.f0.butOk <3> { TheHelp "This button will accept the current edited position and change the \"Domineering\" window to contain this position."}
#    bind .winEditBoard.f0.butCancel <3> { TheHelp "This button cancels the \"Edit...\" request, closes this window and pretends the user never clicked the \"Edit...\" button. " }
}   


proc HandleRows { } {
    DrawInitialBoard .winEditBoard.c
}

proc ChooseHowManyRows { w } {
    global kLabelFont
    global gSlotsX gSlotsY
    global gPosition gInitialPosition
    global thePositionItCameInWith
    
    frame .$w.f0

    button .$w.f0.butCancel \
	    -text "Cancel" \
	    -font $kLabelFont \
	    -command {
	set gSlotsY $theGSlotsYItCameInWith
	HandleRows
	.f5.butInitialPosition config -state normal
	destroy .winEditBoard
	set gInitialPosition $thePositionItCameInWith
	set gPosition $gInitialPosition
    }

    button .$w.f0.butOk \
	    -text "OK" \
	    -font $kLabelFont \
	    -command {
	#pack forget .winEditBoard.f0.but4rows
	#pack forget .winEditBoard.f0.but5rows
	#pack forget .winEditBoard.f0.but6rows
	#pack forget .winEditBoard.f0.but7rows
	#pack forget .winEditBoard.f0.but8rows

	set gInitialPosition [MakeBoardNumber .winEditBoard.c tagEditBoardFox \
		tagEditBoardGoose tagEditBoardDead]
	set gPosition $gInitialPosition
	.f5.butInitialPosition config -state normal
	destroy .winEditBoard

	#PlaceFoxes winEditBoard
    }
	
	
    radiobutton .$w.f0.but4rows \
	    -text "4 rows" \
	    -font $kLabelFont \
	    -variable gSlotsY \
	    -value 4 \
	    -command HandleRows
    
    radiobutton .$w.f0.but5rows \
	    -text "5 rows" \
	    -font $kLabelFont \
	    -variable gSlotsY \
	    -value 5 \
	    -command HandleRows

    radiobutton .$w.f0.but6rows \
	    -text "6 rows" \
	    -font $kLabelFont \
	    -variable gSlotsY \
	    -value 6 \
	    -command HandleRows

    radiobutton .$w.f0.but7rows \
	    -text "7 rows" \
	    -font $kLabelFont \
	    -variable gSlotsY \
	    -value 7 \
	    -command HandleRows

    radiobutton .$w.f0.but8rows \
	    -text "8 rows" \
	    -font $kLabelFont \
	    -variable gSlotsY \
	    -value 8 \
	    -command HandleRows

    message .$w.f1.mes \
	-justify center \
        -font $kLabelFont \
        -text "Click on the number of rows you want. Then place where the fox and geese go. Each slot toggles from a blank to a fox to a goose. When you're done press OK." \
	-width 700 \
	-foreground black

    pack append .$w.f1 \
	    .$w.f1.mes { left fill expand }

    pack  append .$w.f0 \
	    .$w.f0.butOk { left expand fill } \
	    .$w.f0.but4rows { left expand fill } \
	    .$w.f0.but5rows { left expand fill } \
	    .$w.f0.but6rows { left expand fill } \
	    .$w.f0.but7rows { left expand fill } \
	    .$w.f0.but8rows { left expand fill } \
	    .$w.f0.butCancel { left expand fill }

    pack append .$w .$w.f1 { top fill expand } \
	    .$w.c top .$w.f0 { bottom fill expand }
}

#This function is very strange. The slots that have a fox tag are actually where geese are.
#So whenever you want a geese you have to check if the tag of a slot is a tagFox.

proc MakeBoardNumber { w tagFox tagGeese tagBlank } {
    global gSlotSize
    global gSlotsX gSlotsY
    global gTrueSlotsX gTrueSlotsY
    global gRealBoardSize
    global gNumberPieces
    global gNumberFoxes
    global gBoardSize
    
    set theBoard 0
    set piecesLeft 0
    set foxesLeft 0
    set allpieceposition 0
    set insidepiecesposition 0
    set boardsize [expr ($gSlotsX/2)*$gSlotsY]

    set slot [$w find withtag $tagFox]
    #puts stdout "coord: [$w find withtag $tagFox]"
    #puts stdout "Tags: [$w gettags $slot]"

    foreach slot [$w find withtag $tagFox] {
	set slotTags [$w gettags $slot]
	set ySlot [lindex [lindex $slotTags 1] 1]
	#puts stdout "slotTags: $slotTags"
	#puts stdout "GEESE gSlotsY: $gSlotsY, ySlot: $ySlot"
	if {[expr $ySlot < $gSlotsY]} {
	    set foxesLeft [expr {$foxesLeft + 1}]
	}
    }
    
    #puts stdout "gSlotSize: $gSlotSize, $gSlotsY: $gSlotsY"
    #puts stdout "foxesleft: $foxesLeft" 

    set gNumberFoxes $foxesLeft
    set piecesLeft $foxesLeft
    
    foreach slot [$w find withtag $tagGeese] {
	set slotTags [$w gettags $slot]
	set ySlot [lindex [lindex $slotTags 1] 1]
	#puts stdout "FOXES gSlotsY: $gSlotsY, ySlot: $ySlot"
	if {[expr $ySlot < $gSlotsY]} {
	    set piecesLeft [expr {$piecesLeft + 1}]
	}
    }

    set gNumberPieces $piecesLeft

    
    #puts stdout "number of foxes: $gNumberFoxes, number of pieces: $gNumberPieces";
    #puts stdout "gSlotX: $gSlotsX, gSlotY:$gSlotsY"

    for {set i 0} {$i < $gSlotsY} {incr i} {
	for {set j 0} {$j < $gSlotsX} {incr j} {
	    if {[expr $j%2==0]} {
		set trueSlot [expr ($i*4)+($j/2)]
		set trueSlotY $i;
		if { [expr $i%2==0] } {
		    set trueSlotX [expr $j + 1]
		} else { 
		    set trueSlotX $j
		}   
		
		set slot [$w find withtag [list $trueSlotX $trueSlotY]]
		#puts stdout "something: [$w gettag $slot]"
		set tag [lindex [$w gettag $slot] 2]

		#puts stdout "i: $i, j:$j, slot: $slot, tag: $tag, trueSlot: $trueSlot"
		
		
		if {$tag == "tagEditBoardDead"} {
		    set allpieceposition [expr $allpieceposition + [C_ComputeC [expr $boardsize-$trueSlot-1] [expr $piecesLeft-1]]]
		} else {
		    if {$tag == "tagEditBoardGoose"} {
			set insidepiecesposition [expr $insidepiecesposition + [C_ComputeC [expr $piecesLeft-1] [expr $foxesLeft-1]]]
			#puts stdout "YEA!!! pl: $piecesLeft, fl: $foxesLeft"
		    } else {
			set foxesLeft [expr $foxesLeft - 1]
		    }
		    set piecesLeft [expr $piecesLeft - 1]
		}
	    }
	}
    }
    
    #puts stdout "ipp: $insidepiecesposition, bs: $boardsize, tp: $gNumberPieces, app: $allpieceposition"

    set position [expr $insidepiecesposition*[C_ComputeC $boardsize $gNumberPieces]+$allpieceposition]

    #puts stdout "position: $position"
    
    set gTrueSlotsX [expr $gSlotsX/2] 
    set gTrueSlotsY $gSlotsY
    set gRealBoardSize [expr $gTrueSlotsX * $gTrueSlotsY]
    set gBoardSize [expr $gSlotsX * $gSlotsY]

    return $position

}
    
