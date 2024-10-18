####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "L Game"

    global fourSquares
    set fourSquares [list \
      [list 0 0 0 0] \
	   [list 9 5 1 2] \
	   [list 10 6 2 3] \
	   [list 11 7 3 4] \
	   [list 13 9 5 6] \
	   [list 14 10 6 7] \
	   [list 15 11 7 8] \
	   [list 10 6 2 1] \
	   [list 11 7 3 2] \
	   [list 12 8 4 3] \
	   [list 14 10 6 5] \
	   [list 15 11 7 6] \
	   [list 16 12 8 7] \
	   [list 2 3 4 8] \
	   [list 6 7 8 12] \
	   [list 10 11 12 16] \
	   [list 1 2 3 7] \
	   [list 5 6 7 11] \
	   [list 9 10 11 15] \
	   [list 6 7 8 4] \
	   [list 10 11 12 8] \
	   [list 14 15 16 12] \
	   [list 5 6 7 3] \
	   [list 9 10 11 7] \
	   [list 13 14 15 11] \
	   [list 8 12 16 15] \
	   [list 7 11 15 14] \
	   [list 6 10 14 13] \
	   [list 4 8 12 11] \
	   [list 3 7 11 10] \
	   [list 2 6 10 9] \
	   [list 7 11 15 16] \
	   [list 6 10 14 15] \
	   [list 5 9 13 14] \
	   [list 3 7 11 12] \
	   [list 2 6 10 11] \
	   [list 1 5 9 10] \
	   [list 15 14 13 9] \
	   [list 11 10 9 5] \
	   [list 7 6 5 1] \
	   [list 16 15 14 10] \
	   [list 12 11 10 6] \
	   [list 8 7 6 2] \
	   [list 11 10 9 13] \
	   [list 7 6 5 9] \
	   [list 3 2 1 5] \
	   [list 12 11 10 14] \
	   [list 8 7 6 10] \
	   [list 4 3 2 6] \
   ]
   
   # 0 if moving L piece 1 if moving Neutral Piece
   global gPhase
   set gPhase 0

   global gOkay
   set gOkay 1

   global l1 l2 s1 s2
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 19388
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kToWinStandard  "Force your opponent into a position from which he cannot move."
    set kToWinMisere  "Get yourself in a position from which you cannot move."

    global rectSize
    set rectSize 100

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	#SetToWinString "To Win: (fill in)"
	SetToWinString [concat "To Win: " $kToWinStandard]
    } else {
	#SetToWinString "To Win: (fill in)"
	SetToWinString [concat "To Win: " $kToWinMisere]
    }
    SetToMoveString "To Move: Each player takes turns moving his or her L piece into a different position. At the end of each turn, players place one of the neutral circle pieces onto any open square. Click on the small L that represents the location you want to move your L piece. Then click on the square where you want to place the black or white neutral circle."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Michael Savitzky, Alex Kozlowski"
    set kTclAuthors "Zach Bush, Michael Savitzky, Alex Kozlowski"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}


#############################################################################
# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
#############################################################################
proc GS_NameOfPieces {} {

   return [list blue red]

}


#############################################################################
# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.
#############################################################################
proc GS_ColorOfPlayers {} {

	return [list blue red]
    
}


#############################################################################
# GS_SetupRulesFrame sets up the rules frame;
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing
#############################################################################
proc GS_SetupRulesFrame { rulesFrame } {
    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame"]

    global kLabelFont
    set ruleNum 0
    foreach rule $ruleset {
	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
	pack $rulesFrame.rule$ruleNum.label -side left
	set rulePartNum 0
	foreach rulePart [lrange $rule 1 end] {
	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
	    incr rulePartNum
	}
	incr ruleNum
    } 
}


#############################################################################
# GS_GetOption gets the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
#############################################################################
proc GS_GetOption { } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    return $option
}


#############################################################################
# GS_SetOption modifies the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
#############################################################################
proc GS_SetOption { option } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}


#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################
proc GS_Initialize { c } {

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    # $c configure -width 500 -height 500
    global kLabelFont gMisereGame

    global gFrameWidth gFrameHeight
    global gActive
    set gActive 1

    set edgeBuffer 0
    set rectSize [expr [expr $gFrameHeight - $edgeBuffer * 2] / 4]
    set size [expr (4 * $rectSize) + $edgeBuffer]

    $c create rectangle $edgeBuffer $edgeBuffer $size $size \
      -fill grey \
      -tags back

    for {set j 0} {$j < 5} {incr j} {
       set x [expr ($j * $rectSize) + $edgeBuffer]
       $c create line $x $edgeBuffer $x $size -width 1 -tags back
       $c create line $edgeBuffer $x $size $x -width 1 -tags back
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


    ##Make the blue, red, and grey tiles
    for {set j 0} {$j < 16} {incr j} {
       set x [expr (int(($j % 4)) * $rectSize)]
       set y [expr (int(($j / 4)) * $rectSize)]

       $c create rectangle $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -fill blue \
         -tags "blue-$j" 
       $c create rectangle $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -fill red \
         -tags "red-$j" 
       $c create rectangle $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -stipple gray25 \
         -fill blue \
         -outline blue \
         -width 3 \
         -tags "shade-$j shade" 
       $c create oval $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -fill purple \
         -outline black \
         -width 3 \
         -tags "BigO-black-$j"
       $c create oval $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -fill purple \
         -outline white \
         -width 3 \
         -tags "BigO-white-$j"
       smallO $c $j $x $y
    }

    $c bind small <Enter> "itemEnter $c"
    $c bind small <Leave> "itemLeave $c"
    $c bind small <1> "placeL $c current"

    $c bind neutral <Enter> "itemEnter $c"
    $c bind neutral <Leave> "itemLeave $c"
    $c bind neutral <1> "placeO $c current"

    $c raise back
} 

proc makeL {c num x y t t1 t2 t3 t4} {
    global gFrameHeight
    set edgeBuffer 0
    set rectSize [expr [expr $gFrameHeight - $edgeBuffer * 2] / 4]
    set factor [expr $rectSize / 2]
    
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

proc smallO { c num x y } {
   global gFrameHeight
   set edgeBuffer 0
   set rectSize [expr [expr $gFrameHeight - $edgeBuffer * 2] / 4]
   set factor [expr $rectSize / 2]
   $c create oval $x $y [expr $x + $factor] [expr $y + $factor] \
      -fill cyan -outline black -width 2 -tags "BSmallO$num black neutral"
   $c create oval [expr $x + $factor] [expr $y + $factor] \
      [expr $x + $rectSize] [expr $y + $rectSize] \
      -fill cyan -outline white -width 2 -tags "WSmallO$num white neutral"
}

proc itemEnter { c } {
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
      set gRestoreCmd [list $citemconfig current -background $bg]
      $c itemconfig current -background black
      return
   }
   set fill [lindex [$c itemconfig current -fill] 4]
   if {$type == "oval"} {
      set outline [lindex [$c itemconfig current -outline] 4]
      set gRestoreCmd "$c itemconfig current -fill $fill"
      $c itemconfig current -fill $outline
      if {[lsearch [$c gettags current] "black"] != -1} {
         $c itemconfig BigO-black-[expr $gCurrentBlack - 1] -stipple gray25
      } else {
         $c itemconfig BigO-white-[expr $gCurrentWhite - 1] -stipple gray25
      }
   }
   if {$type == "polygon" || $type == "rectangle"} {
      set gRestoreCmd "unShowBigL $c current"
      showBigL $c current
   }
}

proc itemLeave { c } {
   global gRestoreCmd gCurrentBlack gCurrentWhite
   eval $gRestoreCmd
   $c itemconfig BigO-black-[expr $gCurrentBlack - 1] -stipple ""
   $c itemconfig BigO-white-[expr $gCurrentWhite - 1] -stipple ""
}

proc placeL { c smallL } {
   global gPhase gPosition gCurrentL1 gCurrentL2 gPlayerOneTurn gFinalL gCurrentBlack gCurrentWhite gOkay gValueMovels gValMoves
   set gOkay 0
   itemLeave $c
   $c lower shade all
   for {set i 0} {$i <= 48} {incr i} {
      if {[lsearch [$c gettags $smallL] "num$i"] != -1} {
         set gFinalL $i
      }
   }
   set gOkay 1
   set gPhase 1
   drawMoves $c
}

proc placeO { c smallO } {
   global gPhase gPosition gCurrentL1 gCurrentL2 gPlayerOneTurn gFinalL gCurrentBlack gCurrentWhite gOkay gValueMovels gValMoves
   set gOkay 0
   itemLeave $c
   set piece 0
   set value 0
   $c lower shade all
   if {$gPlayerOneTurn} {
      set gPlayerOneTurn [not $gPlayerOneTurn]
   }
   for {set i 0} {$i <= 48} {incr i} {
      puts [lindex [$c gettags $smallO] 0]
      puts "WSmallO$i"
      if {[lsearch [$c gettags $smallO] "WSmallO$i"] != -1} {
         set gCurrentWhite [expr $i + 1]
         set piece 1
         set value [expr $i + 1]
      } elseif {[lsearch [$c gettags $smallO] "BSmallO$i"] != -1} {
         set gCurrentBlack [expr $i + 1]
         set piece 2
         set value [expr $i + 1]
      }
   }

   puts "Piece: $piece\nValue: $value"
   set gOkay 1
   set gPhase 0
   drawMoves $c
   ReturnFromHumanMove [expr (1000 * $gFinalL) + (100 * $piece) + $value]
}

proc raiseLowerL { c Lnum action piece lev} {
   global fourSquares
   set pieces [lindex $fourSquares $Lnum]
   for {set i 0} {$i < 4} {incr i} {
      puts [format "$action-ing $piece-%i" [expr [lindex $pieces $i] - 1]]
      $c $action $piece-[expr [lindex $pieces $i] - 1] $lev
   }
}

proc showBigL {c smallL} {
   global gPlayerOneTurn gCurrentL1 gCurrentL2
   for {set i 1} {$i <= 48} {incr i} {
      if {[lsearch [$c gettags $smallL] "num$i"] != -1} {
         if {$gPlayerOneTurn} {
            raiseLowerL $c $gCurrentL1 lower red back
            raiseLowerL $c $i raise red shade
         } else {
            raiseLowerL $c $gCurrentL2 lower blue back
            raiseLowerL $c $i raise blue shade
         }
      }
   }
}

proc not { bool } {
   if { $bool } {
      return 0
   } else {
      return 1
   }
}

proc unShowBigL { c smallL } {
   global gPlayerOneTurn gCurrentL1 gCurrentL2 gOkay
   for {set i 1} {$i <= 48} {incr i} {
      if {[lsearch [$c gettags $smallL] "num$i"] != -1} {
         if {$gPlayerOneTurn && $gOkay} {
            raiseLowerL $c $i lower red back
            raiseLowerL $c $gCurrentL1 raise red shade
         }
         if {[not $gPlayerOneTurn] && $gOkay} {
            raiseLowerL $c $i lower blue back
            raiseLowerL $c $gCurrentL2 raise blue shade
         }
         raiseLowerL $c $i lower square all
      }
   }
}
#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}


#############################################################################
# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.
#
# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {

#    $c raise back

    global gCurrentL1 gCurrentL2 gCurrentBlack gCurrentWhite gPlayerOneTurn gOkay
    
    set gCurrentL1 [unhashL1 $position]
    set gCurrentL2 [24to48 $gCurrentL1 [unhashL2 $position]]
    set gCurrentBlack [8to16 $gCurrentL1 $gCurrentL2 [unhashS1 $position]]
    set gCurrentWhite [7to16 $gCurrentL1 $gCurrentL2 $gCurrentBlack [unhashS2 $position]]
    if {[unhashTurn $position] == 1} {
       set gPlayerOneTurn 1
    } else {
       set gPlayerOneTurn 0
    }

    $c lower back
    for {set i 0} {$i < 16} {incr i} {
       $c lower blue-$i all
       $c lower shade-$i all
       $c lower red-$i all
       $c lower BigO-white-$i
       $c lower BigO-black-$i
    }
    $c raise back
    raiseLowerL $c $gCurrentL1 raise shade back
    raiseLowerL $c $gCurrentL1 raise red shade
    raiseLowerL $c $gCurrentL2 raise blue all
    $c raise BigO-black-[expr $gCurrentBlack - 1]
    $c raise BigO-white-[expr $gCurrentWhite - 1]

}

proc unhashL1 { position } {
   return [expr int((($position >> 1) / 7 / 8 / 24) % 48) + 1]
}

proc unhashL2 { position } {
   return [expr int((($position >> 1) / 7 / 8) % 24) + 1];
}

proc 24to48 { L1 L2 } {
   set pieces [list \
      [list 0] \
      [list 3 5 6 9 11 12 14 15 19 20 21 24 25 26 27 28 29 31 32 34 40 41 46 47] \
      [list 6 12 21 24 25 26 28 31 33 37 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 4 7 10 21 25 27 30 32 33 36 37 40 0 0 0 0 0 0 0 0 0 0 0] \
      [list 3 6 8 9 12 13 15 16 20 21 25 26 28 29 31 34 40 46 0 0 0 0 0 0] \
      [list 1 9 13 25 28 45 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 2 4 7 10 27 30 33 36 45 48 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 3 6 9 12 21 24 25 26 28 31 33 34 37 0 0 0 0 0 0 0 0 0 0 0] \
      [list 4 10 21 25 27 32 33 36 37 40 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 4 5 7 10 11 17 18 23 24 26 27 30 31 32 33 35 36 37 38 39 40 43 44] \
      [list 3 6 8 9 12 13 16 25 28 31 34 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 9 13 33 36 45 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 2 4 7 10 18 24 27 30 32 33 35 36 37 38 43 45 48 0 0 0 0 0 0] \
      [list 4 5 10 11 15 17 18 21 23 24 26 27 31 32 33 36 37 38 39 40 41 43 44 46] \
      [list 1 18 24 33 36 37 38 40 43 45 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 4 13 16 19 22 33 37 39 42 44 45 48 0 0 0 0 0 0 0 0 0 0 0] \
      [list 4 10 15 18 20 21 24 25 27 28 32 33 37 38 40 41 43 46 0 0 0 0 0 0] \
      [list 9 13 21 25 37 40 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 9 12 13 14 16 19 22 39 42 45 48 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 15 18 21 24 33 36 37 38 40 43 45 46 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 4 16 22 33 37 39 44 45 48 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 2 3 4 7 8 13 16 17 19 22 23 29 30 35 36 38 39 42 43 44 45 47 48] \
      [list 15 18 20 21 24 25 28 37 40 43 46 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 9 13 21 25 45 48 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 2 7 9 12 13 14 16 19 22 30 36 39 42 44 45 47 48 0 0 0 0 0 0] \
      [list 1 2 3 4 5 7 8 10 16 17 22 23 27 29 30 33 35 36 38 39 43 44 45 48] \
      [list 1 2 4 7 9 13 30 36 45 48 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 3 6 8 9 12 13 16 25 28 31 34 45 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 2 4 5 7 10 16 22 27 30 32 33 36 37 39 40 44 45 0 0 0 0 0 0] \
      [list 1 4 21 25 33 37 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 3 6 9 12 21 24 25 26 28 31 34 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 2 4 7 9 10 13 27 30 33 36 45 48 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 3 8 9 12 13 16 28 34 45 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 2 3 6 7 8 9 11 12 13 14 15 16 19 20 25 28 29 31 34 35 41 42 47 48] \
      [list 1 4 7 10 27 30 32 33 36 37 40 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 9 12 21 25 33 37 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 3 6 8 9 11 12 13 14 19 21 24 25 26 28 31 34 42 48 0 0 0 0 0 0] \
      [list 2 3 7 8 9 12 13 14 15 16 17 19 20 22 28 29 34 35 39 41 42 45 47 48] \
      [list 9 12 13 14 16 19 21 25 42 48 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 9 13 15 18 20 21 24 25 28 37 40 43 46 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 3 4 8 9 13 14 16 17 19 22 28 34 39 42 44 45 48 0 0 0 0 0 0] \
      [list 1 13 16 33 37 45 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 15 18 21 24 33 36 37 38 40 43 46 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 9 12 13 14 16 19 21 22 25 39 42 45 48 0 0 0 0 0 0 0 0 0 0 0] \
      [list 9 13 15 20 21 24 25 28 40 46 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 5 6 11 12 14 15 18 19 20 21 23 24 25 26 27 28 31 32 37 40 41 43 46 47] \
      [list 1 4 13 16 19 22 39 42 44 45 48 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 1 21 24 33 37 45 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] \
      [list 6 12 15 18 20 21 23 24 25 26 31 33 36 37 38 40 43 46 0 0 0 0 0 0] \
   ]
   set rel [lindex $pieces $L1]
   return [lindex $rel [expr $L2 - 1]]
}

proc 8to16 { L1 L2 S1as8 } {
   set S1as16 0
   global fourSquares

   for {set i 1} {$i < 17} {incr i} {
      set arr($i) 0
   }
   for {set i 0} {$i < 4} {incr i} {
      set arr([lindex [lindex $fourSquares $L1] $i]) 1
      set arr([lindex [lindex $fourSquares $L2] $i]) 1
   }
   for {set i 1} {$i < 17} {incr i} {
      set current $arr($i)
      if {$current == 0} {
         if {$S1as8 == 1} {
            set arr($i) 2
         }
         set S1as8 [expr $S1as8 - 1]
      }
   }
   for {set i 1} {$i < 17} {incr i} {
      if {$arr($i) == 2} {
         set S1as16 $i
      }
   }
   return $S1as16
}   

proc 7to16 { L1 L2 S1 S2as7 } {
   set S2as16 0
   global fourSquares
   for {set i 1} {$i < 17} {incr i} {
      set arr($i) 0
   }
   for {set i 0} {$i < 4} {incr i} {
      set arr([lindex [lindex $fourSquares $L1] $i]) 1
      set arr([lindex [lindex $fourSquares $L2] $i]) 1
   }
   set arr($S1) 1
   for {set i 1} {$i < 17} {incr i} {
      if {$arr($i) == 0} {
         if {$S2as7 == 1} {
            set arr($i) 2
         }
         set S2as7 [expr $S2as7 - 1]
      }
   }
   for {set i 1} {$i < 17} {incr i} {
      if {$arr($i) == 2} {
         set S2as16 $i
      }
   }
   return $S2as16
}

proc unhashS1 { position } {
   return [expr int((($position >> 1) / 7) % 8) + 1];
}

proc unhashS2 { position } {
   return [expr int(($position >> 1) % 7) + 1];
}

proc unhashTurn { position } {
   return [expr int($position & 1) + 1]
}

proc unhashMoveL { position } {
   return [expr $position / 1000]
}

proc unhashMoveSPiece { position } {
   set total [expr $position - ($position / 1000) * 1000]
   return [expr $total / 100]
}

proc unhashMoveSValue { position } {
   return [expr $position % 100]
}

proc BigL { c pos color } {
   global fourSquares
   for {set j 0} {$j < 16} {incr j} {
      $c lower $color-[expr $j]
   }
   set squares [lindex $fourSquares $pos]
   for {set i 0} {$i < 4} {incr i} {
      $c raise $color-[expr [lindex $squares $i] - 1] 
   }
}

proc BigO { c pos color } {
   for {set j 0} {$j < 16} {incr j} {
      $c lower BigO-$color-$j
   }
   $c raise BigO-$color-[expr $pos - 1]
}

#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    global gActive
    set gActive 1
    GS_DrawPosition $c $position
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    # Optional Procedure
    set turn [unhashTurn $position]
    if {$turn == 0} {
       return red
    } else {
       return blue
    }
}


#############################################################################
# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"
#
# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.
#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {
   global gCurrentL1 gCurrentL2 gCurrentBlack gCurrentWhite gPlayerOneTurn gOkay

   if { $gPlayerOneTurn } {
      set gCurrentL1 [unhashMoveL $theMove]
   } else {
      set gCurrentL2 [unhashMoveL $theMove]
   }

   if {[unhashMoveSPiece $theMove] == 1} {
      set gCurrentBlack [unhashMoveSValue $theMove]
   } else {
      set gCurrentWhite [unhashMoveSValue $theMove]
   }
   GS_DrawPosition $c $newPosition
}


#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

   global gMoveList gMoveType gPosition gPhase 
   set gMoveList $moveList
   set gMoveType $moveType
   set gPosition $position
   # set gPhase 0

   drawMoves $c
}

proc drawMoves { c } {
   global gMoveList gMoveType gPosition gPhase gFinalL gActive
   for {set i 0} {$i < 48} {incr i} {
      $c lower num$i
   }
   for {set i 0} {$i < 16} {incr i} {
      $c lower BSmallO$i
      $c lower WSmallO$i
   }
   if {$gPhase != 0} {
      raiseLowerL $c $gFinalL raise red back
   }

   if {$gActive} {

      foreach item $gMoveList {
         set move [lindex $item 0]
         set value [lindex $item 1]
         set remoteness [lindex $item 2]
         set delta [lindex $item 3]
         set color cyan
         if {$gMoveType == "value" || $gMoveType == "rm"} {
            if {$value == "Tie"} {
               set color yellow
            } elseif {$value == "Lose"} {
               set color green
            } else {
               set color red
            }
         }

         if {$gPhase == 0} {
            set number [unhashMoveL $move]
            $c raise num$number 
            $c itemconfigure num$number -fill $color  
         } else {
            if {[unhashMoveL $move] == $gFinalL} { 
               if {[unhashMoveSPiece $move] == 1} { 
                  puts [unhashMoveSValue $move]
                  $c raise BSmallO[expr [unhashMoveSValue $move] -1] all
                  $c itemconfigure BSmallO[expr [unhashMoveSValue $move] -1] -fill $color
               } else {
                  $c raise WSmallO[expr [unhashMoveSValue $move] -1] all
                  $c itemconfigure WSmallO[expr [unhashMoveSValue $move] -1] -fill $color
               }
            }
         }
      }
   }
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {



}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {

    ### TODO if needed
    GS_DrawPosition $c $positionAfterUndo
}


#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {
}


#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
   
   global gActive
   set gActive 0
   GS_DrawPosition $c $position

	
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is finished.
# This is provided so that you may undo the drawing you did in GS_GameOver if you 
# drew something.
# For instance, if you drew a line crossing out the winning row in tic tac toe, 
# this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

	### TODO if needed

}
