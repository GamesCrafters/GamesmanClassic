
#ifndef GM_MISC_H
#define GM_MISC_H

void*		safe_malloc		( size_t );
void		safe_free		( void* );
char*		safe_strdup		( char* );
void		die			( const char*, ... );

int		table_get		( TABLE, STRING, table_value* );
int		table_get_string	( TABLE, STRING, STRING* );
int		table_get_voidp		( TABLE, STRING, void** );
int		table_get_int		( TABLE, STRING, int* );
int		table_get_position	( TABLE, STRING, POSITION* );
int		table_put		( TABLE*, STRING, table_value* );
int		table_put_string	( TABLE*, STRING, STRING );
int		table_put_voidp		( TABLE*, STRING, void*, void (*)(void*) );
int		table_put_int		( TABLE*, STRING, int );
int		table_put_position	( TABLE*, STRING, POSITION );
void		table_free		( TABLE* );

void		table_iter_new		( table_iter*, TABLE );
STRING		table_iter_key		( table_iter* );
table_value*	table_iter_value	( table_iter* );
int		table_iter_next		( table_iter* );

void		positionlist_free	( POSITIONLIST* );
POSITIONLIST*	positionlist_new_node	( POSITIONLIST*, POSITION );
POSITIONLIST*	positionlist_copy	( POSITIONLIST* );

void		movelist_free		( MOVELIST* );
MOVELIST*	movelist_new_node	( MOVELIST*, MOVE );
MOVELIST*	movelist_copy		( MOVELIST* );

void		stringlist_free		( STRINGLIST* );
STRINGLIST*	stringlist_new_node	( STRINGLIST*, STRING );

#endif

