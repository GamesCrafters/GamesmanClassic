#include "bpdb_schemes.h"
#include "bpdb_bitlib.h"


// possible limitation on the number of skips that can be encoded
UINT64 *bpdb_generic_varnum_precomputed_gap_bits;

void
bpdb_generic_varnum_init( ) {
    int i = 0;
    bpdb_generic_varnum_precomputed_gap_bits = (UINT64 *) malloc( 31 * sizeof(UINT64) );

    bpdb_generic_varnum_precomputed_gap_bits[0] = 3;

    for(i=1; i<64; i++) {
        bpdb_generic_varnum_precomputed_gap_bits[i] = bpdb_generic_varnum_precomputed_gap_bits[i-1] + (1<<(i+1));
        //printf("%d %llu\n", i, bpdb_generic_varnum_precomputed_gap_bits[i]);
    }
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

    //if(64 == leftBits) {
    //    return UINT64_MAX;
    //} else {
        return (1 << leftBits) - 1;
    //}
}


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
