
#include <stdio.h>
#include "gamesman.h"


int main ( int argc, char** argv )
{
	DATABASE*	db;
	DATABASE*	db2;
	int		i;
	short		n;
	
	db_init();
	solver_init();
	
	if (!(db = db_open("")))
		die("Can't open the db.\n");
	
	(*db -> init)(db, 10, (argc < 2 ? 16 : atoi(argv[1])));
	
	for (i = 0; i < 10; i++) {
		n = 100 * (10 - i);
		(*db -> put)(db, i, &n);
	}
	
	for (i = 0; i < 10; i++) {
		n = 0;
		(*db -> get)(db, i, &n);
		printf("%d\n", n);
	}
	
	if (!(db2 = db_open("")))
		die("Cant open the second db.\n");
	
	(*db -> transfer)(db, db2);
	(*db -> free)(db);
	
	for (i = 0; i < 10; i++) {
		n = 0;
		(*db2 -> get)(db2, i, &n);
		printf("%d\n", n);
	}
	
	(*db2 -> free)(db2);
	
	return 0;
}
