#ifndef GMCORE_BITLIB_H
#define GMCORE_BITLIB_H

#include "bpdb_misc.h"

dbFILE		*bitlib_file_open				( char *filename, char *mode );
BOOLEAN		bitlib_file_close				( dbFILE *file );
BOOLEAN		bitlib_file_write_byte			( dbFILE *file, BYTE *buffer );
BYTE		bitlib_file_read_byte			( dbFILE *file );

BYTE		bitlib_right_mask8				( UINT8 maskbits );
UINT64		bitlib_right_mask64				( UINT8 maskbits );

void		bitlib_insert_bits				( BYTE *slice, UINT8 bitOffset, UINT64 value, UINT8 bitsToOutput );
BYTE		bitlib_get_bits_range			( UINT64 value, UINT8 offsetFromRight, UINT8 length );
UINT64		bitlib_read_bits				( BYTE *slice, UINT8 offsetFromLeft, UINT8 bitsToOutput );

void		bitlib_print_byte_in_bits		( BYTE *b );
void		bitlib_print_bytes_in_bits		( BYTE *b, UINT8 bytes );

void		bitlib_value_to_buffer			( dbFILE *file, BYTE *outputBuffer, UINT8 *offsetFromLeft, UINT64 value, UINT8 bitsToOutput );
UINT64		bitlib_read_from_buffer			( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offsetFromLeft, UINT8 length );

UINT8		min8							(UINT8 a, UINT8 b);

#endif /* GMCORE_BITLIB_H */
