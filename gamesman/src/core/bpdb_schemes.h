#ifndef GMCORE_BPDB_SCHEMES_H
#define GMCORE_BPDB_SCHEMES_H

#include "bpdb_misc.h"
#include "bpdb.h"

UINT64		bpdb_mem_read_varnum		( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );
BOOLEAN		bpdb_mem_write_varnum		( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips );

UINT64		bpdb_scott_read_varnum		( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );
BOOLEAN		bpdb_scott_write_varnum		( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips );

#endif /* GMCORE_BPDB_SCHEME_H */
