#include "bpdb_bitlib.h"

UINT8 min8(UINT8 a, UINT8 b) {
	if(a < b) {
		return a;
	} else {
		return b;
	}
	return a;
}

void bitlib_print_byte_in_bits( BYTE *b ) {
	UINT8 bits = BITSINBYTE;

	while( bits > 0 ) {
		printf("%d", (*b >> (bits - 1)) & 1);
		bits--;
	}
	printf("\n");
}

void bitlib_print_bytes_in_bits( BYTE *b, UINT8 bytes ) {
	UINT8 offset = 0;

	while(bytes > 0) {
		bitlib_print_byte_in_bits( b + offset );
		offset++;
		bytes--;
	}
}

BOOLEAN bitlib_file_write_byte( dbFILE *file, BYTE *buffer ) {
	/*if(fwrite(buffer, 1, 1, file) == 1) {
		return TRUE;
	} else {
		return FALSE;
	}*/
	int ret = 0;
	ret = gzwrite(file, buffer, 1);
	if(ret != 1) {
		printf("Most likely bad compression! %d\n", ret);
		//exit(0);
		return FALSE;
	}
	return TRUE;
}

BYTE bitlib_file_read_byte( dbFILE *file ) {
	BYTE readbyte;
	int ret;

	ret = gzread(file, &readbyte,sizeof(BYTE));
	//readbyte = (BYTE) fgetc( file );

	if(ret <= 0) {
		printf("Warning: Most likely bad decompression.");
		exit(0);
	}
	return readbyte;
}

dbFILE *bitlib_file_open( char *filename, char *mode ) {
	//return fopen(filename, mode);
	return gzopen(filename, mode);
}

BOOLEAN bitlib_file_close( dbFILE *file ) {
	//if(fclose(file) == 0) {
	if(gzclose(file) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*
 * offsetFromLeft = offset from the left indicating start of desired slice
 * bitsToOutput = Number of bits to output from value
 */
void bitlib_insert_bits( BYTE *slice, UINT8 offsetFromLeft, UINT64 value, UINT8 bitsToOutput ) {
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

		*slice = *slice | (bitlib_get_bits_range( value, bitsToOutput, min8(bitsToOutput, offsetFromRight) ) << (offsetFromRight - min8(bitsToOutput, offsetFromRight)));
		
		offsetFromLeft = (offsetFromLeft + min8(bitsToOutput, offsetFromRight)) % BITSINBYTE;
		bitsToOutput -= min8(bitsToOutput, offsetFromRight);
		offsetFromRight = BITSINBYTE - offsetFromLeft;
		slice++;
	}
}

BYTE bitlib_get_bits_range( UINT64 value, UINT8 offsetFromRight, UINT8 length ) {
	BYTE bitsinrange = 0;

	value = value << (BITSINPOS - offsetFromRight);
	value = value >> (BITSINPOS - length);
	bitsinrange = (BYTE) value;

	return bitsinrange;
}

// Supports masking between 0 and 8 bits
BYTE bitlib_right_mask8( UINT8 maskbits ) {
	BYTE mask = 0;

	while( maskbits > 0 ) {
		mask = mask << 1;
		mask = mask | 1;
		maskbits--;
	}

	return mask;
}

// Supports masking between 0 and 64 bits
UINT64 bitlib_right_mask64( UINT8 maskbits ) {
	UINT64 mask = 0;

	while( maskbits > 0 ) {
		mask = mask << 1;
		mask = mask | 1;
		maskbits--;
	}

	return mask;
}


UINT64 bitlib_read_bits( BYTE *slice, UINT8 offsetFromLeft, UINT8 bitsToOutput ) {
	// offsetfromright
	UINT8 offsetFromRight = BITSINBYTE - offsetFromLeft;
	UINT64 value = 0;
	BYTE mask = 0;

	BYTE slicecopy;
	slicecopy = *slice;

	while( bitsToOutput > 0 ) {
		//if(offsetFromRight >= bitsToOutput) {
		if(  (BITSINBYTE - offsetFromLeft - bitsToOutput) > 0 ) {
			mask = bitlib_right_mask8( min8(bitsToOutput, offsetFromRight) ) << (offsetFromRight - bitsToOutput);
			slicecopy = (slicecopy & mask) >> (offsetFromRight - bitsToOutput);
		} else {
			mask = bitlib_right_mask8( min8(bitsToOutput, offsetFromRight) );
			slicecopy = (slicecopy & mask);
		}

		value = value << min8(bitsToOutput, offsetFromRight);
		value = value | slicecopy;
		
		//printf("slice %u OL: %u, OR: %u, BTO: %u, value %llu\n", *slice, offsetFromLeft, offsetFromRight, bitsToOutput, value);

		offsetFromLeft = (offsetFromLeft + min8(bitsToOutput, offsetFromRight)) % BITSINBYTE;
		bitsToOutput -= min8(bitsToOutput, offsetFromRight);
		offsetFromRight = BITSINBYTE - offsetFromLeft;

		slice++;
		slicecopy =  *slice;
	}

	return value;
}


void bitlib_value_to_buffer( dbFILE *file, BYTE *outputBuffer, UINT8 *offsetFromLeft, UINT64 value, UINT8 bitsToOutput ) {
	// offsetfromright
	//BOOLEAN writeOut = FALSE;
	UINT8 offsetFromRight = BITSINBYTE - *offsetFromLeft;

	while( bitsToOutput > 0 ) {
		if(offsetFromRight >= bitsToOutput) {
			*outputBuffer = *outputBuffer | (bitlib_get_bits_range( value, bitsToOutput, min8(bitsToOutput, offsetFromRight) ) << (offsetFromRight - bitsToOutput));
		} else {
			*outputBuffer = *outputBuffer | bitlib_get_bits_range( value, bitsToOutput, min8(bitsToOutput, offsetFromRight) );
		}
		
		*offsetFromLeft = (*offsetFromLeft + min8(bitsToOutput, offsetFromRight)) % BITSINBYTE;
		//if(*offsetFromLeft == 8 ) {
		//	writeOut = TRUE;
		//	*offsetFromLeft = *offsetFromLeft % BITSINBYTE;
		//}
		bitsToOutput -= min8(bitsToOutput, offsetFromRight);
		offsetFromRight = BITSINBYTE - *offsetFromLeft;

		//if(writeOut) {
		if(*offsetFromLeft == 0) {
			bitlib_file_write_byte(file, outputBuffer);
			*outputBuffer = 0;
		}
	}
}

UINT64 bitlib_read_from_buffer( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offsetFromLeft, UINT8 length ) {
	UINT64 value = 0;
	UINT8 offsetFromRight = BITSINBYTE - *offsetFromLeft;
	UINT8 mask = 0;

	while( length > 0 ) {
		if(*offsetFromLeft == 8 ) {
			*inputBuffer = bitlib_file_read_byte(inFile);
			*offsetFromLeft -= 8;
			offsetFromRight = BITSINBYTE - *offsetFromLeft;
		}

		value = value << min8( length, offsetFromRight );

		//printf("\n");
		//bitlib_print_byte_in_bits(inputBuffer);
		
		if(length > offsetFromRight) {
			mask = bitlib_right_mask8( min8(length, offsetFromRight) );
			value = value | (UINT64) (*inputBuffer & mask);
		} else {
			mask = bitlib_right_mask8( min8(length, offsetFromRight) );
			value = value | (UINT64) (((mask << (offsetFromRight - length)) & *inputBuffer) >> (BITSINBYTE - *offsetFromLeft - length));
		}

		//printf("OFL: %d, OFR: %d, length: %d, value: %llu\n", *offsetFromLeft, offsetFromRight, length, value);

		//printf("value: %d\n", value);

		*offsetFromLeft += min8( length, offsetFromRight );
		length -= min8( length, offsetFromRight );
		offsetFromRight = BITSINBYTE - *offsetFromLeft;
	}

	return value;
}
