
#include <stdio.h>
#include "gamesman.h"


int main ( int argc, char** argv )
{
	DATABASE*	db;
	DATABASE*	db2;
	int		i;
	unsigned short	n;
	unsigned char	p[5];
	
	db_init();
	solver_init();
	
	if (!(db = db_open("")))
		die("Can't open the db.\n");
	
	(*db -> init)(db, 10, (argc < 2 ? 16 : atoi(argv[1])));
	
	for (i = 0; i < 10; i++) {
		n = 100 * (10 - i);
		p[0] = (n & 0xff00) >> 8;
		p[1] = (n & 0xff);
//		p[0] = i;
		(*db -> put)(db, i, &p[0]);
	}
	
	for (i = 0; i < 10; i++) {
		p[0] = p[1] = 0;
		(*db -> get)(db, i, &p[0]);
		n = (p[0] << 8) | p[1];
//		n = p[0];
		printf("%d\n", n);
	}
	
	if (!(db2 = db_open("")))
		die("Cant open the second db.\n");
	
	(*db -> transfer)(db, db2);
	(*db -> free)(db);
	
	for (i = 0; i < 10; i++) {
//		n = 0;
		p[0] = p[1] = 0;
		(*db2 -> get)(db2, i, &p[0]);
		n = (p[0] << 8) | p[1];
		printf("%d\n", n);
	}
	
	(*db2 -> free)(db2);
	
	return 0;
}
