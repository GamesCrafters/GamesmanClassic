/************************************************************************
**
** NAME:	db_basichash.c
**
** DESCRIPTION:	Basic hashtable implementations
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
#include "db_basichash.h"
#include "db_malloc.h"
#include <assert.h>
//#include <string.h>

static gamesdb_bhashin* gamesdb_basichash_getrow(gamesdb_bhash* hash, gamesdb_pageid id) {
	int row = id & (hash->index_size - 1);
	return ((hash->rows) + row);
}

//returns the pointer to the chunk for easy modification.
//when id is found, returns the pointer to the chunk that contains the loc
//when id is not found, returns the pointer to the chunk with the first free spot, growing the row as necessary
static gamesdb_bhashin* gamesdb_basichash_lookupchunk(gamesdb_bhash* hash, gamesdb_pageid id){
	gamesdb_bhashin* place = gamesdb_basichash_getrow(hash, id);

	int myid = id >> hash->index_bits;

	int i;
	gamesdb_bhashin *prev = NULL, *firstempty = NULL;

	while(place != NULL) {
		for(i=0; i<hash->chunk_size; i++) {
			if(place->id[i] == myid)
				return place;
			if(place->id[i] == -1 && firstempty == NULL)
				firstempty = place;
		}
		prev = place;
		place = place->next;
	}
	if(firstempty != NULL)
		return firstempty; // this fixes the fragmentation problem.
	//new elements are added to the first empty slot instead of at the last place.

	//every row has one initial chunk, so we don't have to worry about modifying the root pointer here.
	gamesdb_bhashin *new_chunk = (gamesdb_bhashin *) gamesdb_SafeMalloc(sizeof(gamesdb_bhashin));
	new_chunk->loc = (gamesdb_frameid*) gamesdb_SafeMalloc(sizeof(gamesdb_frameid) * hash->chunk_size);
	new_chunk->id = (gamesdb_pageid*) gamesdb_SafeMalloc(sizeof(gamesdb_pageid) * hash->chunk_size);
	new_chunk->next = NULL;
	for(i=0; i<hash->chunk_size; i++) {
		new_chunk->id[i] = -1;
	}
	prev->next = new_chunk;

	return new_chunk;
}

/*generates and returns a db_bhash pointer to newly malloced memory.
 * the destructor frees all of the memory. Whatever calls this
 * must also call the destructor eventually.
 */
gamesdb_bhash* gamesdb_basichash_create(int ind_bits, int chk_size){
	int i,j;

	gamesdb_bhash* new = (gamesdb_bhash*) gamesdb_SafeMalloc(sizeof(gamesdb_bhash));

	new->index_bits = ind_bits;
	new->index_size = 1 << ind_bits;
	new->chunk_size = chk_size;
	new->rows = (gamesdb_bhashin*) gamesdb_SafeMalloc(sizeof(gamesdb_bhashin) * new->index_size);

	for(i=0; i<new->index_size; i++) {
		(new->rows)[i].loc = (gamesdb_frameid*) gamesdb_SafeMalloc(sizeof(gamesdb_frameid) * chk_size);
		(new->rows)[i].id = (gamesdb_pageid*) gamesdb_SafeMalloc(sizeof(gamesdb_pageid) * chk_size);
		(new->rows)[i].next = NULL;
		for(j=0; j<chk_size; j++) {
			(new->rows)[i].id[j] = -1;
		}
	}

	return new;
}

//returns the frame_id assosiated with page_id. -1 if it does not exist
gamesdb_frameid gamesdb_basichash_get(gamesdb_bhash* hash, gamesdb_pageid id){
	gamesdb_bhashin* mychunk = gamesdb_basichash_lookupchunk(hash, id);
	int i;
	int myid = id >> hash->index_bits;
	for(i=0; i<hash->chunk_size; i++) {
		if(mychunk->id[i] == myid)
			return mychunk->loc[i];
	}

	return NULL;
}

//Assosciates an id with a loc. Only one id per table. returns 0 on success.
int gamesdb_basichash_put(gamesdb_bhash* hash, gamesdb_pageid id, gamesdb_frameid loc){
	gamesdb_bhashin* place;
	int i;

	place = gamesdb_basichash_lookupchunk(hash,id);

	int firstempty = -1;

	int myid = id >> hash->index_bits;
	for(i=0; i < hash->chunk_size && place->id[i] != myid; i++) {
		if (place->id[i] == -1 && firstempty == -1) {
			firstempty = i;
		}
	};

	if(i == hash->chunk_size) { //firstempty must have something
		assert(firstempty != -1);
		place->id[firstempty] = myid;
		place->loc[firstempty] = loc;
		return 0;
	}else{
		//updating existing entry
		place->loc[i] = loc;
		return 0;
	}
}

/* removes id from the hash table. returns frame_id or NULL if id does not exist
 */
gamesdb_frameid gamesdb_basichash_remove(gamesdb_bhash* hash, gamesdb_pageid id){
	int i;
	gamesdb_bhashin* place;

	place = gamesdb_basichash_lookupchunk(hash,id);
	int myid = id >> hash->index_bits;
	for(i=0; i < hash->chunk_size && place->id[i] != myid; i++) ;
	if(i < hash->chunk_size) {
		place->id[i] = -1;
		return place->loc[i];
	}
	return NULL;
}

void gamesdb_basichash_destroy(gamesdb_bhash* hash){
	int i;
	gamesdb_bhashin *place,*next,*start;

	for(i=0; i < hash->index_size; i++) {
		//once per row.
		place = hash->rows + i;
		start = place;
		while(place != NULL) {
			gamesdb_SafeFree(place->loc);
			gamesdb_SafeFree(place->id);
			next = place->next;
			if(place != start)
				gamesdb_SafeFree(place);
			place = next;
		}
	}
	gamesdb_SafeFree(hash->rows);
	gamesdb_SafeFree(hash);
}
