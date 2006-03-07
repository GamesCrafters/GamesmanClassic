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

#include "db_basichash.h"

/*generates and returns a db_bhash pointer to newly malloced memory.
 * the destructor frees all of the memory. Whatever calls this
 * must also call the destructor eventually.
 */
db_bhash* db_basichash_create(int num_rows,int num_in){
  int i,j;
  db_bhash* new = (db_bhash*) SafeMalloc(sizeof(db_bhash));
  
  new->size = num_rows;
  new->rows = (db_bhashin*) SafeMalloc(sizeof(db_bhashin) * num_rows);
  
  for(i=0;i<num_rows;i++){
    (new->rows)[i]->num = num_in;
    (new->rows)[i]->loc = (frame_id*) SafeMalloc(sizeof(frame_id) * num_in);
    (new->rows)[i]->id = (page_id*) SafeMalloc(sizeof(page_id) * num_in);
    (new->rows)[i]->next = NULL;
    for(j=0;j<num_in,j++){
      (new->rows)[i]->id[j] = -1;
    }
  }
  
  return new;
}

//returns the frame_id assosiated with page_id. -1 if it does not exist
frame_id db_basichash_get(db_bhash* hash, page_id id){
  int i;
  int row = id % hash->size;
  db_bhashin* place = (hash->rows) + row;
  while(place != NULL){
    for(i=0;i<place->num;i++){
      if(place->id[i] == id)
	return place->loc[i];
    }
    place = place->next;
  }
  
  return -1;
}



//Assosciates an id with a loc. Only one id per table. returns 0 on success.
int db_basichash_put(db_bhash* hash, page_id id, frame_id loc){
  db_bhashin* place;
  int i;

  place = db_basichash_getin(hash,id);
  for(i=0;i < place->num && place->id[i] != id;i++);
  
  if(place->id[i] != id){
    //we need to insert into hash table.
    for(i=0;i < place->num && place->id[i] != -1;i++);
    if(place->id[i] != -1){
      //add another link in the list.
      db_bhashin* new = (db_bhashin*) SafeMalloc(sizeof(db_bhashin));
      new->num = place->num;
      new->next = place->next;
      new->loc = (frame_id*) SafeMalloc(sizeof(frame_id)*new->num);
      new->id = (page_id*) SafeMalloc(sizeof(page_id)*new->num);
      for(i=0;i < new->num;i++)
	new->id[i] = -1;
      i = 0;
      place->next = new;
      place = new;
    }
    place->id[i] = id;
    place->loc[i] = loc;
    return 0;
  }else{
    //updating existing entry
    place->loc[i] = loc;
    return 0;
  }
}

/* removes id from the hash table. returns frame_id or -1 if id des nto exist
 */
frame_id db_basichash_remove(db_bhash* hash, page_id id){
  int i;
  db_bhashin* place;

  place = db_basichash_getin(hash,id);
  for(i=0;i < place->num && place->id[i] != id,i++);
  if(place->id[i] == id){
    place->id[i] = -1;
    return place->loc[i];
  }
  return -1;
}

void db_basichash_destroy(db_bhash* hash){
  int i,j;
  db_bhashin *place,*next,*start

  for(i=0;i < hash->size;i++){
    //once per row.
    place = hash->rows + i;
    start = place;
    while(place != NULL){
      SafeFree(place->loc);
      SafeFree(place->id);
      next = place->next;
      if(place != start)
	SafeFree(place);
      place = next;
    }
  }
  SafeFree(hash->rows);
  SafeFree(hash);
}
    
		      

//Helper fucntion to return the internal pointer for easy modification.
db_bhashin* db_basichash_getin(db_bhash* hash, page_id id){
  int i;
  int row = id % hash->size;
  db_bhashin* prev=NULL,firstempty = NULL;
  db_bhashin* place = (hash->rows) + row;
  while(place != NULL){
    for(i=0;i<place->num;i++){
      if(place->id[i] == id)
	return place;
      if(place->id[i] == -1 && firstempty == NULL)
	firstempty = place;
    }
    prev = place;
    place = place->next;
  }
  if(firstempty != NULL)
    return firstempty; // this fixes the fragmentation problem.
  //new elements are added to the first empty slot instead of at the last
  // place.
  return prev;
}

