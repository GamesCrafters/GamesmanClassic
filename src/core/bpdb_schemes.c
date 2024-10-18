/************************************************************************
**
** NAME:    bpdb_schemes.c
**
** DESCRIPTION:    Implementation of encoding and decoding schemes for
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

#include "bpdb_schemes.h"
#include "bpdb_bitlib.h"

SCHEME scheme_new(
        UINT32 id,
        UINT8 (*varnum_gap_bits)( UINT64 consecutiveSkips ),
        UINT8 (*varnum_size_bits)( UINT8 leftBits ),
        UINT64 (*varnum_implicit_amt)( UINT8 leftBits ),
        void (*varnum_init)( ),
        void (*varnum_free)( ),
        BOOLEAN indicator,
        BOOLEAN save
        )
{
	SCHEME s = (SCHEME) malloc( sizeof(struct dbscheme) );
	s->id = id;
	s->varnum_gap_bits = varnum_gap_bits;
	s->varnum_size_bits = varnum_size_bits;
	s->varnum_implicit_amt = varnum_implicit_amt;
	s->varnum_free = varnum_free;
	s->indicator = indicator;
	s->save = save;

	if(NULL != varnum_init) {
		varnum_init();
	}

	return s;
}

void
scheme_free(
        SCHEME s
        )
{
	if( NULL != s->varnum_free) {
		s->varnum_free();
	}
	SAFE_FREE( s );
}

// possible limitation on the number of skips that can be encoded
UINT64 *bpdb_generic_varnum_precomputed_gap_bits;

void
bpdb_generic_varnum_init( ) {
	int i = 0;
	bpdb_generic_varnum_precomputed_gap_bits = (UINT64 *) malloc( 32 * sizeof(UINT64) );

	bpdb_generic_varnum_precomputed_gap_bits[0] = 3;

	for(i=1; i<32; i++) {
		bpdb_generic_varnum_precomputed_gap_bits[i] = (UINT64) bpdb_generic_varnum_precomputed_gap_bits[i-1] + ((UINT64)1<<(i+1));
	}
}

void
bpdb_generic_varnum_free( ) {
	SAFE_FREE(bpdb_generic_varnum_precomputed_gap_bits);
}

UINT8
bpdb_generic_varnum_gap_bits(
        UINT64 consecutiveSkips
        )
{
	int i = 0;
	for(i=0; i<31; i++) {
		if(bpdb_generic_varnum_precomputed_gap_bits[i] > consecutiveSkips) {
			return i+1;
		}
	}

	return 255;
}

UINT8
bpdb_generic_varnum_size_bits( UINT8 leftBits ) {
	return leftBits;
}

UINT64
bpdb_generic_varnum_implicit_amt(
        UINT8 leftBits
        )
{
	// for completeness we should have the 64 == leftBits
	// check, but for all practical purposes, there is no need
	// for it

	return (1 << leftBits) - 1;
}



// possible limitation on the number of skips that can be encoded

UINT8
bpdb_ken_varnum_gap_bits(
        UINT64 consecutiveSkips
        )
{
	UINT8 leftBits = 1;
	UINT8 powerTo = 4;
	UINT64 skipsRepresented = 4;

	while(skipsRepresented < consecutiveSkips)
	{
		skipsRepresented += (UINT64)pow(2, powerTo);
		leftBits++;
		powerTo += 2;
	}

	return leftBits;
}

UINT64
bpdb_ken_varnum_implicit_amt(
        UINT8 leftBits
        )
{
	UINT64 amt = 1;
	UINT64 bits = 2;

	while(leftBits > 1) {
		amt += (UINT64) pow(2, bits);
		bits += 2;
		leftBits--;
	}

	return amt;
}

UINT8
bpdb_ken_varnum_size_bits( UINT8 leftBits ) {
	return leftBits*2;
}
