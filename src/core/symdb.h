/************************************************************************
**
** NAME:    symdb.h
**
** DESCRIPTION:    Accessor functions for the Symmetry Database.
**
** AUTHOR:    Albert Shau
**        GamesCrafters Research Group, UC Berkeley
**        Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:    2007-11-22
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
/* DISCLAIMER:  This is pretty much just a copy and paste of Ken Elkabany's
                Bit Perfect Database
 */


#ifndef GMCORE_SYMDB_H
#define GMCORE_SYMDB_H

#include "bpdb_schemes.h"
#include "bpdb.h"
#include "db.h"

//
// functions for global use
//

GMSTATUS
symdb_init(
        DB_Table *new_db
        );

void
symdb_free( );

GMSTATUS
symdb_free_slice( );

// get/set value
VALUE
symdb_get_value(
        POSITION pos
        );

VALUE
symdb_set_value(
        POSITION pos,
        VALUE val
        );

// get/set remoteness
REMOTENESS
symdb_get_remoteness(
        POSITION pos
        );

void
symdb_set_remoteness(
        POSITION pos,
        REMOTENESS val
        );

// get/set visited
BOOLEAN
symdb_check_visited(
        POSITION pos
        );

void
symdb_mark_visited(
        POSITION pos
        );

void
symdb_unmark_visited(
        POSITION pos
        );

// get/set mex
MEX
symdb_get_mex(
        POSITION pos
        );

void
symdb_set_mex(
        POSITION pos,
        MEX mex);

// get/set winby
WINBY
symdb_get_winby(
        POSITION pos
        );

void
symdb_set_winby(
        POSITION pos,
        WINBY winBy);

//
// functions for internal use
//


UINT64
symdb_get_slice_slot(
        UINT64 position,
        UINT8 index
        );


UINT64
symdb_set_slice_slot(
        UINT64 position,
        UINT8 index,
        UINT64 value
        );


UINT64
symdb_set_slice_slot_max(
        UINT64 position,
        UINT8 index
        );

GMSTATUS
symdb_add_slot(
        UINT8 size,
        char *name,
        BOOLEAN write,
        BOOLEAN adjust,
        BOOLEAN reservemax,
        UINT32 *slotindex
        );

GMSTATUS
symdb_grow_slice(
        BYTE *symdb_array,
        SLICE symdb_slice,
        UINT8 index,
        UINT64 value
        );

GMSTATUS
symdb_shrink_slice(
        BYTE *symdb_array,
        SLICE symdb_slice,
        UINT8 index
        );

GMSTATUS
symdb_allocate(
        );


//
// diagnostic functions
//

void
symdb_print_database( );

void
symdb_dump_database( int num );

void
symdb_analyze_database( int num );

//
//
//

BOOLEAN
symdb_save_database( );

GMSTATUS
symdb_generic_save_database(
        SCHEME scheme,
        char *outfilename
        );

BOOLEAN
symdb_load_database( );

GMSTATUS
symdb_generic_load_database(
        dbFILE inFile,
        SCHEME scheme
        );


BOOLEAN
symdb_generic_write_varnum(
        dbFILE outFile,
        SCHEME scheme,
        BYTE **curBuffer,
        BYTE *outputBuffer,
        UINT32 bufferLength,
        UINT8 *offset,
        UINT64 consecutiveSkips
        );


UINT64
symdb_generic_read_varnum(
        dbFILE inFile,
        SCHEME scheme,
        BYTE **curBuffer,
        BYTE *inputBuffer,
        UINT32 length,
        UINT8 *offset,
        BOOLEAN alreadyReadFirstBit
        );

#endif /* GMCORE_SYMDB_H */
