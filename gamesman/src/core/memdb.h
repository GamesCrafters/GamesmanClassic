#ifndef GMCORE_MEMDB_H
#define GMCORE_MEMDB_H

/* General */
DB_Table*       memdb_init  ();
void            memdb_free ();

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

/* Mex not implemented yet. will implement once db class implemented
void		MexStore		(POSITION pos, MEX mex);
MEX		MexLoad			(POSITION pos);
*/

#endif /* GMCORE_MEMDB_H */
