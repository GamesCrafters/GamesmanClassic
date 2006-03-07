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

#include "db_buf.h"
#include "db_malloc.h"
#include <stdio.h>
#include <string.h>


db_buf_head* db_buf_init(int rec_size, frame_id num_buf, int buf_size, db_store* filep){
  db_buf_head* bufp = (db_buf_head*) SafeMalloc(sizeof(db_buf_head));
  frame_id i;
  
  bufp->filep = filep;
  bufp->n_buf = num_buf;
  bufp->buf_size = buf_size;
  bufp->buffers = (db_buf*) SafeMalloc (sizeof(db_buf) * num_buf);
  bufp->buf_off = (db_offset*) SafeMalloc(sizeof(db_offset) * num_buf);
					  
  for(i=0;i<num_buf;i++){
    bufp->buffers[i].mem = NULL; 
    bufp->buffers[i].dirty = FALSE;
    bufp->buffers[i].id = -1;
    bufp->buf_off[i] = (db_offset) -1;
  }

  return bufp;
}

db_buf_head* db_buf_fileinit(db_store* filep){
  //Not going to read from file yet ok? deal with it.
  return NULL;
}

int db_buf_flush(db_buf_head* bufp, frame_id i){
  //maybe we should write a flush all in here?
  // What logic do we need to use for fast (all) flushing?
  db_offset off;
  db_buf* buf = bufp->buffers + i;
  if(buf->dirty && buf->mem != NULL){
    db_store* filep = bufp->filep;
    
    off = bufp->buf_off[buf->id];
    if(off == (db_offset) -1){
      off = db_nextBlock(filep,(db_offset)bufp->buf_size);
      bufp->buf_off[buf->id] = off;
    }

    db_seek(filep,off,SEEK_SET);//maybe a seek write would be nice?
    db_write(filep,buf->mem,bufp->buf_size);//error check not in.
    //what if it isnt a forward seek?? rewrite seek to allow backwards
    // seeking on open files. (i.e. close and reopen the file... but how?)
    return 0;

  }else{
    return 0;
  }
}

int db_buf_read(db_buf_head* bufp, frame_id spot, page_id id){
  db_offset off;
  db_store* filep;
  int i;
  if(bufp->buffers[spot].mem == NULL)
    bufp->buffers[spot].mem = (char*) SafeMalloc(bufp->buf_size);

  if(bufp->buffers[spot].dirty)
    db_buf_flush(bufp,spot);
  


  db_buf* buf = bufp->buffers + spot;
  
  off = bufp->buf_off[id];
  if(off == (db_offset) -1){
    for(i = 0; i < bufp->buf_size; i++){
      buf->mem[i] = 0;
    }
  }else{
    filep = bufp->filep;
    db_seek(filep,off,SEEK_SET);
    db_read(filep,buf->mem,bufp->buf_size);
    //im still not handeling error cases.
  }  
  buf->id = id;

  return 0;
}
 
int db_buf_destroy(db_buf_head* bufp){
  frame_id i;
  
  for(i=0;i<bufp->n_buf;i++){
    SafeFree(bufp->buffers[i].mem);
  }
  SafeFree(bufp->buffers);
  SafeFree(bufp->buf_off);
  db_close(bufp->filep);
  
  return 0; // no error checking;
}

void db_buf_copyOut(db_buf_head* bufp, char* mem,frame_id fid, int off, int amt){
  memcpy(mem,bufp->buffers[fid].mem+off,amt);
}

void db_buf_copyIn(db_buf_head* bufp, char* mem,frame_id fid, int off, int amt){
  memcpy(bufp->buffers[fid].mem+off,mem,amt);
}
