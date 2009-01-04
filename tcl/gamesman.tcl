#############################################################################
##
## NAME:         gamesman.tcl
##
## DESCRIPTION:  The source code and heart of a Master's project
##               entitled GAMESMAN, which is a two-person, finite
##               perfect-information game generator.
##
## AUTHOR:       Dan Garcia  -  University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         06-23-93
##
## UPDATE HIST:
##
## 06-24-93 1.0b01 : Implemented the undo facility, PrintPosition()
## 02-28-95 1.0b02 : Added color cursors, fixed the tcl ending strings
## 03-01-95 1.0b03 : Changed the 'bad' color to magenta
## 03-03-95 1.0b04 : Started baloon Help, fixed Value moves
## 05-02-95 1.0b05 : Fixed balloon help
## 05-03-95 1.0b06 : Set up balloon help for almost everything
## 05-06-95 1.0b07 : Documented code like crazy
## 05-15-95 1.0    : Final release code for M.S.
## 96-07-04 1.01   : Color About window and fixed 7.5 bugs, like having to
##                   add field for -value for radiobuttons
## 97-03-25 1.02   : Added Remoteness
##
## TODO:
##
## Intelligent Help to know if a radiobutton is selected
##
#############################################################################

global command_line_args
set command_line_args [concat $argv0 $argv]

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
    set gAnimationSpeed 8

    ##by AP
    global gMoveDelay
    set gMoveDelay 1

    global gGameDelay
    set gGameDelay 1
}

#############################################################################
##
## InitWindow
##
## Here we initialize the window. Set up fonts, colors, widgets, helps, etc.
##
#############################################################################

proc InitWindow {} {

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

    #by AP
    global gLeftPlayerType
    set gLeftPlayerType 0

    global gRightPlayerType
    set gRightPlayerType 1

    global gInitialPosition

    global oldVarObjective
    set oldVarObjective -1

    global oldGSOptions
    set oldGSOptions -1

    ### Set the xbm file globals

    ### Recall that kRootDir was set for us by the module
    global kRootDir

    global gifDanGarcia
    set gifDanGarcia "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
    
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

    global gDestroyingBoard
    set gDestroyingBoard 0

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
    
    #by ap
    label .f2.labGame \
	-font $kLabelFont \
	-text "Game:" \
	-foreground $kLabelColor
    
    button .f2.butSolve \
	-text "Solve" \
	-font $kLabelFont \
	-command DoStart
    
    button .f2.butPlay \
	-text "Play" \
	-font $kLabelFont \
	-command DoPlay \
	-state disabled
    
    ### f1 = The Setup frame (Setup: About,Start,Rules,Help,Quit)
    
    frame .f1 \
	-borderwidth 2 \
	-relief raised
    
    label .f1.labSetup \
	-font $kLabelFont \
	-text "Setup:" \
	-foreground $kLabelColor
    
    #by AP
    button .f1.butAbout \
	-text "About" \
	-font $kLabelFont \
	-command DoAbout

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
    
    .f3.entPlayer1 insert 0 "Player"
    .f3.entPlayer2 insert 0 "The computer"

    #by AP
    frame .f6 \
	-borderwidth 2 \
	-relief raised

    label .f6.labType \
	-font $kLabelFont \
	-text "Type:" \
	-foreground $kLabelColor

    frame .f6.subf0 \
	-borderwidth 0 \
	-relief flat

    label .f6.subf0.labEmpty \
	-text "         "


    radiobutton .f6.subf0.leftHuman \
	    -text "Human" \
	    -font $kLabelFont \
	    -variable gLeftPlayerType \
	    -value 0 \
	    -anchor w \
	    -command SetPlayVars
    
    radiobutton .f6.subf0.leftComputer \
	    -text "Computer" \
	    -font $kLabelFont \
	    -variable gLeftPlayerType \
	    -value 1 \
	    -anchor w \
	    -command SetPlayVars

    frame .f6.subf1 \
	-borderwidth 0 \
	-relief flat

    label .f6.subf1.labEmpty \
	-text "         "

    radiobutton .f6.subf1.rightHuman \
	    -text "Human" \
	    -font $kLabelFont \
	    -variable gRightPlayerType \
	    -value 0 \
	    -anchor w \
	    -command SetPlayVars
    
    radiobutton .f6.subf1.rightComputer \
	    -text "Computer" \
	    -font $kLabelFont \
	    -variable gRightPlayerType \
	    -value 1 \
	    -anchor w \
	    -command SetPlayVars

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
    
    #by AP
    pack append .f1 \
	    .f1.labSetup   {left } \
	    .f1.butAbout   {left pady 10 expand fill } \
	    .f1.butHelp    {left pady 10 expand fill } \
	    .f1.butQuit    {left pady 10 expand fill } 

    ##by AP
    pack append .f2 \
	    .f2.labGame      {left} \
	    .f2.butSolve     {left pady 10 padx 10 expand fill } \
	    .f2.butPlay {left pady 10 padx 10 expand fill }

    pack append .f3 \
	    .f3.labName1   {left fill} \
	    .f3.cl         {left} \
	    .f3.entPlayer1 {left fill expand} \
	    .f3.labName2   {left fill} \
	    .f3.cr         {left} \
	    .f3.entPlayer2 {left fill expand}

    #by AP

    pack append .f6.subf0 \
	    .f6.subf0.labEmpty {left fill} \
	    .f6.subf0.leftHuman {top fill expand} \
	    .f6.subf0.leftComputer {top fill expand}

    pack append .f6.subf1 \
	    .f6.subf1.labEmpty {left fill} \
	    .f6.subf1.rightHuman {top fill} \
	    .f6.subf1.rightComputer {top fill expand}

    pack append .f6 \
	    .f6.labType   {left} \
	    .f6.subf0     {left expand fill} \
	    .f6.subf1     {left expand fill}
    
    pack append . \
	    .f0 {top fill} \
	    .f3 {top expand fill} \
	    .f6 {top expand fill} \
	    .f2 {top expand fill} \
	    .f5 {top expand fill} \
	    .f1 {top expand fill}
    
    ### This is completely a hack, but it looks good. When I change my font
    ### I'll have to find the right way to do this. Why do I keep saying that?

    #by AP
    set theWidth 7
    foreach i { \
	    .f0.labStatus \
	    .f5.labModify \
	    .f1.labSetup \
	    .f2.labGame \
	    .f3.labName1 \
	    .f6.labType \
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

#by ap - changed a buncha help strings below

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

	    SetupHelp .f2.butSolve \
		    "Solve the game" \
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
	    
	    SetupHelp .f2.butPlay \
		    "Play Game" \
		    "disabled button" \
		    "Click on this button to play $kGameName against a human. Left goes first. This is currently disabled because you need to click the 'start' button first to solve the game."

	}   

	justClickedStart {

	    ### Here we disable start,rules & enable human, computer1 & 2.

	    SetupHelp .f2.butPlay \
		    "Play Game" \
		    "button" \
		    "Click on this button to play $kGameName against a human. Left goes first."
	    SetupHelp .f2.butSolve \
		    "Solve the game" \
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
	    SetupHelp .winAbout.f0.labDanPhoto \
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
    global oldVarObjective oldGSOptions varObjective
    global command_line_args

    C_Initialize $command_line_args
    C_InitializeDatabases

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

    #by AP
    .f2.butSolve config -state disabled
    
    #.f5.butOptions config -state disabled
    #.f5.butInitialPosition config -state disabled
    
    .f0.mesStatus config -text "Solved. The Game is a $theValue.\nChoose a Play option."

    ### Once we've built the database, you can't change the rules!

    if [winfo exists .gameSpecificOptions] { 
	destroy .gameSpecificOptions 
    } 

    #added by AP
    set oldVarObjective $varObjective
    set oldGSOptions eval[GS_GetGameSpecificOptions]
}

#############################################################################
##
## DisablePlayButtons
##
## Turn off the play buttons - only one game at a time!
##
#############################################################################

proc DisablePlayButtons {} {
    #by AP
    .f2.butPlay config -state disabled
}

#############################################################################
##
## EnablePlayButtons
##
## Turn on the play buttons - no game is being played so play on!
##
#############################################################################

proc EnablePlayButtons {} {
    global kEditInitialPosition

    .f2.butPlay config -state normal

    # added by AP
    .f5.butOptions config -state normal
    
    if { $kEditInitialPosition } {
	.f5.butInitialPosition config -state normal
    }
}

#############################################################################
##
## DisableEditButtons
##
## Turn off the edit rules and starting position buttons
##
#############################################################################

proc DisableEditButtons {} {
    .f5.butOptions config -state disabled
    .f5.butInitialPosition config -state disabled
}

#############################################################################
##
## EnableEditButtons
##
## Turn on the edit rules and starting position buttons
##
#############################################################################

proc EnableEditButtons {} {
    global kEditInitialPosition

    .f5.butOptions config -state normal
    
    if { $kEditInitialPosition } {
	.f5.butInitialPosition config -state normal
    }
}

#############################################################################
##
## DoPlay  (by AP)
##
## The Play buttom was clicked
##
#############################################################################

proc DoPlay {} {
    global gLeftPlayerType
    global gRightPlayerType
    global gInitialPosition 
    global gPosition 
    global gHumanGoesFirst
    global gPlayerOneTurn
    global gMoveDelay

    ##set the appropriate vars
    SetPlayVars

    set gPosition $gInitialPosition
    set gPlayerOneTurn 1

    #Create Board
    DoBoard

    #disable edit buttons
    DisableEditButtons

    GS_NewGame .winBoard.c

    ### Make the computers move if nec., update 
    ### the predictions and the turn.

    while { [winfo exists .winBoard.c] && [C_Primitive $gPosition] == "Undecided" && (($gPlayerOneTurn && $gLeftPlayerType) || (!$gPlayerOneTurn && $gRightPlayerType)) } {
	DoComputersMove
	update
	after [expr int($gMoveDelay * 1000)]
    }
    
    if { $gPosition == $gInitialPosition && (!$gLeftPlayerType || !$gRightPlayerType) } {
	HandleMoves
	HandlePredictions
	HandleTurn
    }
}

#############################################################################
##
## SetPlayVars  (by AP)
##
## Set the Play variables (on new game, or change of player types)
##
#############################################################################

proc SetPlayVars {} {
    global gAgainstComputer
    global gHumanGoesFirst
    global gPlayerOneTurn
    global gPosition gInitialPosition
    global gLeftPlayerType gRightPlayerType
    global gMoveDelay

    if { $gLeftPlayerType && $gRightPlayerType } {
	.f0.mesStatus config -text "Playing computer vs\ncomputer (perfect opponents)"
	set gAgainstComputer 1
	set gHumanGoesFirst 0
    } elseif { !$gLeftPlayerType && !$gRightPlayerType } {
	set gAgainstComputer 0
	set gHumanGoesFirst 1
	.f0.mesStatus config -text "Playing against \na human"
    } else {
	set gAgainstComputer 1
	.f0.mesStatus config -text "Playing against a\nperfect computer opponent"
	if { !$gLeftPlayerType } {
	    set gHumanGoesFirst 1
	} else {
	    set gHumanGoesFirst 0
	}
    }

    ## if game in progress and on current player's turn his type is changed to computer
    while { [winfo exists .winBoard.c] && [C_Primitive $gPosition] == "Undecided" && (($gPlayerOneTurn && $gLeftPlayerType) || (!$gPlayerOneTurn && $gRightPlayerType)) } {
	DoComputersMove
	update
	after [expr int($gMoveDelay * 1000)]
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
    global gifDanGarcia kLabelFont kGameName kCAuthors kTclAuthors kGifAuthors

    toplevel .winAbout
    wm title .winAbout "About GAMESMAN and its author"
    
    ### Disable the last window

    .f1.butAbout config \
        -state disabled
    
    ### Put the messages in

    message .winAbout.msgGAMESMAN \
	-font { Helvetica 18 bold } \
	-width 600 \
        -relief raised \
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

    frame .winAbout.f0 \
	    -borderwidth 1 \
	    -relief raised

    frame .winAbout.f1 \
	    -borderwidth 1 \
	    -relief raised

    message .winAbout.f0.msgArchitecture \
        -font { Helvetica 14 bold } \
        -relief raised \
	-width 320 \
        -borderwidth 1 \
        -justify center \
        -text "Architecture"

    message .winAbout.f0.msgWritten \
        -font { Helvetica 14 } \
        -relief raised \
	-width 300 \
        -borderwidth 1 \
        -anchor w \
        -text "Written by Dan Garcia (ddgarcia@cs.berkeley.edu)"    

     message .winAbout.f0.msgModified \
        -font { Helvetica 14 } \
        -relief raised \
	-width 300 \
        -borderwidth 1 \
        -anchor w \
        -text "Modified by Sunil Ramesh, Alex Perelman, Atilla Gyulassy" 

    message .winAbout.f1.msgModule \
        -font { Helvetica 14 bold } \
        -relief raised \
	-width 320 \
        -borderwidth 1 \
        -justify center \
        -text "Module: $kGameName"
	    

    message .winAbout.f1.msgCCode \
        -font { Helvetica 14 } \
        -relief raised \
	-width 300 \
        -borderwidth 1 \
        -anchor w \
        -text "C Code by: $kCAuthors"  

    message .winAbout.f1.msgTclCode \
        -font { Helvetica 14 } \
        -relief raised \
	-width 300 \
        -borderwidth 1 \
        -anchor w \
        -text "Tcl Code by: $kTclAuthors\n "  

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

    label .winAbout.f0.labDanPhoto \
	-image imageDanGarcia \
        -relief raised \
        -background lightgrey \
        -borderwidth 2

    ### Module team's icon

    catch { image delete imageModuleTeam }
    image create photo imageModuleTeam -file $kGifAuthors

    label .winAbout.f1.labModulePhoto \
	-image imageModuleTeam \
        -relief raised \
        -background lightgrey \
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

    ### Pack the frames (by AP)
    pack append .winAbout.f0 \
         .winAbout.f0.msgArchitecture {top fill}

    pack append .winAbout.f0 \
	 .winAbout.f0.msgWritten {top expand fill}

    pack append .winAbout.f0 \
	 .winAbout.f0.msgModified {top expand fill} 

    pack append .winAbout.f0 \
	 .winAbout.f0.labDanPhoto {top expand fill} 

    pack append .winAbout.f1 \
         .winAbout.f1.msgModule {top fill expand}

    pack append .winAbout.f1 \
	 .winAbout.f1.msgCCode {top fill expand}

    pack append .winAbout.f1 \
	 .winAbout.f1.msgTclCode {top fill expand}

    pack append .winAbout.f1 \
	 .winAbout.f1.labModulePhoto {top expand fill} 

    ### Pack the about box
    
    pack append .winAbout \
         .winAbout.msgGAMESMAN {top fill expand} \
         .winAbout.msgAboutwww   {top fill expand} \
	 .winAbout.butQuit {bottom expand fill} \
	 .winAbout.f0 {left fill expand} \
	 .winAbout.f1 {right fill expand}
	
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
    global varObjective oldVarObjective oldGSOptions
    
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
		-text "$kMisereString (misere)"  \
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
	
	# added by AP - grey out play buttons if options have been changed
	set newGSOptions eval[GS_GetGameSpecificOptions]
	if { $oldVarObjective != $varObjective || $oldGSOptions != $newGSOptions } {
	    DisablePlayButtons
	    .f2.butSolve config -state normal
	} elseif { $oldVarObjective == $varObjective && $oldGSOptions == $newGSOptions } {
	    EnablePlayButtons
	    .f2.butSolve config -state disabled
	}
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
    global gMoveDelay gGameDelay
    
    ### Since we can only play one game at a time...

    DisablePlayButtons

    ### The control window - create it, name it and place it.

    toplevel .winBoardControl
    bind .winBoardControl <Destroy> DestroyBoard
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

    #by AP
    frame .winBoardControl.f7 \
        -borderwidth 2 \
        -relief raised
    frame .winBoardControl.f8 \
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
	-width 550
    
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
	-width 550
    
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

    ## by AP
    label .winBoardControl.f7.labDelay \
        -font $kLabelFont \
        -text "Move Delay:" -width 6\
        -foreground $kLabelColor

    

    scale .winBoardControl.f7.sclDelay \
	-from 0 \
	-to 2 \
	-orient horizontal \
	-length 10c \
	-resolution .1 \
	-variable gMoveDelay
    
    pack append .winBoardControl.f7 \
	    .winBoardControl.f7.labDelay {left} \
	    .winBoardControl.f7.sclDelay {left expand fill}


    label .winBoardControl.f8.labDelay\
        -font $kLabelFont \
        -text "Game Delay:" -width 6\
        -foreground $kLabelColor

    scale .winBoardControl.f8.sclDelay \
	-from 0 \
	-to 2 \
	-orient horizontal \
	-length 10c \
	-resolution .1 \
	-variable gGameDelay

    pack append .winBoardControl.f8 \
	    .winBoardControl.f8.labDelay {left} \
	    .winBoardControl.f8.sclDelay {left expand fill}
    
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
		DestroyBoard
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
	set gPlayerOneTurn 1
	
	### Call the Game-specific New Game command
	DeleteMoves
	GS_NewGame $w

	.winBoardControl.f1.validMoves config -state normal
	.winBoardControl.f1.valueMoves config -state normal

	global varMoves
	if { $varMoves == "validMoves" } {
	    ShowMoves
	} elseif { $varMoves == "valueMoves" } {
	    ShowValueMoves
	} else {
	    DeleteMoves
	}
	
	### Remove all Dead and Alive tags and reset them to what they were
	### when we reset the game.

	$w dtag tagDead
	$w dtag tagAlive
	$w addtag tagDead withtag tagNotInitial
	$w addtag tagAlive withtag tagInitial

	#HandleMoves
	#HandlePredictions
	#HandleTurn

	SetPlayVars
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
             
    ## Broke this up to allow On-The-Fly GUI options -AP
    pack append .winBoardControl \
	    .winBoardControl.f0 {top expand fill} \
	    .winBoardControl.f6 {top expand fill} \
	    .winBoardControl.f5 {top expand fill} \
	    .winBoardControl.f4 {top expand fill} \
	    .winBoardControl.f1 {top expand fill} \
	    .winBoardControl.f7 {top expand fill} \
	    .winBoardControl.f8 {top expand fill}

    # add game-specific GUI options
    GS_AddGameSpecificGUIOnTheFlyOptions .winBoardControl
	    
    # add the "new game", "abort" buttons
    pack append .winBoardControl \
	    .winBoardControl.f2 {top fill expand}

    ### This is completely a hack, but it looks good. When I change my font
    ### I'll have to find the right way to do this. Why do I keep saying that?

    foreach i { \
	    .winBoardControl.f0.labToMove \
	    .winBoardControl.f6.labToWin \
	    .winBoardControl.f5.labTurn \
	    .winBoardControl.f4.labPredictions \
	    .winBoardControl.f1.labShow \
	    .winBoardControl.f7.labDelay \
	    .winBoardControl.f8.labDelay \
	} {
	$i config -width 12 -anchor e
    }

    ### Set up the help strings

    SetupHelpStringsForWindow BoardWindow

    ### Update the size of the window so that I can place things around it.

    update

    ### Now that I've packed and updated .winBoardControl, I can pack and 
    ### place the board below it.

    ### The board window

    toplevel .winBoard
    bind .winBoard <Destroy> DestroyBoard
    wm title .winBoard "GAMESMAN $kGameName board"
    wm geometry .winBoard [GeometryBelow .winBoardControl]

    ### The Canvas on which to draw the board
    ### There's some assumption that full orthogonal boards will be used -
    ### this needs to be modified for other boards.

    canvas .winBoard.c \
        -width  [expr $gSlotsX*$gSlotSize] \
        -height [expr $gSlotsY*$gSlotSize] \
        -relief raised

    for {set i 0} {$i < $gSlotsX} {incr i} { 
        for {set j 0} {$j < $gSlotsY} {incr j} { 
            CreateSlot .winBoard.c $i $j  
	} 
    }

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

    global varMoves

    if { $varMoves == "noMoves" && [winfo exists .winValueMoves] } {
	destroy .winValueMoves
    }
	
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

    foreach theMoveValue [C_GetValueMoves $gPosition 0] {
	set theMove [lindex $theMoveValue 0]
        DrawMove .winBoard.c $theMove cyan $kBigPiece
    } 
    
    if { [winfo exists .winValueMoves] } {
	destroy .winValueMoves
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

    foreach theMoveValue [C_GetValueMoves $gPosition 0] {
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

proc CreateSlot {w slotX slotY} {
    global gSlotSize gSlotList

    set slot [$w create rect \
        [expr $gSlotSize*$slotX] \
        [expr $gSlotSize*$slotY] \
        [expr $gSlotSize*[expr $slotX+1]] \
        [expr $gSlotSize*[expr $slotY+1]] \
        -outline black \
        -width 1 \
        -fill grey \
        -tag tagSlot]

    ### Remember what the id of the slot was in our array

    set gSlotList($slotX,$slotY) $slot

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
    } elseif { $kInteractionType == "Custom" } {
	DrawMoveCustom $w $theMoveArg $color $drawBig
    } else {
	BadElse "DrawMove ($kInteractionType)"
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

proc DrawCircle { w slotX slotY theTag theColor drawBig } {
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
		       -fill $theColor \
		       -tag $theTag]

    if { $drawBig } {
	$w move $theCircle $cornerX $cornerY
	$w addtag tagPieceCoord$slotX$slotY withtag $theCircle
	$w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle
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
	$w bind tagAlive <Leave> "$w itemconf current -fill grey "
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
    global kInteractionType
    global gRightPlayerType gLeftPlayerType
    global gMoveDelay gGameDelay
    set w .winBoard.c

    ## if on current player's turn it was switched from computer to human
    ## or if board was closed during the pause between move/game, then we 
    ## need to kill the recursion stack
    if { ($gPlayerOneTurn && !$gLeftPlayerType) || (!$gPlayerOneTurn && !$gRightPlayerType) || ![winfo exist .winBoard.c] } {
	#puts "kill DoCompMove"
	return
    }

    ### Set the piece for the computer and get the move.
    #puts "Doing Comp Move: Turn=$gPlayerOneTurn Position=$gPosition"

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
    } elseif { $kInteractionType == "Custom" } {
	set theSlot dummyArgNeverSeen
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

    ### Update the standard fields (Predictions, Turns & Moves)

    HandlePredictions
    HandleTurn
    HandleMoves

    ### If the game is over (the primitive value is Undecided), handle it

   if { [C_Primitive $gPosition] != "Undecided" } { 
	HandleGameOver $w $theValue
    }

    #by AP
    #while { ((($gPlayerOneTurn && $gLeftPlayerType) || (!$gPlayerOneTurn && $gRightPlayerType)) 
    #      && [C_Primitive $gPosition] == "Undecided") } {
#	update
	#after [expr int($gMoveDelay * 1000)]
	#DoComputersMove
    #}
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

    } elseif { $kInteractionType == "Custom" } {
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

    } elseif { $kInteractionType == "Custom" } {
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
    global gLeftPlayerType gRightPlayerType
    global gGameDelay
    global gPosition gInitialPosition
    global gAgainstComputer gHumanGoesFirst

    ### Update the status box.
    
    .f0.mesStatus config \
	    -text "[.f3.entPlayer[expr !$gPlayerOneTurn + 1] get] [PastTenseValue $theValue] [.f3.entPlayer[expr $gPlayerOneTurn + 1] get]\nClick 'New Game' for more!"
    
    ### Make all the other slots inactive
    
    $w addtag tagDead withtag tagAlive
    $w dtag tagAlive

    ##by AP - play a new game if comp vs comp
    if { ($gLeftPlayerType && $gRightPlayerType) } {
	update
	after [expr int($gGameDelay * 1000)]
	
	### Make the code a bit easier to read
	
	set w .winBoard.c
	
	### Clear the message field

	.f0.mesStatus config -text "Starting a new game\n"

	### Reset the position to the initial position

	set gPosition $gInitialPosition
	set gPlayerOneTurn 1
	set gAgainstTheComputer 1
	set gHumanGoesFirst 0

	### Call the Game-specific New Game command
	
	DeleteMoves
	
	GS_NewGame $w
	
	global varMoves
	if { $varMoves == "validMoves" } {
	    ShowMoves
	} elseif { $varMoves == "valueMoves" } {
	    ShowValueMoves
	}

	### Remove all Dead and Alive tags and reset them to what they were
	### when we reset the game.

	$w dtag tagDead
	$w dtag tagAlive
	$w addtag tagDead withtag tagNotInitial
	$w addtag tagAlive withtag tagInitial

	#HandleMoves
	#HandlePredictions
	#HandleTurn
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
	return [$kLeftDrawProc  .winBoard.c $slotX $slotY tagPiece blue $kBigPiece]
    } elseif { $thePiece == "O" } { 
	return [$kRightDrawProc .winBoard.c $slotX $slotY tagPiece red $kBigPiece]
    } elseif { $thePiece == "+" } { 
	return [$kBothDrawProc .winBoard.c $slotX $slotY tagPiece magenta $kBigPiece]
    } elseif { $thePiece == "-" } { 
	return [DrawLastMove .winBoard.c $slotX $slotY tagPiece magenta $kBigPiece]
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
	    if { $theValue == "Tie" && $theRemoteness == 255 } {
		SetPredictionField "$thePlayer $thePrediction Draw"
	    } else {
		SetPredictionField "$thePlayer $thePrediction $theValue in $theRemoteness $theMex"
	    }
	} else {
	    SetPredictionField "Disabled"
	}
    }
}

#############################################################################
##
## DestroyBoard
##
## Destroy the board and the control window
##
#############################################################################

proc DestroyBoard {} { 
    global gDestroyingBoard

    if { $gDestroyingBoard == 1 } {
	return
    }

    set gDestroyingBoard 1

    EnablePlayButtons
    
    #added by AP
    EnableEditButtons 
    
    if { [winfo exists .winBoardControl] } {
	destroy .winBoardControl
    }
    if { [winfo exists .winBoard] } {
	destroy .winBoard
    }
    if { [winfo exists .winValueMoves] } {
	destroy .winValueMoves
    }
    .f0.mesStatus config -text "Choose a Play Option\n"
    
    set gDestroyingBoard 0
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

## Initialization script

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
}

### This makes it feel a little like C

main
