#ifndef GMCORE_COLLDB_H
#define GMCORE_COLLDB_H

/* General */
DB_Table*       colldb_init  ();
void            colldb_free ();

/* Value */
VALUE		colldb_get_value	        (POSITION pos);
VALUE		colldb_set_value	        (POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	colldb_get_remoteness	(POSITION pos);
void		colldb_set_remoteness	(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		colldb_check_visited    	(POSITION pos);
void		colldb_mark_visited     	(POSITION pos);
void		colldb_unmark_visited	(POSITION pos);

/* Collision Specific */
typedef struct colldb_node {

  VALUE myValue; 
  POSITION myPos;
  struct colldb_node *next;

} colldb_value_node;

colldb_value_node *colldb_find_pos(POSITION position);
colldb_value_node* colldb_make_node(POSITION pos, VALUE val, colldb_value_node *next);

colldb_value_node *colldb_add_node(POSITION position);


/* Mex not implemented yet. will implement once db class implemented
void		MexStore		(POSITION pos, MEX mex);
MEX		MexLoad			(POSITION pos);
*/

#endif /* GMCORE_COLLDB_H */
