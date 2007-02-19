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
  gamesdb_bman *new = (gamesdb_bman*) gamesdb_SafeMalloc(sizeof(gamesdb_bman));
  new->hash = gamesdb_basichash_create(1024,10);
  new->clock_hand = NULL;
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
    gamesdb_buffer* bufp = db->buffer;

	//find free space, if found make hash changes, return
	gamesdb_bufferpage* page = bufp->pages;
	
    gamesdb_bhash *bhash = db->buf_man->hash;
    
	for (; page!= NULL; page = page->next) {
		if (page->valid == FALSE) {
			gamesdb_basichash_put(bhash, vpn, page);
			return page;
		}
	}
    
    gamesdb_bufferpage *newpage;
    
    //see if we have space for more physical pages, if so grow the memory pool
    if (bufp->num_pages < bufp->max_pages) {
        if (newpage = gamesdb_buf_addpage(db) != NULL) {
            gamesdb_basichash_put(bhash, vpn, newpage);
            return newpage;
        }
    }
    
	//otherwise, pick one from n-chance, make page table changes, and return
	if (DEBUG) {
		printf("db_bufman: No more free pages in page table. Evicting one page using n-chance.\n");
	}
	
    if (bufp->max_pages == 0) {
        printf("db_bufman: max page limit reached zero.");
    }
    
	page = bufp->pages;
	gamesdb_bufferpage *ret = db->buf_man->clock_hand;
	
	while(TRUE) {
		if (ret->valid == TRUE && ret->chances < MAX_CHANCES) {
			ret->chances++;
		} else {
			break;
		}
        if (ret == NULL) {
            ret = bufp->pages;
        } else {
            ret = ret->next;
        }
	}
	gamesdb_basichash_remove(bhash, vpn);
	gamesdb_basichash_put(bhash, vpn, ret);
	
    ret = ret->next;
    if (ret == NULL) {
        ret = bufp->pages;
    }
    
	db->buf_man->clock_hand = ret;
	
	if (DEBUG) {
		printf("db_bufman: evicted page at address %llu\n", ret);
	}
	
	return ret;
}

void gamesdb_bman_destroy(gamesdb_bman* bman){
  gamesdb_basichash_destroy(bman->hash);
  gamesdb_SafeFree(bman);
}
