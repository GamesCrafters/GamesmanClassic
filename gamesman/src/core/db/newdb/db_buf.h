/************************************************************************
**
** NAME:	db_buf.h
**
** DESCRIPTION:	Buffer factory and modifiers.
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

#ifndef GMCORE_DB_BUF_H
#define GMCORE_DB_BUF_H

#include "db_store.h"
#include "db.h"

typedef unsigned long long frame_id;
typedef unsigned long long page_id;

typedef struct buf_struct {
  char* mem;
  page_id id; //buffer id
  boolean dirty;

}db_buf;

typedef struct main_buf_struct {
  db_store* filep;

  db_buf* buffers;
  db_offset* buf_off;
  int buf_size;
  int num_buf;
}db_buf_head;

db_buf_head* db_buf_init(int rec_size, frame_id num_buf, int buf_size, db_store* filep);
int db_buf_flush(db_buf_head* bufp, frame_id i);
int db_buf_read(db_buf_head* bufp, frame_id spot, page_id id);
int db_buf_destroy(db_buf_head* bufp);

#endif /* GMCORE_DB_BUF_H */

