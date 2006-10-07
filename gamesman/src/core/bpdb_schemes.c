#include "bpdb_schemes.h"
#include "bpdb_bitlib.h"

//UINT64 bpdb_mem_read_varnum( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit ) {
//}

BOOLEAN bpdb_mem_write_varnum( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips ) {
    /*int i;

    for(i = 0; i<consecutiveSkips; i++) {
        bitlib_value_to_buffer( outFile, outputBuffer, offset, undecided, bpdb_bits_per_slice );
    }
*/
    return TRUE;
}


BOOLEAN bpdb_scott_varnum( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips );
UINT8 bpdb_scott_varnum_gap_bits( UINT64 consecutiveSkips );
UINT64 bpdb_scott_varnum_implicit_amt( UINT8 leftBits );
UINT64 bpdb_scott_read_varnum( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );

BOOLEAN bpdb_scott_varnum( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips ) {
    UINT8 leftBits, rightBits;
    
    leftBits = bpdb_scott_varnum_gap_bits( consecutiveSkips );
    rightBits = 2*leftBits;

    // temp - ken change
//    bitlib_value_to_buffer( outFile, outputBuffer, offset, bitlib_right_mask64( leftBits), leftBits );
//    bitlib_value_to_buffer( outFile, outputBuffer, offset, 0, 1 );

    consecutiveSkips -= bpdb_scott_varnum_implicit_amt( leftBits );

    // temp - ken change
    //bitlib_value_to_buffer( outFile, outputBuffer, offset, consecutiveSkips, rightBits );

    return TRUE;
}

UINT8 bpdb_scott_varnum_gap_bits( UINT64 consecutiveSkips ) {
    UINT8 leftbits = 1;
    
    while(!(consecutiveSkips < bpdb_scott_varnum_implicit_amt(leftbits + 1)))
    {
        leftbits++;
    }

    return leftbits;
}

UINT64 bpdb_scott_varnum_implicit_amt( UINT8 leftBits ) {
    UINT8 bits = 1;
    UINT8 power = 2;
    UINT64 amt = 1;
    
    while(bits < leftBits) {
        amt += (UINT64) pow(2, power);
        power += 2;
        bits++;
    }

    return amt;
}


UINT64 bpdb_scott_read_varnum( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit ) {
    UINT8 i;
    UINT64 variableNumber = 0;
    UINT8 leftBits, rightBits;

    leftBits = bpdb_generic_read_varnum_consecutive_ones( inFile, inputBuffer, offset, alreadyReadFirstBit );
    rightBits = 2*leftBits;

    for(i = 0; i < rightBits; i++) {
        variableNumber = variableNumber << 1;
        variableNumber = variableNumber | bitlib_read_from_buffer( inFile, inputBuffer, offset, 1 );
    }

    variableNumber += bpdb_scott_varnum_implicit_amt( leftBits );

    return variableNumber;
}
