#!/usr/sww/bin/tclsh

##############################################################################
##
## In GAMESMAN, we have tagAlive and tagDead to represent if a button/slot could
## be active or inactive. But, the way i did it (not making all buttons active),
## then i won't be able to make it tagDead.
##
##
##
##
## slots are placed at 
## (1.1, 0.6) (2.45, 0.6) (3.8, 0.6) 
## (1.1, 1.9) (2.45, 1.9) (3.8, 1.9)
## (1.1, 3.2) (2.45, 3.2) (3.8, 3.2)
##
##
##
##
##############################################################################

global command_line_args
set command_line_args [concat $argv0 $argv]

proc GS_InitGameSpecific {} {

    ### Set the name of the game
    global kGameName
    set kGameName "Rubik's Infinity (3x3)"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    global gSlotSize
    set gSlotSize  100
    ## gSlotsize originally 65, I use 100 for 3 by 3 case

     ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to get marker at their color WINS"
    set kMisereString "First player to get marker at their color LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click with the left button at the top arrows to move a normal piece and the bottom arrows for the special piece"
    set kToWinStandard  "Be the first player to have the marker reach their color"
    set kToWinMisere    "Force your opponent to get their marker to their color"

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 3
    set gSlotsY 3

    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 3375000
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/circle.xbm"
    set xbmRight "$kRootDir/../bitmaps/circle.xbm"
    set macLeft  arrow
    set macRight arrow

    # Authors Info
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Chi Huynh, Edwin Mach"
    set kTclAuthors "Chi Huynh, Edwin Mach"
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
    set kEditInitialPosition 1
    
}


############################################################################
##
## MyHandleMoves (imitates that of HandleMoves (no args) )
##
## 
##
############################################################################


#############################################################################
##
## CreateButton2
##
## Here we create a new slot on the board defined by window w. The value
## of the slot in (slotX,slotY) tells us where on the board to be.
##
#############################################################################

proc CreateButton2 {w aX aY bX bY cX cY dX dY eX eY fX fY tag idnum slotX slotY} {
    global gSlotSize gSlotList

    if { $tag == "normal"}  {
	set button [$w create polygon \
		${aX}c ${aY}c \
		${bX}c ${bY}c \
		${cX}c ${cY}c \
		${dX}c ${dY}c \
	        ${eX}c ${eY}c \
	        ${fX}c ${fY}c \
		-width 0.5c \
		-fill cyan \
		-tag normal ]

	GS_EmbellishButton $w $slotX $slotY $button normal $idnum
    } else { 
	set button [$w create polygon \
		${aX}c ${aY}c \
		${bX}c ${bY}c \
		${cX}c ${cY}c \
		${dX}c ${dY}c \
	        ${eX}c ${eY}c \
	        ${fX}c ${fY}c \
		-width 0.2c \
		-fill cyan \
		-outline white \
		-tag special ]
	GS_EmbellishButton $w $slotX $slotY $button special $idnum
    }

    ### Remember what the id of the slot was in our array
    set gSlotList($idnum) $button

    ### Add any slot embellishments are necessary (numbers, highlights, etc.)
    #GS_EmbellishButton $w $slotX $slotY $button $color $tag
}



#############################################################################
##
## CreateMarker
##
## ARGS: w = canvas, x= where the marker should be located (NOT USED!)
##
#############################################################################
proc CreateMarker { w x } {
    global gSlotSize

    ## X == 100 / (10 * 2)
    set markerX [expr [expr $gSlotSize / 20 ] + 0.25]
    set markerY [expr [expr [expr $gSlotSize * 1.55] / 10] - 0.2]

    set marker [$w create rect \
	    [expr $markerX - 0.2]c ${markerY}c \
	    [expr $markerX + 0.2]c [expr $markerY + 0.9]c \
	    -fill grey \
	    -outline black \
	    -tag marker ]
    $w addtag tagInitial withtag $marker
}



############################################################################
##
## UpdateScoreMarker
##
## ARGS: w = canvas, score = currentscore, whoseturn = turn
##
############################################################################
proc UpdateScoreMarker { w oldscore score whoseturn } {

    #draw scoreboard marker (the X should be +- 0.15c away
    # Markers are on positions 2, 2.85, 3.7, 4.55, 5.4, 6.25, 7.1, 7.95, 8.8, 9.65, 10.5 
    global marker gAnimationSpeed

    set zero 2
    set one 2.85
    set two 3.7
    set three 4.55
    set four 5.4
    set five 6.25
    set six 7.1
    set seven 7.95
    set eight 8.8
    set nine 9.65
    set ten 10.5

    if { $oldscore == 0 } {
	set oldscorepos $zero	
    } elseif {$oldscore == 1 } {
	set oldscorepos $one
    } elseif { $oldscore == 2 } {
	set oldscorepos $two
    } elseif { $oldscore == 3 } {
	set oldscorepos $three
    } elseif { $oldscore == 4 } {
	set oldscorepos $four
    } elseif { $oldscore == 5 } { 
	set oldscorepos $five
    } elseif { $oldscore == 6 } { 
	set oldscorepos $six
    } elseif { $oldscore == 7 } {
	set oldscorepos $seven
    } elseif { $oldscore == 8 } { 
	set oldscorepos $eight
    } elseif { $oldscore == 9 } { 
	set oldscorepos $nine
    } elseif { $oldscore == 10 } {
	set oldscorepos $ten
    } else {
	puts "Error in OLDSCORE! OldScore is invalid number."
    }

    if { $score == 0 } {
	set scorepos $zero	
    } elseif {$score == 1 } {
	set scorepos $one
    } elseif { $score == 2 } {
	set scorepos $two
    } elseif { $score == 3 } {
	set scorepos $three
    } elseif { $score == 4 } {
	set scorepos $four
    } elseif { $score == 5 } { 
	set scorepos $five
    } elseif { $score == 6 } { 	set scorepos $six
    } elseif { $score == 7 } {
	set scorepos $seven
    } elseif { $score == 8 } { 
	set scorepos $eight
    } elseif { $score == 9 } { 
	set scorepos $nine
    } elseif { $score == 10 } {
	set scorepos $ten
    } else {
	puts "Error in SCORE! Score is invalid number."
    }

    set timemoving [ expr $gAnimationSpeed * 2 *[expr $scorepos - $oldscorepos] ]
    if { $timemoving < 0 } {
	set timemoving [expr - $timemoving]
    }
  
    set change [ expr $scorepos - $oldscorepos ]
    if { $timemoving != 0 } {
	set howmuch [ expr $change / $timemoving] 
    }

    ## make slower for me... hehe
    for { set i 1 } { $i < 100 } {incr i} {
	update idletasks 
	update idletasks 
    }

    ### do some animation sliding for marker
    for {set i 1} { $i <= $timemoving } {incr i} {
	update idletasks
	update idletasks
	$w move marker ${howmuch}c 0c 
	##puts "Moved marker from $oldscorepos to $scorepos."
	update idletasks
	update idletasks
    }

    ### Update our internal markers

    ##$w addtag marker withtag marker
    ##$w dtag marker
    ##$w create rect [expr $score - 0.15]c 14.2c [expr $score + 0.15]c 14.8c -fill grey -outline black

    
}



############################################################################
##
## MovePiecesUp (supersedes DrawPiecesOnBoard)
##
## Note: Depending on column number, move those pieces up, and kicks off the first piece.
##
## ARGS: w = canvas, theMove = the move #, first = col type (0->14)
##       top = {1,2,3}, mid={4,5,6}, bot = {7,8,9}, tagID = position number of piece
##
############################################################################
proc MovePieceUp {w tagID oldslotY newslotY } {

    global gAnimationSpeed

    set timemoving $gAnimationSpeed 
    
    ## for some reason, we have to muliply by 2 in order to get the correct position
    set change [expr $newslotY - $oldslotY ]
    set change [expr $change * 2]

    set howmuch [ expr $change / $timemoving] 

    ## make slower for me... hehe
    for { set i 1 } { $i < 20 } {incr i} {
	update idletasks 
    }

    ### do some animation sliding for marker
    for {set i 1} { $i <= $timemoving } {incr i} {
	update idletasks
	
	$w move $tagID 0c ${howmuch}c 
	##puts "Moved piece $tagID -> $howmuch."
	update idletasks
	update idletasks
	update idletasks
	update idletasks
	update idletasks
    }

}



############################################################################
##
## DrawPiecesOnBoard
##
## ARGS: w = canvas, theMove = the move #, first = col type (0->14)
##       top = {1,2,3}, mid={4,5,6}, bot = {7,8,9}
##
############################################################################
proc DrawPiecesOnBoard { w theMove first top mid bot whoseturn white black} {

    global gSlotSize
    ## From the column data, i determine the x-coor of where to put the pieces.
    # draw the columns. Width of each column is just 0.5c
    # startcolumn1 will give us where to start and the widthsize asit depends on gSlotSize
    set startBoardX 0.25
    set startBoardY 0.25
    set endBoardX [expr $gSlotSize /10]
    set endBoardY [expr $gSlotSize /10]
    set widthsize [expr $gSlotSize * 0.006]
    set column1mid [expr [expr $endBoardX / 3] + [expr 1.75 * $startBoardX] ]
    set startcolumn1 [expr $column1mid - [expr $widthsize / 2] ]
    set endcolumn1 [expr $startcolumn1 + $widthsize]

    set column2mid [expr [expr [expr $endBoardX *2] / 3] + [expr $startBoardX *3.5]] 
    set startcolumn2 [expr $column2mid - [expr $widthsize / 2] ]
    set endcolumn2 [expr $startcolumn2 + $widthsize]

    ## we preliminary find what the Y coords are supposed to be...
    set topY [ expr $endBoardY - [expr $widthsize * 3] - 2]
    set midY [ expr $endBoardY - [expr $widthsize * 2] - 1.55]
    set botY [ expr $endBoardY - $widthsize - 0.9]
    set offY $endBoardY
    
    
    ## slots are placed at 
    ## (1.1, 0.6) (2.45, 0.6) (3.8, 0.6) 
    ## (1.1, 1.9) (2.45, 1.9) (3.8, 1.9)
    ## (1.1, 3.2) (2.45, 3.2) (3.8, 3.2)
    ## (1.1, 4.5) (2.45, 4.5) (3.8, 4.5) <-- Not on board, but moves pieces from there, up.

    if { $top == 1 } {
	set slotX 0.7
    } elseif { $top == 2 } {
	set slotX [expr $startcolumn1 + 0.9]
    } elseif { $top == 3} { 
	set slotX [expr $startcolumn2 + 0.8]
    } else {
	puts "Bad top variable passed in!!!"
    }
    
    ## set gagID
    if { $theMove == 1 || $theMove == 4 } {
	set tagID "seven"
    } elseif { $theMove == 2 || $theMove == 5} { 
	set tagID "eight"
    } else {
	set tagID "nine"
    }
    
	if { $first == 0 } {
	    ## Nothing ---
	} elseif { $first == 1 } {
	    puts "In first =1,  gPlayerOneTurn --> $whoseturn"
	    # --o
	    ## This means that the BLUE? piece gets placed
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #set thePiece $whoseturn
	    # special piece
	    if { $thePiece == "blue" && $white == $bot} {
		set thePiece "black"
	    } 
	    set slotY $botY
	    set oldslotY $offY
	    
	    DrawPiece $slotX $oldslotY $thePiece $tagID
	    MovePieceUp $w $tagID $oldslotY $slotY

	} elseif { $first == 2 } {
	    puts "In first =2,  gPlayerOneTurn --> $whoseturn"
	    # -ox
	    ## BLUE, 
	    #set thePiece $whoseturn
	    if { !$whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #speical piece
	    if { $thePiece == "blue" && $white == $bot} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $bot} {
		set thePiece "white"
	    } else {}
	    set slotY $botY 
	    set oldslotY $offY 

	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    # now draw the RED one
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    
	    #special piece
	    if { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } elseif {$thePiece == "blue" && $white == $mid } {
		set thePiece "black"
	    } else {}
	    ##set slotY $midY
	    ##set oldslotY $botY

	    ## draw the new piece and move it up
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID
	    
	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY  
	    ## now move up the old piece
	    #MovePieceUp $w $tagID $oldslotY 
	} elseif { $first == 3 } {
	    puts "In first =3,  gPlayerOneTurn --> $whoseturn"
	    ## -oo
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    # the other piece
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } elseif {$thePiece == "blue" && $white == $mid } {
		set thePiece "black"
	    } else {}
	    ##set slotY $midY
	    ##set oldlsotY $botY
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID
	    
	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY  

	} elseif { $first == 4 } {
	    puts "In first =4,  gPlayerOneTurn --> $whoseturn"
	    ## oxx
	    # piece 1
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID
	    
	    #piece 2
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } else {}
	    ##set slotY $midY
	    ##set oldslotY $botY
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    # piece 3
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $top} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $top} {
		set thePiece "white"
	    } else {}
	    ##set slotY $topY
	    ##set oldslotY $midY
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 5 } {
	    puts "In first =5,  gPlayerOneTurn --> $whoseturn"
	    # oxo
	    # piece 1
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    #piece 2
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } else {}
	    ##set slotY $midY
	    ##set oldslotY $botY
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    # piece 3
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $top} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $top } {
		set thePiece "white"
	    } else {}
	    ##set slotY $topY
	    ##set oldslotY $midY
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 6 } {
	    puts "In first =6,  gPlayerOneTurn --> $whoseturn"
	    # oox
	    # piece 1
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    #piece 2
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } else {}
	    ##set slotY $midY
	    ##set oldslotY $botY
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    # piece 3
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $top} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $top } {
		set thePiece "white"
	    } else {}
	    ##set slotY $topY
	    ##set oldslotY $midY
	    ## DrawPiece $slotX $slotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 7 } {
	    puts "In first =7,  gPlayerOneTurn --> $whoseturn"
	    # ooo
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	   
	    #special piece
	    if { $thePiece == "blue" } {
		if { $white == $top} {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "black"
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif {$white == $mid } {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "black"
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif {$white == $bot } {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ## set thePiece "black"
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} else {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ##set slotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ##set slotY $topY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		}
		## This should move ALL of them up.
		MovePieceUp $w $tagID $oldslotY $slotY 
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		if { $black == $top} {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "white"
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif { $black == $mid } {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "white"
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif {$black == $bot } {
		     set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ## set thePiece "white"
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} else {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		}
		## This should move ALL of them up.
		MovePieceUp $w $tagID $oldslotY $slotY 
	    } else {}
	    
	    
	    
	} elseif { $first == 8 } {
	    puts "In first = 8,  gPlayerOneTurn --> $whoseturn"
	    ## --x
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #set thePiece whoseturn
	    # special piece
	    if { $thePiece == "red" && $black == $bot} {
		set thePiece "white"
	    } 
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 9 } {
	    puts "In first = 9,  gPlayerOneTurn --> $whoseturn"
	    ## -xo
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #set thePiece whoseturn
	    #speical piece
	    if { $thePiece == "blue" && $white == $bot} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $white == $bot} {
		set thePiece "white"
	    } else {}
	    set slotY $botY 
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID
	    # now draw the RED one
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    
	    #special piece
	    if { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } elseif {$thePiece == "blue" && $white == $mid } {
		set thePiece "black"
	    } else {}
	    ## set slotY $midY
	    ## set oldslotY $botY
	    ##DrawPiece $slotX $oldslotY $thePiece $tagID 

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 10 } {
	    puts "In first = 10,  gPlayerOneTurn --> $whoseturn"
	    ## -xx
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID
	    # the other piece
	    ## set slotY $midY
	    ## set oldslotY $botY
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } elseif {$thePiece == "blue" && $white == $mid } {
		set thePiece "black"
	    } else {}
	    ##DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 11 } {
	    puts "In first = 11,  gPlayerOneTurn --> $whoseturn"
	    ## xoo
	    # piece 1
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID
	    
	    #piece 2
	    ## set slotY $midY
	    ## set oldslotY $botY
	    if {  $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } else {}
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    # piece 3
	    ## set slotY $topY
	    ## set oldslotY $midY
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $top} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $top} {
		set thePiece "white"
	    } else {}
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 12 } {
	    puts "In first = 12,  gPlayerOneTurn --> $whoseturn"
	    ## xox
	    # piece 1
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    #piece 2
	    ##set slotY $midY
	    ## set oldslotY $botY
	    if {  $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } else {}
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    # piece 3
	    ##set slotY $topY
	    ##set oldslotY $midY
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $top} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $top } {
		set thePiece "white"
	    } else {}
	    ## DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 13 } {
	    puts "In first =13,  gPlayerOneTurn --> $whoseturn"
	    ## xxo
	    # piece 1
	    if { $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $theMove == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		set thePiece "white"
	    } else {}
	    set slotY $botY
	    set oldslotY $offY
	    DrawPiece $slotX $oldslotY $thePiece $tagID

	    #piece 2
	    ##set slotY $midY
	    ##set oldslotY $botY
	    if {! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $mid} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $mid} {
		set thePiece "white"
	    } else {}
	    ##DrawPiece $slotX $oldslotY $thePiece $tagID

	    # piece 3
	    ##set slotY $topY
	    ##set oldslotY $midY
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	    #special piece
	    if { $thePiece == "blue" && $white == $top} {
		set thePiece "black"
	    } elseif { $thePiece == "red" && $black == $top } {
		set thePiece "white"
	    } else {}
	    ##DrawPiece $slotX $oldslotY $thePiece $tagID

	    ## This should move ALL of them up.
	    MovePieceUp $w $tagID $oldslotY $slotY 

	} elseif { $first == 14 } {
	    puts "In first =14,  gPlayerOneTurn --> $whoseturn"
	    ## xxx
	    if { ! $whoseturn } { set thePiece "blue" } { set thePiece "red" }
	   
	    #special piece
	    if { $thePiece == "blue" } {
		if { $white == $top} {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ## DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "black"
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif {$white == $mid } {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "black"
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID 
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif {$white == $bot } {
		     set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ## set thePiece "black"
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} else {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		}
		## This should move ALL of them up.
		MovePieceUp $w $tagID $oldslotY $slotY 
	    } elseif { $thePiece == "red" && $theMove == $mid} {
		if { $black == $top} {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "white"
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif { $black == $mid } {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set thePiece "white"
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} elseif {$black == $bot } {
		     set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ## set thePiece "white"
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		} else {
		    set slotY $botY
		    set oldslotY $offY
		    DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $midY
		    ## set oldslotY $botY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		    ## set slotY $topY
		    ## set oldslotY $midY
		    ##DrawPiece $slotX $oldslotY $thePiece $tagID
		}
		## This should move ALL of them up.
		MovePieceUp $w $tagID $oldslotY $slotY 
	    } else {
	    }

	    
	} else {
	    puts "Error: Invalid Position number in DrawPiecesOnBoard"
	}
    

}

#############################################################################
##
## MyHandleMove
##
## Ok, the user has just clicked to make a move. Now what do we do? We have
## to update the internals, change the board, and call for a computer move
## if needed.
##
## I mimick Dan's original function "HandleMove" to hardcode my pieces on the
## the board. I have changed all instances of "HandleMove" to "MyHandleMove"
##
##
#############################################################################

proc MyHandleMove { theMove } {
    global gPlayerOneTurn gPosition gSlotSize gAgainstComputer gSlotList
    global gSlotsX gSlotsY kInteractionType gAnimationSpeed

    set w .winBoard.c

    ## So basically, from the gPosition, I want to find out basically what i did
    ## in my C code.
    
    
    
    ## I will need to clear the old board and redraw my pieces on the board.
    #$w delete tagPiece
    # try only deleting the first column
    #$w delete 

    ## 0. Do the move
    ## 1. Update the score
    
    ### Do the move and update the position
    set oldscore [expr [expr ($gPosition / (2*15*15*15*10*10))] % 11 ]
    set gPosition [C_DoMove $gPosition $theMove]

    
    ### Find out the new score
    if { $kInteractionType == "SinglePieceRemoval" } {
	
	## Here, our gPosition has been updated with our move
	## Now we want to parse gPosition and based on its value,
	## we Draw out the board
	set turn [expr $gPosition % 2]
	
	set third [expr ($gPosition / 2) % 15]
	set second [expr [expr ($gPosition / (2*15)) ] % 15 ]
	set first [expr [expr ($gPosition / (2*15*15))] % 15 ]
	set white [expr [expr ($gPosition / (2*15*15*15))] % 10 ]
	set black [expr [expr ($gPosition / (2*15*15*15*10))] % 10 ]
	set score [expr [expr ($gPosition / (2*15*15*15*10*10))] % 11 ]

	## Testing purpose
	##$w delete debug
	##$w create text 1c 16.25c -text "ID num -> $theMove (turn: $turn) ( $first | $second |  $third ) $white $black $score" -fill white -tag debug
    
	## set whoseturn
	set whoseturn $turn

	if { $whoseturn == 0 } {
	    set opposite 1
	} else {
	    set opposite 0
	}

	##$w delete tagPiece
	##$w delete marker

	## By moding by 2, this tells us what piece was just inserted.
	## 0 == blue piece, 1 == red piece
	## whoseturn tells us whose turn it is. 1 == blue, 0 == red.

	## Attempt to slow this down
	for {set i 0} {$i < $gAnimationSpeed} {incr i} {
	    update idletasks
	}

	## Since we are going to move one column at a time, then all we need to do is just call it once.
	
	if { $theMove == 1 || $theMove == 4 } {
	    DrawPiecesOnBoard $w $theMove $first 1 4 7 1 $white $black
	} elseif { $theMove == 2 || $theMove == 5} {
	    DrawPiecesOnBoard $w $theMove $second 2 5 8 1  $white $black
	} else {
	    DrawPiecesOnBoard $w $theMove $third 3 6 9 1 $white $black
	}


	## SET SCORE BOARD
	
	UpdateScoreMarker $w $oldscore $score $whoseturn
	

	### Enable or Disable slots as a result of the move
    
	set theSlot4 $gSlotList(4)
	set theSlot5 $gSlotList(5)
	set theSlot6 $gSlotList(6)

	puts "This is turn --->  $turn"

	## If my own special piece is on the board, then we need to disable
	## slots 4-6. When the turn changes, we need to update (enable/disable)
	## buttons depending if our piece is still on our board.

	if { $turn == 1 && $white != 0 } { 
	    puts "Inside turn 1 & white !=0"
	    GS_HandleEnablesDisables $w $theSlot4 $theMove
	    GS_HandleEnablesDisables $w $theSlot5 $theMove
	    GS_HandleEnablesDisables $w $theSlot6 $theMove
	} elseif { $turn == 0 && $black != 0 } {
	    puts "Inside turn=0 & black !=0"
	    GS_HandleEnablesDisables $w $theSlot4 $theMove
	    GS_HandleEnablesDisables $w $theSlot5 $theMove
	    GS_HandleEnablesDisables $w $theSlot6 $theMove

	} else {
	    ## We want to enable everything
	    set counter 4

	    foreach theMoveValue [C_GetValueMoves $gPosition] {
		set theMove [lindex $theMoveValue 0]
		##DrawMove .winBoard.c $theMove [ValueToColor [lindex $theMoveValue 1]] $kBigPiece
		##EnableButton2 $w $theSlot$counter [ValueToColor [lindex $theMoveValue 1]] special $counter
		set counter [expr $counter + 1]
	    }    
	    ##EnableButton2 $w $theSlot4 cyan special 4
	    ##EnableButton2 $w $theSlot5 cyan special 5
	    ##EnableButton2 $w $theSlot6 cyan special 6
	}

    }

    
    ### Get the new value

    set theValue [C_Primitive $gPosition]


    ### And, depending on the value of the game...

    if { $theValue != "Undecided" } { 

	##This is to ensure that the computer/human gets to make the last move and is DISPLAYed onthe board. 
	if { $gAgainstComputer } {
	    DoComputersMove
	} else {
	    DoHuman
	}
	HandleGameOver $w $theValue

    } elseif { [not $gAgainstComputer] } {

	### Do nothing. It was all taken care of for us already.
	### And it's now the next person's turn.
	### Swap turns
	set gPlayerOneTurn [not $gPlayerOneTurn]
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
## ButtonEnter
##
## This is what we do when we enter a live slot. (We fill it with black)
##
#############################################################################

proc ButtonEnter { w } {
    $w itemconfig current -fill orange
}


#############################################################################
##
## GS_EmbellishButton
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishButton { w slotX slotY slot tag id} {
    global gPosition gSlotsX gSlotsY

    #$w create text  8c 10c -text "GS_EMBELBUTTON" -fill white
    ### Now we put the pieces on the board

    set theIndex [SinglePieceRemovalCoupleMove $slotX $slotY]

    #if { $gPosition & int(pow(2,$theIndex)) } {
    # OK, RUBIKS. This uses the position number and breaks it down. We will 
    # hardcode all colors/moves. I have no yet implemented this. But this actually
    # will put the pieces on the purple game board.
    #}
    ### for TicTacToe, we enable all initial slots
    EnableButton $w $slot $tag $id
    puts "Enable Button here."

    ### And all the slots should be active upon "New Game"
    $w addtag tagInitial withtag $slot
}


#############################################################################
##
## EnableButton2
##
## Here we need to Enable the slot because it's now able to be clicked upon
##
#############################################################################
proc EnableButton2 { w theSlot color tag id} {
    ### Make it cyan

    $w itemconfig $theSlot -fill cyan

    ### Add a 'Alive' tag

    $w addtag tagAlive withtag $theSlot

    ### Remove its 'Dead' tag

    $w dtag $theSlot tagDead

    ### Disable the embellishments
    ##GS_DisableSlotEmbellishments $w $theSlot

}



#############################################################################
##
## EnableButton
##
## Here we need to Enable the slot because it's now able to be clicked upon
##
#############################################################################

proc EnableButton { w theSlot tag id} {
    global gSlotSize kInteractionType varMoves
 
    ## this if statement applies only to my game
    if { $kInteractionType == "SinglePieceRemoval" } {
	
	$w dtag $theSlot tagDead
	$w addtag tagAlive withtag $theSlot
	$w bind $theSlot <1> "MyHandleMove $id"
	
	
	$w bind tagAlive <Enter> "ButtonEnter $w"
	if {$tag == "normal"} {
	    $w bind $theSlot <Leave> "$w itemconf current -fill cyan  "
	} 
	if {$tag == "special"} {
	    $w bind $theSlot <Leave> "$w itemconf  -fill green "
	    ## I deleted "current" from the arg list dec17,2002.
	}
	$w bind tagDead  <1>     { }
	$w bind tagDead  <Enter> { }
	$w bind tagDead  <Leave> { }
    } 

}

#############################################################################
##
## CreateButton
##
## Here we create a new slot on the board defined by window w. The value
## of the slot in (slotX,slotY) tells us where on the board to be.
##
#############################################################################

proc CreateButton {w num1 num2 num3 color tag idnum slotX slotY} {
    global gSlotSize gSlotList

    if { $tag == "normal"}  {
	set button [$w create line ${num1}c 11c ${num2}c 10c ${num3}c 11c -width 0.5c \
		-cap butt -join miter \
		-fill cyan \
		-tag normal]
	GS_EmbellishButton $w $slotX $slotY $button $color normal $idnum
    } else { 
	set button [$w create line ${num1}c 13c ${num2}c 12c ${num3}c 13c \
		-width 0.5c \
		-cap butt -join miter \
		-fill cyan \
		-tag special]
	GS_EmbellishButton $w $slotX $slotY $button $color special $idnum
    }

    ### Remember what the id of the slot was in our array
    set gSlotList($idnum) $button

    ### Add any slot embellishments are necessary (numbers, highlights, etc.)
    #GS_EmbellishButton $w $slotX $slotY $button $color $tag
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

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {
    global gPosition gSlotsX gSlotsY

    #$w create text  5c 12.5c -text "GS_EMBEL" -fill white
    
    set theIndex [SinglePieceRemovalCoupleMove $slotX $slotY]

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
    global gSlotsY gPosition

    return $theMove
    ##return [expr $gSlotsY - $theMove - $gPosition]
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

    return $theMove
    #return [expr $gSlotsY - $theMove - $gPosition]
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
    
    # $w raise tagText tagSlot
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
    global gPosition gSlotsY

    #if { ($gSlotsY - ($gPosition+$theMove+1)) >= 0 } {
	#puts "GS slot = $theSlot, move = $theMove, gPosition = $gPosition, slot = $gSlotList(0,[expr $gSlotsY - ($gPosition+$theMove+1)])"
    #}

    DisableSlot $w $theSlot
    puts "Disabing slor $theSlot $theMove";
    
    ### If they skipped one, we need to disable it
    ### We have NOT done the move yet. This is kind of a hack.
    ### But the slots are listed from bottom to top (stoopid)

    #if { $theMove == 2 } {
	#DisableSlot $w $gSlotList(0,[expr $gSlotsY - ($gPosition + 1)])
    #}

    ### The if statement is to prevent running over the end

    #if { ($gSlotsY - ($gPosition+$theMove+1)) >= 0 } {
	#EnableSlot $w $gSlotList(0,[expr $gSlotsY - ($gPosition+$theMove+1)])
    #}
    #if { ($gSlotsY - ($gPosition+$theMove+2)) >= 0 } {
	#EnableSlot $w $gSlotList(0,[expr $gSlotsY - ($gPosition+$theMove+2)])
    #}
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

    $w delete "nine"
    $w delete "eight"
    $w delete "seven"
    
    ## May3, instead of delete marker here, i will need to INIT marker to 5.
    $w delete marker
    CreateMarker $w 6.25 

    ## Also, i will need to disable and enable embelleshments.
    
    ## reset gPosition for 3 by 3 case
    set gPosition 3375000

    #GS_EnableSlotEmbellishments $w $gSlotList(0,[expr $gSlotsY - 1])
    #GS_EnableSlotEmbellishments $w $gSlotList(0,[expr $gSlotsY - 2])
}





#################################################################
##########################################################
###################################################
##########################################################
##########################################################
##########################################################
##########################################################
##########################################################
# DO NOT MODIFY THE CODE BELOW. BELOW CONTAINS THE MODIFIED 
# VERSION of Dan Garcia's original gamesman.tcl file.
# I modified it so it can accomodate my game.
##########################################################
##########################################################
##########################################################
##########################################################
##########################################################
##########################################################
##########################################################
##########################################################



#############################################################################
##
## SetHelpWindow
##
## Set the fields in the help window if it's open
##
#############################################################################

proc SetHelpWindow { s1 s2 s3 } {
    if { [winfo exists .winHelp] } \
	".winHelp.f1.mesObject  config -text {$s1};
         .winHelp.f2.mesType    config -text {$s2};
         .winHelp.f3.mesPurpose config -text {$s3}"
}

#############################################################################
##
## ClearHelpWindow
##
## Clear the fields in the help window if it's open
##
#############################################################################

proc ClearHelpWindow {} {
    if { [winfo exists .winHelp] } {
	.winHelp.f1.mesObject  config -text ""
	.winHelp.f2.mesType    config -text ""

	### This is a hack to keep the window wide.

	.winHelp.f3.mesPurpose config -text "                                                                                                                                                             "
    }
}
#############################################################################
##
## SetupHelp
##
## Append the binding of 'SetHelpWindow' to whatever existed already in the
## binding for entering the widget w and 'ClearHelpWindow' for leaving it. 
##
#############################################################################

proc SetupHelp { w s1 s2 s3 } {

    ### If this is the first time
    if { [bind $w <Enter>] == "" } {
	set theBinding [bind [winfo class $w] <Any-Enter>]
	bind $w <Enter> +$theBinding
    }
    bind $w <Enter> "+SetHelpWindow {$s1} {$s2} {$s3}"

    ### If this is the first time
    if { [bind $w <Leave>] == "" } {
	set theBinding [bind [winfo class $w] <Any-Leave>]
	bind $w <Leave> +$theBinding
    }
    bind $w <Leave> +ClearHelpWindow
}

#############################################################################
##
## EnableHelps
##
## There are a couple of widgets which will be enabled and disabled often
## because they bring up new windows, so here we have set up a system for
## enabling them easily so we don't have to copy the SetupHelp string many
## times. This procedure is called to print the ENABLED text.
##
#############################################################################

proc EnableHelps { w } {
    global kGameName

    ## The first one is a special case because the button can't simultaneously
    ## and have the help window up, so we don't need to worry about the 
    ## enabled string.

    switch $w {
	.f1.butHelp { ClearHelpWindow }
	.f1.butAbout { SetupHelp $w "About GAMESMAN and its author" "button" "Clicking on it brings up the \"About GAMESMAN and its author\" window" }
	.f5.butOptions { SetupHelp $w "Modify the rules for $kGameName" "button" "Clicking on it brings up another window to modify the rules of $kGameName" }
	default      { SetupHelp $w "$w" "Unknown type" "Unknown Enable" }
    }
}

#############################################################################
##
## DisableHelps
##
## There are a couple of widgets which will be enabled and disabled often
## because they bring up new windows, so here we have set up a system for
## enabling them easily so we don't have to copy the SetupHelp string many
## times. This procedure is called to print the DISABLED text.
##
#############################################################################

proc DisableHelps { w } {
    global kGameName

    switch $w {
	.f1.butHelp { SetupHelp $w "User Interface Balloon Help" "disabled button" "This button is disabled because the \"User Inteface Balloon Help\" window is already open. Clicking on this button will have no effect." }
	.f1.butAbout { SetupHelp $w "About GAMESMAN and its author" "disabled button" "This button is disabled because the \"About GAMESMAN and its author\" window is already open. Clicking on this button will have no effect." }
	.f5.butOptions { SetupHelp .f5.butOptions "Modify the rules for $kGameName" "disabled button" "This button is disabled because the window to modify the rules of $kGameName is already open. Clicking on this button will have no effect." }
	default      { SetupHelp $w "$w" "Unknown type" "Unknown Disable" }
    }
}

#############################################################################
##
## InitConstants
##
## This procedure is used to reserve some keywords (which always begin with
## 'k' to remember they're constants to make the code easier to read. 
## Constants never change. Nice to know somethings don't change around here.
##
#############################################################################

proc InitConstants {} {

    ### These are used in the routine that draws the pieces - we want a small
    ### piece used as a label but a larger piece used in the playing board.

    global kBigPiece
    set kBigPiece 1

    global kSmallPiece
    set kSmallPiece 0

    ### Set the color and font of the labels

    global kLabelFont
    set kLabelFont { Helvetica 12 bold }

    global kLabelColor
    set kLabelColor grey40

    ### Set the animation speed

    global gAnimationSpeed
    set gAnimationSpeed 10 
    ##RUBKIS, make slower
}

#############################################################################
##
## InitWindow
##
## Here we initialize the window. Set up fonts, colors, widgets, helps, etc.
##
#############################################################################

proc InitWindow {} {

    ## make new variable for marker
    #global marker

    ### Import some konstants

    global kBigPiece kSmallPiece
    global kLabelFont kLabelColor

    ### Import the game-specific global variables
    
    global kGameName
    global gPosition
    global gSlotSize
    global xbmLeft xbmRight
    global kMovesOnAllTheTime
    global kEditInitialPosition

    ### Set the xbm file globals

    ### Recall that kRootDir was set for us by the module
    global kRootDir

    global gifDanGarcia
    set gifDanGarcia "$kRootDir/../bitmaps/DanGarcia.gif"
    
    global xbmKrusty bitmapKrusty
    set xbmKrusty "$kRootDir/../bitmaps/krusty.xbm"
    image create bitmap bitmapKrusty \
       -file $xbmKrusty \
       -maskfile $xbmKrusty

    global xbmLightGrey
    set xbmLightGrey "$kRootDir/../bitmaps/lightgrey.xbm"

    global xbmGrey
    set xbmGrey "$kRootDir/../bitmaps/grey.xbm"

    global xbmBlank
    set xbmBlank "$kRootDir/../bitmaps/blank.xbm"

    ### Set the radiobutton globals
    
    global varMoves
    if { $kMovesOnAllTheTime } {
	set varMoves validMoves
    } else {
	set varMoves noMoves
    }
    
    global varPredictions
    set varPredictions butOn
    
    global varObjective
    set varObjective butStandard

    ### Test to see what the window's borders are and set globals
    ### This is really a hack - first we position the window at 0,0
    ### then ask what its rootx and rooty are - these are the widths
    ### of the borders.

    global gBorderX
    global gBorderY
    set gBorderX [winfo rootx .]
    set gBorderY [winfo rooty .]

    ### Set the Title and geometry of the window
    
    wm title    . "GAMESMAN $kGameName v2.0 (1999-05-01)"
#    wm geometry . "+1282+1026"
#    wm geometry . "+0+0"
    wm geometry . "+10+10"
    
    ### Remove the console window on the Mac

    global tcl_platform
    if { $tcl_platform(platform) == "macintosh" || \
         $tcl_platform(platform) == "windows" } {
	console hide
    }

    ### This will be set depending on what the user clicks

    global gHumanGoesFirst

    ### f0 = The Status frame (Status: blah blah)
    
    frame .f0 \
    -borderwidth 2 \
	-relief raised
    
    label .f0.labStatus \
    -font $kLabelFont \
	-text "Status:" \
	-foreground $kLabelColor
    
    message .f0.mesStatus \
	-font $kLabelFont \
	-text "Modify the rules if you wish,\nthen click the \"Start\" button." \
    -justify center \
    -width 500

    ### f5 = The Modify... frame

    frame .f5 \
	-borderwidth 2 \
	-relief raised
    
    label .f5.labModify \
	-font $kLabelFont \
	-text "Modify:" \
	-foreground $kLabelColor
    
    button .f5.butOptions \
	-text "The rules..." \
	-font $kLabelFont \
	-command DoGameSpecificOptions

    button .f5.butInitialPosition \
	-text "The starting position..." \
	-font $kLabelFont \
	-command DoEditInitialPosition

    if { !$kEditInitialPosition } {
	.f5.butInitialPosition config -state disabled
    }

    ### f2 = The Play frame (Play: Human, Computer & Go 1st, Computer & Go 2nd)
    
    frame .f2 \
	-borderwidth 2 \
	-relief raised
    
    label .f2.labPlay \
	-font $kLabelFont \
	-text "Play:" \
	-foreground $kLabelColor
    
    button .f2.butHuman \
	-text "Human" \
	-font $kLabelFont \
	-command "DoHuman" \
	-state disabled
    
    button .f2.butComputer1 \
	-text "Me & go first" \
	-font $kLabelFont \
	-command "set gHumanGoesFirst 1 ; DoComputer" \
	-state disabled
    
    button .f2.butComputer2 \
	-text "Me & go second" \
	-font $kLabelFont \
	-command "set gHumanGoesFirst 0 ; DoComputer" \
	-state disabled
    
    ### f1 = The Setup frame (Setup: About,Start,Rules,Help,Quit)
    
    frame .f1 \
	-borderwidth 2 \
	-relief raised
    
    label .f1.labSetup \
	-font $kLabelFont \
	-text "Setup:" \
	-foreground $kLabelColor
    
    button .f1.butAbout \
	-image bitmapKrusty \
	-command DoAbout

    button .f1.butStart \
	-text "Start" \
	-font $kLabelFont \
	-command DoStart

    button .f1.butHelp \
	-text "Help..." \
	-font $kLabelFont \
	-command DoHelp

    button .f1.butAnalyze \
	-text "Analyze" \
	-font $kLabelFont \
	-command "puts stdout Analyze"
    
    button .f1.butQuit  \
	-text "Quit" \
	-font $kLabelFont \
	-borderwidth 3 \
	-command "destroy ."
    
    ### The Names frame = label and Entry
    
    frame .f3 \
	-borderwidth 2 \
	-relief raised
    
    label .f3.labName1 \
	-font $kLabelFont \
	-text "Left:" \
	-foreground $kLabelColor

    label .f3.cl -bitmap @$xbmLeft -foreground blue

    entry .f3.entPlayer1 \
	-relief sunken \
	-font $kLabelFont \
	-width 15

    label .f3.labName2 \
	-font $kLabelFont \
	-text "Right:" \
	-foreground $kLabelColor

    label .f3.cr -bitmap @$xbmRight -foreground red

    entry .f3.entPlayer2 \
	-relief sunken \
	-font $kLabelFont \
	-width 15
    
    .f3.entPlayer1 insert 0 "Dan"
    .f3.entPlayer2 insert 0 "The computer"

    ### Set up the bindings so that if the player changes the names and
    ### then moved the cursor out of the field, the names are updated on the
    ### game playing window as well.

    foreach i { 1 2 } {
	bind .f3.entPlayer$i <Leave> { 
	    if [winfo exists .winBoardControl] { 
		HandlePredictions
		HandleTurn
	    }
	}
    }
    
    ### Pack everything in
    
    pack append .f0 \
	    .f0.labStatus {left} \
	    .f0.mesStatus {left expand fill}
    
    pack append .f5 \
	    .f5.labModify          {left} \
	    .f5.butOptions         {left expand fill} \
	    .f5.butInitialPosition {left expand fill}
    
    pack append .f1 \
	    .f1.labSetup   {left } \
	    .f1.butAbout   {left pady 10 expand fill } \
	    .f1.butStart   {left pady 10 expand fill } \
	    .f1.butHelp    {left pady 10 expand fill } \
	    .f1.butQuit    {left pady 10 expand fill } 
    
    pack append .f2 \
	    .f2.labPlay      {left} \
	    .f2.butHuman     {left pady 10 padx 10 expand fill } \
	    .f2.butComputer1 {left pady 10 padx 10 expand fill } \
	    .f2.butComputer2 {left pady 10 padx 10 expand fill } 

    pack append .f3 \
	    .f3.labName1   {left fill} \
	    .f3.cl         {left} \
	    .f3.entPlayer1 {left fill expand} \
	    .f3.labName2   {left fill} \
	    .f3.cr         {left} \
	    .f3.entPlayer2 {left fill expand}
    
    pack append . \
	    .f0 {top fill} \
	    .f3 {top expand fill} \
	    .f2 {top expand fill} \
	    .f5 {top expand fill} \
	    .f1 {top expand fill}
    
    ### This is completely a hack, but it looks good. When I change my font
    ### I'll have to find the right way to do this. Why do I keep saying that?

    set theWidth 7
    foreach i { \
	    .f0.labStatus \
	    .f5.labModify \
	    .f1.labSetup \
	    .f2.labPlay \
	    .f3.labName1 \
	} {
	$i config -width 7 -anchor e
    }

    ### Set up all the help strings for the widgets, the last thing always.

    SetupHelpStringsForWindow theMainWindow
}

#############################################################################
##
## SetupHelpStringsForWindow
##
## This procedure's job is to collect the Help Strings in one place as much
## as possible. This way it's really easy to edit/find them.
##
#############################################################################

proc SetupHelpStringsForWindow { keyword } {
    global kGameName
    global kStandardString kMisereString

    ### Depending on the keyword, we setup different help strings for widgets

    switch $keyword {

	theMainWindow {

	    SetupHelp .f0.mesStatus \
		    "Game status" \
		    "dynamic text" \
		    "The status of the game (whether it's been solved or not, value, etc) is shown here"
	    
	    EnableHelps .f1.butAbout

	    SetupHelp .f1.butStart \
		    "Start the game" \
		    "button" \
		    "Clicking on it solves the game through exhaustive search and returns the value to the status window"

	    EnableHelps .f5.butOptions
	    EnableHelps .f1.butHelp

	    SetupHelp .f1.butQuit \
		    "Quit the game" \
		    "button" \
		    "Clicking on it quits the game\n"
	    
	    SetupHelp .f3.entPlayer1 \
		    "Left player name" \
		    "entry field" \
		    "Click the cursor in here then edit it to change the name of the left player"
	    SetupHelp .f3.entPlayer2 \
		    "Right player name" \
		    "entry field" \
		    "Click the cursor in here then edit it to change the name of the right player"
	    SetupHelp .f3.cr \
		    "Right player's playing piece" \
		    "picture" \
		    "This is a picture of the piece used by the right player\n"
	    SetupHelp .f3.cl \
		    "Left player's playing piece" \
		    "picture" \
		    "This is a picture of the piece used by the left player\n"
	    
	    SetupHelp .f2.butHuman \
		    "Play against a Human" \
		    "disabled button" \
		    "Click on this button to play $kGameName against a human. Left goes first. This is currently disabled because you need to click the 'start' button first to solve the game."
	    SetupHelp .f2.butComputer1 \
		    "Play against the computer and go first" \
		    "disabled button" \
		    "Click on this button to play $kGameName against the computer and go first. The computer will be right. This is currently disabled because you need to click the 'start' button first to solve the game."
	    SetupHelp .f2.butComputer2\
		    "Play against the computer and go second" \
		    "disabled button" \
		    "Click on this button to play $kGameName against the computer and go second. The computer will be right. This is currently disabled because you need to click the 'start' button first to solve the game."

	}   

	justClickedStart {

	    ### Here we disable start,rules & enable human, computer1 & 2.

	    SetupHelp .f2.butHuman \
		    "Play against a Human" \
		    "button" \
		    "Click on this button to play $kGameName against a human. Left goes first."
	    SetupHelp .f2.butComputer1 \
		    "Play against the computer and go first" \
		    "button" \
		    "Click on this button to play $kGameName against the computer and go first. The computer will be right."
	    SetupHelp .f2.butComputer2 \
		    "Play against the computer and go second" \
		    "button" \
		    "Click on this button to play $kGameName against the computer and go second. The computer will be right."
	    SetupHelp .f1.butStart \
		    "Start the game" \
		    "disabled button" \
		    "Since you have already solved the game, there is no need for this button so it is disabled. Clicking on it will have no effect."
	    SetupHelp .f5.butOptions "\
		    Modify the rules for $kGameName" \
		    "disabled button" \
		    "This button is disabled because you have already solved the game and built the database of moves and may no longer change the rules of the game. Clicking on this button will have no effect." 

	}

	AboutWindow {

	    DisableHelps .f1.butAbout
	    SetupHelp .winAbout.msgGAMESMAN \
		    "Title" \
		    "static text" \
		    "The name of the toolkit\n"
	    SetupHelp .winAbout.msgAboutwww \
		    "WWW address" \
		    "static text" \
		    "The WWW address for more information about GAMESMAN"
	    SetupHelp .winAbout.msgAbout1 \
		    "Author name, address and status" \
		    "static text" \
		    "This is information about me\n"
	    SetupHelp .winAbout.labDanPhoto \
		    "Dan Garcia" \
		    "picture" \
		    "\"I'm so pretty\"\n-Cassius Clay"
	    SetupHelp .winAbout.msgAbout2 \
		    "Acronym explained" \
		    "static text" \
		    "This is what GAMESMAN stands for\n"
	    SetupHelp .winAbout.butQuit \
		    "OK button" \
		    "button" \
		    "Clicking this button closes the \"About GAMESMAN and its author\" window"

	}

	GameSpecificWindow {

	    DisableHelps .f5.butOptions
	    SetupHelp .gameSpecificOptions.f0.butStandard \
		    "$kStandardString" \
		    "radiobutton" \
		    "Selecting this radiobutton sets the game's rules so that $kStandardString.  This is often called the \"standard\" game."
	    SetupHelp .gameSpecificOptions.f0.butReverse \
		    "$kMisereString" \
		    "radiobutton" \
		    "Selecting this rediobutton sets the game's rules so that $kMisereString. This is called the \"mis\xe8re\" game."
	    SetupHelp .gameSpecificOptions.butQuit "\
		    OK button" \
		    "button" \
		    "Clicking this button closes the \"Modify the rules for $kGameName\" window"

	}

	HelpWindow {

	    DisableHelps .f1.butHelp
	    SetupHelp .winHelp.f0.mesStatus \
		    "User Interface Balloon Help status" \
		    "dynamic text" \
		    "This tells the user how to use Balloon help.\n"
	    SetupHelp .winHelp.f1.mesObject \
		    "Name of Object" \
		    "dynamic text" \
		    "This describes the object being pointed to by the cursor."
	    SetupHelp .winHelp.f2.mesType \
		    "Type of graphics object" \
		    "dynamic text" \
		    "This describes the type of object being pointed to. (e.g. picture, button, etc.)"
	    SetupHelp .winHelp.f3.mesPurpose \
		    "Explanation of the object" \
		    "dynamic text" \
		    "This provides a short description of the object being pointed to by the cursor."
	    SetupHelp .winHelp.butQuit \
		    "OK button" \
		    "button" \
		    "Clicking this button closes the \"User Interface Balloon Help\" window."
	}

	BoardWindow {

	    SetupHelp .winBoardControl.f0.mesToMove \
		    "GAMESMAN $kGameName move instructions" \
		    "dynamic text" \
		    "This tells the user how to make a move in the current game."
	    
	    SetupHelp .winBoardControl.f6.mesToWin \
		    "GAMESMAN $kGameName winning instructions" \
		    "dynamic text" \
		    "This tells the user how to win in the current game."
	    
	    SetupHelp .winBoardControl.f4.predictions \
		    "Predicted outcome" \
		    "dynamic text" \
		    "The predicted outcome of the game is shown here if the \"On\" radiobutton to the left is selected."
	    
	    SetupHelp .winBoardControl.f4.butOn \
		    "Turn predictions on" \
		    "radiobutton" \
		    "Clicking on this radiobutton turns on the prediction on the right"
	    
	    SetupHelp .winBoardControl.f4.butOff \
		    "Turn predictions off" \
		    "radiobutton" \
		    "Clicking on this radiobutton turns off and disables the prediction on the right"
	    
	    SetupHelp .winBoardControl.f5.c \
		    "Current player's playing piece" \
		    "picture" \
		    "This is a picture of the piece used by the current player.\n"
	    
	    SetupHelp .winBoardControl.f5.mesTurn \
		    "Current player" \
		    "dynamic text" \
		    "This field represents whose turn it is and whether they are left or right."
	    
	    SetupHelp .winBoardControl.f1.noMoves \
		    "Show no moves" \
		    "radiobutton" \
		    "Selecting this radiobutton hides all the moves from the board"
	    
	    SetupHelp .winBoardControl.f1.validMoves \
		    "Show all moves" \
		    "radiobutton" \
		    "Selecting this radiobutton shows all of the moves available to the current player displayed in a generic color."
	    
	    SetupHelp .winBoardControl.f1.valueMoves \
		    "Show all value moves" \
		    "radiobutton" \
		    "Selecting this radiobutton shows all of the moves available to the current player, color-coded according to whether the move is winning, tieing or losing"
	    
	    SetupHelp .winBoardControl.f2.undo \
		    "Undo" \
		    "disabled button" \
		    "Undoes the most recent human move by backing up one step in the game tree. It is currently disabled because it is not implemented yet."
	    
	    SetupHelp .winBoardControl.f2.redo \
		    "Redo" \
		    "disabled button" \
		    "Undoes the most recent undo (we call this a \"Redo\") by moving forward down the game tree. It is currently disabled because it is not implemented yet."
	    
	    SetupHelp .winBoardControl.f2.newGame \
		    "New Game" \
		    "button" \
		    "Resets the game to the initial position.\n"
	    
	    SetupHelp .winBoardControl.f2.abort \
		    "Abort" \
		    "button" \
		    "Ends the current game and closes the \"GAMESMAN $kGameName\" window."
	    
	}

	default {
	    puts "Error: Couldn't find switch in SetupHelpStringsForWindow"
	}
    }
}
#############################################################################
##
## DoStart
##
## This is what we do when the user clicks the 'Start' button.
##
#############################################################################

proc DoStart {} {
    global kGameName varObjective gPosition gInitialPosition

    .f0.mesStatus config -text "Solving $kGameName...\n(This may take a while)"

    ### Send initial game-specific options to C procs.

    eval [concat C_SetGameSpecificOptions [expr {$varObjective == "butStandard"}] \
	    [GS_GetGameSpecificOptions]]

    set gPosition $gInitialPosition
    . config -cursor watch
    update
    set theValue [C_DetermineValue $gPosition]
    . config -cursor {}
    update
    
    ### Set up the help strings
    
    SetupHelpStringsForWindow justClickedStart

    ### Enable the play buttons and disable the other ones
    
    EnablePlayButtons
    .f1.butStart config -state disabled
    .f5.butOptions config -state disabled
    .f5.butInitialPosition config -state disabled
    
    .f0.mesStatus config -text "Solved. The Game is a $theValue.\nChoose a Play option."

    ### Once we've built the database, you can't change the rules!

    if [winfo exists .gameSpecificOptions] { 
	destroy .gameSpecificOptions 
    } 
}

#############################################################################
##
## DisablePlayButtons
##
## Turn off the play buttons - only one game at a time!
##
#############################################################################

proc DisablePlayButtons {} {
    .f2.butHuman config -state disabled
    .f2.butComputer1 config -state disabled
    .f2.butComputer2 config -state disabled
}

#############################################################################
##
## EnablePlayButtons
##
## Turn on the play buttons - no game is being played so play on!
##
#############################################################################

proc EnablePlayButtons {} {
    .f2.butHuman config -state normal
    .f2.butComputer1 config -state normal
    .f2.butComputer2 config -state normal
}

#############################################################################
##
## DoHuman
##
## The "Play Human" button was just clicked, so set up some variables and play!
##
#############################################################################

proc DoHuman {} {
    global gAgainstComputer
    global gHumanGoesFirst
    global gPlayerOneTurn
    global gInitialPosition
    global gPosition

    set gAgainstComputer 0
    set gHumanGoesFirst 1
    set gPlayerOneTurn 1
    set gPosition $gInitialPosition

    .f0.mesStatus config -text "Playing against\na human"
    DoBoard
    HandlePredictions
    HandleTurn
}

#############################################################################
##
## DoComputer
##
## The "Play Computer and the $theHumanGoesFirst button was clicked"
## It is assumed gHumanGoesFirst has been set already.
##
#############################################################################

proc DoComputer {} {
    global gAgainstComputer
    global gHumanGoesFirst
    global gPlayerOneTurn
    global gInitialPosition
    global gPosition

    .f0.mesStatus config -text "Playing against a\nperfect computer opponent"
    set gAgainstComputer 1
    set gPlayerOneTurn $gHumanGoesFirst
    set gPosition $gInitialPosition

    ### Create the board, make the computers move if nec., update 
    ### the predictions and the turn.

    DoBoard
    if { !$gPlayerOneTurn } { 
	DoComputersMove 
    } else {
	HandleMoves
	HandlePredictions
	HandleTurn
    }
}

#############################################################################
##
## DoAbout
##
## The "About the Author" button was just clicked.
##
#############################################################################

proc DoAbout {} {
    global gifDanGarcia kLabelFont

    toplevel .winAbout
    wm title .winAbout "About GAMESMAN and its author"
    
    ### Disable the last window

    .f1.butAbout config \
        -state disabled
    
    ### Put the messages in

    message .winAbout.msgGAMESMAN \
	-font { Helvetica 18 bold } \
        -relief raised \
        -width 500 \
        -borderwidth 1 \
        -text "GAMESMAN"

    message .winAbout.msgAbout1 \
        -font $kLabelFont \
        -relief raised \
        -width 600 \
        -borderwidth 1 \
        -justify center \
        -text "by Dan Garcia (ddgarcia@cs.berkeley.edu)\na Grad Student at the University of California at Berkeley"

    message .winAbout.msgAboutwww \
        -font $kLabelFont \
        -relief raised \
        -width 600 \
        -borderwidth 1 \
        -justify center \
        -text "http://www.cs.berkeley.edu/~ddgarcia/software/gamesman/"

    message .winAbout.msgAbout2 \
        -font { Helvetica 14 bold } \
        -relief raised \
        -width 500 \
        -borderwidth 1 \
        -text \
	"(G)ame-independent\n(A)utomatic\n(M)ove-tree\n(E)xhaustive\n(S)earch,\n(M)aniuplation\n(A)nd\n(N)avigation"

    ### New for color icon of me

    catch { image delete imageDanGarcia }
    image create photo imageDanGarcia -file $gifDanGarcia

    label .winAbout.labDanPhoto \
        -image imageDanGarcia \
        -relief raised \
        -background white \
        -borderwidth 2
    
    ### Make the about box modeless
    
    button .winAbout.butQuit \
        -text "OK"  \
	-font $kLabelFont \
	-borderwidth 2 \
        -command {
            .f1.butAbout config -state normal
            destroy .winAbout
            EnableHelps .f1.butAbout
            ClearHelpWindow
    }

    ### Pack the about box
    
    pack append .winAbout \
         .winAbout.msgGAMESMAN {top fill} \
         .winAbout.msgAboutwww   {top fill expand} \
         .winAbout.msgAbout1   {top fill expand} \
         .winAbout.butQuit   {bottom expand fill} \
         .winAbout.labDanPhoto  {left} \
         .winAbout.msgAbout2   {right fill expand}

    ### Set up the help fields
	
    SetupHelpStringsForWindow AboutWindow
}


#############################################################################
##
## GeometryRightOf
##
## This procedure returns the Geometry string needed to place a window
## directly to the right of with the same height as the requested window, w
##
#############################################################################

proc GeometryRightOf { w } {
    global gBorderX gBorderY

    return "+[expr [winfo x $w] + [winfo width $w] + $gBorderX]+[expr [winfo y $w] - $gBorderY]"
}

#############################################################################
##
## GeometryBelow
##
## This procedure returns the Geometry string needed to place a window
## directly to the right of the requested window, w
##
#############################################################################

proc GeometryBelow { w } {
    global gBorderX gBorderY

    return "+[expr [winfo x $w] - $gBorderX]+[expr [winfo y $w] + [winfo height $w] + $gBorderX]"
}

#############################################################################
##
## DoGameSpecificOptions
##
## The 'modify rules' button was just clicked. Now we set up a game-specific
## rule modification setup.
##
#############################################################################

proc DoGameSpecificOptions {} {
    global kLabelFont kLabelColor kGameName
    global kStandardString kMisereString
    
    .f5.butOptions config -state disabled
    
    toplevel .gameSpecificOptions
    wm title .gameSpecificOptions "Modify the rules for $kGameName"
    wm geometry .gameSpecificOptions [GeometryRightOf .]
    
    ### The Radio Buttons for Standard/Reverse game
    
    frame .gameSpecificOptions.f0 \
	    -borderwidth 2 \
	    -relief raised
    
    message .gameSpecificOptions.f0.labMoves \
	    -font $kLabelFont \
	    -text "What would you like your winning condition to be:" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton .gameSpecificOptions.f0.butStandard \
	    -text "$kStandardString (standard)" \
	    -font $kLabelFont \
	    -variable varObjective \
	    -value butStandard
    
    ### On Mac, use \216 but on UNIX and PCs use \xe8

    global tcl_platform
    if { $tcl_platform(platform) == "macintosh" } {
	radiobutton .gameSpecificOptions.f0.butReverse  \
		-text "$kMisereString (misre)"  \
		-font $kLabelFont \
		-variable varObjective \
		-value butReverse
    } else {
	radiobutton .gameSpecificOptions.f0.butReverse  \
		-text "$kMisereString (mis\xe8re)"  \
		-font $kLabelFont \
		-variable varObjective \
		-value butReverse
    }
    
    button .gameSpecificOptions.butQuit \
	    -text "OK" \
	    -font $kLabelFont \
	    -command {
	.f5.butOptions config -state normal
	destroy .gameSpecificOptions
	EnableHelps .f5.butOptions
	ClearHelpWindow
    }
    
    ### Pack it all in
    
    pack append .gameSpecificOptions.f0 \
	    .gameSpecificOptions.f0.labMoves {left} \
	    .gameSpecificOptions.f0.butStandard {top expand fill} \
	    .gameSpecificOptions.f0.butReverse {top expand fill} 
    
    pack append .gameSpecificOptions \
	    .gameSpecificOptions.f0 {top expand fill}

    GS_AddGameSpecificGUIOptions .gameSpecificOptions
    
    pack append .gameSpecificOptions \
	    .gameSpecificOptions.butQuit   {bottom expand fill} 
    
    ### Set up the help fields
	
    SetupHelpStringsForWindow GameSpecificWindow
}

#############################################################################
##
## DoHelp
##
## The Help button was just clicked. What are you going to do about it, huh?
## Think you're a touch guy, what are you going to do about that?
##
#############################################################################

proc DoHelp {} {
    global kLabelFont kLabelColor
    
    ### Disable the last button

    .f1.butHelp config -state disabled
    
    ### Create a new window and name it

    toplevel .winHelp
    wm title .winHelp "User Interface Balloon Help (move cursor around)"
    
    frame .winHelp.f0 \
	    -borderwidth 2 \
        -relief raised
    frame .winHelp.f1 \
        -borderwidth 2 \
        -relief raised
    frame .winHelp.f2 \
        -borderwidth 2 \
        -relief raised
    frame .winHelp.f3 \
        -borderwidth 2 \
        -relief raised

    #### The Status window and message

    label .winHelp.f0.labStatus \
        -font $kLabelFont \
        -text "Status:" \
        -foreground $kLabelColor

    message .winHelp.f0.mesStatus \
        -font $kLabelFont \
        -text "This is Balloon help. To get help,\nposition your cursor over something" \
	-justify center \
	-width 500
    
    #### The Object (whatisit?)

    label .winHelp.f1.labObject \
        -font $kLabelFont \
        -text "Object:" \
        -foreground $kLabelColor

    message .winHelp.f1.mesObject \
        -font $kLabelFont \
        -text "" \
	-justify center \
	-width 500
    
    #### The Type (whatkindofthingisit?)

    label .winHelp.f2.labType \
        -font $kLabelFont \
        -text "Type:" \
        -foreground $kLabelColor

    message .winHelp.f2.mesType \
        -font $kLabelFont \
        -text "" \
	-justify center \
	-width 500
    
    #### The Purpose

    label .winHelp.f3.labPurpose \
        -font $kLabelFont \
        -text "Explanation:" \
        -foreground $kLabelColor

    message .winHelp.f3.mesPurpose \
        -font $kLabelFont \
        -text "                                                                                                                                                             " \
	-justify center \
	-width 500
    
    button .winHelp.butQuit \
        -text "OK" \
	-font $kLabelFont \
        -command { 
	    .f1.butHelp config -state normal
            destroy .winHelp
	    EnableHelps .f1.butHelp
	    ClearHelpWindow
	}
    
    #### Pack it all in

    pack append .winHelp.f0 \
         .winHelp.f0.labStatus {left} \
         .winHelp.f0.mesStatus {left expand fill}
         
    pack append .winHelp.f1 \
         .winHelp.f1.labObject {left} \
         .winHelp.f1.mesObject {left expand fill}
         
    pack append .winHelp.f2 \
         .winHelp.f2.labType {left} \
         .winHelp.f2.mesType {left expand fill}
         
    pack append .winHelp.f3 \
         .winHelp.f3.labPurpose {left} \
         .winHelp.f3.mesPurpose {left expand fill}
         
    pack append .winHelp \
	    .winHelp.f1 { top expand fill } \
	    .winHelp.f2 { top expand fill } \
	    .winHelp.f3 { top expand fill } \
	    .winHelp.butQuit   {bottom expand fill} 
    
    ### Set up the help fields
    
    SetupHelpStringsForWindow HelpWindow

    ### This is completely a hack, but it looks good. When I change my font
    ### I'll have to find the right way to do this. Why do I keep saying that?

    foreach i { \
	    .winHelp.f1.labObject \
	    .winHelp.f2.labType \
	    .winHelp.f3.labPurpose \
	} {
	$i config -width 11 -anchor e
    }

    ### This is clearly a hack to get the width of the Balloon Help constant

    .winHelp.butQuit config -width 64
    .winHelp.f3.labPurpose config -pady 4
    .winHelp.f3.labPurpose config -height 5
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
## DoBoard
##
## We've started a game and set up a few preliminary variables. Let's play!
## I.e. let's create a couple of windows and set them up to play the game. 
##
#############################################################################

proc DoBoard {} {
    global varMoves varPredictions varObjective
    global gSlotsX gSlotsY gSlotSize kLabelFont kLabelColor
    global kGameName
    global kToMove kToWinStandard kToWinMisere
    global kMovesOnAllTheTime
    
    ### Since we can only play one game at a time...

    DisablePlayButtons

    ### The control window - create it, name it and place it.

    toplevel .winBoardControl
    wm title .winBoardControl "GAMESMAN $kGameName control"
    wm geometry .winBoardControl [GeometryRightOf .]
    
    frame .winBoardControl.f0 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f1 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f2 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f3
    frame .winBoardControl.f4 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f5 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f6 \
        -borderwidth 2 \
        -relief raised
    
    #### The "To Move" window and message
    
    label .winBoardControl.f0.labToMove \
        -font $kLabelFont \
        -text "To Move:" \
        -foreground $kLabelColor

    message .winBoardControl.f0.mesToMove \
        -font $kLabelFont \
        -text $kToMove \
	-justify center \
	-width 500
    
    #### The "To Win" window and message

    label .winBoardControl.f6.labToWin \
        -font $kLabelFont \
        -text "To Win:" \
        -foreground $kLabelColor

    if { $varObjective == "butStandard" } {
	set theToWinString $kToWinStandard
    } else {
	set theToWinString $kToWinMisere
    }

    message .winBoardControl.f6.mesToWin \
        -font $kLabelFont \
        -text $theToWinString \
	-justify center \
	-width 500
    
    #### The Whose-turn-is-it window and message
    
    label .winBoardControl.f5.labTurn \
        -font $kLabelFont \
        -text "Turn:" \
        -foreground $kLabelColor

    label .winBoardControl.f5.mesTurn \
        -font $kLabelFont \
        -text ""

    label .winBoardControl.f5.c
    
    #### The Predictions window and message

    label .winBoardControl.f4.labPredictions \
        -font $kLabelFont \
        -text "Prediction:" \
        -foreground $kLabelColor

    radiobutton .winBoardControl.f4.butOn \
        -text "On" \
	-font $kLabelFont \
        -command HandlePredictions \
        -variable varPredictions \
        -value butOn

    radiobutton .winBoardControl.f4.butOff \
        -text "Off" \
	-font $kLabelFont \
        -command HandlePredictions \
        -variable varPredictions \
	-value butOff

    label .winBoardControl.f4.predictions \
        -font $kLabelFont
    
    ### The Radio Buttons for Displaying Moves
    
    label .winBoardControl.f1.labShow \
        -font $kLabelFont \
        -text "Show:" -width 6\
        -foreground $kLabelColor
    
    radiobutton .winBoardControl.f1.noMoves \
        -text "Nothing" \
	-font $kLabelFont \
        -variable varMoves \
	-value noMoves \
        -command DeleteMoves

    radiobutton .winBoardControl.f1.validMoves \
        -text "Moves" \
	-font $kLabelFont \
        -variable varMoves \
	-value validMoves \
        -command ShowMoves 

    radiobutton .winBoardControl.f1.valueMoves  \
        -text "Value Moves" \
	-font $kLabelFont \
        -variable varMoves \
	-value valueMoves \
	-command ShowValueMoves
    
    ### The two bottom buttons, undo and abort
    
    button .winBoardControl.f2.undo \
        -text "Undo" \
	-font $kLabelFont \
	-state disabled

    button .winBoardControl.f2.redo \
        -text "Redo" \
	-font $kLabelFont \
	-state disabled
    
    button .winBoardControl.f2.abort \
	    -text "Abort" \
	    -font $kLabelFont \
	    -command { 
	EnablePlayButtons
	destroy .winBoardControl
	destroy .winBoard
	if { [winfo exists .winValueMoves] } {
	    destroy .winValueMoves
	}
	.f0.mesStatus config -text "Choose a Play Option\n"
    }
    
    button .winBoardControl.f2.newGame \
	    -text "New Game" \
	    -font $kLabelFont \
	    -command { 
	
	### Make the code a bit easier to read
	
	set w .winBoard.c
	
	### Clear the message field

	.f0.mesStatus config -text "Starting a new game\n"

	### Reset the position to the initial position

	set gPosition $gInitialPosition
	set gPlayerOneTurn $gHumanGoesFirst
	
	### Call the Game-specific New Game command

	GS_NewGame $w
	
	### Remove all Dead and Alive tags and reset them to what they were
	### when we reset the game.

	$w dtag tagDead
	$w dtag tagAlive
	$w addtag tagDead withtag tagNotInitial
	$w addtag tagAlive withtag tagInitial

	if { !$gHumanGoesFirst } { DoComputersMove }

	HandleMoves
	HandlePredictions
	HandleTurn
    }
    
    ### Packing it all in.
    
    pack append .winBoardControl.f0 \
	    .winBoardControl.f0.labToMove {left} \
	    .winBoardControl.f0.mesToMove {left expand fill}
         
    pack append .winBoardControl.f6 \
	    .winBoardControl.f6.labToWin {left} \
	    .winBoardControl.f6.mesToWin {left expand fill}
    
    if { $kMovesOnAllTheTime } {
	pack append .winBoardControl.f1 \
		.winBoardControl.f1.labShow {left} \
		.winBoardControl.f1.validMoves {left expand fill} \
		.winBoardControl.f1.valueMoves {left expand fill}
    } else {
	pack append .winBoardControl.f1 \
	    .winBoardControl.f1.labShow {left} \
		.winBoardControl.f1.noMoves {left expand fill} \
		.winBoardControl.f1.validMoves {left expand fill} \
		.winBoardControl.f1.valueMoves {left expand fill}
    }
 
 #         .winBoardControl.f2.undo {left expand fill}
 #        .winBoardControl.f2.redo {left expand fill}

    pack append .winBoardControl.f2 \
         .winBoardControl.f2.newGame {left expand fill} \
         .winBoardControl.f2.abort {left expand fill}
    
    pack append .winBoardControl.f4 \
         .winBoardControl.f4.labPredictions {left} \
         .winBoardControl.f4.predictions {right expand fill} \
         .winBoardControl.f4.butOn {top fill} \
         .winBoardControl.f4.butOff {top fill} 
         
    pack append .winBoardControl.f5 \
        .winBoardControl.f5.labTurn {left} \
	.winBoardControl.f5.c {left} \
        .winBoardControl.f5.mesTurn {left expand fill}
             
    pack append .winBoardControl \
	    .winBoardControl.f0 {top expand fill} \
	    .winBoardControl.f6 {top expand fill} \
	    .winBoardControl.f5 {top expand fill} \
	    .winBoardControl.f4 {top expand fill} \
	    .winBoardControl.f1 {top expand fill} \
	    .winBoardControl.f2 {top fill expand}

    ### This is completely a hack, but it looks good. When I change my font
    ### I'll have to find the right way to do this. Why do I keep saying that?

    foreach i { \
	    .winBoardControl.f0.labToMove \
	    .winBoardControl.f6.labToWin \
	    .winBoardControl.f5.labTurn \
	    .winBoardControl.f4.labPredictions \
	    .winBoardControl.f1.labShow \
	} {
	$i config -width 10 -anchor e
    }

    ### Set up the help strings

    SetupHelpStringsForWindow BoardWindow

    ### Update the size of the window so that I can place things around it.

    update

    ### Now that I've packed and updated .winBoardControl, I can pack and 
    ### place the board below it.

    ### The board window

    toplevel .winBoard
    wm title .winBoard "GAMESMAN $kGameName board"
    wm geometry .winBoard [GeometryBelow .winBoardControl]

    ### The Canvas on which to draw the board
    ### There's some assumption that full orthogonal boards will be used -
    ### this needs to be modified for other boards.

    ### MODIFIED. RUBIKS.
    ## the board is an gSlotSize/10 by gslotSize/10 centimeter play area. :)
    ## I start it at x=0.25 because on unixmachines, theborder gets cutoff! 
    set startBoardX 0.25
    set startBoardY 0.25
    set endBoardX [expr $gSlotSize /10]
    set endBoardY [expr $gSlotSize /10]
    
    set endCanvasX [expr $endBoardX + 0.25]
    set endCanvasY [expr [expr $gSlotSize * 1.65] /10]

    canvas .winBoard.c -width ${endCanvasX}c -height ${endCanvasY}c -background black -relief raised

    # draw the board with border
    set board [ .winBoard.c create rectangle ${startBoardX}c ${startBoardY}c ${endBoardX}c ${endBoardY}c -width 4m \
	    -outline purple -fill #f0f ]

    # draw the columns. Width of each column is just 0.5c
    # startcolumn1 will give us where to start and the widthsize asit depends on gSlotSize
    set widthsize [expr $gSlotSize * 0.006]
    set column1mid [expr [expr $endBoardX / 3] + [expr 1.75 * $startBoardX] ]
    set startcolumn1 [expr $column1mid - [expr $widthsize / 2] ]
    set endcolumn1 [expr $startcolumn1 + $widthsize]

    set column2mid [expr [expr [expr $endBoardX *2] / 3] + [expr $startBoardX *3.5]] 
    set startcolumn2 [expr $column2mid - [expr $widthsize / 2] ]
    set endcolumn2 [expr $startcolumn2 + $widthsize]

    .winBoard.c create rectangle ${startcolumn1}c ${startBoardY}c ${endcolumn1}c ${endBoardY}c -fill purple -outline purple
    .winBoard.c create rectangle ${startcolumn2}c ${startBoardY}c ${endcolumn2}c ${endBoardY}c -fill purple -outline purple

    #### draw the scoreboard
    set scoreredY [expr [expr $gSlotSize * 1.55] / 10]
    set scoreredX [expr $startBoardX + 1]
    set scoreblueX [expr $endBoardX -1 ]
    set scoreblueY [expr $scoreredY + 0.7]

    .winBoard.c create polygon ${scoreredX}c ${scoreredY}c ${scoreblueX}c ${scoreredY}c ${scoreblueX}c ${scoreblueY}c -fill blue
    .winBoard.c create polygon ${scoreredX}c ${scoreredY}c ${scoreredX}c ${scoreblueY}c ${scoreblueX}c ${scoreblueY}c -fill red

    ## draw red circle
    set redcircleX 0.75
    set circleY [expr $scoreredY + 0.5]
    .winBoard.c create oval 0.3c ${scoreredY}c ${redcircleX}c ${circleY}c -fill red
    .winBoard.c create text 0.5c ${circleY}c -text "RED" -fill white

    ## draw blue circle
    set bluecirclestartX [expr $scoreblueX + 0.3]
    set bluecircleendX [expr $bluecirclestartX  +0.5] 
    .winBoard.c create oval ${bluecirclestartX}c ${scoreredY}c ${bluecircleendX}c ${circleY}c -fill blue
    set bluewordstartX [expr $bluecirclestartX + 0.25]
    .winBoard.c create text ${bluewordstartX}c ${circleY}c -text "BLUE" -fill white

    ##.winBoard.c create polygon 1.5c 14.5c 6.25c 13.75c 6.25c 15.25c -fill green
    ##.winBoard.c create polygon 11c 14.5c 6.25c 13.75c 6.25c 15.25c -fill green
    ## 5.25c 14.33c 5.25c 14.66c 2c 14.5c  -fill #0f0 -outline green
    ##.winBoard.c create rectangle 2c 14.4c 10.5c 14.6c -fill darkgreen 
    # make 11 markers
    # Markers are on positions 2, 2.85, 3.7, 4.55, 5.4, 6.25, 7.1, 7.95, 8.8, 9.65, 10.5 
    set modscoreredY [expr $scoreredY + 0.2]
    set modscoreblueY [expr $scoreblueY - 0.2]
    for {set i $scoreredX} {$i <= $scoreblueX} { set i [expr $i+[expr [expr $scoreblueX - $scoreredX] /10 ]]} {
	set x $i
	.winBoard.c create rect ${x}c ${modscoreredY}c ${x}c ${modscoreblueY}c  -fill orange -outline yellow -width 0.125c
    }
    

    # draw special/normal piece announcment.
    ##.winBoard.c create text 1c 10.5c -text "NORMAL\n  PIECE" -fill white
    ##.winBoard.c create text 1c 12.5c -text "SPECIAL\n  PIECE" -fill white

    # draw side markers specifying red/blue side
    ##.winBoard.c create oval {[expr $scoreredX - 0.75]}c ${scoreredY}c {[expr $scoreredX - 0.1 ]}c {[expr $scoreredY + 0.5 ]}c  -fill red -outline red
    
    #3.winBoard.c create oval {[expr $scoreblueX + 0.1]}c ${scoreblueY}c {[expr $scoreblueX + 0.5 ]}c {[expr $scoreblueY - 0.5 ]}c -fill blue -outline blue
    ##.winBoard.c create text 1c 15.75c -text "RED" -fill white
    ##.winBoard.c create text 11.25c 15.75c -text "BLUE" -fill white

    ### Draw the buttons
    ## b and e are the middle
    ## a and f are the left
    ## c and d are the right
    
    ## first button
    set but1bX [expr [expr $gSlotSize /10] / 5.175]
    set but1bY [expr [expr $gSlotSize * 1.05] / 10]
    set but1eY [expr $but1bY + 0.8]
    set but1eX $but1bX
    ##.winBoard.c create text ${but1bX}c ${but1bY}c -text "but1b" -fill white
    ##.winBoard.c create text ${but1eX}c ${but1eY}c -text "but1e" -fill white
    
    set but1aX [expr $but1bX - 1.2]
    set but1aY [expr $but1bY + 1.4]
    set but1fX [expr $but1aX + 0.35]
    set but1fY [expr $but1aY + 0.5]
    ##.winBoard.c create text ${but1aX}c ${but1aY}c -text "but1a" -fill white
    ##.winBoard.c create text ${but1fX}c ${but1fY}c -text "but1f" -fill white

    set but1cX [expr $but1bX + 1.2]
    set but1cY $but1aY
    set but1dX [expr $but1cX - 0.35]
    set but1dY $but1fY
    ##.winBoard.c create text ${but1cX}c ${but1cY}c -text "but1c" -fill white
    ##.winBoard.c create text ${but1dX}c ${but1dY}c -text "but1d" -fill white

    CreateButton2 .winBoard.c $but1aX $but1aY $but1bX $but1bY $but1cX $but1cY $but1dX $but1dY $but1eX $but1eY $but1fX $but1fY normal 1 0 0

    ## second button
    set but1bX [expr [expr $gSlotSize /10] / 1.9]
    set but1bY [expr [expr $gSlotSize * 1.05] / 10]
    set but1eY [expr $but1bY + 0.8]
    set but1eX $but1bX
    ##.winBoard.c create text ${but1bX}c ${but1bY}c -text "but1b" -fill white
    ##.winBoard.c create text ${but1eX}c ${but1eY}c -text "but1e" -fill white
    
    set but1aX [expr $but1bX - 1.2]
    set but1aY [expr $but1bY + 1.4]
    set but1fX [expr $but1aX + 0.35]
    set but1fY [expr $but1aY + 0.5]
    ##.winBoard.c create text ${but1aX}c ${but1aY}c -text "but1a" -fill white
    ##.winBoard.c create text ${but1fX}c ${but1fY}c -text "but1f" -fill white

    set but1cX [expr $but1bX + 1.2]
    set but1cY $but1aY
    set but1dX [expr $but1cX - 0.35]
    set but1dY $but1fY
    
    CreateButton2 .winBoard.c $but1aX $but1aY $but1bX $but1bY $but1cX $but1cY $but1dX $but1dY $but1eX $but1eY $but1fX $but1fY normal 2 0 1

    ## 3rd button
    set but1bX [expr [expr $gSlotSize /10] / 1.175]
    set but1bY [expr [expr $gSlotSize * 1.05] / 10]
    set but1eY [expr $but1bY + 0.8]
    set but1eX $but1bX
    ##.winBoard.c create text ${but1bX}c ${but1bY}c -text "but1b" -fill white
    ##.winBoard.c create text ${but1eX}c ${but1eY}c -text "but1e" -fill white
    
    set but1aX [expr $but1bX - 1.2]
    set but1aY [expr $but1bY + 1.4]
    set but1fX [expr $but1aX + 0.35]
    set but1fY [expr $but1aY + 0.5]
    ##.winBoard.c create text ${but1aX}c ${but1aY}c -text "but1a" -fill white
    ##.winBoard.c create text ${but1fX}c ${but1fY}c -text "but1f" -fill white

    set but1cX [expr $but1bX + 1.2]
    set but1cY $but1aY
    set but1dX [expr $but1cX - 0.35]
    set but1dY $but1fY
    CreateButton2 .winBoard.c $but1aX $but1aY $but1bX $but1bY $but1cX $but1cY $but1dX $but1dY $but1eX $but1eY $but1fX $but1fY normal 3 0 2

    ##4th button
    set but1bX [expr [expr $gSlotSize /10] / 5.175]
    set but1bY [expr [expr $gSlotSize * 1.275] / 10]
    set but1eY [expr $but1bY + 0.8]
    set but1eX $but1bX
    
    set but1aX [expr $but1bX - 1.2]
    set but1aY [expr $but1bY + 1.4]
    set but1fX [expr $but1aX + 0.35]
    set but1fY [expr $but1aY + 0.5]
    
    set but1cX [expr $but1bX + 1.2]
    set but1cY $but1aY
    set but1dX [expr $but1cX - 0.35]
    set but1dY $but1fY

    CreateButton2 .winBoard.c $but1aX $but1aY $but1bX $but1bY $but1cX $but1cY $but1dX $but1dY $but1eX $but1eY $but1fX $but1fY special 4 1 0

    ## 5th button
    set but1bX [expr [expr $gSlotSize /10] / 1.9]
    set but1bY [expr [expr $gSlotSize * 1.275] / 10]
    set but1eY [expr $but1bY + 0.8]
    set but1eX $but1bX
    
    set but1aX [expr $but1bX - 1.2]
    set but1aY [expr $but1bY + 1.4]
    set but1fX [expr $but1aX + 0.35]
    set but1fY [expr $but1aY + 0.5]
    
    set but1cX [expr $but1bX + 1.2]
    set but1cY $but1aY
    set but1dX [expr $but1cX - 0.35]
    set but1dY $but1fY

    CreateButton2 .winBoard.c $but1aX $but1aY $but1bX $but1bY $but1cX $but1cY $but1dX $but1dY $but1eX $but1eY $but1fX $but1fY special 5 1 1

    ## 6th button
    set but1bX [expr [expr $gSlotSize /10] / 1.175]
    set but1bY [expr [expr $gSlotSize * 1.275] / 10]
    set but1eY [expr $but1bY + 0.8]
    set but1eX $but1bX
    
    set but1aX [expr $but1bX - 1.2]
    set but1aY [expr $but1bY + 1.4]
    set but1fX [expr $but1aX + 0.35]
    set but1fY [expr $but1aY + 0.5]
    
    set but1cX [expr $but1bX + 1.2]
    set but1cY $but1aY
    set but1dX [expr $but1cX - 0.35]
    set but1dY $but1fY

    CreateButton2 .winBoard.c $but1aX $but1aY $but1bX $but1bY $but1cX $but1cY $but1dX $but1dY $but1eX $but1eY $but1fX $but1fY special 6 2 1
    

    ##CreateButton .winBoard.c  2.5 3.25 4 red normal 1 0 0
    ##CreateButton .winBoard.c  5.3 6.05 6.80 red normal 2 0 1
    ##CreateButton .winBoard.c  8.167 8.917 9.667 red normal 3 0 2
    
    ##CreateButton .winBoard.c  2.5 3.25 4 turquoise special 4 1 0 
    ##CreateButton .winBoard.c  5.3 6.05 6.80 turquoise special 5 1 1 
    ##CreateButton .winBoard.c  8.167 8.917 9.667 turquoise special 6 1 2
    
    ##puts "b=($but1bX,$but1bY) e=($but1eX,$but1eY) a=($but1aX,$but1aY) d=($but1dX,$but1dY) f=($but1fX,$but1fY) c=($but1cX,$but1cY)"
    
    ### Enable the 9 pieces and slots to be displayed.
    ## slots are placed at 
    ## (1.1, 0.6) (2.45, 0.6) (3.8, 0.6) 
    ## (1.1, 1.9) (2.45, 1.9) (3.8, 1.9)
    ## (1.1, 3.2) (2.45, 3.2) (3.8, 3.2)
   # for {set i 1.1} {$i < 5} {set i [expr $i + 1.35]} { 
	#for {set j 0.6} {$j < 4.5} {set j [expr $j+1.3]} { 
	 #   if { $i < 2.4} { 
		#set column "column1"
#	    } elseif { $i < 3.7 } { 
	#	set column "column2"
	 #   } else { 
		#set column "column3"
	    #}
	    #CreateSlot .winBoard.c $i $j  $column
	#} 
    #}

    ## draw temporary score = 5
    CreateMarker .winBoard.c 6.25 
    ### And all the slots should be active upon "New Game"
    #.winBoard.c addtag tagInitial withtag $marker

    GS_PostProcessBoard .winBoard.c

    pack append .winBoard .winBoard.c top

    ### Update the size of the window so that I can place things around it.

    update

    ### If the values of the radiobutton is set to anything other than the
    ### default, we need to call the routines to set the moves up.

    HandleMoves
}


#############################################################################
##
## ShrinkDeleteTag
##
## Rather than just delete a tag, why not do a cute animation?
##
#############################################################################

proc ShrinkDeleteTag { w theTag } {
    global gAnimationSpeed gSlotSize

    set theScale [expr 1.0 - (2.0 * $gAnimationSpeed.0 / $gSlotSize.0)]

    set theId $theTag
    foreach theId [$w find withtag $theTag] {
	set theCoords [$w coords $theId]
	set originX [expr int([lindex $theCoords 0] + [lindex $theCoords 2]) >> 1]
	set originY [expr int([lindex $theCoords 1] + [lindex $theCoords 3]) >> 1]

	### Scale is relative. You can keep scaling by .9 and it'll go away.

	set theBbox [$w bbox $theId]
	set theWidth [expr [lindex $theBbox 2] - [lindex $theBbox 0]]

	while { $theWidth > 6 } {
	    $w scale $theId $originX $originY $theScale $theScale
	    set theBbox [$w bbox $theId]
	    set theWidth [expr [lindex $theBbox 2] - [lindex $theBbox 0]]
	    update idletasks
	}
	$w delete $theTag
    }
}


#############################################################################
##
## DeleteMoves
##
## This is a small procedure to delete all moves on a board.
## Note that if there are no moves on the board, that's ok.
##
#############################################################################

proc DeleteMoves {} {
    .winBoard.c delete tagMoves
}

#############################################################################
##
## ShowMoves
##
## This is a small procedure to show all available moves on the board.
## For every move returned by C_GetValueMoves, draw it.
##
#############################################################################

proc ShowMoves {} {
    global gPosition kBigPiece gSlotsX gSlotsY

    DeleteMoves

    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
        DrawMove .winBoard.c $theMove cyan $kBigPiece
    } 
}

#############################################################################
##
## ShowValueMoves
##
## This is a small procedure to create all moves but color-code them based
## on their color-value. Also bring up an explanation of the colors.
##
#############################################################################

proc ShowValueMoves {} {
    global gPosition kBigPiece gSlotsX gSlotsY

    DeleteMoves

    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
        DrawMove .winBoard.c $theMove [ValueToColor [lindex $theMoveValue 1]] $kBigPiece
    }

    if { ![winfo exists .winValueMoves] } {
	DoValueMovesExplanation
    }
}

#############################################################################
##
## HandleMoves
##
## This is a generic handler for moves. It does the right function depending
## on the value of the radiobutton varMoves.
##
#############################################################################

proc HandleMoves {} {
    global varMoves gPosition kBigPiece kMovesOnAllTheTime

    ### If the game's just over, disable the move options and delete those on
    ### the screen.

    if { [C_Primitive $gPosition] != "Undecided" } {
	if { $kMovesOnAllTheTime } {
	    .winBoardControl.f1.noMoves config -state disabled
	}
	.winBoardControl.f1.validMoves config -state disabled
	.winBoardControl.f1.valueMoves config -state disabled
	DeleteMoves

    } else {

	### Otherwise, we're still playing. Enable the buttons if they're
	### not already (is this costly?)

	if { $kMovesOnAllTheTime } {
	    .winBoardControl.f1.noMoves config -state normal
	}
	.winBoardControl.f1.validMoves config -state normal
	.winBoardControl.f1.valueMoves config -state normal

	if { $varMoves == "validMoves" } {
	    ShowMoves
	} elseif { $varMoves == "valueMoves" } {
	    ShowValueMoves
	} elseif { $varMoves == "noMoves" } {
	    DeleteMoves
	} else {
	    BadElse HandleMoves
	}
    }
}

#############################################################################
##
## ValueToColor
##
## This returns a color depending on the value of the input. The color scheme
## was meant to simulate a traffic signal. 
##
## Green  = go      = win
## Yellow = slow    = tie
## Red    = stop    = lose
## White  = unknown = undecided
##
#############################################################################

proc ValueToColor { value } {
    if       { $value == "Win"  || $value == "0" } {
        return red4
    } elseif { $value == "Tie"  || $value == "1" } {
	return yellow
    } elseif { $value == "Lose" || $value == "2" } {
	return green
    } else {
	return white
    }
}

#############################################################################
##
## CreateSlot
##
## Here we create a new slot on the board defined by window w. The value
## of the slot in (slotX,slotY) tells us where on the board to be.
##
#############################################################################
proc CreateSlot {w slotX slotY mytag} {
    global gSlotSize gSlotList
    
    set slot [$w create rect \
        [expr $gSlotSize*$slotX] \
        [expr $gSlotSize*$slotY] \
        [expr $gSlotSize*[expr $slotX+1]] \
        [expr $gSlotSize*[expr $slotY+1]] \
        -outline #f0f \
        -width 1 \
        -fill #f0f \
        -tag $mytag]
    

    ### Remember what the id of the slot was in our array

    #set gSlotList($sl) $slot

    ### Add any slot embellishments are necessary (numbers, highlights, etc.)

    GS_EmbellishSlot $w $slotX $slotY $slot 
}
#############################################################################
##
## SlotEnter
##
## This is what we do when we enter a live slot. (We fill it with black)
##
#############################################################################

proc SlotEnter { w } {
    $w itemconfig current -fill black
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
    } else {
	BadElse "DrawMove (kInteractionType)"
    }
}

#############################################################################
##
## DrawMoveSinglePieceRemoval
##
## Here we show a user's move, graphically. The color might be generic or
## it might represent a value. If drawBig is true, we draw it the full size
## of the slot. Otherwise we draw it 1/5 of the size of the slot.
##
#############################################################################

proc DrawMoveSinglePieceRemoval { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]

    set theMove [$w create oval 0 0 $circleSize $circleSize \
	    -outline $color \
	    -fill $color]
    
    if { $drawBig } {
	
	set theMoveArg [GS_ConvertToAbsoluteMove $theMoveArg]

	### Figure out how far out to put the slot into the window.
	
	set slotX   [expr $theMoveArg % $gSlotsX]
	set slotY   [expr $theMoveArg / $gSlotsX]
	set cornerX [expr $slotX   * $gSlotSize + $circleOffset]
	set cornerY [expr $slotY   * $gSlotSize + $circleOffset]

	$w move $theMove $cornerX $cornerY
	$w addtag tagMoves withtag $theMove
	$w addtag tagMoveAlive withtag $theMove
	$w bind tagMoveAlive <1> { HandleMove [SinglePieceRemovalCoupleMove [expr %x/$gSlotSize] [expr %y/$gSlotSize]] }
	$w bind tagMoveAlive <Enter> { .winBoard.c itemconfig $gSlotList([expr %x/$gSlotSize],[expr %y/$gSlotSize]) -fill black }
    }
}

#############################################################################
##
## DrawMoveMultiplePieceRemoval
##
## Here we show a user's move, graphically. The color might be generic or
## it might represent a value. If drawBig is true, we draw it the full size
## of the slot. Otherwise we draw it 1/5 of the size of the slot.
##
#############################################################################

proc DrawMoveMultiplePieceRemoval { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]

    ### Ok, if it's going to go in the "Values" window, let it be a circle
    
    if { !$drawBig } {
	set theMove [$w create oval 0 0 $circleSize $circleSize \
		-outline $color \
		-fill $color]
    } else {

	set theMoveArg [GS_ConvertToAbsoluteMove $theMoveArg]

	if { [llength $theMoveArg] == 1 } {
	    
	    ### If the move is a single piece, draw a circle
	    
	    set theMoveArg [lindex $theMoveArg 0]
	    
	    set theMove [$w create oval 0 0 $circleSize $circleSize \
		    -outline $color \
		    -fill $color]
	    
	    ### Put it in the usual place away from the edge.
	    
	    set slotX   [expr $theMoveArg % $gSlotsX]
	    set slotY   [expr $theMoveArg / $gSlotsX]
	    set cornerX [expr $slotX   * $gSlotSize + $circleOffset]
	    set cornerY [expr $slotY   * $gSlotSize + $circleOffset]
	    
	} else {
	    
	    ### If the move is a multiple move, draw a line.
	    
	    ### The gap from the line to the edge
	    
	    set theLineGap    [expr int($gSlotSize / 15)]
	    
	    ### The number of slots it occupies (= length)
	    
	    set theLineSlots  [llength $theMoveArg]
	    
	    ### The actual length of the line is the width of two slots
	    ### minus the two gaps at the end.
	    
	    set theLineLength [expr ($gSlotSize * $theLineSlots) - (2 * $theLineGap)]
	    
	    set theMoveBegin [lindex $theMoveArg 0]
	    set theMoveEnd   [lindex $theMoveArg [expr [llength $theMoveArg] - 1]]
	    
	    set isitHorizontal [expr ($theMoveEnd - $theMoveBegin) < $gSlotsX]
	    
	    set slotXbegin [expr $theMoveBegin % $gSlotsX]
	    set slotYbegin [expr $theMoveBegin / $gSlotsX]
	    
	    if { $theLineSlots == 2 } {
		set theLineOffset [expr (($isitHorizontal ? $slotXbegin : $slotYbegin) % 2) ? 11 : 3]
	    } elseif { $theLineSlots == 3 } {
		set theLineOffset [expr (($isitHorizontal ? $slotXbegin : $slotYbegin) % 2) ? 9 : 5]
	    } elseif { $theLineSlots == 4 } {
		set theLineOffset 7
	    } else {
		BadElse "theLineSlots in DrawMove"
	    }
	    
	    if { $isitHorizontal } {
		set theLineX $theLineLength
		set theLineY 0
		set theLineOffsetX $theLineGap
		set theLineOffsetY [expr $theLineGap * $theLineOffset + $theLineGap/2]
	    } else {
		set theLineX 0
		set theLineY $theLineLength
		set theLineOffsetX [expr $theLineGap * $theLineOffset + $theLineGap/2]
		set theLineOffsetY $theLineGap
	    }
	    
	    ### Create the line
	    
	    set theMove [$w create line 0 0 $theLineX $theLineY \
		    -width $theLineGap \
		    -capstyle round \
		    -fill $color]
	    
	    set cornerX [expr $slotXbegin   * $gSlotSize + $theLineOffsetX]
	    set cornerY [expr $slotYbegin   * $gSlotSize + $theLineOffsetY]
	}

	$w move $theMove $cornerX $cornerY

	$w addtag tagMoves withtag $theMove
	$w bind $theMove <1> "HandleMove \{$theMoveArg\}"
	$w bind $theMove <Enter> "HandleEnterMultiplePieceRemoval $w \{$theMoveArg\}"
	$w bind $theMove <Leave> "HandleLeaveMultiplePieceRemoval $w \{$theMoveArg\}"
    }
}

#############################################################################
##
## DrawMoveRearranger
##
## Here we show a user's move, graphically. The color might be generic or
## it might represent a value. If drawBig is true, we draw it the full size
## of the slot. Otherwise we draw it 1/5 of the size of the slot.
##
#############################################################################

proc DrawMoveRearranger { w theMoveArg color drawBig } {
    global gSlotSize gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]

    ### Ok, if it's going to go in the "Values" window, let it be a circle
    
    if { !$drawBig } {
	set theMove [$w create oval 0 0 $circleSize $circleSize \
		-outline $color \
		-fill $color]
    } else {

	set theMoveArgAbsolute [GS_ConvertToAbsoluteMove $theMoveArg]

	set theMove [CreateArrow $w $theMoveArgAbsolute $gSlotSize $gSlotsX $gSlotsY $color]

	$w addtag tagMoves withtag $theMove
	$w bind $theMove <1> "HandleMove \{ $theMoveArgAbsolute \}"
	$w bind $theMove <Enter> "$w itemconfig $theMove -fill black"
	$w bind $theMove <Leave> "$w itemconfig $theMove -fill $color"
    }
}

#############################################################################
##
## CreateArrow
##
## Create and return arrow at the specified slots.
##
#############################################################################

proc CreateArrow { w theMoveArgAbsolute theSlotSize theSlotsX theSlotsY color } {

    global gSlotsX

    ### The gap from the line to the edge
    
    set theLineGap    [expr int($theSlotSize / 8)]
    
    ### The actual length of the line is the width of two slots
    ### minus the two gaps at the end.
    
    set theLineLength [expr $theSlotSize - (2 * $theLineGap)]
    
    set theMoveBegin [lindex $theMoveArgAbsolute 0]
    set theMoveEnd   [lindex $theMoveArgAbsolute 1]
    
    set isitHorizontal [expr abs($theMoveEnd - $theMoveBegin) == 1]
    set isitRightDown  [expr $theMoveEnd > $theMoveBegin]
    set isitDiagonal   [not [expr abs($theMoveEnd - $theMoveBegin) == $gSlotsX]]
    
    set theOffsetCenter [expr $isitRightDown ? 1 : -1]
    set theHalfSlot     [expr $theSlotSize >> 1]
    
    set slotXbegin [expr $theMoveBegin % $theSlotsX]
    set slotYbegin [expr $theMoveBegin / $theSlotsX]
    
    set theLineOffset 3
    
    if { $isitHorizontal } {
	set theLineX [expr $theLineLength * $theOffsetCenter]
	set theLineY 0
	set theLineOffsetX [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
	set theLineOffsetY $theHalfSlot
    } elseif { [expr 0 && $isitDiagonal] } {
	set theLineX [expr ($theLineLength + $theLineGap) * $theOffsetCenter]
	set theLineY $theLineX
	set theLineOffsetX [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
	set theLineOffsetY [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
    } else {
	set theLineX 0
	set theLineY [expr $theLineLength * $theOffsetCenter]
	set theLineOffsetX $theHalfSlot
	set theLineOffsetY [expr $theHalfSlot + ($theLineGap * $theOffsetCenter)]
    }   
    
    ### Create the line
    
    set arrow1 [expr $theLineGap * 2]
    set arrow2 [expr $theLineGap * 2]
    set arrow3 [expr $theLineGap * 1]
    set theMove [$w create line 0 0 $theLineX $theLineY \
	    -width $theLineGap \
	    -arrow last \
	    -arrowshape [list $arrow1 $arrow2 $arrow3] \
	    -fill $color]
    
    set cornerX [expr $slotXbegin   * $theSlotSize + $theLineOffsetX]
    set cornerY [expr $slotYbegin   * $theSlotSize + $theLineOffsetY]
    
    $w move $theMove $cornerX $cornerY
    
    return $theMove
}

#############################################################################
##
## HandleEnterMultiplePieceRemoval
##
## Ok, the user has just entered a move. Now what to do?
##
#############################################################################

proc HandleEnterMultiplePieceRemoval { w theMove } {
    global gSlotsX

    ### Here we color all pieces to-be-chosen as black.

    foreach i $theMove {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	$w itemconfig tagPieceCoord$slotX$slotY -fill black
    }
}

#############################################################################
##
## HandleLeaveMultiplePieceRemoval
##
## Ok, the user has just left a move. Now what to do?
##
#############################################################################

proc HandleLeaveMultiplePieceRemoval { w theMove } {
    global gSlotsX

    ### Here we color all pieces to-be-chosen as white.

    foreach i $theMove {
	set slotX [expr $i % $gSlotsX]
	set slotY [expr $i / $gSlotsX]
	$w itemconfig tagPieceCoord$slotX$slotY -fill magenta
    }
}

#############################################################################
##
## DrawHollowCircle
##
## Here we draw a hollow circle on (slotX,slotY) in window w with a tag of 
## theTag. If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawHollowCircle { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set circleWidth [expr $theSlotSize/10.0]
    set startCircle [expr $theSlotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$theSlotSize]
    set cornerY     [expr $slotY*$theSlotSize]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline $theColor \
		       -width $circleWidth \
		       -tag $theTag]

    if { $drawBig } {
	$w move $theCircle $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle

    return $theCircle
}

#############################################################################
##
## DrawCircle
##
## Here we draw a circle on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

### RUBIKS- Modified to take in a border color which is white

proc DrawCircle { w slotX slotY theTag theColor drawBig border tagID} {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set circleWidth [expr $theSlotSize/45.0]
    ##set startCircle [expr $theSlotSize/8.0]
    ##set endCircle   [expr $startCircle*7.0]
    set startCircleX $slotX
    set startCircleY $slotY
    set endCircleX [expr $slotX + $circleWidth]
    set endCircleY [expr $slotY + $circleWidth]
    set cornerX     [expr $slotX*$theSlotSize]
    set cornerY     [expr $slotY*$theSlotSize]

    ##puts "$startCircleX $startCircleY $endCircleX $endCircleY $circleWidth $theSlotSize"
    ##$w create text  ${startCircleX}c ${startCircleY}c -text "($startCircleX, $startCircleY)" -fill white
    ##$w create text  ${endCircleX}c ${endCircleY}c -text "($endCircleX, $endCircleY)" -fill white

    if { $border == "special" } {
	set theCircle [$w create oval ${startCircleX}c ${startCircleY}c \
		${endCircleX}c ${endCircleY}c \
		       -outline white \
		       -width 0.15c \
		       -fill $theColor \
		       -tag $tagID]
		       ###-tag $theTag]
    } else { 
	set theCircle [$w create oval ${startCircleX}c ${startCircleY}c \
		${endCircleX}c ${endCircleY}c \
		       -outline black \
		       -fill $theColor \
		       -tag $tagID]
	###-tag $theTag]
    }

    ## I don't understand what Dan's doing here... actually, WHY. :(
    if { $drawBig } {
	##$w move $theCircle $cornerX $cornerY
	##$w addtag tagPieceCoord$slotX$slotY withtag $theCircle
	##$w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle
    }

    return $theCircle
}

#############################################################################
##
## DrawLastMove
##
## Here we draw a faint sketch of the last person to move.
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawLastMove { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set circleWidth [expr $theSlotSize/10.0]
    set startCircle [expr $theSlotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$theSlotSize]
    set cornerY     [expr $slotY*$theSlotSize]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline $theColor \
		       -tag $theTag]

    if { $drawBig } {
	$w move $theCircle $cornerX $cornerY
	$w addtag tagLastMove withtag $theCircle
    }

    return $theCircle
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

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

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

#############################################################################
##
## DisableSlot
##
## Disable the selected slot in the selected window by making it grey,
## adding a 'tagDead' tag and removing the 'tagAlive' tag
##
#############################################################################

proc DisableSlot { w theSlot } {

    ### Make it grey

    $w itemconfig $theSlot -fill grey

    ### Add a 'Dead' tag

    $w addtag tagDead withtag $theSlot

    ### Remove its 'Alive' tag

    $w dtag $theSlot tagAlive

    ### Disable the embellishments

    GS_DisableSlotEmbellishments $w $theSlot

    ## Render these slots useless.
    $w bind tagDead  <1>     { }
    $w bind tagDead  <Enter> { }
    $w bind tagDead  <Leave> { }

}

#############################################################################
##
## EnableSlot
##
## Here we need to Enable the slot because it's now able to be clicked upon
##
#############################################################################

proc EnableSlot { w theSlot } {
    global gSlotSize kInteractionType

    if { $kInteractionType == "SinglePieceRemoval" } {

	$w dtag $theSlot tagDead
	$w addtag tagAlive withtag $theSlot
	
	$w bind tagAlive <1> { HandleMove [SinglePieceRemovalCoupleMove [expr %x/$gSlotSize] [expr %y/$gSlotSize]] }
	$w bind tagAlive <Enter> "SlotEnter $w"
	#$w bind tagAlive <Leave> "$w itemconf current -fill orange "
	$w bind tagDead  <1>     { }
	$w bind tagDead  <Enter> { }
	$w bind tagDead  <Leave> { }

    }

    GS_EnableSlotEmbellishments $w $theSlot
}

#############################################################################
##
## SinglePieceRemovalCoupleMove
##
## With SinglePieceRemoval, sometimes the move is in (x,y) and we have to 
## convert it to a single move again.
##
#############################################################################

proc SinglePieceRemovalCoupleMove { slotX slotY } {
    global gSlotsX

    ### Row major format. The number of rows times the pieces per row + the # of columns
    return [expr ($gSlotsX * $slotY) + $slotX]
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
	
	#set theSlot $gSlotList($theMoveX,$theMoveY)
	
	### Draw a piece there.

	#DrawPiece $theMoveX $theMoveY $thePiece

    } elseif { $kInteractionType == "MultiplePieceRemoval" } {
	set theSlot dummyArgNeverSeen
    } elseif { $kInteractionType == "Rearranger" } {
	set theSlot dummyArgNeverSeen
    } else {
	BadElse "DoComputersMove (kInteractionType)"
    }

    ### Enable or Disable slots as a result of the move
	
    #GS_HandleEnablesDisables $w $theSlot $theMove

    ### Do the move and check out the position.

    set oldscore [expr [expr ($gPosition / (2*15*15*15*10*10))] % 11]

    set gPosition [C_DoMove $gPosition $theMove]
    set theValue [C_GetValueOfPosition $gPosition]

    ## RUBIKS, Added Apr 27, 2002
    ## unhash code
    set turn [ expr $gPosition  % 2]
    set first [expr [expr ($gPosition / (2*15*15))] % 15]
    set second [expr [expr ($gPosition / (2*15))] % 15]
    set third [expr [expr ($gPosition / (2))] % 15]
    set white [expr [expr ($gPosition / (2*15*15*15))] % 10]
    set black [expr [expr ($gPosition / (2*15*15*15*10))] % 10]
    set score [expr [expr ($gPosition / (2*15*15*15*10*10))] % 11]

    ## clear the board
    ##$w delete tagPiece
    
    ## Attempt to slow this down
    for {set i 0} {$i < [expr $gAnimationSpeed * 10]} {incr i} {
	update idletasks
    }

    if { $theMove == 1 || $theMove == 4} {
	DrawPiecesOnBoard $w $theMove $first 1 4 7 1 $white $black
    } elseif {$theMove == 2 || $theMove == 5} {
	DrawPiecesOnBoard $w $theMove $second 2 5 8 1 $white $black
    } else {
	DrawPiecesOnBoard $w $theMove $third 3 6 9 1 $white $black
    }
    
    puts "Computer: turn: $turn first:$first second:$second third:$third white:$white black:$black score:$score"

    UpdateScoreMarker $w $oldscore $score $turn

    ### Alternate turns

    #set gPlayerOneTurn [not $gPlayerOneTurn]

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

    } else {
	BadElse "HandleMove (kInteractionType)"
    }

    ### Enable or Disable slots as a result of the move
    
    GS_HandleEnablesDisables $w $theSlot [GS_ConvertInteractionToMove $theMove]

    ### Set the piece depending on whose turn it is and draw it.

    if { $kInteractionType == "SinglePieceRemoval" } {

	if { $gPlayerOneTurn } { set thePiece "X" } { set thePiece "O" }

	##DrawPiece $slotX $slotY $thePiece

    } elseif { $kInteractionType == "MultiplePieceRemoval" } {

	### Don't need to do anything here.

    } elseif { $kInteractionType == "Rearranger" } {

	### Don't need to do anything here.

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
## HandleGameOver
##
## The game was just over and we have to clean up.
##
#############################################################################

proc HandleGameOver { w theValue } {

    global gPlayerOneTurn

    ### Update the status box.
    
    .f0.mesStatus config \
	    -text "[.f3.entPlayer[expr !$gPlayerOneTurn + 1] get] [PastTenseValue $theValue] [.f3.entPlayer[expr $gPlayerOneTurn + 1] get]\nClick 'New Game' for more!"
    
    ### Make all the other slots inactive
    
    $w addtag tagDead withtag tagAlive
    $w dtag tagAlive
}

#############################################################################
##
## DrawPiece
##
## Draw a piece at the slot specified by slotX and slotY
##
#############################################################################

## RUBIKS, added an extra argument to the call of $kBothDrawProc (which is usually DrawCircle).
## Also added tagID for animation drawing

proc DrawPiece { slotX slotY thePiece tagID} {
    global kBigPiece kLeftDrawProc kRightDrawProc kBothDrawProc

    if     { $thePiece == "X" } {
	return [$kLeftDrawProc  .winBoard.c $slotX $slotY tagPiece blue $kBigPiece ""]
    } elseif { $thePiece == "O" } { 
	return [$kRightDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece ""]
    } elseif { $thePiece == "+" } { 
	return [$kBothDrawProc .winBoard.c $slotX $slotY tagPiece magenta $kBigPiece ""]
    } elseif { $thePiece == "-" } { 
	return [DrawLastMove .winBoard.c $slotX $slotY tagPiece magenta $kBigPiece ""]
    } elseif { $thePiece == "blue" } { 
	return [$kRightDrawProc .winBoard.c $slotX $slotY tagPiece blue $kBigPiece "" $tagID]
    } elseif { $thePiece == "red" } { 
	return [$kLeftDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece "" $tagID]
    }  elseif { $thePiece == "black" } { 
	return [$kRightDrawProc .winBoard.c $slotX $slotY tagPiece blue $kBigPiece "special" $tagID]
    } elseif { $thePiece == "white" } { 
	return [$kLeftDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece "special" $tagID]
    } else {
	BadElse DrawPiece
    }
}

#############################################################################
##
## not
##
## easier than [expr !]
##
#############################################################################

proc not { x } {
    if { $x } { return 0 } { return 1 }
}

#############################################################################
##
## SetPredictionField
##
## Set the prediction field to the argument
##
#############################################################################

proc SetPredictionField { s } {
    .winBoardControl.f4.predictions config -text $s
}

#############################################################################
##
## HandleTurn
##
## Someone has just made a move and now we have to update the "Turn:" field.
## This is further complicated by the fact that if the game is over we want
## to change "Turn:" to "Winner:" BUT if the game is a tie, we have to not
## put the winner's name there. Sigh.
##
#############################################################################

proc HandleTurn {} {
    global gPlayerOneTurn kSmallPiece gPosition
    global xbmRight xbmLeft xbmBlank

    ### Figure out whether this is a primitive value and whether it's game over

    set thePrimitive [C_Primitive $gPosition]
    set theGameOver [expr { $thePrimitive != "Undecided" }]

    ### If the game's over, we want the previous player. Otherwise this player

    set thePlayersTurn [expr $theGameOver ? !$gPlayerOneTurn : $gPlayerOneTurn]
    set thePlayer      [.f3.entPlayer[expr !$thePlayersTurn + 1] get]
    
    ### If the game's over...

    if { $theGameOver } {

	### Disable the fancy cursor, change the text to "Winner"

	.winBoard.c config -cursor {}
	.winBoardControl.f5.labTurn config -text "Winner:"

	### And update the winner field with "nobody" or the name

	if { $thePrimitive == "Tie" } {
	    .winBoardControl.f5.mesTurn config \
		    -text "Nobody - The game was a tie"
	    .winBoardControl.f5.c config -bitmap @$xbmBlank -foreground black
	} else {

	    ### The left player won if it's his turn and he's win or
	    ### if it's not his turn and the other guy lost.

	    if { ($thePrimitive == "Win" && $gPlayerOneTurn) ||
	    ($thePrimitive == "Lose" && !$gPlayerOneTurn) } {
		
		### The left player just won.
		
		.winBoardControl.f5.c config -bitmap @$xbmLeft -foreground blue
		.winBoardControl.f5.mesTurn config \
			-text "(Left) [.f3.entPlayer1 get]"

	    } else {
		
		### The right player just won.

		.winBoardControl.f5.c config -bitmap @$xbmRight -foreground red
		.winBoardControl.f5.mesTurn config \
			-text "(Right) [.f3.entPlayer2 get]"
	    }
	}
	
    #### So at this point the game is NOT over

    } else {

	### And for that player, we set the "Left/Right" string
	
	if { $thePlayersTurn } {
	    
	    set theLeftRight   "Left"

	    ### And if it's not a tie, we set the bitmap, too

	    if { $thePrimitive != "Tie" } {
		.winBoardControl.f5.c config -bitmap @$xbmLeft -foreground blue
	    } 	    
	    
	} else {
	    
	    set theLeftRight   "Right"
	    
	    ### And if it's not a tie, we set the bitmap, too
	    
	    if { $thePrimitive != "Tie" } {
		.winBoardControl.f5.c config -bitmap @$xbmRight -foreground red
	    } 	    
	}
    
	### So we can set the cursor to the other player (don't do on mac)

	global tcl_platform macLeft macRight
	if { $tcl_platform(platform) == "macintosh" || \
             $tcl_platform(platform) == "windows" } {
	    if { $thePlayersTurn } {
		.winBoard.c config -cursor $macLeft
	    } else {
		.winBoard.c config -cursor $macRight
	    }
	} else {
	    if { $thePlayersTurn } {
		.winBoard.c config -cursor "@$xbmLeft blue"
	    } else {
		.winBoard.c config -cursor "@$xbmRight red"
	    }
	}

	### And change the turn field to the appropriate person.

	.winBoardControl.f5.labTurn config -text "Turn:"

	.winBoardControl.f5.mesTurn config \
		-text "($theLeftRight) $thePlayer's turn"
    }
}

#############################################################################
##
## HandlePredictions
##
## Well, if the game's over, disable everything, otherwise print the prediction
##
#############################################################################

proc HandlePredictions {} {

    global varPredictions gPosition gPlayerOneTurn gAgainstComputer
    
    ### If the game's just over, disable the predictions

    if { [C_Primitive $gPosition] != "Undecided" } {

	.winBoardControl.f4.butOn  config -state disabled
	.winBoardControl.f4.butOff config -state disabled
	SetPredictionField ""

    } else {
	
	### Otherwise, we're still playing. Enable the buttons if they're
	### not already (is this costly?)

	.winBoardControl.f4.butOn  config -state normal
	.winBoardControl.f4.butOff config -state normal

	### Get the value, the player and set the prediction

	set theValue      [C_GetValueOfPosition $gPosition]
	set theRemoteness [C_Remoteness $gPosition]
	## May be empty for Partizan
	set theMex        [C_Mex $gPosition] 
	set thePlayer [.f3.entPlayer[expr !$gPlayerOneTurn + 1] get]

	### It's only definite if we're playing against the computer and we're
	### going to lose (huh?)

	if { $theValue == "Lose" && $gAgainstComputer } {
	    set thePrediction "will"
	} else {
	    set thePrediction "should"
	}

	### And spooge it to the field if the button is on.
	
	if { $varPredictions == "butOn" } {
	    SetPredictionField "$thePlayer $thePrediction $theValue in $theRemoteness $theMex"
	} else {
	    SetPredictionField "Disabled"
	}
    }
}

#############################################################################
##
## PastTenseValue
##
## Change the wording to the past tense (Lose->lost to, etc.)
##
#############################################################################

proc PastTenseValue { value } {
    if { $value == "Win" } {
        return "beat"
    } elseif { $value == "Lose" } {
	return "lost to"
    } elseif { $value == "Tie" } {
	return "tied"
    } else {
	BadElse PastTenseValue
	return "BadElseInPastTenseValue"
    }
}

#############################################################################
##
## BadElse
##
## This is for if-then-elseif-then-else statements to guarantee correctness.
##
#############################################################################

proc BadElse { theFunction } {
    puts "Error: $theFunction\{\} just reached an else clause it shouldn't have!"
}

## Initializeation script

#############################################################################
##
## main
##
## The main procedure. Basically Initialize everything.
##
#############################################################################

proc main {} {
    InitConstants
    GS_InitGameSpecific
    InitWindow
    # Initialize the C backend
    global command_line_args
    C_Initialize $command_line_args
    C_InitializeDatabases
}

### This makes it feel a little like C

main
