
# 
#  the actions to be performed when the toolbar buttons are pressed
#

proc TBaction1 {} {

    .middle.f1.cMLeft raise detVal
    
    # solve the game, 
    global kGameName varObjective gPosition gInitialPosition
    global gGameSolved gGamePlayable
    
    # If the play options pane is open, ignore
    if { $gGameSolved == "true" && $gGamePlayable == "false" } {
	# do nothing
	return
    }

    # Send initial game-specific options to C procs.
    if { $gGameSolved == "false"} {
	.middle.f1.cMLeft raise iIMB
	.middle.f3.cMRight raise play
	.cStatus lower base
	pack forget .middle.f2.fPlayOptions.fBot
	.cToolbar raise iITB
	SetupPlayOptions

	global gLeftName gRightName
	.middle.f1.cMLeft itemconfig LeftName \
	    -text [format "Left:\n%s" $gLeftName]
	.middle.f3.cMRight itemconfig RightName \
	    -text [format "Right:\n%s" $gRightName]
	update
    } else {
	set gGamePlayable false
	.middle.f3.cMRight lower play
	.middle.f1.cMLeft lower detVal
	.cToolbar raise iATB
	.cStatus lower base
	update idletasks
	set gGamePlayable true
	NewGame
	
	.cToolbar bind iOTB1 <Any-Leave> \
		".cToolbar raise iATB1"
    }
}

# Rules button
proc TBaction2 {} {
    .cToolbar raise iITB
    global gWaitingForHuman
    set gWaitingForHuman true
    pack forget .middle.f2.cMain
    global gOldRules
    set gOldRules [GS_GetOption]
    pack .middle.f2.fRules -side bottom -fill both -expand 1
}

# Help button
proc TBaction3 {} {
    .cToolbar raise iITB
    global gWaitingForHuman
    set gWaitingForHuman true
    pack forget .middle.f2.cMain   
    pack .middle.f2.fHelp -side bottom
}

# About button
proc TBaction4 {} {
    .cToolbar raise iITB
    global gWaitingForHuman
    set gWaitingForHuman true
    pack forget .middle.f2.cMain
    pack .middle.f2.fAbout -side bottom -fill both -expand 1
}

# Skins
proc TBaction5 {} {
	.cToolbar raise iITB
	global gWaitingForHuman
	set gWaitingForHuman true
	pack forget .middle.f2.cMain   
	pack .middle.f2.fSkins -side bottom
}

# Unmapped
proc TBaction6 {} {

}

# Unmapped
proc TBaction7 {} {
}

proc SetupPlayOptions {} {

    global gWaitingForHuman
    set gWaitingForHuman true

    global gMoveType gPosition

    GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]

    pack forget .middle.f2.cMain   
    pack .middle.f2.fPlayOptions -side bottom
    .cStatus raise base
}

proc InitWindow { kRootDir kDir kExt } {


    global gWindowWidth gWindowHeight
    global gFrameWidth
    global gSkinsRootDir
    global gSkinsDir
    global gSkinsExt
    global gGamePlayable
    global kLabelFont kPlayerLabelFont kToMoveToWinFont
    global tcl_platform
    global gPredString gWhoseTurn
    global gLeftName gRightName
    global gLeftColor gRightColor
    # jesse: move delay and game delay added fall '03
    global gMoveDelay gGameDelay
    global gReallyUnsolved

    #
    # Initialize the constants
    #
    set gReallyUnsolved false
    set gMoveDelay 1
    set gGameDelay 1
    set gWhoseTurn "Jesse"
    set gPredString ""
    set gWindowHeight 600
    set gWindowWidth 800
    set gFrameWidth [expr $gWindowWidth * 10 / 16]
    wm aspect . 800 600 1600 1200
    set gGamePlayable false
    set gSkinsRootDir "$kRootDir/../tcl/skins/"
    set gSkinsDir "$kDir/"
    set gSkinsExt "$kExt"
    if { $tcl_platform(platform) == "macintosh" || \
         $tcl_platform(platform) == "windows" } {
        console hide
    }
    SetupGamePieces

    #
    # create the Toolbar - this is out of order so that we can automatically create a lot of images
    #
    
    canvas .cToolbar -highlightthickness 0 \
	-bd 0 \
	-width $gWindowWidth \
	-height [expr $gWindowHeight / 30] \
	-background green

	InitButtons $gSkinsRootDir $gSkinsDir $gSkinsExt


    #
    # Main Area
    #

    # create the frames needed to divide up the middle into 3 parts
    frame .middle

    # set the size of the frames and force them to stay that way
    frame .middle.f1 \
	-width [expr $gWindowWidth * 3 / 16] \
	-height [expr $gWindowHeight * 25 / 30]
    frame .middle.f2 \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 25 / 30] 
    frame .middle.f3 \
	-width [expr $gWindowWidth * 3 / 16] \
	-height [expr $gWindowHeight * 25 / 30]

    pack propagate .middle.f1 0
    pack propagate .middle.f2 0
    pack propagate .middle.f3 0

    # set up the necessary canvases in each frame
    canvas .middle.f1.cMLeft -highlightthickness 0 \
	-bd 0 \
	-width [expr $gWindowWidth * 3 / 16] \
	-height [expr $gWindowHeight * 25 / 30] \
	-background black
    
    canvas .middle.f2.cMain -highlightthickness 0 \
	-bd 0 \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 25 / 30] \
	-background white
    
    # 
    # PLAY OPTIONS FRAME
    #
	    
    frame .middle.f2.fPlayOptions \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 25 / 30]
    pack propagate .middle.f2.fPlayOptions 0
    frame .middle.f2.fPlayOptions.fBot \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 2 / 30]
    pack propagate .middle.f2.fPlayOptions.fBot 0

    # this is not packed now <- you cannot cancel
    button .middle.f2.fPlayOptions.fBot.bCancel -text "Cancel" \
	-command {
            .cToolbar bind iOTB2 <Any-Leave> \
		    ".cToolbar raise iATB2"
	    pack forget .middle.f2.fPlayOptions   
	    pack .middle.f2.cMain
	    .cToolbar raise iATB
            .cStatus lower base
	    update
	}
    # clicking OK in the in game play options
    button .middle.f2.fPlayOptions.fBot.bOk -text "OK" \
	-command {
            .cToolbar bind iOTB2 <Any-Leave> \
		    ".cToolbar raise iATB2"
	    pack forget .middle.f2.fPlayOptions   
	    pack .middle.f2.cMain
            .cStatus lower base
	    .cToolbar raise iATB
	    global gSmartness gSmartnessScale
	    C_SetSmarterComputer $gSmartness $gSmartnessScale
            global gLeftName gRightName
            .middle.f1.cMLeft itemconfigure LeftName \
		    -text [format "Left:\n%s" $gLeftName]
            .middle.f3.cMRight itemconfigure RightName \
		    -text [format "Right:\n%s" $gRightName]
	    update
	    if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" } {
		if { $gReallyUnsolved == true } {
		    . config -cursor watch
		    set theValue [C_DetermineValue $gPosition]
		    set gGameSolved true
		    . config -cursor {}
		    set gReallyUnsolved false
		}
	    }
	    DriverLoop
        }
    frame .middle.f2.fPlayOptions.fMid \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 8 / 30] \
	-bd 2
    pack propagate .middle.f2.fPlayOptions.fMid 0
    frame .middle.f2.fPlayOptions.fTop \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 15 / 30] \
	-bd 2
	pack propagate .middle.f2.fPlayOptions.fTop 0
    frame .middle.f2.fPlayOptions.fTop.fLeft \
	-width [expr $gFrameWidth / 2] \
	-height [expr $gWindowHeight * 20 / 30] \
	-bd 2
    frame .middle.f2.fPlayOptions.fTop.fRight \
	-width [expr $gFrameWidth / 2] \
	-height [expr $gWindowHeight * 20 / 30] \
	-bd 2
    # the contents of the play options frame
    radiobutton .middle.f2.fPlayOptions.fTop.fLeft.rPlaysFirst \
	    -text "Left Plays First" \
	    -font $kLabelFont \
	    -variable gPlaysFirst\
	    -value 0 \
            -command {
	         set gWhoseTurn "Left"
            }
    radiobutton .middle.f2.fPlayOptions.fTop.fRight.rPlaysFirst \
	    -text "Right Plays First" \
	    -font $kLabelFont \
	    -variable gPlaysFirst \
	    -value 1 \
            -command {
	         set gWhoseTurn "Right"
            }
    radiobutton .middle.f2.fPlayOptions.fTop.fLeft.rHuman \
	    -text "Human" \
	    -font $kLabelFont \
	    -variable gLeftHumanOrComputer \
	    -value Human \
	-command { 
	    .middle.f2.fPlayOptions.fTop.fLeft.moveDelay configure -state disabled -fg gray -troughcolor gray
	    .middle.f2.fPlayOptions.fTop.fRight.gameDelay configure -state disabled -fg gray -troughcolor gray
	    if { $gRightHumanOrComputer == "Computer" } {
		EnableSmarterComputerInterface
		set gMoveDelay 0
		.middle.f2.fPlayOptions.fTop.fLeft.moveDelay configure -state active -fg black -troughcolor gold
	    } else {
		DisableSmarterComputerInterface
	    }
	}
    radiobutton .middle.f2.fPlayOptions.fTop.fLeft.rComputer \
	    -text "Computer" \
	    -font $kLabelFont \
	    -variable gLeftHumanOrComputer \
	    -value Computer \
	-command {
	    if { $gRightHumanOrComputer == "Computer" } {
		set gMoveDelay 1
		.middle.f2.fPlayOptions.fTop.fRight.gameDelay configure -state active -fg black -troughcolor gold
	    } else {
		set gMoveDelay 0
	    }
	    .middle.f2.fPlayOptions.fTop.fLeft.moveDelay configure -state active -fg black -troughcolor gold
	    EnableSmarterComputerInterface
	}
    label .middle.f2.fPlayOptions.fTop.fLeft.lName \
	    -text "Left Name:" \
	    -font $kLabelFont
    entry .middle.f2.fPlayOptions.fTop.fLeft.eName \
	    -text "Left Name" \
	    -font $kLabelFont \
	    -textvariable gLeftName \
	    -width 20
    radiobutton .middle.f2.fPlayOptions.fTop.fRight.rHuman \
	    -text "Human" \
	    -font $kLabelFont \
	    -variable gRightHumanOrComputer \
	    -value Human \
	-command { 
	    .middle.f2.fPlayOptions.fTop.fLeft.moveDelay configure -state disabled -fg gray -troughcolor gray
	    .middle.f2.fPlayOptions.fTop.fRight.gameDelay configure -state disabled -fg gray -troughcolor gray
	    if { $gLeftHumanOrComputer == "Computer" } {
		EnableSmarterComputerInterface
		set gMoveDelay 0
		.middle.f2.fPlayOptions.fTop.fLeft.moveDelay configure -state active -fg black -troughcolor gold
	    } else {
		DisableSmarterComputerInterface
	    }
	}
    radiobutton .middle.f2.fPlayOptions.fTop.fRight.rComputer \
	    -text "Computer" \
	    -font $kLabelFont \
	    -variable gRightHumanOrComputer \
	    -value Computer \
	-command {
	    if { $gLeftHumanOrComputer == "Computer" } {
		set gMoveDelay 1
		.middle.f2.fPlayOptions.fTop.fRight.gameDelay configure -state active -fg black -troughcolor gold
	    } else {
		set gMoveDelay 0
	    }
	    .middle.f2.fPlayOptions.fTop.fLeft.moveDelay configure -state active -fg black -troughcolor gold
	    EnableSmarterComputerInterface
	}
    label .middle.f2.fPlayOptions.fTop.fRight.lName \
	    -text "Right Name:" \
	    -font $kLabelFont
    entry .middle.f2.fPlayOptions.fTop.fRight.eName \
	    -text "Right Name" \
	    -font $kLabelFont \
	    -textvariable gRightName \
	    -width 20

    ## JESSE: adding scale bars for the speed of computer to computer play
    scale .middle.f2.fPlayOptions.fTop.fLeft.moveDelay -label "Move Delay" \
	-from 0.0 \
	-to 2.0 \
	-resolution 0.1 \
	-length 8c \
	-state disabled \
	-orient horizontal \
	-activebackground blue \
	-foreground gray \
	-variable gMoveDelay
    scale .middle.f2.fPlayOptions.fTop.fRight.gameDelay -label "Game Delay" \
	-from 0.0 \
	-to 2.0 \
	-resolution 0.1 \
	-length 8c \
	-state disabled \
	-orient horizontal \
	-activebackground blue \
	-foreground gray \
	-variable gGameDelay

    # pack in the contents in the correct order
    pack propagate .middle.f2.fPlayOptions.fTop.fLeft 0	
    pack propagate .middle.f2.fPlayOptions.fTop.fRight 0
    ## JESSE: packing scale bars
    #pack .moveDelay 
    #pack .gameDelay
    ## end Jesse

    ## Smarter computer
    frame .middle.f2.fPlayOptions.fMid.fLeft \
	-width [expr $gFrameWidth / 2]
    frame .middle.f2.fPlayOptions.fMid.fRight \
	-width [expr $gFrameWidth / 2]

    global gSmartness gSmartnessScale
    
    label .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer -text "How should the computer play:" -font $kLabelFont
    radiobutton .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly \
	-text "Perfectly always" \
	-font $kLabelFont \
	-variable gSmartness \
	-value Perfectly \
	-command ".middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -state disabled -foreground gray"
    radiobutton .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly \
	-text "Perfectly sometimes" \
	-font $kLabelFont \
	-variable gSmartness \
	-value Imperfectly \
	-command ".middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -state active -foreground black"
    radiobutton .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly \
	-text "Randomly" \
	-font $kLabelFont \
	-variable gSmartness \
	-value Randomly \
	-command ".middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -state disabled -foreground gray"
    radiobutton .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely \
	-text "Misere-ly" \
	-font $kLabelFont \
	-variable gSmartness \
	-value Miserely \
	-command ".middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -state disabled -foreground gray"
    
    scale .middle.f2.fPlayOptions.fMid.fRight.sPerfectness \
	-label "Percent perfectly:" \
	-from 0 \
	-to 100 \
	-variable gSmartnessScale \
	-orient horizontal

    ############################SOLVING OPTIONS############################
    label .middle.f2.fPlayOptions.fMid.fRight.cSolvingOptions \
	    -text "Solving Options:" \
	    -font $kLabelFont
    radiobutton .middle.f2.fPlayOptions.fMid.fRight.cFrontierSolver \
	-text "Frontier Solver" \
	-font $kLabelFont \
	-variable gLowMem \
	-value false \
	-command {
	    SetSolver
	}
    radiobutton .middle.f2.fPlayOptions.fMid.fRight.cLowMemSolver \
	-text "Low Memory Solver" \
	-font $kLabelFont \
	-variable gLowMem \
	-value true \
	-command {
	    SetSolver
	}
    radiobutton .middle.f2.fPlayOptions.fMid.fRight.cTwoBits \
	-text "Two Bit Solving" \
	-font $kLabelFont \
	-variable gTwoBits \
	-value true \
	-command {
	    SetSolver
	}
    button .middle.f2.fPlayOptions.fMid.fRight.cSolve \
	-text "Solve" \
	-font $kLabelFont \
	-command {

	}

    global gLeftHumanOrComputer gRightHumanOrComputer
    global gPlaysFirst
    global gLowMem gTwoBits
    set gPlaysFirst 0
    set gLowMem false
    set gTwoBits false
    if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" } {
	EnableSmarterComputerInterface
    } else {
	DisableSmarterComputerInterface
    }
    
    pack .middle.f2.fPlayOptions.fTop -side top
    pack .middle.f2.fPlayOptions.fMid -side top
    pack .middle.f2.fPlayOptions.fBot -side bottom   
    pack .middle.f2.fPlayOptions.fTop.fLeft -side left
    pack .middle.f2.fPlayOptions.fTop.fRight -side right
    ## smarter computer widgets
    pack .middle.f2.fPlayOptions.fMid.fLeft -side left -fill x -expand 1
    pack .middle.f2.fPlayOptions.fMid.fRight -side right -fill x -expand 1
    pack .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer -side top
    pack .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly -expand 1 -fill both -side top
    pack .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly -expand 1 -fill both -side top
    pack .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly -side top -expand 1 -fill both
    pack .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely -side top -expand 1 -fill both
    pack .middle.f2.fPlayOptions.fMid.fRight.sPerfectness -side top -expand 1 -fill x    
#    pack .middle.f2.fPlayOptions.fMid.fRight.cSolve -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cTwoBits -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cLowMemSolver -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cFrontierSolver -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cSolvingOptions -side bottom -fill both
    ## moveDelay scale bar
    pack .middle.f2.fPlayOptions.fTop.fLeft.moveDelay -side bottom
    pack .middle.f2.fPlayOptions.fTop.fLeft.rHuman -side bottom -fill both -expand 1
    pack .middle.f2.fPlayOptions.fTop.fLeft.rComputer -side bottom -fill both -expand 1
    pack .middle.f2.fPlayOptions.fTop.fLeft.rPlaysFirst -side bottom -fill both -expand 1
    pack .middle.f2.fPlayOptions.fTop.fLeft.eName -side bottom -expand 1
    pack .middle.f2.fPlayOptions.fTop.fLeft.lName -side bottom  -expand 1
    ## gameDelay scale bar
    pack .middle.f2.fPlayOptions.fTop.fRight.gameDelay -side bottom
    pack .middle.f2.fPlayOptions.fTop.fRight.rHuman -side bottom -fill both -expand 1
    pack .middle.f2.fPlayOptions.fTop.fRight.rComputer -side bottom -fill both -expand 1
    pack .middle.f2.fPlayOptions.fTop.fRight.rPlaysFirst -side bottom -fill both -expand 1
    pack .middle.f2.fPlayOptions.fTop.fRight.eName -side bottom -expand 1
    pack .middle.f2.fPlayOptions.fTop.fRight.lName -side bottom  -expand 1
    pack .middle.f2.fPlayOptions.fBot.bOk -side right -fill both -expand 1


    ##########
    ##
    ## Rules Frame 
    ##            
    ##########
    set rulesFrame .middle.f2.fRules
    
    frame $rulesFrame \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 2 / 30]

    

    frame $rulesFrame.module
    frame $rulesFrame.buttons
    
    pack propagate $rulesFrame 0
    
    button $rulesFrame.buttons.bCancel \
	-text "Cancel" \
	-command {
	    pack forget .middle.f2.fRules
	    pack .middle.f2.cMain
	    .cToolbar raise iATB
            RaiseStatusBarIfGameStarted
	    update
	    global gOldRules
	    GS_SetOption $gOldRules
	    DriverLoop
	}
    button $rulesFrame.buttons.bOk \
	-text "Start new game with above rule settings" \
        -command {
	    # Hide rules frame
	    pack forget .middle.f2.fRules
	    pack .middle.f2.cMain
	    .cToolbar raise iATB

	    # Delete old board
	    GS_Deinitialize .middle.f2.cMain
	    update

	    # Set C option and re-initialize 
	    eval [concat C_SetOption [GS_GetOption]]
	    C_InitializeGame
	    C_InitializeDatabases
	    GS_InitGameSpecific
	    GS_Initialize .middle.f2.cMain

	    # Solve this option
	    if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" } {
		set theValue [C_DetermineValue $gPosition]
	    }

	    # New game
	    TBaction1
	}

    pack $rulesFrame.buttons.bCancel -side left -fill both -expand 1
    pack $rulesFrame.buttons.bOk -side right -fill both -expand 1

    GS_SetupRulesFrame $rulesFrame.module

    pack $rulesFrame.buttons -side bottom -fill x
    pack $rulesFrame.module -side top -fill x

    #
    # Help Frame
    #

    set helpFrame .middle.f2.fHelp
    frame $helpFrame \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 25 / 30] 

    frame $helpFrame.buttons
    frame $helpFrame.content

    pack propagate $helpFrame 0

    button $helpFrame.buttons.bReturn -text "Return" \
	-command {
	    pack forget .middle.f2.fHelp   
	    pack .middle.f2.cMain
	    .cToolbar raise iATB
	    RaiseStatusBarIfGameStarted
	    update
	    DriverLoop
	}
    
    pack $helpFrame.buttons.bReturn -fill both -expand 1

    pack $helpFrame.buttons -side bottom -fill x
    pack $helpFrame.content -side top -fill both -expand 1


    #
    # Skins Frame
    #

    set skinsFrame .middle.f2.fSkins
    frame $skinsFrame \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 25 / 30] 

    frame $skinsFrame.buttons
    frame $skinsFrame.content

    pack propagate $skinsFrame 0

    button $skinsFrame.buttons.bReturn -text "Return" \
	-command {
	    pack forget .middle.f2.fSkins  
	    pack .middle.f2.cMain
	    .cToolbar raise iATB
	    RaiseStatusBarIfGameStarted
	    update
	    DriverLoop
	}
    
    pack $skinsFrame.buttons.bReturn -fill both -expand 1

    pack $skinsFrame.buttons -side bottom -fill x
    pack $skinsFrame.content -side top -fill both -expand 1
    
    
    #    
    # About Frame
    #

    set aboutFrame .middle.f2.fAbout
    frame $aboutFrame \
	-width $gFrameWidth \
	-height [expr $gWindowHeight * 25 / 30]

    frame $aboutFrame.buttons
    frame $aboutFrame.content

    pack propagate $aboutFrame 0

    button $aboutFrame.buttons.bReturn -text "Return" \
	-command {
	    pack forget .middle.f2.fAbout   
	    pack .middle.f2.cMain
	    .cToolbar raise iATB
	    RaiseStatusBarIfGameStarted
	    update
	    DriverLoop
	}

    pack $aboutFrame.buttons.bReturn -fill both -expand 1

    # About frame setup occurs after GS_InitGameSpecific

    pack $aboutFrame.buttons -side bottom -fill x
    pack $aboutFrame.content -side top -fill both -expand 1

    # create the right hand frame
    canvas .middle.f3.cMRight -highlightthickness 0 \
	-bd 0 \
	-width [expr $gWindowWidth * 3 / 16] \
	-height [expr $gWindowHeight * 25 / 30] \
	-background red

    .middle.f1.cMLeft create image [expr $gWindowWidth * 3/32] 100 -image iIMB1p -tags [list  iIMB iIMB1]
    .middle.f1.cMLeft create image [expr $gWindowWidth * 3/32] 300 -image iIMB2p -tags [list  iIMB iIMB2]
    .middle.f1.cMLeft create image [expr $gWindowWidth * 3/32] 450 -image iIMB3p -tags [list  iIMB iIMB3]
    #.middle.f1.cMLeft create image [expr $gWindowWidth * 3/32] 250 -image iAMB7p -tags [list detVal]
    .middle.f1.cMLeft create image [expr $gWindowWidth * 3/32] 250 -image iSMB7p -tags [list startupPic]
	    	
    .middle.f3.cMRight create image [expr $gWindowWidth * 3/32] 300 -image iAMB5p -tags [list  iAMB iAMB5]
    .middle.f3.cMRight create image [expr $gWindowWidth * 3/32] 100 -image iIMB4p -tags [list  iIMB iIMB4]
    .middle.f3.cMRight create image [expr $gWindowWidth * 3/32] 300 -image iIMB5p -tags [list  iIMB iIMB5]
    .middle.f3.cMRight create image [expr $gWindowWidth * 3/32] 450 -image iIMB6p -tags [list  iIMB iIMB6]
    .middle.f3.cMRight create image [expr $gWindowWidth * 3/32] 250 -image iAMB8p -tags [list play]

    .middle.f1.cMLeft create text 75 100 \
	    -text "To Win:" \
	    -width 140 \
	    -justify center \
	    -font $kToMoveToWinFont \
	    -anchor center \
	    -tags [list ToWin textitem]

    .middle.f1.cMLeft create text 75 300 \
	    -text "To Move:" \
	    -width 140 \
	    -justify center \
	    -font $kToMoveToWinFont \
	    -anchor center \
	    -tags [list ToMove textitem]

    .middle.f1.cMLeft create text 75 450 \
	    -text [format "Left:\n%s" $gLeftName] \
	    -width 140 \
	    -justify center \
	    -font $kPlayerLabelFont \
	    -anchor center \
	    -tags [list LeftName Names textitem] \
	    -fill $gLeftColor

    .middle.f3.cMRight create text 75 450 \
	    -text [format "Right:\n%s" $gRightName] \
	    -width 140 \
	    -justify center \
	    -font $kPlayerLabelFont \
	    -anchor center \
	    -tags [list RightName Names textitem] \
	    -fill $gRightColor

    .middle.f3.cMRight create text 75 150 \
	    -text [format "Predictions: %s" $gPredString] \
	    -width 140 \
	    -justify center \
	    -font $kLabelFont \
	    -anchor center \
	    -tags [list Predictions textitem]

    .middle.f3.cMRight create text 75 80\
	-text "" \
	-width 140 \
	-justify center \
	-font $kLabelFont \
	-anchor center \
	-tags [list WhoseTurn textitem]

    # this is the play button
    .middle.f3.cMRight bind play <1> {
	if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" } {
	    . config -cursor watch
	    set theValue [C_DetermineValue $gPosition]
	    set gGameSolved true
	    . config -cursor {}
	} else {
	    set gReallyUnsolved true
	    set gGameSolved true
	    NewGame
	    DriverLoop
	}
	pack forget .middle.f2.fPlayOptions
	global gSmartness gSmartnessScale
	C_SetSmarterComputer $gSmartness $gSmartnessScale
	.middle.f3.cMRight raise WhoseTurn
	pack .middle.f2.cMain -expand 1
	pack .middle.f2.fPlayOptions.fBot -side bottom
	.middle.f3.cMRight lower play
	.middle.f1.cMLeft lower detVal
	.middle.f1.cMLeft lower startupPic
	.middle.f2.cMain lower base
	.middle.f1.cMLeft lower detVal
	.middle.f1.cMLeft raise ToWin
	.middle.f1.cMLeft raise ToMove

	.cToolbar raise iATB

	pack .middle.f2.fPlayOptions.fBot -side bottom
	.cToolbar bind iOTB1 <Any-Leave> \
		".cToolbar raise iATB1"
	
	.cStatus lower base
	global gGamePlayable
	set gGamePlayable true
	NewGame
    }	
    .middle.f1.cMLeft lower detVal
    .middle.f3.cMRight lower play

    pack .middle.f1.cMLeft -expand 1
    pack .middle.f2.cMain  -expand 1
    pack .middle.f3.cMRight -expand 1

    pack .middle.f1 -side left -expand 1
    pack .middle.f2 -side left -expand 1
    pack .middle.f3 -expand 1

    #
    # Status Area
    #
    canvas .cStatus -highlightthickness 0 \
	-bd 0 \
	-width $gWindowWidth \
	-height [expr $gWindowHeight * 4 / 30] \
	-background yellow

    #create bar border
    .cStatus create image 400 40 -image iBBB1p -tags [list iABB iABB1 base]
    .cStatus create image 100 40 -image iABB2p -tags [list sbb iABB iABB2 playA def]
    .cStatus create image 100 40 -image iIBB2p -tags [list sbb iIBB iIBB2 playI]
   #create toWin checked
    .cStatus create image 290 27 -image iABB3p -tags [list sbb iABB iABB3 winA]
    .cStatus create image 290 27 -image iIBB3p -tags [list sbb iIBB iIBB3 winI def]
    #create toMove checked
    .cStatus create image 290 52 -image iABB4p -tags [list sbb iABB iABB4 moveA]
    #create toMove unchecked
    .cStatus create image 290 52 -image iIBB4p -tags [list sbb iIBB iIBB4 moveI def]
    
    ######
    ### create the shoow "none" moves toggle
    ### default value for creating the "none" moves is "no"
    #####
    #create none moves filled, old coords 410,22.5
    #.cStatus create image 530 22.5 -image iABB5p -tags [list sbb iABB iABB5 noneA]
    #create none moves unfilled
    #.cStatus create image 530 22.5 -image iIBB5p -tags [list sbb iIBB iIBB5 noneI def]
   
    
    #create all moves filled, old coords 470,22.5
    .cStatus create image 425 25 -image iABB6p -tags [list sbb iABB iABB6 allA def]
    #create all moves unfilled
    .cStatus create image 425 25 -image iIBB6p -tags [list sbb iIBB iIBB6 allI]
    #create value moves filled, old coords 530, 22.5
    .cStatus create image 515 25 -image iABB7p -tags [list sbb iABB iABB7 valueA]
    #create value moves unfilled
    .cStatus create image 515 25 -image iIBB7p -tags [list sbb iIBB iIBB7 valueI def]
    .cStatus create image 470 52 -image iABB8p -tags [list sbb iABB iABB8 predA]
    .cStatus create image 470 52 -image iIBB8p -tags [list sbb iIBB iIBB8 predI def]
    .cStatus create image 700 40 -image iABB9p -tags [list sbb iABB iABB9 undoA def]
    .cStatus create image 700 40 -image iIBB9p -tags [list sbb iIBB iIBB9 undoI]    
    .middle.f2.cMain create image 250 250 -image iAMM1p -tags [list base iAMM iAMM1]

    .cStatus bind playA <ButtonRelease-1> {
	.cToolbar raise iITB
	.cStatus raise base
	SetupPlayOptions

    }

    .cStatus bind playI <ButtonRelease-1> {
	InitWindow $kRootDir BurstSkin gif

	.cStatus raise iABB2
    }

    .cStatus bind winA <ButtonRelease-1> {
	.middle.f1.cMLeft raise iIMB1
	.cStatus raise iIBB3
	update
    }

    .cStatus bind winI <ButtonRelease-1> {
	.middle.f1.cMLeft raise ToWin
	.cStatus raise iABB3
	update
    }

    .cStatus bind moveA <ButtonRelease-1> {
	.middle.f1.cMLeft raise iIMB2
	.cStatus raise moveI
	update
    }

    .cStatus bind moveI <ButtonRelease-1> {
	.middle.f1.cMLeft raise ToMove
	.cStatus raise iABB4
	update
    }

    .cStatus bind noneA <ButtonRelease-1> {
    }

    # iTMB5 is the right frame picture that covers Value Moves Legend
    .cStatus bind noneI <ButtonRelease-1> {
	.middle.f3.cMRight raise iIMB5 
	.cStatus raise noneA 
	.cStatus raise allI
	.cStatus raise valueI
    }
        
    .cStatus bind allA <ButtonRelease-1> {
    }

    .cStatus bind allI <ButtonRelease-1> {
	global gMoveType
	set gMoveType all
	ToggleMoves all
	.middle.f3.cMRight raise iIMB5
	.cStatus raise valueI
	.cStatus raise allA
	.cStatus raise noneI
    }

    .cStatus bind valueA <ButtonRelease-1> {
    }

    .cStatus bind valueI <ButtonRelease-1> {
	global gMoveType
	set gMoveType value
	ToggleMoves value
	.middle.f3.cMRight raise iAMB5
	.cStatus raise valueA
	.cStatus raise allI
	.cStatus raise noneI
    }

    .cStatus bind iABB8 <ButtonRelease-1> {
	.middle.f3.cMRight raise iIMB4
	.middle.f3.cMRight lower Predictions
	.middle.f3.cMRight raise WhoseTurn
	.cStatus raise iIBB8
    }

    .cStatus bind iIBB8 <ButtonRelease-1> {
	.middle.f3.cMRight raise iIMB4
	.middle.f3.cMRight raise Predictions
	.middle.f3.cMRight raise WhoseTurn
	#.cStatus
	.cStatus raise iABB8
    }

    # Undo Button
    .cStatus bind iABB9 <ButtonRelease-1> {
	Undo
    }
    .cStatus raise sbb
    .cStatus raise def
    .cStatus raise base

    .middle.f1.cMLeft raise iIMB
    .middle.f1.cMLeft raise startupPic

    .middle.f3.cMRight lower textitem

    grid config .cToolbar -column 0 -row 1 \
        -columnspan 1 -rowspan 1 
    grid config .middle    -column 0 -row 2 \
        -columnspan 1 -rowspan 1 
    grid config .cStatus  -column 0 -row 3 \
        -columnspan 1 -rowspan 1
}

proc RaiseStatusBarIfGameStarted {} {
    global gGameSolved
    if { $gGameSolved == "true" } {
	.cStatus lower base
    }
}

proc DisableSmarterComputerInterface {} {
    .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer configure -foreground grey
    .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly configure -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly configure -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly configure -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely configure -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -foreground grey -state disabled
}

proc EnableSmarterComputerInterface {} {
    .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer configure -foreground black
    .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly configure -foreground black -state normal
    .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly configure -foreground black -state normal
    .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly configure -foreground black -state normal
    .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely configure -foreground black -state normal
    global gSmartness
    if { $gSmartness == "Imperfectly" } {
	.middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -foreground black -state normal
    }
}

proc SetToWinString { string } {
    .middle.f1.cMLeft itemconfigure ToWin -text $string
}

proc SetToMoveString { string } {
    .middle.f1.cMLeft itemconfigure ToMove -text $string
}

proc SetupHelpFrame { f width } {
    global kDocumentFont
    message $f.summary -width $width -font $kDocumentFont -text "Help is not available in this version of GAMESMAN."
    pack $f.summary -side top
}

proc SetupSkinsFrame { f width } {
    global kDocumentFont
    message $f.summary -width $width -font $kDocumentFont -text "Click one of the following images to select a skin."
    pack $f.summary -side top

    global kLabelFont gSkinsRootDir gSkinsDir gSkinsExt
    radiobutton .middle.f2.fSkins.title\
	    -text "BurstSkin" \
	    -font $kLabelFont \
	    -value Human \
	-command { 
	    InitButtons gSkinsRootDir gSkinsDir gSkinsExt
	}
}

proc SetupAboutFrame { f width } {

    set width [expr $width - 40]
    canvas $f.scrollpane -yscrollcommand "HandleScrollFeedback $f.bar aboutOffset"
    scrollbar $f.bar -command "HandleScrolling $f aboutOffset"
    pack $f.bar -side right -fill y
    
    set sp $f.scrollpane
    
    global kLabelFont kDocumentFont
    message $sp.title -text "About GamesCrafters" -font $kLabelFont -width $width
    message $sp.web -text "http://gamescrafters.berkeley.edu" -font $kLabelFont -width $width
    message $sp.summary -width $width -font $kDocumentFont -text "The GamesCrafters research group at UC Berkeley is dedicated to exploring multiple areas of game-theory and programming. At the core of the project lies GAMESMAN, a program developed to perfectly play finite, two-person board games. Every semester, we add new games to the system by coding a game's basic rules in C along with several possbile game variants. We also design custom graphical interfaces for each game using Tcl/Tk."

    message $sp.gamesman -width $width -font $kLabelFont \
	-text "GAMESMAN Development Team"

    message $sp.coreAuthors -width $width -font $kDocumentFont -text "Advisor & Original Developer: Dr. Dan Garcia\nArchitecture: Albert Cheng, Attila Gyulassy, Damian Hites, JJ Jordan, Elmer Lee, Scott Lindeneau, Alex Perelman, Sunil Ramesh, Bryon Ross, Wei Tu, Peterson Tretheway, Jonathan Tsai, Tom Wang\nGraphics: Alice Chang, Eleen Chiang, Melinda Franco, Cassie Guy, Keith Ho, Kevin Ip, Heather Kwong\nMathematical Analysis: Michel D'sa, Farzad Eskafi, Erwin Vedar\nDocumenation: Cynthia Okita, Judy Tuan\nModule Retrofit: Jeffrey Chiang, Jennifer Lee, Rach Liu, Jesse Phillips"

    global kGameName
    message $sp.moduleName -width $width -font $kLabelFont \
	-text "$kGameName Development Team"

    global kCAuthors kTclAuthors
    message $sp.moduleAuthors -width $width -font $kDocumentFont \
	-text "C Authors: $kCAuthors\nTcl Authors: $kTclAuthors"
    
    global kRootDir
    image create photo GamesCraftersImage -file "$kRootDir/../bitmaps/GamesCrafters2003Fa.gif"
    canvas $sp.photo -height 360 -width 480
    $sp.photo create image 240 180 -image GamesCraftersImage

    pack $sp.title -side top
    pack $sp.web -side top
    pack $sp.photo -side top
    pack $sp.summary -side top
    pack $sp.moduleName -side top
    pack $sp.moduleAuthors -side top
    pack $sp.gamesman -side top
    pack $sp.coreAuthors -side top
    
    global aboutOffset aboutOffsetMax
    set aboutOffset 0
    # This is just an estimate
    set aboutOffsetMax -500
    place $f.scrollpane -in $f -y $aboutOffset

}

# Bryon: I couldn't get built-in scrollbars to work, so I "invented" my own
proc HandleScrolling { f whichOffset args } {
    global $whichOffset [subst $whichOffset]Max

    set first [lindex $args 0]
    if { $first == "moveto" } {
	set frac [lindex $args 1]
	set $whichOffset [expr $frac * [subst $[subst $whichOffset]Max]]
	place $f.scrollpane -in $f -y [subst $[subst $whichOffset]]
    } else {
	set direction [lindex $args 1]
	set newOffset [expr [subst $[subst $whichOffset]] - 10*$direction]
	if { $newOffset >=  [subst $[subst $whichOffset]Max] && $newOffset <= 0 } {
	    set $whichOffset $newOffset
	    place $f.scrollpane -in $f -y [subst $[subst $whichOffset]]
	}
    }
}

proc HandleScrollFeedback { bar whichOffset args } {
    global $whichOffset  [subst $whichOffset]Max
    
    set fraction [expr 1.0 * [subst $[subst $whichOffset]] / [subst $[subst $whichOffset]Max]]
    $bar set $fraction 0
}

proc InitButtons { gSkinsRootDir gSkinsDir gSkinsExt } {

    global gWindowWidth gWindowHeight
	
    #
    # Load all the button images
    #

    foreach mode {A I O} {
	foreach file {1 2 3 4 5 6 7 8} {
	    set name [format i%sTB%s $mode $file]
	    image create photo [subst $name]p -file [format %s%s%s_1_%s.%s $gSkinsRootDir $gSkinsDir $mode $file $gSkinsExt]
	    set type [format i%sTB $mode]
	    .cToolbar create image [expr ($gWindowWidth / 16) + ($file - 1) * $gWindowWidth / 8] [expr $gWindowHeight / 60] \
		-image [subst $name]p -tags [list tbb $type $name]
	}
    } 
    image create photo iIMB1p -file [format %s%sI_2_1.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIMB2p -file [format %s%sI_2_2.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIMB3p -file [format %s%sI_2_3.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIMB4p -file [format %s%sI_2_4.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIMB5p -file [format %s%sI_2_5.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIMB6p -file [format %s%sI_2_6.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iAMB5p -file [format %s%sA_2_5.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iSMB7p -file [format %s%sA_2_7.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    #image create photo iAMB7p -file [format %s%sA_7_1.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iAMB8p -file [format %s%sA_8_1.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iAMM1p -file [format %s%sA_4_1.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iBBB1p -file [format %s%sA_3_1.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB2p -file [format %s%sA_3_2.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB2p -file [format %s%sI_3_2.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB3p -file [format %s%sA_3_3.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB3p -file [format %s%sI_3_3.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB4p -file [format %s%sA_3_4.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB4p -file [format %s%sI_3_4.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    ##image create photo iABB5p -file [format %s%sA_3_5.%s $gSkinsRootDir $gSkinsDir $gSkinsExt] 
    ##image create photo iIBB5p -file [format %s%sI_3_5.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB6p -file [format %s%sA_3_6.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB6p -file [format %s%sI_3_6.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB7p -file [format %s%sI_3_7.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB7p -file [format %s%sA_3_7.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB8p -file [format %s%sI_3_8.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB8p -file [format %s%sA_3_8.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iIBB9p -file [format %s%sI_3_9.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]
    image create photo iABB9p -file [format %s%sA_3_9.%s $gSkinsRootDir $gSkinsDir $gSkinsExt]


    #
    # Deal with everything in the top toolbar
    #

    #
    # Now Bind all the buttons
    #
    
    # set the active action of each button
    set mode A
    foreach file {1 2 3 4 5 8} {#6 7 removed because not used
	set name [format i%sTB%s $mode $file]
	set type [format i%sTB $mode]
	.cToolbar bind $name <Any-Enter> \
	    ".cToolbar raise {iOTB$file}; update idletasks" 
    }
    
    # bind the action of the mouse-Over images
    set mode O
    foreach file {1 2 3 4 5} {#6 7 removed because not used
	set name [format i%sTB%s $mode $file]
	set type [format i%sTB $mode]
	.cToolbar bind $name <ButtonRelease-1> \
	    ".cStatus raise base; \
             update idletasks; \
             TBaction$file"
	.cToolbar bind $name <Any-Leave> \
	    ".cToolbar raise iATB$file; update idletasks"
    }

    # Deal with exit button separately
    .cToolbar dtag iITB8 iITB
    .cToolbar bind iOTB8 <ButtonRelease-1> {
	exit
    }
    .cToolbar bind iOTB8  <Any-Leave> {
	.cToolbar raise iATB8
    }

    # Set up starting display with the active images on top
    .cToolbar raise iATB
}