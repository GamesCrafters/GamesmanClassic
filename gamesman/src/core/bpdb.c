/************************************************************************
**
** NAME:    bpdb.c
**
** DESCRIPTION:    Accessor functions for the bit-perfect database.
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

//
// 2 different storages for write and nowrite
// fixed bugs with writing
// changing functions to inline
// added bpdb to db.h
// clean up

#include "bpdb.h"
#include "bpdb_bitlib.h"
#include "bpdb_schemes.h"

//
// Global Variables
//

//
// stores the format of a slice; in particular the
// names, sizes, and maxvalues of its slots
//

SLICE       bpdb_write_slice = NULL;
SLICE       bpdb_nowrite_slice = NULL;

// in memory database
BYTE        *bpdb_write_array = NULL;
BYTE        *bpdb_nowrite_array = NULL;

// numbers of slices
UINT64      bpdb_slices = 0;

// pointer to scheme list
Scheme_List bpdb_scheme_list = NULL;


/*
 * bpdb_init
 */
void
bpdb_init(
                DB_Table *new_db
                )
{
    GMSTATUS status = STATUS_SUCCESS;
    UINT64 i;

    if(NULL == new_db) {
        status = STATUS_INVALID_INPUT_PARAMETER;
        BPDB_TRACE("bpdb_init()", "Input parameter new_db is null", status);
        goto _bailout;
    }

    //
    // initialize global variables
    //
    bpdb_slices = gNumberOfPositions;

    bpdb_write_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );
    bpdb_nowrite_slice = (SLICE) calloc( 1, sizeof(struct sliceformat) );

    status = bpdb_add_slot( 2, "VALUE", TRUE );         //slot 0
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_init()", "Could not add value slot", status);
        goto _bailout;
    }

    status = bpdb_add_slot( 5, "MEX", TRUE );           //slot 2
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_init()", "Could not add mex slot", status);
        goto _bailout;
    }

    status = bpdb_add_slot( 8, "REMOTENESS", TRUE );    //slot 4
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_init()", "Could not add remoteness slot", status);
        goto _bailout;
    }

    status = bpdb_add_slot( 1, "VISITED", FALSE );       //slot 1
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_init()", "Could not add visited slot", status);
        goto _bailout;
    }

    // fixed heap corruption =p
    bpdb_write_array = (BYTE *) calloc( (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(bpdb_write_slice->bits) ), sizeof(BYTE));
    if(NULL == bpdb_write_array) {
        status = STATUS_NOT_ENOUGH_MEMORY;
        BPDB_TRACE("bpdb_init()", "Could not allocate bpdb_write_array in memory", status);
    }

    bpdb_nowrite_array = (BYTE *) calloc( (size_t)ceil(((double)bpdb_slices/(double)BITSINBYTE) * (size_t)(bpdb_nowrite_slice->bits) ), sizeof(BYTE));
    if(NULL == bpdb_write_array) {
        status = STATUS_NOT_ENOUGH_MEMORY;
        BPDB_TRACE("bpdb_init()", "Could not allocate bpdb_nowrite_array in memory", status);
    }

    // probably do not need this anymore
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

    bpdb_scheme_list = scheme_list_add( bpdb_scheme_list, 0, NULL, bpdb_mem_write_varnum, FALSE );
    bpdb_scheme_list = scheme_list_add( bpdb_scheme_list, 1, bpdb_generic_read_varnum, bpdb_generic_write_varnum, TRUE );
//    bpdb_scheme_list = scheme_list_add( bpdb_scheme_list, 2, bpdb_scott_read_varnum, bpdb_scott_varnum, TRUE );

_bailout:
    return;
    //return status;
}

void
bpdb_free( )
{
    SAFE_FREE(bpdb_write_array);
}


VALUE
bpdb_set_value(
                POSITION pos,
                VALUE val
                )
{
    bpdb_set_slice_slot( (UINT64)pos, VALUESLOT, (UINT64) val );
    // bpdb_set_slice_slot needs to return val
    return val;
}

VALUE
bpdb_get_value(
                POSITION pos
                )
{
    return (VALUE) bpdb_get_slice_slot( (UINT64)pos, VALUESLOT );
}

REMOTENESS
bpdb_get_remoteness(
                    POSITION pos
                    )
{
    return (REMOTENESS) bpdb_get_slice_slot( (UINT64)pos, REMSLOT );
}

void
bpdb_set_remoteness(
                    POSITION pos,
                    REMOTENESS val
                    )
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

BOOLEAN
bpdb_check_visited(
                POSITION pos
                )
{
    return (BOOLEAN) bpdb_get_slice_slot( (UINT64)pos, VISITEDSLOT );
}

void
bpdb_mark_visited(
                POSITION pos
                )
{
    bpdb_set_slice_slot( (UINT64)pos, VISITEDSLOT, (UINT64)1 );
}

void
bpdb_unmark_visited(
                POSITION pos
                )
{
    bpdb_set_slice_slot( (UINT64)pos, VISITEDSLOT, (UINT64)0 );
}

void
bpdb_set_mex(
                POSITION pos,
                MEX mex
                )
{
    bpdb_set_slice_slot( (UINT64)pos, MEXSLOT, (UINT64)mex );
}

MEX
bpdb_get_mex(
                POSITION pos
                )
{
    return (MEX) bpdb_get_slice_slot( (UINT64)pos, MEXSLOT );
}
inline
void
bpdb_set_slice_slot(
                UINT64 position,
                UINT8 index,
                UINT64 value
                )
{
    UINT64 byteOffset = 0;
    UINT8 bitOffset = 0;
    BYTE *bpdb_array = NULL;
    SLICE bpdb_slice = NULL;

    if(index % 2) {
        bpdb_array = bpdb_nowrite_array;
        bpdb_slice = bpdb_nowrite_slice;
    } else {
        bpdb_array = bpdb_write_array;
        bpdb_slice = bpdb_write_slice;
    }
    index /= 2;

    if(value > bpdb_slice->maxvalues[index]) {
        value = bpdb_slice->maxvalues[index];
        if(!bpdb_slice->overflowed[index]) {
            printf("\nWarning: Slot %s with bit size %u had to be rounded from %llu to its maxvalue %llu.\n",
                                                                        bpdb_slice->names[index],
                                                                        bpdb_slice->sizes[index],
                                                                        value,
                                                                        bpdb_slice->maxvalues[index]
                                                                        );
            bpdb_slice->overflowed[index] = TRUE;
        }
    }

    byteOffset = (bpdb_slice->bits * position)/BITSINBYTE;
    bitOffset = ((UINT8)(bpdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
    bitOffset += bpdb_slice->offsets[index];
    
    byteOffset += bitOffset / BITSINBYTE;
    bitOffset %= BITSINBYTE;

    //printf("byteoff: %d bitoff: %d value: %llu length: %d\n", byteOffset, bitOffset, value, length);
    //printf("value: %llu\n", value);
    bitlib_insert_bits( bpdb_array + byteOffset, bitOffset, value, bpdb_slice->sizes[index] );
}


inline
UINT64
bpdb_get_slice_slot(
                UINT64 position,
                UINT8 index
                )
{
    UINT64 byteOffset = 0;
    UINT8 bitOffset = 0;
    BYTE *bpdb_array = NULL;
    SLICE bpdb_slice = NULL;

    if(index % 2) {
        bpdb_array = bpdb_nowrite_array;
        bpdb_slice = bpdb_nowrite_slice;
    } else {
        bpdb_array = bpdb_write_array;
        bpdb_slice = bpdb_write_slice;
    }
    index /= 2;

    byteOffset = (bpdb_slice->bits * position)/BITSINBYTE;
    bitOffset = ((UINT8)(bpdb_slice->bits % BITSINBYTE) * (UINT8)(position % BITSINBYTE)) % BITSINBYTE;
    bitOffset += bpdb_slice->offsets[index];
    
    byteOffset += bitOffset / BITSINBYTE;
    bitOffset %= BITSINBYTE;

    return bitlib_read_bits( bpdb_array + byteOffset, bitOffset, bpdb_slice->sizes[index] );
}


GMSTATUS
bpdb_add_slot(
                UINT8 size,
                char *name,
                BOOLEAN write
                )
{
    GMSTATUS status = STATUS_SUCCESS;    
    SLICE bpdb_slice = NULL;

    if(NULL == name) {
        status = STATUS_INVALID_INPUT_PARAMETER;
        BPDB_TRACE("bpdb_add_slot()", "name is passed in as null", status);
        goto _bailout;
    }

    if(NULL == bpdb_write_slice) {
        status = STATUS_INVALID_INPUT_PARAMETER;
        BPDB_TRACE("bpdb_add_slot()", "bpdb_write_slice must be initialized first. call bpdb_init()", status);
        goto _bailout;
    }

    if(NULL == bpdb_nowrite_slice) {
        status = STATUS_INVALID_INPUT_PARAMETER;
        BPDB_TRACE("bpdb_add_slot()", "bpdb_nowrite_slice must be initialized first. call bpdb_init()", status);
        goto _bailout;
    }

    if(write) {
        bpdb_slice = bpdb_write_slice;
    } else {
        bpdb_slice = bpdb_nowrite_slice;
    }

    bpdb_slice->slots++;
    if(bpdb_slice->slots == 1) {
        bpdb_slice->sizes = (UINT8 *) calloc( 1, sizeof(UINT8) );
        bpdb_slice->offsets = (UINT32 *) calloc( 1, sizeof(UINT32) );
        bpdb_slice->maxvalues = (UINT64 *) calloc( 1, sizeof(UINT64) );
        bpdb_slice->names = (char **) calloc( 1, sizeof(char **) );
        bpdb_slice->overflowed = (BOOLEAN *) calloc( 1, sizeof(BOOLEAN) );
    } else {
        bpdb_slice->sizes = (UINT8 *) realloc( bpdb_slice->sizes, bpdb_slice->slots*sizeof(UINT8) );
        bpdb_slice->offsets = (UINT32 *) realloc( bpdb_slice->offsets, bpdb_slice->slots*sizeof(UINT32) );
        bpdb_slice->maxvalues = (UINT64 *) realloc( bpdb_slice->maxvalues, bpdb_slice->slots*sizeof(UINT64) );
        bpdb_slice->names = (char **) realloc( bpdb_slice->names, bpdb_slice->slots*sizeof(char **) );
        bpdb_slice->overflowed = (BOOLEAN *) realloc( bpdb_slice->overflowed, bpdb_slice->slots*sizeof(BOOLEAN) );
    }

    bpdb_slice->bits += size;
    bpdb_slice->sizes[bpdb_slice->slots-1] = size;
    if(bpdb_slice->slots == 1) {
        bpdb_slice->offsets[bpdb_slice->slots-1] = 0;
    } else {
        bpdb_slice->offsets[bpdb_slice->slots-1] = bpdb_slice->offsets[bpdb_slice->slots - 2] + bpdb_slice->sizes[bpdb_slice->slots - 2];
    }
    bpdb_slice->maxvalues[bpdb_slice->slots-1] = (UINT64)pow(2, size) - 1;
    bpdb_slice->names[bpdb_slice->slots-1] = (char *) malloc( strlen(name) + 1 );
    strcpy( bpdb_slice->names[bpdb_slice->slots-1], name );
    bpdb_slice->overflowed[bpdb_slice->slots-1] = FALSE;

_bailout:
    return status;
}


//
//

void
bpdb_print_database()
{
    UINT64 i = 0;
    UINT8 j = 0;

    printf("\n\nDatabase Printout\n");

    for(i = 0; i < bpdb_slices; i++) {
        printf("Slice %llu  (write) ", i);
        for(j=0; j < (bpdb_write_slice->slots); j++) {
            printf("%s: %llu  ", bpdb_write_slice->names[j], bpdb_get_slice_slot(i, j*2));
        }
        printf("(no write) ");
        for(j=0; j < (bpdb_nowrite_slice->slots); j++) {
            printf("%s: %llu  ", bpdb_nowrite_slice->names[j], bpdb_get_slice_slot(i, j*2+1));
        }
        printf("\n");
    }
}


BOOLEAN
bpdb_save_database()
{
    GMSTATUS status = STATUS_SUCCESS;

    // counter
    int i;
    Scheme_List cur;

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
    cur = bpdb_scheme_list;

    // must free this
    for(i = 0; i<scheme_list_size(bpdb_scheme_list); i++) {
        outfilenames = (char **) malloc( scheme_list_size(bpdb_scheme_list)*sizeof(char*) );
    }
    for(i = 0; i<scheme_list_size(bpdb_scheme_list); i++) {
        outfilenames[i] = (char *) malloc( 256*sizeof(char) );
    }

    i = 0;

    printf("\n");

    // save file under each encoding schemeset_
    while(cur != NULL) {
        // saves with encoding scheme and returns filename
        //sprintf(outfilenames[i], "./data/m%s_%d_bpdb_%d.dat.gz", "test", 1, cur->scheme);
        sprintf(outfilenames[i], "./data/m%s_%d_bpdb_%d.dat.gz", kDBName, getOption(), cur->scheme);

        status = bpdb_generic_save_database( cur, outfilenames[i] );
        if(!GMSUCCESS(status)) {
            BPDB_TRACE("bpdb_save_database()", "call to bpdb_generic_save_database with scheme failed", status);
        } else {
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
    for(i = 0; i < scheme_list_size(bpdb_scheme_list); i++) {
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

    if(!GMSUCCESS(status)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOLEAN
bpdb_generic_write_varnum(
                dbFILE *outFile,
                BYTE *outputBuffer,
                UINT8 *offset,
                UINT64 consecutiveSkips
                )
{
    UINT8 leftBits, rightBits;
    
    leftBits = bpdb_generic_varnum_gap_bits( consecutiveSkips );
    rightBits = leftBits;

    bitlib_value_to_buffer( outFile, outputBuffer, offset, bitlib_right_mask64( leftBits), leftBits );
    bitlib_value_to_buffer( outFile, outputBuffer, offset, 0, 1 );

    consecutiveSkips -= bpdb_generic_varnum_implicit_amt( leftBits );

    bitlib_value_to_buffer( outFile, outputBuffer, offset, consecutiveSkips, rightBits );

    return TRUE;
}

UINT8
bpdb_generic_varnum_gap_bits(
                UINT64 consecutiveSkips
                )
{
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

UINT64
bpdb_generic_varnum_implicit_amt(
                UINT8 leftBits
                )
{
    if(64 == leftBits) {
        return UINT64_MAX;
    } else {
        return (1 << leftBits) - 1;
    }
}

GMSTATUS
bpdb_generic_save_database(
                Scheme_List scheme,
                char *outfilename
                )
{
    GMSTATUS status = STATUS_SUCCESS;

    UINT64 consecutiveSkips = 0;
    BYTE outputBuffer = 0;
    UINT8 offset = 0;
    UINT8 i, j;

    // gzip file ptr
    dbFILE *outFile = NULL;

    UINT64 slice;
    UINT8 slot;

    mkdir("data", 0755) ;

    status = bitlib_file_open(outfilename, "wb", &outFile);
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_generic_save_database()", "call to bitlib to open file failed", status);
        goto _bailout;
    }

    bitlib_value_to_buffer ( outFile, &outputBuffer, &offset, scheme->scheme, 8 );

    bpdb_generic_write_varnum( outFile, &outputBuffer, &offset, bpdb_slices );
    bpdb_generic_write_varnum( outFile, &outputBuffer, &offset, bpdb_write_slice->bits );

    bpdb_generic_write_varnum( outFile, &outputBuffer, &offset, bpdb_write_slice->slots );

    for(i = 0; i < (bpdb_write_slice->slots); i++) {
        bpdb_generic_write_varnum( outFile, &outputBuffer, &offset, bpdb_write_slice->sizes[i] );
        bpdb_generic_write_varnum( outFile, &outputBuffer, &offset, strlen(bpdb_write_slice->names[i]) );
        
        for(j = 0; j<strlen(bpdb_write_slice->names[i]); j++) {
            bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_write_slice->names[i][j], 8 );
        }

        bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_write_slice->overflowed[i], 1 );
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
                for(slot=0; slot < (bpdb_write_slice->slots); slot++) {
                    bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_get_slice_slot(slice, 2*slot), bpdb_write_slice->sizes[slot] );
                }

            } else {
                consecutiveSkips++;
            }
        }
    } else {
        // Loop through all records
        for(slice=0; slice<bpdb_slices; slice++) {
            //bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_get_slice_full(slice), bpdb_slice->bits );
            for(slot=0; slot < (bpdb_write_slice->slots); slot++) {
                bitlib_value_to_buffer( outFile, &outputBuffer, &offset, bpdb_get_slice_slot(slice, 2*slot), bpdb_write_slice->sizes[slot] );
            }
        }
    }


    if(consecutiveSkips != 0) {
        scheme->write_varnum(outFile, &outputBuffer, &offset, consecutiveSkips);
        consecutiveSkips = 0;
    }

    if(offset != 0) {
        bitlib_file_write_byte(outFile, &outputBuffer);
    }

    status = bitlib_file_close(outFile);
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_generic_save_database()", "call to bitlib to close file failed", status);
        goto _bailout;
    }

_bailout:
    return status;
}



BOOLEAN
bpdb_load_database( )
{
    GMSTATUS status = STATUS_SUCCESS;
    Scheme_List cur;

    // filename
    char outfilename[256];
  
    dbFILE *inFile = NULL;
    FILE *testOpen = NULL;

    // file information
    UINT8 fileFormat;

    // open file
    sprintf(outfilename, "./data/m%s_%d_bpdb.dat.gz", kDBName, getOption());

    if(NULL != (testOpen = fopen(outfilename, "r"))) {
        fclose(testOpen);

        status = bitlib_file_open(outfilename, "rb", &inFile);
        if(!GMSUCCESS(status)) {
            BPDB_TRACE("bpdb_load_database()", "call to bitlib to open file failed", status);
            goto _bailout;
        }
    } else {
        return FALSE;
    }
    
    // read file header
    fileFormat = bitlib_file_read_byte( inFile );

    // print fileinfo
    printf("\nEncoding Scheme: %d\n", fileFormat);
    
    cur = bpdb_scheme_list;

    while(cur->scheme != fileFormat) {
        cur = cur->next;
    }
    
    status = bpdb_generic_load_database( inFile, cur );
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_load_database()", "call to bpdb_generic_load_database to load db with recognized scheme failed", status);
        goto _bailout;
    }

    // close file
    status = bitlib_file_close(inFile);
    if(!GMSUCCESS(status)) {
        BPDB_TRACE("bpdb_load_database()", "call to bitlib to close file failed", status);
        goto _bailout;
    }

_bailout:
    if(!GMSUCCESS(status)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

GMSTATUS
bpdb_generic_load_database(
                dbFILE *inFile,
                Scheme_List scheme
                )
{
    GMSTATUS status = STATUS_SUCCESS;

    UINT64 currentSlice = 0;
    UINT8 currentSlot = 0;

    UINT64 numOfSlicesHeader = 0;
    UINT8 bitsPerSliceHeader = 0;
    UINT8 numOfSlotsHeader = 0;
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

    if( bpdb_write_slice->slots ) {
        slotsMade = TRUE;
    }

    //
    // outputs slots in db
    // perhaps think about what happens if a slot is missing
    //

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

        // read overflowed bit
        if(bitlib_read_from_buffer( inFile, &inputBuffer, &offset, 1 )) {
            printf("\tWarning: Slot contains values that have been capped due to overflow.\n");
        }
    }


    if( scheme->indicator ) {
        while(currentSlice < numOfSlicesHeader) {
            if(bitlib_read_from_buffer( inFile, &inputBuffer, &offset, 1 ) == 0) {
        
                for(currentSlot = 0; currentSlot < (bpdb_write_slice->slots); currentSlot++) {
                    bpdb_set_slice_slot( currentSlice, 2*currentSlot,
                                bitlib_read_from_buffer( inFile, &inputBuffer, &offset, bpdb_write_slice->sizes[currentSlot]) );
                }

                currentSlice++;
            } else {
                
                skips = scheme->read_varnum( inFile, &inputBuffer, &offset, TRUE );

                for(i = 0; i < skips; i++, currentSlice++) {
                    bpdb_set_slice_slot( currentSlice, 0, undecided );
                }
            }
        }
    } else {
        for( currentSlice = 0; currentSlice < numOfSlicesHeader; currentSlice++ ) {
            for(currentSlot = 0; currentSlot < (bpdb_write_slice->slots); currentSlot++) {
                bpdb_set_slice_slot( currentSlice, 2*currentSlot,
                            bitlib_read_from_buffer( inFile, &inputBuffer, &offset, bpdb_write_slice->sizes[currentSlot]) );
            }
        }
    }

    return status;
}

UINT64
bpdb_generic_read_varnum(
                dbFILE *inFile,
                BYTE *inputBuffer,
                UINT8 *offset,
                BOOLEAN alreadyReadFirstBit
                )
{
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

UINT8
bpdb_generic_read_varnum_consecutive_ones(
                dbFILE *inFile,
                BYTE *inputBuffer,
                UINT8 *offset,
                BOOLEAN alreadyReadFirstBit
                )
{
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
