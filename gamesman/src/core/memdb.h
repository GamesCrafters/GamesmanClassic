#ifndef GMCORE_MEMDB_H
#define GMCORE_MEMDB_H

#define FILEVER 1

typedef short cellValue;


/* General */
void       	memdb_init 		(DB_Table *new_db);
void       	memdb_free 		();

/* Value */
VALUE		memdb_get_value	        (POSITION pos);
VALUE		memdb_set_value	        (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	memdb_get_remoteness	(POSITION pos);
void		memdb_set_remoteness	(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		memdb_check_visited    	(POSITION pos);
void		memdb_mark_visited     	(POSITION pos);
void		memdb_unmark_visited	(POSITION pos);

/* Mex */
MEX		memdb_get_mex		(POSITION pos);
void		memdb_set_mex		(POSITION pos, MEX mex);

/* saving to/reading from a file */
BOOLEAN		memdb_save_database	();
BOOLEAN		memdb_load_database	();

#endif /* GMCORE_MEMDB_H */
