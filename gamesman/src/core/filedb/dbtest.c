/************************************************************************
**
** NAME:	db.c
**
** DESCRIPTION:	Gamescrafters Database Test Program
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include <stdio.h>
#include "db.h"

int main(int argc, char *argv[])
{
	Position totalrecs = 10000;
	size_t recsize = sizeof(short);
	size_t buffers = 100;
	char dbname[] = "testdb.dat\0";

	games_db *testdb = db_create(recsize, buffers, dbname);
	printf("Starting DB test...\n");
	
	Position i;
	short data = 0, result = 0;

	for (i=0;i<totalrecs;i++) {
		data = i % (1<<15);
		db_put(testdb, &data, i);
		db_get(testdb, &result, i);
		if (data!=result) {
			printf("ERROR: position %llu, saved %d, got %d\n", i, data, result);
			break;
		}
	}

	printf("Ending DB test...\n");
	db_destroy(testdb);
	return 0;
}