/************************************************************************
**
** NAME:	db_bman.c
**
** DESCRIPTION:	Buffer manager.
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

#include "db_bman.h"
#include "db_basichash.h"
#include "db_buf.h"
#include "db_malloc.h"

/* buffer replacement stratagy and replacement tools.
 * ask the buffer manager for a specific buffer to be brought into memory.
 * the buffer manager finds it, and returns the buffer_id of the corresponding
 * buffer.
 */

/* r_fn is the replacement stratagy function.
 */


db_bman* bman_init(db_buf_head* bufp){
     // frame_id (*r_fn) (db_bman*, void*)){
  db_bman *new = (db_bman*) SafeMalloc(sizeof(db_bman));
  new->bufp = bufp;
  //new->replace_fun = r_fn;
  // should initialize r_fn too... which will return an internal data pointer
  // which we then pass into r_fn on subsequent calls.
  // Nah... just make it deal with the first time it has been called and allow
  // it to setup its own internal state.
  new->hash = db_basichash_create(1024,10);
  return new;
}


/*Find's page_id and returns the location. Brings it in if the buffer is not
 *in memory.
 */
frame_id bman_find(db_bman* dat, page_id id){
  frame_id ret = db_basichash_get(dat->hash,id);
  if(ret == -1){
    // bring into buffers.
    //ret = dat->replace_fun(dat,null); //null is going to be used for db_tell()
    ret = 0;
    db_buf_flush(dat->bufp,ret);
    db_buf_read(dat->bufp,ret,id);
  }
  return ret;
}

void bman_destroy(db_bman* bman){
  db_basichash_destroy(bman->hash);
  SafeFree(bman);
}
