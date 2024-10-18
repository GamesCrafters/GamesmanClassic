#ifndef DB_TYPES_H_
#define DB_TYPES_H_

#include <zlib.h>
#include "db_globals.h"

//basic types
typedef unsigned long long gamesdb_position;

typedef unsigned long long gamesdb_offset;

//virtual
typedef unsigned long long gamesdb_pageid;

typedef char gamesdb_boolean;
typedef char gamesdb_counter;

//buffer page
typedef struct gamesdb_bufferpage_struct {
	char *mem;
	gamesdb_pageid tag;
	gamesdb_boolean valid;
	gamesdb_boolean dirty;
	gamesdb_counter chances;
	struct gamesdb_bufferpage_struct *next;
} gamesdb_bufferpage;

//physical
//very hacky, will change soon
//typedef struct gamesdb_bufferpage_struct * gamesdb_frameid;
#define gamesdb_frameid struct gamesdb_bufferpage_struct*

//backing store
typedef struct dbfile_struct {
	char* filename; //disk file name
	// page_id* pagemap;
	gzFile* filep;  //the disk file descriptor
	gamesdb_pageid current_page; //the current page
	gamesdb_pageid last_page; //offset of the next page after the end
	int dir_size; //the number of bits grouped inside one directory of disk pages
}gamesdb_store;

//hash
typedef struct db_bhashin_struct {
	gamesdb_frameid* loc;
	gamesdb_pageid* id;
	struct db_bhashin_struct* next;
} gamesdb_bhashin;


typedef struct {
	int index_size;
	int index_bits;
	int chunk_size;
	gamesdb_bhashin* rows;

} gamesdb_bhash;

typedef struct {
	gamesdb_bufferpage* pages;
	int rec_size; //number of bytes in a record
	int buf_size; //number of records in a buffer
	int num_pages; //number of pages in memory
	int max_pages;
}gamesdb_buffer;

//buffer manager
typedef struct {
	//gamesdb_buffer* bufp;
	//frame_id (*replace_fun) (db_bman*);
	gamesdb_bhash *hash;
	gamesdb_bufferpage *clock_hand;
} gamesdb_bman;

//the db object, so to speak
typedef struct {
	gamesdb_bman* buf_man;
	gamesdb_buffer* buffer;
	gamesdb_store* store;
	//gamesdb_pageid num_page;
} gamesdb;


#endif /*DB_TYPES_H_*/
