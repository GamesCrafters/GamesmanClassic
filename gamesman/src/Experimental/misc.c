
/*
** Miscellaneous Functions
** John Jordan
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "gamesman.h"


/*
** Truly miscellaneous functions
*/

void* safe_malloc ( size_t sz )
{
	void*	ret;
	
	ret = malloc(sz);
	if (!ret)
		die("Memory allocation failure\n");
	
	return ret;
}

void safe_free ( void* ptr )
{
	if (ptr)
		free(ptr);
}

char* safe_strdup ( char* s )
{
	char*	ret;
	size_t	len;
	
	len = strlen(s) + 1;
	
	ret = (char*) safe_malloc(len);
	memcpy(ret, s, len);
	
	return ret;
}

void die ( const char* fmt, ... )
{
	va_list	args;
	
	va_start(args, fmt);
	
	fprintf(stderr, "FATAL: ");
	vfprintf(stderr, fmt, args);
	
	exit(1);
}



/*
** The TABLE datatype
*/

int table_get ( TABLE tab, STRING key, table_value* value )
{
	TABLE		ptr;
	
	for (ptr = tab; ptr; ptr = ptr -> next) {
		if (!strcasecmp(key, ptr -> key)) {
			memcpy(value, &ptr -> value, sizeof(table_value));
			return 0;
		}
	}
	
	return -1;
}

int table_get_string ( TABLE tab, STRING key, STRING* value )
{
	table_value	res;
	int		ret;
	
	if ((ret = table_get(tab, key, &res)))
		return ret;
	
	if (res . type != VAL_STRING)
		return res . type;
	
	*value = res . value . str;
	return 0;
}

int table_get_voidp ( TABLE tab, STRING key, void** ptr )
{
	table_value	res;
	int		ret;
	
	if ((ret = table_get(tab, key, &res)))
		return ret;
	
	if (res . type != VAL_VOIDP)
		return res . type;
	
	*ptr = res . value . ptr;
	return 0;
}

int table_get_position ( TABLE tab, STRING key, POSITION* p )
{
	table_value	res;
	int		ret;
	
	if ((ret = table_get(tab, key, &res)))
		return ret;
	
	if (res . type != VAL_POS)
		return res . type;
	
	*p = res . value . pos;
	return 0;
}

int table_get_int ( TABLE tab, STRING key, int* n )
{
	table_value	res;
	int		ret;
	
	if ((ret = table_get(tab, key, &res)))
		return ret;
	
	if (res . type != VAL_INT)
		return res . type;
	
	*n = res . value . num;
	return 0;
}

int table_put ( TABLE* tab, char* key, table_value* value )
{
	TABLE		ptr;
	
	for (ptr = *tab; ptr; ptr = ptr -> next) {
		if (!strcasecmp(key, ptr -> key)) {
			if (ptr -> value . delete)
				(*ptr -> value . delete)(ptr -> value . value . ptr);
			memcpy(&ptr -> value, value, sizeof(table_value));
			return 0;
		}
	}
	
	ptr = (TABLE) safe_malloc(sizeof(struct table_entry));
	memcpy(&ptr -> value, value, sizeof(table_value));
	ptr -> key = safe_strdup(key);
	ptr -> next = *tab;
	
	*tab = ptr;
	
	return 0;
}

int table_put_string ( TABLE* tab, STRING key, STRING value )
{
	table_value	v;
	
	v . delete = safe_free;
	v . type = VAL_STRING;
	v . value . str = safe_strdup(value);
	
	return table_put(tab, key, &v);
}

int table_put_voidp ( TABLE* tab, STRING key, void* ptr, void (*delete)(void*) )
{
	table_value	v;
	
	v . delete = delete;
	v . type = VAL_VOIDP;
	v . value . ptr = ptr;
	
	return table_put(tab, key, &v);
}

int table_put_int ( TABLE* tab, STRING key, int value )
{
	table_value	v;
	
	v . delete = NULL;
	v . type = VAL_INT;
	v . value . num = value;
	
	return table_put(tab, key, &v);
}

int table_put_position ( TABLE* tab, STRING key, POSITION p )
{
	table_value	v;
	
	v . delete = NULL;
	v . type = VAL_POS;
	v . value . pos = p;
	
	return table_put(tab, key, &v);
}

void table_free ( TABLE* tab )
{
	TABLE	ptr;
	TABLE	next;
	
	for (ptr = *tab; ptr; ptr = next) {
		next = ptr -> next;
		if (ptr -> value . delete)
			(*ptr -> value . delete)(ptr -> value . value . ptr);
		safe_free(ptr);
	}
	
	*tab = NULL;
}



/*
** table_iter data type
*/

void table_iter_new ( table_iter* obj, TABLE tab )
{
	*obj = tab;
}

STRING table_iter_key ( table_iter* obj )
{
	return (obj ? (*obj) -> key : NULL);
}

table_value* table_iter_value ( table_iter* obj )
{
	return (obj ? &(*obj) -> value : NULL);
}

int table_iter_next ( table_iter* obj )
{
	*obj = (*obj) -> next;
	
	return !!(*obj);
}



/*
** POSITIONLIST data type
*/

void positionlist_free ( POSITIONLIST* ptr )
{
	POSITIONLIST*	next;
	
	for (; ptr; ptr = next) {
		next = ptr -> next;
		safe_free(ptr);
	}
}

POSITIONLIST* positionlist_new_node ( POSITIONLIST* lst, POSITION pos )
{
	POSITIONLIST*	ret;
	
	ret = (POSITIONLIST*) safe_malloc(sizeof(POSITIONLIST));
	ret -> next = lst;
	ret -> position = pos;
	
	return ret;
}

POSITIONLIST* positionlist_copy ( POSITIONLIST* ptr )
{
	POSITIONLIST*	ret;
	POSITIONLIST**	dest_ptr;
	
	ret = NULL;
	for (dest_ptr = &ret; ptr; ptr = ptr -> next) {
		*dest_ptr = positionlist_new_node(NULL, ptr -> position);
		dest_ptr = &(*dest_ptr) -> next;
	}
	
	return ret;
}



/*
** MOVELIST data type
*/

void movelist_free ( MOVELIST* ptr )
{
	MOVELIST*	next;
	
	for (; ptr; ptr = next) {
		next = ptr -> next;
		safe_free(ptr);
	}
}

MOVELIST* movelist_new_node ( MOVELIST* lst, MOVE move )
{
	MOVELIST*	ret;
	
	ret = (MOVELIST*) safe_malloc(sizeof(MOVELIST));
	ret -> next = lst;
	ret -> move = move;
	
	return ret;
}

/**
 *  Returns a new copy of the given MOVELIST.
 *  Does NOT reverse the order of the MOVELIST.
 */

MOVELIST* movelist_copy ( MOVELIST* ptr )
{
	MOVELIST*	ret;
	MOVELIST**	dest_ptr;
	
	ret = NULL;
	for (dest_ptr = &ret; ptr; ptr = ptr -> next) {
		*dest_ptr = movelist_new_node(NULL, ptr -> move);
		dest_ptr = &(*dest_ptr) -> next;
	}
	
	return ret;
}

