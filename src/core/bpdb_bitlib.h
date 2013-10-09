/************************************************************************
**
** NAME:    bpdb_bitlib.h
**
** DESCRIPTION:    Accessor functions for the Bit-Manipulation Library
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

#ifndef GMCORE_BITLIB_H
#define GMCORE_BITLIB_H

#include "bpdb_misc.h"

//
// for the record
// bitlib_insert_bits and
// bitlib_read_bits support numbers up to and including 64 bits
// the limiter being UINT64
//

GMSTATUS
bitlib_file_open(
        char *filename,
        char *mode,
        dbFILE *db
        );

GMSTATUS
bitlib_file_close(
        dbFILE file
        );

GMSTATUS
bitlib_file_seek(
        dbFILE db,
        UINT32 byteIndex,
        int whence
        );


GMSTATUS
bitlib_file_write_bytes(
        dbFILE file,
        BYTE *buffer,
        UINT32 length
        );


GMSTATUS
bitlib_file_read_bytes(
        dbFILE file,
        BYTE *buffer,
        UINT32 length
        );


BYTE
bitlib_right_mask8(
        UINT8 maskbits
        );


UINT64
bitlib_right_mask64(
        UINT8 maskbits
        );


void
bitlib_insert_bits(
        BYTE *slice,
        UINT8 bitOffset,
        UINT64 value,
        UINT8 bitsToOutput
        );


BYTE
bitlib_get_bits_range(
        UINT64 value,
        UINT8 offsetFromRight,
        UINT8 length
        );


UINT64
bitlib_read_bits(
        BYTE *slice,
        UINT8 offsetFromLeft,
        UINT8 bitsToOutput
        );

void
bitlib_print_byte_in_bits(
        BYTE *b
        );

void
bitlib_print_bytes_in_bits(
        BYTE *b,
        UINT8 bytes
        );


void
bitlib_value_to_buffer(
        dbFILE file,
        BYTE **curBuffer,
        BYTE *outputBuffer,
        UINT32 bufferLength,
        UINT8 *offsetFromLeft,
        UINT64 value,
        UINT8 bitsToOutput
        );


UINT64
bitlib_read_from_buffer(
        dbFILE inFile,
        BYTE **curBuffer,
        BYTE *inputBuffer,
        UINT32 bufferLength,
        UINT8 *offsetFromLeft,
        UINT8 length
        );

#endif /* GMCORE_BITLIB_H */
