#ifndef GMCORE_BPDB_H
#define GMCORE_BPDB_H

#include "bpdb_misc.h"

//void		bpdb_init								( UINT64 slices, UINT64 bits_per_slice );
void		bpdb_init								( DB_Table *new_db );
void       	bpdb_free								( );

/* Value */
VALUE		bpdb_get_value								(POSITION pos);
VALUE		bpdb_set_value								(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	bpdb_get_remoteness							(POSITION pos);
void		bpdb_set_remoteness							(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		bpdb_check_visited							(POSITION pos);
void		bpdb_mark_visited							(POSITION pos);
void		bpdb_unmark_visited							(POSITION pos);

/* Mex */
MEX		bpdb_get_mex								(POSITION pos);
void		bpdb_set_mex								(POSITION pos, MEX mex);

//extern		UINT8	bpdb_bits_per_slice;

//
// functions for internal use
//

UINT64		bpdb_get_slice_slot							( UINT64 position, UINT8 index );
UINT64		bpdb_get_slice_full							( UINT64 position );
void		bpdb_set_slice_slot							( UINT64 position, UINT8 index, UINT64 value );
void		bpdb_set_slice_full							( UINT64 position, UINT64 value );
GMSTATUS	bpdb_add_slot								( UINT8 size, char *name, BOOLEAN write );

void		bpdb_print_database							( );

BOOLEAN		bpdb_save_database							( );
BOOLEAN		bpdb_generic_save_database						( Scheme_List scheme, char *outfilename );
BOOLEAN		bpdb_load_database							( );
BOOLEAN		bpdb_generic_load_database						( dbFILE *inFile, Scheme_List scheme );

BOOLEAN		bpdb_generic_varnum							( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips );
UINT8		bpdb_generic_varnum_gap_bits						( UINT64 consecutiveSkips );
UINT64		bpdb_generic_varnum_implicit_amt					( UINT8 leftBits );

UINT64		bpdb_generic_read_varnum						( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );
UINT8		bpdb_generic_read_varnum_consecutive_ones				( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );

#endif /* GMCORE_BPDB_H */
