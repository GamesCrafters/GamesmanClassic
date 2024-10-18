/************************************************************************
**
** NAME:	db.h
**
** DESCRIPTION:	Gamescrafters Database Global Header File
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

#ifndef GMCORE_GAMESDB_H
#define GMCORE_GAMESDB_H

#include "db_types.h"
#include "db_store.h"
#include "db_buf.h"
#include "db_bman.h"
#include "db_malloc.h"
#include "db_basichash.h"


gamesdb*        gamesdb_create  (int rec_size, gamesdb_pageid max_recs, gamesdb_pageid max_pages, int cluster_size, char* db_name);
void            gamesdb_destroy (gamesdb* data);
void            gamesdb_get             (gamesdb* gdb, char* mem, gamesdb_position pos);
void            gamesdb_put             (gamesdb* gdb, char* mem, gamesdb_position pos);

#endif /* GMCORE_GAMESDB_H */
