####################################################
# Rubik's Magic TCL/TK front end
# by Jeffrey Chiang and Bryon Ross
# Last Modified: 03-09-09
#
# based on  a template for tcl module creation
# created by Alex Kozlowski and Peterson Trethewey
####################################################

# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Rubik's Magic"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Bryon Ross, Jeff Chiang"
    set kTclAuthors "Bryon Ross, Jeff Chiang"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the initial position of the board
    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set toWin and toMove
    global gMisereGame gTwoInARow gBoardsizeOption
    if { $gMisereGame } {
	set toWin1 "To Win: "
    } else {
	set toWin1 "To Lose: "
    }

    set toWin2 "Allow "
    
    if { $gTwoInARow } {
	set toWin3 "two"
    } else {
	set toWin3 "three"
    }

    set toWin4 " of your opponent's same-color pieces to be in a row at the end of your turn or the game."

    SetToWinString [concat $toWin1 $toWin2 $toWin3 $toWin4]

    SetToMoveString "To Move: If possible, flip an opponent's piece to an open adjacent space by clicking an arrow. This will change the color of the piece and its position.  Then place your black or white piece on the board by selecting the desired piece in an  empty square."

    global WIDTH BOARDSIZE
    if { $gBoardsizeOption == "0" } {
	set WIDTH 3
	set BOARDSIZE 9
    } elseif { $gBoardsizeOption == "1" } {
	set WIDTH 4
	set BOARDSIZE 12
    }
}


# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
# The left player's piece should be the first item in the list.
# The right player's piece should be second.

proc GS_NameOfPieces {} {
    return [list x o]
}

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

proc GS_ColorOfPlayers {} {
    return [list blue red]
}


# Setup the rules frame
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

proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    set twoInARowRule \
	[list \
	     "Two in a row equivalent to three in a row" \
	     "No" \
	     "Yes" \
	    ]

    set boardsizeRule \
	[list \
	     "Width and length of board" \
	     "3x3" \
	     "4x3" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $twoInARowRule $boardsizeRule]

    # Declare and initialize rule globals
    global gMisereGame gTwoInARow gBoardsizeOption
    set gMisereGame 0
    set gTwoInARow 0
    # 0=3x3 1=4x3
    set gBoardsizeOption 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gTwoInARow" "gBoardsizeOption"]

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

# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code

proc GS_GetOption { } {
    global gMisereGame gTwoInARow gBoardsizeOption
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    set option [expr $option + $gTwoInARow*2]
    set option [expr $option + $gBoardsizeOption*2*2]
    return $option
}


# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing

proc GS_SetOption { option } {
    global gMisereGame gTwoInARow gBoardsizeOption
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
    set gTwoInARow [expr $option/2%2]
    set gBoardsizeOption [expr $option/(2*2)%2]
}

proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}

# GS_Initialize draws the graphics for the game on the canvas c
# You could put an opening animation or just draw an empty board.
# This function is called after GS_InitGameSpecific

proc GS_Initialize { c } {

    global blockSize WIDTH BOARDSIZE boardWidth boardHeight
    global firstHalfAlreadyAnimated gFrameWidth gFrameHeight preferredHeight preferredWidth

    set HEIGHT [expr $BOARDSIZE/$WIDTH]
    set firstHalfAlreadyAnimated 0

    set preferredHeight [min $gFrameWidth $gFrameHeight]
    set preferredWidth $preferredHeight
    
    # The label has been removed due to an error in GAMESMAN
    # set labelSize 50.0
    set labelSize 0.0

    set widthLimit [expr $preferredWidth/$WIDTH]
    set heightLimit [expr ($preferredHeight-$labelSize)/$HEIGHT]

    if {$heightLimit < $widthLimit} {
	set blockSize $heightLimit
    } else {
	set blockSize $widthLimit
    }

    set boardWidth [expr $blockSize * $WIDTH]
    set boardHeight [expr $blockSize * $HEIGHT]

    $c delete all

    # draw base
    $c configure -width [expr $blockSize*$WIDTH] -height [expr $blockSize*$HEIGHT+$labelSize]
    $c create line 0 [expr ($blockSize*$HEIGHT+$labelSize)/2] [expr $blockSize*$WIDTH] [expr ($blockSize*$HEIGHT+$labelSize)/2] -width [expr $blockSize*$HEIGHT+$labelSize] -fill grey50 -tag base

    # draw vertical gridlines
    for {set i 1} {$i <= $WIDTH} {incr i} {
	$c create line [expr $boardWidth*$i/$WIDTH] 0 [expr $boardWidth*$i/$WIDTH] $boardHeight -width 2 -tag base
    }

    # draw horizontal gridlines
    for {set i 1} {$i <= $HEIGHT} {incr i} {
	$c create line 0 [expr $boardHeight*$i/$HEIGHT] $boardWidth [expr $boardHeight*$i/$HEIGHT] -width 2 -tag base
    }

    # board outline
    $c create line 0 0 $boardWidth 0 -width 2 -tag base 
    $c create line 0 0 0 $boardHeight -width 2 -tag base 
    $c create line 0 $boardHeight $boardWidth $boardHeight -width 2 -tag base 
    $c create line $boardWidth 0 $boardWidth $boardHeight -width 2 -tag base 

    MakePieces $c 0
    MakeArrows $c 0
    $c raise base all

    # The winner indicator does not function correctly due to a bug in GAMESMAN 
    # font create ourArial -family arial -size 35 
    # $c create line 0 [expr $boardHeight+25] $boardWidth [expr $boardHeight+25] -width 50 -fill grey50 -tag base
    # $c create line 0 $boardHeight 0 [expr $boardHeight*50] -width 2 -tag base
    # $c create line $boardWidth $boardHeight $boardWidth [expr $boardHeight+50] -width 2 -tag base
    # $c create line 0 [expr $boardHeight+50] $boardWidth [expr $boardHeight+50] -width 2 -tag base
    # $c create text [expr $boardWidth/2] [expr $boardHeight+25] -font ourArial -text "Rubik's Magic" -tag labelrubix

    global firstHalfAlreadyAnimated
    set firstHalfAlreadyAnimated 0

    update idletasks
} 

proc MakeArrows { c num } {
    global BOARDSIZE WIDTH
    BoxArrow $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    if { $num < [expr $BOARDSIZE-1] } {
	MakeArrows $c [expr $num + 1]
    }
}

proc BoxArrow { c x y tag } {
    global blockSize BOARDSIZE WIDTH
    set unit [expr $blockSize/100.0]
    set gap [expr $blockSize/10]
    set displacement [expr ($blockSize-2*$gap)/3]
    
    set x [expr $x * $blockSize]
    set y [expr $y * $blockSize]

    set arrowList [list [expr $gap*2] [expr $gap*2] $gap]

    $c create line [expr $x + $unit*50] [expr $y - $displacement] [expr $x + $unit*50] [expr $y + $displacement] -width $gap -arrow first -fill cyan -tag a-U$tag -arrowshape $arrowList
    $c create line [expr $x + $unit*50] [expr $y + $blockSize - $displacement] [expr $x + $unit*50] [expr $y + $blockSize + $displacement] -width $gap -arrow last -fill cyan -tag a-D$tag -arrowshape $arrowList
    $c create line [expr $x + $displacement] [expr $y + $unit*50] [expr $x - $displacement] [expr $y + $unit*50] -width $gap -arrow last -fill cyan -tag a-L$tag -arrowshape $arrowList
    $c create line [expr $x + $blockSize - $displacement] [expr $y + $unit*50] [expr $x + $blockSize + $displacement] [expr $y + $unit*50] -width $gap -arrow last -fill cyan -tag a-R$tag -arrowshape $arrowList

    $c lower a-U$tag base
    $c lower a-D$tag base
    $c lower a-L$tag base
    $c lower a-R$tag base

}

proc MakePieces { c num } {
    global BOARDSIZE WIDTH

    MakeX $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    MakeO $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    Makex $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    Makeo $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    
    MakeClickX $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    MakeClickO $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    MakeClickx $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num
    MakeClicko $c [expr $num % $WIDTH] [expr $num / $WIDTH] $num

    if { $num < [expr $BOARDSIZE-1] } {
	MakePieces $c [expr $num + 1]
    }
}

proc Makex { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize]
    set y [expr $y * $blockSize]
    $c create line [expr $x + $unit*2] [expr $y + $unit*50] [expr $x +  $unit*98] [expr $y +  $unit*50] -width [expr $unit*96] -fill black -tag xb-$tag
    $c create line [expr $x  +  $unit*10] [expr $y +  $unit*10] [expr $x +  $unit*90] [expr $y +  $unit*90] -width [expr $unit*10] -fill blue -tag xc-$tag
    $c create line [expr $x  +  $unit*10] [expr $y +  $unit*90] [expr $x +  $unit*90] [expr $y +  $unit*10] -width [expr $unit*10] -fill blue -tag xc-$tag
    $c lower xb-$tag base
    $c lower xc-$tag base
}
 
proc MakeX { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize]
    set y [expr $y * $blockSize]
    $c create line [expr $x +  $unit*2] [expr $y +  $unit*50] [expr $x +  $unit*98] [expr $y +  $unit*50] -width [expr $unit*96] -fill white -tag Xb-$tag
    $c create line [expr $x  +  $unit*10] [expr $y +  $unit*10] [expr $x +  $unit*90] [expr $y +  $unit*90] -width [expr $unit*10] -fill blue -tag Xc-$tag
    $c create line [expr $x  +  $unit*10] [expr $y +  $unit*90] [expr $x +  $unit*90] [expr $y +  $unit*10] -width [expr $unit*10] -fill blue -tag Xc-$tag
    $c lower Xb-$tag base
    $c lower Xc-$tag base
}

proc Makeo { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize]
    set y [expr $y * $blockSize]
    $c create line [expr $x + $unit*2] [expr $y + $unit*50] [expr $x + $unit*98] [expr $y + $unit*50] -width [expr $unit*96] -fill black -tag ob-$tag
    $c create oval [expr $x  + $unit*10] [expr $y + $unit*10] [expr $x + $unit*90] [expr $y + $unit*90] -width [expr $unit*10] -outline red -tag oc-$tag
    $c lower ob-$tag base
    $c lower oc-$tag base
}

proc MakeO { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize]
    set y [expr $y * $blockSize]
    $c create line [expr $x + $unit*2] [expr $y + $unit*50] [expr $x + $unit*98] [expr $y + $unit*50] -width [expr $unit*96] -fill white -tag Ob-$tag
    $c create oval [expr $x  + $unit*10] [expr $y + $unit*10] [expr $x + $unit*90] [expr $y + $unit*90] -width [expr $unit*10] -outline red -tag Oc-$tag
    $c lower Ob-$tag base
    $c lower Oc-$tag base
}

proc MakeClicko { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize + $unit*17]
    set y [expr $y * $blockSize]

    #value color
    $c create line [expr $x-$unit*3] [expr $y + $unit*50] [expr $x + $unit*28] [expr $y + $unit*50] -width [expr $unit*31] -fill black -tag coc-$tag
    #contrast box
    $c create line [expr $x-$unit*1] [expr $y + $unit*50] [expr $x + $unit*26] [expr $y + $unit*50] -width [expr $unit*27] -fill black -tag cob-$tag

    $c create line $x [expr $y + $unit*50] [expr $x + $unit*25] [expr $y + $unit*50] -width [expr $unit*25] -fill black -tag cob-$tag
    $c create oval [expr $x  + $unit*3] [expr $y + $unit*41] [expr $x + $unit*21] [expr $y + $unit*59] -width [expr $unit*3] -outline red -tag cob-$tag

    $c lower coc-$tag base
    $c lower cob-$tag base
}

proc MakeClickO { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize + $unit*58]
    set y [expr $y * $blockSize]

    #value color
    $c create line [expr $x-$unit*3] [expr $y + $unit*50] [expr $x + $unit*28] [expr $y + $unit*50] -width [expr $unit*31] -fill black -tag cOc-$tag
    #contrast box
    $c create line [expr $x-$unit*1] [expr $y + $unit*50] [expr $x + $unit*26] [expr $y + $unit*50] -width [expr $unit*27] -fill black -tag cOb-$tag
    
    $c create line $x [expr $y + $unit*50] [expr $x + $unit*25] [expr $y + $unit*50] -width [expr $unit*25] -fill white -tag cOb-$tag
    $c create oval [expr $x  + $unit*3] [expr $y + $unit*41] [expr $x + $unit*21] [expr $y + $unit*59] -width [expr $unit*3] -outline red -tag cOb-$tag

    $c lower cOc-$tag base
    $c lower cOb-$tag base
}

proc MakeClickx { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize + $unit*17]
    set y [expr $y * $blockSize]

    #value color
    $c create line [expr $x-$unit*3] [expr $y + $unit*50] [expr $x + $unit*28] [expr $y + $unit*50] -width [expr $unit*31] -fill black -tag cxc-$tag
    #contrast box
    $c create line [expr $x-$unit*1] [expr $y + $unit*50] [expr $x + $unit*26] [expr $y + $unit*50] -width [expr $unit*27] -fill black -tag cxb-$tag

    $c create line $x [expr $y + $unit*50] [expr $x + $unit*25] [expr $y + $unit*50] -width [expr $unit*25] -fill black -tag cxb-$tag
    $c create line [expr $x  + $unit*3] [expr $y + $unit*41] [expr $x + $unit*21] [expr $y + $unit*59] -width [expr $unit*3] -fill blue -tag cxb-$tag
    $c create line [expr $x  + $unit*21] [expr $y + $unit*41] [expr $x + $unit*3] [expr $y + $unit*59] -width [expr $unit*3] -fill blue -tag cxb-$tag
    
    $c lower cxc-$tag base
    $c lower cxb-$tag base
}

proc MakeClickX { c x y tag } {
    global blockSize
    set unit [expr $blockSize/100.0]

    set x [expr $x * $blockSize + $unit*58]
    set y [expr $y * $blockSize]

    #value color
    $c create line [expr $x-$unit*3] [expr $y + $unit*50] [expr $x + $unit*28] [expr $y + $unit*50] -width [expr $unit*31] -fill black -tag cXc-$tag
    #contrast box
    $c create line [expr $x-$unit*1] [expr $y + $unit*50] [expr $x + $unit*26] [expr $y + $unit*50] -width [expr $unit*27] -fill black -tag cXb-$tag

    $c create line $x [expr $y + $unit*50] [expr $x + $unit*25] [expr $y + $unit*50] -width [expr $unit*25] -fill white -tag cXb-$tag
    $c create line [expr $x  + $unit*3] [expr $y + $unit*41] [expr $x + $unit*21] [expr $y + $unit*59] -width [expr $unit*3] -fill blue -tag cXb-$tag
    $c create line [expr $x  + $unit*21] [expr $y + $unit*41] [expr $x + $unit*3] [expr $y + $unit*59] -width [expr $unit*3] -fill blue -tag cXb-$tag

    $c lower cXc-$tag base
    $c lower cXb-$tag base
}
    
proc GS_Deinitialize { c } {   
    $c delete all
}

# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.

# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.

proc Unhash {position arrayname} {
    upvar $arrayname a
    global BOARDSIZE

    set boardString [C_Unhash $position]

    for {set i 0} {$i < $BOARDSIZE} {incr i} {
	set a($i) [string index $boardString $i]
    }
}

proc Hash { arrayname } {
    upvar $arrayname a
    global BOARDSIZE

    set boardString ""
    for {set i 0} {$i < $BOARDSIZE} {incr i} {
	set boardString $boardString$a($i)
    }
    
    set newPosition [C_Hash $boardString]
    return $newPosition
}

proc GS_DrawPosition { c position } {
    global BOARDSIZE
    $c raise base all
    # $c raise labelrubix base
    set a(0) 0

    Unhash $position a

    for {set i 0} {$i < $BOARDSIZE} {incr i} {
	if { $a($i) != "-" } {
	    $c raise [subst $a($i)]b-$i base
	    $c raise [subst $a($i)]c-$i [subst $a($i)]b-$i
	}
    }
    update idletasks
}

# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    global BOARDSIZE

    set a(bob) 0
    set xCount 0
    set oCount 0
    Unhash $position a
    for {set i 0} {$i < $BOARDSIZE} {incr i} {
	if {$a($i) == "x" || $a($i) == "X"} {
	    incr xCount
	} elseif {$a($i) == "o" || $a($i) == "O"} {
	    incr oCount
	}
    }

    if {$oCount == $xCount} {
	return "x"
    }
    return "o"
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global firstHalfAlreadyAnimated BOARDSIZE

    set moveNum [lindex $theMove 0]

    set flippedPiece [expr $moveNum%$BOARDSIZE]
    set direction [expr $moveNum/$BOARDSIZE%4]
    set newPiece [expr $moveNum/($BOARDSIZE*4)%$BOARDSIZE]
    set orientation [expr $moveNum/($BOARDSIZE*4*$BOARDSIZE)%5]
    set cantFlip [expr ($moveNum/($BOARDSIZE*$BOARDSIZE*4*5)%2)]

    if { $direction == "0" } { set direction U }
    if { $direction == "1" } { set direction D }
    if { $direction == "2" } { set direction L }
    if { $direction == "3" } { set direction R }

    if { $orientation == "0" } { set orientation - }
    if { $orientation == "1" } { set orientation o }
    if { $orientation == "2" } { set orientation O }
    if { $orientation == "3" } { set orientation x }
    if { $orientation == "4" } { set orientation X }

    if { $firstHalfAlreadyAnimated == 1 || $cantFlip == 1 } {
	AnimateMovePart2 $c $orientation $newPiece
    } else {
	set board(bob) 0
	Unhash $oldPosition board
	set startingOrientation $board($flippedPiece)
	AnimateMovePart1 $c $flippedPiece $startingOrientation $direction
	set midPosition [PositionFromMovePart1 board $oldPosition $flippedPiece $direction $startingOrientation]
	GS_DrawPosition $c $midPosition
	AnimateMovePart2 $c $orientation $newPiece
    }

    GS_DrawPosition $c $newPosition
}

proc AnimateMovePart1 { c movedFrom startingOrientation direction } {
    global blockSize
    set unit [expr $blockSize/100]

    # increment must be divisible by 100
    set increment [ScaleUpAnimation 10]

    if { $startingOrientation == "o" || $startingOrientation == "x" } {
	set colorIncrement $increment; set initial 0
    } else {
	set colorIncrement -$increment; set initial 100
    }

    if { $direction == "U" } { set xDir 0; set yDir -1 }
    if { $direction == "D" } { set xDir 0; set yDir 1 }
    if { $direction == "L" } { set xDir -1; set yDir 0 }
    if { $direction == "R" } { set xDir 1; set yDir 0 }

    for { set i 0 } { $i < [expr 100/$increment] } { incr i } {
	$c move [subst $startingOrientation]b-$movedFrom [expr $xDir*$increment*$unit] [expr $yDir*$increment*$unit]
	$c move [subst $startingOrientation]c-$movedFrom [expr $xDir*$increment*$unit] [expr $yDir*$increment*$unit]
	$c itemconfigure [subst $startingOrientation]b-$movedFrom -fill grey[expr $initial+int($i*$colorIncrement)]
#	after 0
	update idletasks
    }

    $c lower [subst $startingOrientation]b-$movedFrom base
    $c lower [subst $startingOrientation]c-$movedFrom base
    $c move [subst $startingOrientation]b-$movedFrom [expr $xDir*-100*$unit] [expr $yDir*-100*$unit]
    $c move [subst $startingOrientation]c-$movedFrom [expr $xDir*-100*$unit] [expr $yDir*-100*$unit]
    $c itemconfigure [subst $startingOrientation]b-$movedFrom -fill grey$initial
   
    global firstHalfAlreadyAnimated
    set firstHalfAlreadyAnimated 1
}

proc AnimateMovePart2 { c orientation newPiece } {
    #if the bar is set higher, make the number of frames and duration lower
    set numframes [ScaleDownAnimation 5]
#    set animDuration [ScaleDownAnimation 1000]
#    set clicksPerFrame [expr $animDuration/$numframes]
    set increment [expr 50/$numframes]

    $c itemconfigure [subst $orientation]b-$newPiece -fill grey50
    $c raise [subst $orientation]b-$newPiece base
    if { $orientation == "o" || $orientation == "x" } {
	set colorIncrement -$increment
	set finalVal 0
    } else {
	set colorIncrement $increment
	set finalVal 100
    }
    for { set i 50 } { $i<=100 && $i>=0 } { set i [expr $i + $colorIncrement] } {
	$c itemconfigure [subst $orientation]b-$newPiece -fill grey[expr int($i)]
	update idletasks
#	after [expr int($clicksPerFrame)]
    }

    $c itemconfigure [subst $orientation]b-$newPiece -fill grey[subst $finalVal]
    update idletasks

    global firstHalfAlreadyAnimated
    set firstHalfAlreadyAnimated 0
}


# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.

# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of available moves to the player.  These moves are represented as numbers (same as in C)

# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveValueToColor that takes in moveType and a moveValue and returns the correct color.
# For example [MoveValueToColor "value" "Tie"] would return yellow and [MoveValueToColor "all" "anything"] would return cyan.

proc GS_ShowMoves { c moveType position moveList } {
    global BOARDSIZE
    set a(bob) 0
    set v(bob) 0

    global movePart1Memory firstHalfAlreadyAnimated
    global gInitialPosition

    if { $position == $gInitialPosition } {
	set firstHalfAlreadyAnimated 0
    }
    
    if { [llength $moveList] <= 0 } {
	return
    }

    set aMove [lindex [lindex $moveList 0] 0]

    if { $firstHalfAlreadyAnimated == 1 } {
	ShowMovesPart2 $c $moveType [lindex $movePart1Memory 0] [lindex $movePart1Memory 1] [lindex $movePart1Memory 2] [lindex $movePart1Memory 3]
    } elseif { [expr ($aMove/($BOARDSIZE*$BOARDSIZE*4*5)%2)] == 1 } {
	set secondMoves [list]
	foreach item $moveList {
	    set moveNum [lindex $item 0]
	    set value [lindex $item 1]
	    set newPiece [expr $moveNum/($BOARDSIZE*4)%$BOARDSIZE]
	    set orientation [expr $moveNum/($BOARDSIZE*4*$BOARDSIZE)%5]
	    if { $orientation == "0" } { set orientation - }
	    if { $orientation == "1" } { set orientation o }
	    if { $orientation == "2" } { set orientation O }
	    if { $orientation == "3" } { set orientation x }
	    if { $orientation == "4" } { set orientation X }
	    set secondMove [list $newPiece $orientation $value $moveNum]
	    set secondMoves [linsert $secondMoves end $secondMove]
	}
	ShowMovesPart2 $c $moveType 0 0 $position $secondMoves
    } else {

	InitializeMoveArray a v 
	
	foreach item $moveList {
	    ParseMove $item a v
	}
        
	set directionList [list U D L R]
	for { set i 0 } { $i < $BOARDSIZE } { incr i } {
	    foreach dir $directionList {
		set idTag a-$dir$i
		if {$v($idTag) != "noval"} {
		    $c raise $idTag all
		    $c itemconfigure $idTag -fill [MoveValueToColor $moveType $v($idTag)]
		    $c bind $idTag <ButtonRelease-1> "ReturnFromMovePart1 $c $moveType $dir $i $position $a($idTag)"
		}
	    }
	}
	
	update idletasks
    }
}

proc PositionFromMovePart1 { boardName position movedFrom direction startingOrientation } {
    global WIDTH
    upvar board $boardName

    if { $direction == "U" } { set movedTo [expr $movedFrom-$WIDTH] }
    if { $direction == "D" } { set movedTo [expr $movedFrom+$WIDTH] }
    if { $direction == "L" } { set movedTo [expr $movedFrom-1] }
    if { $direction == "R" } { set movedTo [expr $movedFrom+1] }

    if { $startingOrientation == "o" } { 
	set newPiece O 
    } elseif { $startingOrientation == "O" } {
	set newPiece o
    } elseif { $startingOrientation == "x" } {
	set newPiece X
    } else {
	set newPiece x
    }
    
    set board($movedFrom) "-"
    set board($movedTo) $newPiece
    
    set newPosition [Hash board]
    
    return $newPosition
}

proc ReturnFromMovePart1 { c moveType direction movedFrom position args } {
    set board(bob) 0
    
    # the last arguement isn't valid, but it's never used
    GS_HideMoves $c $moveType $position $args
    Unhash $position board

    set secondMoves $args
    set piece $board($movedFrom)

    # Animation code
    AnimateMovePart1 $c $movedFrom $piece $direction

    set newPosition [PositionFromMovePart1 board $position $movedFrom $direction $piece]
    GS_DrawPosition $c $newPosition

    update idletasks
   
    ShowMovesPart2 $c $moveType $direction $movedFrom $position $secondMoves
}

proc ShowMovesPart2 { c moveType direction movedFrom position secondMoves } {

    global movePart1Memory
    set movePart1Memory [list $direction $movedFrom $position $secondMoves]

    foreach move $secondMoves {
	set newPiece [lindex $move 0]
	set orientation [lindex $move 1]
	set value [lindex $move 2]
	set moveNum [lindex $move 3]
	set color [MoveValueToColor $moveType $value] 

	set foreground c[subst $orientation]b-$newPiece
	set background c[subst $orientation]c-$newPiece

	$c raise $background base
	$c raise $foreground $background
	if { $orientation == "o" || $orientation == "O" } {
	    $c itemconfigure $background -fill $color
	} else {
	    $c itemconfigure $background -fill $color
	}
	$c bind $background <ButtonRelease-1> "ReturnFromHumanMove $moveNum"
	$c bind $foreground <ButtonRelease-1> "ReturnFromHumanMove $moveNum"
    }
}


proc InitializeMoveArray { aName vName } {
    upvar $aName a
    upvar $vName v
    global BOARDSIZE

    set directionList [list U D L R]
    for { set i 0 } { $i < $BOARDSIZE } { incr i } {
	foreach direction $directionList {
	    set a(a-$direction$i) [list]
	    set v(a-$direction$i) noval
	}
    }
}

proc ParseMove { move aName vName } {
    upvar $aName a
    upvar $vName v
    global BOARDSIZE

    set moveNum [lindex $move 0]
    set value [lindex $move 1]

    set flippedPiece [expr $moveNum%$BOARDSIZE]
    set direction [expr $moveNum/$BOARDSIZE%4]
    set newPiece [expr $moveNum/($BOARDSIZE*4)%$BOARDSIZE]
    set orientation [expr $moveNum/($BOARDSIZE*4*$BOARDSIZE)%5]

    if { $direction == "0" } { set direction U }
    if { $direction == "1" } { set direction D }
    if { $direction == "2" } { set direction L }
    if { $direction == "3" } { set direction R }

    if { $orientation == "0" } { set orientation - }
    if { $orientation == "1" } { set orientation o }
    if { $orientation == "2" } { set orientation O }
    if { $orientation == "3" } { set orientation x }
    if { $orientation == "4" } { set orientation X }
    
    set idTag a-$direction$flippedPiece 
    
    set secondMove [list $newPiece $orientation $value $moveNum]

    set a($idTag) [linsert $a($idTag) end $secondMove]

    if { $value == "Lose" || $v($idTag) == "Lose" } { set arrowValue "Lose"} elseif { $value == "Tie" || $v($idTag) == "Tie" } { set arrowValue "Tie" } else { set arrowValue "Win" }

    set v($idTag) $arrowValue						     

}


proc MoveValueToColor { moveType value } {
    set color grey50
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



# GS_HideMoves the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    global BOARDSIZE
    set directionList [list U D L R]
    for {set i 0} {$i < $BOARDSIZE} {incr i} {
	foreach dir $directionList {
	    $c lower a-$dir$i base
	}
	$c lower cxb-$i base
	$c lower cxc-$i base
	$c lower cXb-$i base
	$c lower cXc-$i base
	$c lower cob-$i base
	$c lower coc-$i base
	$c lower cOb-$i base
	$c lower cOc-$i base
    }
    update idletasks
}



# GS_HandleUndo draws a move undoing itself sortof.
# the names of the arguments explain themsleves but just to clarify...
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A

# By default this function just calls GS_DrawPosition, but you certainly don't need to keep that.

proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
    global firstHalfAlreadyAnimated
    set firstHalfAlreadyAnimated 0
    GS_DrawPosition $c $positionAfterUndo
}


#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
    
    global preferredWidth
    set size $preferredWidth
    set fontsize [expr int($size / 20)]
    
    # Tell us it's "Game Over!" and announce and winner
    $c create rectangle 0 [expr $size/2 - 50] $size [expr $size/2 + 50] -fill gray -width 1 -outline black -tag "gameover"
    $c create text [expr $size/2] [expr $size/2] -text "Game Over! $nameOfWinner Wins" -font "Arial $fontsize" -fill black -tag "gameover"
	
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

    # Delete "Game Over!" text
    $c delete gameover
 }
