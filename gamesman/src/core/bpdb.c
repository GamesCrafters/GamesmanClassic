/************************************************************************
**
** NAME:	bpdb.c
**
** DESCRIPTION:	Accessor functions for the bit-perfect database.
**
** AUTHOR:	Ken Elkabany
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-05-01
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

#include "bpdb.h"
#include "bpdb_bitlib.h"
#include "bpdb_schemes.h"

/*
 * Global Variables
 */

// in memory database
BYTE		*bpdb_array;

// numbers of slices
UINT64		bpdb_slices;

// bits per slice
UINT8		bpdb_bits_per_slice;

// number of slots
UINT8		bpdb_slot_count = 0;

// array of slots sizes
UINT8		*bpdb_slot_sizes;

// names of slots
char		**bpdb_slot_names;

// pointer to scheme list
Scheme_List	bpdb_list;

// size of scheme list
int bpdb_list_size;

/*int main() {
	BOOLEAN save = FALSE;
	bpdb_init( 10, 19 );

	if(save) {
		bpdb_add_slot( 3, "VALUE" );
		bpdb_add_slot( 15, "REMOTENESS" );
		bpdb_add_slot( 1, "BIT" );

		bpdb_set_slice_slot( 0, 0, undecided );
		bpdb_set_slice_slot( 1, 0, 7 );
		bpdb_set_slice_slot( 1, 1, 2048 );
		bpdb_set_slice_slot( 1, 1, 432 );
		bpdb_set_slice_slot( 1, 2, 1 );
		bpdb_set_slice_slot( 3, 0, 3 );
		bpdb_set_slice_slot( 3, 1, 3 );
		bpdb_set_slice_slot( 4, 0, undecided );
		bpdb_set_slice_slot( 9, 2, 1 );

		//bitlib_print_bytes_in_bits( bpdb_array, 24 );

		bpdb_save_database();
	} else {
		bpdb_load_database();
	}

	bpdb_print_database();

	//bitlib_print_bytes_in_bits( bpdb_array, 24 );

	return 0;
}*/


/*
 * bpdb_init
 */
//void bpdb_init( UINT64 slices, UINT64 bits_per_slice )
void bpdb_init( DB_Table *new_db ) {
	UINT64 i;

	bpdb_slices = gNumberOfPositions;
	bpdb_bits_per_slice = 16;

	bpdb_array = (BYTE *) SafeMalloc( (size_t)ceil((size_t)(bpdb_slices/BITSINBYTE) * (size_t)(bpdb_bits_per_slice) ) * sizeof(BYTE));

	// Maybe uneeded, depending on the capabilities of SafeMalloc
	for(i = 0; i < ceil(bpdb_slices*bpdb_bits_per_slice/BITSINBYTE); i++) {
		bpdb_array[i] = 0;
	}

	bpdb_add_slot( 2, "VALUE" );		//slot 0
	bpdb_add_slot( 5, "MEX" );			//slot 1
	bpdb_add_slot( 8, "REMOTENESS" );	//slot 2
	bpdb_add_slot( 1, "VISITED" );		//slot 3

	for(i = 0; i < bpdb_slices; i++) {
		bpdb_set_slice_slot( i, VALUESLOT, undecided );
	}

    new_db->get_value = bpdb_get_value;
    new_db->put_value = bpdb_set_value;
    new_db->get_remoteness = bpdb_get_remoteness;
    new_db->put_remoteness = bpdb_set_remoteness;
    new_db->check_visited = bpdb_check_visited;
    new_db->mark_visited = bpdb_mark_visited;
    new_db->unmark_visited = bpdb_unmark_visited;
    new_db->get_mex = bpdb_get_mex;
    new_db->put_mex = bpdb_set_mex;
    new_db->save_database = bpdb_save_database;
    new_db->load_database = bpdb_load_database;
    
    new_db->free_db = bpdb_free;

	bpdb_list = scheme_list_add( bpdb_list, 0, NULL, bpdb_mem_write_varnum, FALSE ); bpdb_list_size++;
	bpdb_list = scheme_list_add( bpdb_list, 1, bpdb_generic_read_varnum, bpdb_generic_varnum, TRUE ); bpdb_list_size++;
//	bpdb_list = scheme_list_add( bpdb_list, 2, bpdb_scott_read_varnum, bpdb_scott_varnum, TRUE ); bpdb_list_size++;
}

void bpdb_free() {

}


VALUE bpdb_set_value(POSITION pos, VALUE val)
{
	bpdb_set_slice_slot( (UINT64)pos, VALUESLOT, (UINT64) val );
	// bpdb_set_slice_slot needs to return val
    return val;
}

VALUE bpdb_get_value(POSITION pos)
{
    return (VALUE) bpdb_get_slice_slot( (UINT64)pos, VALUESLOT );
}

REMOTENESS bpdb_get_remoteness(POSITION pos)
{
	return (REMOTENESS) bpdb_get_slice_slot( (UINT64)pos, REMSLOT );
}

void bpdb_set_remoteness (POSITION pos, REMOTENESS val)
{
	bpdb_set_slice_slot( (UINT64)pos, REMSLOT, (REMOTENESS) val );
    
	/*
	if(val > REMOTENESS_MAX) {
        printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",val,pos,REMOTENESS_MAX);
        ExitStageRight();
        exit(0);
    }
	*/
}

BOOLEAN bpdb_check_visited(POSITION pos)
{
	return (BOOLEAN) bpdb_get_slice_slot( (UINT64)pos, VISITEDSLOT );
}

void bpdb_mark_visited (POSITION pos)
{
	bpdb_set_slice_slot( (UINT64)pos, VISITEDSLOT, (UINT64)1 );
}

void bpdb_unmark_visited (POSITION pos)
{
	bpdb_set_slice_slot( (UINT64)pos, VISITEDSLOT, (UINT64)0 );
}

void bpdb_set_mex(POSITION pos, MEX mex)
{
	bpdb_set_slice_slot( (UINT64)pos, MEXSLOT, (UINT64)mex );
}

MEX bpdb_get_mex(POSITION pos)
{
	return (MEX) bpdb_get_slice_slot( (UINT64)pos, MEXSLOT );
}


void bpdb_set_slice_full( UINT64 position, UINT64 value ) {
	BYTE *slice;
	UINT64 byteOffset;
	UINT8 bitOffset;

	byteOffset = (bpdb_bits_per_slice * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_bits_per_slice % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;

	slice = (bpdb_array + byteOffset);
	bitlib_insert_bits( slice, bitOffset, value, bpdb_bits_per_slice );
}

void bpdb_set_slice_slot( UINT64 position, UINT8 index, UINT64 value ) {
	int i;
	BYTE *slice;
	UINT64 byteOffset;
	UINT8 bitOffset;
	UINT8 length;

	byteOffset = (bpdb_bits_per_slice * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_bits_per_slice % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;

	for(i = 0; i < index; i++) {
		bitOffset += bpdb_slot_sizes[i];
	}

	length = bpdb_slot_sizes[i];
	
	while(!(bitOffset < BITSINBYTE)) {
		bitOffset -= BITSINBYTE;
		byteOffset++;
	}

	slice = (bpdb_array + byteOffset);
	//printf("byteoff: %d bitoff: %d value: %llu length: %d\n", byteOffset, bitOffset, value, length);
	//printf("value: %llu\n", value);
	bitlib_insert_bits( slice, bitOffset, value, length );
}

UINT64 bpdb_get_slice_full( UINT64 position ) {
	BYTE *slice;
	UINT64 byteOffset;
	UINT8 bitOffset;

	byteOffset = (bpdb_bits_per_slice * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_bits_per_slice % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;

	slice = (bpdb_array + byteOffset);
	return bitlib_read_bits( slice, bitOffset, bpdb_bits_per_slice );
}


UINT64 bpdb_get_slice_slot( UINT64 position, UINT8 index ) {
	int i;
	BYTE *slice;
	UINT64 byteOffset;
	UINT8 bitOffset;
	UINT8 length;

	byteOffset = (bpdb_bits_per_slice * position)/BITSINBYTE;
	bitOffset = ((UINT8)(bpdb_bits_per_slice % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;

	for(i = 0; i < index; i++) {
		bitOffset += bpdb_slot_sizes[i];
	}
	length = bpdb_slot_sizes[i];
	
	while(!(bitOffset < BITSINBYTE)) {
		bitOffset -= BITSINBYTE;
		byteOffset++;
	}

	slice = (bpdb_array + byteOffset);
	return bitlib_read_bits( slice, bitOffset, length );
}


BOOLEAN bpdb_add_slot( UINT8 size, char *name ) {
	int i;
	UINT8 bitsUsed = 0;
	
	if(!(size > 0)) {
		printf("Error: Slot width must have a positive value\n");
		return FALSE;
	}

	for(i = 0; i<bpdb_slot_count; i++) {
		bitsUsed += bpdb_slot_sizes[i];
	}
	if((bitsUsed + size) > bpdb_bits_per_slice) {
		printf("Error: Allocating more bits for slots, than bits possible\n");
		return FALSE;
	}
	
	bpdb_slot_count++;

	if(bpdb_slot_count == 1) {
		bpdb_slot_sizes = (UINT8 *) calloc( 1, sizeof(UINT8) );
		bpdb_slot_names = (char **) calloc( 1, sizeof(char **) );
	} else {
		bpdb_slot_sizes = (UINT8 *) realloc( bpdb_slot_sizes, bpdb_slot_count*sizeof(UINT8) );
		bpdb_slot_names = (char **) realloc( bpdb_slot_names, bpdb_slot_count*sizeof(char **) );
	}

	*(bpdb_slot_sizes + bpdb_slot_count - 1) = size;
	*(bpdb_slot_names + bpdb_slot_count - 1) = (char *) malloc( strlen(name) + 1 );
	strcpy( *(bpdb_slot_names + bpdb_slot_count - 1), name );

	return TRUE;
}




void bpdb_print_database() {
	UINT64 i;
	UINT8 j;

	for(i = 0; i < bpdb_slices; i++) {
		printf("Slice %llu: %llu\n", i, bpdb_get_slice_full(i));
		for(j=0; j<bpdb_slot_count; j++) {
			printf("%s: %llu  ", bpdb_slot_names[j], bpdb_get_slice_slot(i, j));
		}
		printf("\n\n");
		//bitlib_print_bytes_in_bits( &slice, bytesToPrint );
	}
}




BOOLEAN bpdb_save_database()
{
	// counter
	int i;
	Scheme_List cur;
	BOOLEAN success = FALSE;

	// file names of saved files
	char **outfilenames;

	// final file name
	char outfilename[256];

	// track smallest file
	int smallestscheme = 0;
	int smallestsize = -1;

	// struct for fileinfo
	struct stat fileinfo;

	// set counters
	i = 0;
	cur = bpdb_list;

	for(i = 0; i<bpdb_list_size; i++) {
		outfilenames = (char **) malloc( bpdb_list_size*sizeof(char*) );
	}
	for(i = 0; i<bpdb_list_size; i++) {
		outfilenames[i] = (char *) malloc( 256*sizeof(char) );
	}

	i = 0;

	printf("\n");

	// save file under each encoding scheme
	while(cur != NULL) {
		// saves with encoding scheme and returns filename
		//sprintf(outfilenames[i], "./data/m%s_%d_bpdb_%d.dat.gz", "test", 1, cur->scheme);
		sprintf(outfilenames[i], "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), cur->scheme);

		success = bpdb_generic_save_database( cur, outfilenames[i] );

		if(success) {
			// get size of file
			stat(outfilenames[i], &fileinfo);

			printf("Scheme: %d. Wrote %s with size of %d\n", cur->scheme, outfilenames[i], (int)fileinfo.st_size);

			// if file is a smaller size, set min
			if(smallestsize == -1 || fileinfo.st_size < smallestsize) {
				smallestscheme = i;
				smallestsize  = fileinfo.st_size;
			}
		}
		cur = cur->next;
		i++;
	}

	printf("Choosing scheme: %d\n", smallestscheme);
	
	// for each file, delete if not the smallest encoding,
	// and if it is, rename it to the final file name.
	for(i = 0; i < bpdb_list_size; i++) {
		if(i == smallestscheme) {

			// rename smallest file to final file name
			sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());
			//sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", "test", 1);
			printf("Renaming %s to %s\n", outfilenames[i], outfilename);
			rename(outfilenames[i], outfilename);
		} else {

			// delete files that are not the smallest
			printf("Removing %s\n", outfilenames[i]);
			remove(outfilenames[i]);
		}
	}

	return TRUE;
}

BOOLEAN bpdb_generic_varnum( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips ) {
	UINT8 leftBits, rightBits;
	
	leftBits = bpdb_generic_varnum_gap_bits( consecutiveSkips );
	rightBits = leftBits;

	bitlib_value_to_buffer( outFile, outputBuffer, offset, bitlib_right_mask64( leftBits), leftBits );
	bitlib_value_to_buffer( outFile, outputBuffer, offset, 0, 1 );

	consecutiveSkips -= bpdb_generic_varnum_implicit_amt( leftBits );

	bitlib_value_to_buffer( outFile, outputBuffer, offset, consecutiveSkips, rightBits );

	return TRUE;
}

UINT8 bpdb_generic_varnum_gap_bits( UINT64 consecutiveSkips ) {
	UINT8 leftBits = 1;
	UINT8 powerTo = 2;
	UINT64 skipsRepresented = 2;
	
	while(skipsRepresented < consecutiveSkips)
	{
		skipsRepresented += (UINT64)pow(2, powerTo);
		leftBits++;
		powerTo++;
	}
	return leftBits;
}

UINT64 bpdb_generic_varnum_implicit_amt( UINT8 leftBits ) {
	return (int)pow(2, leftBits) - 1;
}

BOOLEAN bpdb_generic_save_database( Scheme_List scheme, char *outfilename )
{
	UINT64 consecutiveSkips = 0;
	BYTE outputBuffer = 0;
	UINT8 offset = 0;
	UINT8 i, j;

	// gzip file ptr
    dbFILE *outFile;

	UINT64 slice;

    mkdir("data", 0755) ;
    if((outFile = bitlib_file_open(outfilename, "wb")) == NULL) {
		printf("Unable to create compressed data file\n");
    }

	bitlib_value_to_buffer ( outFile, &outputBuffer, &offset, scheme->scheme, 8 );

	bpdb_generic_varnum( outFile, &outputBuffer, &offset, bpdb_slices );
	bpdb_generic_varnum( outFile, &outputBuffer, &offset, bpdb_bits_per_slice );

	bpdb_generic_varnum( outFile, &outputBuffer, &offset, bpdb_slot_count );

	for(i = 0; i<bpdb_slot_count; i++) {
		bpdb_generic_varnum( outFile, &outputBuffer, &offset, bpdb_slot_sizes[i] );
		bpdb_generic_varnum( outFile, &outputBuffer, &offset, strlen(bpdb_slot_names[i]) );
		
		for(j = 0; j<strlen(bpdb_slot_names[i]); j++) {
			bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_slot_names[i][j], 8 );
		}
	}

	if(scheme->indicator) {
		for(slice=0; slice<bpdb_slices; slice++) {

			// Check if the slice has a mapping
			if(bpdb_get_slice_slot( slice, 0 ) != undecided) {
				
				// If so, then check to see if skips must be outputted
				if(consecutiveSkips != 0) {
					// Put skips into output buffer
					scheme->write_varnum(outFile, &outputBuffer, &offset, consecutiveSkips);
					// Reset skip counter
					consecutiveSkips = 0;
				}

				bitlib_value_to_buffer( outFile, &outputBuffer, &offset, 0, 1 );
				bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_get_slice_full(slice), bpdb_bits_per_slice );

			} else {
				consecutiveSkips++;
			}
		}
	} else {
		// Loop through all records
		for(slice=0; slice<bpdb_slices; slice++) {
			bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_get_slice_full(slice), bpdb_bits_per_slice );
		}
	}


	if(consecutiveSkips != 0) {
		scheme->write_varnum(outFile, &outputBuffer, &offset, consecutiveSkips);
		consecutiveSkips = 0;
	}

	if(offset != 0) {
		bitlib_file_write_byte(outFile, &outputBuffer);
	}

	if(bitlib_file_close(outFile)) {
		return TRUE;
	} else {
		return FALSE;
	}
	
	return FALSE;
}



BOOLEAN bpdb_load_database()
{
	Scheme_List cur;

	// filename
	char outfilename[256];

    //BOOLEAN correctDBVer = FALSE;
	BOOLEAN success = FALSE;
  
	dbFILE *inFile;

	// file information
	UINT8 fileFormat;

	// open file
	sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());
	//sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", "test", 1);
	if((inFile = bitlib_file_open(outfilename, "rb")) == NULL) {
		return FALSE;
	}
	
	// read file header
	fileFormat = bitlib_file_read_byte( inFile );

	// print fileinfo
	printf("\nEncoding Scheme: %d\n", fileFormat);
	
	cur = bpdb_list;

	while(cur->scheme != fileFormat) {
		cur = cur->next;
	}
	
	success = bpdb_generic_load_database( inFile, cur );

	// close file
	if(bitlib_file_close(inFile)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN bpdb_generic_load_database( dbFILE *inFile, Scheme_List scheme ) {
	UINT64 currentSlice = 0;

    UINT64 numOfSlicesHeader;
	UINT8 bitsPerSliceHeader;
	UINT8 numOfSlotsHeader;
	BYTE inputBuffer = 0;
	UINT8 offset = 0;
	UINT64 skips = 0;
	UINT64 i, j;
	char tempchar;
	char * tempname;
	UINT8 tempnamesize;
	UINT8 tempsize;
	BOOLEAN slotsMade = FALSE;

	inputBuffer = bitlib_file_read_byte( inFile );

	// Read Header
	numOfSlicesHeader = bpdb_generic_read_varnum( inFile, &inputBuffer, &offset, FALSE );
	bitsPerSliceHeader = bpdb_generic_read_varnum( inFile, &inputBuffer, &offset, FALSE );
	numOfSlotsHeader = bpdb_generic_read_varnum( inFile, &inputBuffer, &offset, FALSE );

	printf("Slices: %llu, Bits per slice: %d, offset: %d\n", numOfSlicesHeader, bitsPerSliceHeader, offset);

	if( bpdb_slot_count ) {
		slotsMade = TRUE;
	}

	for(i = 0; i<numOfSlotsHeader; i++) {
		tempsize = bpdb_generic_read_varnum( inFile, &inputBuffer, &offset, FALSE );
		tempnamesize = bpdb_generic_read_varnum( inFile, &inputBuffer, &offset, FALSE );
		printf("Slot size %llu: %d\n", i, tempsize);
		printf("\tString size %llu: %d\n", i, tempnamesize);

		tempname = (char *) malloc((tempnamesize + 1) * sizeof(char));

		for(j = 0; j<tempnamesize; j++) {
			tempchar = (char)bitlib_read_from_buffer( inFile, &inputBuffer, &offset, 8 );
			*(tempname + j) = tempchar;
		}
		*(tempname + j) = '\0';

		printf("\tString name: %s\n", tempname);
		if( !slotsMade ) {
			//bpdb_add_slot(tempsize, tempname);
		}
	}


	if( scheme->indicator ) {
		while(currentSlice < numOfSlicesHeader) {
			if(bitlib_read_from_buffer( inFile, &inputBuffer, &offset, 1 ) == 0) {
				bpdb_set_slice_full( currentSlice, bitlib_read_from_buffer( inFile, &inputBuffer, &offset, bpdb_bits_per_slice ) );
				currentSlice++;
			} else {
				
				skips = scheme->read_varnum( inFile, &inputBuffer, &offset, TRUE );

				for(i = 0; i < skips; i++, currentSlice++) {
					bpdb_set_slice_full( currentSlice, 0 );
					bpdb_set_slice_slot( currentSlice, 0, undecided );
				}
			}
		}
	} else {
		for( currentSlice = 0; currentSlice < numOfSlicesHeader; currentSlice++ ) {
			bpdb_set_slice_full( currentSlice, bitlib_read_from_buffer( inFile, &inputBuffer, &offset, bpdb_bits_per_slice ) );
		}
	}

	return TRUE;
}

UINT64 bpdb_generic_read_varnum( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit ) {
	UINT8 i;
	UINT64 variableNumber = 0;
	UINT8 leftBits, rightBits;

	leftBits = bpdb_generic_read_varnum_consecutive_ones( inFile, inputBuffer, offset, alreadyReadFirstBit );
	rightBits = leftBits;

	for(i = 0; i < rightBits; i++) {
		variableNumber = variableNumber << 1;
		variableNumber = variableNumber | bitlib_read_from_buffer( inFile, inputBuffer, offset, 1 );
	}

	variableNumber += bpdb_generic_varnum_implicit_amt( leftBits );

	return variableNumber;
}

UINT8 bpdb_generic_read_varnum_consecutive_ones( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit ) {
	UINT8 consecutiveOnes;

	if(alreadyReadFirstBit) {
		consecutiveOnes = 1;
	} else {
		consecutiveOnes = 0;
	}

	while(bitlib_read_from_buffer( inFile, inputBuffer, offset, 1 )) {
		consecutiveOnes++;
	}
	return consecutiveOnes;
}
