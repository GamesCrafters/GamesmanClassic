#ifndef GMCORE_TWOBITDB_H
#define GMCORE_TWOBITDB_H



/* General */
DB_Table*       twobitdb_init ();
void            twobitdb_free ();

/* Value */
VALUE		twobitdb_get_value	(POSITION pos);
VALUE		twobitdb_set_value	(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	twobitdb_get_remoteness	(POSITION pos);
void		twobitdb_set_remoteness	(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		twobitdb_check_visited   	(POSITION pos);
void		twobitdb_mark_visited    	(POSITION pos);
void		twobitdb_unmark_visited	(POSITION pos);

#endif /* GMCORE_TWOBITDB_H */
