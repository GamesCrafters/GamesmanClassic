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

#include <stdio.h>
#include "db_types.h"
#include "db_bman.h"
#include "db_basichash.h"
#include "db_buf.h"
#include "db_malloc.h"

/* buffer replacement stratagy and replacement tools.
 * ask the buffer manager for a specific buffer to be brought into memory.
 * the buffer manager finds it, and returns the pageid of the corresponding
 * page.
 */

gamesdb_bman* gamesdb_bman_init(){
  // frame_id (*r_fn) (db_bman*, void*)){
  gamesdb_bman *new = (gamesdb_bman*) gamesdb_SafeMalloc(sizeof(gamesdb_bman));
  //new->bufp = bufp;
  //new->replace_fun = r_fn;
  // should initialize r_fn too... which will return an internal data pointer
  // which we then pass into r_fn on subsequent calls.
  // Nah... just make it deal with the first time it has been called and allow
  // it to setup its own internal state.
  new->hash = gamesdb_basichash_create(1024,10);
  new->clock_hand = 0;
  return new;
}


/*Find's page_id and returns the location. -1 if not found.
 */
gamesdb_frameid gamesdb_bman_find(gamesdb* db, gamesdb_pageid id){
  return gamesdb_basichash_get(db->buf_man->hash,id);
}

/* this will be called to find space on physical memory for a page
 * either a new unused one, or one to be brought in from disk
 */
gamesdb_frameid gamesdb_bman_replace(gamesdb* db, gamesdb_pageid vpn) {
	//find free space, if found make hash changes, return
	int i;
	gamesdb_bufferpage* startpage = db->buffers->buffers;
	
	for (i = 0; i < db->num_page; i++) {
		if (startpage->valid == FALSE) {
			gamesdb_basichash_put(db->buf_man->hash, vpn, i);
			return i;
		}
		startpage ++;
	}
	//otherwise, pick one from n-chance, make hash changes, and return
	if (DEBUG) {
		printf("db_bufman: No more free pages in page table. Evicting one page using n-chance.\n");
	}
	
	gamesdb_buffer* bufp = db->buffers;
	startpage = bufp->buffers;
	gamesdb_frameid ret = db->buf_man->clock_hand;
	
	while(TRUE) {
		if (startpage->valid == TRUE && startpage->chances < MAX_CHANCES) {
			startpage->chances++;
		} else {
			break;
		}
		if (ret < (db->num_page-1)) {
			ret ++;
			startpage++;
		} else {
			ret = 0;
			startpage = bufp->buffers;
		}
	}
	gamesdb_basichash_remove(db->buf_man->hash, (db->buffers->buffers+ret)->tag);
	gamesdb_basichash_put(db->buf_man->hash, vpn, ret);
	
	db->buf_man->clock_hand = (ret + 1) % db->num_page;
	
	if (DEBUG) {
		printf("db_bufman: evicted page %llu\n", ret);
	}
	
	return ret;
}

void gamesdb_bman_destroy(gamesdb_bman* bman){
  gamesdb_basichash_destroy(bman->hash);
  gamesdb_SafeFree(bman);
}
