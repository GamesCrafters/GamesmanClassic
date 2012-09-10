/************************************************************************
**
** NAME:	db_basichash.h
**
** DESCRIPTION:	Basic Hash Table implementation.
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

#ifndef GMCORE_DB_BASICHASH_H
#define GMCORE_DB_BASICHASH_H

#include  <string.h>
#include  "db_buf.h"
#include  "db_types.h"

gamesdb_bhash*      gamesdb_basichash_create    (int ind_bits, int chk_size);
gamesdb_frameid     gamesdb_basichash_get       (gamesdb_bhash* hash, gamesdb_pageid id);
gamesdb_frameid     gamesdb_basichash_remove    (gamesdb_bhash* hash, gamesdb_pageid id);
int gamesdb_basichash_put       (gamesdb_bhash* hash, gamesdb_pageid, gamesdb_frameid );
void                gamesdb_basichash_destroy   (gamesdb_bhash* hash);

#endif /* GMCORE_DB_BASICHASH_H */
