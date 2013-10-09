/************************************************************************
**
** NAME:    bpdb.c
**
** DESCRIPTION:    Implementation of the Bit-Perfect Database.
**
** AUTHOR:    Ken Elkabany
**        GamesCrafters Research Group, UC Berkeley
**        Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:    2006-05-01
**
** LICENSE:    This file is part of GAMESMAN,
**        The Finite, Two-person Perfect-Information Game Generator
**        Released under the GPL:
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
   TODO:
 * 1. set better init sizes
   2. go again support
 * 3. load all schemes so that all dbs can be loaded, but don't use all schemes
   for saving
   1. SUPPORT tiers
   2. **fix bit widths from shrinks and grows

   Note:
   1. supports up to 256 slices each 64-bits in size
   2. **this support is violated in the shrink and grow (should fix soon)

 */
#include "bpdb.h"
#include "gamesman.h"
#include "bpdb_bitlib.h"
#include "bpdb_schemes.h"
#include "bpdb_misc.h"


//typedef enum

//
// Global Variables
//

DB_Table    *functionsMapping;

dbFILE          bpdb_readFile = NULL;
BOOLEAN bpdb_readFromDisk = FALSE;
SCHEME bpdb_readScheme = NULL;
UINT32 bpdb_readStart = 0;
UINT8 bpdb_readOffset = 0;

//
// stores the format of a slice; in particular the
// names, sizes, and maxvalues of its slots
//

SLICE bpdb_write_slice = NULL;
SLICE bpdb_nowrite_slice = NULL;

//
// in-memory database; write is for data that will
// be written to file while nowrite is only temporary
//

BYTE        *bpdb_write_array = NULL;
BYTE        *bpdb_nowrite_array = NULL;

size_t bpdb_write_array_length = 0;
size_t bpdb_nowrite_array_length = 0;

//
// numbers of slices
//

UINT64 bpdb_slices = 0;

//
// pointer to list of schemes
//

SLIST bpdb_schemes = NULL;

//
// pointer to the encoding scheme that will be used
// for encoding variable numbers in the db file header
//

SCHEME bpdb_headerScheme = NULL;

//
// these slots provide legacy support for non-slots
// aware solvers
//

UINT32 BPDB_VALUESLOT = 0;
UINT32 BPDB_WINBYSLOT = 0;
UINT32 BPDB_MEXSLOT = 0;
UINT32 BPDB_REMSLOT = 0;
UINT32 BPDB_VISITEDSLOT = 0;

//
// graphical purposes - used to test whether a new
// line has been printed or not
//

BOOLEAN bpdb_have_printed = FALSE;

//
// the buffer size in bytes that the bpdb is allowed
// to use when reading and writing data from a file
//

UINT32 bpdb_buffer_length = 10000;



/*++

   Routine Description:

    bpdb_init initializes the Bit-Perfect Database
    pseudo-C object. This function must be called before
    any other bpdb function. It allocates the bpdb write
    and nowrite slices, which are the slice formats for data
    that is written to file and not written, respectively.
    Furthermore, if a variable slice aware solver is not
    being used, the following default slots are added:
    Value, visited, mex and remotness. Furthermore, the
    DB_Table is populated with pointers to the bpdb
    functions. Last, the available schemes are added to the
    bpdb_schemes list for the purpose of encoding and decoding
    databases saved to a file.

   Arguments:

    new_db - pointer to the database structure that contains
            functional pointers that a database must
            implement.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bpdb_init(
        DB_Table *new_db
        )
{
	GMSTATUS status = STATUS_SUCCESS;

	//
	// check input parameter
	//

	if(NULL == new_db) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("bpdb_init()", "Input parameter new_db is null", status);
		goto _bailout;
	}

	if(0 == gNumberOfPositions) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("bpdb_init()", "Invalid gNumberOfPositions value(0)", status);
		goto _bailout;
	}

	functionsMapping = new_db;

	//
	// initialize global variables
	//

	bpdb_slices = gNumberOfPositions;

	//
	// allocate space to store the format of a slice
	//

	bpdb_write_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
	if(NULL == bpdb_write_slice) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_init()", "Could not allocate bpdb_write_slice in memory", status);
		goto _bailout;
	}

	bpdb_nowrite_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
	if(NULL == bpdb_nowrite_slice) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_init()", "Could not allocate bpdb_nowrite_slice in memory", status);
		goto _bailout;
	}

	//
	// if the solver being used is not variable slices aware, then add the default slots
	// that all old gamesman solvers use including value, mex, remoteness and visited
	//

	if(!gBitPerfectDBSolver) {

		// add value slot
		status = bpdb_add_slot( 2, "VALUE", TRUE, TRUE, FALSE, &BPDB_VALUESLOT ); //slot 0
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_init()", "Could not add value slot", status);
			goto _bailout;
		}

		// add mex slot
		status = bpdb_add_slot(5, "MEX", TRUE, TRUE, FALSE, &BPDB_MEXSLOT );    //slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_init()", "Could not add mex slot", status);
			goto _bailout;
		}

		// add winby slot
		status = bpdb_add_slot(5, "WINBY", TRUE, TRUE, FALSE, &BPDB_WINBYSLOT ); //slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_init()", "Could not add winby slot", status);
			goto _bailout;
		}

		// add remoteness slot
		status = bpdb_add_slot( 8, "REMOTENESS", TRUE, TRUE, TRUE, &BPDB_REMSLOT ); //slot 4
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_init()", "Could not add remoteness slot", status);
			goto _bailout;
		}

		// add visited slot
		status = bpdb_add_slot( 1, "VISITED", FALSE, FALSE, FALSE, &BPDB_VISITEDSLOT ); //slot 1
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_init()", "Could not add visited slot", status);
			goto _bailout;
		}

		//
		// allocate the memory the db requires
		//

		status = bpdb_allocate();
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_init()", "Failed call to bpdb_allocate() to allocate bpdb arrays", status);
			goto _bailout;
		}
	}

	//
	// set functional pointers for generic DB_Table
	//

	new_db->get_value = bpdb_get_value;
	new_db->put_value = bpdb_set_value;
	new_db->get_remoteness = bpdb_get_remoteness;
	new_db->put_remoteness = bpdb_set_remoteness;
	new_db->check_visited = bpdb_check_visited;
	new_db->mark_visited = bpdb_mark_visited;
	new_db->unmark_visited = bpdb_unmark_visited;
	new_db->get_mex = bpdb_get_mex;
	new_db->put_mex = bpdb_set_mex;
	new_db->get_winby = bpdb_get_winby;
	new_db->put_winby = bpdb_set_winby;
	new_db->save_database = bpdb_save_database;
	new_db->load_database = bpdb_load_database;
	new_db->allocate = bpdb_allocate;
	new_db->add_slot = bpdb_add_slot;
	new_db->get_slice_slot = bpdb_get_slice_slot;
	new_db->set_slice_slot = bpdb_set_slice_slot;
	new_db->set_slice_slot_max = bpdb_set_slice_slot_max;
	new_db->free_db = bpdb_free;

	// create a new singly-linked list of schemes
	bpdb_schemes = slist_new();

	// create the default no-compression scheme
	SCHEME scheme0 = scheme_new( 0, NULL, NULL, NULL, NULL, NULL, FALSE, TRUE );

	// create the original variable skips compression scheme
	SCHEME scheme1 = scheme_new( 1, bpdb_generic_varnum_gap_bits,
	                             bpdb_generic_varnum_size_bits,
	                             bpdb_generic_varnum_implicit_amt,
	                             bpdb_generic_varnum_init,
	                             bpdb_generic_varnum_free,
	                             TRUE, gBitPerfectDBSchemes );

	SCHEME scheme2 = scheme_new( 2, bpdb_ken_varnum_gap_bits,
	                             bpdb_ken_varnum_size_bits,
	                             bpdb_ken_varnum_implicit_amt,
	                             NULL,
	                             NULL,
	                             TRUE,
	                             gBitPerfectDBAllSchemes );

	// set the original variable skips compression scheme
	// as the scheme that will be used to encode the db header
	bpdb_headerScheme = scheme1;

	//
	// add the schemes to the schemes list
	//

	bpdb_schemes = slist_add( bpdb_schemes, scheme0 );
	bpdb_schemes = slist_add( bpdb_schemes, scheme1 );
	bpdb_schemes = slist_add( bpdb_schemes, scheme2 );

_bailout:
	return status;
}


/*++

   Routine Description:

    bpdb_allocate allocates the space required by the database
    to save data for each potential slice. The function requires
    that the slice format information has been defined.

   Arguments:

    None

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bpdb_allocate( )
{
	GMSTATUS status = STATUS_SUCCESS;
	UINT64 i = 0;

	// check whether slice formats have been set
	if(0 == bpdb_write_slice->bits && 0 == bpdb_nowrite_slice->bits) {
		status = STATUS_SLICE_FORMAT_NOT_SET;
		BPDB_TRACE("bpdb_allocate()", "Slices are width 0 which indicates that they have not been formatted yet", status);
		goto _bailout;
	}

	if(bpdb_readFromDisk) {
		return STATUS_SUCCESS;
	}

	bpdb_write_array_length = (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(bpdb_write_slice->bits));
	bpdb_nowrite_array_length = (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(bpdb_nowrite_slice->bits));

	// allocate room for data that will be written out to file
	bpdb_write_array = (BYTE *) calloc( bpdb_write_array_length, sizeof(BYTE) );
	if(NULL == bpdb_write_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_allocate()", "Could not allocate bpdb_write_array in memory", status);
		goto _bailout;
	}

	// allocate room for transient data that will only be stored in memory
	bpdb_nowrite_array = (BYTE *) calloc( bpdb_nowrite_array_length, sizeof(BYTE));
	if(NULL == bpdb_write_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_allocate()", "Could not allocate bpdb_nowrite_array in memory", status);
		goto _bailout;
	}

	// cannot put in init since bpdbsolver does not have any slots inputted
	// at the time when init is called
	if(!gBitPerfectDBAdjust) {
		for(i = 0; i < (bpdb_write_slice->slots); i++) {
			bpdb_write_slice->adjust[i] = FALSE;
		}
		for(i = 0; i < (bpdb_nowrite_slice->slots); i++) {
			bpdb_nowrite_slice->adjust[i] = FALSE;
		}
	}

	// everything will work without this for loop,
	// until someone changes the value of the undecided enum
	// which is at the time of writing this 0.

	//for(i = 0; i < bpdb_slices; i++) {
	//    bpdb_set_slice_slot( i, BPDB_VALUESLOT, undecided );
	//}

_bailout:
	return status;
}


/*++

   Routine Description:

    bpdb_free_slice frees the resources used by the slice format
    description structure.

   Arguments:

    sl - slice to be freed.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bpdb_free_slice( SLICE sl )
{
	GMSTATUS status = STATUS_SUCCESS;
	int i = 0;

	if(NULL == sl) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("bpdb_free_slice()", "Input parameter sl is null", status);
		goto _bailout;
	}

	SAFE_FREE( sl->size );
	SAFE_FREE( sl->offset );
	SAFE_FREE( sl->maxvalue );
	SAFE_FREE( sl->maxseen );

	for(i = 0; i < (sl->slots); i++) {
		SAFE_FREE( sl->name[i] );
	}

	SAFE_FREE( sl->name );
	SAFE_FREE( sl->overflowed );
	SAFE_FREE( sl->adjust );
	SAFE_FREE( sl->reservemax );

	SAFE_FREE( sl );

_bailout:
	return status;
}

/*++

   Routine Description:

    bpdb_free frees the resources used by th bpdb. This includes
    freeing both in-memory and to-be-written data, as well as
    slice formats. Schemes, as well as the list that contains them,
    are also freed.

   Arguments:

    None

   Return value:

    None

   --*/

void
bpdb_free( )
{
	GMSTATUS status = STATUS_SUCCESS;
	SLIST cur = NULL;

	// free open file if necessary
	if(bpdb_readFromDisk) {
		status = bitlib_file_close(bpdb_readFile);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_load_database()", "call to bitlib to open file failed", status);
			goto _bailout;
		}
	}

	// free write slice format
	status = bpdb_free_slice( bpdb_write_slice );
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("bpdb_free()", "Call to free bpdb_write_slice failed", status);
		goto _bailout;
	}

	// free nowrite slice format
	status = bpdb_free_slice( bpdb_nowrite_slice );
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("bpdb_free()", "Call to free bpdb_nowrite_slice failed", status);
		goto _bailout;
	}

	// free in-memory database
	SAFE_FREE( bpdb_nowrite_array );

	// free database to be written
	SAFE_FREE( bpdb_write_array );

	// free each scheme
	cur = bpdb_schemes;
	while( NULL != cur ) {
		scheme_free(cur->obj);
		cur = cur->next;
	}

	// free the list that contained the schemes
	slist_free( bpdb_schemes );

_bailout:
	//return status;
	return;
}


VALUE
bpdb_set_value(
        POSITION pos,
        VALUE val
        )
{
	bpdb_set_slice_slot( (UINT64)pos, BPDB_VALUESLOT, (UINT64) val );
	return val;
}

VALUE
bpdb_get_value(
        POSITION pos
        )
{
	return (VALUE) functionsMapping->get_slice_slot( (UINT64)pos, BPDB_VALUESLOT );
}

REMOTENESS
bpdb_get_remoteness(
        POSITION pos
        )
{
	REMOTENESS rem = (REMOTENESS) functionsMapping->get_slice_slot( (UINT64)pos, BPDB_REMSLOT );
	if(bpdb_write_slice->maxvalue[BPDB_REMSLOT/2]+1 == rem) {
		return REMOTENESS_MAX;
	} else {
		return rem;
	}
}

void
bpdb_set_remoteness(
        POSITION pos,
        REMOTENESS val
        )
{
	bpdb_set_slice_slot( (UINT64)pos, BPDB_REMSLOT, (REMOTENESS) val );
}

BOOLEAN
bpdb_check_visited(
        POSITION pos
        )
{
	return (BOOLEAN) functionsMapping->get_slice_slot( (UINT64)pos, BPDB_VISITEDSLOT );
}

void
bpdb_mark_visited(
        POSITION pos
        )
{
	bpdb_set_slice_slot( (UINT64)pos, BPDB_VISITEDSLOT, (UINT64)1 );
}

void
bpdb_unmark_visited(
        POSITION pos
        )
{
	bpdb_set_slice_slot( (UINT64)pos, BPDB_VISITEDSLOT, (UINT64)0 );
}

void
bpdb_set_mex(
        POSITION pos,
        MEX mex
        )
{
	bpdb_set_slice_slot( (UINT64)pos, BPDB_MEXSLOT, (UINT64)mex );
}

MEX
bpdb_get_mex(
        POSITION pos
        )
{
	return (MEX) functionsMapping->get_slice_slot( (UINT64)pos, BPDB_MEXSLOT );
}

void
bpdb_set_winby(
        POSITION pos,
        WINBY winBy
        )
{
	bpdb_set_slice_slot( (UINT64)pos, BPDB_WINBYSLOT, (UINT64)winBy );
}

WINBY
bpdb_get_winby(
        POSITION pos
        )
{
	return (WINBY) functionsMapping->get_slice_slot( (UINT64)pos, BPDB_WINBYSLOT );
}

/*++

   Routine Description:

    bpdb_set_slice_slot sets a specific slot of a given slice to the
    passed in value.

    Flow:
    1. Checks whether the write is being done to the nowrite array
        or the write array
    2. Checks whether this is the max value the slot has seen so far
    3. If this value is larger than the slot capacity then either
        1. The slot will be enlarged
        2. An overflow warning is written
    4. The proper offsets are calculated and the slot is updated

   Arguments:

    position - the slice to be modified
    index - the slot to be modified
    value - the new value of the slot

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/


UINT64
bpdb_set_slice_slot(
        UINT64 position,
        UINT8 index,
        UINT64 value
        )
{
	UINT64 byteOffset = 0;
	UINT8 bitOffset = 0;
	BYTE *bpdb_array = NULL;
	SLICE bpdb_slice = NULL;
	BOOLEAN write = TRUE;

	if(index % 2) {
		bpdb_array = bpdb_nowrite_array;
		bpdb_slice = bpdb_nowrite_slice;
		write = FALSE;
	} else {
		bpdb_array = bpdb_write_array;
		bpdb_slice = bpdb_write_slice;
	}
	index /= 2;

	if(value > bpdb_slice->maxseen[index]) {
		bpdb_slice->maxseen[index] = value;
	}

	if(value > bpdb_slice->maxvalue[index]) {
		if(bpdb_slice->adjust[index]) {
			bpdb_grow_slice(bpdb_array, bpdb_slice, index, value);
			if(write) {
				bpdb_array = bpdb_write_array;
			} else {
				bpdb_array = bpdb_nowrite_array;
			}
		} else {
			if(!bpdb_slice->overflowed[index]) {
				if(!bpdb_have_printed) {
					bpdb_have_printed = TRUE;
					printf("\n");
				}
				printf("Warning: Slot %s with bit size %u had to be rounded from %llu to its maxvalue %llu.\n",
				       bpdb_slice->name[index],
				       bpdb_slice->size[index],
				       value,
				       bpdb_slice->maxvalue[index]
				       );
				bpdb_slice->overflowed[index] = TRUE;
			}
			value = bpdb_slice->maxvalue[index];
		}
	}

	byteOffset = (bpdb_slice->bits * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
	bitOffset += bpdb_slice->offset[index];

	byteOffset += bitOffset / BITSINBYTE;
	bitOffset %= BITSINBYTE;

	//printf("byteoff: %d bitoff: %d value: %llu length: %d\n", byteOffset, bitOffset, value, length);
	//printf("value: %llu\n", value);
	bitlib_insert_bits( bpdb_array + byteOffset, bitOffset, value, bpdb_slice->size[index] );

	return value;
}


/*++

   Routine Description:

    bpdb_set_slice_slot_max sets a specific slot of a given slice to the
    passed in value.

    Flow:
    1. Checks whether the write is being done to the nowrite array
        or the write array
    2. Checks whether this is the max value the slot has seen so far
    3. If this value is larger than the slot capacity then either
        1. The slot will be enlarged
        2. An overflow warning is written
    4. The proper offsets are calculated and the slot is updated

   Arguments:

    position - the slice to be modified
    index - the slot to be modified
    value - the new value of the slot

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/


UINT64
bpdb_set_slice_slot_max(
        UINT64 position,
        UINT8 index
        )
{
	UINT64 byteOffset = 0;
	UINT8 bitOffset = 0;
	BYTE *bpdb_array = NULL;
	SLICE bpdb_slice = NULL;

	if(index % 2) {
		bpdb_array = bpdb_nowrite_array;
		bpdb_slice = bpdb_nowrite_slice;
	} else {
		bpdb_array = bpdb_write_array;
		bpdb_slice = bpdb_write_slice;
	}
	index /= 2;

	if(!bpdb_slice->reservemax[index]) {
		BPDB_TRACE("bpdb_set_slice_slot_max()", "slot without its maxvalue reserved is being set to max", STATUS_INVALID_OPERATION);
	}

	byteOffset = (bpdb_slice->bits * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
	bitOffset += bpdb_slice->offset[index];

	byteOffset += bitOffset / BITSINBYTE;
	bitOffset %= BITSINBYTE;

	bitlib_insert_bits( bpdb_array + byteOffset, bitOffset, bpdb_slice->maxvalue[index]+1, bpdb_slice->size[index] );

	return bpdb_slice->maxvalue[index]+1;
}


GMSTATUS
bpdb_grow_slice(
        BYTE *bpdb_array,
        SLICE bpdb_slice,
        UINT8 index,
        UINT64 value
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	BYTE *bpdb_new_array = NULL;

	// information about conversion
	UINT32 newSlotSize = 0;
	UINT32 oldSlotSize = 0;
	UINT32 bitsToAddToSlot = 0;

	UINT32 oldSliceSize = 0;
	UINT32 newSliceSize = 0;

	// counters
	UINT64 currentSlot = 0;
	UINT64 currentSlice = 0;

	// used to count the number
	// of bits a new value needs
	UINT64 temp = value;

	// offsets of old slices
	UINT32 fbyteOffset = 0;
	UINT32 fbitOffset = 0;

	// offsets of new slices
	UINT32 tbyteOffset = 0;
	UINT32 tbitOffset = 0;

	// offsets of old slices + slot
	UINT32 mfbyteOffset = 0;
	UINT32 mfbitOffset = 0;

	// offsets of new slices + slot
	UINT32 mtbyteOffset = 0;
	UINT32 mtbitOffset = 0;

	// size of right and left portions of the slice
	// **left**|**middle/expanding***|**right**
	UINT32 rightSize = 0;
	UINT32 leftSize = 0;

	// save the data of a slice before copying it
	UINT64 data = 0;

	//
	// main code
	//

	// debug-temp
	//bpdb_dump_database(1);

	if(bpdb_slice->reservemax[index]) {
		temp++;
	}

	// determine size of new largest value
	while(0 != temp) {
		bitsToAddToSlot++;
		temp = temp >> 1;
	}

	//
	// WE WANT TO DO THESE CONVERSIONS FIRST B/C
	// WE DO NOT WANT TO CHANGE ANY OF THE MEMBER VARS
	// IF WE DO, AND THE ALLOC FAILS, THE DB WILL BE DAMAGED
	// THIS MAINTAINS INTEGRITY AFTER AN ALLOC FAIL
	//

	// find difference between old size, and the new
	// size required for the slot
	bitsToAddToSlot -= bpdb_slice->size[index];

	// save old slice size
	oldSlotSize = bpdb_slice->size[index];

	// number of additional bits
	newSlotSize = oldSlotSize + bitsToAddToSlot;

	// bits
	oldSliceSize = bpdb_slice->bits;
	newSliceSize = bpdb_slice->bits + bitsToAddToSlot;
	if(!bpdb_have_printed) {
		bpdb_have_printed = TRUE;
		printf("\n");
	}

	if(gBitPerfectDBVerbose) {
		printf("Expanding database (Slot %s %u bits->%u bits)... ", bpdb_slice->name[index], oldSlotSize, newSlotSize);
	}

	// allocate new space needed for the larger database
	bpdb_new_array = (BYTE *) realloc( bpdb_array, (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) ) * sizeof(BYTE));
	if(NULL == bpdb_new_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_grow_slice()", "Could not allocate new database", status);
		goto _bailout;
	}

	// determine whether new array replaces the write
	// array or the nowrite array
	if(bpdb_array == bpdb_write_array) {
		bpdb_write_array = bpdb_new_array;
		bpdb_write_array_length = (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	} else {
		bpdb_nowrite_array = bpdb_new_array;
		bpdb_nowrite_array_length = (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	}

	// debug-temp
	//bpdb_dump_database(2);

	// set slot to new size
	bpdb_slice->size[index] += bitsToAddToSlot;
	// set slice to new size
	bpdb_slice->bits += bitsToAddToSlot;

	// set offsets to new values
	for(currentSlot = 0; currentSlot < bpdb_slice->slots; currentSlot++) {
		if(currentSlot > index) {
			bpdb_slice->offset[currentSlot] += bitsToAddToSlot;
		}
	}

	// set new max value
	UINT64 oldmaxvalue = bpdb_slice->maxvalue[index];
	bpdb_slice->maxvalue[index] = (UINT64)pow(2, newSlotSize) - 1;
	if(bpdb_slice->reservemax[index]) {
		bpdb_slice->maxvalue[index]--;
	}

	leftSize = bpdb_slice->offset[index];
	rightSize = bpdb_slice->bits - bpdb_slice->size[index] - leftSize;

	//printf( "start while (value %llu, add %d)\n", value, bitsToAddToSlot );

	// for each record starting from the max record
	// copy from the old db format to the new db format
	currentSlice = bpdb_slices - 1;

	// could not use for loop since currentSlice will wrap from 0 to its max value
	while(TRUE) {
		fbyteOffset = (oldSliceSize * currentSlice)/BITSINBYTE;
		fbitOffset = ((UINT8)(oldSliceSize % BITSINBYTE) * (UINT8)(currentSlice % BITSINBYTE)) % BITSINBYTE;
		fbyteOffset += fbitOffset / BITSINBYTE;
		fbitOffset %= BITSINBYTE;

		tbyteOffset = (newSliceSize * currentSlice)/BITSINBYTE;
		tbitOffset = ((UINT8)(newSliceSize % BITSINBYTE) * (UINT8)(currentSlice % BITSINBYTE)) % BITSINBYTE;
		tbyteOffset += tbitOffset / BITSINBYTE;
		tbitOffset %= BITSINBYTE;

		//printf("Slice %llu (%u, %u) (%u, %u) ", currentSlice, fbyteOffset, fbitOffset, tbyteOffset, tbitOffset);

		// right
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize + oldSlotSize;
		mtbitOffset += leftSize + newSlotSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( bpdb_new_array + mfbyteOffset, mfbitOffset, rightSize );
		//printf("%llu(%u) ", data, rightSize);
		bitlib_insert_bits( bpdb_new_array + mtbyteOffset, mtbitOffset, data, rightSize );


		// middle
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize;
		mtbitOffset += leftSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( bpdb_new_array + mfbyteOffset, mfbitOffset, oldSlotSize );
		//printf("%llu(%u %u) ", data, oldSlotSize, newSlotSize);
		if(bpdb_slice->reservemax[index] && data == oldmaxvalue + 1) {
			data = bpdb_slice->maxvalue[index] + 1;
		}
		bitlib_insert_bits( bpdb_new_array + mtbyteOffset, mtbitOffset, data, newSlotSize );

		// left
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;

		data = bitlib_read_bits( bpdb_new_array + mfbyteOffset, mfbitOffset, leftSize );
		//printf("%llu(%u) \n", data, leftSize);
		bitlib_insert_bits( bpdb_new_array + mtbyteOffset, mtbitOffset, data, leftSize );


		if(currentSlice == 0) break;
		currentSlice--;
	}

	printf("done\n");

	/*
	   // debugging information
	   printf("Offsets: ");
	   for(currentSlot = 0; currentSlot < bpdb_slice->slots; currentSlot++) {
	    printf(" %u(%llu)", bpdb_slice->offset[currentSlot], currentSlot);
	   }
	   printf("\n");

	   printf("OldSlice: %u NewSlice: %u OldSlot: %u NewSlot: %u Totalbits %u\n", oldSliceSize, newSliceSize, oldSlotSize, newSlotSize, bpdb_slice->bits);
	   printf("Leftsize: %u Rightsize: %u\n", leftSize, rightSize);
	 */

	// debug-temp
	//bpdb_dump_database(3);

_bailout:
	return status;
}



GMSTATUS
bpdb_shrink_slice(
        BYTE *bpdb_array,
        SLICE bpdb_slice,
        UINT8 index
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	BYTE *bpdb_new_array = NULL;

	// information about conversion
	UINT32 newSlotSize = 0;
	UINT32 oldSlotSize = 0;
	UINT32 bitsToShrink = 0;

	UINT32 oldSliceSize = 0;
	UINT32 newSliceSize = 0;

	// counters
	UINT64 currentSlot = 0;
	UINT64 currentSlice = 0;

	// used to count the number
	// of bits a new value needs
	UINT64 temp = bpdb_slice->maxseen[index];

	// offsets of old slices
	UINT32 fbyteOffset = 0;
	UINT32 fbitOffset = 0;

	// offsets of new slices
	UINT32 tbyteOffset = 0;
	UINT32 tbitOffset = 0;

	// offsets of old slices + slot
	UINT32 mfbyteOffset = 0;
	UINT32 mfbitOffset = 0;

	// offsets of new slices + slot
	UINT32 mtbyteOffset = 0;
	UINT32 mtbitOffset = 0;

	// size of right and left portions of the slice
	// **left**|**middle/expanding***|**right**
	UINT32 rightSize = 0;
	UINT32 leftSize = 0;

	// save the data of a slice before copying it
	UINT64 data = 0;

	//
	// main code
	//

	// debug-temp
	//bpdb_dump_database(1);

	if(bpdb_slice->reservemax[index]) {
		temp++;
	}

	// determine size of new largest value
	while(0 != temp) {
		bitsToShrink++;
		temp = temp >> 1;
	}

	//
	// WE WANT TO DO THESE CONVERSIONS FIRST B/C
	// WE DO NOT WANT TO CHANGE ANY OF THE MEMBER VARS
	// IF WE DO, AND THE ALLOC FAILS, THE DB WILL BE DAMAGED
	// THIS MAINTAINS INTEGRITY AFTER AN ALLOC FAIL
	//

	// find difference between old size, and the new
	// size required for the slot
	bitsToShrink = bpdb_slice->size[index] - bitsToShrink;

	// save old slice size
	oldSlotSize = bpdb_slice->size[index];

	// number of additional bits
	newSlotSize = oldSlotSize - bitsToShrink;

	// bits
	oldSliceSize = bpdb_slice->bits;
	newSliceSize = bpdb_slice->bits - bitsToShrink;
	if(!bpdb_have_printed) {
		bpdb_have_printed = TRUE;
		printf("\n");
	}

	if(gBitPerfectDBVerbose) {
		printf("Shrinking (Slot %s %u bits->%u bits)... ", bpdb_slice->name[index], oldSlotSize, newSlotSize);
	}

	// debug-temp
	//bpdb_dump_database(2);

	// set slot to new size
	bpdb_slice->size[index] -= bitsToShrink;
	// set slice to new size
	bpdb_slice->bits -= bitsToShrink;

	// set offsets to new values
	for(currentSlot = 0; currentSlot < bpdb_slice->slots; currentSlot++) {
		if(currentSlot > index) {
			bpdb_slice->offset[currentSlot] -= bitsToShrink;
		}
	}

	// set new max value
	UINT64 oldmaxvalue = bpdb_slice->maxvalue[index];
	bpdb_slice->maxvalue[index] = (UINT64)pow(2, newSlotSize) - 1;
	if(bpdb_slice->reservemax[index]) {
		bpdb_slice->maxvalue[index]--;
	}

	leftSize = bpdb_slice->offset[index];
	rightSize = bpdb_slice->bits - bpdb_slice->size[index] - leftSize;

	//printf( "start while (value %llu, add %d)\n", value, bitsToAddToSlot );

	// for each record
	// copy from the old db format to the new db format

	for(currentSlice = 0; currentSlice < bpdb_slices; currentSlice++) {
		fbyteOffset = (oldSliceSize * currentSlice)/BITSINBYTE;
		fbitOffset = ((UINT8)(oldSliceSize % BITSINBYTE) * (UINT8)(currentSlice % BITSINBYTE)) % BITSINBYTE;
		fbyteOffset += fbitOffset / BITSINBYTE;
		fbitOffset %= BITSINBYTE;

		tbyteOffset = (newSliceSize * currentSlice)/BITSINBYTE;
		tbitOffset = ((UINT8)(newSliceSize % BITSINBYTE) * (UINT8)(currentSlice % BITSINBYTE)) % BITSINBYTE;
		tbyteOffset += tbitOffset / BITSINBYTE;
		tbitOffset %= BITSINBYTE;

		//printf("Slice %llu (%u, %u) (%u, %u) ", currentSlice, fbyteOffset, fbitOffset, tbyteOffset, tbitOffset);

		// left
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;

		data = bitlib_read_bits( bpdb_array + mfbyteOffset, mfbitOffset, leftSize );
		//printf("%llu(%u) \n", data, leftSize);
		bitlib_insert_bits( bpdb_array + mtbyteOffset, mtbitOffset, data, leftSize );

		// middle
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize;
		mtbitOffset += leftSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( bpdb_array + mfbyteOffset, mfbitOffset, oldSlotSize );
		if(bpdb_slice->reservemax[index] && data == oldmaxvalue + 1) {
			data = bpdb_slice->maxvalue[index] + 1;
		}
		//printf("%llu(%u %u) ", data, oldSlotSize, newSlotSize);
		bitlib_insert_bits( bpdb_array + mtbyteOffset, mtbitOffset, data, newSlotSize );

		// right
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize + oldSlotSize;
		mtbitOffset += leftSize + newSlotSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( bpdb_array + mfbyteOffset, mfbitOffset, rightSize );
		//printf("%llu(%u) ", data, rightSize);
		bitlib_insert_bits( bpdb_array + mtbyteOffset, mtbitOffset, data, rightSize );


	}

	if(gBitPerfectDBVerbose) {
		printf("done\n");
	}


	// allocate new space needed for the larger database
	bpdb_new_array = (BYTE *) realloc( bpdb_array, (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) ) * sizeof(BYTE));
	if(NULL == bpdb_new_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_shrink_slice()", "Could not allocate new database", status);
		goto _bailout;
	}

	// determine whether new array replaces the write
	// array or the nowrite array
	if(bpdb_array == bpdb_write_array) {
		bpdb_write_array = bpdb_new_array;
		bpdb_write_array_length = (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	} else {
		bpdb_nowrite_array = bpdb_new_array;
		bpdb_nowrite_array_length = (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	}

	/*
	   // debugging information
	   printf("Offsets: ");
	   for(currentSlot = 0; currentSlot < bpdb_slice->slots; currentSlot++) {
	    printf(" %u(%llu)", bpdb_slice->offset[currentSlot], currentSlot);
	   }
	   printf("\n");

	   printf("OldSlice: %u NewSlice: %u OldSlot: %u NewSlot: %u Totalbits %u\n", oldSliceSize, newSliceSize, oldSlotSize, newSlotSize, bpdb_slice->bits);
	   printf("Leftsize: %u Rightsize: %u\n", leftSize, rightSize);
	 */

	// debug-temp
	//bpdb_dump_database(3);

_bailout:
	return status;
}


/*++

   Routine Description:

    bpdb_get_slice_slot retrieves the value of a specific slot of a
    given slice.

    Flow:
    1. Check whether reading from the write array or nowrite
        array.
    2. Determine byte and bit offset, and read

   Arguments:

    position - the slice to be read from
    index - the slot to be read

   Return value:

    Value of the requested slot

   --*/


UINT64
bpdb_get_slice_slot_disk(
        UINT64 position,
        UINT8 index
        );


UINT64
bpdb_get_slice_slot(
        UINT64 position,
        UINT8 index
        )
{
	UINT64 byteOffset = 0;
	UINT8 bitOffset = 0;
	BYTE *bpdb_array = NULL;
	SLICE bpdb_slice = NULL;

	if(index % 2) {
		bpdb_array = bpdb_nowrite_array;
		bpdb_slice = bpdb_nowrite_slice;
	} else {
		bpdb_array = bpdb_write_array;
		bpdb_slice = bpdb_write_slice;
	}
	index /= 2;

	byteOffset = (bpdb_slice->bits * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
	bitOffset += bpdb_slice->offset[index];

	byteOffset += bitOffset / BITSINBYTE;
	bitOffset %= BITSINBYTE;

	return bitlib_read_bits( bpdb_array + byteOffset, bitOffset, bpdb_slice->size[index] );
}


UINT64
bpdb_get_slice_slot_disk(
        UINT64 position,
        UINT8 index
        )
{
	BYTE *inputBuffer = NULL;
	BYTE *curBuffer = NULL;

	index /= 2;

	// assign offset of first bit
	UINT8 offset = bpdb_readOffset;

	// counters
	UINT64 currentSlice = 0;
	UINT8 currentSlot = 0;

	// Eventually REMOVE these NULL checks, once the
	// code is mature and these NULL errors do not occur.
	if(NULL == bpdb_readFile) {
		BPDB_TRACE("bpdb_get_slice_slot_disk()", "bpdb_readFile is NULL.", STATUS_INVALID_INPUT_PARAMETER);
		goto _bailout;
	}

	if(NULL == bpdb_readScheme) {
		BPDB_TRACE("bpdb_get_slice_slot_disk()", "bpdb_readScheme is NULL.", STATUS_INVALID_INPUT_PARAMETER);
		goto _bailout;
	}

	// seek to desired location
	bitlib_file_seek(bpdb_readFile, bpdb_readStart, SEEK_SET);

	// initialize buffer
	inputBuffer = alloca( bpdb_buffer_length * sizeof(BYTE) );
	memset( inputBuffer, 0, bpdb_buffer_length );
	curBuffer = inputBuffer;

	// read in data to buffer
	bitlib_file_read_bytes( bpdb_readFile, inputBuffer, bpdb_buffer_length );

	if(bpdb_readScheme->indicator) {

		while(currentSlice < position) {
			if(bitlib_read_from_buffer( bpdb_readFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, 1 ) == 0) {

				for(currentSlot = 0; currentSlot < (bpdb_write_slice->slots); currentSlot++) {
					bitlib_read_from_buffer( bpdb_readFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->size[currentSlot]);
				}
				currentSlice++;
			} else {
				UINT64 skips = bpdb_generic_read_varnum( bpdb_readFile, bpdb_readScheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, TRUE );
				currentSlice += skips;
			}
		}

		if(currentSlice == position)
		{
			// if slice is part of a range of skips, return 0
			if(bitlib_read_from_buffer( bpdb_readFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, 1 ) != 0) {
				return 0;
			}

			for(currentSlot = 0; currentSlot < (bpdb_write_slice->slots); currentSlot++) {
				UINT64 value = bitlib_read_from_buffer( bpdb_readFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->size[currentSlot]);

				if(currentSlot == index)
				{
					return value;
				}
			}
		}
		else
		{
			// if skips passed the sought position,
			// return 0
			return 0;
		}
	} else {
		// computation for a scheme 0 encoded db
		UINT64 byteOffset = 0;
		UINT8 bitOffset = 0;

		byteOffset = (bpdb_write_slice->bits * position)/BITSINBYTE;
		bitOffset = ((UINT8)(bpdb_write_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
		bitOffset += bpdb_write_slice->offset[index];

		byteOffset += bitOffset / BITSINBYTE;
		bitOffset %= BITSINBYTE;

		bitlib_file_seek(bpdb_readFile, bpdb_readStart+byteOffset, SEEK_SET);

		bitlib_file_read_bytes( bpdb_readFile, inputBuffer, bpdb_buffer_length );

		return bitlib_read_from_buffer( bpdb_readFile, &curBuffer, inputBuffer, bpdb_buffer_length, &bitOffset, bpdb_write_slice->size[index]);
	}

_bailout:
	return 0;
}

/*++

   Routine Description:

    bpdb_add_slot retrieves the value of a specific slot of a
    given slice.

    Flow:
    1. Check whether reading from the write array or nowrite
        array.
    2. Determine byte and bit offset, and read

   Arguments:

    size - size of new slot in bits
    name - name of the new slot
    write - whether the new slot will be written to file
    adjust - whether the new slot should be adjustable
    reservemax - whether the largest encodable number
            should be reserved
    slotindex - ptr will be dereferenced and set to the index
            of the new slot for use in later accesses

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bpdb_add_slot(
        UINT8 size,
        char *name,
        BOOLEAN write,
        BOOLEAN adjust,
        BOOLEAN reservemax,
        UINT32 *slotindex
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	SLICE bpdb_slice = NULL;

	//
	// check input parameters
	//

	if(NULL == name) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("bpdb_add_slot()", "name is passed in as null", status);
		goto _bailout;
	}

	if(NULL == bpdb_write_slice) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("bpdb_add_slot()", "bpdb_write_slice must be initialized first. call bpdb_init()", status);
		goto _bailout;
	}

	if(NULL == bpdb_nowrite_slice) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("bpdb_add_slot()", "bpdb_nowrite_slice must be initialized first. call bpdb_init()", status);
		goto _bailout;
	}

	// even indexes are used for slots to be written
	// odd indexes are used for slots that are in-memory
	if(write) {
		bpdb_slice = bpdb_write_slice;
		*slotindex = 0;
	} else {
		bpdb_slice = bpdb_nowrite_slice;
		*slotindex = 1;
	}

	*slotindex += bpdb_slice->slots*2;

	// for backwards compatibility
	if(strcmp(name, "VALUE") == 0) BPDB_VALUESLOT = *slotindex;
	else if(strcmp(name, "MEX") == 0) BPDB_MEXSLOT = *slotindex;
	else if(strcmp(name, "WINBY") == 0) BPDB_WINBYSLOT = *slotindex;
	else if(strcmp(name, "REMOTENESS") == 0) BPDB_REMSLOT = *slotindex;
	else if(strcmp(name, "VISITED") == 0) BPDB_VISITEDSLOT = *slotindex;

	bpdb_slice->slots++;

	//
	// allocate new slot structure
	//

	if(bpdb_slice->slots == 1) {
		bpdb_slice->size = (UINT8 *) calloc( 1, sizeof(UINT8) );
		bpdb_slice->offset = (UINT32 *) calloc( 1, sizeof(UINT32) );
		bpdb_slice->maxvalue = (UINT64 *) calloc( 1, sizeof(UINT64) );
		bpdb_slice->maxseen = (UINT64 *) calloc( 1, sizeof(UINT64) );
		bpdb_slice->name = (char **) calloc( 1, sizeof(char **) );
		bpdb_slice->overflowed = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
		bpdb_slice->adjust = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
		bpdb_slice->reservemax = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
	} else {
		bpdb_slice->size = (UINT8 *) realloc( bpdb_slice->size, bpdb_slice->slots*sizeof(UINT8) );
		bpdb_slice->offset = (UINT32 *) realloc( bpdb_slice->offset, bpdb_slice->slots*sizeof(UINT32) );
		bpdb_slice->maxvalue = (UINT64 *) realloc( bpdb_slice->maxvalue, bpdb_slice->slots*sizeof(UINT64) );
		bpdb_slice->maxseen = (UINT64 *) realloc( bpdb_slice->maxseen, bpdb_slice->slots*sizeof(UINT64) );
		bpdb_slice->name = (char **) realloc( bpdb_slice->name, bpdb_slice->slots*sizeof(char **) );
		bpdb_slice->overflowed = (BOOLEAN *) realloc( bpdb_slice->overflowed, bpdb_slice->slots*sizeof(BOOLEAN) );
		bpdb_slice->adjust = (BOOLEAN *) realloc( bpdb_slice->adjust, bpdb_slice->slots*sizeof(BOOLEAN) );
		bpdb_slice->reservemax = (BOOLEAN *) realloc( bpdb_slice->reservemax, bpdb_slice->slots*sizeof(BOOLEAN) );
	}

	//
	// set new slot attributes
	//

	bpdb_slice->bits += size;
	bpdb_slice->size[bpdb_slice->slots-1] = size;
	if(bpdb_slice->slots == 1) {
		bpdb_slice->offset[bpdb_slice->slots-1] = 0;
	} else {
		bpdb_slice->offset[bpdb_slice->slots-1] = bpdb_slice->offset[bpdb_slice->slots - 2] + bpdb_slice->size[bpdb_slice->slots - 2];
	}
	bpdb_slice->maxvalue[bpdb_slice->slots-1] = (UINT64)pow(2, size) - 1;
	bpdb_slice->maxseen[bpdb_slice->slots-1] = 0;
	bpdb_slice->name[bpdb_slice->slots-1] = (char *) malloc( strlen(name) + 1 );
	strcpy( bpdb_slice->name[bpdb_slice->slots-1], name );
	bpdb_slice->overflowed[bpdb_slice->slots-1] = FALSE;
	bpdb_slice->adjust[bpdb_slice->slots-1] = adjust;

	// if max is reserved, reduce the maxvalue by 1 since the max should
	// only be set by a special function call
	bpdb_slice->reservemax[bpdb_slice->slots-1] = reservemax;
	if(reservemax) {
		bpdb_slice->maxvalue[bpdb_slice->slots-1]--;
	}

_bailout:
	return status;
}


/*++

   Routine Description:

    bpdb_print_database prints each slice of the database with
    each slot indicated to stdout.

   Arguments:

    None

   Return value:

    None

   --*/

void
bpdb_print_database()
{
	UINT64 i = 0;
	UINT8 j = 0;

	printf("\n\nDatabase Printout\n");

	for(i = 0; i < bpdb_slices; i++) {
		printf("Slice %llu  (write) ", i);
		for(j=0; j < (bpdb_write_slice->slots); j++) {
			printf("%s: %llu  ", bpdb_write_slice->name[j], bpdb_get_slice_slot(i, j*2));
		}
		printf("(no write) ");
		for(j=0; j < (bpdb_nowrite_slice->slots); j++) {
			printf("%s: %llu  ", bpdb_nowrite_slice->name[j], bpdb_get_slice_slot(i, j*2+1));
		}
		printf("\n");
	}
}


/*++

   Routine Description:

    bpdb_dump_database dumps the contents of the database to
    a file.

   Arguments:

    num - identifier for file name

   Return value:

    None

   --*/

void
bpdb_dump_database( int num )
{
	char filename[256];
	UINT64 i = 0;
	UINT8 j = 0;
	UINT32 currentSlot = 0;

	mkdir("dumps", 0755);

	sprintf(filename, "./dumps/m%s_%d_bpdb_%d.dump", kDBName, getOption(), num);

	FILE *f = fopen( filename, "wb");

	fprintf(f, "\nDatabase Printout\n");

	fprintf(f, "Offsets: ");
	for(currentSlot = 0; currentSlot < bpdb_write_slice->slots; currentSlot++) {
		fprintf(f, " %u(%u)", bpdb_write_slice->offset[currentSlot], currentSlot);
	}
	fprintf(f, "\n");

	for(i = 0; i < bpdb_slices; i++) {
		fprintf(f, "Slice %llu  (write) ", i);
		for(j=0; j < (bpdb_write_slice->slots); j++) {
			fprintf(f, "%s: %llu  ", bpdb_write_slice->name[j], bpdb_get_slice_slot(i, j*2));
		}
		fprintf(f, "(no write) ");
		for(j=0; j < (bpdb_nowrite_slice->slots); j++) {
			fprintf(f, "%s: %llu  ", bpdb_nowrite_slice->name[j], bpdb_get_slice_slot(i, j*2+1));
		}
		fprintf(f, "\n");
	}

	fclose(f);
}


/*++

   Routine Description:

    bpdb_analyze_database is used for experimentation and
    analysis

   Arguments:

    num - identifier for file to be dumped to

   Return value:

    None

   --*/

void
bpdb_analyze_database( int num )
{
	char filename[256];
	UINT64 i = 0;
//    UINT8 j = 0;
//    UINT32 currentSlot = 0;

	mkdir("dumps", 0755);

	sprintf(filename, "./dumps/m%s_%d_bpdb_analyze_%d.dump", kDBName, getOption(), num);

	FILE *f = fopen( filename, "wb");

	HTABLE ht = htable_new( 1000 );

	//printf("Num of write array length: %u\n", bpdb_write_array_length);

	for(i=0; i < bpdb_write_array_length; i++) {
		UINT32 val = htable_get( ht, bpdb_write_array[i] );
		htable_set( ht, bpdb_write_array[i], val + 1 );
	}

	HTABLE_SLIST hs = NULL;
	for(i = 0; i< ht->size; i++) {
		hs = ht->buckets[i];
		while( NULL != hs ) {
			fprintf(f, "%x: %u\n", hs->key, hs->value);
			hs = hs->next;
		}
	}

/*
    fprintf(f, "\nDatabase Printout\n");

    fprintf(f, "Offsets: ");
    for(currentSlot = 0; currentSlot < bpdb_write_slice->slots; currentSlot++) {
        fprintf(f, " %u(%u)", bpdb_write_slice->offset[currentSlot], currentSlot);
    }
    fprintf(f, "\n");

    for(i = 0; i < bpdb_slices; i++) {
        fprintf(f, "Slice %llu  (write) ", i);
        for(j=0; j < (bpdb_write_slice->slots); j++) {
            fprintf(f, "%s: %llu  ", bpdb_write_slice->name[j], bpdb_get_slice_slot(i, j*2));
        }
        fprintf(f, "(no write) ");
        for(j=0; j < (bpdb_nowrite_slice->slots); j++) {
            fprintf(f, "%s: %llu  ", bpdb_nowrite_slice->name[j], bpdb_get_slice_slot(i, j*2+1));
        }
        fprintf(f, "\n");
    }
 */
	fclose(f);
}


/*++

   Routine Description:

    bpdb_save_database writes the (write) database to
    file.

    The database is encoded with each activated scheme
    and picks the smallest. At any point in time, only
    two schemes are saved to file, then the smaller of
    the two is deleted and another scheme is encoded
    and compared.

   Arguments:

    None

   Return value:

    TRUE on successful execution, or FALSE otherwise


   --*/

BOOLEAN
bpdb_save_database()
{
	// debug-temp
	//bpdb_print_database();
	//bpdb_analyze_database(1);

	GMSTATUS status = STATUS_SUCCESS;

	// counter
	int i = 0;
	SLIST cur = NULL;

	// file names of saved files
	char **outfilenames;

	// final file name
	char outfilename[256];

	// track smallest file
	int smallestscheme = 0;
	int smallestsize = -1;

	// struct for fileinfo
	struct stat fileinfo;

	if(0 == slist_size(bpdb_schemes)) {
		status = STATUS_NO_SCHEMES_INSTALLED;
		BPDB_TRACE("bpdb_save_database()", "no encoding schemes installed to save db file", status);
		goto _bailout;
	}

	// set counters
	i = 0;
	cur = bpdb_schemes;

	// allocate memory for all db file names
	outfilenames = (char **) malloc( slist_size(bpdb_schemes)*sizeof(char*) );
	if(NULL == outfilenames) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_save_database()", "Could not allocate outfilenames in memory", status);
		goto _bailout;
	}

	// allocate memory for each filename
	for(i = 0; i<slist_size(bpdb_schemes); i++) {
		outfilenames[i] = (char *) malloc( 256*sizeof(char) );
		if(NULL == outfilenames[i]) {
			status = STATUS_NOT_ENOUGH_MEMORY;
			BPDB_TRACE("bpdb_save_database()", "Could not allocate outfilename in memory", status);
			goto _bailout;
		}
	}

/*
    // debug-temp
    // print out the status of the slots after solving
    printf("\nNum slots: %u\n", bpdb_write_slice->slots);

    for(i = 0; i<bpdb_write_slice->slots; i++) {
        printf("%s: %llu\n", bpdb_write_slice->name[i], bpdb_write_slice->maxseen[i]);
    }
 */

	bpdb_have_printed = FALSE;

	UINT64 temp = 0;
	UINT8 bitsNeeded = 0;
	for(i = 0; i < (bpdb_write_slice->slots); i++) {
		temp = bpdb_write_slice->maxseen[i];
		bitsNeeded = 0;

		if(bpdb_write_slice->reservemax[i]) {
			temp++;
		}

		while(0 != temp) {
			bitsNeeded++;
			temp = temp >> 1;
		}

		if(bitsNeeded < bpdb_write_slice->size[i] && bpdb_write_slice->adjust[i]) {
			bpdb_shrink_slice( bpdb_write_array, bpdb_write_slice, i );
		}
	}

	printf("\n");

	i = 0;

	// save file under each encoding scheme
	while(NULL != cur) {
		// if we are not intending to use this scheme for saving, then
		// skip it
		if(!(((SCHEME)cur->obj)->save)) {
			cur = cur->next;
			i++;
			continue;
		}

		// saves with encoding scheme and returns filename
		sprintf(outfilenames[i], "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), ((SCHEME)cur->obj)->id);

		status = bpdb_generic_save_database( (SCHEME) cur->obj, outfilenames[i] );
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_save_database()", "call to bpdb_generic_save_database with scheme failed", status);
		} else {
			// get size of file
			stat(outfilenames[i], &fileinfo);

			if(gBitPerfectDBVerbose) {
				printf("Scheme: %d. Wrote %s with size of %d\n", ((SCHEME)cur->obj)->id, outfilenames[i], (int)fileinfo.st_size);
			}

			// if file is a smaller size, set min
			if(smallestsize == -1 || fileinfo.st_size < smallestsize) {
				if(smallestsize != -1) {
					if(gBitPerfectDBVerbose) {
						printf("Removing %s\n", outfilenames[smallestscheme]);
					}
					remove(outfilenames[smallestscheme]);
				}
				smallestscheme = i;
				smallestsize = fileinfo.st_size;
			} else {
				if(gBitPerfectDBVerbose) {
					printf("Removing %s\n", outfilenames[i]);
				}
				remove(outfilenames[i]);
			}
		}
		cur = cur->next;
		i++;
	}

	if(gBitPerfectDBVerbose) {
		printf("Choosing scheme: %d\n", smallestscheme);
	}

	// rename smallest file to final file name
	sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());

	if(gBitPerfectDBVerbose) {
		printf("Renaming %s to %s\n", outfilenames[smallestscheme], outfilename);
	}
	rename(outfilenames[smallestscheme], outfilename);

_bailout:

	for(i = 0; i<slist_size(bpdb_schemes); i++) {
		SAFE_FREE(outfilenames[i]);
	}

	SAFE_FREE(outfilenames);


	if(!GMSUCCESS(status)) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*++

   Routine Description:

    bpdb_generic_write_varnum writes an input specified
    number to a given buffer using a custom encoding
    scheme.

   Arguments:

    outFile - pointer to file struct for output
    scheme - scheme to use to write variable number
    curBuffer - pointer to buffer dictates where to start
                writing
    outputBuffer - start of buffer
    bufferLength - length of buffer
    offset - bit offset within a byte of the buffer
    consecutiveSkips - variable number to write.

   Return value:

    TRUE on successful execution, or FALSE otherwise


   --*/

BOOLEAN
bpdb_generic_write_varnum(
        dbFILE outFile,
        SCHEME scheme,
        BYTE **curBuffer,
        BYTE *outputBuffer,
        UINT32 bufferLength,
        UINT8 *offset,
        UINT64 consecutiveSkips
        )
{
	UINT8 leftBits, rightBits;

	leftBits = scheme->varnum_gap_bits( consecutiveSkips );
	rightBits = scheme->varnum_size_bits(leftBits);

	bitlib_value_to_buffer( outFile, curBuffer, outputBuffer, bufferLength, offset, bitlib_right_mask64( leftBits), leftBits );
	bitlib_value_to_buffer( outFile, curBuffer, outputBuffer, bufferLength, offset, 0, 1 );

	consecutiveSkips -= scheme->varnum_implicit_amt( leftBits );

	bitlib_value_to_buffer( outFile, curBuffer, outputBuffer, bufferLength, offset, consecutiveSkips, rightBits );

	return TRUE;
}

/*++

   Routine Description:

    bpdb_save_database saves the contents of the bpdb_write_array
    to file using the scheme and filename specified as inputs.

    There are two ways that it may save the database. If scheme 0
    is used, which means that no variable skips encoding is enabled,
    then the save is done in one large chunk by one command.
    Otherwise, the function iterates through each slice and slot,
    outputting the value to a buffer which spills into the output
    file.

   Arguments:

    scheme - variable skips encoding scheme to use
    outfilename - filename to use for output

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bpdb_generic_save_database(
        SCHEME scheme,
        char *outfilename
        )
{
	GMSTATUS status = STATUS_SUCCESS;

	UINT64 consecutiveSkips = 0;
	UINT8 offset = 0;
	UINT8 i, j;

	// gzip file ptr
	dbFILE outFile = NULL;

	UINT64 slice;
	UINT8 slot;

	BYTE *outputBuffer = NULL;
	BYTE *curBuffer = NULL;

	outputBuffer = alloca( bpdb_buffer_length * sizeof(BYTE));
	memset(outputBuffer, 0, bpdb_buffer_length);
	curBuffer = outputBuffer;

	mkdir("data", 0755);

	status = bitlib_file_open(outfilename, "wb", &outFile);
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("bpdb_generic_save_database()", "call to bitlib to open file failed", status);
		goto _bailout;
	}

	bitlib_value_to_buffer ( outFile, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, scheme->id, 8 );

	bpdb_generic_write_varnum( outFile, bpdb_headerScheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_slices );
	bpdb_generic_write_varnum( outFile, bpdb_headerScheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->bits );

	bpdb_generic_write_varnum( outFile, bpdb_headerScheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->slots );

	for(i = 0; i < (bpdb_write_slice->slots); i++) {
		bpdb_generic_write_varnum( outFile, bpdb_headerScheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->size[i]+1 );
		bpdb_generic_write_varnum( outFile, bpdb_headerScheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, strlen(bpdb_write_slice->name[i]) );

		for(j = 0; j<strlen(bpdb_write_slice->name[i]); j++) {
			bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->name[i][j], 8 );
		}

		bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->overflowed[i], 1 );
	}

	if(scheme->indicator) {
		for(slice = 0; slice<bpdb_slices; slice++) {

			// Check if the slice has a mapping
			if(bpdb_get_slice_slot( slice, 0 ) != undecided) {

				// If so, then check to see if skips must be outputted
				if(consecutiveSkips != 0) {
					// Put skips into output buffer
					bpdb_generic_write_varnum( outFile, scheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, consecutiveSkips);
					// Reset skip counter
					consecutiveSkips = 0;
				}
				bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, 0, 1 );
				for(slot=0; slot < (bpdb_write_slice->slots); slot++) {
					bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, bpdb_get_slice_slot(slice, 2*slot), bpdb_write_slice->size[slot] );
				}

			} else {
				consecutiveSkips++;
			}
		}
	} else {

		if(curBuffer != outputBuffer || offset != 0) {
			if(offset == 0) {
				bitlib_file_write_bytes(outFile, outputBuffer, curBuffer-outputBuffer);
			} else {
				bitlib_file_write_bytes(outFile, outputBuffer, curBuffer-outputBuffer+1);
			}
		}

		bitlib_file_write_bytes(outFile, bpdb_write_array, bpdb_write_array_length);
	}

	if(consecutiveSkips != 0) {
		bpdb_generic_write_varnum( outFile, scheme, &curBuffer, outputBuffer, bpdb_buffer_length, &offset, consecutiveSkips);
		consecutiveSkips = 0;
	}

	if(curBuffer != outputBuffer || offset != 0) {
		bitlib_file_write_bytes(outFile, outputBuffer, curBuffer-outputBuffer+1);
	}

	status = bitlib_file_close(outFile);
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("bpdb_generic_save_database()", "call to bitlib to close file failed", status);
		goto _bailout;
	}

_bailout:
	return status;
}


/*++

   Routine Description:

    bpdb_load_database is an exposed function that can be
    called to load a database for the current game.

    Flow:
    1. Checks whether the desired file exists to load from
    2. Finds the matching scheme to decode the file with
    3. Calls bpdb_generic_load_database() to read file

   Arguments:

    None

   Return value:

    TRUE on successful execution, or FALSE otherwise

   --*/


BOOLEAN
bpdb_load_database( )
{
	GMSTATUS status = STATUS_SUCCESS;
	SLIST cur = NULL;

	// filename
	char outfilename[256];

	dbFILE inFile = NULL;
	FILE *testOpen = NULL;

	// file information
	UINT8 fileFormat;

	// open file
	sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());

	if(NULL != (testOpen = fopen(outfilename, "r"))) {
		fclose(testOpen);

		status = bitlib_file_open(outfilename, "rb", &inFile);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_load_database()", "call to bitlib to open file failed", status);
			goto _bailout;
		}
	} else {
		// return FALSE to indicate that the
		// db does not exist
		return FALSE;
	}

	// read file header

	// TO DO: TEST IF BOOLEAN IS TRUE
	bitlib_file_read_bytes( inFile, &fileFormat, 1 );

	if(gBitPerfectDBVerbose) {
		printf("\n\nDatabase Header Information\n");

		// print fileinfo
		printf("Encoding Scheme: %d\n", fileFormat);
	}

	cur = bpdb_schemes;

	while(((SCHEME)cur->obj)->id != fileFormat) {
		if( NULL == cur || NULL == cur->next) {
			status = STATUS_SCHEME_NOT_FOUND;
			BPDB_TRACE("bpdb_load_database()", "cannot find scheme to decode and decompress database", status);
			goto _bailout;
		}

		cur = cur->next;
	}

	bpdb_readScheme = (SCHEME)cur->obj;

	status = bpdb_generic_load_database( inFile, (SCHEME)cur->obj );
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("bpdb_load_database()", "call to bpdb_generic_load_database to load db with recognized scheme failed", status);
		goto _bailout;
	}

	// if the db will be read from file, assign the
	// file pointer to the zero-memory player file pointer
	// otherwise, close the file
	if(bpdb_readFromDisk) {
		bpdb_readFile = inFile;
	} else {
		status = bitlib_file_close(inFile);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_load_database()", "call to bitlib to close file failed", status);
			goto _bailout;
		}
	}

	// debug-temp
	//bpdb_print_database();

_bailout:
	if(!GMSUCCESS(status)) {
		return FALSE;
	} else {
		return TRUE;
	}
}


/*++

   Routine Description:

    bpdb_generic_load_database opens an existing file,
    reads/interprets its contents, and stores the data in
    an in-memory database.

    Flow:
    1. Reads number of slices
    2. Reads bits per slice
    3. Reads number of slots per header
    4. Reads in stored header and adds slots to its own
    5. Allocates database
    6. Reads in database contents

   Arguments:

    inFile - pointer to db file to read in
    scheme - scheme necessary to decode variable skips in
             the db file

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bpdb_generic_load_database(
        dbFILE inFile,
        SCHEME scheme
        )
{
	GMSTATUS status = STATUS_SUCCESS;

	UINT64 currentSlice = 0;
	UINT8 currentSlot = 0;

	UINT64 numOfSlicesHeader = 0;
	UINT8 bitsPerSliceHeader = 0;
	UINT8 numOfSlotsHeader = 0;
	UINT8 offset = 0;
	UINT64 skips = 0;
	UINT64 i, j;
	char * tempname;
	UINT8 tempnamesize = 0;
	UINT8 tempsize = 0;
	UINT32 tempslotindex = 0;
	BOOLEAN tempoverflowed;

	BYTE *inputBuffer = NULL;
	BYTE *curBuffer = NULL;

	inputBuffer = alloca( bpdb_buffer_length * sizeof(BYTE) );
	memset( inputBuffer, 0, bpdb_buffer_length );
	curBuffer = inputBuffer;

	// TO DO - check return value
	bitlib_file_read_bytes( inFile, inputBuffer, bpdb_buffer_length );

	// Read Header
	numOfSlicesHeader = bpdb_generic_read_varnum( inFile, bpdb_headerScheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, FALSE );
	bitsPerSliceHeader = bpdb_generic_read_varnum( inFile, bpdb_headerScheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, FALSE );
	numOfSlotsHeader = bpdb_generic_read_varnum( inFile, bpdb_headerScheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, FALSE );

	if(gBitPerfectDBVerbose) {
		printf("Slices: %llu\nBits per slice: %d\nSlots per slice: %d\n\n", numOfSlicesHeader, bitsPerSliceHeader, numOfSlotsHeader);
	}

	bpdb_free_slice( bpdb_write_slice );

	bpdb_write_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
	if(NULL == bpdb_write_slice) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("bpdb_init()", "Could not allocate bpdb_write_slice in memory", status);
		goto _bailout;
	}

	//
	// outputs slots in db
	// perhaps think about what happens if a slot is missing
	//

	for(i = 0; i<numOfSlotsHeader; i++) {
		// read size slice in bits
		tempsize = bpdb_generic_read_varnum( inFile, bpdb_headerScheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, FALSE );
		tempsize--;

		// read size of slot name in bytes
		tempnamesize = bpdb_generic_read_varnum( inFile, bpdb_headerScheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, FALSE );

		// allocate room for slot name
		tempname = (char *) malloc((tempnamesize + 1) * sizeof(char));

		// read name
		for(j = 0; j<tempnamesize; j++) {
			*(tempname + j) = (char)bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, 8 );
		}
		*(tempname + j) = '\0';

		// read overflowed bit
		tempoverflowed = bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, 1 );

		// output information on each slot to the user
		if(gBitPerfectDBVerbose) {
			printf("Slot: %s (%u bytes)\n", tempname, tempnamesize);
			printf("\tBit Width: %u\n", tempsize);
		}

		if(tempoverflowed) {
			printf("\tWarning: Some values have been capped at %llu due to overflow.\n", (UINT64)pow(2, tempsize) - 1);
		}

		status = bpdb_add_slot(tempsize, tempname, TRUE, FALSE, FALSE, &tempslotindex);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("bpdb_generic_load_database()", "call to bpdb_add_slot to add slot from database file failed", status);
			goto _bailout;
		}

		SAFE_FREE( tempname );
	}

	bpdb_readStart = curBuffer - inputBuffer + 1;
	bpdb_readOffset = offset;

	if(!scheme->indicator) {
		bpdb_readStart++;
	}

	if(gBitPerfectDBZeroMemoryPlayer) {
		if(gBitPerfectDBVerbose) {
			printf("\nUsing Zero-Memory Player...\n");
		}
		bpdb_readFromDisk = TRUE;
		functionsMapping->get_slice_slot = bpdb_get_slice_slot_disk;
		return STATUS_SUCCESS;
	}

	status = bpdb_allocate();
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("bpdb_generic_load_database()", "call to bpdb_allocate failed", status);
		goto _bailout;
	}

	if( scheme->indicator ) {
		showDBLoadingStatus (Clean);

		while(currentSlice < numOfSlicesHeader) {

			if(bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, 1 ) == 0) {

				for(currentSlot = 0; currentSlot < (bpdb_write_slice->slots); currentSlot++) {
					bpdb_set_slice_slot( currentSlice, 2*currentSlot,
					                     bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, bpdb_write_slice->size[currentSlot]) );
				}
				showDBLoadingStatus (Update);

				currentSlice++;
			} else {
				skips = bpdb_generic_read_varnum( inFile, scheme, &curBuffer, inputBuffer, bpdb_buffer_length, &offset, TRUE );

				for(i = 0; i < skips; i++, currentSlice++) {
					bpdb_set_slice_slot( currentSlice, 0, undecided );
					showDBLoadingStatus (Update);
				}
			}
		}
	} else {

		gzrewind(inFile);
		bitlib_file_seek( inFile, bpdb_readStart, SEEK_SET);
		bitlib_file_read_bytes( inFile,
		                        bpdb_write_array,
		                        bpdb_write_array_length
		                        );

	}

_bailout:
	return status;
}

/*++

   Routine Description:

    bpdb_generic_read_varnum opens an existing file,
    reads/interprets its contents, and stores the data in
    an in-memory database.

    Flow:
    1. Reads number of slices
    2. Reads bits per slice
    3. Reads number of slots per header
    4. Reads in stored header and adds slots to its own
    5. Allocates database
    6. Reads in database contents

   Arguments:

    outFile - pointer to file struct to read from
    scheme - scheme to use to read variable number
    curBuffer - pointer to buffer dictates where to start
                reading
    outputBuffer - start of buffer
    bufferLength - length of buffer
    offset - bit offset within a byte of the buffer
    alreadyReadFirstBit - If the first bit has been read,
            then it must be taken into consideration since
            the number of 1 bits dictates the size of the
            encoded variable number

   Return value:

    UINT64 representing the size of the variable number

   --*/


UINT64
bpdb_generic_read_varnum(
        dbFILE inFile,
        SCHEME scheme,
        BYTE **curBuffer,
        BYTE *inputBuffer,
        UINT32 length,
        UINT8 *offset,
        BOOLEAN alreadyReadFirstBit
        )
{
	UINT8 i;
	UINT64 variableNumber = 0;
	UINT8 leftBits = 0;
	UINT8 rightBits = 0;

	if(alreadyReadFirstBit) {
		leftBits = 1;
	}

	while(bitlib_read_from_buffer( inFile, curBuffer, inputBuffer, length, offset, 1 )) {
		leftBits++;
	}

	rightBits = scheme->varnum_size_bits(leftBits);

	for(i = 0; i < rightBits; i++) {
		variableNumber = variableNumber << 1;
		variableNumber = variableNumber | bitlib_read_from_buffer( inFile, curBuffer, inputBuffer, length, offset, 1 );
	}

	variableNumber += scheme->varnum_implicit_amt( leftBits );

	return variableNumber;
}
