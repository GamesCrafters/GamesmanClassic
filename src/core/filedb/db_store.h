
/************************************************************************
**
** NAME:	db_store.h
**
** DESCRIPTION:	Gamescrafters Database File Functions
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

//a db store implementation. it only knows about pages, not records.
//all read/write ops are on pages, which are groups of more than one records.

#ifndef GMCORE_DB_FILE_H
#define GMCORE_DB_FILE_H

#include "db_types.h"

gamesdb_store*  gamesdb_open    (gamesdb* db, char* filename, int cluster_size);
int             gamesdb_close           (gamesdb_store* db);
void            gamesdb_seek            (gamesdb_store* db, gamesdb_pageid page);
int             gamesdb_read            (gamesdb* db, gamesdb_pageid page, gamesdb_bufferpage* buf);
int             gamesdb_write           (gamesdb* db, gamesdb_pageid page, gamesdb_bufferpage* buf);
//page_id       db_newPage  (db_store* db);

#endif /* GMCORE_DB_FILE_H */
