#ifndef GMCORE_BPDB_SCHEMES_H
#define GMCORE_BPDB_SCHEMES_H

#include <zlib.h>

BOOLEAN	bpdb_save_database_mem( int schemeversion, char *outfilename );
BOOLEAN	bpdb_load_database_mem( gzFile *filep );

BOOLEAN bpdb_save_database_dan( int schemeversion, char *outfilename );
BOOLEAN bpdb_load_database_dan( gzFile *inFile );

BOOLEAN bpdb_save_database_scott( int schemeversion, char *outfilename );
BOOLEAN bpdb_load_database_scott( gzFile *inFile );

BOOLEAN bpdb_save_database_ken( int schemeversion, char *outfilename );
BOOLEAN bpdb_load_database_ken( gzFile *inFile );

#endif /* GMCORE_BPDB_SCHEME_H */
