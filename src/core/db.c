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

#include "bpdb.h"
#include "gamesman.h"
#include "memdb.h"
#include "twobitdb.h"
#include "colldb.h"
#include "netdb.h"
#include "filedb.h"
#include "tierdb.h"
#include "symdb.h"

/* Provide optional support for randomized-hash based collision database, dependent on GMP */
#ifdef HAVE_GMP
#include "univdb.h"
#endif


/* internal function prototypes */
void        db_analysis_hook    (); /* hijacks the pointer in db_put_value in order to call AnalyzePosition() first */
VALUE       db_original_put_value(POSITION pos, VALUE data);
/* default functions common to all db's*/

/*will make this return the function table later*/
void        db_create();
void        db_destroy();
void        db_initialize();

/* these are generic functions that will be executed when the database is uninitialized */
void            db_free                 ();
VALUE           db_get_value            (POSITION pos);
VALUE           db_put_value            (POSITION pos, VALUE data);
REMOTENESS      db_get_remoteness       (POSITION pos);
void            db_put_remoteness       (POSITION pos, REMOTENESS data);
BOOLEAN         db_check_visited        (POSITION pos);
void            db_mark_visited         (POSITION pos);
void            db_unmark_visited       (POSITION pos);
MEX             db_get_mex              (POSITION pos);
void            db_put_mex              (POSITION pos, MEX theMex);
WINBY           db_get_winby            (POSITION pos);
void            db_put_winby            (POSITION pos, WINBY winBy);
BOOLEAN         db_save_database        ();
BOOLEAN         db_load_database        ();
void            db_get_bulk             (POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length);

/*internal variables*/

DB_Table *db_functions;

/*
** function code
*/
void db_create() {

	/*if there is an old database table, get rid of it*/
	db_destroy();

	/* get a new table */
	db_functions = (DB_Table *) SafeMalloc(sizeof(DB_Table));

	/*set all function pointers to NULL, and each database can choose*/
	/*whatever ones they wanna implement and associate them*/

	db_functions->get_value = db_get_value;
	db_functions->put_value = db_put_value;
	db_functions->get_remoteness = db_get_remoteness;
	db_functions->put_remoteness = db_put_remoteness;
	db_functions->check_visited = db_check_visited;
	db_functions->mark_visited = db_mark_visited;
	db_functions->unmark_visited = db_unmark_visited;
	db_functions->get_mex = db_get_mex;
	db_functions->put_mex = db_put_mex;
	db_functions->get_winby = db_get_winby;
	db_functions->put_winby = db_put_winby;
	db_functions->save_database = db_save_database;
	db_functions->load_database = db_load_database;
	db_functions->free_db = db_free;
	db_functions->get_bulk = db_get_bulk;
}

void db_destroy() {
	if(db_functions) {
		if(db_functions->free_db)
			db_functions->free_db();
		SafeFree(db_functions);
	}
}

void db_initialize() {
	GMSTATUS status = STATUS_SUCCESS;

	if (kSupportsTierGamesman && gTierGamesman) {
		tierdb_init(db_functions);
	} else if (gBitPerfectDB) {
		if (gSymmetries)
			status = symdb_init(db_functions);
		else
			status = bpdb_init(db_functions);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("db_initialize()", "Attempt to initialize the bpdb by calling bpdb_init failed", status);
			goto _bailout;
		}
	} else if(gTwoBits) {
		twobitdb_init(db_functions);
	} else if(gCollDB) {
		colldb_init(db_functions);
	}

#ifdef HAVE_GMP
	else if(gUnivDB) {
		db_functions = univdb_init();
	}
#endif

	else if(gNetworkDB) {
		netdb_init(db_functions);
	}

	else if(gFileDB) {
		filedb_init(db_functions);
	}

	else {
		memdb_init(db_functions);
	}
	//printf("\nCalling hooking function\n");
	//db_analysis_hook();
_bailout:
	return;
}

void db_analysis_hook() {
	db_functions->original_put_value = db_functions->put_value;
	db_functions->put_value = AnalyzePosition;

	if (db_functions->put_value == NULL) {
		printf("Function hook failed\n");
	} else {
		printf("Function successfully hooked\n");
	}
}

VALUE db_original_put_value(POSITION pos, VALUE data) {
	return(db_functions->original_put_value(pos, data));
}

void db_free(){
	return;
}

VALUE db_get_value(POSITION pos){
	printf("DB: Cannot read value of position " POSITION_FORMAT ". The database is uninitialized.\n", pos);
	ExitStageRight();
	exit(0);
}

VALUE db_put_value(POSITION pos, VALUE data){
	printf("DB: Cannot store value of position " POSITION_FORMAT ". The database is uninitialized.\n", pos);
	ExitStageRight();
	exit(0);
}

REMOTENESS db_get_remoteness(POSITION pos){
	return kBadRemoteness;
}

void db_put_remoteness(POSITION pos, REMOTENESS data){
	return;
}

BOOLEAN db_check_visited(POSITION pos){
	return FALSE;
}

void db_mark_visited(POSITION pos){
	return;
}

void db_unmark_visited(POSITION pos){
	return;
}

MEX db_get_mex(POSITION pos){
	return kBadMexValue;
}

void db_put_mex(POSITION pos, MEX theMex){
	return;
}

WINBY db_get_winby(POSITION pos) {
	return 0;
}

void db_put_winby(POSITION pos, WINBY winBy) {
	return;
}

BOOLEAN db_save_database(){
	//printf("NOTE: The database cannot be saved.");
	return FALSE;
}

BOOLEAN db_load_database(){
	//printf("NOTE: The database cannot be loaded.");
	return FALSE;
}

void db_get_bulk (POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length) {
	int i;
	for (i = 0; i < length; i++) {
		ValueArray[i] = GetValueOfPosition(positions[i]);
		remotenessArray[i] = Remoteness(positions[i]);
	}
}

void CreateDatabases()
{
	db_create();
}

void InitializeDatabases()
{
	db_initialize();
}

// Returns true if lookup table exists, false otherwise
BOOLEAN ReinitializeTierDB()
{
	// If lookup table exists
	// Set New Value, Remoteness, Mex functions
	return tierdb_reinit(db_functions);
}

void DestroyDatabases()
{
	db_destroy();
}

GMSTATUS
Allocate ( )
{
	return db_functions->allocate();
}

UINT64
GetSlot(
        UINT64 position,
        UINT8 index
        )
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	return db_functions->get_slice_slot(position, index);
}

UINT64
SetSlot(
        UINT64 position,
        UINT8 index,
        UINT64 value
        )
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	if(index == gValueSlot)
		AnalyzePosition(position, value);
	return db_functions->set_slice_slot(position, index, value);
}

UINT64
SetSlotMax(
        UINT64 position,
        UINT8 index
        )
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	return db_functions->set_slice_slot_max(position, index);
}

GMSTATUS
AddSlot(
        UINT8 size,
        char *name,
        BOOLEAN write,
        BOOLEAN adjust,
        BOOLEAN reservemax,
        UINT32 *slotindex
        )
{
	GMSTATUS value = db_functions->add_slot(size, name, write, adjust, reservemax, slotindex);;
	if (strcmp(name, "VALUE") == 0)
		gValueSlot = *slotindex;
	return value;
}

VALUE StoreValueOfPosition(POSITION position, VALUE value)
{
	showStatus(Update);

	if(gSymmetries)
		position = gCanonicalPosition(position);
	AnalyzePosition(position,value);
	return db_functions->put_value(position,value);
}


VALUE GetValueOfPosition(POSITION position)
{
	if(((gMenuMode != Analysis) || gMenuMode == Evaluated) && gSymmetries)
		position = gCanonicalPosition(position);
	return db_functions->get_value(position);
}


REMOTENESS Remoteness(POSITION position)
{
	if(((gMenuMode != Analysis) || gMenuMode == Evaluated) && gSymmetries)
		position = gCanonicalPosition(position);
	return db_functions->get_remoteness(position);
}


void SetRemoteness (POSITION position, REMOTENESS remoteness)
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	db_functions->put_remoteness(position,remoteness);
}


BOOLEAN Visited(POSITION position)
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	return db_functions->check_visited(position);
}


void MarkAsVisited (POSITION position)
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	db_functions->mark_visited(position);
}

void UnMarkAsVisited (POSITION position)
{
	if(gSymmetries)
		position = gCanonicalPosition(position);
	db_functions->unmark_visited(position);
}

void UnMarkAllAsVisited()
{
	int i;

	for(i = 0; i < gNumberOfPositions; i++)
	{
		db_functions->unmark_visited(i);
	}

}


void MexStore(POSITION position, MEX theMex)
{
	/* do we need this?? */
	if(gSymmetries)
		position = gCanonicalPosition(position);

	db_functions->put_mex(position, theMex);
}

MEX MexLoad(POSITION position)
{
	/* do we need this?? */
	if(gSymmetries)
		position = gCanonicalPosition(position);

	return db_functions->get_mex(position);
}

void WinByStore(POSITION position, WINBY winBy)
{
	/* do we need this?? */
	if(gSymmetries)
		position = gCanonicalPosition(position);

	db_functions->put_winby(position, winBy);
}

WINBY WinByLoad(POSITION position)
{
	WINBY result;
	/* do we need this?? */
	if(gSymmetries)
		position = gCanonicalPosition(position);

	result = db_functions->get_winby(position);
	if (result > ((1 << (MEX_BITS-1))-1))
		result |= ~MEX_MAX;
	return result;
}

BOOLEAN SaveDatabase() {
	return db_functions->save_database();
}

BOOLEAN LoadDatabase() {
	return db_functions->load_database();
}

void GetValueAndRemotenessOfPositionBulk(POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length) {
	db_functions->get_bulk(positions, ValueArray, remotenessArray, length);
}
