/************************************************************************
**
** NAME:	db_buf.c
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

#include "db_types.h"
#include "db_buf.h"
#include "db_malloc.h"
#include "db_store.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// a buffer contains at least one record.
//each record has rec_size bytes, along with one extra byte to keep track of valid bit
gamesdb_buffer* gamesdb_buf_init(gamesdb_pageid rec_size, gamesdb_pageid max_recs, gamesdb_pageid num_buf){
	gamesdb_buffer* bufp = (gamesdb_buffer*) gamesdb_SafeMalloc(sizeof(gamesdb_buffer));

	bufp->num_pages = 0;
	bufp->max_pages = num_buf;
	bufp->rec_size = rec_size;
	bufp->buf_size = max_recs;
	bufp->pages = NULL;

	return bufp;
}

gamesdb_bufferpage *gamesdb_buf_addpage(gamesdb* db) {
	gamesdb_bufferpage *buf = (gamesdb_bufferpage *) gamesdb_SafeMalloc(sizeof(gamesdb_bufferpage));
	if (buf == NULL) {
		return buf;
	}
	gamesdb_buffer *bufp = db->buffer;
	buf->mem = (char *) gamesdb_SafeMalloc(sizeof(char) * bufp->buf_size * bufp->rec_size);
	if (buf->mem == NULL) {
		gamesdb_SafeFree(buf);
		return NULL;
	}
	buf->tag = 0;
	buf->valid = GAMESDB_FALSE;
	buf->chances = 0;
	buf->dirty = GAMESDB_FALSE;
	buf->next = bufp->pages;
	bufp->pages = buf;
	bufp->num_pages++;
	return buf;
}

void gamesdb_buf_removepage(gamesdb *db) {
	gamesdb_bufferpage *oldhead = db->buffer->pages;
	if (oldhead == NULL) {
		printf("db_buf: There are no pages in the pool. WTF?");
		return;
	}
	gamesdb_bufferpage *newhead = oldhead->next;
	if (newhead == NULL) {
		printf("db_buf: There is only one page in the pool. Cannot shrink anymore.");
		return;
	}
	gamesdb_buf_write(db, oldhead);
	gamesdb_SafeFree(oldhead->mem);
	gamesdb_SafeFree(oldhead);
	db->buffer->pages = newhead;
	db->buffer->num_pages--;
}

int gamesdb_buf_flush_all(gamesdb* db) {
	//with the assumption that the db_store is clustered
	//a straight squential scan will cause ordered forward access to the db_store
	//this will be as fast as it gets
	//int i;
	gamesdb_buffer* bufp = db->buffer;
	gamesdb_bufferpage* buf;

	for (buf = bufp->pages; buf != NULL; buf=buf->next) {
		if (buf->dirty == GAMESDB_TRUE) {
			gamesdb_buf_write(db, buf);
		}
	}
	return 0;
}

//reads a page from disk
int gamesdb_buf_read(gamesdb* db, gamesdb_frameid spot, gamesdb_pageid vpn) {

	//load in the new page
	gamesdb_read(db, vpn, spot);

	assert(spot->tag == 0 || spot->tag == vpn);
	assert(spot->dirty == GAMESDB_FALSE);

	//validate the entire page first
	spot->valid = GAMESDB_TRUE;
	if (spot->tag == 0)
		//the buffer is uninitialized, this means no record exists in the page
		spot->tag = vpn;

	assert(spot->tag == vpn);

	if (GAMESDB_DEBUG) {
		printf("buf_read: spot = %u, buf_tag = %llu, mytag = %llu\n", (unsigned int)spot, spot->tag, vpn);
	}

	return 0;
}

//writes a page to disk
int gamesdb_buf_write(gamesdb* db, gamesdb_frameid spot){

	//gamesdb_buffer* bufp = db->buffer;

	//we don't have to write the page if it's clean
	if(spot->dirty == GAMESDB_TRUE) {
		spot->chances = 0;
		gamesdb_write(db, spot->tag, spot);
		spot->dirty = GAMESDB_FALSE;
		if (GAMESDB_DEBUG) {
			printf("buf_write: spot = %u, buf_tag = %llu\n", (unsigned int)spot, spot->tag);
		}
	}

	return 0;
}

int gamesdb_buf_destroy(gamesdb* db){

	gamesdb_buf_flush_all(db);

	gamesdb_buffer *bufp = db->buffer;

	gamesdb_bufferpage *current = bufp->pages;
	gamesdb_bufferpage *nextp;

	assert(current != NULL);
	for(; current != NULL; current = nextp) {
		nextp = current->next;
		gamesdb_SafeFree(current->mem);
		gamesdb_SafeFree(current);
	}

	gamesdb_SafeFree(bufp);
	return 0; // no error checking;
}
