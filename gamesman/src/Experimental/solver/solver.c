
/*
** Solver repository
** John Jordan (jjjordan@berkeley.edu)
*/

#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stddef.h>

#include "gamesman.h"
#include "solver_local.h"


/*
** Solver table
*/

static TABLE	SOLVER_LIST = NULL;


/*
** Local function prototypes
*/

static void	solver_load	( SOLVER_CLASS* );


/*
** Code
*/

void solver_init ( void )
{
	solver_load(NULL);
}

void solver_load ( SOLVER_CLASS* sc )
{
	if (!sc)
		return;
	
	table_put_voidp(&SOLVER_LIST, sc -> class_name, sc, NULL);
}

SOLVER_CLASS* solver_get ( STRING class_name )
{
	void*	ret;
	
	if (table_get_voidp(SOLVER_LIST, class_name, &ret))
		return NULL;
	
	return (SOLVER_CLASS*) ret;
}

SOLVER* solver_new ( STRING class_name, GAME* game, DATABASE* db )
{
	SOLVER_CLASS*	sc;
	
	if (!(sc = solver_get(class_name)))
		return NULL;
	
	return (*sc -> create)(game, db);
}
