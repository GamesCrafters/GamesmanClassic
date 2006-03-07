/************************************************************************
**
** NAME:	db_bman.h
**
** DESCRIPTION:	Gamescrafters buffer manager
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

#ifndef GMCORE_DB_BMAN_H
#define GMCORE_DB_BMAN_H

#include "db_buf.h"
#include "db_store.h"
#include "db_basichash.h"

typedef struct db_bman_struct {
  db_buf_head* bufp;
  frame_id (*replace_fun) (db_bman*);
  db_bhash *hash;
} db_bman;

db_bman* bman_init(db_buf_head*,frame_id (*r_fn));
buf_loc bman_find(db_bman*,page_id);



#endif /* GMCORE_DB_BMAN_H */
