
#ifndef GM_DB_H
#define GM_DB_H

void		db_init		( void );
void		db_load		( DATABASE_CLASS* );
DATABASE_CLASS*	db_get		( STRING );
DATABASE_CLASS*	db_find_handler	( STRING );
DATABASE*	db_open		( STRING );

#endif /* GM_DB_H */
