#############################################################################
##
## NAME:         mLgame.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Lgame
##
## AUTHOR:       Michael Savitzky, Alex Kozlowski
##
## DATE:         04-01-02
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
    
    global gPlayerOneTurn
    global gCurrentL1
    global gCurrentL2
    global gCurrentBlack
    global gCurrentWhite
    global xbmLeft
    set xbmLeft "../bitmaps/circle.xbm"
    global gValueMoves
    global gAgainstComputer
    set gAgainstComputer 0
    global gRestoreCmd
    set gRestoreCmd ""
    global gPosition
    global gInitL
    global gFinalL
    global gOkay
    global gPhase
    set gPhase 1

    ### Set the name of the game
    
    global kGameName
    set kGameName "Lgame"
    
    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules
    
    global kStandardString kMisereString
    set kStandardString "Forcing the other player to be stuck WINS"
    set kMisereString "Forcing the other player to be stuck LOSES"
    
    ### Set the strings to tell the user how to move and what the goal is.
    
    global kToMove kToWinStandard kToWinMisere
    set kToMove "Each player takes turns moving his or her L piece into a different position. At the end of each turn, players place one of the neutral circle pieces onto any open square. Click on the small L that represents the location you want to move your L piece. Then click on the square where you want to place the black or white neutral circle."
    set kToWinStandard  "Force your opponent into a position from which he cannot move."
    set kToWinMisere  "Get yourself in a position from which you cannot move."
    
    ### Set the size of the board
    
    global gSlotsX gSlotsY 
    set gSlotsX 4
    set gSlotsY 4
    
    ### Set the initial position of the board in our representation
    
    global gInitialPosition gPosition
    set gInitialPosition 19388
    set gPosition $gInitialPosition
    
    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.
    
    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/circle.xbm"
    set xbmRight "$kRootDir/../bitmaps/circle.xbm"
    set macLeft  cross
    set macRight cross

    ### Authors Names and Photo (by AP)
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Michael Savitzky, Alex Kozlowski"
    set kTclAuthors "Michael Savitzky, Alex Kozlowski"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
    
    ### Set the procedures that will draw the pieces
    
    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCircle
    set kRightDrawProc DrawCircle
    set kBothDrawProc  DrawCircle
    
    ### What type of interaction will it be, sir?
    
    global kInteractionType
    set kInteractionType MultiplePieceRemoval
    
    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1
    
    ### Do you support editing of the initial position, sir?
    
    global kEditInitialPosition
    set kEditInitialPosition 0
}

#############################################################################
##
## DoEditInitialPosition
##
## Create the initial board.
##
#############################################################################

proc DoEditInitialPosition {} {
    global kGameName gSlotsX gSlotsY gSlotSize
    set gSlotsX 4
    set gSlotsY 4
    set gSlotSize 100
    toplevel .initBoard
    wm title .initBoard "GAMESMAN $kGameName Board Initializer"
    canvas .initBoard.c -width [expr $gSlotsX*$gSlotSize] \
	    -height [expr $gSlotsY*$gSlotSize] -relief raised
    set c .initBoard.c
    GS_LoadBoard $c
    $c lower all back
    $c raise small back
}

#############################################################################
##
## makeL
##
## Create a small L object with orientation NUM at coordinate (X,Y) with
## tags t, t1, t2, t3, and t4.
##
#############################################################################

proc makeL {c num x y t t1 t2 t3 t4} {
    global gSlotSize
    set factor [expr $gSlotSize / 2]
    
    if {$num == 1} {
	$c create polygon [expr ($x + .25) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .25) * $factor] [expr ( $y + .125 ) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .375) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25 
    }
    if {$num == 2} {
	$c create polygon [expr ( $x + .75) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .5) * $factor] [expr ( $y + .125 ) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .375) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 3} {
	$c create polygon [expr (($x + .125) * $factor)] [expr ( $y + .25) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .25) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .625) * $factor] [expr ( $y + .25 ) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .5) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 4} {
	$c create polygon [expr ( $x + .125) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .625) * $factor] [expr ( $y + .5 ) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 5} {
	$c create polygon [expr ( $x + .75) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .5) * $factor] [expr ( $y + .625 ) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    if {$num == 6} {
	$c create polygon [expr ( $x + .25) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .25) * $factor] [expr ( $y + .875 ) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .625) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25  
    }
    
    if {$num == 7} {
	$c create polygon [expr ( $x + .125) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .25) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .125) * $factor] [expr ( $y + .75 ) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 8} {
	$c create polygon [expr ( $x + .125) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .75) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .125) * $factor] [expr ( $y + .25 ) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
}

#############################################################################
##
## makeNeutral
##
## Create the small neutral piece of color COLOR at the coordinate (X,Y).
## Add the concatenation of T and its color to its list of tags.
##
#############################################################################

proc makeNeutral {c color x y t} {
    global gSlotSize 
    
    if {$color == "Black"} {
	$c create oval [expr ($x + .075) * $gSlotSize] [expr ($y + .075) * $gSlotSize] \
		[expr ($x + .5) * $gSlotSize] [expr ($y + .5) * $gSlotSize] \
		-tags "$t neutral Black [myappend $t Black]" -fill purple -outline black -width 4
    }
    if {$color == "White"} {
	$c create oval [expr ($x + .5) * $gSlotSize] [expr ($y + .5) * $gSlotSize] \
		[expr ($x + .925) * $gSlotSize] [expr ($y + .925) * $gSlotSize] \
		-tags "$t neutral White [myappend $t White]" -fill purple -outline white -width 4
    }
}

#############################################################################
##
## makeBigO
##
## Create a large neutral piece of color COLOR at coordinate (X, Y) and 
## include T as one of its tags.
##
#############################################################################

proc makeBigO {c color x y t} {    
    global gSlotSize
    $c create oval [expr ($x + .073) * $gSlotSize] [expr ($y + .075) * $gSlotSize] \
	    [expr ($x + .925) * $gSlotSize] [expr ($y + .925) * $gSlotSize] \
	    -tags "$t $color bigO" -fill purple -outline $color -width 4
}

#############################################################################
##
## GS_LoadBoard
##
## Load up the canvas with the appropriate pieces.  
##
#############################################################################

proc GS_LoadBoard {c} {
    global gSlotSize

    ##Make the background
    $c create rectangle 0 0 [expr 4 * $gSlotSize] [expr 4 * $gSlotSize] -fill grey -tags back
    for {set j 0} {$j <= 5} {incr j} {
	set x [expr $j * $gSlotSize]
	$c create line $x 0 $x [expr 4 * $gSlotSize] -width 1
    }
    for {set j 0} {$j <= 5} {incr j} {
	set x [expr $j * $gSlotSize]
	$c create line 0 $x [expr 4 * $gSlotSize] $x -width 1
    }
    
    ##Make the small L-pieces
    makeL $c 1 0 1 num1 sq9 sq5 sq1 sq2
    makeL $c 1 3 1 num2 sq2 sq3 sq6 sq10
    makeL $c 1 5 1 num3 sq11 sq3 sq7 sq4
    makeL $c 1 0 3 num4 sq13 sq9 sq5 sq6
    makeL $c 1 3 3 num5 sq14 sq10 sq7 sq6
    makeL $c 1 5 3 num6 sq15 sq11 sq7 sq8
    
    makeL $c 2 2 1 num7 sq10 sq2 sq6 sq1
    makeL $c 2 4 1 num8 sq11 sq3 sq7 sq2
    makeL $c 2 7 1 num9 sq12 sq8 sq3 sq4
    makeL $c 2 2 3 num10 sq14 sq10 sq6 sq5
    makeL $c 2 4 3 num11 sq15 sq11 sq7 sq6
    makeL $c 2 7 3 num12 sq16 sq12 sq8 sq7
    
    makeL $c 3 6 0 num13 sq3 sq8 sq4 sq2
    makeL $c 3 6 3 num14 sq6 sq7 sq8 sq12
    makeL $c 3 6 5 num15 sq10 sq11 sq12 sq16
    makeL $c 3 4 0 num16 sq3 sq7 sq1 sq2
    makeL $c 3 5 2 num17 sq5 sq6 sq7 sq11
    makeL $c 3 4 5 num18 sq9 sq10 sq11 sq15
    
    makeL $c 4 6 2 num19 sq6 sq7 sq8 sq4
    makeL $c 4 6 4 num20 sq11 sq12 sq10 sq8
    makeL $c 4 6 7 num21 sq14 sq15 sq16 sq12
    makeL $c 4 4 2 num22 sq6 sq5 sq3 sq7
    makeL $c 4 5 5 num23 sq9 sq10 sq11 sq7
    makeL $c 4 4 7 num24 sq13 sq14 sq15 sq11
    
    makeL $c 5 7 6 num25 sq8 sq12 sq16 sq15
    makeL $c 5 4 6 num26 sq7 sq11 sq15 sq14
    makeL $c 5 2 6 num27 sq6 sq10 sq14 sq13
    makeL $c 5 7 4 num28 sq4 sq8 sq11 sq12
    makeL $c 5 4 4 num29 sq3 sq7 sq11 sq10
    makeL $c 5 2 4 num30 sq6 sq2 sq10 sq9
    
    makeL $c 6 5 6 num31 sq7 sq11 sq15 sq16
    makeL $c 6 3 6 num32 sq6 sq10 sq14 sq15
    makeL $c 6 0 6 num33 sq9 sq5 sq13 sq14
    makeL $c 6 5 4 num34 sq3 sq7 sq11 sq12
    makeL $c 6 3 4 num35 sq2 sq6 sq10 sq11
    makeL $c 6 0 4 num36 sq9 sq5 sq1 sq10
    
    makeL $c 7 1 7 num37 sq9 sq15 sq14 sq13
    makeL $c 7 1 4 num38 sq9 sq5 sq11 sq10
    makeL $c 7 1 2 num39 sq7 sq5 sq1 sq6
    makeL $c 7 3 7 num40 sq16 sq15 sq14 sq10
    makeL $c 7 2 5 num41 sq12 sq11 sq10 sq6
    makeL $c 7 3 2 num42 sq8 sq2 sq7 sq6
    
    makeL $c 8 1 5 num43 sq9 sq11 sq10 sq13
    makeL $c 8 1 3 num44 sq9 sq5 sq7 sq6
    makeL $c 8 1 0 num45 sq3 sq5 sq1 sq2
    makeL $c 8 3 5 num46 sq12 sq11 sq10 sq14
    makeL $c 8 2 2 num47 sq8 sq7 sq6 sq10
    makeL $c 8 3 0 num48 sq4 sq3 sq6 sq2

    ##Make the blue L-pieces
    image create photo blueB -file "../bitmaps/blue.gif"     
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 1] \
	    -image blueB -anchor center -tags "blueSquare1 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 1] \
	    -image blueB -anchor center -tags "blueSquare2 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 1] \
	    -image blueB -anchor center -tags "blueSquare3 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 1] \
	    -image blueB -anchor center -tags "blueSquare4 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 3] \
	    -image blueB -anchor center -tags "blueSquare5 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 3] \
	    -image blueB -anchor center -tags "blueSquare6 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 3] \
	    -image blueB -anchor center -tags "blueSquare7 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 3] \
	    -image blueB -anchor center -tags "blueSquare8 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 5] \
	    -image blueB -anchor center -tags "blueSquare9 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 5] \
	    -image blueB -anchor center -tags "blueSquare10 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 5] \
	    -image blueB -anchor center -tags "blueSquare11 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 5] \
	    -image blueB -anchor center -tags "blueSquare12 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 7] \
	    -image blueB -anchor center -tags "blueSquare13 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 7] \
	    -image blueB -anchor center -tags "blueSquare14 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 7] \
	    -image blueB -anchor center -tags "blueSquare15 blueSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 7] \
	    -image blueB -anchor center -tags "blueSquare16 blueSquare"
    
    ##Make the red L-pieces
    image create photo redB -file "../bitmaps/red.gif" 
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 1] \
	    -image redB -anchor center -tags "redSquare1 redSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 1] \
	    -image redB -anchor center -tags "redSquare2 redSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 1] \
	    -image redB -anchor center -tags "redSquare3 redSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 1] \
	    -image redB -anchor center -tags "redSquare4 redSquare"
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 3] \
	    -image redB -anchor center -tags "redSquare5 redSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 3] \
	    -image redB -anchor center -tags "redSquare6 redSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 3] \
	    -image redB -anchor center -tags "redSquare7 redSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 3] \
	    -image redB -anchor center -tags "redSquare8 redSquare"
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 5] \
	    -image redB -anchor center -tags "redSquare9 redSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 5] \
	    -image redB -anchor center -tags "redSquare10 redSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 5] \
	    -image redB -anchor center -tags "redSquare11 redSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 5] \
	    -image redB -anchor center -tags "redSquare12 redSquare"
    $c create image [expr ($gSlotSize / 2) * 1] [expr ($gSlotSize / 2) * 7] \
	    -image redB -anchor center -tags "redSquare13 redSquare"
    $c create image [expr ($gSlotSize / 2) * 3] [expr ($gSlotSize / 2) * 7] \
	    -image redB -anchor center -tags "redSquare14 redSquare"
    $c create image [expr ($gSlotSize / 2) * 5] [expr ($gSlotSize / 2) * 7] \
	    -image redB -anchor center -tags "redSquare15 redSquare"
    $c create image [expr ($gSlotSize / 2) * 7] [expr ($gSlotSize / 2) * 7] \
	    -image redB -anchor center -tags "redSquare16 redSquare"
    
    ###Make the colored squares that make up the shaded version of the big L-piece
    $c create rectangle [expr 0 * $gSlotSize] [expr 0 * $gSlotSize] [expr 1 * $gSlotSize] \
	    [expr 1 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade1" -outline blue -width 3
    $c create rectangle [expr 1 * $gSlotSize] [expr 0 * $gSlotSize] [expr 2 * $gSlotSize] \
	    [expr 1 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade2" -outline blue -width 3
    $c create rectangle [expr 2 * $gSlotSize] [expr 0 * $gSlotSize] [expr 3 * $gSlotSize] \
	    [expr 1 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade3" -outline blue -width 3
    $c create rectangle [expr 3 * $gSlotSize] [expr 0 * $gSlotSize] [expr 4 * $gSlotSize] \
	    [expr 1 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade4" -outline blue -width 3
    $c create rectangle [expr 0 * $gSlotSize] [expr 1 * $gSlotSize] [expr 1 * $gSlotSize] \
	    [expr 2 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade5" -outline blue -width 3
    $c create rectangle [expr 1 * $gSlotSize] [expr 1 * $gSlotSize] [expr 2 * $gSlotSize] \
	    [expr 2 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade6" -outline blue -width 3
    $c create rectangle [expr 2 * $gSlotSize] [expr 1 * $gSlotSize] [expr 3 * $gSlotSize] \
	    [expr 2 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade7" -outline blue -width 3
    $c create rectangle [expr 3 * $gSlotSize] [expr 1 * $gSlotSize] [expr 4 * $gSlotSize] \
	    [expr 2 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade8" -outline blue -width 3
    $c create rectangle [expr 0 * $gSlotSize] [expr 2 * $gSlotSize] [expr 1 * $gSlotSize] \
	    [expr 3 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade9" -outline blue -width 3
    $c create rectangle [expr 1 * $gSlotSize] [expr 2 * $gSlotSize] [expr 2 * $gSlotSize] \
	    [expr 3 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade10" -outline blue -width 3
    $c create rectangle [expr 2 * $gSlotSize] [expr 2 * $gSlotSize] [expr 3 * $gSlotSize] \
	    [expr 3 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade11" -outline blue -width 3
    $c create rectangle [expr 3 * $gSlotSize] [expr 2 * $gSlotSize] [expr 4 * $gSlotSize] \
	    [expr 3 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade12" -outline blue -width 3
    $c create rectangle [expr 0 * $gSlotSize] [expr 3 * $gSlotSize] [expr 1 * $gSlotSize] \
	    [expr 4 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade13" -outline blue -width 3
    $c create rectangle [expr 1 * $gSlotSize] [expr 3 * $gSlotSize] [expr 2 * $gSlotSize] \
	    [expr 4 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade14" -outline blue -width 3
    $c create rectangle [expr 2 * $gSlotSize] [expr 3 * $gSlotSize] [expr 3 * $gSlotSize] \
	    [expr 4 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade15" -outline blue -width 3
    $c create rectangle [expr 3 * $gSlotSize] [expr 3 * $gSlotSize] [expr 4 * $gSlotSize] \
	    [expr 4 * $gSlotSize] -stipple gray25 -fill blue -tags "shade shade16" -outline blue -width 3
        
    ##Make the small neutral pieces
    makeNeutral $c Black 0 0 neut1
    makeNeutral $c White 0 0 neut1
    makeNeutral $c Black 1 0 neut2
    makeNeutral $c White 1 0 neut2
    makeNeutral $c Black 2 0 neut3
    makeNeutral $c White 2 0 neut3
    makeNeutral $c Black 3 0 neut4
    makeNeutral $c White 3 0 neut4
    makeNeutral $c Black 0 1 neut5
    makeNeutral $c White 0 1 neut5
    makeNeutral $c Black 1 1 neut6
    makeNeutral $c White 1 1 neut6
    makeNeutral $c Black 2 1 neut7
    makeNeutral $c White 2 1 neut7
    makeNeutral $c Black 3 1 neut8
    makeNeutral $c White 3 1 neut8
    makeNeutral $c Black 0 2 neut9
    makeNeutral $c White 0 2 neut9
    makeNeutral $c Black 1 2 neut10
    makeNeutral $c White 1 2 neut10
    makeNeutral $c Black 2 2 neut11
    makeNeutral $c White 2 2 neut11
    makeNeutral $c Black 3 2 neut12
    makeNeutral $c White 3 2 neut12
    makeNeutral $c Black 0 3 neut13
    makeNeutral $c White 0 3 neut13
    makeNeutral $c Black 1 3 neut14
    makeNeutral $c White 1 3 neut14
    makeNeutral $c Black 2 3 neut15
    makeNeutral $c White 2 3 neut15
    makeNeutral $c Black 3 3 neut16
    makeNeutral $c White 3 3 neut16
    
    ##Make the big neutral ("big 0") pieces
    makeBigO $c Black 0 0 bigBlack1
    makeBigO $c White 0 0 bigWhite1
    makeBigO $c Black 1 0 bigBlack2
    makeBigO $c White 1 0 bigWhite2
    makeBigO $c Black 2 0 bigBlack3
    makeBigO $c White 2 0 bigWhite3
    makeBigO $c Black 3 0 bigBlack4
    makeBigO $c White 3 0 bigWhite4
    makeBigO $c Black 0 1 bigBlack5
    makeBigO $c White 0 1 bigWhite5
    makeBigO $c Black 1 1 bigBlack6
    makeBigO $c White 1 1 bigWhite6
    makeBigO $c Black 2 1 bigBlack7
    makeBigO $c White 2 1 bigWhite7
    makeBigO $c Black 3 1 bigBlack8
    makeBigO $c White 3 1 bigWhite8
    makeBigO $c Black 0 2 bigBlack9
    makeBigO $c White 0 2 bigWhite9
    makeBigO $c Black 1 2 bigBlack10
    makeBigO $c White 1 2 bigWhite10
    makeBigO $c Black 2 2 bigBlack11
    makeBigO $c White 2 2 bigWhite11
    makeBigO $c Black 3 2 bigBlack12
    makeBigO $c White 3 2 bigWhite12
    makeBigO $c Black 0 3 bigBlack13
    makeBigO $c White 0 3 bigWhite13
    makeBigO $c Black 1 3 bigBlack14
    makeBigO $c White 1 3 bigWhite14
    makeBigO $c Black 2 3 bigBlack15
    makeBigO $c White 2 3 bigWhite15
    makeBigO $c Black 3 3 bigBlack16
    makeBigO $c White 3 3 bigWhite16

    ##Make all bindings
    $c bind small <Any-Enter> "itemEnter $c"
    $c bind small <Any-Leave> "itemLeave $c"
    $c bind small <1> "placeL $c current"
    
    $c bind neutral <1> "moveBigO $c current"
    $c bind neutral <Any-Enter> "itemEnter $c"
    $c bind neutral <Any-Leave> "itemLeave $c"
    
    $c bind bigO <Any-Enter> "hideNeutralsBigO $c"
    $c bind bigO <Any-Enter> "itemEnterTest $c"
    $c bind bigO <Any-Leave> "itemLeaveTest $c"
    $c bind bigO <1> "moveBigO $c current"
    
    $c raise back
}

#############################################################################
##
## raiseLowerL
##
## Raise or lower (depending on ACTION) the four squares associated with 
## PIECE that are under L-piece number LNUM to level LEV. For example, the 
## command "raiseLowerL $c 1 lower num all" would cause all pieces with tag
## num9, num5, num1, or num2 (9, 5, 1, and 2 are the four squares that 
## L-piece 1 are comprised of) to be lowered to the very bottom.
##
#############################################################################

proc raiseLowerL {c Lnum action piece lev} {
    set db [list 9 5 1 2 10 2 3 6 11 7 3 4 13 9 5 6 14 10 6 7 15 11 7 8 10 2 6 1 11 2 3 7 12 8 3 4 14 10 6 5 15 11 7 6 16 12 8 7 2 3 8 4 6 7 8 12 10 11 12 16 1 2 3 7 5 6 7 11 9 10 11 15 6 7 8 4 11 12 10 8 14 15 16 12 5 6 3 7 9 10 11 7 13 14 15 11 8 12 16 15 7 11 15 14 6 10 14 13 4 8 12 11 3 7 11 10 6 2 10 9 7 11 15 16 6 10 14 15 5 9 13 14 3 7 11 12 2 6 10 11 1 5 9 10 15 14 13 9 11 10 9 5 7 6 5 1 16 15 14 10 12 11 10 6 8 2 7 6 11 10 9 13 7 6 5 9 3 2 1 5 12 11 10 14 8 7 6 10 4 2 3 6]
    for {set i 0} {$i < 4} {incr i} {
	set num [lindex $db [expr (($Lnum - 1) * 4) + $i]]
	$c $action [myappend $piece $num] $lev
    }
}

#############################################################################
##
## itemEnterTest
##
## Test to see if the move is in its second phase when entering a big
## neutral piece. If so, call itemEnter on that piece; if not, do nothing.
##
#############################################################################

proc itemEnterTest {c} {
    global gPhase
    if {$gPhase == 2} {
	itemEnter $c
    }
}

#############################################################################
##
## itemLeaveTest
##
## Test to see if the move is in its second phase when leaving a big
## neutral piece. If so, call itemLeave on that piece; if not, do nothing.
##
#############################################################################

proc itemLeaveTest {c} {
    global gPhase
    if {$gPhase == 2} {
	itemLeave $c
    }
}


#############################################################################
##
## itemEnter
##
## Do a specific action that depends on the object being entered by the
## mouse. Remember the original state of the pieces before doing so.
##
#############################################################################

proc itemEnter {c} {
    global gRestoreCmd gCurrentBlack gCurrentWhite
    
    if {[winfo depth $c] == 1} {
	set gRestoreCmd {}
	return
    }
    set type [$c type current]
    if {$type == "window"} {
	set gRestoreCmd {}
	return
    }
    if {$type == "bitmap"} {
	set bg [lindex [$c itemconf current -background] 4]
	set gRestoreCmd [list $c itemconfig current -background $bg]
	$c itemconfig current -background black
	return
    }
    set fill [lindex [$c itemconfig current -fill] 4]
    if {$type == "oval"} {
	set outline [lindex [$c itemconfig current -outline] 4]
	set gRestoreCmd "$c itemconfig current -fill $fill"
	$c itemconfig current -fill $outline
	if {[lsearch [$c gettags current] Black] != -1} {
	    $c itemconfig bigBlack$gCurrentBlack -stipple gray25
	} else {
	    $c itemconfig bigWhite$gCurrentWhite -stipple gray25
	}
    } 
    if {$type == "polygon" || $type == "rectangle"} {
	set gRestoreCmd "unShowBigL $c current"
	showBigL $c current
    }
}

#############################################################################
##
## itemLeave
##
## Restore all pieces to the way they were before entering the object.
##
#############################################################################

proc itemLeave {c} {
    global gRestoreCmd gCurrentBlack gCurrentWhite
    eval $gRestoreCmd
    $c itemconfig bigBlack$gCurrentBlack -stipple ""
    $c itemconfig bigWhite$gCurrentWhite -stipple ""
}

#############################################################################
##
## showBigL
##
## Upon entering smallL (each small L represents a possible move for the
## L-piece), show where the L-piece would move to and shade the location 
## where the L-piece currently is.
##
#############################################################################

proc showBigL {c smallL} {
    global gPlayerOneTurn gCurrentL1 gCurrentL2
    for {set i 1} {$i <= 48} {incr i} {
	if {[lsearch [$c gettags $smallL] "num$i"] != -1} {
	    if {$gPlayerOneTurn} {
		raiseLowerL $c $gCurrentL1 lower blueSquare back
		raiseLowerL $c $i raise blueSquare shade
	    } else {
		raiseLowerL $c $gCurrentL2 lower redSquare back
		raiseLowerL $c $i raise redSquare shade
	    }
	}
    }
}

#############################################################################
##
## unShowBigL
##
## Upon leaving smallL, move the L-piece back to its current position.
##
#############################################################################

proc unShowBigL {c smallL} {
    global gPlayerOneTurn gCurrentL1 gCurrentL2 gOkay
    for {set i 1} {$i <= 48} {incr i} {
	if {[lsearch [$c gettags $smallL] "num$i"] != -1} {
	    if {$gPlayerOneTurn && $gOkay} {
		raiseLowerL $c $i lower blueSquare back
		raiseLowerL $c $gCurrentL1 raise blueSquare shade
	    }
	    if {[not $gPlayerOneTurn] && $gOkay} {
		raiseLowerL $c $i lower redSquare back
		raiseLowerL $c $gCurrentL2 raise redSquare shade
	    }
	    raiseLowerL $c $i lower square all
	}
    }
}

#############################################################################
##
## placeL
##
## Once the new position of the L-piece is chosen (denoted by smallL), 
## raise the big L-piece to the top of the board. Also, if valueMoves is on,
## then color all the neutral pieces appropriately.
##
#############################################################################

proc placeL {w smallL} {
    global gPhase gPosition gCurrentL1 gCurrentL2 gPlayerOneTurn gFinalL gCurrentBlack gCurrentWhite gOkay gValueMoves gVarMoves
    set gOkay 0
    itemLeave $w
    $w lower shade all
    placeLHelper $w $smallL
    
    ## for valueMoves
    if {$gVarMoves == "valueMoves"} {
	set specValueMoves [list]
	for {set i 0} {$i < [expr [llength $gValueMoves] / 13]} {incr i} {
	    set thisValueMove [lindex $gValueMoves [expr $i * 13]]
	    if {[getNewL [lindex $thisValueMove 0]] == $gFinalL} { 
		for {set j 0} {$j < 13} {incr j} {
		    set specValueMoves [linsert $specValueMoves end [lindex $gValueMoves [expr $i * 13 + $j]]]
		}
	    }
	}
	for {set k 0} {$k < 13} {incr k} {
	    set thisMove [lindex $specValueMoves $k]
	    set thisOPiece [getOPiece [lindex $thisMove 0]]
	    set thisOPieceColor Black
	    if {$thisOPiece == 2} {
		set thisOPieceColor White
	    }
	    set thisOSquare [getNewO [lindex $thisMove 0]]
	    set thisOValue [lindex $thisMove 1]
	    if {$thisOValue == "Win"} {
		if {$k == 12} {
		    $w itemconfig bigBlack$gCurrentBlack -fill red4
		    $w itemconfig bigWhite$gCurrentWhite -fill red4
		}
		if {$k != 12} {
		    $w itemconfig neut$thisOSquare$thisOPieceColor -fill red4 
		}
	    }
	    if {$thisOValue == "Lose"} {
		if {$k == 12} {
		    $w itemconfig bigBlack$gCurrentBlack -fill green
		    $w itemconfig bigWhite$gCurrentWhite -fill green
		}
		if {$k != 12} {
		    $w itemconfig neut$thisOSquare$thisOPieceColor -fill green 
		}
	    }
	    if {$thisOValue == "Tie"} {
		if {$k == 12} {
		    $w itemconfig bigBlack$gCurrentBlack -fill yellow
		    $w itemconfig bigWhite$gCurrentWhite -fill yellow
		}
		if {$k != 12} {
		    $w itemconfig neut$thisOSquare$thisOPieceColor -fill yellow 
		}
	    }
	}
    }
    
    if {$gPlayerOneTurn} {
	raiseLowerL $w $gCurrentL1 raise blueSquare all
    } else {
	raiseLowerL $w $gCurrentL2 raise redSquare all
    }
    set gPhase 2
}

#############################################################################
##
## placeLHelper
##
## Called by placeL, this does some more chores necessary when switching to
## the second phase of the move. It sets a few global variables, lowers the
## old L-piece (where it used to be), and raises all the neutral pieces.
##
#############################################################################

proc placeLHelper {c smallL} {
    global gPlayerOneTurn gCurrentL1 gCurrentL2 gCurrentBlack gCurrentWhite gInitL gFinalL
    set num 0
    if {$gPlayerOneTurn} {
	set piece blueSquare
    } else {
	set piece redSquare
    }
    for {set i 1} {$i <= 48} {incr i} {
	if {[lsearch [$c gettags $smallL] "num$i"] != -1} {
	    if {$gPlayerOneTurn} {
		set gInitL $gCurrentL1
		set gCurrentL1 $i
		set gFinalL $gCurrentL1
	    } else {
		set gInitL $gCurrentL2 
		set gCurrentL2 $i
		set gFinalL $gCurrentL2  
	    }
	    set num $i
	    $c lower $piece all
	}
    }
    $c lower small back
    $c raise neutral back
    $c lower neut$gCurrentBlack all
    $c lower neut$gCurrentWhite all
}

#############################################################################
##
## moveBigO
##
## The main function for completing the second phase of the move (moving the
## neutral piece, aka the BigO). Important steps include changing the color
## of the cursor, lowering the small neutral pieces, checking if the game is
## over, and allowing the computer to move if necessary.
##
#############################################################################

proc moveBigO {c littleO} {
    global gPlayerOneTurn gPhase gOkay xbmLeft gAgainstComputer gValueMoves gPosition gHumanGoesFirst
    if {$gPhase == 1} {
	return
    }
    itemLeave $c
    moveBigOHelper $c $littleO
    set gPhase 1
    set gOkay 1

    global tcl_platform
    if {$gPlayerOneTurn} {
	set gPlayerOneTurn [not $gPlayerOneTurn]
	if { $tcl_platform(platform) != "macintosh" && \
		 $tcl_platform(platform) != "windows" } {
	    $c config -cursor "@$xbmLeft red"
	}
    } else {
	set gPlayerOneTurn [not $gPlayerOneTurn]
	if { $tcl_platform(platform) != "macintosh" && \
		 $tcl_platform(platform) != "windows" } {
	    $c config -cursor "@$xbmLeft blue"
	}
    }
    $c lower neutral back
    if {[llength $gValueMoves] == 0} {
	HandleGameOver $c [C_Primitive $gPosition]
    }
    HandleTurn
    HandlePredictions
    if {$gAgainstComputer && [not $gPlayerOneTurn]} {
	DoComputersMove
    }
}

#############################################################################
##
## moveBigOHelper
##
## This does the remaining steps necessary to complete the second phase of
## the move. Important steps done here include lowering the neutral piece
## where it used to be, raising it where it is now, raising all the 
## necessary small L pieces, and coloring all the small L pieces 
## appropriately if valueMoves is on.
##
#############################################################################

proc moveBigOHelper {c littleO} {
    global gPlayerOneTurn gCurrentL1 gCurrentL2 gCurrentWhite gCurrentBlack gPosition gInitL gFinalL gValueMoves gVarMoves gSlotsX gSlotSize
    
    set num 0
    for {set i 1} {$i <= 16} {incr i} {
	if {([lsearch [$c gettags $littleO] "neut$i"] != -1) ||
	([lsearch [$c gettags $littleO] "bigBlack$i"] != -1) ||
	([lsearch [$c gettags $littleO] "bigWhite$i"] != -1)} {
	    set num $i
	    break
	}
    }
    set color Black
    if {([lsearch [$c gettags $littleO] White] != -1)} {
	set color White
    }
    if {$color == "Black"} {
	set tempNum $gCurrentBlack
	set gCurrentBlack $num
    } else {
	set tempNum $gCurrentWhite
	set gCurrentWhite $num
    }
    if {$gPlayerOneTurn} {
      $c lower redSquare all
    } else {
	$c lower blueSquare all
    }
    $c lower neutral back
    $c lower big$color$tempNum all
    $c raise big$color$num all
    $c raise small back
    $c lower sq$num back
    if {$gPlayerOneTurn} {
	raiseLowerL $c $gCurrentL1 lower sq all
	$c lower num$gCurrentL2 all
	$c itemconfig shade -fill red -outline red 
	raiseLowerL $c $gCurrentL2 raise shade back
	raiseLowerL $c $gCurrentL2 raise redSquare shade
    } else {
	raiseLowerL $c $gCurrentL2 lower sq all
	$c lower num$gCurrentL1 all
	$c itemconfig shade -fill blue -outline blue
	raiseLowerL $c $gCurrentL1 raise shade back
	raiseLowerL $c $gCurrentL1 raise blueSquare shade
    }
    $c lower sq$gCurrentBlack all
    $c lower sq$gCurrentWhite all
    
    ## for valueMoves    
    set newO $num
    if {$newO < 10} {
	set newO [myappend 0 $newO]
    }
    set move [myappend [myappend $gFinalL 1] $newO]
    if {[string match $color White] == 1} {
	set move [myappend [myappend $gFinalL 2] $newO]
    }
    set gPosition [C_DoMove $gPosition $move]
    set gValueMoves [C_GetValueMoves $gPosition 0]
    if {$gVarMoves == "valueMoves"} {	
	for {set i 0} {$i < [expr [llength $gValueMoves] / 13]} {incr i} {
	    set thisL [getNewL [lindex [lindex $gValueMoves [expr $i * 13]] 0]]
	    set thisColor red4
	    for {set j 0} {$j < 13} {incr j} {
		set theValue [lindex [lindex $gValueMoves [expr $i * 13 + $j]] 1]
		if {[string match Lose $theValue] == 1} {
		    set thisColor green
		    break
		}
		if {[string match Tie $theValue] == 1} {
		    set thisColor yellow
		}
	    }
	    $c itemconfig num$thisL -fill $thisColor 
	}
    }
    
    $c itemconfig bigO -fill purple
    
}

#############################################################################
##
## getNewL
##
## Return the number of the L-piece's new position after move MOVE.
##
#############################################################################

proc getNewL {move} {
    return [expr [expr $move - [expr $move % 1000]] / 1000]
}

#############################################################################
##
## getNewO
##
## Return the number of the neutral piece's new position after move MOVE.
##
#############################################################################

proc getNewO {move} {
    return [expr $move % 100]
}

#############################################################################
##
## getOPiece
##
## Return the color of the neutral piece (0 = black, 1 = white) associated 
## with move MOVE.
##
#############################################################################

proc getOPiece {move} {
    set base [expr $move % 1000]
    return [expr [expr $base - [expr $base % 100]] / 100]
}

#############################################################################
##
## myappend
##
## Append the first argument to the value of the second argument.
##
#############################################################################

proc myappend {arg1 arg2} {
    append arg1 $arg2
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame {c} {
    
    global gCurrentL1 gCurrentL2 gCurrentBlack gCurrentWhite gPlayerOneTurn gPosition gInitL gFinalL gPhase gOkay xbmLeft gValueMoves gVarMoves gHumanGoesFirst
    
    ##Set some globals
    set gPlayerOneTurn 1
    set gPhase 1
    set gOkay 1
    set gInitL 1
    set gFinalL 1
    set gPosition 19388
    if {[not $gHumanGoesFirst]} {
	set gPlayerOneTurn 0
	set gPosition 19389
    }
    
    global tcl_platform
    if { $tcl_platform(platform) != "macintosh" && \
	     $tcl_platform(platform) != "windows" } {
	$c config -cursor "@$xbmLeft blue"
    }

    $c itemconfig shade -fill blue -outline blue
    $c itemconfig bigO -fill purple
    
    ##Lower all necessary objects below the grey background
    $c lower back
    for {set i 0} {$i <= 16} {incr i} {
	$c lower square$i all
	$c lower shade all
    }
    $c lower neutral back
    for {set i 0} {$i <= 16} {incr i} {
	$c lower redSquare$i all
	$c lower blueSquare$i all
    }
    $c lower bigO all
        
    ##Place L-piece8
    set gCurrentL1 8
    raiseLowerL $c 8 raise shade back
    raiseLowerL $c 8 raise blueSquare shade
    
    ##Place L-piece32
    raiseLowerL $c 32 raise redSquare all
    set gCurrentL2 32
    
    ##Place the black neutral piece
    $c raise bigBlack1 all
    set gCurrentBlack 1
    
    ##Place the white neutral piece
    $c raise bigWhite16 all
    set gCurrentWhite 16
    
    ##Lower the small L pieces that are not valid
    raiseLowerL $c $gCurrentL2 lower sq all
    $c lower sq$gCurrentBlack back
    $c lower sq$gCurrentWhite back
    $c lower num$gCurrentL1 all
    
    ##Color the small L-pieces if valueMoves is on
    set gValueMoves [C_GetValueMoves $gPosition 0]    
    if {$gVarMoves == "valueMoves"} {
	for {set i 0} {$i < [expr [llength $gValueMoves] / 13]} {incr i} {
	    set thisL [getNewL [lindex [lindex $gValueMoves [expr $i * 13]] 0]]
	    set thisColor red4
	    for {set j 0} {$j < 13} {incr j} {
		set theValue [lindex [lindex $gValueMoves [expr $i * 13 + $j]] 1]
		if {[string match Lose $theValue] == 1} {
		    set thisColor green
		    break
		}
		if {[string match Tie $theValue] == 1} {
		    set thisColor yellow
		}
	    }
	    $c itemconfig num$thisL -fill $thisColor 
	}
    }

    ##If the human player goes second, then do the computer's move
    if {[not $gHumanGoesFirst]} {
	DoComputersMove
    }
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
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {
    
}

#############################################################################
##
## GS_ConvertInteractionToMove
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertInteractionToMove { theMove } {

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

    return {}
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
