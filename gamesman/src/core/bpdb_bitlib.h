#ifndef GMCORE_BITLIB_H
#define GMCORE_BITLIB_H

#include "bpdb_misc.h"

dbFILE		*bitlib_file_open			( char *filename, char *mode );
BOOLEAN		bitlib_file_close			( dbFILE *file );
inline BOOLEAN		bitlib_file_write_byte			( dbFILE *file, BYTE *buffer );
inline BYTE		bitlib_file_read_byte			( dbFILE *file );

inline BYTE		bitlib_right_mask8			( UINT8 maskbits );
inline UINT64		bitlib_right_mask64			( UINT8 maskbits );

inline void		bitlib_insert_bits			( BYTE *slice, UINT8 bitOffset, UINT64 value, UINT8 bitsToOutput );
inline BYTE		bitlib_get_bits_range			( UINT64 value, UINT8 offsetFromRight, UINT8 length );
inline UINT64		bitlib_read_bits			( BYTE *slice, UINT8 offsetFromLeft, UINT8 bitsToOutput );

void		bitlib_print_byte_in_bits		( BYTE *b );
void		bitlib_print_bytes_in_bits		( BYTE *b, UINT8 bytes );

inline void		bitlib_value_to_buffer			( dbFILE *file, BYTE *outputBuffer, UINT8 *offsetFromLeft, UINT64 value, UINT8 bitsToOutput );
inline UINT64		bitlib_read_from_buffer			( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offsetFromLeft, UINT8 length );

inline UINT8		min8					( UINT8 a, UINT8 b );

#endif /* GMCORE_BITLIB_H */
