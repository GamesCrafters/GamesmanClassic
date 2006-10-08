#ifndef GMCORE_BPDB_SCHEMES_H
#define GMCORE_BPDB_SCHEMES_H

#include "bpdb_misc.h"
#include "bpdb.h"

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
