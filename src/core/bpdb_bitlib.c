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


/*++

   Routine Description:

    bitlib_print_byte_in_bits is a utility function
    used to print out the contents of a byte in bits

   Arguments:

    b - pointer to the byte to print out in bits

   Return value:

    None

   --*/

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


/*++

   Routine Description:

    bitlib_print_bytes_in_bits is a utility function
    used to print out the contents of multiple bytes
    in bits directly to stdout.

   Arguments:

    b - pointer to the byte to print out in bits
    bytes - number of bytes to print (capped at 255)

   Return value:

    None

   --*/

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

/*++

   Routine Description:

    bitlib_file_write_bytes writes the requested number of
    bytes from the buffer into a FILE.

   Arguments:

    dbFile - pointer to the FILE object.
    buffer - pointer to the buffer that contains the data to
            be written.
    length - the number of bytes to be written; the buffer MUST
            have at least length bytes available.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bitlib_file_write_bytes(
        dbFILE file,
        BYTE *buffer,
        UINT32 length
        )
{
	GMSTATUS status = STATUS_SUCCESS;

	if(gzwrite(file, buffer, length) != length) {
		status = STATUS_BAD_COMPRESSION;
		BPDB_TRACE("bitlib_file_write_bytes()", "call to gzwrite returned an improper value", status);
	}

	return status;
}


/*++

   Routine Description:

    bitlib_file_read_bytes reads the requested number of
    bytes from a FILE pointer into a passed in buffer.

   Arguments:

    dbFile - pointer to the FILE object.
    buffer - pointer to the buffer that data should be
            read into.
    length - the number of bytes to be read; the buffer MUST
            have at least length bytes available.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bitlib_file_read_bytes(
        dbFILE file,
        BYTE *buffer,
        UINT32 length
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	int ret = 0;

	if((ret = gzread(file, buffer, length)) <= 0) {
		status = STATUS_BAD_DECOMPRESSION;
		// Commented out because, when using level files, this isn't always an "error"...
		//BPDB_TRACE("bitlib_file_read_bytes()", "call to gzread returned a failed value", status);
		//printf("Gzip error: %s\n", gzerror(file, &ret));
	}

	return status;
}

/*++

   Routine Description:

    bitlib_file_open opens a given file.

   Arguments:

    filename - name of file
    mode - mode to open file; for example, "wb" is to write
        and "rb" is to read.
    file - handle to dbFILE; input a null dbFILE that will
        be assigned dbFILE at the completion of the function.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bitlib_file_open(
        char *filename,
        char *mode,
        dbFILE *db
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


/*++

   Routine Description:

    bitlib_file_seek seeks to a byte within a given file
    from the beginning of the file.

   Arguments:

    file - pointer to a dbFILE.
    byteIndex - byte to be seeked to.
    whence - can be SEEK_SET (start of file), or SEEK_CUR
            (current position of file pointer)

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/

GMSTATUS
bitlib_file_seek(
        dbFILE db,
        UINT32 byteIndex,
        int whence
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	int ret;

	if((ret = gzseek(db, byteIndex, whence)) < 0) {
		status = STATUS_FILE_COULD_NOT_BE_SEEKED;
		BPDB_TRACE("bitlib_file_open()", "gzopen failed to seek in file", status);
		printf("Gzip error: %s\n", gzerror(db, &ret));
		goto _bailout;
	}

_bailout:
	return status;
}

/*++

   Routine Description:

    bitlib_file_close closes a given file.

   Arguments:

    file - pointer to the file object to be closed.

   Return value:

    STATUS_SUCCESS on successful execution, or neccessary
    error on failure.

   --*/
GMSTATUS
bitlib_file_close(
        dbFILE file
        )
{
	GMSTATUS status = STATUS_SUCCESS;
	int ret;

	if((ret = gzclose(file)) != 0) {
		status = STATUS_FILE_COULD_NOT_BE_CLOSED;
		BPDB_TRACE("bitlib_file_close()", "gzclose failed to close file", status);
		printf("Gzip error: %s\n", gzerror(file, &ret));
		goto _bailout;
	}

_bailout:
	return status;
}


/*++

   Routine Description:

    bitlib_insert_bits writes a variable length set of bits
    into a byte array beginning at an arbitrary bit offset.

   Arguments:

    slice - byte array to write the set of bits into.
    offsetFromLeft - offset from the left(most significant bit)
        of the slice byte indicating where to start writing
        the set of bits.
    value - contains the sequential set of bits to write.
    bitsToOutput - the number of bits from value to output,
        starting from the least significant bit(0).

   Return value:

    None.

   --*/

void
bitlib_insert_bits(
        BYTE *slice,
        UINT8 offsetFromLeft,
        UINT64 value,
        UINT8 bitsToOutput
        )
{
	UINT8 offsetFromRight = BITSINBYTE - offsetFromLeft;
	BYTE mask = 0;

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


/*++

   Routine Description:

    bitlib_get_bits_range returns the desired subset of the
    value input. The desired subset is defined as the subset
    of input value. The subset is of size length bits. The
    range of the subset is defined by [most significant bit:
    least significant bit] = [length+offsetFromRight:
    offsetFromRight].

   Arguments:

    value - data to retrieve bit subsets from
    offsetFromRight - offset from the least significant bit
        of value to create subset from.
    length - number of bits from the offset to create new
        subset from.

   Return value:

    Desired subset of value.

   --*/

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


/*++

   Routine Description:

    bitlib_right_mask8 returns a bit level mask.
    For example, bitlib_right_mask8(5) returns
    00011111.

   Arguments:

    maskbits - number of bits to mask

   Return value:

    8-bit Mask

   --*/

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


/*++

   Routine Description:

    bitlib_right_mask64 returns a bit level mask.
    It is a 64-bit version of bitlib_right_mask8.

   Arguments:

    maskbits - number of bits to mask

   Return value:

    64-bit Mask

   --*/

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


/*++

   Routine Description:

    bitlib_read_bits returns a subset of bits from
    a byte array.

   Arguments:

    slice - byte array to read bits from.
    offsetFromLeft - offset from the most significant bit
        of the byte to begin reading from.
    bitsToOutput - number of bits to read

   Return value:

    Subset of byte array that was read

   --*/

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
	BYTE slicecopy = 0;

	while( bitsToOutput > 0 ) {
		slicecopy = *slice;

		if(  (BITSINBYTE - offsetFromLeft - bitsToOutput) > 0 ) {
			mask = bitlib_right_mask8( MIN(bitsToOutput, offsetFromRight) ) << (offsetFromRight - bitsToOutput);
			slicecopy = (slicecopy & mask) >> (offsetFromRight - bitsToOutput);
		} else {
			mask = bitlib_right_mask8( MIN(bitsToOutput, offsetFromRight) );
			slicecopy = (slicecopy & mask);
		}

		value = value << MIN(bitsToOutput, offsetFromRight);
		value = value | slicecopy;

		offsetFromLeft = (offsetFromLeft + MIN(bitsToOutput, offsetFromRight)) % BITSINBYTE;
		bitsToOutput -= MIN(bitsToOutput, offsetFromRight);
		offsetFromRight = BITSINBYTE - offsetFromLeft;

		slice++;
	}

	return value;
}


void
bitlib_value_to_buffer(
        dbFILE file,
        BYTE **curBuffer,
        BYTE *outputBuffer,
        UINT32 bufferLength,
        UINT8 *offsetFromLeft,
        UINT64 value,
        UINT8 bitsToOutput
        )
{
	UINT8 offsetFromRight = BITSINBYTE - *offsetFromLeft;

	while( bitsToOutput > 0 ) {
		if(offsetFromRight >= bitsToOutput) {
			**curBuffer = **curBuffer | (bitlib_get_bits_range( value, bitsToOutput, MIN(bitsToOutput, offsetFromRight) ) << (offsetFromRight - bitsToOutput));
		} else {
			**curBuffer = **curBuffer | bitlib_get_bits_range( value, bitsToOutput, MIN(bitsToOutput, offsetFromRight) );
		}

		*offsetFromLeft = (*offsetFromLeft + MIN(bitsToOutput, offsetFromRight)) % BITSINBYTE;
		bitsToOutput -= MIN(bitsToOutput, offsetFromRight);
		offsetFromRight = BITSINBYTE - *offsetFromLeft;

		if(*offsetFromLeft == 0) {
			(*curBuffer)++;
			if(*curBuffer == outputBuffer + bufferLength) {
				bitlib_file_write_bytes(file, outputBuffer, bufferLength);
				*curBuffer = outputBuffer;
				memset(outputBuffer, 0, bufferLength);
			}
		}
	}
}

UINT64
bitlib_read_from_buffer(
        dbFILE inFile,
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
			(*curBuffer)++;
			*offsetFromLeft = 0;
			offsetFromRight = BITSINBYTE;
			if(*curBuffer == inputBuffer + bufferLength) {
				bitlib_file_read_bytes(inFile, inputBuffer, bufferLength);
				*curBuffer = inputBuffer;
			}
		}

		value = value << MIN( length, offsetFromRight );

		if(length > offsetFromRight) {
			mask = bitlib_right_mask8( MIN(length, offsetFromRight) );
			value = value | (UINT64) (**curBuffer & mask);
		} else {
			mask = bitlib_right_mask8( MIN(length, offsetFromRight) );
			value = value | (UINT64) (((mask << (offsetFromRight - length)) & **curBuffer) >> (BITSINBYTE - *offsetFromLeft - length));
		}

		*offsetFromLeft += MIN( length, offsetFromRight );
		length -= MIN( length, offsetFromRight );
		offsetFromRight = BITSINBYTE - *offsetFromLeft;
	}

	return value;
}
