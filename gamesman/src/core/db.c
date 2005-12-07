/************************************************************************
**
** NAME:	db.c
**
** DESCRIPTION:	Generic Database Functions and Database Class Accessors
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

/*
** Needs to be built up to implement The new DB Class abstraction as is
** Found in the Expeimental directory. However we first need to make
** The existing functions abstract.
*/


#include "gamesman.h"
#include "db.h"
#include "memdb.h"
#include "twobitdb.h"
#include "colldb.h"
#include "globals.h"

/* Provide optional support for randomized-hash based collision database, dependent on GMP */
#ifdef HAVE_GMP
#include "univdb.h"
#endif

DB_Table *db_functions;

/*
** Code
*/
void InitializeDatabases()
{
    db_initialize();
}

void db_initialize(){

    if (gTwoBits) {
        db_functions = twobitdb_init();
    } else if(gCollDB){
	db_functions = colldb_init();
    }
#ifdef HAVE_GMP
    else if(gUnivDB) {
	db_functions = univdb_init();
    }
#endif
    else {
	memdb_init(db_functions);
    }
}

void db_create() {
    
    /*if there is an old database table, get rid of it*/
    db_free();

    /* get a new table */
    db_functions = (DB_Table *) SafeMalloc(sizeof(DB_Table));

    /*set all function pointers to NULL, and each database can choose*/
    /*whatever ones they wanna implement and associate them*/
    db_functions->get_value = NULL;
    db_functions->put_value = NULL;
    db_functions->get_remoteness = NULL;
    db_functions->put_remoteness = NULL;
    db_functions->check_visited = NULL;
    db_functions->mark_visited = NULL;
    db_functions->unmark_visited = NULL;
    db_functions->get_mex = NULL;
    db_functions->put_mex = NULL;
    db_functions->save_database = NULL;
    db_functions->load_database = NULL;
    db_functions->free_db = NULL;
}

void db_free() {
    if(db_functions) {
	if(db_functions->free_db)
	    db_functions->free_db();
	SafeFree(db_functions);
    }
}

VALUE StoreValueOfPosition(POSITION position, VALUE value)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->put_value) {
	printf("FATAL: This db does not support storing of values. It might be read-only...\n");
	crash = TRUE;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to store value for nonexistant position " POSITION_FORMAT ".\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->put_value(position,value);
}


VALUE GetValueOfPosition(POSITION position)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->get_value) {
	printf("FATAL: This db does not support reading of values. It might be write only...\n");
	crash = TRUE;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to get value for nonexistant position " POSITION_FORMAT ".\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(((kLoopy && gMenuMode != Analysis) || gMenuMode == Evaluated) && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->get_value(position);
}


REMOTENESS Remoteness(POSITION position)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->get_remoteness) {
	if(gPrintDatabaseInfo)
	    printf("WARNING: This db does not support reading of remoteness.");
	return kBadRemoteness;
    }
    if(position > gNumberOfPositions) {
	printf("FATAL: Attempt to get remoteness for nonexistant position " POSITION_FORMAT ".\n", position);
        crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(((kLoopy && gMenuMode != Analysis) || gMenuMode == Evaluated) && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->get_remoteness(position);
}
    

void SetRemoteness (POSITION position, REMOTENESS remoteness)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->put_remoteness){
	if (gPrintDatabaseInfo)
	    printf("WARNING: This db does not support storing of remoteness value.");
	//just don't do anything
	return ;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to get remoteness for nonexistant position " POSITION_FORMAT ".\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    db_functions->put_remoteness(position,remoteness);
}
 

BOOLEAN Visited(POSITION position)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->check_visited) {
	printf("WARNING: This db does not support checking of visit status.");
	return FALSE;
    }
    if(position > gNumberOfPositions) {
	printf("FATAL: Attempt to check visit status for nonexistant position " POSITION_FORMAT ".\n", position);
        crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->check_visited(position);
}


void MarkAsVisited (POSITION position)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->mark_visited){
	if (gPrintDatabaseInfo)
	    printf("WARNING: This db does not support marking of visit status.");
	//just don't do anything
	return ;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to mark nonexistant position " POSITION_FORMAT " as visited.\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    db_functions->mark_visited(position);
}

void UnMarkAsVisited (POSITION position)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->unmark_visited){
	if (gPrintDatabaseInfo)
	    printf("WARNING: This db does not support unmarking of visit status.");
	//just don't do anything
	return ;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to unmark nonexistant position " POSITION_FORMAT " as visited.\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    db_functions->unmark_visited(position);
}

void UnMarkAllAsVisited()
{
    POSITION i = 0;
 
    if(!db_functions->unmark_visited){
	if (gPrintDatabaseInfo)
	    printf("WARNING: This db does not support unmarking of visit status.");
	//just don't do anything
	return ;
    }
    
    for(i = 0; i < gNumberOfPositions; i++)
    {
        db_functions->unmark_visited(i);
    }

}


void MexStore(POSITION position, MEX theMex)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->put_mex){
	if (gPrintDatabaseInfo)
	    printf("WARNING: This db does not support storing of MEX value.");
	//just don't do anything
	return ;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to store mex value for nonexistant position " POSITION_FORMAT ".\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    /* do we need this?? */
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);

    db_functions->put_mex(position, theMex);
}

MEX MexLoad(POSITION position)
{
    BOOLEAN crash = FALSE;

    if(!db_functions->get_mex){
	if (gPrintDatabaseInfo)
	    printf("WARNING: This db does not support reading of MEX value.");
	return kBadMexValue;
    }
    if(position > gNumberOfPositions){
	printf("FATAL: Attempt to read mex value for nonexistant position " POSITION_FORMAT ".\n", position);
	crash = TRUE;
    }
    if(crash){
	ExitStageRight();
        exit(0);
    }

    /* do we need this?? */
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);

    return db_functions->get_mex(position);
}

BOOLEAN saveDatabase() {
    if(db_functions->save_database)
	return db_functions->save_database();    
    else {
	if (gPrintDatabaseInfo)
	    printf("NOTE: This db does not support saving. No data will be saved.\n");
	return FALSE;
    }
}

BOOLEAN loadDatabase() {
    if(db_functions->load_database)
	return db_functions->load_database();
    else {
	if (gPrintDatabaseInfo)
	    printf("NOTE: This db does not support loading. No data will be loaded.\n");
	return FALSE;
    }
}
