#ifndef GMCORE_BPDB_MISC_H
#define GMCORE_BPDB_MISC_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include "types.h"
//#include "gamesman.h"

typedef gzFile dbFILE;
/*
typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef long long int INT64;
typedef unsigned long long int UINT64;

typedef UINT32 GMSTATUS;
*/

// new
//#define BUFFERLENGTH 2

#define BITSINBYTE 8
#define BITSINPOS 64

#define SAFE_FREE(ptr) \
        if(NULL != ptr) \
            free(ptr);

#define BPDB_TRACE(fnc, msg, err) \
        fprintf(stderr, "\nERROR CODE 0x%04x : %s : %s\n", (err), (fnc), (msg));

#define STATUS_SUCCESS                  0x0
#define STATUS_NOT_ENOUGH_MEMORY        0x1
#define STATUS_INVALID_INPUT_PARAMETER  0x2

#define STATUS_FILE_COULD_NOT_BE_OPENED 0x3
#define STATUS_FILE_COULD_NOT_BE_CLOSED 0x4

#define STATUS_MISSING_DEPENDENT_MODULE 0x5

#define GMSUCCESS(status) \
        (STATUS_SUCCESS == (status))

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/*
#define VALUESLOT 0
#define MEXSLOT 2
#define REMSLOT 4
#define VISITEDSLOT 1
*/

typedef struct  dbscheme {
    // numeric identifier
    UINT32          id;

    // pointers to load and save functions
    UINT8           (*varnum_gap_bits)      ( UINT64 consecutiveSkips );

    UINT8           (*varnum_size_bits)     ( UINT8 leftBits );

    UINT64          (*varnum_implicit_amt)  ( UINT8 leftBits );

    BOOLEAN         indicator;

} *SCHEME;

/* List structure for schemes */
typedef struct singlylinkedlist {

    void            *obj;

    // next scheme
    struct singlylinkedlist *next;
} *SLIST;

SCHEME scheme_new(
                UINT32 id,
                UINT8 (*varnum_gap_bits) ( UINT64 consecutiveSkips ),
                UINT8 (*varnum_size_bits) ( UINT8 leftBits ),
                UINT64 (*varnum_implicit_amt) ( UINT8 leftBits ),
                BOOLEAN indicator
                );

//void scheme_free(
//                SCHEME s;
//                );

SLIST slist_new( );

SLIST slist_add(
                SLIST sl,
                void *obj
                );

UINT32 slist_size(
                SLIST sl
                );

#endif /* GMCORE_BPDB_MISC_H */
