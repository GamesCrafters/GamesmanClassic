proc HandleFGClick { w x y } {
    global EditBoardCurrentMode

    puts stdout $EditBoardCurrentMode

    set theTags [$w gettags current]
    
    if { $EditBoardCurrentMode == "fox" } {
	if { [lsearch $theTags tagEditBoardDead] != -1 } {
	    $w dtag current tagEditBoardDead
	    $w addtag tagEditBoardFox withtag current
	    #put a fox piece at the current slot
	    
	    $w itemconfig current -fill red

	} elseif { [lsearch $theTags tagEditBoardGoose] != -1 } {
	    $w dtag current tagEditBoardGoose
	    $w addtag tagEditBoardFox withtag current
	    #take the fox piece away at the current slot#	    

	    $w itemconfig current -fill red

	} else {
	    #don't do anything
	}
	
	set EditBoardCurrentMode "geese"

    } elseif { $EditBoardCurrentMode == "geese" } {
	if { [lsearch $theTags tagEditBoardDead] != -1 } {
	    $w dtag current tagEditBoardDead
	    $w addtag tagEditBoardGoose withtag current
	    #put a goose piece at the current slot

	    $w itemconfig current -fill blue

	} elseif { [lsearch $theTags tagEditBoardFox] != -1 } {
	    $w dtag current tagEditBoardFox
	    $w addtag tagEditBoardGoose withtag current
	    #take the goose piece away at the current slot

	    $w itemconfig current -fill blue
	} else {
	    #don't do anything
	}

	set EditBoardCurrentMode "dead"

    } else {
	#not in any mode so don't do anything
	if { [lsearch $theTags tagEditBoardFox] != -1} {
	    $w dtag current tagEditBoardFox
	    $w addtag tagEditBoardDead withtag current

	    $w itemconfig current -fill grey
	} elseif { [lsearch $theTags tagEditBoardGoose] != -1} {
	    $w dtag current tagEditBoardGoose
	    $w addtag tagEditBoardDead withtag current
	
	    $w itemconfig current -fill grey
	} else {
	}

	set EditBoardCurrentMode "fox"
    }
}