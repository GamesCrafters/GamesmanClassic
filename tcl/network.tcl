######################################################################
##
## Online play dialogs
##
######################################################################
proc SetupOnlinePlay {} {
	## Create the online config/setup frame
	CreateOnlineSetupFrame
	
	## Disable the "play now" image/canvas
	.middle.f3.cMRight configure -state disabled

	## Fill the fOnlineDialog with the login screen (to start off with)
	DisplayUserLogin
}

proc CreateOnlineSetupFrame { } {
	## Create the master online config/setup frame
	set w [expr [winfo width .middle.f2.fPlayOptions] * 0.8]
	set h [expr [winfo height .middle.f2.fPlayOptions] * 0.8]
	frame .middle.f2.fOnlineSetupBg -width $w -height $h -relief raised -bd 2

	## Display it over the current config screen
	set ulx [expr [winfo width .middle.f2.fPlayOptions] * 0.1]
	set uly [expr [winfo height .middle.f2.fPlayOptions] * 0.1]
	place .middle.f2.fOnlineSetupBg -x $ulx -y $uly -in .middle.f2.fPlayOptions
}

proc DisplayUserLogin { } {
	global kLabelFont
	global gUsername
	global gPassword
	global gSessionId
	global gOnlineTxtFile
	
	## Create a new frame
	NewOnlineSetupFrame
	
	## Add our fields 
	frame .middle.f2.fOnlineSetup.fCols
	frame .middle.f2.fOnlineSetup.fCols.fCol1
	frame .middle.f2.fOnlineSetup.fCols.fCol2
	frame .middle.f2.fOnlineSetup.fButtons
	label .middle.f2.fOnlineSetup.lInstructions -text "Please log in" -font $kLabelFont -padx 2 -pady 2
	label .middle.f2.fOnlineSetup.fCols.fCol1.lUsername -text "Username:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.eUsername -textvariable username -font $kLabelFont
	label .middle.f2.fOnlineSetup.fCols.fCol1.lPassword -text "Password:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.ePassword -textvariable password -show "*" -font $kLabelFont
	label .middle.f2.fOnlineSetup.lOr -text "Or" -font $kLabelFont -padx 2 -pady 20
	button .middle.f2.fOnlineSetup.bRegister -text "Register New User" \
    -command {
      ## Hide and delete the frame and display the register user frame
      place forget .middle.f2.fOnlineSetup
      destroy .middle.f2.fOnlineSetup
      DisplayUserRegistration
    }
	button .middle.f2.fOnlineSetup.fButtons.bLogin -text "Log in" -width 10 \
	-command {
		## Login		
		set errMsg [LoginUser $username $password]
		if { $errMsg == "" } {
			## Hide/destroy this frame and display the online users
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			DisplayOnlineUsers
		} else {
			## Show the error to the user
			DisplayOKModal "Error: $errMsg" "OK"
		}
	}
	button .middle.f2.fOnlineSetup.fButtons.bCancel -text "Cancel" -width 10 \
	-command {
		global gGameSolved
	
		## Hide and delete the frame and the background
		place forget .middle.f2.fOnlineSetup
		place forget .middle.f2.fOnlineSetupBg
		destroy .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetupBg
		if { $gGameSolved == "true" } {
			## Display the play options frame buttons
			pack .middle.f2.fPlayOptions.fBot -side bottom
		}
		## Enable the "play now" image/canvas
		.middle.f3.cMRight configure -state normal		
	}
	
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lUsername -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lPassword -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol2.eUsername -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol2.ePassword -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol1 -side left
	pack .middle.f2.fOnlineSetup.fCols.fCol2 -side left
	pack .middle.f2.fOnlineSetup.fButtons.bCancel -side right 
	pack .middle.f2.fOnlineSetup.fButtons.bLogin -side right 
	pack .middle.f2.fOnlineSetup.lInstructions -side top 
	pack .middle.f2.fOnlineSetup.fCols -side top
	pack .middle.f2.fOnlineSetup.fButtons -side top 
	pack .middle.f2.fOnlineSetup.lOr -side top 	
	pack .middle.f2.fOnlineSetup.bRegister -side top 

	## See if we have some saved credentials 
	if { [file exists $gOnlineTxtFile] } {
	    set fileptr [open $gOnlineTxtFile r]
	    gets $fileptr kUsername
	    gets $fileptr kPassword
	    close $fileptr		

		set currUsername [.middle.f2.fOnlineSetup.fCols.fCol2.eUsername get]
		set currPassword [.middle.f2.fOnlineSetup.fCols.fCol2.ePassword get]
		## Only used these saved credentials if existing credentials aren't already entered
		if {$kUsername != "" && $kPassword != "" && $currUsername == "" && $currPassword == ""} {
			.middle.f2.fOnlineSetup.fCols.fCol2.eUsername insert 0 $kUsername		    
			.middle.f2.fOnlineSetup.fCols.fCol2.ePassword insert 0 $kPassword		    
		}			
		focus .middle.f2.fOnlineSetup.fButtons.bLogin		
	} else {	
		focus .middle.f2.fOnlineSetup.fCols.fCol2.eUsername
	}
	
	## Hide the play options frame buttons
	pack forget .middle.f2.fPlayOptions.fBot
}

proc LoginUser {username password} {
	global gUsername
	global gPassword
	global gSessionId
	global gOnlineTxtFile
	
	DisplayStatusModal "Logging in... Please wait"
	set result [C_LoginUser $username $password]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		set gSessionId [string range $result $num end]
		set gUsername $username
		set gPassword $password		
	    #Save login credentials to file
	    global gLeftName gRightName
	    set fileptr [open $gOnlineTxtFile w+]
	    puts $fileptr $gUsername
	    puts $fileptr $gPassword
	    close $fileptr				
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc DisplayUserRegistration { } {
	global kLabelFont
	
	## Create a new frame
	NewOnlineSetupFrame
	
	## Add our fields 
	frame .middle.f2.fOnlineSetup.fCols
	frame .middle.f2.fOnlineSetup.fCols.fCol1
	frame .middle.f2.fOnlineSetup.fCols.fCol2
	frame .middle.f2.fOnlineSetup.fButtons
	label .middle.f2.fOnlineSetup.lInstructions -text "Please provide user registration information" -font $kLabelFont -padx 2 -pady 2
	label .middle.f2.fOnlineSetup.fCols.fCol1.lUsername -text "Username:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.eUsername -textvariable username -font $kLabelFont
	label .middle.f2.fOnlineSetup.fCols.fCol1.lPassword -text "Password:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.ePassword -textvariable password -show "*" -font $kLabelFont
	button .middle.f2.fOnlineSetup.fButtons.bLogin -text "Register" -width 10 \
	-command {
		## Register		
		set errMsg [RegisterUser $username $password]
		if { $errMsg == "" } {
			## Hide/destroy this frame and display the user login
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			DisplayUserLogin
		} else {
			## Show the error to the user
			DisplayOKModal "Error: $errMsg" "OK"
		}
	}
	button .middle.f2.fOnlineSetup.fButtons.bCancel -text "Cancel" -width 10 \
	-command {
		## Hide/destroy this frame and display the user login
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayUserLogin
	}
	
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lUsername -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lPassword -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol2.eUsername -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol2.ePassword -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol1 -side left
	pack .middle.f2.fOnlineSetup.fCols.fCol2 -side left
	pack .middle.f2.fOnlineSetup.fButtons.bCancel -side right 
	pack .middle.f2.fOnlineSetup.fButtons.bLogin -side right 
	pack .middle.f2.fOnlineSetup.lInstructions -side top 
	pack .middle.f2.fOnlineSetup.fCols -side top
	pack .middle.f2.fOnlineSetup.fButtons -side top 
	focus .middle.f2.fOnlineSetup.fCols.fCol2.eUsername
}

proc RegisterUser {username password} {
	DisplayStatusModal "Registering... Please wait"
	set result [C_RegisterUser $username $password]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc DisplayOnlineUsers { } {
	global kLabelFont
	
	## Create a new frame
	NewOnlineSetupFrame
	
	## Add our fields 
	frame .middle.f2.fOnlineSetup.fButtons
	label .middle.f2.fOnlineSetup.lInstructions -text "Join an existing game" -font $kLabelFont -padx 2 -pady 2
	scrolledLB .middle.f2.fOnlineSetup.lbOnlineUsers -listboxHeight 14 -listboxWidth 40 -selectionCommand EnableOnlineUsersJoinButton
	label .middle.f2.fOnlineSetup.lOr -text "Or" -font $kLabelFont -padx 2 -pady 20
	button .middle.f2.fOnlineSetup.bStartOwnGame -text "Start own game" \
	-command {
		## Stop refreshing online users and stop checking for game accepts
		StopRefreshingOnlineUsers	
		StopCheckingForGameAccepts
		## Hide and delete the frame and display the register game frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup
		DisplayGameRegistration
	}
	button .middle.f2.fOnlineSetup.fButtons.bJoin -text "Join" -width 10 -state disabled \
	-command {
		global gGameInterests
		## Get the current selection
		set slxn [.middle.f2.fOnlineSetup.lbOnlineUsers selectionIndex]
		if { $slxn != "" } {
			## Stop refreshing online users and stop checking for game accepts
			StopRefreshingOnlineUsers
			StopCheckingForGameAccepts
			## Evaluate the selection
			set id [.middle.f2.fOnlineSetup.lbOnlineUsers selectionId $slxn]
			set errMsg [JoinGame [lindex $id 0]]
			if { $errMsg == "" } {
				## Let the user know we've expressed interest in the game
				DisplayOKModal "You've requested to join a game. As soon as the other player accepts you'll be notified." "OK"
				lappend gGameInterests $id				
			} else {
				## Show the error to the user
				DisplayOKModal "Error: $errMsg" "OK"
			}
			## Continue refreshing/checking
			StartRefreshingOnlineUsers
			StartCheckingForGameAccepts 		
		}
	}
	button .middle.f2.fOnlineSetup.fButtons.bCancel -text "Cancel" -width 10 \
	-command {
		global gUsername	
		global gPassword
		global gSessionId
		global gGameSolved
		
		## Stop refreshing online users
		StopRefreshingOnlineUsers
		StopCheckingForGameAccepts
		## Logout and ignore any errors
		C_LogoutUser $gUsername $gPassword $gSessionId	
		## Hide and delete the frame and the background
		place forget .middle.f2.fOnlineSetup
		place forget .middle.f2.fOnlineSetupBg
		destroy .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetupBg
		if { $gGameSolved == "true" } {
			## Display the play options frame buttons
			pack .middle.f2.fPlayOptions.fBot -side bottom
		}
				
		## Enable the "play now" image/canvas
		.middle.f3.cMRight configure -state normal		
	}
	
	pack .middle.f2.fOnlineSetup.fButtons.bCancel -side right 
	pack .middle.f2.fOnlineSetup.fButtons.bJoin -side right 
	pack .middle.f2.fOnlineSetup.lInstructions -side top 
	pack .middle.f2.fOnlineSetup.lbOnlineUsers -side top
	pack .middle.f2.fOnlineSetup.fButtons -side top 
	pack .middle.f2.fOnlineSetup.lOr -side top 	
	pack .middle.f2.fOnlineSetup.bStartOwnGame -side top 
	
	## Hide the play options frame buttons
	pack forget .middle.f2.fPlayOptions.fBot

	## Bootstrap refreshing the online user's loop (refresh immediately
	## and let it queue itself in a loop until we stop it). Same with
	## checking for game accepts.
	update
	StartRefreshingOnlineUsers
	StartCheckingForGameAccepts 		
}

proc EnableOnlineUsersJoinButton {} {
	global kGameName
	## Enable the Join button if a "valid" game is selected
	set slxn [.middle.f2.fOnlineSetup.lbOnlineUsers selectionIndex]
	if { $slxn != "" } {
		if { ![.middle.f2.fOnlineSetup.lbOnlineUsers isEnabled $slxn] } {
			.middle.f2.fOnlineSetup.lbOnlineUsers deselect $slxn
			.middle.f2.fOnlineSetup.fButtons.bJoin configure -state disabled
		} else {
			set id [.middle.f2.fOnlineSetup.lbOnlineUsers selectionId $slxn]
			if { $kGameName == [lindex $id 1] } {
				.middle.f2.fOnlineSetup.fButtons.bJoin configure -state normal
			} else {
				.middle.f2.fOnlineSetup.fButtons.bJoin configure -state disabled
			}				
		}
	} else {
		.middle.f2.fOnlineSetup.fButtons.bJoin configure -state disabled
	}
}

proc StartRefreshingOnlineUsers {} {
	global gRefreshingOnlineUsersId
	set gRefreshingOnlineUsersId [after idle RefreshOnlineUsers]
}

proc ContinueRefreshOnlineUsers {} {
	global gRefreshingOnlineUsersId
	global gOnlineUsersRefreshPeriod
	set gRefreshingOnlineUsersId [after $gOnlineUsersRefreshPeriod RefreshOnlineUsers]
}

proc RefreshOnlineUsers {} {
	global kGameName
	global gRefreshingOnlineUsersId
	global gUsername
	global gPassword
	global gSessionId
	
	set result [C_GetUsers $gUsername $gPassword $gSessionId]
	scan $result "%d:%n" errCode num
	if { $errCode == 0 } {
		## Got results, update the online users list and repeat
		set slxn [.middle.f2.fOnlineSetup.lbOnlineUsers selectionIndex]
		if { $slxn != "" } {
			set slxn [.middle.f2.fOnlineSetup.lbOnlineUsers selectionId $slxn]
			set slxn [lindex $slxn 4]
		}		
		.middle.f2.fOnlineSetup.lbOnlineUsers clear
		set records [split [string range $result $num end] "\n"]
		foreach record $records {
			if { $record != "" } {
				set fields [split $record ":"]
				set name [lindex $fields 0]
				set playing [lindex $fields 1]
				set idleMins [lindex $fields 2]
				set game ""
				set gameId -1
				set gameName ""
				set gameVariant ""
				set gameDescription ""
				if { $playing == "1" } {
					set status "\[X\]"
				} elseif { $idleMins > 3 } {
					set status "\[$idleMins\]"
				} else {
					set status "\[  \]"
				}
				if { [llength $fields] > 4 } {
					set gameId [lindex $fields 4]
					set gameName [lindex $fields 5]
					set gameVariant [lindex $fields 6]
					set gameDescription [lindex $fields 7]
					set game "- $gameName (var: $gameVariant) $gameDescription"
				}
				.middle.f2.fOnlineSetup.lbOnlineUsers insert end [list $gameId $gameName $gameVariant $gameDescription $name] "$status $name $game"
				if {$game != "" && $gameName != $kGameName} {
					.middle.f2.fOnlineSetup.lbOnlineUsers setEnabled end 0
				}
				if {$name == $slxn} {
					.middle.f2.fOnlineSetup.lbOnlineUsers select end
				}
			}
		}
		## Enable the online users Join button
		EnableOnlineUsersJoinButton
		## Requeue another loop
		set gRefreshingOnlineUsersId [after idle ContinueRefreshOnlineUsers]
	} else {
		## Error response, don't loop. Display an error modal. 	
		set errMsg [string range $result $num end]
		DisplayOKModal "Error: $errMsg" "OK"
	}
}

proc StopRefreshingOnlineUsers {} {
	global gRefreshingOnlineUsersId
	after cancel $gRefreshingOnlineUsersId
}

proc JoinGame { gameId } {
	global gUsername
	global gPassword
	global gSessionId
	
	DisplayStatusModal "Joining game $gameId... Please wait"
	set result [C_JoinGame $gUsername $gPassword $gSessionId $gameId]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc StartCheckingForGameAccepts { } {
	global gCheckingForGameAcceptsId
	set gCheckingForGameAcceptsId [after idle CheckForGameAccepts]
}

proc ContinueCheckForGameAccepts {} {
	global gCheckingForGameAcceptsId
	global gGameAcceptsRefreshPeriod
	set gCheckingForGameAcceptsId [after $gGameAcceptsRefreshPeriod CheckForGameAccepts]
}

proc CheckForGameAccepts {} {
	global gGameInterests
	global gCheckingForGameAcceptsId
	global gUsername
	global gPassword
	global gSessionId
	
	for {set i 0} {$i<[llength $gGameInterests]} {} {
		set gameInfo [lindex $gGameInterests $i]
		set result [C_ReceivedChallenge $gUsername $gPassword $gSessionId [lindex $gameInfo 0]]
		scan $result "%d:%n" errCode num
		if { $errCode == 0 } {
			set status [string range $result $num end]
			if { $status == "1" } {
				## Remove the gameInfo from the list
				lset gGameInterests [lreplace $gGameInterests $i $i]
				set play [DisplayYesNoModal "You have been selected by [lindex $gameInfo 4] to play game:\n[lindex $gameInfo 1] (var: [lindex $gameInfo 2])" "Accept" "Decline"]
				if { $play } {
					## Accept the challenge and play
					StopCheckingForGameAccepts
					AcceptChallenge $gameInfo 1
				} else {
					## Decline the challenge and continue later
					AcceptChallenge $gameInfo 0
				}
				break
			} elseif { $status == 0 } {
				## Remove the gameInfo from the list silently and continue
				lset gGameInterests [lreplace $gGameInterests $i $i]
			} elseif { $status == 2 } {
				## We don't know if we've been selected or not yet. Just continue.
				incr i
			}				
		} elseif { $errCode == 307 } {
			## Remove the gameInfo entry silently and continue
			lset gGameInterests [lreplace $gGameInterests $i $i]
		} else {
			## Error response, don't loop. Display an error modal and remove the gameInfo entry. 	
			lset gGameInterests [lreplace $gGameInterests $i $i]
			set errMsg [string range $result $num end]
			DisplayOKModal "Error: $errMsg" "OK"
			break;
		}
	}
	## Continue (if necessary)
	if { [llength $gGameInterests] > 0 } {
		set gCheckingForGameAcceptsId [after idle ContinueCheckForGameAccepts]
	} else {
		StopCheckingForGameAccepts
	}
}

proc StopCheckingForGameAccepts {} {
	global gCheckingForGameAcceptsId
	after cancel $gCheckingForGameAcceptsId
}

proc DisplayGameRegistration { } {
	global kLabelFont
	global kGameName
	global gPlaysFirst	
	
	## Create a new frame
	NewOnlineSetupFrame
	
	## Add our fields 
	frame .middle.f2.fOnlineSetup.fCols
	frame .middle.f2.fOnlineSetup.fCols.fCol1
	frame .middle.f2.fOnlineSetup.fCols.fCol2
	frame .middle.f2.fOnlineSetup.fButtons
	label .middle.f2.fOnlineSetup.lInstructions -text "Please provide a short description of this game" -font $kLabelFont -padx 2 -pady 2
	label .middle.f2.fOnlineSetup.fCols.fCol1.lGameName -text "Game:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.eGameName -textvariable gamename -font $kLabelFont
	label .middle.f2.fOnlineSetup.fCols.fCol1.lGameVariant -text "Variant:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.eGameVariant -textvariable gamevariant -font $kLabelFont
	label .middle.f2.fOnlineSetup.fCols.fCol1.lGameDesc -text "Description:" -font $kLabelFont -padx 2 -pady 2
	entry .middle.f2.fOnlineSetup.fCols.fCol2.eGameDesc -textvariable gamedescription -font $kLabelFont
	label .middle.f2.fOnlineSetup.fCols.fCol1.lMoveFirst -text "I move first:" -font $kLabelFont -padx 2 -pady 2
	checkbutton .middle.f2.fOnlineSetup.fCols.fCol2.bMoveFirst -variable movefirst	
	button .middle.f2.fOnlineSetup.fButtons.bLogin -text "Register game" \
	-command {
		## Register		
		set errMsg [RegisterGame $gamename $gamevariant $gamedescription $movefirst]
		if { $errMsg == "" } {
			## Hide/destroy this frame and display the interested users frame
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			DisplayInterestedUsers
		} else {
			## Show the error to the user
			DisplayOKModal "Error: $errMsg" "OK"
		}
	}
	button .middle.f2.fOnlineSetup.fButtons.bCancel -text "Cancel" -width 10 \
	-command {
		## Hide/destroy this frame and display the online users frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayOnlineUsers
	}
	
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameName delete 0 end
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameName insert 0 $kGameName
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameVariant delete 0 end
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameVariant insert 0 [GS_GetOption]
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameDesc delete 0 end
	if {$gPlaysFirst == 0 } {
		.middle.f2.fOnlineSetup.fCols.fCol2.bMoveFirst select
	} else {
		.middle.f2.fOnlineSetup.fCols.fCol2.bMoveFirst deselect
	}	
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameName configure -state disabled
	.middle.f2.fOnlineSetup.fCols.fCol2.eGameVariant configure -state disabled
	.middle.f2.fOnlineSetup.fCols.fCol2.bMoveFirst configure -state disabled
		
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lGameName -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lGameVariant -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lGameDesc -side top 	
	pack .middle.f2.fOnlineSetup.fCols.fCol1.lMoveFirst -side top 		
	pack .middle.f2.fOnlineSetup.fCols.fCol2.eGameName -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol2.eGameVariant -side top 
	pack .middle.f2.fOnlineSetup.fCols.fCol2.eGameDesc -side top 	
	pack .middle.f2.fOnlineSetup.fCols.fCol2.bMoveFirst -side top 		
	pack .middle.f2.fOnlineSetup.fCols.fCol1 -side left
	pack .middle.f2.fOnlineSetup.fCols.fCol2 -side left
	pack .middle.f2.fOnlineSetup.fButtons.bCancel -side right 
	pack .middle.f2.fOnlineSetup.fButtons.bLogin -side right 
	pack .middle.f2.fOnlineSetup.lInstructions -side top 
	pack .middle.f2.fOnlineSetup.fCols -side top
	pack .middle.f2.fOnlineSetup.fButtons -side top 
	focus .middle.f2.fOnlineSetup.fCols.fCol2.eGameDesc
}

proc RegisterGame {gamename gamevariant gamedescription movefirst} {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo
	
	DisplayStatusModal "Registering game... Please wait"
	set result [C_RegisterGame $gUsername $gPassword $gSessionId $gamename $gamevariant $gamedescription $movefirst]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		set gGameInfo [list [string range $result $num end] $gamename $gamevariant $gamedescription $gUsername]
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc AcceptChallenge {gameInfo acc} {
	global gUsername
	global gPassword
	global gSessionId
	
	if { $acc } {
		DisplayStatusModal "Accepting game... Please wait"
		set result [C_AcceptChallenge $gUsername $gPassword $gSessionId [lindex $gameInfo 0] "1"]
		scan $result "%d:%n" errCode num
		DestroyModal
		if { $errCode == 0 } {
			StopRefreshingOnlineUsers
			set moveFirst [string range $result $num end]
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup						
			StartOnlinePlay $gameInfo $moveFirst $gUsername
		} else {
			DisplayOKModal "Error: [string range $result $num end]" "OK"
		}
	} else {
		DisplayStatusModal "Declining game... Please wait"
		set result [C_AcceptChallenge $gUsername $gPassword $gSessionId [lindex $gameInfo 0] "0"]
		scan $result "%d:%n" errCode num
		DestroyModal
		if { $errCode != 0 && $errCode != 307 && $errCode != 308 } {
			DisplayOKModal "Error: [string range $result $num end]" "OK"
		}		
	}
}

proc DisplayInterestedUsers { } {
	global kLabelFont
	
	## Create a new frame
	NewOnlineSetupFrame
	
	## Add our fields 
	frame .middle.f2.fOnlineSetup.fButtons
	label .middle.f2.fOnlineSetup.lInstructions -text "Select a challenger" -font $kLabelFont -padx 2 -pady 2
	scrolledLB .middle.f2.fOnlineSetup.lbInterestedUsers -listboxHeight 7 -listboxWidth 40 -selectionCommand EnableInterestedUsersAcceptButton
	button .middle.f2.fOnlineSetup.fButtons.bAccept -text "Accept" -width 10 -state disabled \
	-command {
		## Get the current selection
		set slxn [.middle.f2.fOnlineSetup.lbInterestedUsers selectionIndex]
		if { $slxn != "" } {
			## Stop refreshing interested users
			StopRefreshingInterestedUsers
			## Evaluate the selection
			set id [.middle.f2.fOnlineSetup.lbInterestedUsers selectionId $slxn]
			set errMsg [SelectChallenger $id]
			if { $errMsg != "" } {
				## Show the error to the user
				DisplayOKModal "Error: $errMsg" "OK"
				## Start refreshing again
				StartRefreshingInterestedUsers
			}
		}
	}
	button .middle.f2.fOnlineSetup.fButtons.bCancel -text "Cancel" -width 10 \
	-command {
		## Stop refreshing interested users
		StopRefreshingInterestedUsers
		set errMsg [UnregisterGame]
		if { $errMsg != "" } {
			## Show the error to the user
			DisplayOKModal "Error: $errMsg" "OK"
			## Hide/destroy this frame and display the register game frame
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			DisplayGameRegistration
		}	
	}
	
	pack .middle.f2.fOnlineSetup.fButtons.bCancel -side right 
	pack .middle.f2.fOnlineSetup.fButtons.bAccept -side right 
	pack .middle.f2.fOnlineSetup.lInstructions -side top 
	pack .middle.f2.fOnlineSetup.lbInterestedUsers -side top
	pack .middle.f2.fOnlineSetup.fButtons -side top 
	
	## Bootstrap refreshing the interested user's loop (refresh immediately
	## and let it queue itself in a loop until we stop it)
	update
	StartRefreshingInterestedUsers
}

proc EnableInterestedUsersAcceptButton {} {
	## Enable the Accept button if a challenger is selected
	set slxn [.middle.f2.fOnlineSetup.lbInterestedUsers selectionIndex]
	if { $slxn != "" } {
		.middle.f2.fOnlineSetup.fButtons.bAccept configure -state normal
	} else {
		.middle.f2.fOnlineSetup.fButtons.bAccept configure -state disabled
	}
}

proc StartRefreshingInterestedUsers {} {
	global gRefreshingInterestedUsersId
	set gRefreshingInterestedUsersId [after idle RefreshInterestedUsers]
}

proc ContinueRefreshInterestedUsers {} {
	global gRefreshingInterestedUsersId
	global gInterestedUsersRefreshPeriod
	set gRefreshingInterestedUsersId [after $gInterestedUsersRefreshPeriod RefreshInterestedUsers]
}

proc RefreshInterestedUsers {} {
	global gGameInfo
	global gRefreshingInterestedUsersId
	global gUsername
	global gPassword
	global gSessionId
	
	set result [C_GetGameStatus $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]]
	scan $result "%d:%n" errCode num
	if { $errCode == 0 } {
		## Got results, update the interested users list and repeat
		set slxn [.middle.f2.fOnlineSetup.lbInterestedUsers selectionIndex]
		if { $slxn != "" } {
			set slxn [.middle.f2.fOnlineSetup.lbInterestedUsers selectionId $slxn]
		}		
		.middle.f2.fOnlineSetup.lbInterestedUsers clear
		set records [split [string range $result $num end] "\n"]
		foreach record $records {
			if { $record != "" } {
				set fields [split $record ":"]
				set name [lindex $fields 0]
				set playing [lindex $fields 1]
				set idleMins [lindex $fields 2]
				if { $playing == "1" } {
					set status "\[X\]"
				} elseif { $idleMins > 3 } {
					set status "\[$idleMins\]"
				} else {
					set status "\[  \]"
				}
				.middle.f2.fOnlineSetup.lbInterestedUsers insert end $name "$status $name"
				if {$name == $slxn} {
					.middle.f2.fOnlineSetup.lbInterestedUsers select end
				}
			}
		}
		## Enable the Accept button only if a user is selected
		EnableInterestedUsersAcceptButton
		## Requeue another loop
		set gRefreshingInterestedUsersId [after idle ContinueRefreshInterestedUsers]
	} else {
		## Error response, don't loop. Display an error modal. 	
		set errMsg [string range $result $num end]
		DisplayOKModal "Error: $errMsg" "OK"
	}
}

proc StopRefreshingInterestedUsers {} {
	global gRefreshingInterestedUsersId
	after cancel $gRefreshingInterestedUsersId
}

proc UnregisterGame { } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo
	
	DisplayStatusModal "Unregistering game... Please wait"
	set result [C_UnregisterGame $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		set gGameInfo ""
		## Hide/destroy this frame and display the register game frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayGameRegistration
		return ""
	} elseif { $errCode == 312 } {
		## A previously selected challenger has accepted since our last check.
		set challenger [string range $result $num end]
		DisplayOKModal "The previously selected challenger: $challenger has accepted and is waiting." "OK"
		## Hide/destroy this frame and display the wait for challenger frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayWaitForChallenger $challenger
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc SelectChallenger { challenger } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo
	
	DisplayStatusModal "Selecting challenger... Please wait"
	set result [C_SelectChallenger $gUsername $gPassword $gSessionId [lindex $gGameInfo 0] $challenger]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		## Hide/destroy this frame and display the wait for challenger frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayWaitForChallenger $challenger
		return ""
	} elseif { $errCode == 312 } {
		## A previously selected challenger has accepted since our last check.
		set challenger [string range $result $num end]
		DisplayOKModal "The previously selected challenger: $challenger has accepted and is waiting." "OK"
		## Hide/destroy this frame and display the wait for challenger frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayWaitForChallenger $challenger
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc DeselectChallenger { } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo
	
	DisplayStatusModal "Deselecting challenger... Please wait"
	set result [C_DeselectChallenger $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]]
	scan $result "%d:%n" errCode num
	DestroyModal
	if { $errCode == 0 } {
		## Hide/destroy this frame and display the interested users frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayInterestedUsers
		return ""
	} elseif { $errCode == 312 } {
		## A previously selected challenger has accepted since our last check.
		set challenger [string range $result $num end]
		DisplayOKModal "The previously selected challenger: $challenger has accepted and is waiting." "OK"
		## Hide/destroy this frame and display the wait for challenger frame
		place forget .middle.f2.fOnlineSetup
		destroy .middle.f2.fOnlineSetup			
		DisplayWaitForChallenger $challenger
		return ""
	} else {
		return [string range $result $num end]
	}
}

proc DisplayWaitForChallenger { challenger } {
	global kLabelFont
	
	## Create a new frame
	NewOnlineSetupFrame
	
	## Add our fields 
	label .middle.f2.fOnlineSetup.lInstructions -text "Checking if challenger: $challenger has accepted. Please be patient" -font $kLabelFont -padx 2 -pady 2
	button .middle.f2.fOnlineSetup.bCancel -text "Cancel" -width 10 \
	-command {
		## Stop checking if the challenger has accepted
		StopCheckingForAcceptedChallenger
		set errMsg [DeselectChallenger]
		if { $errMsg != "" } {
			## Show the error to the user
			DisplayOKModal "Error: $errMsg" "OK"
			## Hide/destroy this frame and display the interested users frame
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			DisplayInterestedUsers
		}	
	}
	
	pack .middle.f2.fOnlineSetup.lInstructions -side top 
	pack .middle.f2.fOnlineSetup.bCancel -side top 

	## Bootstrap refreshing the interested user's loop (refresh immediately
	## and let it queue itself in a loop until we stop it)
	update
	StartCheckingForAcceptedChallenger $challenger
}

proc StartCheckingForAcceptedChallenger { challenger } {
	global gCheckingForAcceptedChallengerId
	set gCheckingForAcceptedChallengerId [after idle [list CheckForAcceptedChallenger $challenger]]
}

proc ContinueCheckForAcceptedChallenger { challenger } {
	global gCheckingForAcceptedChallengerId
	global gAcceptedChallengerRefreshPeriod
	set gCheckingForAcceptedChallengerId [after $gAcceptedChallengerRefreshPeriod CheckForAcceptedChallenger $challenger]
}

proc CheckForAcceptedChallenger { challenger } {
	global gCheckingForAcceptedChallengerId
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo
	
	set result [C_AcceptedChallenge $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]]
	scan $result "%d:%n" errCode num
	if { $errCode == 0 } {
		scan [string range $result $num end] "%d:%s" status moveFirst
		if {$status == "1"} {
			## Challenger accepted
			DisplayOKModal "Challenger accepted." "Start game"
			## Hide/destroy this frame and display the interested users frame
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			StartOnlinePlay $gGameInfo $moveFirst $challenger
		} elseif { $status == "0" } {
			## Challenger declined
			DisplayOKModal "Challenger declined. Please select another interested challenger." "OK"
			## Hide/destroy this frame and display the interested users frame
			place forget .middle.f2.fOnlineSetup
			destroy .middle.f2.fOnlineSetup			
			DisplayInterestedUsers
		} elseif { $status == "2" } {
			## Challenger not yet responded
			set gCheckingForAcceptedChallengerId [after idle ContinueCheckForAcceptedChallenger $challenger]		
		}
	} else {
		## Error response, don't loop. Display an error modal	
		set errMsg [string range $result $num end]
		DisplayOKModal "Error: $errMsg" "OK"
	}
}

proc StopCheckingForAcceptedChallenger {} {
	global gCheckingForAcceptedChallengerId
	after cancel $gCheckingForAcceptedChallengerId
}

proc StartOnlinePlay { gameInfo moveFirst challenger } {
	global gWhoseTurn
	global gPlaysFirst
	global gRightName
	global gLeftName
	global gComputerIsOnlinePlayer
	global gUsername
	global gGameInfo
	
	## Setup the game from the gameInfo
	set gGameInfo $gameInfo
	set gComputerIsOnlinePlayer 1
	.middle.f2.fPlayOptions.fTop.fRight.rComputer select
	.middle.f2.fPlayOptions.fTop.fBot.fRight.bOnline configure -state disable

	set iAmHost 0
	if { $gUsername == [lindex $gameInfo 4] } {
		set iAmHost 1
	}
	if { $moveFirst == "1" } {
		set gWhoseTurn "Left"
		set gPlaysFirst 0
	} else {
		set gWhoseTurn "Right"
		set gPlaysFirst 1
	}

	if { $iAmHost } {
		## Hosting the game	
		set gRightName $challenger
		set gLeftName [lindex $gameInfo 4]
	} else {		## Opponent is the host
		set gRightName [lindex $gameInfo 4]
		set gLeftName $challenger	
	}

    ## Set C option and re-initialize 
    C_SetOption [lindex $gameInfo 2]
    C_InitializeGame
    C_InitializeDatabases
    GS_InitGameSpecific
    GS_Initialize .middle.f2.cMain

	## Disable the online game play options
	DisableOnlineGamePlayOptions

	## New game
	clickedPlayNow	
}

#########################################################
## Reusable GUI functions for Online setup windowing
#########################################################

proc NewOnlineSetupFrame { } {
	## Create the new frame
	set w [expr [winfo width .middle.f2.fPlayOptions] * 0.8]
	set h [expr [winfo height .middle.f2.fPlayOptions] * 0.8]
	
	frame .middle.f2.fOnlineSetup -width [expr $w - 4] -height [expr $h - 4]

	## Display it over the background frame
	place .middle.f2.fOnlineSetup -x 0 -y 0 -in .middle.f2.fOnlineSetupBg
	pack propagate .middle.f2.fOnlineSetup 0
}

proc DisplayStatusModal { msgText } {
	global kLabelFont

	set w [expr [winfo width .middle.f2.fOnlineSetup] - 0]
	set h [expr [winfo height .middle.f2.fOnlineSetup] - 0]
	frame .middle.f2.fOnlineSetup.fModal -width $w -height $h -relief raised -bd 2
	pack propagate .middle.f2.fOnlineSetup.fModal 0
		
	message .middle.f2.fOnlineSetup.fModal.lMessage -text $msgText -font $kLabelFont -width $w -padx 5 -pady 5
	place .middle.f2.fOnlineSetup.fModal -x 0 -y 0 -in .middle.f2.fOnlineSetup
	pack .middle.f2.fOnlineSetup.fModal.lMessage -side top -fill both
	update
	focus .middle.f2.fOnlineSetup.fModal 
}

proc DisplayOKModal { msgText okText } {
	global kLabelFont

	set w [expr [winfo width .middle.f2.fOnlineSetup] - 0]
	set h [expr [winfo height .middle.f2.fOnlineSetup] - 0]
	frame .middle.f2.fOnlineSetup.fModal -width $w -height $h -relief raised -bd 2
	pack propagate .middle.f2.fOnlineSetup.fModal 0

	message .middle.f2.fOnlineSetup.fModal.lMessage -text $msgText -width $w -font $kLabelFont -padx 5 -pady 5
	button .middle.f2.fOnlineSetup.fModal.bMessage -text $okText -width 10 -command { DestroyModal }
	place .middle.f2.fOnlineSetup.fModal -x 0 -y 0 -in .middle.f2.fOnlineSetup
	pack .middle.f2.fOnlineSetup.fModal.lMessage -side top -fill both
	pack .middle.f2.fOnlineSetup.fModal.bMessage -side top
	update
	focus .middle.f2.fOnlineSetup.fModal
	tkwait window .middle.f2.fOnlineSetup.fModal
}

proc DisplayYesNoModal { msgText yesText noText } {
	global kLabelFont
	global tmp
	
	set w [expr [winfo width .middle.f2.fOnlineSetup] - 0]
	set h [expr [winfo height .middle.f2.fOnlineSetup] - 0]
	frame .middle.f2.fOnlineSetup.fModal -width $w -height $h -relief raised -bd 2
	pack propagate .middle.f2.fOnlineSetup.fModal 0

	frame .middle.f2.fOnlineSetup.fModal.fButtons
	message .middle.f2.fOnlineSetup.fModal.lMessage -text $msgText -width $w -font $kLabelFont -padx 5 -pady 5
	button .middle.f2.fOnlineSetup.fModal.fButtons.bYes -text $yesText -command { DestroyModal; global tmp; set tmp 1; }
	button .middle.f2.fOnlineSetup.fModal.fButtons.bNo -text $noText -command { DestroyModal; global tmp; set tmp 0; }
	place .middle.f2.fOnlineSetup.fModal -x 0 -y 0 -in .middle.f2.fOnlineSetup
	pack .middle.f2.fOnlineSetup.fModal.fButtons.bYes -side left
	pack .middle.f2.fOnlineSetup.fModal.fButtons.bNo -side left
	pack .middle.f2.fOnlineSetup.fModal.lMessage -side top -fill both
	pack .middle.f2.fOnlineSetup.fModal.fButtons -side top
	update
	focus .middle.f2.fOnlineSetup.fModal
	tkwait window .middle.f2.fOnlineSetup.fModal
	return $tmp
}

proc DestroyModal { } {
	place forget .middle.f2.fOnlineSetup.fModal
	destroy .middle.f2.fOnlineSetup.fModal
}


#########################################################
## Reusable scrolledLB
#########################################################
lappend auto_path .
package provide scrolledLB 1.0
###################################################################
# proc scrolledLB {args}
#    Create a scrolledLB megawidget
#    External entry point for creating a megawidget.
# Arguments
#    ?parentFrame?    A frame to use for the parent,
#    				  If this is not provided, the megawidget
#    				  is a child of the root frame (".").
# Results
#    Creates a procedure with the megawidget name for processing
#    widget commands. Returns the name of the megawidget.
proc scrolledLB {args} {
	set newWidget [eval scrolledLB::MakescrolledLB $args]
	set newCmd [format {return [namespace eval %s %s %s $args ]} scrolledLB scrolledLBProc $newWidget]
	proc $newWidget {args} $newCmd
	return $newWidget
}
namespace eval scrolledLB {
	variable scrolledLBState
		
	# Assign a couple defaults
	set scrolledLBState(unique) 0
	set scrolledLBState(debug) 0
	
	##############################################################
	# proc MakescrolledLB {args}
	#    Create a scrolledLB megawidget
	# Arguments
	#    ?parentFrame?    A frame to use for the parent.
	#    				  If this is not provided, the megawidget
	#  					  is a child of the root frame (".").
	#    ?-scrollposition left/right?
	#  					  The side of the listbox for the scrollbar
	# Results
	#    Returns the name of the parent frame for use as a
	#    megawidget.
	proc MakescrolledLB {args} {
		variable scrolledLBState
		
		# Set the default name
		set holder .scrolledLB_$scrolledLBState(unique)
		incr scrolledLBState(unique)
		
		# If the first argument is a window path, use that as
		# the base name for this widget.
		if {[string first "." [lindex $args 0]] == 0} {
			set holder [lindex $args 0]
			set args [lreplace $args 0 0]
		}

		# Set the state lists here
		set entryState [list 5 6 ]
		set scrolledLBState(entryState) [list ]
		set scrolledLBState(entryId) [list ]
		
		# Set Command line option defaults here,
		#    height and width are freebies
		set scrolledLBState($holder.height) 5
		set scrolledLBState($holder.width) 20
		set scrolledLBState($holder.scrollSide) 1
		set scrolledLBState($holder.listboxHeight) 10
		set scrolledLBState($holder.listboxWidth) 20
		set scrolledLBState($holder.listSide) 0
		set scrolledLBState($holder.titleText) ""
		set scrolledLBState($holder.selectionCommand) ""	
		foreach {key val } $args {
			set keyName [string range $key 1 end]
			if {![info exists scrolledLBState($holder.$keyName)]} {
				regsub -all "$holder." [array names scrolledLBState $holder.*] "" okList error "Bad option" "Invalid option '$key'.\nMust be one of $okList"
			}
			set scrolledLBState($holder.$keyName) $val
		}
		
		# Create master Frame
		frame $holder -height $scrolledLBState($holder.height) \
		-width $scrolledLBState($holder.width) \
		-class ScrolledLB
		
		# Apply invocation options to the master frame, as appropriate
		foreach {opt val} $args {
			catch {$holder configure $opt $val}
		}
		Draw $holder
		
		# We can't have two widgets with the same name.
		#    Rename the base frame procedure for this
		#    widget to $holder.fr so that we can use
		#    $holder as the widget procedure name
		#    for the megawidget.
		uplevel #0 rename $holder $holder.fr
		
		# When this window is destroyed,
		#    destroy the associated command.
		bind $holder <Destroy> "+ rename $holder {}"
		return $holder
	}

	##############################################################
	# proc Draw {parent}--
	#    creates the subwidgets and maps them into the parent
	# Arguments
	#    parent    The parent frame for these widgets
	# Results
	#    New windows are created and mapped.
	proc Draw {parent} {
		variable scrolledLBState
		set tmp [scrollbar $parent.yscroll -orient vertical -command "$parent.list yview" ]
		set scrolledLBState($parent.subWidgetName.yscroll) $tmp
		grid $tmp -row 1 -sticky ns -column $scrolledLBState($parent.scrollSide)

		set tmp [scrollbar $parent.xscroll -orient horizontal -command "$parent.list xview" ]
		set scrolledLBState($parent.subWidgetName.xscroll) $tmp
		grid $tmp -row 2 -sticky ew -column 0

		set tmp [listbox $parent.list \
		-yscrollcommand "$parent.yscroll set" \
		-xscrollcommand "$parent.xscroll set" \
		-height $scrolledLBState($parent.listboxHeight)\
		-width $scrolledLBState($parent.listboxWidth) ]
		if { $scrolledLBState($parent.selectionCommand) != "" } {
			bind $tmp <<ListboxSelect>> $scrolledLBState($parent.selectionCommand)
		}		
		set scrolledLBState($parent.subWidgetName.list) $tmp
		grid $tmp -row 1 -column $scrolledLBState($parent.listSide)
		if { $scrolledLBState($parent.titleText) != "" } {
			set tmp [label $parent.title -text $scrolledLBState($parent.titleText) ]
			set scrolledLBState($parent.subWidgetName.title) $tmp
			grid $tmp -row 0 -column 0 -columnspan 2
		}
	}

	##############################################################
	# proc DoWidgetCommand {widgetName widgets cmd}--
	#    Perform operations on subwidgets
	# Arguments
	#    widgetName: The name of the holder frame
	#    widgets:    A list of the public names for subwidgets
	#    cmd:    	 A command to evaluate on each of these widgets
	# Results
	#    Does stuff about the subwidgets
	proc DoWidgetCommand {widgetName widgets cmd} {
		variable scrolledLBState
		foreach widget $widgets {
			set index $widgetName.subWidgetName.$widget
			eval $scrolledLBState($index) $cmd
		}
	}
	
	##############################################################
	# proc scrolledLBProc {widgetName subCommand args}
	#    The master procedure for handling this megawidget's
	#    subcommands
	# Arguments
	#    widgetName:    The name of the scrolledLB
	#    widget
	#    subCommand    The subCommand for this cmd
	#    args:    The rest of the command line
	#    arguments
	# Default subCommands are:
	# configure - configure the parent frame
	# widgetconfigure - configure or query a subwidget.
	#  					mega widgetconfigure itemID -key value
	#  				    mega widgetconfigure itemID -key
	# widgetcget - get the configuration of a widget
	# 			   mega widgetcget itemID -key
	# widgetcommand - perform a command on a widget
	#    			  mega widgetcommand itemID commandString
	# names - return the name or names that match a pattern
	#  		  mega names # Get names of all widgets
	#  		  mega names *a* # Get names of widgets with an 'a' in them.
	# subwidget - return the full pathname of a requested subwidget
	# Results
	#    Evaluates a subcommand and returns a result if required.
	proc scrolledLBProc {widgetName subCommand args} {
		variable scrolledLBState
		set cmdArgs $args
		switch -- $subCommand {
	
			configure {
				return [eval $widgetName.fr configure $cmdArgs]
			}
	
			widgetconfigure {
				set sbwid [lindex $cmdArgs 0]
				set cmd [lrange $cmdArgs 1 end]
				set index $widgetName.subWidgetName.$sbwid
				catch {eval $scrolledLBState($index) configure $cmd} rtn
				return $rtn
			}

			setEnabled {
				if { [lindex $cmdArgs 1] == 0 } {
					$widgetName.list itemconfigure [lindex $cmdArgs 0] -foreground gray
				} else {
					$widgetName.list itemconfigure [lindex $cmdArgs 0] -foreground black
				}			
				lset scrolledLBState(entryState) [lindex $cmdArgs 0] [lindex $cmdArgs 1]
			}

			isEnabled { 
				return [lindex $scrolledLBState(entryState) [lindex $cmdArgs 0]]
			}
	
			clear {
				lset scrolledLBState(entryId) [list ]
				lset scrolledLBState(entryState) [list ]
				$widgetName.list delete 0 end
			}
						
			insert {
				lset scrolledLBState(entryId) [linsert $scrolledLBState(entryId)  [lindex $cmdArgs 0] [lindex $cmdArgs 1]]
				lset scrolledLBState(entryState) [linsert $scrolledLBState(entryState) [lindex $cmdArgs 0] 1]
				$widgetName.list insert [lindex $cmdArgs 0] [lindex $cmdArgs 2]
			}
	
			selectionVal {
				return [$widgetName.list get [lindex $cmdArgs 0]]
			}

			selectionIndex {
				set lst [$widgetName.list curselection]
				foreach l $lst {
					return $l
				}
				return ""
			}
	
			selectionId {
				return [lindex $scrolledLBState(entryId) [lindex $cmdArgs 0]]
			}
				
			select {
				$widgetName.list selection set [lindex $cmdArgs 0]
			}
			
			deselect {
				$widgetName.list selection clear [lindex $cmdArgs 0]
			}
							
			widgetcget {
				if {[llength $cmdArgs] != 2} {
					error "$widgetName cget subWidgetName -option"
				}
				set sbwid [lindex $cmdArgs 0]
				set index $widgetName.subWidgetName.$sbwid
				set cmd [lrange $cmdArgs 1 end]
				catch {eval $scrolledLBState($index) cget $cmd} rtn
				return $rtn
			}

			widgetcommand {
				return [eval DoWidgetCommand $widgetName $cmdArgs]
			}
	
			names {
				if {[string match $cmdArgs ""]} {
					set pattern $widgetName.subWidgetName.*
				} else {
					set pattern $widgetName.subWidgetName.$cmdArgs
				}
				foreach n [array names scrolledLBState $pattern] {
					foreach {d w s name} [split $n .] {}
					lappend names $name
				}
				return $names
			}
			
			subwidget {
				set name [lindex $cmdArgs 0]
				set index $widgetName.subWidgetName.$name
				if {[info exists scrolledLBState($index)]} {
					return $scrolledLBState($index)
				}
			}
	
			default {
				error "bad command" "Invalid Command: \
				$subCommand \n \
				must be configure, widgetconfigure, \
				widgetcget, names, isEnabled, setEnabled, select, deselect\
				clear, insert, selection, selectionId, \
				or subwidget"
			}
		}
	}
}

