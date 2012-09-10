#ifndef GMCORE_UNIVDB_H
#define GMCORE_UNIVDB_H

#include "db.h"
#include "gamesman.h"

typedef struct univdb_entry {

	POSITION position;
	VALUE flags;
	struct univdb_entry *chain;

} univdb_entry;

DB_Table *univdb_init();

void univdb_free();

VALUE univdb_get_value (POSITION position);
VALUE univdb_put_value (POSITION position, VALUE value);

REMOTENESS univdb_get_remoteness (POSITION position);
void univdb_put_remoteness (POSITION position, REMOTENESS remoteness);

BOOLEAN univdb_check_visited (POSITION position);
void univdb_mark_visited (POSITION position);
void univdb_unmark_visited (POSITION position);

MEX univdb_get_mex (POSITION position);
void univdb_put_mex (POSITION position, MEX mex);
BOOLEAN univdb_save_database();
BOOLEAN univdb_load_database();

#endif
