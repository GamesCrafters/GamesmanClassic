/************************************************************************
**
** NAME:	coldb.c
**
** DESCRIPTION:	Accessor functions for the in-memory database.
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

#include "gamesman.h"
#include "colldb.h"

void            colldb_free ();

/* Value */
VALUE		colldb_get_value	(POSITION pos);
VALUE		colldb_set_value	(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	colldb_get_remoteness	(POSITION pos);
void		colldb_set_remoteness	(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		colldb_check_visited    (POSITION pos);
void		colldb_mark_visited     (POSITION pos);
void		colldb_unmark_visited	(POSITION pos);

/* Mex */
MEX		colldb_get_mex		(POSITION pos);
void		colldb_set_mex		(POSITION pos, MEX mex);


/* Collision Specific */
typedef struct colldb_node {

  VALUE myValue; 
  char myPos;
  struct colldb_node *next;

} colldb_value_node;

colldb_value_node *colldb_find_pos(POSITION position);
colldb_value_node* colldb_make_node(POSITION pos, VALUE val, colldb_value_node *next);

colldb_value_node *colldb_add_node(POSITION position);

colldb_value_node** colldb_hash_table;
POSITION colldb_num_allocated;
int colldb_HASHSIZE;
int hitcount;
int accesscount;
int remotecount;
int visitcount;
/*
** Code
*/


void colldb_init(DB_Table *new_db)
{
    hitcount = 0;
    colldb_HASHSIZE = (int)(gNumberOfPositions >> 8) + 1;

    //setup internal memory table
    colldb_hash_table = (colldb_value_node **) SafeMalloc (colldb_HASHSIZE * sizeof(colldb_value_node*));
    //printf("initalizing collision database. %d rows allocated.\n",colldb_HASHSIZE);
  
    memset(colldb_hash_table, 0x0000, colldb_HASHSIZE * sizeof(colldb_value_node*));

    //set function pointers
    new_db->get_value = colldb_get_value;
    new_db->put_value = colldb_set_value;
    new_db->get_remoteness = colldb_get_remoteness;
    new_db->put_remoteness = colldb_set_remoteness;
    new_db->check_visited = colldb_check_visited;
    new_db->mark_visited = colldb_mark_visited;
    new_db->unmark_visited = colldb_unmark_visited;
    new_db->get_mex = colldb_get_mex;
    new_db->put_mex = colldb_set_mex;
    
    new_db->free_db = colldb_free;
  
    return;
}

void colldb_free(){

    int i;
    colldb_value_node *next, *cur;

    if(colldb_hash_table){
	for(i=0;i<colldb_HASHSIZE;i++){
	    cur = colldb_hash_table[i];
	    while(cur != NULL){
		next = cur->next;
		SafeFree(cur);
		cur = next;
	    }
	}
	SafeFree(colldb_hash_table);
    }
}

colldb_value_node *colldb_find_pos(POSITION position){

    POSITION i;
    char key;
    colldb_value_node *cur;

    key = (char) (position / colldb_HASHSIZE);
    i = position % colldb_HASHSIZE;
    cur = colldb_hash_table[i];
    
    while(cur != NULL){
	if(cur->myPos == key){
	    hitcount++;
	    if(hitcount%1000 == 0){
		//printf("hit: %d, access: %d, visit: %d, remote: %d\n",hitcount,accesscount,visitcount,remotecount);
	    }	
	    return cur;	
	}
	cur = cur->next;
    }
    
    return NULL;
}

colldb_value_node* colldb_make_node(POSITION pos, VALUE val, colldb_value_node *next){

    colldb_value_node *ptr = (colldb_value_node *) SafeMalloc (sizeof(colldb_value_node));

    ptr->myPos = (char) (pos / colldb_HASHSIZE);
    ptr->myValue = val;
    ptr->next = next;
    colldb_num_allocated++;
    
    if(colldb_num_allocated % 100 == 0){
	//printf("number allocated: "POSITION_FORMAT"\n",colldb_num_allocated);
	fflush(NULL);
    }
    
    return ptr;
}

colldb_value_node *colldb_add_node(POSITION position){
  POSITION i;
  i = position % colldb_HASHSIZE;
 
  colldb_hash_table[i] = colldb_make_node(position, undecided, colldb_hash_table[i]);
  
  return colldb_hash_table[i];
}

VALUE colldb_set_value(POSITION pos, VALUE val)
{
  colldb_value_node *ptr = colldb_find_pos(pos);

  if(ptr == NULL)
    ptr = colldb_add_node(pos);
  
  ptr->myValue = (ptr->myValue & ~VALUE_MASK) | (val & VALUE_MASK);    

  return (ptr->myValue & VALUE_MASK);
}

VALUE colldb_get_value(POSITION pos)
{
  colldb_value_node *ptr = colldb_find_pos(pos);

  if(ptr == NULL)
    return undecided;

  return (ptr->myValue & VALUE_MASK);

}

REMOTENESS colldb_get_remoteness(POSITION pos)
{
  colldb_value_node *ptr = colldb_find_pos(pos);

  if(ptr == NULL) {
      //printf("HAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAH!!!!!\n\n\n\n\n");
      return 0;
  }

  return (ptr->myValue & REMOTENESS_MASK) >> REMOTENESS_SHIFT;
  
}

void colldb_set_remoteness (POSITION pos, REMOTENESS val)
{
    colldb_value_node *ptr;

/*  
  if(remoteness > REMOTENESS_MAX) {
    printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",remoteness,position,REMOTENESS_MAX);
    ExitStageRight();
    exit(0);
  }
*/
    //printf("pos: %llu, remoteness: %d\n", pos, val);

    ptr = colldb_find_pos(pos);

    if(ptr == NULL)
	ptr = colldb_add_node(pos);
  
    ptr->myValue = (ptr->myValue & ~REMOTENESS_MASK) | (val << REMOTENESS_SHIFT);
}

BOOLEAN colldb_check_visited(POSITION pos)
{
  colldb_value_node *ptr = colldb_find_pos(pos);

  if(ptr == NULL)
    return FALSE;

  //printf("check pos: %llu, %d\n", pos, ptr->myValue & VISITED_MASK);

  return ((ptr->myValue & VISITED_MASK) == VISITED_MASK);
}

void colldb_mark_visited (POSITION pos)
{
  colldb_value_node *ptr = colldb_find_pos(pos);

  if(ptr == NULL)
      ptr = colldb_add_node(pos);

  //printf("mark pos: %llu\n", pos);

  ptr->myValue = ptr->myValue | VISITED_MASK;
}

void colldb_unmark_visited (POSITION pos)
{
  colldb_value_node *ptr = colldb_find_pos(pos);
  
  if(ptr == NULL) {
      //printf("this should never happen!!! unmark_visited\n");
      return;
  }

  //printf("unmark pos: %llu\n", pos);

  ptr->myValue = ptr->myValue & ~VISITED_MASK;
}

void colldb_set_mex(POSITION pos, MEX mex)
{
    colldb_value_node *ptr = colldb_find_pos(pos);
    
    if (ptr == NULL) {
	//printf("this should never happen, but I will do it anyway. set_mex\n");
	ptr = colldb_add_node(pos);
    }

    ptr->myValue = (ptr->myValue & (~MEX_MASK)) | (mex << MEX_SHIFT);
}

MEX colldb_get_mex(POSITION pos)
{
    colldb_value_node *ptr = colldb_find_pos(pos);

    if (ptr == NULL) {
	//printf("you are trying to access a cell that is not in the db already.\n");
	return 0;
    }

    return (MEX)((ptr->myValue & MEX_MASK) >> MEX_SHIFT);

}
