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
    set kGameName "3Dot"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "standard WINS"
    set kMisereString "standard LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click on an oval to first move your colored piece, then your white piece. You score one point for each dot covered with your colored piece. You must move both your colored and the white piece each turn."
    set kToWinStandard  "If your have 12 points and your opponent has at least 6 you win! If your have 12, but your opponent has under 6, you lose!!"
    set kToWinMisere  "If your have 12 points and your opponent has less than 6 you win! If your have 12, but your opponent has at least 6, you lose!!"

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 3
    set gSlotsY 3
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 0x001009BD
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
    set kCAuthors "Attila Gyulassy"
    set kTclAuthors "Attila Gyulassy"
    set kGifAuthors "$kRootDir/../bitmaps/lite3team.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCircle
    set kRightDrawProc DrawCircle
    set kBothDrawProc  DrawCircle


#    global xbmLeft xbmRight macLeft macRight kRootDir
#    set xbmLeft  "$kRootDir/../bitmaps/plus.xbm"
#    set xbmRight "$kRootDir/../bitmaps/plus.xbm"
#    set macLeft  cross
#    set macRight cross

    ### Set the procedures that will draw the pieces
    #####%%%%this is where i need to modify gamesman
#    global kLeftDrawProc kRightDrawProc kBothDrawProc
#    set kLeftDrawProc  DrawCross
#    set kRightDrawProc DrawHollowCircle
#    set kBothDrawProc  DrawCircle

    ### What type of interaction will it be, sir?
    ####%%%%modify gamesman to support my types of moves
    global kInteractionType
    set kInteractionType "3DotSpecial"

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 0

    global kMoveToYCoordHor 
    set  kMoveToYCoordHor [list 210 110 10 210 110 10 ]
    
    global kMoveToXCoordHor 
    set  kMoveToXCoordHor [list 10 10 10 110 110 110 ]
    
    global kMoveToYCoordVer 
    set  kMoveToYCoordVer [list 10 10 10 110 110 110 ]
    
    global kMoveToXCoordVer 
    set  kMoveToXCoordVer [list 10 110 210 10 110 210 ]
    
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
    destroy $w.c
    destroy $w.f1
    Do_3Dot_Thing $w

}







###
# i'm thinking to ignore most of what gamesman gives us and go straight for the meat of the matter!!
#
##
#
# set up the initial menu 
##
#set f1 .f1
#frame .f1

#label .f1.welcome -text "welcome to gamesman"
#pack append .f1 .f1.welcome 
#pack append .f1 
##

###%%% add pieces needed for m3dot
image create photo blankv -file "$kRootDir/../bitmaps/blankv.gif" -format GIF
image create photo blankh -file "$kRootDir/../bitmaps/blankh.gif" -format GIF
image create photo redv -file "$kRootDir/../bitmaps/redv.gif" -width 100 -height 200 -format GIF
image create photo redh -file "$kRootDir/../bitmaps/redh.gif" -width 200 -height 100 -format GIF
image create photo whitev -file "$kRootDir/../bitmaps/whitev.gif" -width 100 -height 200 -format GIF
image create photo whiteh -file "$kRootDir/../bitmaps/whiteh.gif" -width 200 -height 100 -format GIF
image create photo bluev -file "$kRootDir/../bitmaps/bluev.gif" -width 100 -height 200 -format GIF
image create photo blueh -file "$kRootDir/../bitmaps/blueh.gif" -width 200 -height 100 -format GIF
global kPlayerTurn
set kPlayerTurn "r"
global mywindow
global myMove    
global redScore
global blueScore
set redScore 0
set blueScore 0
font create scoreFont -size 24 -weight bold

proc Do_3Dot_Thing { w } {
#    puts "creating my 3dot board"
    #set up the board
    
    global kPlayerTurn
    set kPlayerTurn "r"
    canvas $w.c -width 320 -height 360 
    
    global mywindow
    set mywindow $w
    #SET UP THE GRID
    $w.c create rectangle 0 0 320 360 -fill gray -tags thegrid
    foreach temp {10 110 210 310} {
	$w.c create line 10 $temp 310 $temp -tags thegrid
	$w.c create line $temp 10 $temp 310 -tags thegrid
    }
    
    # create initial score
    global gPosition 
    $w.c create text 100 335 -text [getRedScore $gPosition] -tag RedScore -font scoreFont -fill red
    $w.c create text 200 335 -text [getBlueScore $gPosition] -tag BlueScore -font scoreFont -fill blue

    #% create the initial configuration
#    $w.c create image 10 110 -image blankv -anchor nw -tag {blankvR RED blankpiece visiblePieces ver}
    $w.c create image 10 110 -image redv -anchor nw -tag {redv RED redpiece visiblePieces ver currR}
    $w.c create image 10 110 -image redv -anchor nw -tag {redv2 RED redpiece ver }
    $w.c create image 110 110 -image whitev -anchor nw -tag {whitev WHITE whitepiece visiblePieces ver currW}    
    $w.c create image 110 110 -image whitev -anchor nw -tag {whitev2 WHITE whitepiece ver}
#    $w.c create image 110 110 -image blankv -anchor nw -tag {blankvW WHITE blankpiece visiblePieces ver}
    $w.c create image 210 110 -image bluev -anchor nw -tag {bluev BLUE bluepiece visiblePieces ver currB}
    $w.c create image 210 110 -image bluev -anchor nw -tag {bluev2 BLUE bluepiece  ver}
#    $w.c create image 210 110 -image blankv -anchor nw -tag {blankvB BLUE blankpiece visiblePieces ver}
    
#    $w.c create image 10 110 -image blankh -anchor nw -tag {blankhR RED blankpiece  hor}
    $w.c create image 10 110 -image redh -anchor nw -tag {redh RED redpiece  hor}
    $w.c create image 10 110 -image redh -anchor nw -tag {redh2 RED redpiece  hor}
#    $w.c create image 110 110 -image blankh -anchor nw -tag {blankhW WHITE blankpiece  hor}
    $w.c create image 110 110 -image whiteh -anchor nw -tag {whiteh WHITE whitepiece  hor}
    $w.c create image 110 110 -image whiteh -anchor nw -tag {whiteh2 WHITE whitepiece  hor}
    $w.c create image 210 110 -image blueh -anchor nw -tag {blueh BLUE bluepiece  hor}
    $w.c create image 210 110 -image blueh -anchor nw -tag {blueh2 BLUE bluepiece  hor}
#    $w.c create image 210 110 -image blankh -anchor nw -tag {blankhB BLUE blankpiece  hor}    
    
    #create the dots
    foreach temp {40 140 240} {
	$w.c create oval [expr $temp] 240 [expr $temp + 40] 280 -tags thedots
    } 
    $w.c itemconfig thedots -fill darkgray
    
    #global RedBPos
    #set RedBPos [list v 10 110]
    #global WhiteBPos
    #set WhiteBPos [list v 110 110]
    #global BlueBPos
    #set BlueBPos [list v 210 110]
    #create the move ovals

    foreach temp {50 150 250} {
	$w.c create oval [expr $temp] 90 [expr $temp + 20] 130 \
		-tags [list h[expr $temp]v90 thebuttons vertical]
	$w.c create oval [expr $temp] 190 [expr $temp + 20] 230 \
		-tags [list thebuttons h[expr $temp]v190 vertical]
	
	$w.c create oval 90 [expr $temp] 130 [expr $temp + 20] \
		-tags [list thebuttons h90v[expr $temp] horizontal] 
	$w.c create oval 190 [expr $temp] 230 [expr $temp + 20] \
		-tags [list thebuttons h190v[expr $temp] horizontal]
	
    } 
    $w.c itemconfig thebuttons -width 1
    $w.c bind horizontal <Any-Enter> {
	if {$kPlayerTurn == "r"} {
	    $mywindow.c raise current
	    $mywindow.c itemconfig current -width 2 
	    $mywindow.c lower currR   
	    $mywindow.c coords redh2 \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower redh2 current
	}
	if {$kPlayerTurn == "b"} {
	    $mywindow.c raise current
	    $mywindow.c itemconfig current -width 2 
	    $mywindow.c lower currB   
	    $mywindow.c coords blueh2 \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower blueh2 current
	}
	if {$kPlayerTurn == "rw" || $kPlayerTurn == "bw" } {
	    $mywindow.c raise current
	    $mywindow.c itemconfig current -width 2 
	    $mywindow.c lower currW   
	    $mywindow.c coords whiteh2 \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower whiteh2 current
	}
    }

    # HORIZONTAL LEFT CLICK BINDING
    ########################################
    $mywindow.c bind horizontal <1> {
	global myMove
	if {$kPlayerTurn == "r"} then {
	    set kPlayerTurn "rw"

	    set theValue [C_Primitive $gPosition]
	    
	    ### And, depending on the value of the game...
	    
	    if { $theValue != "Undecided" } { 
	    
		HandleGameOver $w $theValue
		return
	    }



	    $mywindow.c coords blankvR \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhR \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords redh \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower currR
	    $mywindow.c lower blankvR
	    $mywindow.c dtag currR visiblePieces
	    $mywindow.c dtag currR currR
	    $mywindow.c addtag currR withtag redh 
	    $mywindow.c addtag currR withtag blankhR
	    $mywindow.c addtag visiblePieces withtag redh
	    $mywindow.c lower current	    
	    set myMove [clickToMove "h" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
#	    puts "mymove: $myMove"
	    set myMove [expr $myMove << 4]
#	    puts "mymove: $myMove"
	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves2 $mywindow.c [expr $myMove >> 4]		
	    } else {
		Show3DotValueMoves2 $mywindow.c [expr $myMove >> 4]		
	    }
	    
	} elseif {$kPlayerTurn == "rw"} then {
	    set kPlayerTurn "b"
	    $mywindow.c coords blankvW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords whiteh \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower currW
	    $mywindow.c lower blankvW
	    $mywindow.c dtag currW visiblePieces
	    $mywindow.c dtag currW currW
	    $mywindow.c addtag currW withtag whiteh 
	    $mywindow.c addtag currW withtag blankhW
	    $mywindow.c addtag visiblePieces withtag whiteh
	    $mywindow.c lower current
	    global myMove
	    set temp [clickToMove "h" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
#	    #puts "mymove: $myMove"
	    set myMove [expr $myMove | $temp]
	    set gPosition [C_DoMove $gPosition $myMove]

	    #puts "mymove: $myMove"

	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves1 $mywindow.c 		
	    } else {
		Show3DotValueMoves1 $mywindow.c 		
	    }
	    
	    
	    HandlePredictions; HandleTurn
	    HandleTurnEnd $mywindow
	} elseif {$kPlayerTurn == "bw"} then {
	    set kPlayerTurn "r"
	    $mywindow.c coords blankvW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords whiteh \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower currW
	    $mywindow.c lower blankvW
	    $mywindow.c dtag currW visiblePieces
	    $mywindow.c dtag currW currW
	    $mywindow.c addtag currW withtag whiteh 
	    $mywindow.c addtag currW withtag blankhW
	    $mywindow.c addtag visiblePieces withtag whiteh
	    $mywindow.c lower current

	    set temp [clickToMove "h" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
	    #puts "mymove: $myMove"
	    set myMove [expr $myMove | $temp]
	    set gPosition [C_DoMove $gPosition $myMove]

	    #puts "mymove: $myMove"
	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves1 $mywindow.c 		
	    } else {
		Show3DotValueMoves1 $mywindow.c 		
	    }
	    
	    HandlePredictions; HandleTurn
	    HandleTurnEnd $mywindow
	} elseif {$kPlayerTurn == "b"} then {



	    set theValue [C_Primitive $gPosition]
	    
	    ### And, depending on the value of the game...
	    
	    if { $theValue != "Undecided" } { 
	    
		HandleGameOver $w $theValue
		return
	    }



	    set kPlayerTurn "bw"

	    $mywindow.c coords blankvB \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhB \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords blueh \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c lower currB
	    $mywindow.c lower blankvB
	    $mywindow.c dtag currB visiblePieces
	    $mywindow.c dtag currB currB
	    $mywindow.c addtag currB withtag blueh 
	    $mywindow.c addtag currB withtag blankhW
	    $mywindow.c addtag visiblePieces withtag blueh
	    $mywindow.c lower current

	    set myMove [clickToMove "h" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
	    #puts "mymove: $myMove"
	    set myMove [expr $myMove << 4]
	    #puts "mymove: $myMove"

	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves2 $mywindow.c [expr $myMove >> 4]		
	    } else {
		Show3DotValueMoves2 $mywindow.c [expr $myMove >> 4]		
	    }
	    
	} 
    }

    $w.c bind vertical <Any-Enter> {
	if {$kPlayerTurn == "r"} {
	    $mywindow.c raise current
	    $mywindow.c itemconfig current -width 2 
	    $mywindow.c lower currR   
	    $mywindow.c coords redv2 \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower redv2 current
	}
	if {$kPlayerTurn == "b"} {
	    $mywindow.c raise current
	    $mywindow.c itemconfig current -width 2 
	    $mywindow.c lower currB   
	    $mywindow.c coords bluev2 \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower bluev2 current
	}
	if {$kPlayerTurn == "rw" || $kPlayerTurn == "bw" } {
	    $mywindow.c raise current
	    $mywindow.c itemconfig current -width 2 
	    $mywindow.c lower currW   
	    $mywindow.c coords whitev2 \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower whitev2 current
	}
    }

    $mywindow.c bind vertical <1> {
	if {$kPlayerTurn == "r"} then {

	    set theValue [C_Primitive $gPosition]
	    
	    ### And, depending on the value of the game...
	    
	    if { $theValue != "Undecided" } { 
	    
		HandleGameOver $w $theValue
		return
	    }


	    set kPlayerTurn "rw"
	    $mywindow.c coords blankvR \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhR \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords redv \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower currR
	    $mywindow.c lower blankhR
	    $mywindow.c dtag currR visiblePieces
	    $mywindow.c dtag currR currR
	    $mywindow.c addtag currR withtag redv 
	    $mywindow.c addtag currR withtag blankvR
	    $mywindow.c addtag visiblePieces withtag redv
	    $mywindow.c lower current	    

	    set myMove [clickToMove "v" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
	    #puts "mymove: $myMove"
	    set myMove [expr $myMove << 4]
	    #puts "mymove: $myMove"

	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves2 $mywindow.c [expr $myMove >> 4]		
	    } else {
		Show3DotValueMoves2 $mywindow.c [expr $myMove >> 4]		
	    }
	    

	} elseif {$kPlayerTurn == "rw"} then {
	    set kPlayerTurn "b"
	    $mywindow.c coords blankvW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords whitev \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower currW
	    $mywindow.c lower blankhW
	    $mywindow.c dtag currW visiblePieces
	    $mywindow.c dtag currW currW
	    $mywindow.c addtag currW withtag whitev 
	    $mywindow.c addtag currW withtag blankvW
	    $mywindow.c addtag visiblePieces withtag whitev
	    $mywindow.c lower current
	    global myMove
	    set temp [clickToMove "v" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
	    #puts "mymove: $myMove"
	    set myMove [expr $myMove | $temp]
	    set gPosition [C_DoMove $gPosition $myMove]

	    #puts "mymove: $myMove"


	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves1 $mywindow.c 
	    } else {
		Show3DotValueMoves1 $mywindow.c 
	    }

	    HandlePredictions; HandleTurn
	    HandleTurnEnd $mywindow

	} elseif {$kPlayerTurn == "bw"} then {
	    set kPlayerTurn "r"
	    $mywindow.c coords blankvW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhW \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords whitev \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower currW
	    $mywindow.c lower blankhW
	    $mywindow.c dtag currW visiblePieces
	    $mywindow.c dtag currW currW
	    $mywindow.c addtag currW withtag whitev 
	    $mywindow.c addtag currW withtag blankvW
	    $mywindow.c addtag visiblePieces withtag whitev
	    $mywindow.c lower current
	    global myMove
	    set temp [clickToMove "v" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
	    #puts "mymove: $myMove"
	    set myMove [expr $myMove | $temp]
	    set gPosition [C_DoMove $gPosition $myMove]

	    #puts "mymove: $myMove"


	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves1 $mywindow.c 
	    } else {
		Show3DotValueMoves1 $mywindow.c 
	    }
	    HandlePredictions; HandleTurn
	    HandleTurnEnd $mywindow

	} elseif {$kPlayerTurn == "b"} then {

	    set theValue [C_Primitive $gPosition]
	    
	    ### And, depending on the value of the game...
	    
	    if { $theValue != "Undecided" } { 
	    
		HandleGameOver $w $theValue
		return
	    }

	    set kPlayerTurn "bw"
	    $mywindow.c coords blankvB \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c coords blankhB \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 80] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 40]
	    $mywindow.c coords bluev \
		    [expr [lindex [eval $mywindow.c coords current] 0] - 40] \
		    [expr [lindex [eval $mywindow.c coords current] 1] - 80]
	    $mywindow.c lower currB
	    $mywindow.c lower blankhB
	    $mywindow.c dtag currB visiblePieces
	    $mywindow.c dtag currB currB
	    $mywindow.c addtag currB withtag bluev 
	    $mywindow.c addtag currB withtag blankvW
	    $mywindow.c addtag visiblePieces withtag bluev
	    $mywindow.c lower current

	    set myMove [clickToMove "v" \
		    [lindex [eval $mywindow.c coords current] 0]  \
		    [lindex [eval $mywindow.c coords current] 1] ]
	    #puts "mymove: $myMove"
	    set myMove [expr $myMove << 4]
	    #puts "mymove: $myMove"
	    global varMoves
	    if { $varMoves == "validMoves" } {
		Show3DotMoves2 $mywindow.c [expr $myMove >> 4]
	    } else {
		Show3DotValueMoves2 $mywindow.c [expr $myMove >> 4]
	    }
	} 
    }


    ####################
    $mywindow.c bind thebuttons <Any-Leave> {
	if {$kPlayerTurn == "r"} then {
	    #puts "leaving window"
	    global mywindow
	    $mywindow.c itemconfig current -width 1
	    $mywindow.c lower redpiece
	    $mywindow.c raise currR
	} elseif {$kPlayerTurn == "rw"} then {
	    #puts "leaving window"
	    global mywindow
	    $mywindow.c itemconfig current -width 1
	    $mywindow.c lower whitepiece
	    $mywindow.c raise currW
	} elseif {$kPlayerTurn == "bw"} then {
	    global mywindow
 	    $mywindow.c itemconfig current -width 1
	    $mywindow.c lower whitepiece
	    $mywindow.c raise currW
	} elseif {$kPlayerTurn == "b"} then {
	    global mywindow
	    $mywindow.c itemconfig current -width 1
	    $mywindow.c lower bluepiece
	    $mywindow.c raise currB
	}
    }
    

    
    #make the right pieces show up	
    
    $w.c lower hor
    $w.c lower ver
    $w.c raise visiblePieces    

    #$w.c addtag hidden withtag hor
    #$w.c lower ver
    
    #pack $w.c
    GS_PostProcessBoard $w.c
    #%
    #puts "this actually gets executed"
    pack append $w $w.c top
    global redScore
    global blueScore
    frame $w.f1
    label $w.f1.l1 -text "RED: $redScore" -anchor sw
    label $w.f1.l2 -text "BLUE: $blueScore" -anchor se
    pack $w.f1
    Show3DotMoves1 $w.c
    $w.c raise visiblePieces    
}


proc Show3DotMoves1 { w } {
    
    global kCurrntColoredMove
    global kTurnPhase
    set kTurnPase 2
        
    $w dtag currentMoves currentMoves
    $w lower thebuttons
    $w itemconfig thebuttons -fill cyan
    global gPosition 
    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
	#puts "asdfasdfasdfasdf: [expr $theMove >> 4]"
	#at this point we have move, just as it was returned from gamesman
	set orient [expr [expr $theMove >> 4] & 1]
	set pos [expr [expr $theMove >> 5] - 1]
	#puts $pos
	if {$orient == 0} {
	    set xvalue [expr [expr [expr $pos / 3] * 100] + 90]
	    set yvalue [expr [expr [expr 2 - [expr $pos % 3]] * 100] + 50]	
	} else {
	    set yvalue [expr [expr [expr $pos / 3] * 100] + 90]
	    set xvalue [expr [expr [expr $pos % 3] * 100] + 50]
	}
	$w addtag currentMoves withtag h[expr $xvalue]v[expr $yvalue]
	#XSputs $theMove
	#$w itemconfig currentMoves -fill red

	$w raise currentMoves
    }
}

proc Show3DotMoves2 { w coloredmove } {

    global kCurrntColoredMove
    global kTurnPhase
    set kTurnPase 1
    set kCurrntColoredMove $coloredmove
    
    #puts "SHOWDOT MOVES 2 : $coloredmove"
    $w dtag currentMoves currentMoves
    $w lower thebuttons
    global gPosition 
    $w itemconfig thebuttons -fill cyan
    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
	#puts "[expr $theMove >> 4] : == ? $coloredmove "
	if { $coloredmove == [expr $theMove >> 4]} {
	    #puts "got here"
	    #at this point we have move, just as it was returned from gamesman
	    set orient [expr [expr $theMove & 0x0F] & 1]
	    set pos [expr [expr [expr $theMove & 0x0F] >> 1]  - 1]
	    #puts $pos
	    if {$orient == 0} {
		set xvalue [expr [expr [expr $pos / 3] * 100] + 90]
		set yvalue [expr [expr [expr 2 - [expr $pos % 3]] * 100] + 50]	
	    } else {
		set yvalue [expr [expr [expr $pos / 3] * 100] + 90]
		set xvalue [expr [expr [expr $pos % 3] * 100] + 50]
	    }
	    $w addtag currentMoves withtag h[expr $xvalue]v[expr $yvalue]
	    #XSputs $theMove
	    #$w itemconfig currentMoves -fill red
	    
	    $w raise currentMoves
	}
    }
}

proc Show3DotValueMoves1 { w } {
    global kCurrntColoredMove
    global kTurnPhase
    set kTurnPase 2
    #set kCurrntColoredMove $coloredmove
    
    $w dtag currentMoves currentMoves
    $w lower thebuttons
    
    global gPosition 
    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
	

	#puts "asdfasdfasdfasdf: [expr $theMove >> 4]"
	#at this point we have move, just as it was returned from gamesman
	set orient [expr [expr $theMove >> 4] & 1]
	set pos [expr [expr $theMove >> 5] - 1]
	#puts $pos
	if {$orient == 0} {
	    set xvalue [expr [expr [expr $pos / 3] * 100] + 90]
	    set yvalue [expr [expr [expr 2 - [expr $pos % 3]] * 100] + 50]	
	} else {
	    set yvalue [expr [expr [expr $pos / 3] * 100] + 90]
	    set xvalue [expr [expr [expr $pos % 3] * 100] + 50]
	}
	$w addtag currentMoves withtag h[expr $xvalue]v[expr $yvalue]	
	global currColor
	set currColor red4
    	foreach  theMoveValue2 [C_GetValueMoves $gPosition] {
	    if { [expr [lindex $theMoveValue2 0] & 0xF0] == [expr $theMove & 0xF0]} {
		#puts "successasdfasdfasdf"
		#puts $currColor
		
		#puts [lindex $theMoveValue2 1]
		#puts [expr [lindex $theMoveValue2 1] == "Lose"]
#		if {[lindex $theMoveValue2 1] == "Lose"} {puts "good work!!"}
		if { [lindex $theMoveValue2 1] == "Tie" } {
		    if { $currColor == "red4" } {
			set currColor yellow
			#puts $currColor
		    }
		} elseif {[lindex $theMoveValue2 1] == "Lose"} {
		    set currColor green
		    #puts $currColor
		}
	    }
	}
	#puts $currColor
	$w itemconfig h[expr $xvalue]v[expr $yvalue] -fill $currColor
	
	    
	    #XSputs $theMove
	#$w itemconfig currentMoves -fill red

	$w raise currentMoves
    }
}
global kCurrntColoredMove
global kTurnPhase
set kTurnPhase 1

proc Show3DotValueMoves2 { w coloredmove } {

    global kCurrntColoredMove
    global kTurnPhase
    set kTurnPase 1
    set kCurrntColoredMove $coloredmove
    #puts "SHOWDOT MOVES 2 : $coloredmove"
    $w dtag currentMoves currentMoves
    $w lower thebuttons
    global gPosition 
    foreach theMoveValue [C_GetValueMoves $gPosition] {
	set theMove [lindex $theMoveValue 0]
	#puts "[expr $theMove >> 4] : == ? $coloredmove "
	if { $coloredmove == [expr $theMove >> 4]} {
	    #puts "got here"
	    #at this point we have move, just as it was returned from gamesman
	    set orient [expr [expr $theMove & 0x0F] & 1]
	    set pos [expr [expr [expr $theMove & 0x0F] >> 1]  - 1]
	    #puts $pos
	    if {$orient == 0} {
		set xvalue [expr [expr [expr $pos / 3] * 100] + 90]
		set yvalue [expr [expr [expr 2 - [expr $pos % 3]] * 100] + 50]	
	    } else {
		set yvalue [expr [expr [expr $pos / 3] * 100] + 90]
		set xvalue [expr [expr [expr $pos % 3] * 100] + 50]
	    }
	    $w addtag currentMoves withtag h[expr $xvalue]v[expr $yvalue]
	    #XSputs $theMove
	    #$w itemconfig currentMoves -fill red
	    $w itemconfig h[expr $xvalue]v[expr $yvalue] -fill [ValueToColor [lindex $theMoveValue 1]]
	    $w raise currentMoves
	}
    }

}
global newmove

proc clickToMove { o x y } {
    if { $o == "h" } {
	#puts "here i am "
	#puts $x
	#puts $y
	set row [expr [expr $x - 90] / 100]
	set col [expr [expr [expr 250 - $y] / 100] + 1]
	set newmove [expr int([expr $col + [expr 3 * $row]]) << 1]
	set newmove $newmove
	#puts "@@@@ : $newmove"
	return $newmove 
    } else {
	set col [expr [expr [expr $x - 50] / 100] + 1]
	set row [expr [expr $y - 90] / 100]
	set newmove [expr int( [expr $col + [expr 3 * $row]]) << 1]
	set newmove [expr $newmove + 1]
	#puts "asdf: $newmove"
	#puts "@@@@2 : $newmove"
	return $newmove 
    }
}

proc getBlueScore { pos } {
    set pos [expr [expr $pos >> 12] & 0x0F]
    return $pos
}

proc getRedScore { pos } {
    set pos [expr [expr $pos >> 16] & 0x0F]
    return $pos
}

proc HandleTurnEnd { w } {
    global gAgainstComputer
    global kPlayerTurn
    global gPosition
    global gPlayerOneTurn
    set $gPlayerOneTurn [expr !$gPlayerOneTurn]
	#HandleGameOver $w $theValue
    
    $w.c itemconfig RedScore -text [getRedScore $gPosition] 
    $w.c itemconfig BlueScore -text [getBlueScore $gPosition] 
    
    if { [not $gAgainstComputer] } {

	### Do nothing. It was all taken care of for us already.
	### And it's now the next person's turn.
	
    } else { 

	### It's now time to do the computer's move.

	set theValue [C_Primitive $gPosition]

	### And, depending on the value of the game...

	if { $theValue != "Undecided" } { 
	    
	    HandleGameOver $w $theValue
	    return
	}

	
	#DoComputersMove
	set theMove  [C_GetComputersMove $gPosition]
		#puts "hellow worod"
	
	set gPosition [C_DoMove $gPosition $theMove]
	ConvertMoveToInteraction $w $theMove
		
	if { $kPlayerTurn == "r" } {
	    set kPlayerTurn "b"
	} else {
	    set kPlayerTurn "r"
	}
	### We return because 'DoComputersMove' does the three Handlers already
	
	HandlePredictions

### handle the game ending here since if the position is primitice, then the
### player won't be able to move, hence the game ends

	set theValue [C_Primitive $gPosition]

	if { $theValue != "Undecided" } { 
	    
	    HandleGameOver $w $theValue
	    return
	}

	return
    }
}

proc ConvertMoveToInteraction { w theMove } {
    global kPlayerTurn
    global gPosition

    global kMoveToYCoordHor 
    global kMoveToXCoordHor 
    global kMoveToYCoordVer 
    global kMoveToXCoordVer 
    #puts "hellow worod: $theMove"

    #if the computer just moved, then turns are reversed
    if { $kPlayerTurn == "r" } {
	#puts "hellow worod"

	set ori [expr [expr $theMove >> 4] & 1]
	set pos [expr [expr $theMove >> 5] - 1]
	if { $ori == 1 } {
	    set xpos [lindex $kMoveToXCoordVer $pos]
	    set ypos [lindex $kMoveToYCoordVer $pos]

	    $w.c coords blankvR $xpos $ypos		
	    $w.c coords blankhR $xpos $ypos		
	    $w.c coords redv $xpos $ypos		
	    
	    $w.c lower RED
	    $w.c dtag currR visiblePieces
	    $w.c dtag currR currR
	    $w.c dtag currR RED
	    $w.c addtag currR withtag redv 
	    $w.c addtag currR withtag blankvW
	    $w.c addtag visiblePieces withtag currR
	    #$w.c lower current
	    $w.c raise visiblePieces
	
	} else {
	    set xpos [lindex $kMoveToXCoordHor $pos]
	    set ypos [lindex $kMoveToYCoordHor $pos]
	

	    $w.c coords blankvR $xpos $ypos		
	    $w.c coords blankhR $xpos $ypos		
	    $w.c coords redh $xpos $ypos		
	    
	    $w.c lower RED
	    $w.c dtag currR visiblePieces
	    $w.c dtag currR currR
	    $w.c dtag currR RED
	    $w.c addtag currR withtag redh 
	    $w.c addtag currR withtag blankhW
	    $w.c addtag visiblePieces withtag currR
	    #$w.c lower current
	    $w.c raise visiblePieces
	    # now show the white moves

	}
	# now show the white moves
	
	#Show3DotMoves2 $w.c [expr $myMove >> 4]


    } elseif { $kPlayerTurn == "b" } {
	#puts "hellow worod"
	
	set ori [expr [expr $theMove >> 4] & 1]
	set pos [expr [expr $theMove >> 5] - 1]
	if { $ori == 1 } {
	    set xpos [lindex $kMoveToXCoordVer $pos]
	    set ypos [lindex $kMoveToYCoordVer $pos]
	    
	    $w.c coords blankvB $xpos $ypos		
	    $w.c coords blankhB $xpos $ypos		
	    $w.c coords bluev $xpos $ypos		
	    
	    $w.c lower BLUE
	    $w.c dtag currB visiblePieces
	    $w.c dtag currB currB
	    $w.c dtag currB BLUE
	    $w.c addtag currB withtag bluev 
	    $w.c addtag currB withtag blankvW
	    $w.c addtag visiblePieces withtag currB
	    #$w.c lower current
	    $w.c raise visiblePieces

	} else {
	    set xpos [lindex $kMoveToXCoordHor $pos]
	    set ypos [lindex $kMoveToYCoordHor $pos]

	    #puts "ConvertMovetoInteraction: $pos $xpos $ypos"
	    $w.c coords blankvB $xpos $ypos		
	    $w.c coords blankhB $xpos $ypos		
	    $w.c coords blueh $xpos $ypos		
	    

	    $w.c lower BLUE
	    $w.c dtag currB visiblePieces
	    $w.c dtag currB currB
	    $w.c dtag currB BLUE
	    $w.c addtag currB withtag blueh 
	    $w.c addtag currB withtag blankhW
	    $w.c addtag visiblePieces withtag currB
	    #$w.c lower current
	    $w.c raise visiblePieces
	    # now show the white moves

	}

	
	#Show3DotMoves2 $w.c [expr $theMove >> 4]
    }
  
    #puts "barf1"

    # REFRESH BOARD
    $w.c raise visiblePieces
    Show3DotMoves2 $w.c [expr $theMove >> 4]

    # NOW WAIT A LITTLE BIT
    set barf [clock clicks]
    while {[clock clicks] < [expr $barf + 500000]} { }
    #puts "barf2"
    # END WAITING


    set ori [expr $theMove & 1]
    set pos [expr [expr [expr $theMove & 0x0F] >>1] - 1]
    if { $ori == 1 } {
	set xpos [lindex $kMoveToXCoordVer $pos]
	set ypos [lindex $kMoveToYCoordVer $pos]

	$w.c coords blankvW $xpos $ypos
	$w.c coords blankhW $xpos $ypos
	$w.c coords whitev $xpos $ypos
	
	$w.c lower WHITE
	$w.c dtag currW visiblePieces
	$w.c dtag currW currW
	$w.c dtag currW WHITE
	
	$w.c addtag currW withtag whitev 
	$w.c addtag currW withtag blankvW
	$w.c addtag visiblePieces withtag currW
	$w.c lower current

    } else {
	set xpos [lindex $kMoveToXCoordHor $pos]
	set ypos [lindex $kMoveToYCoordHor $pos]

	$w.c coords blankvW $xpos $ypos
	$w.c coords blankhW $xpos $ypos
	$w.c coords whiteh $xpos $ypos

	$w.c lower WHITE
	$w.c dtag currW visiblePieces
	$w.c dtag currW currW
	$w.c dtag currW WHITE
	
	$w.c addtag currW withtag whiteh 
	$w.c addtag currW withtag blankhW
	$w.c addtag visiblePieces withtag currW
	$w.c lower current

    }
    $w.c raise visiblePieces

    global varMoves
    if { $varMoves == "validMoves"} {
	Show3DotMoves1 $w.c 
    } else {
	Show3DotValueMoves1 $w.c 
    }
    $w.c itemconfig RedScore -text [getRedScore $gPosition] 
    $w.c itemconfig BlueScore -text [getBlueScore $gPosition] 


}	


