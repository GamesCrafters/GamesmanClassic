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

#ifndef GMCORE_DB_H
#define GMCORE_DB_H

#include "db_globals.h"

#include "db_store.h"
#include "db_buf.h"
#include "db_bman.h"
#include "db_malloc.h"
#include "db_basichash.h"


typedef struct {
  db_bman* buf_man;
  db_buf_head* buffers;
  db_store* filep;
  frame_id num_buf;
  page_id num_page;
} games_db;

#endif /* GMCORE_DB_H */
