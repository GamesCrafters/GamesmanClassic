/************************************************************************
**
** NAME:         gsolve.c
**
** DESCRIPTION:  The source code and heart of a Master's project
**               entitled GAMESMAN, which is a polymorphic perfect-
**               information game generator.
**
** AUTHOR:       Dan Garcia  -  University of California at Berkeley
**               Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:         07/15/91
**
** UPDATE HIST:
**
** 07-17-91 0.9a1 : Implemented the undo facility, PrintPosition()
** 05-15-95 1.0   : Final release code for M.S.
**
**************************************************************************/


/************************************************************************
**
** NAME:        main
**
** DESCRIPTION: Call the menus routine which handles everything. 
**
** CALLS:       Initialize()
**              Menus()
**
************************************************************************/

main()
{
	Initialize();
	Menus();
	return(1); /* this satisfies lint's complaint... */
}

