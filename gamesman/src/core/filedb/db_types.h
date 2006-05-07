#ifndef DB_TYPES_H_
#define DB_TYPES_H_

#include <zlib.h>
#include "db_globals.h"

//basic types
typedef unsigned long long gamesdb_position;

typedef unsigned long long gamesdb_offset;

typedef unsigned long long gamesdb_frameid;
typedef unsigned long long gamesdb_pageid;

typedef char boolean;

//backing store
typedef struct dbfile_struct{
  char* filename;   //disk file name
 // page_id* pagemap;
  gzFile* filep;	//the disk file descriptor
  gamesdb_pageid current_page;  //the current page
  gamesdb_pageid last_page; //offset of the next page after the end
}gamesdb_store;

//hash
typedef struct db_bhashin_struct {
  int num;
  gamesdb_frameid* loc;
  gamesdb_pageid* id;
  struct db_bhashin_struct* next;
} gamesdb_bhashin;


typedef struct {
  int size;
  gamesdb_bhashin* rows;

} gamesdb_bhash;

//buffer
typedef struct {
  char mem[MEM_ARRAY_SIZE];
  gamesdb_pageid tag;
  boolean valid;
}gamesdb_bufferpage;

typedef struct {
  gamesdb_store* filep;

  gamesdb_bufferpage* buffers;
  boolean* dirty;
  int rec_size; //number of bytes in a record
  int buf_size; //number of records in a buffer
  int n_buf;
}gamesdb_buffer;

//buffer manager
typedef struct {
  gamesdb_buffer* bufp;
  //frame_id (*replace_fun) (db_bman*);
  gamesdb_bhash *hash;
} gamesdb_bman;

//the db object, so to speak
typedef struct {
//  db_bman* buf_man;
  gamesdb_buffer* buffers;
  gamesdb_store* store;
  gamesdb_pageid num_page;
} gamesdb;


#endif /*DB_TYPES_H_*/
