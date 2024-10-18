/************************************************************************
**
** NAME:    bpdb_schemes.h
**
** DESCRIPTION:    Accessor functions for encoding and decoding schemes
**                 used in the Bit-Perfect Database
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

#ifndef GMCORE_BPDB_SCHEMES_H
#define GMCORE_BPDB_SCHEMES_H

//#include "bpdb.h"
#include "bpdb_misc.h"

typedef struct  dbscheme {
	// numeric identifier
	UINT32 id;

	// functional pointer to calculate the number of consecutive 1s
	// needed to encode the number of skipped slices
	UINT8 (*varnum_gap_bits)( UINT64 consecutiveSkips );

	// functional pointer to calculate the number of bits that are used
	// to encode the size of the gap
	UINT8 (*varnum_size_bits)( UINT8 leftBits );

	// functional pointer to calculate the number of skipped slices
	// implicit in the number of bits used to encode the size of the gap
	UINT64 (*varnum_implicit_amt)( UINT8 leftBits );

	void (*varnum_free)( );

	// whether or not this scheme performs variable-skips encoding
	BOOLEAN indicator;

	// whether or not this scheme should be used for encoding dbs
	BOOLEAN save;

} *SCHEME;

SCHEME
scheme_new(
        UINT32 id,
        UINT8 (*varnum_gap_bits)( UINT64 consecutiveSkips ),
        UINT8 (*varnum_size_bits)( UINT8 leftBits ),
        UINT64 (*varnum_implicit_amt)( UINT8 leftBits ),
        void (*varnum_init)( ),
        void (*varnum_free)( ),
        BOOLEAN indicator,
        BOOLEAN save
        );

void
scheme_free(
        SCHEME s
        );

extern
UINT64 *bpdb_generic_varnum_precomputed_gap_bits;

void
bpdb_generic_varnum_init( );

void
bpdb_generic_varnum_free( );

UINT8
bpdb_generic_varnum_gap_bits(
        UINT64 consecutiveSkips
        );

UINT64
bpdb_generic_varnum_implicit_amt(
        UINT8 leftBits
        );

UINT8
bpdb_generic_varnum_size_bits(
        UINT8 leftBits
        );

UINT8
bpdb_ken_varnum_gap_bits(
        UINT64 consecutiveSkips
        );

UINT64
bpdb_ken_varnum_implicit_amt(
        UINT8 leftBits
        );

UINT8
bpdb_ken_varnum_size_bits(
        UINT8 leftBits
        );

#endif /* GMCORE_BPDB_SCHEME_H */
