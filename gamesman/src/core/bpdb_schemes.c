/*
	To-dos:
	*1. Rename functions to bpdb_
	2. Comment more code
	3. Add feature to force gamesman to use one encoding scheme

	START A THREAD ON THE NEWSGROUP
*/

/************************************************************************
**
** NAME:	bpdb_schemes.c
**
** DESCRIPTION:	Contains all the different schemes, under the format
** bpdb_[schemename]_... that specify the different ways the bpdb can
** compress a file.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
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

#include <zlib.h>
#include <netinet/in.h>
#include "gamesman.h"
#include "bpdb.h"
#include "bpdb_schemes.h"

/*
** Code
*/

/***********
************
**
**	Function Headers for Encoding and Decoding
**
**	generic functions for all schemes					
*/

// Functions for encoding
void bpdb_global_add_to_buffer( gzFile *outFile, BYTE *outputbuffer, int *offset, BYTE r, int length );
int bpdb_global_make_right_mask( int n );

// Functions for decoding
int bpdb_global_find_consecutive_ones( gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit );
int bpdb_global_read_next_byte( gzFile *inFile, BYTE *inputbuffer, int offset );
int bpdb_global_read_next_bits( gzFile *inFile, BYTE *inputbuffer, int *offset, int length );
int bpdb_global_read_next_bit( gzFile *inFile, BYTE *inputbuffer, int *offset );




/***********
************
**
**	Function Implementations for Encoding
**
**	generic functions for all schemes					
*/

/* Copies char and requested length to outputbuffer
 * if full, writes to disk. */
void bpdb_global_add_to_buffer(gzFile *outFile, BYTE *outputbuffer, int *offset, BYTE r, int length) {

	if( (length + *offset) >=8 ) {
		char bpdb_global_add_to_buffer =  r >> (*offset - (BITSINBYTE - length));

		bpdb_write_byte(outFile, *outputbuffer | bpdb_global_add_to_buffer);
		*outputbuffer = 0;
		*outputbuffer = (bpdb_global_make_right_mask(*offset - (BITSINBYTE - length)) & r) << (BITSINBYTE - (*offset - (BITSINBYTE - length)));
		*offset = (*offset + length) % 8;
	} else {		
		*outputbuffer = *outputbuffer | (r << (BITSINBYTE - *offset - length));
		*offset += length;
	}
}

/* Creates a mask (ex. 0000111...) with the number of right
 * aligned 1s equal to the input variable n */
int bpdb_global_make_right_mask(int n) {
	int res = 0;

	while(n > 0) {
		res = res << 1;
		res++;
		n--;
	}

	return res;
}




/***********
************
**
**	Function Implementations for Decoding
**
**	generic functions for all schemes					
*/

/* Reads individual bits from inputbuffer until it reaches a 0 and returns how
 * many consecutive ones there were */
int bpdb_global_find_consecutive_ones(gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit) {
	int ones = 0;

	if(alreadyReadFirstBit) {
		ones = 1;
	}

	while(bpdb_global_read_next_bit(inFile, inputbuffer, offset)) {
		ones++;
	}
	return ones;
}

/* Returns the next byte from the input buffer */
int bpdb_global_read_next_byte(gzFile *inFile, BYTE *inputbuffer, int offset) {
	BYTE nextbyte = 0;
	int length = BITSINBYTE;
	
	if(offset >= 8 ) {
		*inputbuffer = bpdb_read_byte(inFile);
		offset -= 8;
	}
	
	nextbyte = (*inputbuffer) << offset;
	length -= (BITSINBYTE - offset);
	offset += 8;

	if(offset >= 8 && length > 0) {
		*inputbuffer = bpdb_read_byte(inFile);
		offset -= 8;
		nextbyte = nextbyte | (*inputbuffer >> (BITSINBYTE - offset));
	}

	return nextbyte;
}

/* Returns the next n bits from the inputbuffer, where n is specified by the
 * input variable length. */
int bpdb_global_read_next_bits(gzFile *inFile, BYTE *inputbuffer, int *offset, int length) {
	int value = 0;

	if(*offset >= 8 ) {
		*inputbuffer = bpdb_read_byte(inFile);
		*offset -= 8;
	}

	if((8 - *offset) < length) {
		value = (*inputbuffer & bpdb_global_make_right_mask(8 - *offset)) << (length - (8 - *offset));		
		*inputbuffer = bpdb_read_byte(inFile);

		length -= (8 - *offset);
		value = value | (*inputbuffer >> (8 - length));
		*offset = length;
	} else {
		value = (*inputbuffer >> (8 - length - *offset)) & bpdb_global_make_right_mask(length);
		*offset += length;
	}
	
	return value;
}

/* Returns the next bit from the input buffer */
int bpdb_global_read_next_bit(gzFile *inFile, BYTE *inputbuffer, int *offset) {
	int bitvalue;

	if(*offset >= 8 ) {
		*inputbuffer = bpdb_read_byte(inFile);
		*offset -= 8;
	}

	bitvalue = bpdb_global_make_right_mask(1) & ((*inputbuffer << *offset) >> 7);
	(*offset)++;

	return bitvalue;
}




/********************************
*********************************
**
** Scheme: Mem
** Encoding
**		
** Details:
** The original method for saving the database,
** which originated from memdb.c. A straight dump
** of memory is done to disk with space optimization
** dependent on gzip.
** 
*/

BOOLEAN bpdb_save_database_mem ( int schemeversion, char *outfilename )
{
	// number of positions
    POSITION numPos[1];

	// integrity checks
    int goodCompression = 1;
    int goodClose = 0;

	// counter
    unsigned long i;

    POSITION tot = 0,sTot = gNumberOfPositions;

	// gzip file ptr
	gzFile *filep;

	// create directory if it does not exist
    mkdir("data", 0755) ;

	// format file name
    sprintf(outfilename, "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), schemeversion);

	// open file for writing
    if((filep = gzopen(outfilename, "wb")) == NULL) {
		if(kDebugDetermineValue) {
			printf("Unable to create compressed data file\n");
        }
        return FALSE;
    }

    numPos[0] = htonl(gNumberOfPositions);
    
	// Write version number
	goodCompression = bpdb_write_byte(filep, (BYTE)schemeversion);

	goodCompression += gzwrite(filep, numPos, sizeof(POSITION));
    for(i=0; i<gNumberOfPositions && goodCompression; i++) { //convert to network byteorder for platform independence.
            bpdb_array[i] = htons(bpdb_array[i]);
            goodCompression = gzwrite(filep, bpdb_array+i, sizeof(cellValue));
            tot += goodCompression;
            bpdb_array[i] = ntohs(bpdb_array[i]);
            //gzflush(filep,Z_FULL_FLUSH);
    }
    goodClose = gzclose(filep);

    if(goodCompression && (goodClose == 0)) {
        if(kDebugDetermineValue && ! gJustSolving) {
            printf("File Successfully compressed\n");
		}
		return TRUE;
	} else {
        if(kDebugDetermineValue) {
                fprintf(stderr, "\nError in file compression.\n Error codes:\ngzwrite error: %d\ngzclose error:%d\nBytes To Be Written: " POSITION_FORMAT "\nBytes Written: " POSITION_FORMAT "\n",goodCompression, goodClose,sTot*4,tot);
        }
        remove(outfilename);
        return FALSE;
    }
}




/********************************
*********************************
**
** Scheme: Mem
** Decoding
**	
*/

BOOLEAN bpdb_load_database_mem( gzFile *filep )
{
    POSITION numPos[1];
    int goodDecompression = 1;
    POSITION i;

    goodDecompression = gzread(filep,numPos,sizeof(POSITION));
    *numPos = ntohl(*numPos);
    if(*numPos != gNumberOfPositions && kDebugDetermineValue) {
            printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions\n\n");
            return FALSE;
    }
	/***
     ** Ver. 0 Memdb's Decoding Scheme
     ***/
	for(i = 0; i < gNumberOfPositions && goodDecompression; i++) {
		goodDecompression = gzread(filep, bpdb_array+i, sizeof(cellValue));
		bpdb_array[i] = ntohs(bpdb_array[i]);
	}
    /***
     ** End Ver. 0
     ***/

	if(goodDecompression) {
			if(kDebugDetermineValue) {
					printf("File Successfully Decompressed\n");
			}
			return TRUE;
	} else {
			for(i = 0 ; i < gNumberOfPositions ; i++)
					bpdb_array[i] = undecided ;
			if(kDebugDetermineValue) {
					printf("\n\nError in file decompression:\ngzread error: %d\n",goodDecompression);
			}
			return FALSE;
	}
}




/********************************
*********************************
**
** Scheme: Dan
** Encoding
**		
** Details:
** The original bit-perfect scheme. A skip is indicated
** by a 1 bit. The number of consecutive ones, including the
** indicator one then specifies the size of the field
** required to specify the number of consecutively skipped
** slices.
** 
*/

void bpdb_dan_variablenumber_to_buffer( gzFile *outFile, BYTE *outputbuffer, int *offset, unsigned int n );
int bpdb_dan_bits_for_gap( int gaplength );

BOOLEAN bpdb_save_database_dan( int schemeversion, char *outfilename )
{   
    
    int goodCompression = 1;
    int goodClose = 0;
    POSITION tot = 0,sTot = gNumberOfPositions;
    
	BYTE outputbuffer = 0;
	// amount offset in bits
	int offset = 0;

	unsigned int numOfRecords = gNumberOfPositions;
	unsigned int sizeOfRecord = 2;
	unsigned int numOfSkips = 0;

	BYTE *temp;
    unsigned int i = 0, j = 0;
	
	// gzip file ptr
    gzFile *outFile;

    mkdir("data", 0755) ;
    sprintf(outfilename, "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), schemeversion);
    if((outFile = gzopen(outfilename, "wb")) == NULL) {
        if(kDebugDetermineValue){
            printf("Unable to create compressed data file\n");
        }
        return FALSE;
    }

	bpdb_global_add_to_buffer( outFile, &outputbuffer, &offset, schemeversion, 8 );
    bpdb_dan_variablenumber_to_buffer(outFile, &outputbuffer, &offset, gNumberOfPositions);

	// Loop through all records
	for(i=0; i<numOfRecords; i++) {

		// Check if the record is valid
		if(GetValueOfPosition(i) != undecided) {
			
			// If so, then check to see if skips must be outputted
			if(numOfSkips != 0) {
				// Put skips into output buffer
				bpdb_dan_variablenumber_to_buffer(outFile, &outputbuffer, &offset, numOfSkips);
				// Reset skip counter
				numOfSkips = 0;
			}

			// Output bit indicating a valid entry
			bpdb_global_add_to_buffer(outFile, &outputbuffer, &offset, 0, 1);

			bpdb_array[i] = htons(bpdb_array[i]);
	
			// Grab the data
			temp = (BYTE *) (bpdb_array+i);
			//printf("\tData grabbed.%d\n", i);
			for(j = 0; j < sizeOfRecord; j++) {
				bpdb_global_add_to_buffer(outFile, &outputbuffer, &offset, *(temp + j), 8);
			}
			bpdb_array[i] = ntohs(bpdb_array[i]);
		} else {
			numOfSkips++;
		}
	}

	if(numOfSkips != 0) {
		bpdb_dan_variablenumber_to_buffer(outFile, &outputbuffer, &offset, numOfSkips);
		numOfSkips = 0;
	}

	if(offset != 0) {
		bpdb_write_byte(outFile, outputbuffer);
	}

	goodClose = gzclose(outFile);

	if(goodClose == 0)
	{
	    if(kDebugDetermineValue && ! gJustSolving){
			printf("File Successfully compressed\n");
	    }
		return TRUE;
	} else {
        if(kDebugDetermineValue){
            fprintf(stderr, "\nError in file compression.\n Error codes:\ngzwrite error: %d\ngzclose error:%d\nBytes To Be Written: " POSITION_FORMAT "\nBytes Written: " POSITION_FORMAT "\n",goodCompression, goodClose,sTot*4,tot);
        }
        remove(outfilename);
        return FALSE;
	}
	return FALSE;
}

void bpdb_dan_variablenumber_to_buffer(gzFile *outFile, BYTE *outputbuffer, int *offset, unsigned int n) {
	int i = 0, ni = 0;
	int numbits = bpdb_dan_bits_for_gap(n);

	BYTE *bitarray = (BYTE *)SafeMalloc((2 * numbits + 1) * sizeof(char));

	for(i = 0; i < numbits; i++ ) {
		bitarray[i] = 1;
	}

	bitarray[i] = 0;
	i++;

	if(numbits == 1) {
		n -= 1;
	} else {
		n++;
		n -= (int)pow(2, numbits);
	}

	for(; i < (2*numbits + 1); i++) {
		ni = (i - numbits - 1);
		bitarray[i] = n << ((31 - (numbits - ni - 1))) >> 31;
	}

	for(i=0; i< (2*numbits + 1); i++) {
		bpdb_global_add_to_buffer(outFile, outputbuffer, offset, bitarray[i],1);
	}
}

int bpdb_dan_bits_for_gap(int gaplength) {
	int bits = 1;
	int numsupto = 2;
	int powerto = 2;
	
	while(numsupto < gaplength)
	{
		numsupto += (int)pow(2, powerto);
		bits++;
		powerto++;
	}
	return bits;
}






/********************************
*********************************
**
** Scheme: Dan
** Decoding
**		
*/

int bpdb_dan_read_variablenumber( gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit );

BOOLEAN bpdb_load_database_dan( gzFile *inFile )
{
	// specify size in bytes
	int sizeOfRecord = 2;
	
	// vars for decoding
	BYTE inputbuffer = 0;
	int offset = 8;

	// counters
	int gapSize = 0;
	int currentRecord = 0;
	int numOfRecords = 0;

	// counters
    int i;
	BYTE *temp;

	// read number of records
	//inputbuffer = bpdb_read_byte(inFile);
	numOfRecords = bpdb_dan_read_variablenumber(inFile, &inputbuffer, &offset, FALSE);

    if(numOfRecords != gNumberOfPositions && kDebugDetermineValue){
        printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions\n\n");
        return FALSE;
    }
    
	/***
     ** Ver. 1 Dan's Decoding Scheme
     ***/

	while(currentRecord < numOfRecords) {
		if(bpdb_global_read_next_bit(inFile, &inputbuffer, &offset) == 0) {
			temp = (BYTE *) (bpdb_array + currentRecord);
			
			for(i = 0; i < sizeOfRecord; i++) {
				*(temp + i) = (BYTE) bpdb_global_read_next_byte(inFile, &inputbuffer, offset);
			}

			*(bpdb_array + currentRecord) = ntohs( *(bpdb_array + currentRecord) );
			currentRecord++;
		} else {
			gapSize = bpdb_dan_read_variablenumber(inFile, &inputbuffer, &offset, TRUE);

			for(i = 0; i < gapSize; i++, currentRecord++) {
				bpdb_set_value(currentRecord, undecided);
			}
		}
	}

    /***
     ** End Ver. 1
     ***/

    if(kDebugDetermineValue){
		printf("File Successfully Decompressed\n");
    }
    return TRUE;
}

int bpdb_dan_read_variablenumber(gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit) {
	int i;
	int gapsize = 0;
	int ones = bpdb_global_find_consecutive_ones(inFile, inputbuffer, offset, alreadyReadFirstBit);

	for(i = 0; i < ones; i++) {
		gapsize = gapsize << 1;
		gapsize = gapsize | bpdb_global_read_next_bit(inFile, inputbuffer, offset);
	}

	if(ones == 1) {
		gapsize += 1;
	} else {
		gapsize--;
		gapsize += (int)pow(2, ones);
	}

	return gapsize;
}





/********************************
*********************************
**
** Scheme: Scott
** Encoding
**		
** Details:
** Same as Dan's, but for every 1 bit specifying the size of a skip
** the number of bits used to determine the size of the skip grows
** exponentially. 1:1, 2:3, 3:7, 4:15,...
** 
*/

void bpdb_scott_variablenumber_to_buffer( gzFile *outFile, BYTE *outputbuffer, int *offset, unsigned int n );
int bpdb_scott_bits_for_gap( int gaplength );

int bpdb_scott_implicit_amount( int leftbits );
int bpdb_scott_number_of_right_bits( int leftbits );
int bpdb_scott_pow2_greater_than(int *length);

BOOLEAN bpdb_save_database_scott( int schemeversion, char *outfilename )
{   
    int goodCompression = 1;
    int goodClose = 0;
    POSITION tot = 0,sTot = gNumberOfPositions;
    
	BYTE outputbuffer = 0;
	// amount offset in bits
	int offset = 0;

	unsigned int numOfRecords = gNumberOfPositions;
	unsigned int sizeOfRecord = 2;
	unsigned int numOfSkips = 0;

	BYTE *temp;
    unsigned int i = 0, j = 0;
	
	// gzip file ptr
    gzFile *outFile;
    
    mkdir("data", 0755) ;
    sprintf(outfilename, "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), schemeversion);
    if((outFile = gzopen(outfilename, "wb")) == NULL) {
        if(kDebugDetermineValue){
            printf("Unable to create compressed data file\n");
        }
        return FALSE;
    }

	bpdb_global_add_to_buffer( outFile, &outputbuffer, &offset, schemeversion, 8 );
	//printf("blah 1\n");
    bpdb_scott_variablenumber_to_buffer(outFile, &outputbuffer, &offset, gNumberOfPositions);

	//printf("blah 2\n");
	// Loop through all records
	for(i=0; i<numOfRecords; i++) {

		// Check if the record is valid
		if(GetValueOfPosition(i) != undecided) {
			
			// If so, then check to see if skips must be outputted
			if(numOfSkips != 0) {
				// Put skips into output buffer
				bpdb_scott_variablenumber_to_buffer(outFile, &outputbuffer, &offset, numOfSkips);
				// Reset skip counter
				numOfSkips = 0;
			}

			// Output bit indicating a valid entry
			bpdb_global_add_to_buffer(outFile, &outputbuffer, &offset, 0, 1);

			bpdb_array[i] = htons(bpdb_array[i]);
	
			// Grab the data
			temp = (BYTE *) (bpdb_array+i);
			//printf("\tData grabbed.%d\n", i);
			for(j = 0; j < sizeOfRecord; j++) {
				bpdb_global_add_to_buffer(outFile, &outputbuffer, &offset, *(temp + j), 8);
			}
			bpdb_array[i] = ntohs(bpdb_array[i]);
		} else {
			numOfSkips++;
		}
	}

	printf("Number of skips remaining: %d\n", numOfSkips);
	if(numOfSkips != 0) {
		bpdb_scott_variablenumber_to_buffer(outFile, &outputbuffer, &offset, numOfSkips);
		numOfSkips = 0;
	}

	if(offset != 0) {
		bpdb_write_byte(outFile, outputbuffer);
	}

	goodClose = gzclose(outFile);

	if(goodClose == 0)
	{
	    if(kDebugDetermineValue && ! gJustSolving){
			printf("File Successfully compressed\n");
	    }
		return TRUE;
	} else {
        if(kDebugDetermineValue){
            fprintf(stderr, "\nError in file compression.\n Error codes:\ngzwrite error: %d\ngzclose error:%d\nBytes To Be Written: " POSITION_FORMAT "\nBytes Written: " POSITION_FORMAT "\n",goodCompression, goodClose,sTot*4,tot);
        }
        remove(outfilename);
        return FALSE;
	}
	return FALSE;
}


void bpdb_scott_variablenumber_to_buffer(gzFile *outFile, BYTE *outputbuffer, int *offset, unsigned int n) {
	int i = 0, ni = 0;
	int leftbits = 0;
	int rightbits = bpdb_scott_bits_for_gap(n);

	//printf("test 1\n");
	leftbits = bpdb_scott_pow2_greater_than(&rightbits);

	//printf("test 2\n");

	BYTE *bitarray = (BYTE *)SafeMalloc((leftbits + rightbits + 1) * sizeof(char));

	

	for(i = 0; i < leftbits; i++ ) {
		bitarray[i] = 1;
	}

	bitarray[i] = 0;
	i++;

	/*if(rightbits == 1) {
		n -= 1;
	} else {
		n++;
		n -= (int)pow(2, leftbits);
	}*/
	//printf("size: %d", n);
	n -= bpdb_scott_implicit_amount(leftbits);

	//printf(", left: %d, right: %d, value: %d\n", leftbits, rightbits, n);

	for(; i < (leftbits + rightbits + 1); i++) {
		ni = (i - leftbits - 1);
		bitarray[i] = n << ((31 - (rightbits - ni - 1))) >> 31;
	}

	for(i=0; i< (leftbits + rightbits + 1); i++) {
		bpdb_global_add_to_buffer(outFile, outputbuffer, offset, bitarray[i],1);
	}
}

int bpdb_scott_implicit_amount( int leftbits ) {
	int bits = 1;
	int subamt = 0;

	while( bits <= leftbits ) {
		subamt += (int)pow(2, bpdb_scott_number_of_right_bits(bits));
		bits++;
	}

	return subamt;
}

int bpdb_scott_number_of_right_bits( int leftbits ) {
	int rightbits = 1;
	int bits = 1;

	while(bits <= leftbits) {
		rightbits += (int)pow(2, bits);
		bits ++;
	}

	return rightbits;
}

int bpdb_scott_pow2_greater_than( int *length ) {
	int bits = 1;
	int lengthrepresented = 1;
	int power = 1;

	while(lengthrepresented < *length) {
		lengthrepresented += (int)pow(2, power);
		power++;
		bits++;
	}

	*length = lengthrepresented;
	return bits;

}






/********************************
*********************************
**
** Scheme: Scott
** Decoding
** 
*/

int bpdb_scott_read_variablenumber( gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit );
int bpdb_scott_bits_for_gap( int ones );

BOOLEAN bpdb_load_database_scott( gzFile *inFile )
{
	// specify size in bytes
	int sizeOfRecord = 2;
	
	// vars for decoding
	BYTE inputbuffer = 0;
	int offset = 8;

	// counters
	int gapSize = 0;
	int currentRecord = 0;
	int numOfRecords = 0;

	// counters
    int i;
	BYTE *temp;

	// read number of records
	printf("I found the # of records to be: %d\n", numOfRecords);
	numOfRecords = bpdb_scott_read_variablenumber(inFile, &inputbuffer, &offset, FALSE);
	printf("I found the # of records to be: %d\n", numOfRecords);

    if(numOfRecords != gNumberOfPositions && kDebugDetermineValue){
        printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions\n\n");
        return FALSE;
    }
    
	/***
     ** Ver. 2 Scott's Encoding Scheme
     ***/

	while(currentRecord < numOfRecords) {
		
		if(bpdb_global_read_next_bit(inFile, &inputbuffer, &offset) == 0) {
			temp = (BYTE *) (bpdb_array + currentRecord);
			
			for(i = 0; i < sizeOfRecord; i++) {
				*(temp + i) = (BYTE) bpdb_global_read_next_byte(inFile, &inputbuffer, offset);
			}

			*(bpdb_array + currentRecord) = ntohs( *(bpdb_array + currentRecord) );
			currentRecord++;
		} else {
			gapSize = bpdb_scott_read_variablenumber(inFile, &inputbuffer, &offset, TRUE);

			for(i = 0; i < gapSize; i++, currentRecord++) {
				bpdb_set_value(currentRecord, undecided);
				printf("Record %d : %d", currentRecord, numOfRecords);
			}
		}
	}

    /***
     ** End Ver. 1
     ***/

    if(kDebugDetermineValue){
		printf("File Successfully Decompressed\n");
    }
    return TRUE;
}

int bpdb_scott_read_variablenumber(gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit) {
	int i;
	unsigned int gapsize = 0;
	int ones = bpdb_global_find_consecutive_ones(inFile, inputbuffer, offset, alreadyReadFirstBit);
	//printf("Numbers of ones: %d\n", ones);
	int gapbits = bpdb_scott_bits_for_gap(ones);
	//printf("Numbers of gapbits: %d\n", gapbits);

	for(i = 0; i < gapbits; i++) {
		gapsize = gapsize << 1;
		gapsize = gapsize | bpdb_global_read_next_bit(inFile, inputbuffer, offset);
	}

	//printf("The amount to add: %d\n", bpdb_scott_implicit_amount(ones));

	//printf("before add gapsize: %d\n", gapsize);
	gapsize += bpdb_scott_implicit_amount(ones);
	//printf("after add gapsize: %d\n", gapsize);

	return gapsize;
}

int bpdb_scott_bits_for_gap(int ones) {
	int bits = 1;
	int repbits = 1;
	int power = 1;

	while(bits < ones) {
		repbits += (int)pow(2, power);
		power++;
		bits++;
	}

	return repbits;
}





/********************************
*********************************
**
** Scheme: Ken
** Encoding
**		
** Details:
** Same as Dan's, but for every 1 bit specifying the size of a skip
** 2 bits are used to show the size of the skip
** 
*/

void bpdb_ken_variablenumber_to_buffer( gzFile *outFile, BYTE *outputbuffer, int *offset, unsigned int n );
int bpdb_ken_bits_for_gap( int gaplength );
int bpdb_ken_implicit_amount( int ones );

BOOLEAN bpdb_save_database_ken( int schemeversion, char *outfilename )
{   
    int goodCompression = 1;
    int goodClose = 0;
    POSITION tot = 0,sTot = gNumberOfPositions;
    
	BYTE outputbuffer = 0;
	// amount offset in bits
	int offset = 0;

	unsigned int numOfRecords = gNumberOfPositions;
	unsigned int sizeOfRecord = 2;
	unsigned int numOfSkips = 0;

	BYTE *temp;
    unsigned int i = 0, j = 0;
	
	// gzip file ptr
    gzFile *outFile;
    
    mkdir("data", 0755) ;
    sprintf(outfilename, "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), schemeversion);
    if((outFile = gzopen(outfilename, "wb")) == NULL) {
        if(kDebugDetermineValue){
            printf("Unable to create compressed data file\n");
        }

		return FALSE;
    }

	bpdb_global_add_to_buffer( outFile, &outputbuffer, &offset, schemeversion, 8 );
    bpdb_ken_variablenumber_to_buffer(outFile, &outputbuffer, &offset, gNumberOfPositions);

	// Loop through all records
	for(i=0; i<numOfRecords; i++) {
		// Check if the record is valid
		if(GetValueOfPosition(i) != undecided) {
			
			// If so, then check to see if skips must be outputted
			if(numOfSkips != 0) {
				// Put skips into output buffer
				bpdb_ken_variablenumber_to_buffer(outFile, &outputbuffer, &offset, numOfSkips);
				// Reset skip counter
				numOfSkips = 0;
			}

			// Output bit indicating a valid entry
			bpdb_global_add_to_buffer(outFile, &outputbuffer, &offset, 0, 1);

			bpdb_array[i] = htons(bpdb_array[i]);
	
			// Grab the data
			temp = (BYTE *) (bpdb_array+i);
			for(j = 0; j < sizeOfRecord; j++) {
				bpdb_global_add_to_buffer(outFile, &outputbuffer, &offset, *(temp + j), 8);
			}
			bpdb_array[i] = ntohs(bpdb_array[i]);
		} else {
			numOfSkips++;
		}
	}

	if(numOfSkips != 0) {
		bpdb_ken_variablenumber_to_buffer(outFile, &outputbuffer, &offset, numOfSkips);
		numOfSkips = 0;
	}

	if(offset != 0) {
		bpdb_write_byte(outFile, outputbuffer);
	}

	goodClose = gzclose(outFile);

	if(goodClose == 0)
	{
	    if(kDebugDetermineValue && ! gJustSolving){
			printf("File Successfully compressed\n");
	    }
		return TRUE;
	} else {
        if(kDebugDetermineValue){
            fprintf(stderr, "\nError in file compression.\n Error codes:\ngzwrite error: %d\ngzclose error:%d\nBytes To Be Written: " POSITION_FORMAT "\nBytes Written: " POSITION_FORMAT "\n",goodCompression, goodClose,sTot*4,tot);
        }
        remove(outfilename);
        return FALSE;
	}
	return FALSE;
}

void bpdb_ken_variablenumber_to_buffer(gzFile *outFile, BYTE *outputbuffer, int *offset, unsigned int n) {
	int i = 0, ni = 0;
	int leftbits = 0;
	int rightbits = 0;
	leftbits =  bpdb_ken_bits_for_gap(n);
	rightbits = 2 * leftbits;

	BYTE *bitarray = (BYTE *)SafeMalloc((leftbits + rightbits + 1) * sizeof(char));

	for(i = 0; i < leftbits; i++ ) {
		bitarray[i] = 1;
	}

	bitarray[i] = 0;
	i++;

	n -= bpdb_ken_implicit_amount(leftbits);
	//printf(", new n: %d\n", n);
	for(; i < (leftbits + rightbits + 1); i++) {
		ni = (i - leftbits - 1);
		bitarray[i] = n << ((31 - (rightbits - ni - 1))) >> 31;
	}

	for(i=0; i< (leftbits + rightbits + 1); i++) {
		bpdb_global_add_to_buffer(outFile, outputbuffer, offset, bitarray[i],1);
	}
}

int bpdb_ken_bits_for_gap(int gaplength) {
	int leftbits = 1;
	
	while(!(gaplength < bpdb_ken_implicit_amount(leftbits + 1)))
	{
		leftbits++;
	}

	return leftbits;
}

int bpdb_ken_implicit_amount(int ones) {
	int bits = 1;
	int power = 2;
	int amt = 1;
	
	while(bits < ones) {
		amt += (int) pow(2, power);
		power += 2;
		bits++;
	}

	return amt;
}






/********************************
*********************************
**
** Scheme: Ken
** Decoding
**					
*/

int bpdb_ken_read_variablenumber( gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit );

BOOLEAN bpdb_load_database_ken( gzFile *inFile )
{
	// specify size in bytes
	int sizeOfRecord = 2;
	
	// vars for decoding
	BYTE inputbuffer = 0;
	int offset = 8;

	// counters
	int gapSize = 0;
	int currentRecord = 0;
	int numOfRecords = 0;

	// counters
    int i;
	BYTE *temp;

	// read number of records
	//inputbuffer = bpdb_read_byte(inFile);
	numOfRecords = bpdb_ken_read_variablenumber(inFile, &inputbuffer, &offset, FALSE);
	//printf("Number of records yo: %d\n", numOfRecords);

    if(numOfRecords != gNumberOfPositions && kDebugDetermineValue){
        printf("\n\nError in file decompression: Stored gNumberOfPositions differs from internal gNumberOfPositions\n\n");
        return FALSE;
    }
    
	/***
     ** Ver. 1 Dan's Decoding Scheme
     ***/

	while(currentRecord < numOfRecords) {
		if(bpdb_global_read_next_bit(inFile, &inputbuffer, &offset) == 0) {
			temp = (BYTE *) (bpdb_array + currentRecord);
			
			for(i = 0; i < sizeOfRecord; i++) {
				*(temp + i) = (BYTE) bpdb_global_read_next_byte(inFile, &inputbuffer, offset);
			}

			*(bpdb_array + currentRecord) = ntohs( *(bpdb_array + currentRecord) );
			currentRecord++;
		} else {
			gapSize = bpdb_ken_read_variablenumber(inFile, &inputbuffer, &offset, TRUE);

			for(i = 0; i < gapSize; i++, currentRecord++) {
				bpdb_set_value(currentRecord, undecided);
			}
		}
	}

    /***
     ** End Ver. 1
     ***/

    if(kDebugDetermineValue){
		printf("File Successfully Decompressed\n");
    }
    return TRUE;
}

int bpdb_ken_read_variablenumber(gzFile *inFile, BYTE *inputbuffer, int *offset, int alreadyReadFirstBit) {
	int i;
	int gapsize = 0;
	int ones = bpdb_global_find_consecutive_ones(inFile, inputbuffer, offset, alreadyReadFirstBit);
	
	for(i = 0; i < 2*ones; i++) {
		gapsize = gapsize << 1;
		gapsize = gapsize | bpdb_global_read_next_bit(inFile, inputbuffer, offset);
	}
	//printf("Number of ones: %d, Gap Size: %d", ones, gapsize);
	gapsize += bpdb_ken_implicit_amount(ones);
	//printf(", full size: %d\n", gapsize);

	return gapsize;
}
