
/*
** Database repository
** John Jordan (jjjordan@berkeley.edu)
*/

#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "gamesman.h"
#include "db_local.h"


/*
** Database table
*/

static TABLE	DB_LIST = NULL;


/*
** Code
*/

void db_init ( void )
{
	/* First one is default */
	db_load(memdb_init());
}

void db_load ( DATABASE_CLASS* dbc )
{
	if (!dbc)
		return;
	
	if (!DB_LIST)
		table_put_voidp(&DB_LIST, "default", dbc, NULL);
	
	table_put_voidp(&DB_LIST, dbc -> class_name, dbc, NULL);
}

DATABASE_CLASS* db_get ( STRING class_name )
{
	void*	ret;
	
	if (table_get_voidp(DB_LIST, class_name, &ret))
		return NULL;
	
	return (DATABASE_CLASS*) ret;
}

DATABASE_CLASS* db_find_handler ( STRING resource )
{
	table_iter	i;
	table_value*	ptr;
	char*		r;
	size_t		h_len;
	int		n;
	
	if (!(r = index(resource, ':')))
		return db_get("default");
	
	h_len = r - resource;
	
	table_iter_new(&i, DB_LIST);
	
	for (n = table_iter_new(&i, DB_LIST); !n; n = table_iter_next(&i)) {
		if (!(ptr = table_iter_value(&i)))
			return NULL;
		
		if (!strncasecmp(table_iter_key(&i), resource, h_len))
			return ptr -> value . ptr;
	}
	
	return NULL;
}

DATABASE* db_open ( STRING resource )
{
	DATABASE_CLASS*	c;
	
	if (!(c = db_find_handler(resource)))
		return NULL;
	
	return (*c -> create)(resource);
}
