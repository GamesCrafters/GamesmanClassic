#ifndef GMCORE_BPDB_MISC_H
#define GMCORE_BPDB_MISC_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include "gamesman.h"

typedef gzFile dbFILE;

typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef long long int INT64;
typedef unsigned long long int UINT64;

typedef UINT32 GMSTATUS;

#define BITSINBYTE 8
#define BITSINPOS 64

#define SAFE_FREE(ptr) \
        if(NULL != ptr) \
            free(ptr);

#define BPDB_TRACE(fnc, msg, err) \
        fprintf(stderr, "ERROR CODE 0x%04x : %s : %s\n", (err), (fnc), (msg));

#define STATUS_SUCCESS 0x0
#define STATUS_NOT_ENOUGH_MEMORY 0x1
#define STATUS_INVALID_INPUT_PARAMETER 0x2

#define STATUS_FILE_COULD_NOT_BE_OPENED 0x3
#define STATUS_FILE_COULD_NOT_BE_CLOSED 0x4

#define GMSUCCESS(status) \
        (STATUS_SUCCESS == (status))

#define VALUESLOT 0
#define MEXSLOT 2
#define REMSLOT 4
#define VISITEDSLOT 1

/* List structure for schemes */
typedef struct Schemelist {
    // numeric identifier
    int            scheme;

    // pointers to load and save functions
    UINT64        (*read_varnum)    ( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );
    BOOLEAN        (*write_varnum)    ( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips );

    BOOLEAN        indicator;

    // next scheme
    struct Schemelist *next;
} *Scheme_List;

Scheme_List scheme_list_new();
Scheme_List scheme_list_add(Scheme_List sl, int schemenum, UINT64 (*read_varnum)( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit ), BOOLEAN (*write_varnum)( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips ), BOOLEAN indicator);
UINT8 scheme_list_size( Scheme_List sl );

#endif /* GMCORE_BPDB_MISC_H */
