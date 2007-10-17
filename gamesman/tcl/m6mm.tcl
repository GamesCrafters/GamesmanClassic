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
    set kGameName "6 Men's Morris"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ____ WINS"
    set kMisereString "First player to ____ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: (fill in)"
    } else {
	SetToWinString "To Win: (fill in)"
    }
    SetToMoveString "To Move: (fill in)"
    
	############   GLOBAL VARS #######################

	global dotgap dotmid
    global x1 x2 x3 x4 x6 y1 y4 y7 y11 y14
    global pieceSize pieceOutline xColor oColor pieceOffset
    global dotSize dotExpandAmount lineWidth lineColor baseColor base
    global dotx1 dotx2 dotx3 dotx4 dotx5 doty1 doty2 doty3 doty4 doty5
    global px1 px2 px3 px4 px5 py1 py2 py3 py4 py5
    global horizArrows vertArrows slideDelay goDelay animQuality
    global gFrameWidth gFrameHeight
    global boardSize squareSize leftBuffer topBuffer
	global numPositions
	
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "(Fill this in)"
    set kTclAuthors "Daniel Wei, Kevin Liu, Patricia Fong"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
	
	 ## IMPORTANT: These are variables used to change the board.
    # board size
	set numPositions 16
    set boardSize [expr [min $gFrameWidth $gFrameHeight] - 200]
    set squareSize [expr $boardSize / 5]
    set leftBuffer [expr [expr $gFrameWidth - $boardSize] / 2]
    set topBuffer [expr [expr $gFrameHeight - $boardSize] / 2]
    #coordinates:
    set dotgap [expr $boardSize / 5] 
    set dotmid [expr $dotgap / 2]
    set firstXCoord [expr $leftBuffer + $dotmid]
    set firstYCoord [expr $topBuffer + $dotmid]
    ## pieces
    set pieceSize [expr $dotmid - ($dotmid / 4)]
    set pieceOutline [expr $pieceSize / 14]
    set xColor blue4
    set oColor red2
    set pieceOffset $dotgap
    ## dots
    set dotSize [expr $pieceSize / 2.6]
    set dotExpandAmount [expr 3 * ( $dotSize / 4 )]
    ## lines
    set lineWidth [ expr 3 * ( $pieceOutline / 2 ) ]
    set lineColor CadetBlue4
    ## base
    set baseColor white
    ## arrow lists
    set horizArrows {list ac ca ce ec gh hg hi ih kl lk no on qr rq rs sr uw wu wy yw}
    set vertArrows  {list ak ka ku uk gl lg lq ql ch hc rw wr in ni ns sn eo oe oy yo}
    ## animation delay
    set slideDelay 20000
    set goDelay 3000000
    set animQuality "low"
    # x and y position numbers
    set x1 $firstXCoord
    set x2 [expr $x1 + 2 * $dotgap]
    set x3 [expr $x2 + 2 * $dotgap]
    set x4 [expr $x1 + $dotgap]
    set x5 $x2
    set x6 [expr $x2 + $dotgap]
    set x7 $x1
    set x8 $x4
    set x9 $x6
	set x10 $x3
	set x11 $x4
	set x12 $x5
	set x13 $x6
	set x14 $x1
	set x15 $x2
	set x16 $x3
    
    set y1 $firstYCoord
    set y2 $y1
    set y3 $y1
    set y4 [expr $y1 + $dotgap]
    set y5 $y4
    set y6 $y4
    set y7 [expr $y4 + $dotgap]
    set y8 $y7
    set y9 $y7
	set y10 $y7
	set y11 [expr $y7 + $dotgap]
	set y12 $y11
	set y13 $y11
	set y14 [expr $y11 + $dotgap]
	set y15 $y14
	set y16 $y14
        
    set dotx1 [expr $x1 - [expr $dotSize / 2]];
    set dotx2 [expr $x2 - [expr $dotSize / 2]];
    set dotx3 [expr $x3 - [expr $dotSize / 2]];
	set dotx4 [expr $x4 - [expr $dotSize / 2]];
	set dotx5 [expr $x5 - [expr $dotSize / 2]];
    set doty1 [expr $y1 - [expr $dotSize / 2]];
    set doty2 [expr $y4 - [expr $dotSize / 2]];
    set doty3 [expr $y7 - [expr $dotSize / 2]];
    set doty4 [expr $y11 - [expr $dotSize / 2]];
	set doty5 [expr $y14 - [expr $dotSize / 2]];
	
    set px1 [expr $x1 - [expr $pieceSize / 2]];
    set px2 [expr $x2 - [expr $pieceSize / 2]];
    set px3 [expr $x3 - [expr $pieceSize / 2]];
	set px4 [expr $x4 - [expr $pieceSize / 2]];
	set px5 [expr $x5 - [expr $pieceSize / 2]];
    set py1 [expr $y1 - [expr $pieceSize / 2]];
    set py2 [expr $y4 - [expr $pieceSize / 2]];
    set py3 [expr $y7 - [expr $pieceSize / 2]];
	set py4 [expr $y11 - [expr $pieceSize / 2]];
	set py5 [expr $y14 - [expr $pieceSize / 2]];
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

    return [list X O _]
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
    global gMisereGame
    if { $gMisereGame == 0 } {
        set option 2
    } else {
        set option 1
    }
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
    global gMisereGame
    if { $option == 1 } {
        set gMisereGame 1
    } else {
        set gMisereGame 0
    }
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

    # you may want to start by setting the size of the canvas; this line isn't necessary
    #$c configure -width 500 -height 500
	
	global boardSize squareSize leftBuffer topBuffer
    global x1 x2 x3 x4 x6 y1 y4 y7 y11 y14
	global numPositions
	
	set counter 0
	set i 0
	set j 0
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter 
	set i 2
	set j 0
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter 
	set i 4
	set j 0
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter 
	set i 1
	set j 1
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 2
	set j 1
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 3
	set j 1
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 0
	set j 2
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base-$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 1
	set j 2
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 3
	set j 2
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 4
	set j 2
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 1
	set j 3
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 2
	set j 3
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 3
	set j 3
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 0
	set j 4
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 2
	set j 4
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	set i 4
	set j 4
		$c create rect \
	[expr $leftBuffer + [expr $i * $squareSize]] \
	[expr $topBuffer + [expr $j * $squareSize]] \
	[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
	[expr $topBuffer + [expr ($j+1) * $squareSize]] \
	-fill white -tag [list base base$counter]
		$c create oval \
	[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
	[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
	[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
	-fill white -tag [list moveindicators mi-$counter]
		
		$c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		
		$c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove [expr $counter*[expr $numPositions*$numPositions + $numPositions+1]]"
		incr counter
	
		$c create rect \
	[expr [expr $leftBuffer + [expr 0 * $squareSize]] - 40] \
	[expr [expr $topBuffer + [expr 0 * $squareSize]] - 40] \
	[expr [expr $leftBuffer + [expr 5 * $squareSize]] + 40] \
	[expr [expr $topBuffer + [expr 5 * $squareSize]] + 40] \
	-fill white -tag base
		
		# horizontal lines
	set xa $x1
	set ya $y1
	set xb $x3
	set yb $y1
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x4
	set ya $y4
	set xb $x6
	set yb $y4
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x1
	set ya $y7
	set xb $x4
	set yb $y7
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x6
	set ya $y7
	set xb $x3
	set yb $y7
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x4
	set ya $y11
	set xb $x6
	set yb $y11
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x1
	set ya $y14
	set xb $x3
	set yb $y14
	$c create line $xa $ya $xb $yb -tag base -width 5
	
	# vertical lines
	set xa $x1
	set ya $y1
	set xb $x1
	set yb $y14
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x4
	set ya $y4
	set xb $x4
	set yb $y11
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x2
	set ya $y1
	set xb $x2
	set yb $y4
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x2
	set ya $y11
	set xb $x2
	set yb $y14
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x6
	set ya $y4
	set xb $x6
	set yb $y11
	$c create line $xa $ya $xb $yb -tag base -width 5
	set xa $x3
	set ya $y1
	set xb $x3
	set yb $y14
	$c create line $xa $ya $xb $yb -tag base -width 5
	
    MakePieces $c 0
    
    $c raise base
    update idletasks
} 

proc MakePieces { c num } {
  
	MakeBlue $c [expr $num % 5] [expr $num / 5] $num
    MakeRed $c [expr $num % 5] [expr $num / 5] $num
	
    if { $num != 24 } {
        MakePieces $c [expr $num + 1]
    }
}


proc MakeRed { c x y tag } {
    global squareSize leftBuffer topBuffer

    set x [expr $x * $squareSize + $leftBuffer]
    set y [expr $y * $squareSize + $topBuffer]

    $c create oval [expr $x  + 20] [expr $y + 20] [expr $x + [expr $squareSize - 20]] [expr $y + [expr $squareSize - 20]] -fill red -tag X-$tag
    $c lower X-$tag base

}

proc MakeBlue { c x y tag } {
    global squareSize leftBuffer topBuffer

    set x [expr $x * $squareSize + $leftBuffer]
    set y [expr $y * $squareSize + $topBuffer]
	
    $c create oval [expr $x  + 20] [expr $y + 20] [expr $x + [expr $squareSize - 20]] [expr $y + [expr $squareSize - 20]] -fill blue -tag O-$tag
    $c lower O-$tag base

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
    global dotmid leftBuffer topBuffer
    #$c raise base
    DrawPieces $c $position

}

proc DrawPieces {c position } {
    global numPositions
	
	$c raise base
	
	set a(0) 0
    
    UnHashBoard $position a
	
    for {set i 0} {$i < $numPositions} {incr i} {   
        if {$a($i) == "X"} {
            $c raise X-$i base
        } elseif {$a($i) == "O"} {
            $c raise O-$i base
        } 
    }
	update idletasks
}

proc UnHashBoard {position arrayname} {
    global numPositions

    upvar $arrayname a
    
    set board [C_CustomUnhash $position]
    
    for {set i 0} {$i < $numPositions} {incr i} {
        if {[string equal [string index $board $i] "X"]} {   
            set a($i) X
        } elseif {[string equal [string index $board $i] "O"]} {
            set a($i) O
        } else {
            set a($i) _
        }
    }
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

	GS_DrawPosition $c $position
	#$c raise o1 base
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    global numPositions
    
	set turn [C_ReturnTurn $position]
	
	if {$turn == "X"} {
		set val X
	} else {
		set val O
	}
	
    return $val
    
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

	global numPositions
	
	set theMoveTo [expr [expr $theMove / $numPositions] % $numPositions]
	
	switch $theMoveTo {
	
	1 {
		set theMoveTo 2
	}
	2 {
		set theMoveTo 4
	}
	3 {
		set theMoveTo 6
	}
	4 {
		set theMoveTo 7
	}
	5 {
		set theMoveTo 8
	}
	6 {
		set theMoveTo 10
	}
	7 {
		set theMoveTo 11
	}
	8 {
		set theMoveTo 13
	}
	9 {
		set theMoveTo 14
	}
	10 {
		set theMoveTo 16
	}
	11 {
		set theMoveTo 17
	}
	12 {
		set theMoveTo 18
	}
	13 {
		set theMoveTo 20
	}
	14 {
		set theMoveTo 22
	}
	15 {
		set theMoveTo 24
	}
	default {
		set theMoveTo $theMoveTo
	}
	
	
	}
   
   #C_CustomUnhash [$theMoveTo]
    set piece X
	
    if { [GS_WhoseMove $oldPosition] == "O"} {
        set piece O

	$c raise $piece-$theMoveTo base
	update idletasks
    }
    
       

	if { [GS_WhoseMove $oldPosition] == "X"} {
		$c raise $piece-$theMoveTo base
		update idletasks
	}

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
	
	global numPositions
	# ONLY FIRST STAGE
	#C_CustomUnhash [$moveList]
	
	foreach item $moveList {
        set move [lindex $item 0]
		
        set value [lindex $item 1]
        set color cyan
        
		set move [expr [expr $move / $numPositions] % $numPositions]
        $c raise mi-$move base
        
        if {$moveType == "value"} {
            if {$value == "Tie"} {
                set color yellow
            } elseif {$value == "Lose"} {
                set color green
            } else {
                set color red4
            }
        }
        $c itemconfigure mi-$move -fill $color
		
		#update idletasks
    }
    update idletasks
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
    $c lower moveindicators base

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
    #GS_DrawPosition $c $positionAfterUndo
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

	### TODO if needed
	
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
