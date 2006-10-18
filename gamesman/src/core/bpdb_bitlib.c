/************************************************************************
**
** NAME:    bpdb_bitlib.c
**
** DESCRIPTION:    Implementation of the Bit-Manipulation Library
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

#include "gamesman.h"
#include "bpdb_bitlib.h"

void
bitlib_print_byte_in_bits(
                BYTE *b
                )
{
    UINT8 bits = BITSINBYTE;

    while( bits > 0 ) {
        printf("%d", (*b >> (bits - 1)) & 1);
        bits--;
    }
    printf("\n");
}

void
bitlib_print_bytes_in_bits(
                BYTE *b,
                UINT8 bytes
                )
{
    UINT8 offset = 0;

    while(bytes > 0) {
        bitlib_print_byte_in_bits( b + offset );
        offset++;
        bytes--;
    }
}

inline
BOOLEAN
bitlib_file_write_byte(
                dbFILE *file,
                BYTE *buffer,
                UINT32 length
                )
{
    /*if(fwrite(buffer, 1, 1, file) == 1) {
        return TRUE;
    } else {
        return FALSE;
    }*/
    int ret = 0;
    ret = gzwrite(file, buffer, length);
    if(ret != length) {
        printf("Most likely bad compression! %d\n", ret);
        //exit(0);
        return FALSE;
    }
    return TRUE;
}

inline
BOOLEAN
bitlib_file_read_byte(
                dbFILE *file,
                BYTE *buffer,
                UINT32 length
                )
{
    int ret;

    // length in bytes (length * sizeof(BYTE))
    ret = gzread(file, buffer, length);
    //readbyte = (BYTE) fgetc( file );

    if(ret <= 0) {
        printf("Warning: Most likely bad decompression.");
        //exit(0);
        return FALSE;
    }
    return TRUE;
}

GMSTATUS
bitlib_file_open(
                char *filename,
                char *mode,
                dbFILE **db
                )
{
    GMSTATUS status = STATUS_SUCCESS;

    *db = gzopen(filename, mode);
    if(NULL == *db) {
        status = STATUS_FILE_COULD_NOT_BE_OPENED;
        BPDB_TRACE("bitlib_file_open()", "gzopen failed to open file", status);
        goto _bailout;
    }

_bailout:
    return status;
}

GMSTATUS
bitlib_file_close(
                dbFILE *file
                )
{
    GMSTATUS status = STATUS_SUCCESS;

    if(gzclose(file) != 0) {
        status = STATUS_FILE_COULD_NOT_BE_CLOSED;
        BPDB_TRACE("bitlib_file_close()", "gzclose failed to close file", status);
        goto _bailout;
    }

_bailout:
    return status;
}

/*
 * offsetFromLeft = offset from the left indicating start of desired slice
 * bitsToOutput = Number of bits to output from value
 */
inline
void
bitlib_insert_bits(
                BYTE *slice,
                UINT8 offsetFromLeft,
                UINT64 value,
                UINT8 bitsToOutput
                )
{
    //assert( offsetFromLeft < 64 );

    // offsetfromright
    UINT8 offsetFromRight = BITSINBYTE - offsetFromLeft;
    BYTE mask = 0;

    //printf("value %llu, bitstooutput %d\n", value, bitsToOutput);

    while( bitsToOutput > 0 ) {
        mask = bitlib_right_mask8( offsetFromLeft ) << offsetFromRight;

        if( (BITSINBYTE - offsetFromLeft - bitsToOutput) > 0 ) {
            mask = mask | bitlib_right_mask8( BITSINBYTE - offsetFromLeft - bitsToOutput );
        }

        *slice = *slice & mask;

        *slice = *slice | (bitlib_get_bits_range( value, bitsToOutput, MIN(bitsToOutput, offsetFromRight) ) << (offsetFromRight - MIN(bitsToOutput, offsetFromRight)));
        
        offsetFromLeft = (offsetFromLeft + MIN(bitsToOutput, offsetFromRight)) % BITSINBYTE;
        bitsToOutput -= MIN(bitsToOutput, offsetFromRight);
        offsetFromRight = BITSINBYTE - offsetFromLeft;
        slice++;
    }
}

inline
BYTE
bitlib_get_bits_range(
                UINT64 value,
                UINT8 offsetFromRight,
                UINT8 length
                )
{
    BYTE bitsinrange = 0;

    value = value << (BITSINPOS - offsetFromRight);
    value = value >> (BITSINPOS - length);
    bitsinrange = (BYTE) value;

    return bitsinrange;
}

// Supports masking between 0 and 8 bits
inline
BYTE
bitlib_right_mask8(
                UINT8 maskbits
                )
{
    BYTE mask = 0;

    while( maskbits > 0 ) {
        mask = mask << 1;
        mask = mask | 1;
        maskbits--;
    }

    return mask;
}

// Supports masking between 0 and 64 bits
inline
UINT64
bitlib_right_mask64(
                UINT8 maskbits
                )
{
    UINT64 mask = 0;

    while( maskbits > 0 ) {
        mask = mask << 1;
        mask = mask | 1;
        maskbits--;
    }

    return mask;
}


inline
UINT64
bitlib_read_bits(
                BYTE *slice,
                UINT8 offsetFromLeft,
                UINT8 bitsToOutput
)
{
    // offsetfromright
    UINT8 offsetFromRight = BITSINBYTE - offsetFromLeft;
    UINT64 value = 0;
    BYTE mask = 0;

    BYTE slicecopy;
    slicecopy = *slice;

    while( bitsToOutput > 0 ) {
        //if(offsetFromRight >= bitsToOutput) {
        if(  (BITSINBYTE - offsetFromLeft - bitsToOutput) > 0 ) {
            mask = bitlib_right_mask8( MIN(bitsToOutput, offsetFromRight) ) << (offsetFromRight - bitsToOutput);
            slicecopy = (slicecopy & mask) >> (offsetFromRight - bitsToOutput);
        } else {
            mask = bitlib_right_mask8( MIN(bitsToOutput, offsetFromRight) );
            slicecopy = (slicecopy & mask);
        }

        value = value << MIN(bitsToOutput, offsetFromRight);
        value = value | slicecopy;
        
        //printf("slice %u OL: %u, OR: %u, BTO: %u, value %llu\n", *slice, offsetFromLeft, offsetFromRight, bitsToOutput, value);

        offsetFromLeft = (offsetFromLeft + MIN(bitsToOutput, offsetFromRight)) % BITSINBYTE;
        bitsToOutput -= MIN(bitsToOutput, offsetFromRight);
        offsetFromRight = BITSINBYTE - offsetFromLeft;

        slice++;
        slicecopy =  *slice;
    }

    return value;
}


inline
void
bitlib_value_to_buffer(
                dbFILE *file,
                BYTE **curBuffer,
                BYTE *outputBuffer,
                UINT32 bufferLength,
                UINT8 *offsetFromLeft,
                UINT64 value,
                UINT8 bitsToOutput
                )
{
    // offsetfromright
    //BOOLEAN writeOut = FALSE;
    UINT8 offsetFromRight = BITSINBYTE - *offsetFromLeft;

    while( bitsToOutput > 0 ) {
        if(offsetFromRight >= bitsToOutput) {
            **curBuffer = **curBuffer | (bitlib_get_bits_range( value, bitsToOutput, MIN(bitsToOutput, offsetFromRight) ) << (offsetFromRight - bitsToOutput));
        } else {
            **curBuffer = **curBuffer | bitlib_get_bits_range( value, bitsToOutput, MIN(bitsToOutput, offsetFromRight) );
        }
        
        *offsetFromLeft = (*offsetFromLeft + MIN(bitsToOutput, offsetFromRight)) % BITSINBYTE;
        //if(*offsetFromLeft == 8 ) {
        //    writeOut = TRUE;
        //    *offsetFromLeft = *offsetFromLeft % BITSINBYTE;
        //}
        bitsToOutput -= MIN(bitsToOutput, offsetFromRight);
        offsetFromRight = BITSINBYTE - *offsetFromLeft;

        //if(writeOut) {
        if(*offsetFromLeft == 0) {
            (*curBuffer)++;
            if(*curBuffer == outputBuffer + bufferLength) {
                bitlib_file_write_byte(file, outputBuffer, bufferLength);
                *curBuffer = outputBuffer;
                memset(outputBuffer, 0, bufferLength);
                // maybe memset to 0
            }
            
            //*outputBuffer = 0;
        }
    }
}

inline
UINT64
bitlib_read_from_buffer(
                dbFILE *inFile,
                BYTE **curBuffer,
                BYTE *inputBuffer,
                UINT32 bufferLength,
                UINT8 *offsetFromLeft,
                UINT8 length
                )
{
    UINT64 value = 0;
    UINT8 offsetFromRight = BITSINBYTE - *offsetFromLeft;
    UINT8 mask = 0;

    while( length > 0 ) {
        if(*offsetFromLeft == 8 ) {
            //*inputBuffer = bitlib_file_read_byte(inFile);
            //*offsetFromLeft -= 8;
            //offsetFromRight = BITSINBYTE - *offsetFromLeft;

            (*curBuffer)++;
            *offsetFromLeft = 0;
            offsetFromRight = BITSINBYTE;
            if(*curBuffer == inputBuffer + bufferLength) {
                bitlib_file_read_byte(inFile, inputBuffer, bufferLength);
                *curBuffer = inputBuffer;
                //memset(outputBuffer, 0, bufferLength);
                // maybe memset to 0
            }
        }

        value = value << MIN( length, offsetFromRight );

        //printf("\n");
        //bitlib_print_byte_in_bits(inputBuffer);
        
        if(length > offsetFromRight) {
            mask = bitlib_right_mask8( MIN(length, offsetFromRight) );
            value = value | (UINT64) (**curBuffer & mask);
        } else {
            mask = bitlib_right_mask8( MIN(length, offsetFromRight) );
            value = value | (UINT64) (((mask << (offsetFromRight - length)) & **curBuffer) >> (BITSINBYTE - *offsetFromLeft - length));
        }

        //printf("OFL: %d, OFR: %d, length: %d, value: %llu\n", *offsetFromLeft, offsetFromRight, length, value);

        //printf("value: %d\n", value);

        *offsetFromLeft += MIN( length, offsetFromRight );
        length -= MIN( length, offsetFromRight );
        offsetFromRight = BITSINBYTE - *offsetFromLeft;
    }

    return value;
}
