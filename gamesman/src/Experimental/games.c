
/*
** Game repository
** John Jordan (jjjordan@berkeley.edu)
*/

#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "gamesman.h"


/*
** Game table
*/

static TABLE	GAME_LIST = NULL;


/*
** Code
*/

void game_load ( GAME_CLASS* gc )
{
	if (!gc)
		return;
	
	table_put_voidp(&GAME_LIST, gc -> class_name, gc, NULL);
}

GAME_CLASS* game_get ( STRING class_name )
{
	void*	ret;
	
	if (table_get_voidp(GAME_LIST, class_name, &ret))
		return NULL;
	
	return (GAME_CLASS*) ret;
}

GAME* game_new ( STRING class_name )
{
	GAME_CLASS*	c;
	
	if (!(c = game_get(class_name)))
		return NULL;
	
	return (*c -> create)();
}
