#ifndef GMCORE_BPDB_H
#define GMCORE_BPDB_H

#include <zlib.h>
#include "gamesman.h"

#define BITSINBYTE 8
typedef unsigned char BYTE;
typedef short cellValue;

/* General */
extern		cellValue*	bpdb_array;

void		bpdb_init(DB_Table *new_db);
int			bpdb_write_byte( gzFile *outFile, BYTE r );
BYTE		bpdb_read_byte( gzFile *inFile );

VALUE bpdb_set_value(POSITION pos, VALUE val);

#endif /* GMCORE_BPDB_H */
