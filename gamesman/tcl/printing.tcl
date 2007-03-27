global leftMistakes rightMistakes

proc prepare { c scale buffer xOffset yOffset nameOfWinner items} {
	# hide oxy background
	$c itemconfigure background -state hidden
	createNew $c $scale $buffer $yOffset $nameOfWinner
	displayMistakes $c
	scaleItems $c $items $scale $xOffset $yOffset false
}

proc createNew { c scale buffer yOffset nameOfWinner} {
	global gLeftName gRightName gFrameWidth kGameName
	set players [GS_NameOfPieces]
	set left [lindex $players 0]
	set right [lindex $players 1]
	# create new stuff
	$c create rectangle 0 0 $gFrameWidth $gFrameWidth \
		-fill white -outline "" -tag __printing
	# lower the temp background below the base
	$c lower __printing base
	# create names
	$c create text [expr $gFrameWidth / 2] $buffer -anchor center -text \
		"$gLeftName vs $gRightName" -font {Helvetica 24} -tag __printing
	# display winner
	$c create text [expr $gFrameWidth / 2] \
		[expr $yOffset + $gFrameWidth * $scale + $buffer] \
		-anchor center -text "$nameOfWinner Won" -font {Helvetica 24} -tag __printing
	$c create text [expr $gFrameWidth / 2] \
		[expr $yOffset + $gFrameWidth * $scale + 3 * $buffer] \
		-anchor center -text "Left is $left" -font {Helvetica 24} -tag __printing

	$c create text [expr $gFrameWidth / 2] \
		[expr $yOffset + $gFrameWidth * $scale + 4 * $buffer] \
		-anchor center -text "Right is $right" -font {Helvetica 24} -tag __printing
	
	update idletasks
}

proc displayMistakes { c } {
	global leftMistakes rightMistakes 
	global maxBoardStringWidth maxBoardStringHeight
	global gFrameWidth
	set buffer 25
	set gap 50
	set epsilon 5
	set fontSize [expr int(($gFrameWidth / 2 - 2 * $buffer - $gap) \
	 				/ 2 / $maxBoardStringWidth)]
	set offset 325
	set statY 150
	set statStep [expr $maxBoardStringHeight * $fontSize + $gap]
	# create list of mistakes
	# display the move the player made
	# then the better move next to it
	### requires that the tcl file has a simulateMove function
	### which returns the position value of simulating a move from
	### a given position
	###
	### also requires function called position to string
	### which takes in a position and returns the string
	### representation of the position

	set index 0
	foreach side [list $leftMistakes $rightMistakes] {
		set turn [lindex [GS_NameOfPieces] $index]
		# create the mistake headers
		# TODO remove hardcode
		if { $index == 0 } {
			$c create text 100 50 -text "Left Mistakes" \
			-font {Helvetica 20} \
			-tag __printing -anchor n
		} else {
			$c create text 400 50 -text "Right Mistakes" \
			-font {Helvetica 20} \
			-tag __printing -anchor n		
		}
		if { [llength $side] == 0 } {
			$c create text [expr $gFrameWidth / 2 * $index + 125] \
				$statY -text "None" \
				-font "Courier $fontSize" -tag __printing -anchor s
		
		} else {
			for { set i 0 } { $i < [llength $side] } {incr i} {
				set mistake [lindex $side $i]
				set oldPos [lindex $mistake 6]
				set badMove [lindex $mistake 1]
				set goodMove [lindex $mistake 4]
				set badPos [simulateMove $oldPos $badMove $turn]
				set goodPos [simulateMove $oldPos $goodMove $turn]
				set y [expr $statY + $i * $statStep]
				# number the mistakes
				$c create text [expr $offset * $index + (2 - $index) * $buffer \
									- $fontSize - $epsilon] \
					[expr $y - $fontSize * 2] \
					-text [expr $i + 1]) -font "Courier $fontSize" \
					-tag __printing -anchor w
				
				# draw bad move
				# let starting x be at offset due to canvas * index
				# this lets the left side start correctly since left
				# has index of 0 so offset overall is 0
				# then add on a buffer, adjust buffer on the left to
				# allow more space, handled by (2 - 1) * buffer
				$c create text [expr $offset * $index + (2 - $index) * $buffer] \
					$y -text [positionToString $badPos] \
					-font "Courier $fontSize" -tag __printing -anchor w
				# draw good move
				# the starting spot for x is same as above
				# also increment the start spot by the gap size
				# and how much space the bad board took up
				$c create text [expr $offset * $index + (2 - $index) * $buffer \
								+ $gap + $maxBoardStringWidth * $fontSize] \
					$y -text [positionToString $goodPos] \
					-font "Courier $fontSize" -tag __printing -anchor e
			}
		}
		incr index
	}
	update idletasks
}

proc scaleObjs { c items scale } {
	foreach item $items {
		$c scale $item 0 0 $scale $scale
		#ugly hack to to configure only if item
		#has that property
		catch {
			set wValue [$c itemcget $item -width]
			$c itemconfigure $item -width [expr $wValue * $scale]
		}
	}
}

proc moveObjs { c items newX newY } {
	foreach item $items {
		$c move $item $newX $newY
	}
}

proc scaleItems { c items scale xOffset yOffset undo} {	
	if { $undo  == true } {
		moveObjs $c $items $xOffset $yOffset
		scaleObjs $c $items $scale
	} else {
		scaleObjs $c $items $scale
		moveObjs $c $items $xOffset $yOffset	
	}
	update idletasks
}

# clean up stuff
# undo the scaling
# delete the temporary items
# redisplay oxy
proc cleanUp { c items scale xOffset yOffset} {
	scaleItems $c $items $scale $xOffset $yOffset true
	$c delete __printing
	$c itemconfigure background -state normal
}


# dumps the canvas c to a file called name.ps
# automatically appends ps extension to file
# if no ps directory exists, then it creates it
proc makePS { c items name nameOfWinner} {
	# check for dir
	if { ![file exists "ps/"] } {
		file mkdir "ps"
	}
	global gFrameWidth
	set scale 0.25
	set xOffset [expr $gFrameWidth / 2 -  $gFrameWidth * $scale / 2]
	set yOffset 50
	set buffer 25
	
	# make the stuff that we need
	prepare $c $scale $buffer $xOffset $yOffset $nameOfWinner $items
	# dump ps file
	$c postscript -file "ps/$name.ps" -pagewidth 8.5i -pagey 11.0i -pageanchor n
	# cleanup stuff that we made
	cleanUp $c $items [expr 1 / $scale] [expr -$xOffset] [expr -$yOffset]
}

proc setMistakesLists { } {
	global gMistakeList leftMistakes rightMistakes
	set leftMistakes []
	set rightMistakes []
	foreach mistake $gMistakeList {
		if {[lindex $mistake 0] == "Left" } {
			lappend leftMistakes [lrange $mistake 1 [llength $mistake]]
		} elseif {[lindex $mistake 0] == "Right" } {
			lappend rightMistakes [lrange $mistake 1 [llength $mistake]]
		}
	}
}

proc generateMistakeExec { } {
	checkFolders
	
	global leftMistakes rightMistakes kGameName
	
	set left "ps/left.ps"
	set right "ps/right.ps"
	set leftOut "ps/leftMerge.ps"
	set rightOut "ps/rightMerge.ps"
	set output "ps/output.ps"
	set outputMerge "ps/outputMerge.ps"
	set outputPDF "ps/output.pdf"
	set blank "ps/blank.ps"
	set gsStr "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pswrite -sOutputFile="
	set leftExec ""
	set rightExec ""
	set pathStr "ps/$kGameName/$kGameName"
	set i 0
	# loop through mistakes and generate the exec calls we need
	foreach lists [list $leftMistakes $rightMistakes] {
		foreach mistake $lists {
			set oldPos [lindex $mistake 6]
			set badMove [lindex $mistake 1]
			set badPos [C_DoMove $oldPos $badMove]
			set str "$pathStr\_$oldPos\_v.ps $pathStr\_$badPos.ps"
			if { $i == 0 } {
				set leftExec "$leftExec $str"
			} else {
				set rightExec "$rightExec $str"
			}
		}
		incr i
	}
	
	# if we added something then merge and psnup
	# else we change to point to a blank page
	if { $leftExec != "" } {
		set leftExec "$gsStr$left $leftExec"
		eval $leftExec
		exec /usr/bin/psnup -q -4 -pletter $left $leftOut
	} else {
		set leftOut	$blank
	}
	
	if { $rightExec != "" } {
		set rightExec "$gsStr$right $rightExec"
		eval $rightExec
		exec /usr/bin/psnup -q -4 -pletter $right $rightOut
	} else {
		set rightOut $blank
	}
	
	# combine the left and right outputs
	set mergeExec "$gsStr$output $leftOut $rightOut"
	eval $mergeExec
	exec /usr/bin/psnup -q -2 -d -pletter $output $outputMerge
	
	# use gs to generate a pdf...
	makePDF $outputMerge $outputPDF
	# delete temp files
	# check to see if we made any first
	# don't want to delete the blank.ps file
	if { $leftExec != "" } {
		file delete $leftOut
	}
	
	if { $rightExec != "" } {
		file delete $rightOut
	}
	file delete $left
	file delete $right
	file delete $output
}

proc doPrinting {} {
	setMistakesLists
	generateMistakeExec
}

# make string to use for eval
# then call it
proc makePDF { input output} {
	set gsStr "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sOutputFile=$output"
	set gsStr "$gsStr -sDEVICE=pdfwrite -c .setpdfwrite -f $input"
	eval $gsStr
}

# do the setup required for capturing a move
# 
proc doCapture { moveType position theMoves value} {
	# hide old and display new
	GS_HideMoves .middle.f2.cMain $moveType $position $theMoves
	if {$value == true} {
		GS_ShowMoves .middle.f2.cMain "value" $position $theMoves
	} else {
		GS_ShowMoves .middle.f2.cMain "all" $position $theMoves		
	}
	update idletasks
		
	# capture screen shot
	capture .middle.f2.cMain $position $value
		
	# hide new and display old
	if {$value == true} {
		GS_HideMoves .middle.f2.cMain "value" $position $theMoves
	} else {
		GS_HideMoves .middle.f2.cMain "all" $position $theMoves
	}
	GS_ShowMoves .middle.f2.cMain $moveType $position $theMoves
	update idletasks
}

proc capture { c pos value} {
	checkFolders
	
	global kGameName
	set name "$kGameName\_$pos"
	if { $value == true} {
		set name "$name\_v.ps"
	} else {
		set name "$name.ps"		
	}
	
	# capture to game directory
	if { ![file exists "ps/$kGameName/$name"] } {
		# hide background to reduce drawing time
		$c itemconfigure background -state hidden
		$c postscript -file "ps/$kGameName/$name" -pagewidth 7.5i -pagey 10.0i -pageanchor n
		# show it again
		$c itemconfigure background -state normal
	}
}

proc checkFolders {} {
	global kGameName
	# check for directories
	if { ![file exists "ps/"] } {
		file mkdir "ps"
	}
	
	if { ![file exists "ps/$kGameName"] } {
		file mkdir "ps/$kGameName"
	}
}

proc makeWinnerBanner {} {
}