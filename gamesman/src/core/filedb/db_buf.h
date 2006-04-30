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

//

#ifndef GMCORE_DB_BUF_H
#define GMCORE_DB_BUF_H

#include "db_globals.h"
#include "db_store.h"

typedef struct main_buf_struct {
  db_store* filep;

  db_buffer_page* buffers;
  boolean* dirty;
  int rec_size; //number of bytes in a record
  int buf_size; //number of records in a buffer
  int n_buf;
}db_buffer;

db_buffer*	 	db_buf_init		(int rec_size, page_id num_buf, db_store* filep); 
int 			db_buf_read		(db_buffer* bufp, Position spot, void* value);
int				db_buf_write	(db_buffer* bufp, Position spot, const void* value);
int 			db_buf_destroy	(db_buffer* bufp);

#endif /* GMCORE_DB_BUF_H */

