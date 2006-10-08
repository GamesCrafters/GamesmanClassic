#ifndef GMCORE_BPDB_H
#define GMCORE_BPDB_H

#include "bpdb_misc.h"

//
// slice format
//

typedef struct sliceformat {
    UINT8 *size;
    UINT32 *offset;
    UINT64 *maxvalue;
    UINT64 *maxseen;
    char **name;
    UINT8 slots;
    UINT32 bits;
    BOOLEAN *overflowed;
    BOOLEAN *adjust;
} *SLICE;


//
// functions for global use
//

void
bpdb_init(
                DB_Table *new_db
                );

void
bpdb_free( );

void
bpdb_free_slice( );

// get/set value
VALUE
bpdb_get_value(
                POSITION pos
                );

VALUE
bpdb_set_value(
                POSITION pos,
                VALUE val
                );

// get/set remoteness
REMOTENESS
bpdb_get_remoteness(
                POSITION pos
                );

void
bpdb_set_remoteness(
                POSITION pos,
                REMOTENESS val
                );

// get/set visited
BOOLEAN
bpdb_check_visited(
                POSITION pos
                );

void
bpdb_mark_visited(
                POSITION pos
                );

void
bpdb_unmark_visited(
                POSITION pos
                );

// get/set mex
MEX
bpdb_get_mex(
                POSITION pos
                );

void
bpdb_set_mex(
                POSITION pos,
                MEX mex);



//
// functions for internal use
//

inline
UINT64
bpdb_get_slice_slot(
                UINT64 position,
                UINT8 index
                );

inline
UINT64
bpdb_set_slice_slot(
                UINT64 position,
                UINT8 index,
                UINT64 value
                );

GMSTATUS
bpdb_add_slot(
                UINT8 size,
                char *name,
                BOOLEAN write,
                BOOLEAN adjust,
                UINT32 *slotindex
                );

GMSTATUS
bpdb_grow_slice(
                BYTE *bpdb_array,
                SLICE bpdb_slice,
                UINT8 index,
                UINT64 value
                );

GMSTATUS
bpdb_shrink_slice(
                BYTE *bpdb_array,
                SLICE bpdb_slice,
                UINT8 index
                );

GMSTATUS
bpdb_allocate(
        );


//
// diagnostic functions
//

void
bpdb_print_database( );

void
bpdb_dump_database( int num );

//
//
//

BOOLEAN
bpdb_save_database( );

GMSTATUS
bpdb_generic_save_database(
                SCHEME scheme,
                char *outfilename
                );

BOOLEAN
bpdb_load_database( );

GMSTATUS
bpdb_generic_load_database(
                dbFILE *inFile,
                SCHEME scheme
                );


BOOLEAN
bpdb_generic_write_varnum(
                dbFILE *outFile,
                SCHEME scheme,
                BYTE **curBuffer,
                BYTE *outputBuffer,
                UINT32 bufferLength,
                UINT8 *offset,
                UINT64 consecutiveSkips
                );

UINT8
bpdb_generic_varnum_gap_bits(
                UINT64 consecutiveSkips
                );

UINT64
bpdb_generic_varnum_implicit_amt(
                UINT8 leftBits
                );

UINT8
bpdb_generic_varnum_size_bits(
                UINT8 leftBits
                );

UINT64
bpdb_generic_read_varnum(
                dbFILE *inFile,
                SCHEME scheme,
                BYTE **curBuffer,
                BYTE *inputBuffer,
                UINT32 length,
                UINT8 *offset,
                BOOLEAN alreadyReadFirstBit
                );
/*
UINT8
bpdb_generic_read_varnum_consecutive_ones(
                dbFILE *inFile,
                BYTE **curBuffer,
                BYTE *inputBuffer,
                UINT32 length,
                UINT8 *offset,
                BOOLEAN alreadyReadFirstBit
                );
*/
#endif /* GMCORE_BPDB_H */
