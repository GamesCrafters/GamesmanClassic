#  the actions to be performed when the toolbar buttons are pressed
#
proc TBaction1 {} {
  # solve the game, 
  global kGameName varObjective gPosition gInitialPosition
  global gGameSolved gGamePlayable
  global gComputerIsOnlinePlayer
  global moveHistoryY

  set moveHistoryY 0
  # If the play options pane is open, ignore
  if { $gGameSolved == "true" && $gGamePlayable == "false" } {
    # do nothing
    return
  }

  # Send initial game-specific options to C procs.
  if { $gGameSolved == "false" } {
    .middle.f1.cMLeft raise iDMB
    .middle.f3.cMRight raise play
    .cToolbar raise iDTB
    #.cStatus lower base
    pack forget .middle.f2.fPlayOptions.fBot
    SetupPlayOptions
    global gLeftName gRightName gWindowWidth
  
    # Let Left: and Right: fonts scale with gWindowWidth
    set playerFontSize [expr int($gWindowWidth / 70)]
    .middle.f1.cMLeft itemconfig LeftName \
      -text [format "Left:\n%s" $gLeftName] \
      -font "Helvetica $playerFontSize bold"
    .middle.f3.cMRight itemconfig RightName \
      -text [format "Right:\n%s" $gRightName] \
      -font "Helvetica $playerFontSize bold"
    .middle.f1.cMLeft itemconfigure moveHistoryLeftName \
      -text [format "<-- %s Winning" $gLeftName]
    .middle.f1.cMLeft itemconfigure moveHistoryRightName \
      -text [format "%s Winning -->" $gRightName]
    update
  } else {
    set gGamePlayable false
    .middle.f3.cMRight lower play
    .cToolbar raise iITB
    .cToolbar raise iDTB3
    .cStatus lower base
    update idletasks
    set gGamePlayable true
    NewGame
    .cToolbar bind iOTB1 <Any-Leave> {
      .cToolbar raise iITB1
      .middle.f1.cMLeft lower startupPicOver
    }
  }
}

# Load button
proc TBaction2 {} {
  global gMoveSoFar gRightHumanOrComputer gLeftHumanOrComputer gPlaysFirst gGameSolved
  global gMoveDelay gGameDelay gLeftName gRightName kGameName kSavedFileTypes
  global gNewGame kRules kScriptName
  global gWhoseTurn
 
  set savedFile [tk_getOpenFile -filetypes $kSavedFileTypes]
  if {$savedFile eq ""} {
    return
  }
  set fileptr [open $savedFile r]
  gets $fileptr kScriptName
  gets $fileptr kGameName
  gets $fileptr sMoveList
  gets $fileptr kRules
  gets $fileptr gLeftName
  gets $fileptr sLeftHumanOrComputer
  gets $fileptr gRightName  
  gets $fileptr sRightHumanOrComputer 
  gets $fileptr gPlaysFirst
  close $fileptr
  
  set gLeftHumanOrComputer $sLeftHumanOrComputer
  set gRightHumanOrComputer $sRightHumanOrComputer
  set gWhoseTurn "Left"
	
	global gComputerIsOnlinePlayer
	set gComputerIsOnlinePlayer 0 
    
  set gNewGame "true"
  switchRules $kRules
  
  if { $gGameSolved == "false" } {
    clickedPlayNow
  }

  set gLeftHumanOrComputer "Human"
  set gRightHumanOrComputer "Human"

  if { $gPlaysFirst == 0 } {
    set gWhoseTurn "Left"
  } else {
    set gWhoseTurn "Right"
  }

  for { set i [expr [llength $sMoveList] - 1] } { $i >= 0 } { incr  i -1 } {
    ReturnFromHumanMoveHelper [lindex $sMoveList $i]
  }

  set gRightHumanOrComputer $sRightHumanOrComputer
  set gLeftHumanOrComputer $sLeftHumanOrComputer

  RaiseStatusBarIfGameStarted
}

# Save button
proc TBaction3 {} {
  global gMovesSoFar gRightHumanOrComputer gLeftHumanOrComputer gPlaysFirst
  global gMoveDelay gGameDelay gLeftName gRightName kGameName kSavedFileTypes
  global kScriptName

  set savedFile [tk_getSaveFile -filetypes $kSavedFileTypes  -defaultextension ".gcs"]
  if {$savedFile eq ""} {
    return
  }
  set fileptr [open $savedFile w+]
  puts $fileptr $kScriptName
  puts $fileptr $kGameName
  puts $fileptr $gMovesSoFar
  puts $fileptr [GS_GetOption]
  puts $fileptr $gLeftName
  puts $fileptr $gLeftHumanOrComputer
  puts $fileptr $gRightName
  puts $fileptr $gRightHumanOrComputer
  puts $fileptr $gPlaysFirst
  close $fileptr

  RaiseStatusBarIfGameStarted
}

# Rules
proc TBaction4 {} {
  .cToolbar raise iDTB
  global gWaitingForHuman
  set gWaitingForHuman true
  pack forget .middle.f2.cMain
  global gOldRules
  set gOldRules [GS_GetOption]
  pack .middle.f2.fRules -side bottom -fill both -expand 1
}

# About
proc TBaction5 {} {
  .cToolbar raise iDTB
  global gWaitingForHuman
  set gWaitingForHuman true
  pack forget .middle.f2.cMain
  pack .middle.f2.fAbout -side bottom -fill both -expand 1
}

# Skins
proc TBaction6 {} {
  .cToolbar raise iDTB
  global gWaitingForHuman
  set gWaitingForHuman true
  pack forget .middle.f2.cMain   
  pack .middle.f2.fSkins -side bottom
}

# Help button
proc TBaction7 {} {
  .cToolbar raise iDTB
  global gWaitingForHuman
  set gWaitingForHuman true
  pack forget .middle.f2.cMain   
  pack .middle.f2.fHelp -side bottom
}

proc TBaction8 {} {
	## If we're logged in, log out
	global gUsername 
	global gPassword 
	global gSessionId
	global gGameInfo
	if {$gUsername != "" && $gPassword != "" && $gSessionId != ""} {
		if {$gGameInfo != ""} {
			## Quit any game we may be playing. Ignore any errors
			C_SendResign $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]
		}
		## Ignore any errors
		C_LogoutUser $gUsername $gPassword $gSessionId
	}

  #save player name to file
  global gLeftName gRightName
  set fileptr [open playername.txt w+]
  puts $fileptr $gLeftName
  puts $fileptr $gRightName
  close $fileptr

  #save skin to file
  global gSkinsDir
  set fileptr [open skin.txt w+]
  puts $fileptr $gSkinsDir
  close $fileptr

  exit
}

proc SetupPlayOptions {} {
  global gWaitingForHuman 
  global gComputerIsOnlinePlayer 
  global gGameInfo
  global gGameSolved
  set gWaitingForHuman true

  global gMoveType gPosition

  #why was this line ever here??
  #GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]

  pack forget .middle.f2.cMain   
  pack .middle.f2.fPlayOptions -side bottom
  .cStatus raise base
  
  if { $gComputerIsOnlinePlayer == 1 } {
    ## Just finished from an online game. Show the online users list.
    EndGame
    set gComputerIsOnlinePlayer 0
    set gGameSolved false
    NewGame
    set gGameSolved true    	
    DisplayOnlineUsers    	    	
  }
}

proc InitWindow { kRootDir kExt } {
  global gWindowWidth gWindowHeight gWindowWidthRatio gWindowHeightRatio
  global gFrameWidth gFrameHeight
  global convertExists
  global gSkinsRootDir gSkinsDir gSkinsExt
  global gGamePlayable
  global kLabelFont kPlayerLabelFont kToMoveToWinFont kDocumentFont kValueHistoryLabelFont
  global gFontColor
  global tcl_platform
  global gPredString gWhoseTurn
  global gMaxVVHSize 
  
  global moveHistoryY
  set moveHistoryY 0

  # sean: skipinputonsinglemove added fall '05
  global gSkipInputOnSingleMove 
  global gLeftName gRightName
  global gLeftColor gRightColor

  # jesse: move delay and game delay added fall '03
  global gMoveDelay gGameDelay
  global gReallyUnsolved
  global gWaitingForHuman
  global gNewGame

  #move value history
  global moveHistoryList moveHistoryCanvas moveHistoryVisible gPredictionsOn
  global gameMenuToDriverLoop
  global maxRemoteness maxMoveString

  #save filetypes
  global kSavedFileTypes
  #Redo
  global gRedoList
  # mistakes
  global gMistakeList

  # Network play
  package require scrolledLB
  option add *Scrolledlb*Listbox*background white	
  global gOnlineTxtFile
  set gOnlineTxtFile "online.txt"
  global gUsername
  set gUsername ""	
  global gPassword
  set gPassword ""	
  global gSessionId
  set gSessionId ""
  global gGameInfo  
  set gGameInfo ""
  global gGameInterests	
  set gGameInterests [list ]	
  global rpcResponse
  set rpcResponse ""
  global gComputerIsOnlinePlayer
  set gComputerIsOnlinePlayer 0    
  global gCheckingForGameAcceptsId
  set gCheckingForGameAcceptsId 0
  global gGameAcceptsRefreshPeriod
  set gGameAcceptsRefreshPeriod 5000
  global gRefreshingOnlineUsersId
  set gRefreshingOnlineUsersId 0
  global gOnlineUsersRefreshPeriod
  set gOnlineUsersRefreshPeriod 5000
  global gRefreshingInterestedUsersId
  set gRefreshingInterestedUsersId 0
  global gInterestedUsersRefreshPeriod
  set gInterestedUsersRefreshPeriod 5000
  global gCheckingForAcceptedChallengerId
  set gCheckingForAcceptedChallengerId 0
  global gAcceptedChallengerRefreshPeriod
  set gAcceptedChallengerRefreshPeriod 5000
  global gGetRemotePlayersMoveMaxTries
  set gGetRemotePlayersMoveMaxTries 48
  global gGetRemotePlayersMoveRefreshPeriod
  set gGetRemotePlayersMoveRefreshPeriod 500
  
  #
  # Initialize the constants
  #
  set gReallyUnsolved false
  set gMoveDelay 0
  set gGameDelay 1
  set gWhoseTurn "Left"
  set gPredString ""

  set gSkinsRootDir "$kRootDir/../tcl/skins/"
  #set gSkinsDir "$kDir"
  set gSkinsExt "$kExt"
  set gFontColor "black"

  set moveHistoryList []
  set moveHistoryCanvas .middle.f1.cMLeft

  set moveHistoryVisible false
  set gPredictionsOn false
  set gameMenuToDriverLoop false
  set maxRemoteness 0

  set kSavedFileTypes {
    {{All Save Files}             {.gcs}}
    {{GamesCrafters Save File}    {.gcs}}
  }
  #	{{Smart Game Format}          {.sgf}          }
  #	{{Portable Game Notation}     {.pgn}          }

  set gRedoList [list]
  set gMistakeList [list]

  set convertExists false
  if { [file exists "$kRootDir/../bitmaps/convertTest.gif"] } {
    file delete "$kRootDir/../bitmaps/convertTest.gif"
  }
  if { [catch {exec convert -size 50x50 xc:black "$kRootDir/../bitmaps/convertTest.gif"}] } {
    set convertExists false
  } else {
    if { [file exists "$kRootDir/../bitmaps/convertTest.gif"] } {
      set convertExists true
    }
  }

  global fixed_window_size
  if { !$convertExists } {
    wm geometry . =800x600
  } elseif { [info exists fixed_window_size] } {
    #if the good user specified command line arg for fixed size windows
    wm geometry . =$fixed_window_size
  } else {
    set aspectRatioWidth 4
    set aspectRatioHeight 3
    set maxsize [wm maxsize .]
    set maxwidth [lindex $maxsize 0]
    set maxheight 0.9*[lindex $maxsize 1]

    set tempWidth [expr $maxheight * $aspectRatioWidth / $aspectRatioHeight]
    set tempHeight [expr $maxwidth * $aspectRatioHeight / $aspectRatioWidth]
    if { $tempHeight > $maxheight } {
      set maxheight [expr [expr $maxheight / $aspectRatioHeight] * $aspectRatioHeight]
      set maxwidth [expr $maxheight * $aspectRatioWidth / $aspectRatioHeight]
    } else {
      if { $tempWidth > $maxwidth } {
        set maxwidth [expr [expr $maxwidth / $aspectRatioWidth] * $aspectRatioWidth]
        set maxheight [expr $maxwidth * $aspectRatioHeight / $aspectRatioWidth]
      }
    }
    wm geometry . =[expr int($maxwidth)]x[expr int($maxheight)]
  }
  update

  set gWindowWidth [winfo width .]
  set gWindowHeight [winfo height .]

  set gWindowWidthRatio [expr $gWindowWidth/800.0]
  set gWindowHeightRatio [expr $gWindowHeight/600.0]

  set gFrameWidth [expr $gWindowWidth * 10 / 16.0]
  set gFrameHeight [expr $gWindowHeight * 5 / 6.0]


  if { $tcl_platform(platform) == "windows" } {
    set kLabelFont "Helvetica [expr int($gWindowWidthRatio * 10)] bold"
    set kDocumentFont "Helvetica [expr int($gWindowWidthRatio * 10)]"
    set kToMoveToWinFont "Helvetica [expr int($gWindowWidthRatio * 9)] bold"
    set kPlayerLabelFont "Helvetica [expr int($gWindowWidthRatio * 15)] bold"
    set kValueHistoryLabelFont "Helvetica [expr int($gWindowWidthRatio * -7)]"
  } else {
    set kLabelFont "Helvetica [expr int($gWindowWidthRatio * 12)] bold"
    set kDocumentFont "Helvetica [expr int($gWindowWidthRatio * 12)]"
    set kToMoveToWinFont "Helvetica [expr int($gWindowWidthRatio * 12)] bold"
    set kPlayerLabelFont "Helvetica [expr int($gWindowWidthRatio * 18)] bold"
    set kValueHistoryLabelFont "Helvetica [expr int($gWindowWidthRatio * -8)]"
  }

  set maxMoveString [font measure $kValueHistoryLabelFont "0"]

  set gGamePlayable false
  set gWaitingForHuman false
  set gSkipInputOnSingleMove false
  set gNewGame false
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
    -height [expr $gWindowHeight / 30.0] \
    -background black

  InitButtons $gSkinsRootDir $gSkinsDir $gSkinsExt


  #
  # Main Area
  #

  # create the frames needed to divide up the middle into 3 parts
  frame .middle

  # set the size of the frames and force them to stay that way
  frame .middle.f1 \
    -width [expr $gWindowWidth * 3 / 16.0] \
    -height $gFrameHeight
  frame .middle.f2 \
    -width $gFrameWidth \
    -height $gFrameHeight
  frame .middle.f3 \
    -width [expr $gWindowWidth * 3 / 16.0] \
    -height $gFrameHeight

  pack propagate .middle.f1 0
  pack propagate .middle.f2 0
  pack propagate .middle.f3 0

  # set up the necessary canvases in each frame
  canvas .middle.f1.cMLeft -highlightthickness 0 \
    -bd 0 \
    -width [expr $gWindowWidth * 3 / 16.0] \
    -height $gFrameHeight \
    -background black

  canvas .middle.f2.cMain -highlightthickness 0 \
    -bd 0 \
    -width $gFrameWidth \
    -height $gFrameHeight \
    -background white
  
  # since main frame is square, the width and height are the same
  #	-height [expr $gWindowHeight * 25 / 30] \
  
  # 
  # PLAY OPTIONS FRAME
  #
    
  frame .middle.f2.fPlayOptions \
    -width $gFrameWidth \
    -height $gFrameHeight
  #[expr $gWindowHeight * 25 / 30]
  pack propagate .middle.f2.fPlayOptions 0

  frame .middle.f2.fPlayOptions.fBot \
    -width $gFrameWidth \
    -height [expr $gWindowHeight * 2 / 30.0]
  pack propagate .middle.f2.fPlayOptions.fBot 0

  # this is not packed now <- you cannot cancel
  button .middle.f2.fPlayOptions.fBot.bCancel -text "Cancel" \
    -command {
      .cToolbar bind iOTB4 <Any-Leave> \
      ".cToolbar raise iITB4"
      pack forget .middle.f2.fPlayOptions   
      pack .middle.f2.cMain -expand 1
      .cToolbar raise iITB
      .cStatus lower base
      update
    }

  # clicking OK in the in game play options
  button .middle.f2.fPlayOptions.fBot.bOk -text "OK" \
    -command {
      .cToolbar bind iOTB4 <Any-Leave> \
      ".cToolbar raise iITB4"
      pack forget .middle.f2.fPlayOptions   
      pack .middle.f2.cMain -expand 1
      .cStatus lower base
      .cToolbar raise iITB
      global gSmartness gSmartnessScale
      C_SetSmarterComputer $gSmartness $gSmartnessScale
      global gLeftName gRightName
        .middle.f1.cMLeft itemconfigure LeftName \
        -text [format "Left:\n%s" $gLeftName]
        .middle.f3.cMRight itemconfigure RightName \
        -text [format "Right:\n%s" $gRightName]
        .middle.f1.cMLeft itemconfigure moveHistoryLeftName \
        -text [format "<-- %s Winning" $gLeftName]
        .middle.f1.cMLeft itemconfigure moveHistoryRightName \
        -text [format "%s Winning -->" $gRightName]
      update
      global gUseDB, gUsingTiers, gMovesSoFar, gInitialPosition
      if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" || $gUseDB == "true" } {
        if { $gReallyUnsolved == true } {
          . config -cursor watch
          set theValue [C_DetermineValue $gPosition]
          
          # TIER-GAMESMAN
          if { $gUsingTiers == 1 } {
            # Calling C_DetermineValue on a Tier games actually solves the whole game and
            # returns the initial position. So we have to replay the whole game
            set gPosition [C_InitHashWindow $gInitialPosition]
            for { set i [expr [llength $gMovesSoFar] - 1] } { $i >= 0 } { incr  i -1 } {
              set gPosition [C_DoMove $gPosition [lindex $gMovesSoFar $i]]
              set gPosition [C_HashWindow $gPosition]
            }
          }
          
          set gGameSolved true
          . config -cursor {}
          set gReallyUnsolved false
          .middle.f1.cMLeft lower progressBar
          .cStatus raise rulesA
          .cStatus raise historyI
          .cStatus raise valueI
          .cStatus raise allA
          .cStatus raise predI
        }
      }
      set gameMenuToDriverLoop true
      DriverLoop
    }

  frame .middle.f2.fPlayOptions.fMid \
    -width $gFrameWidth \
    -height [expr $gWindowHeight * 10 / 30.0] \
    -bd 2
  pack propagate .middle.f2.fPlayOptions.fMid 0

  frame .middle.f2.fPlayOptions.fTop \
    -width $gFrameWidth \
    -height [expr $gWindowHeight * 14 / 30.0] \
    -bd 2

	pack propagate .middle.f2.fPlayOptions.fTop 0
  frame .middle.f2.fPlayOptions.fTop.fBot \
    -width $gFrameWidth \
    -height [expr $gWindowHeight * 5 / 30.0] \
    -bd 2
  frame .middle.f2.fPlayOptions.fTop.fLeft \
    -width [expr $gFrameWidth / 2] \
    -height [expr $gWindowHeight * 14 / 30.0] \
    -bd 2
  frame .middle.f2.fPlayOptions.fTop.fRight \
    -width [expr $gFrameWidth / 2] \
    -height [expr $gWindowHeight * 14 / 30.0] \
    -bd 2
	pack propagate .middle.f2.fPlayOptions.fTop.fBot 0
  frame .middle.f2.fPlayOptions.fTop.fBot.fLeft \
    -width [expr $gFrameWidth / 2] \
    -height [expr $gWindowHeight * 5 / 30.0] \
    -bd 2
  frame .middle.f2.fPlayOptions.fTop.fBot.fRight \
    -width [expr $gFrameWidth / 2] \
    -height [expr $gWindowHeight * 5 / 30.0] \
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
      .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state disabled -fg gray -troughcolor gray
      .middle.f2.fPlayOptions.fTop.fBot.fRight.gameDelay configure -state disabled -fg gray -troughcolor gray
      if { $gRightHumanOrComputer == "Computer" } {
        EnableSmarterComputerInterface
        set gMoveDelay 0
        .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state active -fg black -troughcolor gold
      } else {
        DisableSmarterComputerInterface	
        .middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline configure -state active		
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
        .middle.f2.fPlayOptions.fTop.fBot.fRight.gameDelay configure -state active -fg black -troughcolor gold
      } else {
        set gMoveDelay 0
      }
      .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state active -fg black -troughcolor gold
      EnableSmarterComputerInterface
      .middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline configure -state disable
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
      .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state disabled -fg gray -troughcolor gray
      .middle.f2.fPlayOptions.fTop.fBot.fRight.gameDelay configure -state disabled -fg gray -troughcolor gray
      if { $gLeftHumanOrComputer == "Computer" } {
        EnableSmarterComputerInterface
        set gMoveDelay 0
        .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state active -fg black -troughcolor gold
      } else {
        DisableSmarterComputerInterface
        .middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline configure -state active		
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
        .middle.f2.fPlayOptions.fTop.fBot.fRight.gameDelay configure -state active -fg black -troughcolor gold
      } else {
        set gMoveDelay 0
      }
      .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state active -fg black -troughcolor gold
      EnableSmarterComputerInterface
      .middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline configure -state disable
    }

  label .middle.f2.fPlayOptions.fTop.fRight.lName \
    -text "Right Name:" \
    -font $kLabelFont

  entry .middle.f2.fPlayOptions.fTop.fRight.eName \
    -text "Right Name" \
    -font $kLabelFont \
    -textvariable gRightName \
    -width 20

  button .middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline \
    -text "Online Play" \
    -font $kLabelFont \
    -state disabled \
    -command {
      SetupOnlinePlay
    }

  ## JESSE: adding scale bars for the speed of computer to computer play
  scale .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay -label "Move Delay" \
    -from 0.0 \
    -to 2.0 \
    -resolution 0.1 \
    -length 8c \
    -state disabled \
    -orient horizontal \
    -activebackground blue \
    -foreground gray \
    -variable gMoveDelay

  scale .middle.f2.fPlayOptions.fTop.fBot.fRight.gameDelay -label "Game Delay" \
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
  pack propagate .middle.f2.fPlayOptions.fTop.fBot.fLeft 0	
  pack propagate .middle.f2.fPlayOptions.fTop.fBot.fRight 0
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

  global gDeltaRemote
  checkbutton .middle.f2.fPlayOptions.fMid.fRight.rDelta \
    -text "Show Delta Remoteness" \
    -font $kLabelFont \
    -variable gDeltaRemote \
    -onvalue true \
    -offvalue false	
  global gSkipInputOnSingleMove gUseDB
  set gUseDB true
  checkbutton .middle.f2.fPlayOptions.fMid.fLeft.rUseDB \
    -text "Load existing database or solve" \
    -font $kLabelFont \
    -variable gUseDB \
    -onvalue true \
    -offvalue false

  checkbutton .middle.f2.fPlayOptions.fMid.fLeft.rAutoMove \
    -text "Automove if one possible move" \
    -font $kLabelFont \
    -variable gSkipInputOnSingleMove \
    -onvalue true \
    -offvalue false

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

  global gAnimationSpeed

  scale .middle.f2.fPlayOptions.fMid.fRight.animationDelay \
      -label "Animation speed:" \
      -from -5 \
      -to 5 \
      -variable gAnimationSpeed \
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
  pack .middle.f2.fPlayOptions.fTop.fBot -side bottom
  pack .middle.f2.fPlayOptions.fTop.fBot.fLeft -side left
  pack .middle.f2.fPlayOptions.fTop.fBot.fRight -side right
  pack .middle.f2.fPlayOptions.fTop.fLeft -side left
  pack .middle.f2.fPlayOptions.fTop.fRight -side right
  
  ## smarter computer widgets
  pack .middle.f2.fPlayOptions.fMid.fLeft -side left -fill x -expand 1
  pack .middle.f2.fPlayOptions.fMid.fRight -side right -fill x -expand 1
  pack .middle.f2.fPlayOptions.fMid.fLeft.rUseDB -side top
  pack .middle.f2.fPlayOptions.fMid.fLeft.rAutoMove -side top
  pack .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer -side top
  pack .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly -expand 1 -fill both -side top
  pack .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly -expand 1 -fill both -side top
  pack .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly -side top -expand 1 -fill both
  pack .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely -side top -expand 1 -fill both
  .middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure -state disabled -foreground gray
  pack .middle.f2.fPlayOptions.fMid.fRight.rDelta -side top
  pack .middle.f2.fPlayOptions.fMid.fRight.sPerfectness -side top -expand 1 -fill x
  pack .middle.f2.fPlayOptions.fMid.fRight.animationDelay -side top -expand 1 -fill x

#    pack .middle.f2.fPlayOptions.fMid.fRight.cSolve -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cTwoBits -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cLowMemSolver -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cFrontierSolver -side bottom -fill both
#    pack .middle.f2.fPlayOptions.fMid.fRight.cSolvingOptions -side bottom -fill both

  ## moveDelay scale bar
  .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay configure -state active -fg black -troughcolor gold
  pack .middle.f2.fPlayOptions.fTop.fBot.fLeft.moveDelay -side bottom

  ## gameDelay scale bar
  pack .middle.f2.fPlayOptions.fTop.fBot.fRight.gameDelay -side bottom
  pack .middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline -side bottom -expand 1
  
  pack .middle.f2.fPlayOptions.fTop.fLeft.rHuman -side bottom -fill both -expand 1
  pack .middle.f2.fPlayOptions.fTop.fLeft.rComputer -side bottom -fill both -expand 1
  pack .middle.f2.fPlayOptions.fTop.fLeft.rPlaysFirst -side bottom -fill both -expand 1
  pack .middle.f2.fPlayOptions.fTop.fLeft.eName -side bottom -expand 1
  pack .middle.f2.fPlayOptions.fTop.fLeft.lName -side bottom  -expand 1

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
  -height [expr $gWindowHeight * 2 / 30.0]

  frame $rulesFrame.module
  frame $rulesFrame.buttons
  
  pack propagate $rulesFrame 0
  
  button $rulesFrame.buttons.bCancel \
  -text "Cancel" \
  -command {
      pack forget .middle.f2.fRules
      pack .middle.f2.cMain -expand 1
      .cToolbar raise iITB		    
          RaiseStatusBarIfGameStarted
          DisableOnlineGamePlayOptions
      update
      global gOldRules
      GS_SetOption $gOldRules
      set gameMenuToDriverLoop true
      DriverLoop
  }

  button $rulesFrame.buttons.bOk \
    -text "Start new game with above rule settings" \
    -command {
      # Hide rules frame
      pack forget .middle.f2.fRules
      pack .middle.f2.cMain -expand 1
      .cToolbar raise iITB
      .cToolbar raise iDTB3
      DisableOnlineGamePlayOptions
    
      # Delete old board
      .middle.f2.cMain delete {!background}
      update

      # Reset the online values
      global gComputerIsOnlinePlayer
      set gComputerIsOnlinePlayer 0 
  
      # Set C option and re-initialize 
      eval [concat C_SetOption [GS_GetOption]]
      C_InitializeGame
      C_InitializeDatabases
      GS_InitGameSpecific
      GS_Initialize .middle.f2.cMain

      # Solve this option
      global gUseDB
      if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" || $gUseDB == "true"} {
        set theValue [C_DetermineValue $gPosition]
        .middle.f1.cMLeft lower progressBar
      } else {
        global gReallyUnsolved
        set gReallyUnsolved true
      }
      # New game
      #TBaction1
      clickedPlayNow
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
  -height $gFrameHeight 
  #[expr $gWindowHeight * 25 / 30] 

  frame $helpFrame.buttons
  frame $helpFrame.content

  pack propagate $helpFrame 0

  button $helpFrame.buttons.bReturn -text "Return" \
    -command {
      pack forget .middle.f2.fHelp   
      pack .middle.f2.cMain -expand 1
      global gWaitingForHuman
      set gWaitingForHuman false
      .cToolbar raise iITB
      global gMovesSoFar
      .cToolbar raise iDTB3
      RaiseStatusBarIfGameStarted
      DisableOnlineGamePlayOptions
      update
      set gameMenuToDriverLoop true
      DriverLoop
    }
  
  pack $helpFrame.buttons.bReturn -fill both -expand 1

  pack $helpFrame.buttons -side bottom -fill x
  pack $helpFrame.content -side top -fill both -expand 1


  ########
  ### Skins Frame
  ########

  set skinsFrame .middle.f2.fSkins
  frame $skinsFrame \
    -width $gFrameWidth \
    -height $gFrameHeight
  #[expr $gWindowHeight * 25 / 30] 

  frame $skinsFrame.buttons
  frame $skinsFrame.content
  frame $skinsFrame.content.left
  frame $skinsFrame.content.right

  pack propagate $skinsFrame 0

  button $skinsFrame.buttons.bReturn -text "Return" \
    -command {
      InitButtons $gSkinsRootDir $gSkinsDir $gSkinsExt
      pack forget .middle.f2.fSkins  
      pack .middle.f2.cMain -expand 1
      global gWaitingForHuman
      set gWaitingForHuman false
      .cToolbar raise iITB
      .cToolbar raise iDTB3
      RaiseStatusBarIfGameStarted
      DisableOnlineGamePlayOptions
      update
      set gameMenuToDriverLoop true
      DriverLoop
    }
  
  pack $skinsFrame.buttons.bReturn -fill both -expand 1

  set skins [list]
  if { [file isdirectory $gSkinsRootDir] } {
    set include "*HiRes*"
    foreach dir [glob -directory  $gSkinsRootDir */] {
      if {[string match $include $dir] &&  \
          [file exists [format %s/screenshot.ppm $dir]]} {
        lappend skins $dir
      }
    }
  }

  set alignmentParity 0
  foreach skin $skins {
    set dirs [file split $skin]
    set name [lindex $dirs [expr [llength $dirs] -1]]

    image create photo [format %s_screenshot $name] \
      -file [format %sscreenshot.ppm $skin]

    if {0 == [expr $alignmentParity % 2]} {
      set align "left"
    } else {
      set align "right"
    }
    button $skinsFrame.content.$align.b$name\
      -compound top\
      -image [format %s_screenshot $name]\
      -text $name\
      -command " \
      InitButtons $gSkinsRootDir [format %s/ $name] ppm; \
      TBaction6; \
      "
    pack $skinsFrame.content.$align.b$name -ipadx 4 -ipady 4 -anchor n
    incr alignmentParity 1
  }

  pack $skinsFrame.buttons -side bottom -fill x
  pack $skinsFrame.content -side top -fill both -expand 1
  pack $skinsFrame.content.left -side left -fill y
  pack $skinsFrame.content.right -side right -fill y

  
  #    
  # About Frame
  #

  set aboutFrame .middle.f2.fAbout
  frame $aboutFrame \
    -width $gFrameWidth \
    -height $gFrameWidth
  #[expr $gWindowHeight * 25 / 30]

  frame $aboutFrame.buttons
  frame $aboutFrame.content

  pack propagate $aboutFrame 0

  button $aboutFrame.buttons.bReturn -text "Return" \
    -command {
      pack forget .middle.f2.fAbout   
      pack .middle.f2.cMain -expand 1
      global gWaitingForHuman
      set gWaitingForHuman false
      .cToolbar raise iITB
      .cToolbar raise iDTB3
      RaiseStatusBarIfGameStarted
      DisableOnlineGamePlayOptions
      update
      set gameMenuToDriverLoop true
      DriverLoop
    }

  pack $aboutFrame.buttons.bReturn -fill both -expand 1

  # About frame setup occurs after GS_InitGameSpecific

  pack $aboutFrame.buttons -side bottom -fill x
  pack $aboutFrame.content -side top -fill both -expand 1

  # create the right hand frame
  canvas .middle.f3.cMRight -highlightthickness 0 \
    -bd 0 \
    -width [expr $gWindowWidth * 3 / 16.0] \
    -height $gFrameHeight \
    -background black

  .middle.f1.cMLeft create image 0 0 -anchor nw -image iIMB1p \
    -tags [list  iIMB iIMB1]
  .middle.f1.cMLeft create image 0 [expr $gWindowHeightRatio * 200] -anchor nw \
    -image iIMB2p -tags [list  iIMB iIMB2]
  .middle.f1.cMLeft create image 0 [expr $gWindowHeightRatio * 400] -anchor nw \
    -image iIMB3p -tags [list  iIMB iIMB3]
  .middle.f1.cMLeft create image 0 0 -anchor nw -image iDMB1p \
    -tags [list  iDMB iDMB1]
  .middle.f1.cMLeft create image 0 [expr $gWindowHeightRatio * 200] -anchor nw \
    -image iDMB2p -tags [list  iDMB iDMB2]
  .middle.f1.cMLeft create image 0 [expr $gWindowHeightRatio * 400] -anchor nw \
    -image iDMB3p -tags [list  iDMB iDMB3]
  .middle.f1.cMLeft create image 0 0 -anchor nw -image iAMB7p \
    -tags [list startupPic]
  .middle.f1.cMLeft create image 0 0 -anchor nw -image iOMB7p \
    -tags [list startupPicOver]
      
  .middle.f3.cMRight create image 0 [expr $gWindowHeightRatio * 200] -anchor nw \
    -image iAMB5p -tags [list  iAMB iAMB5]

  .middle.f3.cMRight create image 0 0 -anchor nw -image iIMB4p \
    -tags [list  iIMB iIMB4]
  .middle.f3.cMRight create image 0 [expr $gWindowHeightRatio * 200] -anchor nw \
    -image iIMB5p -tags [list  iIMB iIMB5]
  .middle.f3.cMRight create image 0 [expr $gWindowHeightRatio * 400] -anchor nw \
    -image iIMB6p -tags [list  iIMB iIMB6]
  .middle.f3.cMRight create image 0 0 -anchor nw -image iDMB4p \
    -tags [list  iDMB iDMB4]
  .middle.f3.cMRight create image 0 [expr $gWindowHeightRatio * 200] -anchor nw \
    -image iDMB5p -tags [list  iDMB iDMB5]
  .middle.f3.cMRight create image 0 [expr $gWindowHeightRatio * 400] -anchor nw \
    -image iDMB6p -tags [list  iDMB iDMB6]
  .middle.f3.cMRight create image 0 0 -anchor nw -image iAMB8p -tags [list play]
  .middle.f3.cMRight create image 0 0 -anchor nw -image iOMB8p \
    -tags [list playOver]


  ## MOVE HISTORY SIDEBAR ##

  set titleY [expr $gWindowWidthRatio * 10]
  set descY [expr $gWindowWidthRatio * 20]
  set graphTopY [expr $gWindowWidthRatio * 35]

  set center [expr $gWindowWidthRatio * 75]

  .middle.f1.cMLeft create text $center $titleY \
    -text "Value History:" \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kToMoveToWinFont \
    -fill $gFontColor \
    -anchor center \
    -tags [list moveHistory moveHistoryTitle textitem]

  .middle.f1.cMLeft create text 0 $descY \
    -text "<-- $gLeftName Winning" \
    -font $kValueHistoryLabelFont \
    -fill $gFontColor \
    -anchor w \
    -tags [list moveHistory moveHistoryDesc moveHistoryLeftName textitem]

  .middle.f1.cMLeft create text [expr 2*$center] $descY \
    -text "$gRightName Winning -->" \
    -font $kValueHistoryLabelFont \
    -fill $gFontColor \
    -anchor e \
    -tags [list moveHistory moveHistoryDesc moveHistoryRightName textitem]

  .middle.f1.cMLeft create line \
    $center $graphTopY $center [expr $gWindowWidthRatio * 430] \
    -fill $gFontColor \
    -width 2 \
    -tags [list moveHistory moveHistoryCenterLine]

  ## END MOVE HISTORY SIDEBAR ##

  .middle.f1.cMLeft create text \
    [expr $gWindowWidthRatio * 75] [expr $gWindowHeightRatio * 100] \
    -text "To Win:" \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kToMoveToWinFont \
    -fill $gFontColor \
    -anchor center \
    -tags [list ToWin textitem]

  .middle.f1.cMLeft create text \
    [expr $gWindowWidthRatio * 75] [expr $gWindowHeightRatio * 300] \
    -text "To Move:" \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kToMoveToWinFont \
    -fill $gFontColor \
    -anchor center \
    -tags [list ToMove textitem]

  .middle.f1.cMLeft create text \
    [expr $gWindowWidthRatio * 75] [expr $gWindowHeightRatio * 450] \
    -text [format "Left:\n%s" $gLeftName] \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kPlayerLabelFont \
    -anchor center \
    -tags [list LeftName Names textitem] \
    -fill $gLeftColor

  .middle.f3.cMRight create text \
    [expr $gWindowWidthRatio * 75] [expr $gWindowHeightRatio * 450] \
    -text [format "Right:\n%s" $gRightName] \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kPlayerLabelFont \
    -anchor center \
    -tags [list RightName Names textitem] \
    -fill $gRightColor

  .middle.f3.cMRight create text \
    [expr $gWindowWidthRatio * 75] [expr $gWindowHeightRatio * 150] \
    -text [format "Predictions: %s" $gPredString] \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kLabelFont \
    -fill $gFontColor \
    -anchor center \
    -tags [list Predictions textitem]

  .middle.f3.cMRight create text \
    [expr $gWindowWidthRatio * 75] [expr $gWindowHeightRatio * 80]\
    -text "" \
    -width [expr $gWindowWidthRatio * 140] \
    -justify center \
    -font $kLabelFont \
    -fill $gFontColor \
    -anchor center \
    -tags [list WhoseTurn textitem]

  # Percent Solved/Loaded Progress Bar
  .middle.f1.cMLeft create rectangle \
    10 [expr $gWindowHeightRatio * 60] 10 [expr $gWindowHeightRatio * 80] \
    -fill black \
    -outline "" \
    -width 0 \
    -fill $gFontColor \
    -tags [list progressBarSlider progressBar]

  .middle.f1.cMLeft create rectangle \
    10 [expr $gWindowHeightRatio * 60] \
    [expr $gWindowWidthRatio * 150 - 10] [expr $gWindowHeightRatio * 80] \
    -fill "" \
    -outline $gFontColor \
    -tags [list progressBarBox progressBar]

  .middle.f1.cMLeft create text \
    10 [expr $gWindowHeightRatio * 10] \
    -justify left \
    -anchor nw \
    -font $kLabelFont \
    -fill $gFontColor \
    -text "" \
    -tags [list progressBarText progressBar textitem]

  # this is the left panel item "click to play"
  global gNewGame
  .middle.f1.cMLeft bind startupPic <Enter> {
    .middle.f1.cMLeft raise startupPicOver
    .cToolbar raise iOTB1
    update idletasks
  }

  .middle.f1.cMLeft bind startupPicOver <ButtonPress-1> {
    TBaction1
    set gNewGame true
    .middle.f1.cMLeft raise iDMB
    .cToolbar lower iOTB1
    update idletasks
  }

  .middle.f1.cMLeft bind startupPicOver <Leave> {
    if { $gNewGame == "false" } {
      .cToolbar lower iOTB1
      .middle.f1.cMLeft raise startupPic
    } else {
      .middle.f1.cMLeft raise iDMB
      .cToolbar lower iOTB1
    }
    update idletasks
  }


  # this is the play now button
  .middle.f3.cMRight bind play <Enter> {
    .middle.f3.cMRight raise playOver
    update idletasks
  }

  .middle.f3.cMRight bind playOver <Leave> {
    if { $gGameSolved == "false" } {
        .middle.f3.cMRight raise play
    } else {
        .middle.f3.cMRight raise iIMB
    }
    update idletasks
  }

  .middle.f3.cMRight bind playOver <ButtonPress-1> {
    clickedPlayNow
  }

  .middle.f3.cMRight lower play
  .middle.f3.cMRight lower playOver
  .middle.f3.cMRight raise iDMB

  pack .middle.f1.cMLeft -expand 1 -fill both 
  pack .middle.f2.cMain -expand 1
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
    -height [expr $gWindowHeight * 4 / 30.0] \
    -background black

  #create bar border
  .cStatus create image 0 0 -anchor nw -image iBBB1p \
    -tags [list iABB iABB1 base]
  .cStatus create image 0 [expr $gWindowHeightRatio * 40] -anchor w \
    -image iABB2p -tags [list sbb iABB iABB2 playA]
  .cStatus create image 0 [expr $gWindowHeightRatio * 40]  -anchor w \
    -image iIBB2p -tags [list sbb iIBB iIBB2 playI def]
  .cStatus create image 0 [expr $gWindowHeightRatio * 40]  -anchor w \
    -image iOBB2p -tags [list sbb iOBB iOBB2 playO]
  .cStatus create image 0 [expr $gWindowHeightRatio * 40]  -anchor w \
    -image iDBB2p -tags [list sbb iDBB iDBB2 playD]

  #create rules disabled
  .cStatus create image \
  [expr $gWindowWidthRatio * 290] [expr $gWindowHeightRatio * 27] \
    -image iDBB3p -tags [list sbb iDBB iDBB3 rulesD]

  #create rules selected
  .cStatus create image \
  [expr $gWindowWidthRatio * 290] [expr $gWindowHeightRatio * 27] \
    -image iABB3p -tags [list sbb iABB iABB3 rulesA]

  #create rules unselected
  .cStatus create image \
  [expr $gWindowWidthRatio * 290] [expr $gWindowHeightRatio * 27] \
    -image iIBB3p -tags [list sbb iIBB iIBB3 rulesI def]

  #create value history disabled
  .cStatus create image \
  [expr $gWindowWidthRatio * 290] [expr $gWindowHeightRatio * 52] \
    -image iDBB4p -tags [list sbb iDBB iDBB4 historyD]

  #create value history selected
  .cStatus create image \
  [expr $gWindowWidthRatio * 290] [expr $gWindowHeightRatio * 52] \
    -image iABB4p -tags [list sbb iABB iABB4 historyA]

  #create value history unselected
  .cStatus create image \
  [expr $gWindowWidthRatio * 290] [expr $gWindowHeightRatio * 52] \
    -image iIBB4p -tags [list sbb iIBB iIBB4 historyI def]
  

  ######
  ### create the show "none" moves toggle
  ### default value for creating the "none" moves is "no"
  #####
  #create none moves filled, old coords 410,22.5
  #.cStatus create image 530 22.5 -image iABB5p -tags [list sbb iABB iABB5 noneA]
  #create none moves unfilled
  #.cStatus create image 530 22.5 -image iIBB5p -tags [list sbb iIBB iIBB5 noneI def]
 
  #create the cover for the moves image
  .cStatus create image \
  [expr $gWindowWidthRatio * 425] [expr $gWindowHeightRatio * 25] \
    -image iDBB6p -tags [list sbb iDBB iDBB6 allD]

  #create all moves filled, old coords 470,22.5
  .cStatus create image \
  [expr $gWindowWidthRatio * 425] [expr $gWindowHeightRatio * 25] \
    -image iABB6p -tags [list sbb iABB iABB6 allA def]

  #create all moves unfilled
  .cStatus create image \
  [expr $gWindowWidthRatio * 425] [expr $gWindowHeightRatio * 25] \
    -image iIBB6p -tags [list sbb iIBB iIBB6 allI]

  #create the cover for the values image
  .cStatus create image \
  [expr $gWindowWidthRatio * 515] [expr $gWindowHeightRatio * 25] \
    -image iDBB7p -tags [list sbb iDBB iDBB7 valueD]

  #create value moves filled, old coords 530, 22.5
  .cStatus create image \
  [expr $gWindowWidthRatio * 515] [expr $gWindowHeightRatio * 25] \
    -image iABB7p -tags [list sbb iABB iABB7 valueA]

  #create value moves unfilled
  .cStatus create image \
  [expr $gWindowWidthRatio * 515] [expr $gWindowHeightRatio * 25] \
    -image iIBB7p -tags [list sbb iIBB iIBB7 valueI def]

  #predictions
  .cStatus create image \
  [expr $gWindowWidthRatio * 470] [expr $gWindowHeightRatio * 52] \
    -image iDBB8p -tags [list sbb iDBB iDBB8 predD]
  .cStatus create image \
  [expr $gWindowWidthRatio * 470] [expr $gWindowHeightRatio * 52] \
    -image iABB8p -tags [list sbb iABB iABB8 predA]
  .cStatus create image \
  [expr $gWindowWidthRatio * 470] [expr $gWindowHeightRatio * 52] \
    -image iIBB8p -tags [list sbb iIBB iIBB8 predI def]

  #undo
  .cStatus create image \
  [expr $gWindowWidthRatio * 700] [expr $gWindowHeightRatio * 40] \
    -anchor e -image iABB9p -tags [list sbb iABB iABB9 undoA]
  .cStatus create image \
  [expr $gWindowWidthRatio * 700] [expr $gWindowHeightRatio * 40] \
    -anchor e -image iIBB9p -tags [list sbb iIBB iIBB9 undoI]
  .cStatus create image \
  [expr $gWindowWidthRatio * 700] [expr $gWindowHeightRatio * 40] \
    -anchor e -image iOBB9p -tags [list sbb iOBB iOBB9 undoO]
  .cStatus create image \
  [expr $gWindowWidthRatio * 700] [expr $gWindowHeightRatio * 40] \
    -anchor e -image iDBB9p -tags [list sbb iDBB iDBB9 undoD def]

  #redo
  .cStatus create image \
  $gWindowWidth [expr $gWindowHeightRatio * 40] \
    -anchor e -image iABB10p -tags [list sbb iABB iABB10 redoA]
  .cStatus create image \
  $gWindowWidth [expr $gWindowHeightRatio * 40] \
    -anchor e -image iIBB10p -tags [list sbb iIBB iIBB10 redoI]
  .cStatus create image \
  $gWindowWidth [expr $gWindowHeightRatio * 40] \
    -anchor e -image iOBB10p -tags [list sbb iOBB iOBB10 redoO]
  .cStatus create image \
  $gWindowWidth [expr $gWindowHeightRatio * 40] \
    -anchor e -image iDBB10p -tags [list sbb iDBB iDBB10 redoD def]

  .middle.f2.cMain create image 0 0 -anchor nw -image iAMM1p -tags [list base iAMM iAMM1]

  #play options
  .cStatus bind playI <Any-Enter> {
    .cStatus raise playO; update idletasks;
  }
  .cStatus bind playO <ButtonRelease-1> {
    .cToolbar raise iDTB
    .cStatus raise base
    SetupPlayOptions
  }
  .cStatus bind playO <Any-Leave> {
    .cStatus raise playI; update idletasks;
  }
  .cStatus bind playO <ButtonPress-1> {
    .cStatus raise playA; update idletasks;
  }


  .cStatus bind rulesA <ButtonRelease-1> {
    #.middle.f1.cMLeft raise iIMB1
    #.middle.f1.cMLeft raise iIMB2
    #.cStatus raise iIBB3
    update
  }

  .cStatus bind rulesI <ButtonRelease-1> {
    .middle.f1.cMLeft raise iIMB1
    .middle.f1.cMLeft raise iIMB2
    .middle.f1.cMLeft raise ToWin
    .middle.f1.cMLeft raise ToMove
    .middle.f1.cMLeft lower moveHistory

    set moveHistoryVisible false
    .cStatus raise historyI
    .cStatus raise rulesA
    #.cStatus raise iABB3
    update
  }

  .cStatus bind historyA <ButtonRelease-1> {
    #.middle.f1.cMLeft raise iIMB1
    #.middle.f1.cMLeft raise iIMB2
    #.cStatus raise historyI
    update
  }

  .cStatus bind historyI <ButtonRelease-1> {
    .middle.f1.cMLeft raise iIMB1
    .middle.f1.cMLeft raise iIMB2
    .middle.f1.cMLeft raise iIMB3
    .middle.f1.cMLeft raise moveHistory
    .middle.f1.cMLeft raise LeftName
    if { $gPredictionsOn == false } {
      .middle.f1.cMLeft lower moveHistoryValidMoves
    } else {
      .middle.f1.cMLeft raise moveHistoryLine
      .middle.f1.cMLeft raise moveHistoryPlot
    }
    set moveHistoryVisible true
    .cStatus raise rulesI
    .cStatus raise historyA
    #.cStatus raise iABB4
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
    $moveHistoryCanvas lower moveHistoryValidMoves
    set gPredictionsOn false
    .middle.f3.cMRight raise WhoseTurn
    .cStatus raise iIBB8
  }

  .cStatus bind iIBB8 <ButtonRelease-1> {
    .middle.f3.cMRight raise iIMB4
    .middle.f3.cMRight raise Predictions
    if { $moveHistoryVisible == true } {
      $moveHistoryCanvas raise moveHistoryLine
      $moveHistoryCanvas raise moveHistoryPlot
    }
    set gPredictionsOn true
    .middle.f3.cMRight raise WhoseTurn
    #.cStatus
    .cStatus raise iABB8
  }

  # Undo Button
  global gMovesSoFar
  #still need to start with disabled and switch to inactive when first move is made
  .cStatus bind undoI <Any-Enter> {
    .cStatus raise undoO; update idletasks;
  }
  .cStatus bind undoO <ButtonRelease-1> {
    Undo
  }
  .cStatus bind undoO <Any-Leave> {
    global gMovesSoFar
    if { 0 == [llength $gMovesSoFar]} {
    } else {
      .cStatus raise undoI; update idletasks;
    }
  }
  .cStatus bind undoO <ButtonPress-1> {
    if (1) {
      #moves made > 0
      .cStatus raise undoA; update idletasks;
    } else {#moves made = 0
      .cStatus raise undoD; update idletasks;
    }
  }

  # Redo Button
  global gRedoList
  .cStatus bind redoI <Any-Enter> {
    .cStatus raise redoO; update idletasks;
  }
  .cStatus bind redoO <ButtonRelease-1> {
    global gRedoList
    Redo 1
    if { 0 == [llength $gRedoList]} {
      .cStatus raise redoD; update idletasks;
    }
    }
    .cStatus bind redoO <Any-Leave> {
    global gRedoList
    if { 0 == [llength $gRedoList]} {
      .cStatus raise redoD; update idletasks;
    } else {
      .cStatus raise redoI; update idletasks;
    }
  }
  .cStatus bind redoO <ButtonPress-1> {
    .cStatus raise redoA; update idletasks;
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

  # scroll the scrollable visual value history items
  bind . <Key-Next> {updateHidden -100} 
  bind . <Key-Prior> {updateHidden 100} 
  bind . <Key-Home> {resetHidden} 
}

proc resetHidden { } {
  global moveHistoryY
  updateHidden [expr -$moveHistoryY] 
}

proc updateHidden { delta } {
  global moveHistoryCanvas moveHistoryY moveHistoryVisible
  global moveHistoryList
  global gWindowWidthRatio gPredictionsOn  

  set numMoves [llength $moveHistoryList]
  set pieceRadius [expr 3.0 + $gWindowWidthRatio]
  set top [expr $gWindowWidthRatio * 35 + $pieceRadius]
  set bottom [expr $gWindowWidthRatio * 430]
  set baseY [expr $top + $numMoves * $pieceRadius]

  # we can only scroll down once we have scolled
  # up at least once
  if { $moveHistoryY >= 0 && $delta > 0 } {
    return
  }

  # only allow scrolling if we have a certain number of pieces
  # we check for delta != -moveHistoryY because
  # delta == -moveHistoryY when we are reseting
  # we don't want an infinite loop
  if { $baseY <= [expr $bottom - 2 * $pieceRadius] &&
      $delta != [expr -$moveHistoryY]} {
    # we have scroll off and some recent
    # undo has set us below the threshold
    # so we need to reset the move history
    if { $moveHistoryY != 0 } {
      resetHidden
    }
    return
  } 

  # we need to update the Y delta for plotting
  set moveHistoryY [expr $moveHistoryY + $delta]
  $moveHistoryCanvas move moveHistoryScroll 0 $delta 
  update idletasks

  # get the bounding box then calculate all
  # visible elements between top and bottom
  set bbox [$moveHistoryCanvas bbox all] 
  set items [$moveHistoryCanvas find overlapping \
              [lindex $bbox 0] \
              $top \
              [lindex $bbox 2] \
              $bottom]
  
  # loop over all the elements
  # and get the visual value history stuff that
  # is visible, set a tag to indicate that it
  # is visible
  set count 0
  foreach item $items {
    set tags [$moveHistoryCanvas itemcget $item -tags] 
    if { [lsearch $tags moveHistoryScroll] != -1 } {
      # handle case when line does not exist completely in bounding
      # box
      if { [$moveHistoryCanvas type $item] == "line"} {
        set coords [$moveHistoryCanvas coords $item]
        set topY [lindex $coords 1]
        set botY [lindex $coords 3]
        if { ($topY < $top && $botY >= $top) ||
            ($topY < $bottom && 
              $botY >= $bottom) } {
            continue 
        }
      } 

      $moveHistoryCanvas addtag visibleMove withtag $item
      incr count
    }
  }

  # lower the moveHistoryScoll items below the moveHistoryPlot
  $moveHistoryCanvas lower moveHistoryScroll moveHistoryPlot
  # then raise the visible moves
  $moveHistoryCanvas raise visibleMove 

  # lower the items that should not appear
  # due to flags
  if { $gPredictionsOn == false } {
    $moveHistoryCanvas lower moveHistoryValidMoves
  }

  if { $moveHistoryVisible == false } {
    $moveHistoryCanvas lower moveHistory
  }

  # delete tags and update
  $moveHistoryCanvas dtag visibleMove
  update idletasks
}

proc RaiseStatusBarIfGameStarted {} {
    global gGameSolved gMovesSoFar
    if { $gGameSolved == "true" } {
      .cStatus lower base
    }
    if {0 == [llength $gMovesSoFar]} {
      .cToolbar raise iDTB3
    } else {
      .cToolbar raise iITB3
    }

}


proc switchRules { rules } {
    global gLeftHumanOrComputer gRightHumanOrComputer gPosition
    # Hide rules frame
    pack forget .middle.f2.fRules
    pack .middle.f2.cMain -expand 1
    .cToolbar raise iITB

    # Delete old board
    .middle.f2.cMain delete {!background}
    update

    GS_SetOption $rules
    # Set C option and re-initialize
    eval [concat C_SetOption $rules]
    C_InitializeGame
    C_InitializeDatabases
    GS_InitGameSpecific
    GS_Initialize .middle.f2.cMain
    
    # Solve this option
    global gUseDB
    if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" || $gUseDB == "true"} {
      set theValue [C_DetermineValue $gPosition]
      .middle.f1.cMLeft lower progressBar
    } else {
      # TIER-GAMESMAN
      set gPosition [C_InitTierGamesmanIfNeeded $gPosition]
    }

    # New game
    TBaction1
}


proc clickedPlayNow {} {
    global gLeftHumanOrComputer gRightHumanOrComputer gUseDB
    global theValue gGameSolved gReallyUnsolved gPosition
    
    if { $gLeftHumanOrComputer == "Computer" || $gRightHumanOrComputer == "Computer" || $gUseDB == "true" } {
      . config -cursor watch
      set theValue [C_DetermineValue $gPosition]
      .middle.f1.cMLeft lower progressBar
      set gGameSolved true
      . config -cursor {}
    } else {
      # TIER-GAMESMAN
      set gPosition [C_InitTierGamesmanIfNeeded $gPosition]
        
      set gReallyUnsolved true
      set gGameSolved true
    }

    pack forget .middle.f2.fPlayOptions
    global gSmartness gSmartnessScale
    C_SetSmarterComputer $gSmartness $gSmartnessScale
    pack .middle.f2.cMain -expand 1
    pack .middle.f2.fPlayOptions.fBot -side bottom

    .middle.f3.cMRight lower play
    .middle.f1.cMLeft lower startupPic
    .middle.f1.cMLeft raise iIMB
    .middle.f3.cMRight raise iIMB
    .middle.f2.cMain lower base
    .middle.f1.cMLeft raise ToWin
    .middle.f1.cMLeft raise ToMove
    .middle.f1.cMLeft lower moveHistory
    set moveHistoryVisible false

    .cStatus raise rulesA
    .cStatus raise historyI
    
    .cToolbar raise iITB
    .cToolbar raise iDTB3	
	
    pack .middle.f2.fPlayOptions.fBot -side bottom

    .cToolbar bind iOTB1 <Any-Leave> {
      .cToolbar raise iITB1
      .middle.f1.cMLeft lower startupPicOver
    }
    DisableOnlineGamePlayOptions
    
    .cStatus lower base
    global gGamePlayable
    set gGamePlayable true
    
    # TIER-GAMESMAN
    global gUsingTiers
    set gUsingTiers [C_UsingTiers]
    
    .middle.f1.cMLeft lower moveHistory
    NewGame

    if {$gReallyUnsolved} {
      .cStatus raise rulesD
      .cStatus raise historyD
      .cStatus raise allD
      .cStatus raise valueD
      .cStatus raise predD
      .middle.f3.cMRight raise iIMB5
      global gMoveType
      set gMoveType all
      ToggleMoves all
      set moveHistoryVisible false
    } else {
      .cStatus raise rulesA
      .cStatus raise historyI
      .cStatus raise valueI
      .cStatus raise allA
      .cStatus raise predI
    }
    
    update
    .middle.f1.cMLeft lower moveHistory
    .middle.f3.cMRight raise WhoseTurn
    .middle.f1.cMLeft raise LeftName
    .middle.f3.cMRight raise RightName
    .cStatus raise undoD
}

proc DisableOnlineGamePlayOptions {} {
    global gComputerIsOnlinePlayer
    
    if { $gComputerIsOnlinePlayer } {
		## Disable those options not available for online play
		.cToolbar raise iDTB1
		.cToolbar raise iDTB2
		.cToolbar raise iDTB3
		.cToolbar raise iDTB4    
	}
}

proc DisableSmarterComputerInterface {} {
    global gUseDB
    set gUseDB false
    .middle.f2.fPlayOptions.fMid.fLeft.rUseDB configure -state active
    .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer configure \
      -foreground grey
    .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly configure \
      -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly configure \
      -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly configure \
      -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely configure \
      -foreground grey -state disabled
    .middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure \
      -foreground grey -state disabled
}

proc EnableSmarterComputerInterface {} {
    .middle.f2.fPlayOptions.fMid.fLeft.rUseDB configure -state disabled
    .middle.f2.fPlayOptions.fMid.fLeft.lSmarterComputer configure \
      -foreground black
    .middle.f2.fPlayOptions.fMid.fLeft.rSCPerfectly configure \
      -foreground black -state normal
    .middle.f2.fPlayOptions.fMid.fLeft.rSCImperfectly configure \
      -foreground black -state normal
    .middle.f2.fPlayOptions.fMid.fLeft.rSCRandomly configure \
      -foreground black -state normal
    .middle.f2.fPlayOptions.fMid.fLeft.rSCMiserely configure \
      -foreground black -state normal
    global gSmartness gUseDB
    set gUseDB true
    if { $gSmartness == "Imperfectly" } {
      .middle.f2.fPlayOptions.fMid.fRight.sPerfectness configure \
        -foreground black -state normal
    }
}

proc SetToWinString { string } {
    .middle.f1.cMLeft itemconfigure ToWin -text $string
}

proc SetToMoveString { string } {
    .middle.f1.cMLeft itemconfigure ToMove -text $string
}

proc SetupHelpFrame { f width } {
    global kDocumentFont kLabelFont

#    set width [expr $width - 16]
    set center [expr $width / 2] 
    set yOffset 10 
    set step 22

    ## Create scrollpane, scrollbar
    set sp $f.scrollpane
	
    canvas $sp 

    #-scrollregion {0 0 0 400 } -yscrollcommand "$f.vscroll set"
	 
    #scrollbar $f.vscroll -orient vertical -command "$sp yview"

    ## Title Message (summary)
    $sp create text $center $yOffset -width $width -font $kLabelFont \
      -anchor n \
      -text "Welcome to GAMESMAN 3.0 (Gold)"
	

    #pack $f.vscroll -side right -fill y
    pack $sp -side top -fill both -expand y

    ## Create Image of Skin in Scrollpane
    global kRootDir gSkinsExt gSkinsDir gSkinsRootDir
    
    image create photo Screenshot \
      -file [format %s%s/%s.%s $gSkinsRootDir $gSkinsDir "screenshot" $gSkinsExt]
    $sp create image $center 100 -image Screenshot
    
    ## Create Help messages in the Scrollpane
    $sp create text $center 200 -anchor n -width $width -font $kDocumentFont \
      -text "To start a New Game, click the NEW GAME button on the Toolbar.\
               \nTo change the rules of this game, click the RULES button on the Toolbar.\
           \nClicking the ABOUT button displays a brief introduction about GamesCrafters.\
           \nTo change the skin for GAMESMAN, click the SKINS button on the Toolbar.\
           \nClicking the Help button brings you to this window.\
           \nTo quit the game and exit GAMESMAN, click the QUIT button on the Toolbar." 
}

proc SetupSkinsFrame { f width } {
    global kDocumentFont
#    message $f.summary -width $width -font $kDocumentFont -text "Click one of the following images to select a skin."
#    pack $f.summary -side top

    global kLabelFont gSkinsRootDir gSkinsDir gSkinsExt

}

# got tcl/tk scrollbar to work properly with About Frame
proc SetupAboutFrame { f width } {
    global kRootDir
    global kGameName
    global kCAuthors kTclAuthors
    global kLabelFont kDocumentFont

    # 16 is the width of scrollbar
    # the incr size may need to be 
    # tweaked if you change step
    set width [expr $width - 16]
    set center [expr $width / 2] 
    set yOffset 10 
    set step 22
    set sp $f.scrollpane

    image create photo GamesCraftersImage \
      -file "$kRootDir/../bitmaps/GamesCrafters2005FaNamesSmall.ppm"
    
    canvas $sp -yscrollcommand "$f.bar set" -width $width -height 500 
    scrollbar $f.bar -command "$sp yview" -orient v

    pack $f.bar -side right -fill y
    
    $sp create text $center $yOffset  \
      -text "About GamesCrafters" -font $kLabelFont \
      -width $width \
      -anchor n \
      -tags [list about aboutTitle]

    incr yOffset $step
    $sp create text $center $yOffset -text "http://gamescrafters.berkeley.edu" \
      -font $kLabelFont -width $width \
      -anchor n \
      -tags [list about aboutWeb]

    incr yOffset $step
    $sp create text $center $yOffset -width $width -font $kDocumentFont \
      -text "The GamesCrafters research group at UC Berkeley is dedicated \
          to exploring multiple areas of game-theory and programming. \
          At the core of the project lies GAMESMAN, a program developed \
          to perfectly play finite, two-person board games. Every semester, \
          we add new games to the system by coding a game's basic rules in \
          C along with several possbile game variants. We also design \
          custom graphical interfaces for each game using Tcl/Tk." \
      -anchor n \
      -tags [list about aboutSummary]

    incr yOffset [expr $step * 5]
    $sp create text $center $yOffset -width $width -font $kLabelFont \
      -text "GAMESMAN Development Team" \
      -anchor n \
      -tags [list about aboutGamesman]

    incr yOffset $step
    $sp create image $center $yOffset -anchor n -image GamesCraftersImage \
      -tags [list about aboutImage]

    incr yOffset [expr $step * 17]
    $sp create text $center $yOffset -width $width -font $kDocumentFont \
      -text "Advisor & Original Developer: Dr. Dan Garcia" \
      -anchor n \
      -tags [list about aboutCoreAuthors]

##### Old Development Team #####
# change to text item in canvas if you want
# to readd
#    message $sp.coreAuthors -width $width -font $kDocumentFont -text "Advisor & Original Developer: Dr. Dan Garcia\nArchitecture: Albert Cheng, Attila Gyulassy, Damian Hites, JJ Jordan, Elmer Lee, Scott Lindeneau, Alex Perelman, Sunil Ramesh, Bryon Ross, Wei Tu, Peterson Tretheway, Jonathan Tsai, Tom Wang\nGraphics: Alice Chang, Eleen Chiang, Melinda Franco, Cassie Guy, Keith Ho, Kevin Ip, Heather Kwong\nMathematical Analysis: Michel D'sa, Farzad Eskafi, Erwin Vedar\nDocumenation: Cynthia Okita, Judy Tuan\nModule Retrofit: Jeffrey Chiang, Jennifer Lee, Rach Liu, Jesse Phillips"
##### ~~//~~ #####

    incr yOffset $step
    $sp create text $center $yOffset -width $width -font $kLabelFont \
      -text "$kGameName Development Team" \
      -anchor n \
      -tags [list about aboutModuleName]
    
    incr yOffset $step
    $sp create text $center $yOffset -width $width -font $kDocumentFont \
      -text "C Authors: $kCAuthors\nTcl Authors: $kTclAuthors" \
      -anchor n \
      -tags [list about aboutModuleAuthors]
    
    pack $f.scrollpane -in $f 
    $f.scrollpane config -scrollregion [$f.scrollpane bbox all] 
}

proc bestMove { turn theValue theRemoteness prevPossible lastMove } {
	global gMovesSoFar gPosition gJustUndone gMistakeList
	set pos [lindex $prevPossible 0]
	set prev [lindex $prevPossible 1]
	if { [llength $prev] < 1 || $gJustUndone == true} {
		return
	}

	# check the value of current move
	# it should match up with the best of the prevPossible moves
	set bestRemote $theRemoteness
	set bestMove $lastMove
	set bestType $theValue
	set mistake false

	foreach item $prev {
		set val [lindex $item 1]
		
		if { $bestType == "Win" } {
			if { $val == "Tie" || $val == "Lose" } {
				# we are at a winning position
				# could they have given us a tying or losing
				# position instead?
				set mistake true
				set bestMove [lindex $item 0]
				set bestRemote [lindex $item 2]
				set bestType $val
				if {$val == "Lose" } {
					break
				}
			}
		} elseif { $bestType == "Tie" } {
			# we are at a tie
			# could they have given us a losing position?
			if { $val == "Lose" } {
				set mistake true
				set bestMove [lindex $item 0]
				set bestRemote [lindex $item 2]
				set bestType $val
				# break since this is the last possible
				# state change
				break
			}
		} else {
			# cant make a worst mistake
			break
		}
	}
	
	# if there was a mistake then added it to list
	# we keep track of moves so far for undoing later to get positions
	# append in the form of 
	# { value theMove theRemoteness betterValue bestMove
	#		bestRemoteness oldPosition}
	# whose turn is it
	if { $mistake == true } {
		if { $turn == "Left" } {
			set whoMade "Right"
		} else {
			set whoMade "Left"
		}
		lappend gMistakeList [list $whoMade $theValue $lastMove $theRemoteness $bestType $bestMove $bestRemote $pos]
	} else {
		# no mistake append an empty list
		lappend gMistakeList [list]
	}
}

#turn = "Left" or "Right"
#theValue = "Win","Lose", or "Tie"
#theRemoteness = current positions remoteness
#theMoves = [move value (resulting positions remoteness)]
proc plotMove { turn theValue theRemoteness theMoves lastMove } {
    global moveHistoryList moveHistoryCanvas moveHistoryVisible
    global gWindowWidthRatio
    global gFontColor
    global maxRemoteness maxMoveString
    global kValueHistoryLabelFont
    global gPredictionsOn gMovesSoFar
    global gMistakeList gPosition oldMoveList
    # we need to offset the Y by how much we have scrolled
    global moveHistoryY

    $moveHistoryCanvas delete moveHistoryValidMoveLines

    set trueLastMove $lastMove
    set drawRemoteness 255
    set pieceRadius [expr 3.0 + $gWindowWidthRatio]
    set center [expr $gWindowWidthRatio * 75]
    set top [expr $gWindowWidthRatio * 35 + $pieceRadius]
    set bottom [expr $gWindowWidthRatio * 430]
    set labelsY [expr $gWindowWidthRatio * 29]

    set numMoves [llength $moveHistoryList]
    set numMovesSoFar [llength $gMovesSoFar]
    if { $lastMove != "" } {
      set lastMove [C_MoveToString $lastMove]
    }
    set moveStringWidth [font measure $kValueHistoryLabelFont $lastMove]

    #do actual plotting
    set mult 1.0
    set color grey
    if {$theValue == "Win" && $turn == "Left"} {
      set mult -1.0
    } elseif {$theValue == "Win" && $turn == "Right"} {
      set mult 1.0
    } elseif {$theValue == "Lose" && $turn == "Left"} {
      set mult 1.0
    } elseif {$theValue == "Lose" && $turn == "Right"} {
      set mult -1.0
    } elseif { $theRemoteness == $drawRemoteness } {
      set mult 0.0
    }

    if { $theValue == "Win"} {
      set color green
      set lineColor red4
    } elseif { $theValue == "Lose"} {
      set color red4
      set lineColor green
    } else {
      set color yellow
      set lineColor yellow
    }

    if { $maxRemoteness == 0 } {
      set deltax 0
    } else {
      set deltax \
        [expr [expr $center - $pieceRadius - $maxMoveString] / $maxRemoteness]
    }
    set oldDeltaX $deltax

    set nextMaxRemoteness 0
    for {set i 0} {$i < [llength $theMoves]} {incr i} {
      set temp [lindex [lindex $theMoves $i] 2]
      if { $nextMaxRemoteness < $temp && $temp < $drawRemoteness } {
		    set nextMaxRemoteness $temp
      }
    }

    if { $moveStringWidth > $maxMoveString || ($theRemoteness < $drawRemoteness && \
	      ($theRemoteness > $maxRemoteness || $nextMaxRemoteness > $maxRemoteness)) } {
      if { $moveStringWidth > $maxMoveString } {
		    set maxMoveString $moveStringWidth
      }

      if {$theRemoteness < $drawRemoteness && \
          ($theRemoteness > $maxRemoteness || $nextMaxRemoteness > $maxRemoteness) } {
        if { $theRemoteness >= $nextMaxRemoteness } {
          set maxRemoteness [expr $theRemoteness + 1]
        } else {
          set maxRemoteness [expr $nextMaxRemoteness + 1]
        }
      }
    
      if { $maxRemoteness == 0 } {
          set deltax 0
      } else {
          set deltax \
            [expr [expr $center - $pieceRadius - $maxMoveString] / $maxRemoteness]
      }

      rescaleX $center $pieceRadius $oldDeltaX $deltax
    }
    
    set y [expr $moveHistoryY + $top + $pieceRadius * $numMoves]
    set base_y [expr $y - $moveHistoryY]
    set nextY [expr $moveHistoryY + $top + 2 * $pieceRadius + $pieceRadius * $numMoves]

    #draw faint lines at every remoteness value
    if {$oldDeltaX != $deltax} {
      global tk_library
	
      set labelBufferSpace 11.0; #in pixels
	
      $moveHistoryCanvas delete moveHistory1Line
      $moveHistoryCanvas delete moveHistoryLabelsRemoteness
	
      for {set i 0} {$i <= $maxRemoteness} {incr i} {
		    set reverse [expr $maxRemoteness - $i]
		    set stipple @[file join $tk_library demos images gray25.xbm]
		    #set stipple ""
			set width 2

		    if { [expr $reverse % 5] == 0 } {
          set stipple ""
          set width 1

          if { $reverse == 0 } {
				    set width 2
          }

          if { [expr $i * $deltax] > $labelBufferSpace } { #don't draw labels too close to center label
            .middle.f1.cMLeft create text [expr $center - $i * $deltax] $labelsY \
              -text $reverse \
              -font $kValueHistoryLabelFont \
              -fill $gFontColor \
              -anchor center \
              -tags [list moveHistory moveHistoryLabels moveHistoryLabelsRemoteness textitem]
            .middle.f1.cMLeft create text [expr $center + $i * $deltax] $labelsY \
              -text $reverse \
              -font $kValueHistoryLabelFont \
              -fill $gFontColor \
              -anchor center \
              -tags [list moveHistory moveHistoryLabels moveHistoryLabelsRemoteness textitem]
          }
		    }

		    $moveHistoryCanvas create line \
          [expr $center - $i * $deltax] $top \
          [expr $center - $i * $deltax] $bottom \
          -fill $gFontColor \
          -stipple $stipple \
          -width $width \
          -tags [list moveHistory moveHistory1Line moveHistory1LineLeft]

        $moveHistoryCanvas create line \
          [expr $center + [expr $i * $deltax]] $top \
          [expr $center + [expr $i * $deltax]] $bottom \
          -fill $gFontColor \
          -stipple $stipple \
          -width $width \
          -tags [list moveHistory moveHistory1Line moveHistory1LineRight]
      }
    }

    #draw label
   .middle.f1.cMLeft create text $center $labelsY \
      -text "D" \
      -font $kValueHistoryLabelFont \
      -fill $gFontColor \
      -anchor center \
      -tags [list moveHistory moveHistoryLabels textitem]

    if { $turn == "Right" } {
      set moveStringX 0
      set anchor w
    } else {
      set moveStringX [expr 2 * $center]
      set anchor e
    }

    .middle.f1.cMLeft create text $moveStringX $y \
      -text $lastMove \
      -font $kValueHistoryLabelFont \
      -fill $gFontColor \
      -anchor $anchor \
      -tags [list moveHistoryScroll moveHistory moveHistoryMoveString moveString$y textitem]

    set xDistance [expr [expr $maxRemoteness - $theRemoteness] * $deltax * $mult]

    set x [expr $center + $xDistance]
    set xOpposite [expr $center - $xDistance]

    if { $numMoves > 0} {
      set prev [lindex $moveHistoryList [expr $numMoves - 1]]
      set prevCoords [$moveHistoryCanvas coords $prev]
      set prevX [expr [expr [lindex $prevCoords 0] + [lindex $prevCoords 2]] / 2]
      set prevY [expr [expr [lindex $prevCoords 1] + [lindex $prevCoords 3]] / 2]
      # offset the prevY by moveHistoryY 
      set base_prev_y [expr $prevY - $moveHistoryY]
      if { "" != [set prevCoordsOpposite [$moveHistoryCanvas coords oppositeLine$base_prev_y]] } {
		    set prevXOpposite [lindex $prevCoordsOpposite 2]
      } else {
		    set prevXOpposite $prevX
      }
    } else {
      set prevX $x
      set prevY $y
      set prevXOpposite $x
    }
    
    set plottedLine \
      [$moveHistoryCanvas create line $prevX $prevY $x $y \
	     -fill $lineColor \
	     -width 1 \
	     -tags [list moveHistoryScroll moveHistory moveHistoryLine]]

    set plottedMove \
      [$moveHistoryCanvas create oval \
	     [expr $x - $pieceRadius] [expr $y - $pieceRadius] [expr $x + $pieceRadius] [expr $y + $pieceRadius] \
		 -fill $color \
	     -outline "" \
	     -width 3 \
	     -tags [list moveHistoryScroll moveHistory moveHistoryPlot moveHistoryPosition$numMovesSoFar]]

    if { $theValue == "Tie" && $theRemoteness != $drawRemoteness } {
      set plottedLineOpposite \
        [$moveHistoryCanvas create line $prevXOpposite $prevY $xOpposite $y \
          -fill $lineColor \
          -width 1 \
          -tags [list moveHistoryScroll moveHistory moveHistoryLine opposite$base_y oppositeLine$base_y]]
	
      set plottedMoveOpposite \
		    [$moveHistoryCanvas create oval \
          [expr $xOpposite - $pieceRadius] [expr $y - $pieceRadius] [expr $xOpposite + $pieceRadius] [expr $y + $pieceRadius] \
          -fill $color \
          -outline "" \
          -tags [list moveHistoryScroll moveHistory moveHistoryPlot opposite$base_y oppositePiece$base_y moveHistoryPosition$numMovesSoFar]]
    }

    $moveHistoryCanvas bind moveHistoryPosition$numMovesSoFar <ButtonRelease-1> \
      "undoToPosition $numMovesSoFar;"

    $moveHistoryCanvas bind moveHistoryPosition$numMovesSoFar <Enter> \
      "$moveHistoryCanvas itemconfigure moveHistoryPosition$numMovesSoFar -outline white;"

    $moveHistoryCanvas bind moveHistoryPosition$numMovesSoFar <Leave> \
      "$moveHistoryCanvas itemconfigure moveHistoryPosition$numMovesSoFar -outline \"\";"
    
    #old moves deleted at beginning of proc so they dont stick around during animation
    for {set i 0} {$i < [llength $theMoves]} {incr i} {
      set moveRemoteness [lindex [lindex $theMoves $i] 2]
      set moveValue [lindex [lindex $theMoves $i] 1]
	
      set mult 1.0
      set color grey
      #multiplier is opposite of normal because it will be other players turn
      if {$moveValue == "Win" && $turn == "Left"} {
		    set mult 1.0
      } elseif {$moveValue == "Win" && $turn == "Right"} {
		    set mult -1.0
      } elseif {$moveValue == "Lose" && $turn == "Left"} {
		    set mult -1.0
      } elseif {$moveValue == "Lose" && $turn == "Right"} {
		    set mult 1.0
      } elseif { $moveRemoteness == $drawRemoteness } {
		    set mult 0.0
      }
	
      if { $moveValue == "Win"} {
		    set color green
		    set lineColor red4
      } elseif { $moveValue == "Lose"} {
		    set color red4
		    set lineColor green
      } else {
		    set color yellow
		    set lineColor yellow
      }
	
      set moveXDistance [expr [expr $maxRemoteness - $moveRemoteness] * $deltax * $mult]
		
      set moveX [expr $center + $moveXDistance]
      #set xOpposite [expr $center - $moveXDistance]
      $moveHistoryCanvas create line $x $y $moveX $nextY \
		    -fill $lineColor \
		    -width 1 \
		    -tags [list moveHistoryScroll moveHistory moveHistoryLine moveHistoryValidMoves moveHistoryValidMoveLines]
	
      $moveHistoryCanvas create oval \
		    [expr $moveX - $pieceRadius / 2] [expr $nextY - $pieceRadius / 2] \
		    [expr $moveX + $pieceRadius / 2] [expr $nextY + $pieceRadius / 2] \
		    -fill $color \
		    -outline "" \
		    -tags [list moveHistoryScroll moveHistory moveHistoryPlot moveHistoryValidMoves moveHistoryDots$y]
    }

    $moveHistoryCanvas raise moveHistoryPlot

    if { $gPredictionsOn == false } {
      $moveHistoryCanvas lower moveHistoryValidMoves
    }
    if { $moveHistoryVisible == false } {
      $moveHistoryCanvas lower moveHistory
    }

    #done with plotting
    # calculate best move things
    if { [llength $oldMoveList] > 0 } {
      bestMove $turn $theValue $theRemoteness [lindex $oldMoveList [expr [llength $oldMoveList] - 1]] $trueLastMove
    }

    #add new move to list
    lappend moveHistoryList $plottedLine $plottedMove
    lappend oldMoveList [list $gPosition $theMoves]

    if { $y >= $bottom } {
      updateHidden -100
    } else {
      updateHidden 0
    }
}

proc unplotMove { numUndo } {
    global moveHistoryList moveHistoryCanvas gMistakeList oldMoveList moveHistoryY
    set moveBackNum [expr [expr $numUndo + 1] * 2]
    set len [llength $moveHistoryList]
    set newLast [expr $len - $moveBackNum - 1]

    #delete items
    for {set i 0} {$i<[expr $moveBackNum+1] && [expr $len - $i] >= 0} {incr i} {
      set end [lindex $moveHistoryList [expr $len - $i]]
      # offset to the moveHistoryY
      set y [expr [lindex [$moveHistoryCanvas coords $end] 3] - $moveHistoryY]
      
      $moveHistoryCanvas delete opposite$y
      $moveHistoryCanvas delete moveString$y
      $moveHistoryCanvas delete moveHistoryDots$y
      $moveHistoryCanvas delete $end
    }

    #remove item from list
    set moveHistoryList [lrange $moveHistoryList 0 $newLast]
    set gMistakeList [lrange $gMistakeList 0 $newLast]
    set oldMoveList [lrange $oldMoveList 0 $newLast]

    updateHidden 0
}

#undos to the nth position in the game, indexed at 0
proc undoToPosition { positionIndex } {
    global gMovesSoFar
    set numMoves [llength $gMovesSoFar]
    if { $positionIndex > $numMoves } {
      #this is an invalid undo
      puts "Invalid undo attempted."
      return
    }
    set numUndo [expr $numMoves - $positionIndex]
    UndoNMoves $numUndo
}

proc clearMoveHistory { } {
    global moveHistoryList moveHistoryCanvas
    $moveHistoryCanvas delete moveHistoryPlot
    $moveHistoryCanvas delete moveHistoryLine
    $moveHistoryCanvas delete moveHistoryMoveString
    global oldMoveList gMistakeList
    set oldMoveList []
    set gMistakeList []
    set moveHistoryList []
}

proc rescaleX { center pieceRadius oldDeltaX newDeltaX } {
    global moveHistoryList moveHistoryCanvas
    global maxRemoteness
    for {set i 0} {$i<[llength $moveHistoryList]} {incr i} {
      set current [lindex $moveHistoryList $i]
      set currentCoords [$moveHistoryCanvas coords $current]
      if { [$moveHistoryCanvas type $current] == "oval" } {
		    set lineIn [lindex $moveHistoryList [expr $i - 1]]
		    set lineInCoords [$moveHistoryCanvas coords $lineIn]
		    if {$i < [expr [llength $moveHistoryList] - 1]} {
          set lineOut [lindex $moveHistoryList [expr $i + 1]]
          set lineOutCoords [$moveHistoryCanvas coords $lineOut]
		    }
		    set oldX [lindex $lineInCoords 2]
		    set y [lindex $lineInCoords 3]
	
		    #handle tie opposite pieces
		    set isTie false
		    if { [$moveHistoryCanvas type opposite$y] != "" } {
          set isTie true
		    }
	
		    if {$oldX < $center} {
          set mult -1
		    } elseif {$oldX > $center} {
          set mult 1
		    } else {
          set mult 0
		    }
		    set oldXDistance [expr {($oldX - $center) * $mult}]
		    if { $oldDeltaX == 0 } {
          set oldRemoteness 0
		    } else {
          set oldRemoteness [expr $oldXDistance / $oldDeltaX]
		    }
	
		    #handle validMove dots
		    foreach dot [$moveHistoryCanvas find withtag moveHistoryDots$y] {
          set dotCoords [$moveHistoryCanvas coords $dot]
          set xCenter [expr ([lindex $dotCoords 0] + [lindex $dotCoords 2]) / 2]

          if {$xCenter < $center} {
              set dotMult -1
          } elseif {$xCenter > $center} {
              set dotMult 1
          } else {
              set dotMult 0
          }

          if { $oldDeltaX == 0 } {
              set oldDotRemoteness 0
          } else {
              set oldDotRemoteness [expr ($xCenter - $center) / $oldDeltaX]
          }

          set newX [expr $center + ($oldDotRemoteness * $dotMult * $newDeltaX)]
          lset dotCoords 0 [expr $newX-$pieceRadius/2]
          lset dotCoords 2 [expr $newX+$pieceRadius/2]
          $moveHistoryCanvas coords $dot $dotCoords
		    }
	
		    set newXDistance [expr $oldRemoteness * $mult * $newDeltaX]
		    set steps 5
		    set shift [expr $newXDistance / $steps]
		    global gAnimationSpeed
		    #total time in ms for one piece to move to new location
		    #range 0-100
		    set delay [expr {(10 - $gAnimationSpeed + 5) * 10}]
		    if { $mult != 0 } {
          for {set j 1} {$j<=$steps} {incr j} {
				    set newX [expr {$center + $shift * $j}]
				    set xOpposite [expr $center + $center - $newX]
				    lset lineInCoords 2 $newX
				    if {$i == 1} {
						lset lineInCoords 0 $newX
				    }
				    $moveHistoryCanvas coords $lineIn $lineInCoords
				    lset currentCoords 0 [expr $newX - $pieceRadius]
				    lset currentCoords 2 [expr $newX + $pieceRadius]
				    $moveHistoryCanvas coords $current $currentCoords
				    if {$i < [expr [llength $moveHistoryList] - 1]} {
              lset lineOutCoords 0 $newX
              $moveHistoryCanvas coords $lineOut $lineOutCoords
				    }
				    
				    if { $isTie } {
              set nextY [expr $y + 2 * $pieceRadius]
              set oppLineInCoords \
                [$moveHistoryCanvas coords oppositeLine$y]
              set oppPieceCoords \
                [$moveHistoryCanvas coords oppositePiece$y]
              if {"" != [set oppLineOutCoords [$moveHistoryCanvas coords oppositeLine$nextY]]} {
						    lset oppLineOutCoords 0 $xOpposite
						    $moveHistoryCanvas coords oppositeLine$nextY $oppLineOutCoords
              }
              lset oppLineInCoords 2 $xOpposite
              lset oppPieceCoords 0 [expr $xOpposite - $pieceRadius]
              lset oppPieceCoords 2 [expr $xOpposite + $pieceRadius]
              $moveHistoryCanvas coords oppositeLine$y $oppLineInCoords
              $moveHistoryCanvas coords oppositePiece$y $oppPieceCoords
				    }    
				    update idletasks
				    after [expr $delay / $steps]
          }
		    }
      }
    }
}

proc InitButtons { skinsRootDir skinsDir skinsExt } {
    global gWindowWidth gWindowHeight gWindowWidthRatio gWindowHeightRatio
    global convertExists
    global gSkinsExt gSkinsDir gSkinsRootDir
    global gFontColor
	
    #
    # Load all the button images
    #

    set gSkinsExt $skinsExt
    set gSkinsDir $skinsDir
    set gSkinsRootDir $skinsRootDir

    ### Set FONT COLOR for each skin ###
    if { $gSkinsDir == "EarthFromSpace_HiRes/" || \
	    $gSkinsDir == "SpaceCloud_HiRes/"} {
      set gFontColor "white"
    } else {
      set gFontColor "black"
    }

    if { [winfo exists .middle.f1.cMLeft] } {
      .middle.f1.cMLeft itemconfig textitem -fill $gFontColor
      .middle.f1.cMLeft itemconfig moveHistoryTitle -fill $gFontColor
      .middle.f1.cMLeft itemconfig moveHistoryCenterLine -fill $gFontColor
      .middle.f1.cMLeft itemconfig moveHistory1Line -fill $gFontColor
      .middle.f1.cMLeft itemconfig ToWin -fill $gFontColor
      .middle.f1.cMLeft itemconfig ToMove -fill $gFontColor
      .middle.f1.cMLeft itemconfig progressBarSlider -fill $gFontColor
      .middle.f1.cMLeft itemconfig progressBarText -fill $gFontColor
      .middle.f1.cMLeft itemconfig progressBarBox -outline $gFontColor
    }

    if { [winfo exists .middle.f3.cMRight] } {
      .middle.f3.cMRight itemconfig textitem -fill $gFontColor
      .middle.f3.cMRight itemconfig Predictions -fill $gFontColor
      .middle.f3.cMRight itemconfig WhoseTurn -fill $gFontColor
    }

    ### Determine scaling of images ###
    ### gWindow*Ratio is based on the original 800x600 size,
    #### but HiRes skins are 2560x1920 so they must be scaled down by 3.2
    if { [string match *\H\i\R\e\s* $gSkinsDir] } {
      set scalePercent \
        [expr $gWindowWidthRatio * 100 / 3.2]x[expr $gWindowHeightRatio * 100 / 3.2]%!
    } else {
      set scalePercent \
        [expr $gWindowWidthRatio * 100]x[expr $gWindowHeightRatio * 100]%!
    }

    ### If system has the convert utility use the directory for the specified resolution
    #### otherwise use the 800x600 directory
    if { $convertExists } {
      set resolutionDir [format %sx%s/ $gWindowWidth $gWindowHeight]
    } else {
      set resolutionDir "800x600/"
    }
    set resolutionExists true

    if { ![file isdirectory \
        [format %s%s/%s $skinsRootDir $skinsDir $resolutionDir]] && \
        $convertExists } {
      file mkdir [format %s%s/%s $skinsRootDir $skinsDir $resolutionDir]
      set resolutionExists false
    }


    #Load top toolbar images
    foreach mode {A I O D} {
      foreach file {1 2 3 4 5 6 7 8} {
		    set name [format i%sTB%s $mode $file]
	
		    if { $convertExists && (!$resolutionExists || \
            ![file exists [format %s%s/%s%s_1_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]])} {
          exec convert -resize $scalePercent -depth 8 \
					[format %s%s/%s_1_%s.%s $skinsRootDir $skinsDir $mode $file $skinsExt] \
					 [format %s%s/%s%s_1_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
		    }
	
		    #old way
		    #image create photo [subst $name]p -file [format %s%s/%s_1_%s.%s $skinsRootDir $skinsDir $mode $file $skinsExt]
		    #new way (images in directories by resolution)
		    image create photo [subst $name]p \
          -file [format %s%s/%s%s_1_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
	
		    set type [format i%sTB $mode]
		    .cToolbar create image \
				[expr 100 * ($file - 1) * $gWindowWidthRatio] 0 \
          -anchor nw -image [subst $name]p -tags [list tbb $type $name]
      }
    }

    #Load images for middle section
    foreach mode {D I} {
      foreach file {1 2 3 4 5 6} {
		    set name [format i%sMB%s $mode $file]
	
		    if { $convertExists && (!$resolutionExists || \
            ![file exists [format %s%s/%s%s_2_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]])} {
          exec convert -resize $scalePercent -depth 8 \
					 [format %s%s/%s_2_%s.%s $skinsRootDir $skinsDir $mode $file $skinsExt] \
					 [format %s%s/%s%s_2_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
		    }
	
		    image create photo [subst $name]p \
          -file [format %s%s/%s%s_2_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
      }	
    }

    if { $convertExists && !$resolutionExists } {
      #currently no check whether each of these images exists
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/A_2_5.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sA_2_5.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/A_2_7.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sA_2_7.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/O_2_7.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sO_2_7.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/A_8_1.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sA_8_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/O_8_1.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sO_8_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/A_4_1.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sA_4_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
      exec convert -resize $scalePercent -depth 8 \
        [format %s%s/A_3_1.%s $skinsRootDir $skinsDir $skinsExt] \
        [format %s%s/%sA_3_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    }

    image create photo iAMB5p \
      -file [format %s%s/%sA_2_5.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    image create photo iAMB7p \
      -file [format %s%s/%sA_2_7.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    image create photo iOMB7p \
      -file [format %s%s/%sO_2_7.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    image create photo iAMB8p \
      -file [format %s%s/%sA_8_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    image create photo iOMB8p \
      -file [format %s%s/%sO_8_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    image create photo iAMM1p \
      -file [format %s%s/%sA_4_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]
    #Load images for bottom bar
    image create photo iBBB1p \
      -file [format %s%s/%sA_3_1.%s $skinsRootDir $skinsDir $resolutionDir $skinsExt]

    foreach mode {A I O D} {
      foreach file {2 9 10} {
		    set name [format i%sBB%s $mode $file]
	
		    if { $convertExists && (!$resolutionExists || \
            ![file exists [format %s%s/%s%s_3_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]])} {
          exec convert -resize $scalePercent -depth 8 \
					[format %s%s/%s_3_%s.%s $skinsRootDir $skinsDir $mode $file $skinsExt] \
					[format %s%s/%s%s_3_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
		    }
	
		    image create photo [subst $name]p \
          -file [format %s%s/%s%s_3_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
      }
    }

    foreach mode {A I D IO ID AO AD} {
      foreach file {3 4 6 7 8} {
		    set name [format i%sBB%s $mode $file]
		    if { $convertExists && (!$resolutionExists || \
            ![file exists [format %s%s/%s%s_3_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]])} {
          exec convert -resize $scalePercent -depth 8 \
					 [format %s%s/%s_3_%s.%s $skinsRootDir $skinsDir $mode $file $skinsExt] \
					 [format %s%s/%s%s_3_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
		    }
	
		    image create photo [subst $name]p \
          -file [format %s%s/%s%s_3_%s.%s $skinsRootDir $skinsDir $resolutionDir $mode $file $skinsExt]
      }
    }

    #
    # Now Bind all the buttons
    #
    global gNewGame

    #
    # Deal with everything in the top toolbar
    #
    # set the inactive action of each button (mouse not over)
    set mode I
    foreach file {1 2 3 4 5 6 7 8} {
      set name [format i%sTB%s $mode $file]
      set type [format i%sTB $mode]
      .cToolbar bind $name <Enter> \
		    ".cToolbar raise {iOTB$file}; update idletasks"
    }

    # bind the action of the mouse-Over images (mouse over)
    set mode O
    foreach file {1 2 3 4 5 6 7 8} {
      set name [format i%sTB%s $mode $file]
      set type [format i%sTB $mode]
      if { $file == 2 || $file == 3 } {
		    .cToolbar bind $name <ButtonRelease-1> \
				"TBaction$file;"
      } else {
		    .cToolbar bind $name <ButtonRelease-1> \
				".cStatus raise base; \
	            update idletasks; \
	            TBaction$file;"
      }
      .cToolbar bind $name <Leave> \
        ".cToolbar raise iITB$file; update idletasks"
      .cToolbar bind $name <ButtonPress-1> \
		    ".cToolbar raise iATB$file; update idletasks"
    }

    #overwrite button bindings for new game button so it reacts with "click to play"
    .cToolbar bind iITB1 <Enter> {
      .cToolbar raise iOTB1
      if { $gNewGame == "false" } {
		    .middle.f1.cMLeft raise startupPicOver
      }
      update idletasks
    }
    .cToolbar bind iOTB1 <Leave> {
      .cToolbar raise iITB1
      .middle.f1.cMLeft lower startupPicOver
      update idletasks
    }
    .cToolbar bind iOTB1 <ButtonRelease-1> {
      .cStatus raise base
      set gNewGame true
      update idletasks
      TBaction1
    }
    .cToolbar dtag iDTB8 iDTB
    # Set up starting display with the inactive images on top
    .cToolbar raise iITB
    .cToolbar raise iDTB3

    #
    # Deal with other mouse over images
    #
}

# increments the loading progress bar by $percent percent
# call with percent=0 to reset the bar
proc advanceLoadingProgressBar { percent } {
  advanceProgressBar $percent "Loading Database"
}

# increments the solving progress bar by $percent percent
# call with percent=0 to reset the bar
# default str is "Solving Game"
proc advanceProgressBar { percent {str "Solving Game"} } {
  global gFrameHeight gWindowWidthRatio
  set percentDelta [expr {($gWindowWidthRatio * 150 - 20) / 100.0}]
  set barCoords [.middle.f1.cMLeft coords progressBarSlider]
  set xCoord [expr [lindex $barCoords 2] + $percent * $percentDelta]
  set percentDone [expr ($xCoord - [lindex $barCoords 0]) / $percentDelta]

  if {$percentDone > 100.0} {
    set percentDone 100.0
  }

  if {$percent == 0} {
    lset barCoords 2 [lindex $barCoords 0]
    set percentDone 0
  } else {
    lset barCoords 2 $xCoord
  }

  .middle.f1.cMLeft coords progressBarSlider $barCoords
  .middle.f1.cMLeft itemconfig progressBarText \
    -text "$str:\n$percentDone%"
  .middle.f1.cMLeft raise progressBar
  update idletasks
}

