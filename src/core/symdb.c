/************************************************************************
**
** NAME:    symdb.c
**
** DESCRIPTION:    Implementation of the Symmetry Database.
**
** AUTHOR:Albert Shau
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
/* DISCLAIMER:  This is really just a copy and paste of Ken Elkabany's Bit
                Perfect Database.  Just want a separate db for symmetries
 */
#include "symdb.h"
#include "gamesman.h"
#include "bpdb_bitlib.h"
#include "bpdb_schemes.h"
#include "bpdb_misc.h"

//typedef enum

//
// Global Variables
//

extern DB_Table    *functionsMapping;

dbFILE          symdb_readFile = NULL;
BOOLEAN symdb_readFromDisk = FALSE;
SCHEME symdb_readScheme = NULL;
UINT32 symdb_readStart = 0;
UINT8 symdb_readOffset = 0;

//
// stores the format of a slice; in particular the
// names, sizes, and maxvalues of its slots
//

SLICE symdb_write_slice = NULL;
SLICE symdb_nowrite_slice = NULL;

//
// in-memory database; write is for data that will
// be written to file while nowrite is only temporary
//

BYTE        *symdb_write_array = NULL;
BYTE        *symdb_nowrite_array = NULL;

size_t symdb_write_array_length = 0;
size_t symdb_nowrite_array_length = 0;

//
// numbers of slices
//

UINT64 symdb_slices = 0;

//
// pointer to list of schemes
//

SLIST symdb_schemes = NULL;

//
// pointer to the encoding scheme that will be used
// for encoding variable numbers in the db file header
//

SCHEME symdb_headerScheme = NULL;

//
// these slots provide legacy support for non-slots
// aware solvers
//

UINT32 SYMDB_VALUESLOT = 0;
UINT32 SYMDB_WINBYSLOT = 0;
UINT32 SYMDB_MEXSLOT = 0;
UINT32 SYMDB_REMSLOT = 0;
UINT32 SYMDB_VISITEDSLOT = 0;

//
// graphical purposes - used to test whether a new
// line has been printed or not
//

BOOLEAN symdb_have_printed = FALSE;

//
// the buffer size in bytes that the bpdb is allowed
// to use when reading and writing data from a file
//

UINT32 symdb_buffer_length = 10000;



/*++

   Routine Description:

    symdb_init initializes the Bit-Perfect Database
    pseudo-C object. This function must be called before
    any other bpdb function. It allocates the bpdb write
    and nowrite slices, which are the slice formats for data
    that is written to file and not written, respectively.
    Furthermore, if a variable slice aware solver is not
    being used, the following default slots are added:
    Value, visited, mex and remotness. Furthermore, the
    DB_Table is populated with pointers to the bpdb
    functions. Last, the available schemes are added to the
    symdb_schemes list for the purpose of encoding and decoding
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
symdb_init(
        DB_Table *new_db
        )
{
	GMSTATUS status = STATUS_SUCCESS;

	//
	// check input parameter
	//

	if(NULL == new_db) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("symdb_init()", "Input parameter new_db is null", status);
		goto _bailout;
	}

	if(0 == gNumberOfPositions) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("symdb_init()", "Invalid gNumberOfPositions value(0)", status);
		goto _bailout;
	}

	functionsMapping = new_db;

	//
	// initialize global variables
	//

	symdb_slices = gNumberOfPositions;

	//
	// allocate space to store the format of a slice
	//

	symdb_write_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
	if(NULL == symdb_write_slice) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_init()", "Could not allocate symdb_write_slice in memory", status);
		goto _bailout;
	}

	symdb_nowrite_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
	if(NULL == symdb_nowrite_slice) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_init()", "Could not allocate symdb_nowrite_slice in memory", status);
		goto _bailout;
	}

	//
	// if the solver being used is not variable slices aware, then add the default slots
	// that all old gamesman solvers use including value, mex, remoteness and visited
	//

	if(!gBitPerfectDBSolver) {

		// add value slot
		status = symdb_add_slot( 2, "VALUE", TRUE, TRUE, FALSE, &SYMDB_VALUESLOT ); //slot 0
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_init()", "Could not add value slot", status);
			goto _bailout;
		}

		// add mex slot
		status = symdb_add_slot(5, "MEX", TRUE, TRUE, FALSE, &SYMDB_MEXSLOT );    //slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_init()", "Could not add mex slot", status);
			goto _bailout;
		}

		// add winby slot
		status = symdb_add_slot(5, "WINBY", TRUE, TRUE, FALSE, &SYMDB_WINBYSLOT ); //slot 2
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_init()", "Could not add winby slot", status);
			goto _bailout;
		}

		// add remoteness slot
		status = symdb_add_slot( 8, "REMOTENESS", TRUE, TRUE, TRUE, &SYMDB_REMSLOT ); //slot 4
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_init()", "Could not add remoteness slot", status);
			goto _bailout;
		}

		// add visited slot
		status = symdb_add_slot( 1, "VISITED", FALSE, FALSE, FALSE, &SYMDB_VISITEDSLOT ); //slot 1
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_init()", "Could not add visited slot", status);
			goto _bailout;
		}

		//
		// allocate the memory the db requires
		//

		status = symdb_allocate();
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_init()", "Failed call to symdb_allocate() to allocate bpdb arrays", status);
			goto _bailout;
		}
	}

	//
	// set functional pointers for generic DB_Table
	//

	new_db->get_value = symdb_get_value;
	new_db->put_value = symdb_set_value;
	new_db->get_remoteness = symdb_get_remoteness;
	new_db->put_remoteness = symdb_set_remoteness;
	new_db->check_visited = symdb_check_visited;
	new_db->mark_visited = symdb_mark_visited;
	new_db->unmark_visited = symdb_unmark_visited;
	new_db->get_mex = symdb_get_mex;
	new_db->put_mex = symdb_set_mex;
	new_db->get_winby = symdb_get_winby;
	new_db->put_winby = symdb_set_winby;
	new_db->save_database = symdb_save_database;
	new_db->load_database = symdb_load_database;
	new_db->allocate = symdb_allocate;
	new_db->add_slot = symdb_add_slot;
	new_db->get_slice_slot = symdb_get_slice_slot;
	new_db->set_slice_slot = symdb_set_slice_slot;
	new_db->set_slice_slot_max = symdb_set_slice_slot_max;
	new_db->free_db = symdb_free;

	// create a new singly-linked list of schemes
	symdb_schemes = slist_new();

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
	symdb_headerScheme = scheme1;

	//
	// add the schemes to the schemes list
	//

	symdb_schemes = slist_add( symdb_schemes, scheme0 );
	symdb_schemes = slist_add( symdb_schemes, scheme1 );
	symdb_schemes = slist_add( symdb_schemes, scheme2 );

_bailout:
	return status;
}


/*++

   Routine Description:

    symdb_allocate allocates the space required by the database
    to save data for each potential slice. The function requires
    that the slice format information has been defined.

   Arguments:

    None

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
symdb_allocate( )
{
	GMSTATUS status = STATUS_SUCCESS;
	UINT64 i = 0;

	// check whether slice formats have been set
	if(0 == symdb_write_slice->bits && 0 == symdb_nowrite_slice->bits) {
		status = STATUS_SLICE_FORMAT_NOT_SET;
		BPDB_TRACE("symdb_allocate()", "Slices are width 0 which indicates that they have not been formatted yet", status);
		goto _bailout;
	}

	if(symdb_readFromDisk) {
		return STATUS_SUCCESS;
	}

	symdb_write_array_length = (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(symdb_write_slice->bits));
	symdb_nowrite_array_length = (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(symdb_nowrite_slice->bits));

	// allocate room for data that will be written out to file
	symdb_write_array = (BYTE *) calloc( symdb_write_array_length, sizeof(BYTE) );
	if(NULL == symdb_write_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_allocate()", "Could not allocate symdb_write_array in memory", status);
		goto _bailout;
	}

	// allocate room for transient data that will only be stored in memory
	symdb_nowrite_array = (BYTE *) calloc( symdb_nowrite_array_length, sizeof(BYTE));
	if(NULL == symdb_write_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_allocate()", "Could not allocate symdb_nowrite_array in memory", status);
		goto _bailout;
	}

	// cannot put in init since bpdbsolver does not have any slots inputted
	// at the time when init is called
	if(!gBitPerfectDBAdjust) {
		for(i = 0; i < (symdb_write_slice->slots); i++) {
			symdb_write_slice->adjust[i] = FALSE;
		}
		for(i = 0; i < (symdb_nowrite_slice->slots); i++) {
			symdb_nowrite_slice->adjust[i] = FALSE;
		}
	}

	// everything will work without this for loop,
	// until someone changes the value of the undecided enum
	// which is at the time of writing this 0.

	//for(i = 0; i < symdb_slices; i++) {
	//    symdb_set_slice_slot( i, SYMDB_VALUESLOT, undecided );
	//}

_bailout:
	return status;
}


/*++

   Routine Description:

    symdb_free_slice frees the resources used by the slice format
    description structure.

   Arguments:

    sl - slice to be freed.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
symdb_free_slice( SLICE sl )
{
	GMSTATUS status = STATUS_SUCCESS;
	int i = 0;

	if(NULL == sl) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("symdb_free_slice()", "Input parameter sl is null", status);
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

    symdb_free frees the resources used by th bpdb. This includes
    freeing both in-memory and to-be-written data, as well as
    slice formats. Schemes, as well as the list that contains them,
    are also freed.

   Arguments:

    None

   Return value:

    None

   --*/

void
symdb_free( )
{
	GMSTATUS status = STATUS_SUCCESS;
	SLIST cur = NULL;

	// free open file if necessary
	if(symdb_readFromDisk) {
		status = bitlib_file_close(symdb_readFile);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_load_database()", "call to bitlib to open file failed", status);
			goto _bailout;
		}
	}

	// free write slice format
	status = symdb_free_slice( symdb_write_slice );
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("symdb_free()", "Call to free symdb_write_slice failed", status);
		goto _bailout;
	}

	// free nowrite slice format
	status = symdb_free_slice( symdb_nowrite_slice );
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("symdb_free()", "Call to free symdb_nowrite_slice failed", status);
		goto _bailout;
	}

	// free in-memory database
	SAFE_FREE( symdb_nowrite_array );

	// free database to be written
	SAFE_FREE( symdb_write_array );

	// free each scheme
	cur = symdb_schemes;
	while( NULL != cur ) {
		scheme_free(cur->obj);
		cur = cur->next;
	}

	// free the list that contained the schemes
	slist_free( symdb_schemes );

_bailout:
	//return status;
	return;
}


VALUE
symdb_set_value(
        POSITION pos,
        VALUE val
        )
{
	symdb_set_slice_slot( (UINT64)pos, SYMDB_VALUESLOT, (UINT64) val );
	return val;
}

VALUE
symdb_get_value(
        POSITION pos
        )
{
	return (VALUE) functionsMapping->get_slice_slot( (UINT64)pos, SYMDB_VALUESLOT );
}

REMOTENESS
symdb_get_remoteness(
        POSITION pos
        )
{
	REMOTENESS rem = (REMOTENESS) functionsMapping->get_slice_slot( (UINT64)pos, SYMDB_REMSLOT );
	if(symdb_write_slice->maxvalue[SYMDB_REMSLOT/2]+1 == rem) {
		return REMOTENESS_MAX;
	} else {
		return rem;
	}
}

void
symdb_set_remoteness(
        POSITION pos,
        REMOTENESS val
        )
{
	symdb_set_slice_slot( (UINT64)pos, SYMDB_REMSLOT, (REMOTENESS) val );
}

BOOLEAN
symdb_check_visited(
        POSITION pos
        )
{
	return (BOOLEAN) functionsMapping->get_slice_slot( (UINT64)pos, SYMDB_VISITEDSLOT );
}

void
symdb_mark_visited(
        POSITION pos
        )
{
	symdb_set_slice_slot( (UINT64)pos, SYMDB_VISITEDSLOT, (UINT64)1 );
}

void
symdb_unmark_visited(
        POSITION pos
        )
{
	symdb_set_slice_slot( (UINT64)pos, SYMDB_VISITEDSLOT, (UINT64)0 );
}

void
symdb_set_mex(
        POSITION pos,
        MEX mex
        )
{
	symdb_set_slice_slot( (UINT64)pos, SYMDB_MEXSLOT, (UINT64)mex );
}

MEX
symdb_get_mex(
        POSITION pos
        )
{
	return (MEX) functionsMapping->get_slice_slot( (UINT64)pos, SYMDB_MEXSLOT );
}

void
symdb_set_winby(
        POSITION pos,
        WINBY winBy
        )
{
	symdb_set_slice_slot( (UINT64)pos, SYMDB_WINBYSLOT, (UINT64)winBy );
}

WINBY
symdb_get_winby(
        POSITION pos
        )
{
	return (WINBY) functionsMapping->get_slice_slot( (UINT64)pos, SYMDB_WINBYSLOT );
}

/*++

   Routine Description:

    symdb_set_slice_slot sets a specific slot of a given slice to the
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
symdb_set_slice_slot(
        UINT64 position,
        UINT8 index,
        UINT64 value
        )
{
	UINT64 byteOffset = 0;
	UINT8 bitOffset = 0;
	BYTE *symdb_array = NULL;
	SLICE symdb_slice = NULL;
	BOOLEAN write = TRUE;

	if(index % 2) {
		symdb_array = symdb_nowrite_array;
		symdb_slice = symdb_nowrite_slice;
		write = FALSE;
	} else {
		symdb_array = symdb_write_array;
		symdb_slice = symdb_write_slice;
	}
	index /= 2;

	if(value > symdb_slice->maxseen[index]) {
		symdb_slice->maxseen[index] = value;
	}

	if(value > symdb_slice->maxvalue[index]) {
		if(symdb_slice->adjust[index]) {
			symdb_grow_slice(symdb_array, symdb_slice, index, value);
			if(write) {
				symdb_array = symdb_write_array;
			} else {
				symdb_array = symdb_nowrite_array;
			}
		} else {
			if(!symdb_slice->overflowed[index]) {
				if(!symdb_have_printed) {
					symdb_have_printed = TRUE;
					printf("\n");
				}
				printf("Warning: Slot %s with bit size %u had to be rounded from %llu to its maxvalue %llu.\n",
				       symdb_slice->name[index],
				       symdb_slice->size[index],
				       value,
				       symdb_slice->maxvalue[index]
				       );
				symdb_slice->overflowed[index] = TRUE;
			}
			value = symdb_slice->maxvalue[index];
		}
	}

	byteOffset = (symdb_slice->bits * position)/BITSINBYTE;
	bitOffset = ((UINT8)(symdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
	bitOffset += symdb_slice->offset[index];

	byteOffset += bitOffset / BITSINBYTE;
	bitOffset %= BITSINBYTE;

	//printf("byteoff: %d bitoff: %d value: %llu length: %d\n", byteOffset, bitOffset, value, length);
	//printf("value: %llu\n", value);
	bitlib_insert_bits( symdb_array + byteOffset, bitOffset, value, symdb_slice->size[index] );

	return value;
}


/*++

   Routine Description:

    symdb_set_slice_slot_max sets a specific slot of a given slice to the
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
symdb_set_slice_slot_max(
        UINT64 position,
        UINT8 index
        )
{
	UINT64 byteOffset = 0;
	UINT8 bitOffset = 0;
	BYTE *symdb_array = NULL;
	SLICE symdb_slice = NULL;

	if(index % 2) {
		symdb_array = symdb_nowrite_array;
		symdb_slice = symdb_nowrite_slice;
	} else {
		symdb_array = symdb_write_array;
		symdb_slice = symdb_write_slice;
	}
	index /= 2;

	if(!symdb_slice->reservemax[index]) {
		BPDB_TRACE("symdb_set_slice_slot_max()", "slot without its maxvalue reserved is being set to max", STATUS_INVALID_OPERATION);
	}

	byteOffset = (symdb_slice->bits * position)/BITSINBYTE;
	bitOffset = ((UINT8)(symdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
	bitOffset += symdb_slice->offset[index];

	byteOffset += bitOffset / BITSINBYTE;
	bitOffset %= BITSINBYTE;

	bitlib_insert_bits( symdb_array + byteOffset, bitOffset, symdb_slice->maxvalue[index]+1, symdb_slice->size[index] );

	return symdb_slice->maxvalue[index]+1;
}


GMSTATUS
symdb_grow_slice(
        BYTE *symdb_array,
        SLICE symdb_slice,
        UINT8 index,
        UINT64 value
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	BYTE *symdb_new_array = NULL;

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
	//symdb_dump_database(1);

	if(symdb_slice->reservemax[index]) {
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
	bitsToAddToSlot -= symdb_slice->size[index];

	// save old slice size
	oldSlotSize = symdb_slice->size[index];

	// number of additional bits
	newSlotSize = oldSlotSize + bitsToAddToSlot;

	// bits
	oldSliceSize = symdb_slice->bits;
	newSliceSize = symdb_slice->bits + bitsToAddToSlot;
	if(!symdb_have_printed) {
		symdb_have_printed = TRUE;
		printf("\n");
	}

	if(gBitPerfectDBVerbose) {
		printf("Expanding database (Slot %s %u bits->%u bits)... ", symdb_slice->name[index], oldSlotSize, newSlotSize);
	}

	// allocate new space needed for the larger database
	symdb_new_array = (BYTE *) realloc( symdb_array, (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) ) * sizeof(BYTE));
	if(NULL == symdb_new_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_grow_slice()", "Could not allocate new database", status);
		goto _bailout;
	}

	// determine whether new array replaces the write
	// array or the nowrite array
	if(symdb_array == symdb_write_array) {
		symdb_write_array = symdb_new_array;
		symdb_write_array_length = (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	} else {
		symdb_nowrite_array = symdb_new_array;
		symdb_nowrite_array_length = (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	}

	// debug-temp
	//symdb_dump_database(2);

	// set slot to new size
	symdb_slice->size[index] += bitsToAddToSlot;
	// set slice to new size
	symdb_slice->bits += bitsToAddToSlot;

	// set offsets to new values
	for(currentSlot = 0; currentSlot < symdb_slice->slots; currentSlot++) {
		if(currentSlot > index) {
			symdb_slice->offset[currentSlot] += bitsToAddToSlot;
		}
	}

	// set new max value
	UINT64 oldmaxvalue = symdb_slice->maxvalue[index];
	symdb_slice->maxvalue[index] = (UINT64)pow(2, newSlotSize) - 1;
	if(symdb_slice->reservemax[index]) {
		symdb_slice->maxvalue[index]--;
	}

	leftSize = symdb_slice->offset[index];
	rightSize = symdb_slice->bits - symdb_slice->size[index] - leftSize;

	//printf( "start while (value %llu, add %d)\n", value, bitsToAddToSlot );

	// for each record starting from the max record
	// copy from the old db format to the new db format
	currentSlice = symdb_slices - 1;

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

		data = bitlib_read_bits( symdb_new_array + mfbyteOffset, mfbitOffset, rightSize );
		//printf("%llu(%u) ", data, rightSize);
		bitlib_insert_bits( symdb_new_array + mtbyteOffset, mtbitOffset, data, rightSize );


		// middle
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize;
		mtbitOffset += leftSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( symdb_new_array + mfbyteOffset, mfbitOffset, oldSlotSize );
		//printf("%llu(%u %u) ", data, oldSlotSize, newSlotSize);
		if(symdb_slice->reservemax[index] && data == oldmaxvalue + 1) {
			data = symdb_slice->maxvalue[index] + 1;
		}
		bitlib_insert_bits( symdb_new_array + mtbyteOffset, mtbitOffset, data, newSlotSize );

		// left
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;

		data = bitlib_read_bits( symdb_new_array + mfbyteOffset, mfbitOffset, leftSize );
		//printf("%llu(%u) \n", data, leftSize);
		bitlib_insert_bits( symdb_new_array + mtbyteOffset, mtbitOffset, data, leftSize );


		if(currentSlice == 0) break;
		currentSlice--;
	}

	printf("done\n");

	/*
	   // debugging information
	   printf("Offsets: ");
	   for(currentSlot = 0; currentSlot < symdb_slice->slots; currentSlot++) {
	    printf(" %u(%llu)", symdb_slice->offset[currentSlot], currentSlot);
	   }
	   printf("\n");

	   printf("OldSlice: %u NewSlice: %u OldSlot: %u NewSlot: %u Totalbits %u\n", oldSliceSize, newSliceSize, oldSlotSize, newSlotSize, symdb_slice->bits);
	   printf("Leftsize: %u Rightsize: %u\n", leftSize, rightSize);
	 */

	// debug-temp
	//symdb_dump_database(3);

_bailout:
	return status;
}



GMSTATUS
symdb_shrink_slice(
        BYTE *symdb_array,
        SLICE symdb_slice,
        UINT8 index
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	BYTE *symdb_new_array = NULL;

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
	UINT64 temp = symdb_slice->maxseen[index];

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
	//symdb_dump_database(1);

	if(symdb_slice->reservemax[index]) {
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
	bitsToShrink = symdb_slice->size[index] - bitsToShrink;

	// save old slice size
	oldSlotSize = symdb_slice->size[index];

	// number of additional bits
	newSlotSize = oldSlotSize - bitsToShrink;

	// bits
	oldSliceSize = symdb_slice->bits;
	newSliceSize = symdb_slice->bits - bitsToShrink;
	if(!symdb_have_printed) {
		symdb_have_printed = TRUE;
		printf("\n");
	}

	if(gBitPerfectDBVerbose) {
		printf("Shrinking (Slot %s %u bits->%u bits)... ", symdb_slice->name[index], oldSlotSize, newSlotSize);
	}

	// debug-temp
	//symdb_dump_database(2);

	// set slot to new size
	symdb_slice->size[index] -= bitsToShrink;
	// set slice to new size
	symdb_slice->bits -= bitsToShrink;

	// set offsets to new values
	for(currentSlot = 0; currentSlot < symdb_slice->slots; currentSlot++) {
		if(currentSlot > index) {
			symdb_slice->offset[currentSlot] -= bitsToShrink;
		}
	}

	// set new max value
	UINT64 oldmaxvalue = symdb_slice->maxvalue[index];
	symdb_slice->maxvalue[index] = (UINT64)pow(2, newSlotSize) - 1;
	if(symdb_slice->reservemax[index]) {
		symdb_slice->maxvalue[index]--;
	}

	leftSize = symdb_slice->offset[index];
	rightSize = symdb_slice->bits - symdb_slice->size[index] - leftSize;

	//printf( "start while (value %llu, add %d)\n", value, bitsToAddToSlot );

	// for each record
	// copy from the old db format to the new db format

	for(currentSlice = 0; currentSlice < symdb_slices; currentSlice++) {
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

		data = bitlib_read_bits( symdb_array + mfbyteOffset, mfbitOffset, leftSize );
		//printf("%llu(%u) \n", data, leftSize);
		bitlib_insert_bits( symdb_array + mtbyteOffset, mtbitOffset, data, leftSize );

		// middle
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize;
		mtbitOffset += leftSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( symdb_array + mfbyteOffset, mfbitOffset, oldSlotSize );
		if(symdb_slice->reservemax[index] && data == oldmaxvalue + 1) {
			data = symdb_slice->maxvalue[index] + 1;
		}
		//printf("%llu(%u %u) ", data, oldSlotSize, newSlotSize);
		bitlib_insert_bits( symdb_array + mtbyteOffset, mtbitOffset, data, newSlotSize );

		// right
		mfbyteOffset = fbyteOffset; mfbitOffset = fbitOffset;
		mtbyteOffset = tbyteOffset; mtbitOffset = tbitOffset;
		mfbitOffset += leftSize + oldSlotSize;
		mtbitOffset += leftSize + newSlotSize;
		mfbyteOffset += mfbitOffset / BITSINBYTE;
		mfbitOffset %= BITSINBYTE;
		mtbyteOffset += mtbitOffset / BITSINBYTE;
		mtbitOffset %= BITSINBYTE;

		data = bitlib_read_bits( symdb_array + mfbyteOffset, mfbitOffset, rightSize );
		//printf("%llu(%u) ", data, rightSize);
		bitlib_insert_bits( symdb_array + mtbyteOffset, mtbitOffset, data, rightSize );


	}

	if(gBitPerfectDBVerbose) {
		printf("done\n");
	}


	// allocate new space needed for the larger database
	symdb_new_array = (BYTE *) realloc( symdb_array, (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) ) * sizeof(BYTE));
	if(NULL == symdb_new_array) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_shrink_slice()", "Could not allocate new database", status);
		goto _bailout;
	}

	// determine whether new array replaces the write
	// array or the nowrite array
	if(symdb_array == symdb_write_array) {
		symdb_write_array = symdb_new_array;
		symdb_write_array_length = (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	} else {
		symdb_nowrite_array = symdb_new_array;
		symdb_nowrite_array_length = (size_t)ceil(((double)symdb_slices/(double)BITSINBYTE) * (size_t)(newSliceSize) );
	}

	/*
	   // debugging information
	   printf("Offsets: ");
	   for(currentSlot = 0; currentSlot < symdb_slice->slots; currentSlot++) {
	    printf(" %u(%llu)", symdb_slice->offset[currentSlot], currentSlot);
	   }
	   printf("\n");

	   printf("OldSlice: %u NewSlice: %u OldSlot: %u NewSlot: %u Totalbits %u\n", oldSliceSize, newSliceSize, oldSlotSize, newSlotSize, symdb_slice->bits);
	   printf("Leftsize: %u Rightsize: %u\n", leftSize, rightSize);
	 */

	// debug-temp
	//symdb_dump_database(3);

_bailout:
	return status;
}


/*++

   Routine Description:

    symdb_get_slice_slot retrieves the value of a specific slot of a
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
symdb_get_slice_slot_disk(
        UINT64 position,
        UINT8 index
        );

UINT64
symdb_get_slice_slot(
        UINT64 position,
        UINT8 index
        )
{
	UINT64 byteOffset = 0;
	UINT8 bitOffset = 0;
	BYTE *symdb_array = NULL;
	SLICE symdb_slice = NULL;

	if(index % 2) {
		symdb_array = symdb_nowrite_array;
		symdb_slice = symdb_nowrite_slice;
	} else {
		symdb_array = symdb_write_array;
		symdb_slice = symdb_write_slice;
	}
	index /= 2;

	byteOffset = (symdb_slice->bits * position)/BITSINBYTE;
	bitOffset = ((UINT8)(symdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
	bitOffset += symdb_slice->offset[index];

	byteOffset += bitOffset / BITSINBYTE;
	bitOffset %= BITSINBYTE;

	return bitlib_read_bits( symdb_array + byteOffset, bitOffset, symdb_slice->size[index] );
}

UINT64
symdb_get_slice_slot_disk(
        UINT64 position,
        UINT8 index
        )
{
	BYTE *inputBuffer = NULL;
	BYTE *curBuffer = NULL;

	index /= 2;

	// assign offset of first bit
	UINT8 offset = symdb_readOffset;

	// counters
	UINT64 currentSlice = 0;
	UINT8 currentSlot = 0;

	// Eventually REMOVE these NULL checks, once the
	// code is mature and these NULL errors do not occur.
	if(NULL == symdb_readFile) {
		BPDB_TRACE("symdb_get_slice_slot_disk()", "symdb_readFile is NULL.", STATUS_INVALID_INPUT_PARAMETER);
		goto _bailout;
	}

	if(NULL == symdb_readScheme) {
		BPDB_TRACE("symdb_get_slice_slot_disk()", "symdb_readScheme is NULL.", STATUS_INVALID_INPUT_PARAMETER);
		goto _bailout;
	}

	// seek to desired location
	bitlib_file_seek(symdb_readFile, symdb_readStart, SEEK_SET);

	// initialize buffer
	inputBuffer = alloca( symdb_buffer_length * sizeof(BYTE) );
	memset( inputBuffer, 0, symdb_buffer_length );
	curBuffer = inputBuffer;

	// read in data to buffer
	bitlib_file_read_bytes( symdb_readFile, inputBuffer, symdb_buffer_length );

	if(symdb_readScheme->indicator) {

		while(currentSlice < position) {
			if(bitlib_read_from_buffer( symdb_readFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, 1 ) == 0) {

				for(currentSlot = 0; currentSlot < (symdb_write_slice->slots); currentSlot++) {
					bitlib_read_from_buffer( symdb_readFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, symdb_write_slice->size[currentSlot]);
				}
				currentSlice++;
			} else {
				UINT64 skips = symdb_generic_read_varnum( symdb_readFile, symdb_readScheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, TRUE );
				currentSlice += skips;
			}
		}

		if(currentSlice == position)
		{
			// if slice is part of a range of skips, return 0
			if(bitlib_read_from_buffer( symdb_readFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, 1 ) != 0) {
				return 0;
			}

			for(currentSlot = 0; currentSlot < (symdb_write_slice->slots); currentSlot++) {
				UINT64 value = bitlib_read_from_buffer( symdb_readFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, symdb_write_slice->size[currentSlot]);

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

		byteOffset = (symdb_write_slice->bits * position)/BITSINBYTE;
		bitOffset = ((UINT8)(symdb_write_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
		bitOffset += symdb_write_slice->offset[index];

		byteOffset += bitOffset / BITSINBYTE;
		bitOffset %= BITSINBYTE;

		bitlib_file_seek(symdb_readFile, symdb_readStart+byteOffset, SEEK_SET);

		bitlib_file_read_bytes( symdb_readFile, inputBuffer, symdb_buffer_length );

		return bitlib_read_from_buffer( symdb_readFile, &curBuffer, inputBuffer, symdb_buffer_length, &bitOffset, symdb_write_slice->size[index]);
	}

_bailout:
	return 0;
}

/*++

   Routine Description:

    symdb_add_slot retrieves the value of a specific slot of a
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
symdb_add_slot(
        UINT8 size,
        char *name,
        BOOLEAN write,
        BOOLEAN adjust,
        BOOLEAN reservemax,
        UINT32 *slotindex
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	SLICE symdb_slice = NULL;

	//
	// check input parameters
	//

	if(NULL == name) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("symdb_add_slot()", "name is passed in as null", status);
		goto _bailout;
	}

	if(NULL == symdb_write_slice) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("symdb_add_slot()", "symdb_write_slice must be initialized first. call symdb_init()", status);
		goto _bailout;
	}

	if(NULL == symdb_nowrite_slice) {
		status = STATUS_INVALID_INPUT_PARAMETER;
		BPDB_TRACE("symdb_add_slot()", "symdb_nowrite_slice must be initialized first. call symdb_init()", status);
		goto _bailout;
	}

	// even indexes are used for slots to be written
	// odd indexes are used for slots that are in-memory
	if(write) {
		symdb_slice = symdb_write_slice;
		*slotindex = 0;
	} else {
		symdb_slice = symdb_nowrite_slice;
		*slotindex = 1;
	}

	*slotindex += symdb_slice->slots*2;

	// for backwards compatibility
	if(strcmp(name, "VALUE") == 0) SYMDB_VALUESLOT = *slotindex;
	else if(strcmp(name, "MEX") == 0) SYMDB_MEXSLOT = *slotindex;
	else if(strcmp(name, "WINBY") == 0) SYMDB_WINBYSLOT = *slotindex;
	else if(strcmp(name, "REMOTENESS") == 0) SYMDB_REMSLOT = *slotindex;
	else if(strcmp(name, "VISITED") == 0) SYMDB_VISITEDSLOT = *slotindex;

	symdb_slice->slots++;

	//
	// allocate new slot structure
	//

	if(symdb_slice->slots == 1) {
		symdb_slice->size = (UINT8 *) calloc( 1, sizeof(UINT8) );
		symdb_slice->offset = (UINT32 *) calloc( 1, sizeof(UINT32) );
		symdb_slice->maxvalue = (UINT64 *) calloc( 1, sizeof(UINT64) );
		symdb_slice->maxseen = (UINT64 *) calloc( 1, sizeof(UINT64) );
		symdb_slice->name = (char **) calloc( 1, sizeof(char **) );
		symdb_slice->overflowed = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
		symdb_slice->adjust = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
		symdb_slice->reservemax = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
	} else {
		symdb_slice->size = (UINT8 *) realloc( symdb_slice->size, symdb_slice->slots*sizeof(UINT8) );
		symdb_slice->offset = (UINT32 *) realloc( symdb_slice->offset, symdb_slice->slots*sizeof(UINT32) );
		symdb_slice->maxvalue = (UINT64 *) realloc( symdb_slice->maxvalue, symdb_slice->slots*sizeof(UINT64) );
		symdb_slice->maxseen = (UINT64 *) realloc( symdb_slice->maxseen, symdb_slice->slots*sizeof(UINT64) );
		symdb_slice->name = (char **) realloc( symdb_slice->name, symdb_slice->slots*sizeof(char **) );
		symdb_slice->overflowed = (BOOLEAN *) realloc( symdb_slice->overflowed, symdb_slice->slots*sizeof(BOOLEAN) );
		symdb_slice->adjust = (BOOLEAN *) realloc( symdb_slice->adjust, symdb_slice->slots*sizeof(BOOLEAN) );
		symdb_slice->reservemax = (BOOLEAN *) realloc( symdb_slice->reservemax, symdb_slice->slots*sizeof(BOOLEAN) );
	}

	//
	// set new slot attributes
	//

	symdb_slice->bits += size;
	symdb_slice->size[symdb_slice->slots-1] = size;
	if(symdb_slice->slots == 1) {
		symdb_slice->offset[symdb_slice->slots-1] = 0;
	} else {
		symdb_slice->offset[symdb_slice->slots-1] = symdb_slice->offset[symdb_slice->slots - 2] + symdb_slice->size[symdb_slice->slots - 2];
	}
	symdb_slice->maxvalue[symdb_slice->slots-1] = (UINT64)pow(2, size) - 1;
	symdb_slice->maxseen[symdb_slice->slots-1] = 0;
	symdb_slice->name[symdb_slice->slots-1] = (char *) malloc( strlen(name) + 1 );
	strcpy( symdb_slice->name[symdb_slice->slots-1], name );
	symdb_slice->overflowed[symdb_slice->slots-1] = FALSE;
	symdb_slice->adjust[symdb_slice->slots-1] = adjust;

	// if max is reserved, reduce the maxvalue by 1 since the max should
	// only be set by a special function call
	symdb_slice->reservemax[symdb_slice->slots-1] = reservemax;
	if(reservemax) {
		symdb_slice->maxvalue[symdb_slice->slots-1]--;
	}

_bailout:
	return status;
}


/*++

   Routine Description:

    symdb_print_database prints each slice of the database with
    each slot indicated to stdout.

   Arguments:

    None

   Return value:

    None

   --*/

void
symdb_print_database()
{
	UINT64 i = 0;
	UINT8 j = 0;

	printf("\n\nDatabase Printout\n");

	for(i = 0; i < symdb_slices; i++) {
		printf("Slice %llu  (write) ", i);
		for(j=0; j < (symdb_write_slice->slots); j++) {
			printf("%s: %llu  ", symdb_write_slice->name[j], symdb_get_slice_slot(i, j*2));
		}
		printf("(no write) ");
		for(j=0; j < (symdb_nowrite_slice->slots); j++) {
			printf("%s: %llu  ", symdb_nowrite_slice->name[j], symdb_get_slice_slot(i, j*2+1));
		}
		printf("\n");
	}
}


/*++

   Routine Description:

    symdb_dump_database dumps the contents of the database to
    a file.

   Arguments:

    num - identifier for file name

   Return value:

    None

   --*/

void
symdb_dump_database( int num )
{
	char filename[256];
	UINT64 i = 0;
	UINT8 j = 0;
	UINT32 currentSlot = 0;

	mkdir("dumps", 0755);

	sprintf(filename, "./dumps/m%s_%d_symdb_%d.dump", kDBName, getOption(), num);

	FILE *f = fopen( filename, "wb");

	fprintf(f, "\nDatabase Printout\n");

	fprintf(f, "Offsets: ");
	for(currentSlot = 0; currentSlot < symdb_write_slice->slots; currentSlot++) {
		fprintf(f, " %u(%u)", symdb_write_slice->offset[currentSlot], currentSlot);
	}
	fprintf(f, "\n");

	for(i = 0; i < symdb_slices; i++) {
		fprintf(f, "Slice %llu  (write) ", i);
		for(j=0; j < (symdb_write_slice->slots); j++) {
			fprintf(f, "%s: %llu  ", symdb_write_slice->name[j], symdb_get_slice_slot(i, j*2));
		}
		fprintf(f, "(no write) ");
		for(j=0; j < (symdb_nowrite_slice->slots); j++) {
			fprintf(f, "%s: %llu  ", symdb_nowrite_slice->name[j], symdb_get_slice_slot(i, j*2+1));
		}
		fprintf(f, "\n");
	}

	fclose(f);
}


/*++

   Routine Description:

    symdb_analyze_database is used for experimentation and
    analysis

   Arguments:

    num - identifier for file to be dumped to

   Return value:

    None

   --*/

void
symdb_analyze_database( int num )
{
	char filename[256];
	UINT64 i = 0;
//    UINT8 j = 0;
//    UINT32 currentSlot = 0;

	mkdir("dumps", 0755);

	sprintf(filename, "./dumps/m%s_%d_symdb_analyze_%d.dump", kDBName, getOption(), num);

	FILE *f = fopen( filename, "wb");

	HTABLE ht = htable_new( 1000 );

	//printf("Num of write array length: %u\n", symdb_write_array_length);

	for(i=0; i < symdb_write_array_length; i++) {
		UINT32 val = htable_get( ht, symdb_write_array[i] );
		htable_set( ht, symdb_write_array[i], val + 1 );
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
    for(currentSlot = 0; currentSlot < symdb_write_slice->slots; currentSlot++) {
        fprintf(f, " %u(%u)", symdb_write_slice->offset[currentSlot], currentSlot);
    }
    fprintf(f, "\n");

    for(i = 0; i < symdb_slices; i++) {
        fprintf(f, "Slice %llu  (write) ", i);
        for(j=0; j < (symdb_write_slice->slots); j++) {
            fprintf(f, "%s: %llu  ", symdb_write_slice->name[j], symdb_get_slice_slot(i, j*2));
        }
        fprintf(f, "(no write) ");
        for(j=0; j < (symdb_nowrite_slice->slots); j++) {
            fprintf(f, "%s: %llu  ", symdb_nowrite_slice->name[j], symdb_get_slice_slot(i, j*2+1));
        }
        fprintf(f, "\n");
    }
 */
	fclose(f);
}


/*++

   Routine Description:

    symdb_save_database writes the (write) database to
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
symdb_save_database()
{
	// debug-temp
	//symdb_print_database();
	//symdb_analyze_database(1);

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

	if(0 == slist_size(symdb_schemes)) {
		status = STATUS_NO_SCHEMES_INSTALLED;
		BPDB_TRACE("symdb_save_database()", "no encoding schemes installed to save db file", status);
		goto _bailout;
	}

	// set counters
	i = 0;
	cur = symdb_schemes;

	// allocate memory for all db file names
	outfilenames = (char **) malloc( slist_size(symdb_schemes)*sizeof(char*) );
	if(NULL == outfilenames) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_save_database()", "Could not allocate outfilenames in memory", status);
		goto _bailout;
	}

	// allocate memory for each filename
	for(i = 0; i<slist_size(symdb_schemes); i++) {
		outfilenames[i] = (char *) malloc( 256*sizeof(char) );
		if(NULL == outfilenames[i]) {
			status = STATUS_NOT_ENOUGH_MEMORY;
			BPDB_TRACE("symdb_save_database()", "Could not allocate outfilename in memory", status);
			goto _bailout;
		}
	}

/*
    // debug-temp
    // print out the status of the slots after solving
    printf("\nNum slots: %u\n", symdb_write_slice->slots);

    for(i = 0; i<symdb_write_slice->slots; i++) {
        printf("%s: %llu\n", symdb_write_slice->name[i], symdb_write_slice->maxseen[i]);
    }
 */

	symdb_have_printed = FALSE;

	UINT64 temp = 0;
	UINT8 bitsNeeded = 0;
	for(i = 0; i < (symdb_write_slice->slots); i++) {
		temp = symdb_write_slice->maxseen[i];
		bitsNeeded = 0;

		if(symdb_write_slice->reservemax[i]) {
			temp++;
		}

		while(0 != temp) {
			bitsNeeded++;
			temp = temp >> 1;
		}

		if(bitsNeeded < symdb_write_slice->size[i] && symdb_write_slice->adjust[i]) {
			symdb_shrink_slice( symdb_write_array, symdb_write_slice, i );
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
		sprintf(outfilenames[i], "./data/m%s_%d_symdb_%d.dat.gz", kDBName, getOption(), ((SCHEME)cur->obj)->id);

		status = symdb_generic_save_database( (SCHEME) cur->obj, outfilenames[i] );
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_save_database()", "call to symdb_generic_save_database with scheme failed", status);
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
	sprintf(outfilename, "./data/m%s_%d_symdb.dat.gz", kDBName, getOption());

	if(gBitPerfectDBVerbose) {
		printf("Renaming %s to %s\n", outfilenames[smallestscheme], outfilename);
	}
	rename(outfilenames[smallestscheme], outfilename);

_bailout:

	for(i = 0; i<slist_size(symdb_schemes); i++) {
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

    symdb_generic_write_varnum writes an input specified
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
symdb_generic_write_varnum(
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

    symdb_save_database saves the contents of the symdb_write_array
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
symdb_generic_save_database(
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

	outputBuffer = alloca( symdb_buffer_length * sizeof(BYTE));
	memset(outputBuffer, 0, symdb_buffer_length);
	curBuffer = outputBuffer;

	mkdir("data", 0755);

	status = bitlib_file_open(outfilename, "wb", &outFile);
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("symdb_generic_save_database()", "call to bitlib to open file failed", status);
		goto _bailout;
	}

	bitlib_value_to_buffer ( outFile, &curBuffer, outputBuffer, symdb_buffer_length, &offset, scheme->id, 8 );

	symdb_generic_write_varnum( outFile, symdb_headerScheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_slices );
	symdb_generic_write_varnum( outFile, symdb_headerScheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_write_slice->bits );

	symdb_generic_write_varnum( outFile, symdb_headerScheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_write_slice->slots );

	for(i = 0; i < (symdb_write_slice->slots); i++) {
		symdb_generic_write_varnum( outFile, symdb_headerScheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_write_slice->size[i]+1 );
		symdb_generic_write_varnum( outFile, symdb_headerScheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, strlen(symdb_write_slice->name[i]) );

		for(j = 0; j<strlen(symdb_write_slice->name[i]); j++) {
			bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_write_slice->name[i][j], 8 );
		}

		bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_write_slice->overflowed[i], 1 );
	}

	if(scheme->indicator) {
		for(slice = 0; slice<symdb_slices; slice++) {

			// Check if the slice has a mapping
			if(symdb_get_slice_slot( slice, 0 ) != undecided) {

				// If so, then check to see if skips must be outputted
				if(consecutiveSkips != 0) {
					// Put skips into output buffer
					symdb_generic_write_varnum( outFile, scheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, consecutiveSkips);
					// Reset skip counter
					consecutiveSkips = 0;
				}
				bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, symdb_buffer_length, &offset, 0, 1 );
				for(slot=0; slot < (symdb_write_slice->slots); slot++) {
					bitlib_value_to_buffer( outFile, &curBuffer, outputBuffer, symdb_buffer_length, &offset, symdb_get_slice_slot(slice, 2*slot), symdb_write_slice->size[slot] );
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

		bitlib_file_write_bytes(outFile, symdb_write_array, symdb_write_array_length);
	}

	if(consecutiveSkips != 0) {
		symdb_generic_write_varnum( outFile, scheme, &curBuffer, outputBuffer, symdb_buffer_length, &offset, consecutiveSkips);
		consecutiveSkips = 0;
	}

	if(curBuffer != outputBuffer || offset != 0) {
		bitlib_file_write_bytes(outFile, outputBuffer, curBuffer-outputBuffer+1);
	}

	status = bitlib_file_close(outFile);
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("symdb_generic_save_database()", "call to bitlib to close file failed", status);
		goto _bailout;
	}

_bailout:
	return status;
}


/*++

   Routine Description:

    symdb_load_database is an exposed function that can be
    called to load a database for the current game.

    Flow:
    1. Checks whether the desired file exists to load from
    2. Finds the matching scheme to decode the file with
    3. Calls symdb_generic_load_database() to read file

   Arguments:

    None

   Return value:

    TRUE on successful execution, or FALSE otherwise

   --*/


BOOLEAN
symdb_load_database( )
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
	sprintf(outfilename, "./data/m%s_%d_symdb.dat.gz", kDBName, getOption());

	if(NULL != (testOpen = fopen(outfilename, "r"))) {
		fclose(testOpen);

		status = bitlib_file_open(outfilename, "rb", &inFile);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_load_database()", "call to bitlib to open file failed", status);
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

	cur = symdb_schemes;

	while(((SCHEME)cur->obj)->id != fileFormat) {
		if( NULL == cur || NULL == cur->next) {
			status = STATUS_SCHEME_NOT_FOUND;
			BPDB_TRACE("symdb_load_database()", "cannot find scheme to decode and decompress database", status);
			goto _bailout;
		}

		cur = cur->next;
	}

	symdb_readScheme = (SCHEME)cur->obj;

	status = symdb_generic_load_database( inFile, (SCHEME)cur->obj );
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("symdb_load_database()", "call to symdb_generic_load_database to load db with recognized scheme failed", status);
		goto _bailout;
	}

	// if the db will be read from file, assign the
	// file pointer to the zero-memory player file pointer
	// otherwise, close the file
	if(symdb_readFromDisk) {
		symdb_readFile = inFile;
	} else {
		status = bitlib_file_close(inFile);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_load_database()", "call to bitlib to close file failed", status);
			goto _bailout;
		}
	}

	// debug-temp
	//symdb_print_database();

_bailout:
	if(!GMSUCCESS(status)) {
		return FALSE;
	} else {
		return TRUE;
	}
}


/*++

   Routine Description:

    symdb_generic_load_database opens an existing file,
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
symdb_generic_load_database(
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

	inputBuffer = alloca( symdb_buffer_length * sizeof(BYTE) );
	memset( inputBuffer, 0, symdb_buffer_length );
	curBuffer = inputBuffer;

	// TO DO - check return value
	bitlib_file_read_bytes( inFile, inputBuffer, symdb_buffer_length );

	// Read Header
	numOfSlicesHeader = symdb_generic_read_varnum( inFile, symdb_headerScheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, FALSE );
	bitsPerSliceHeader = symdb_generic_read_varnum( inFile, symdb_headerScheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, FALSE );
	numOfSlotsHeader = symdb_generic_read_varnum( inFile, symdb_headerScheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, FALSE );

	if(gBitPerfectDBVerbose) {
		printf("Slices: %llu\nBits per slice: %d\nSlots per slice: %d\n\n", numOfSlicesHeader, bitsPerSliceHeader, numOfSlotsHeader);
	}

	symdb_free_slice( symdb_write_slice );

	symdb_write_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
	if(NULL == symdb_write_slice) {
		status = STATUS_NOT_ENOUGH_MEMORY;
		BPDB_TRACE("symdb_init()", "Could not allocate symdb_write_slice in memory", status);
		goto _bailout;
	}

	//
	// outputs slots in db
	// perhaps think about what happens if a slot is missing
	//

	for(i = 0; i<numOfSlotsHeader; i++) {
		// read size slice in bits
		tempsize = symdb_generic_read_varnum( inFile, symdb_headerScheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, FALSE );
		tempsize--;

		// read size of slot name in bytes
		tempnamesize = symdb_generic_read_varnum( inFile, symdb_headerScheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, FALSE );

		// allocate room for slot name
		tempname = (char *) malloc((tempnamesize + 1) * sizeof(char));

		// read name
		for(j = 0; j<tempnamesize; j++) {
			*(tempname + j) = (char)bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, 8 );
		}
		*(tempname + j) = '\0';

		// read overflowed bit
		tempoverflowed = bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, 1 );

		// output information on each slot to the user
		if(gBitPerfectDBVerbose) {
			printf("Slot: %s (%u bytes)\n", tempname, tempnamesize);
			printf("\tBit Width: %u\n", tempsize);
		}

		if(tempoverflowed) {
			printf("\tWarning: Some values have been capped at %llu due to overflow.\n", (UINT64)pow(2, tempsize) - 1);
		}

		status = symdb_add_slot(tempsize, tempname, TRUE, FALSE, FALSE, &tempslotindex);
		if(!GMSUCCESS(status)) {
			BPDB_TRACE("symdb_generic_load_database()", "call to symdb_add_slot to add slot from database file failed", status);
			goto _bailout;
		}

		SAFE_FREE( tempname );
	}

	symdb_readStart = curBuffer - inputBuffer + 1;
	symdb_readOffset = offset;

	if(!scheme->indicator) {
		symdb_readStart++;
	}

	if(gBitPerfectDBZeroMemoryPlayer) {
		if(gBitPerfectDBVerbose) {
			printf("\nUsing Zero-Memory Player...\n");
		}
		symdb_readFromDisk = TRUE;
		functionsMapping->get_slice_slot = symdb_get_slice_slot_disk;
		return STATUS_SUCCESS;
	}

	status = symdb_allocate();
	if(!GMSUCCESS(status)) {
		BPDB_TRACE("symdb_generic_load_database()", "call to symdb_allocate failed", status);
		goto _bailout;
	}

	if( scheme->indicator ) {
		showDBLoadingStatus (Clean);

		while(currentSlice < numOfSlicesHeader) {

			if(bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, 1 ) == 0) {

				for(currentSlot = 0; currentSlot < (symdb_write_slice->slots); currentSlot++) {
					symdb_set_slice_slot( currentSlice, 2*currentSlot,
					                      bitlib_read_from_buffer( inFile, &curBuffer, inputBuffer, symdb_buffer_length, &offset, symdb_write_slice->size[currentSlot]) );
				}
				showDBLoadingStatus (Update);

				currentSlice++;
			} else {
				skips = symdb_generic_read_varnum( inFile, scheme, &curBuffer, inputBuffer, symdb_buffer_length, &offset, TRUE );

				for(i = 0; i < skips; i++, currentSlice++) {
					symdb_set_slice_slot( currentSlice, 0, undecided );
					showDBLoadingStatus (Update);
				}
			}
		}
	} else {

		gzrewind(inFile);
		bitlib_file_seek( inFile, symdb_readStart, SEEK_SET);
		bitlib_file_read_bytes( inFile,
		                        symdb_write_array,
		                        symdb_write_array_length
		                        );

	}

_bailout:
	return status;
}

/*++

   Routine Description:

    symdb_generic_read_varnum opens an existing file,
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
symdb_generic_read_varnum(
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
