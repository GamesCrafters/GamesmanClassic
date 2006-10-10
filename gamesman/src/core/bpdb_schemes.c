#include "bpdb_schemes.h"
#include "bpdb_bitlib.h"

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
