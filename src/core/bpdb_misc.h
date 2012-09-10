/************************************************************************
**
** NAME:    bpdb_misc.h
**
** DESCRIPTION:    Accessor functions for miscellaneous functions used in
**                 the Bit-Perfect Database
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

#ifndef GMCORE_BPDB_MISC_H
#define GMCORE_BPDB_MISC_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include "types.h"
#include "gamesman.h"

typedef gzFile dbFILE;
/*
   typedef unsigned char BYTE;
   typedef unsigned char UINT8;
   typedef unsigned int UINT32;
   typedef long long int INT64;
   typedef unsigned long long int UINT64;

   typedef UINT32 GMSTATUS;
 */


#define BITSINBYTE 8
#define BITSINPOS 64

#define SAFE_FREE(ptr) \
	if(NULL != ptr) \
		free(ptr);

#define BPDB_TRACE(fnc, msg, err) \
	fprintf(stderr, "\nERROR CODE 0x%04x : %s : %s\n", (err), (fnc), (msg));

#define STATUS_SUCCESS                      0x0
#define STATUS_NOT_ENOUGH_MEMORY            0x1
#define STATUS_INVALID_INPUT_PARAMETER      0x2

#define STATUS_FILE_COULD_NOT_BE_OPENED     0x3
#define STATUS_FILE_COULD_NOT_BE_CLOSED     0x4
#define STATUS_BAD_COMPRESSION              0x5
#define STATUS_BAD_DECOMPRESSION            0x6
#define STATUS_FILE_COULD_NOT_BE_SEEKED         0x7

#define STATUS_MISSING_DEPENDENT_MODULE     0x10
#define STATUS_NO_SCHEMES_INSTALLED         0x11
#define STATUS_SLICE_FORMAT_NOT_SET         0x12
#define STATUS_SCHEME_NOT_FOUND             0x13

#define STATUS_INVALID_OPERATION            0x99

#define GMSUCCESS(status) \
	(STATUS_SUCCESS == (status))

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))


//
// structure of a singly-linked list
//

typedef struct singlylinkedlist {

	// object stored in the linked list
	void            *obj;

	// pointer to the next node
	struct singlylinkedlist *next;
} *SLIST;

SLIST
slist_new( );

SLIST
slist_add(
        SLIST sl,
        void *obj
        );

UINT32
slist_size(
        SLIST sl
        );

void
slist_free(
        SLIST sl
        );


//
// structure of a singly-linked list
//

typedef struct hashtable_singlylinkedlist {

	UINT32 key;
	UINT32 value;

	// pointer to the next node
	struct hashtable_singlylinkedlist *next;
} *HTABLE_SLIST;

HTABLE_SLIST
htable_slist_new( );

HTABLE_SLIST
htable_slist_add(
        HTABLE_SLIST sl,
        UINT32 key,
        UINT32 value
        );

UINT32
htable_slist_size(
        HTABLE_SLIST sl
        );

void
htable_slist_free(
        HTABLE_SLIST sl
        );


typedef struct hashtable {
	UINT32 size;
	HTABLE_SLIST *buckets;
} *HTABLE;

HTABLE
htable_new(
        UINT32 size
        );

void
htable_add(
        HTABLE ht,
        UINT32 key,
        UINT32 value
        );

void
htable_set(
        HTABLE ht,
        UINT32 key,
        UINT32 value
        );

BOOLEAN
htable_contains(
        HTABLE ht,
        UINT32 key
        );

UINT32
htable_get(
        HTABLE ht,
        UINT32 key
        );

void
htable_free(
        HTABLE ht
        );

#endif /* GMCORE_BPDB_MISC_H */
