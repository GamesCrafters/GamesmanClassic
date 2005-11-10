/************************************************************************
**
** NAME:	filedb.c
**
** DESCRIPTION:	Functions to load and store databases from/to
**		compressed files.
**		
** AUTHOR:	Scott Lindeneau
**		GamesCrafters Research Group, UC Berkeley
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

#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "gamesman.h"

typedef struct fdb {
	
	char*	filename;
	gzFile* filep;
	int		leadin_size;


}filedb_data;

char* filedb_det_filename(DB_Table*);
int filedb_get(POSITION pos, RECORDTYPE type, char* data);
itn filedb_put(POSITION pos, RECORDTYPE type, char* data);

void filedb_setup(DB_Table* my_table){
	filedb_data* storage = (filedb_data*) SafeMalloc(sizeof(filedb_data));

	(*my_table).db_internal_storage = storage;

	storage->filename = filedb_det_filename(my_table);
	filedb_open(my_table);
	

	//Check to see if the file has the correct stuff in it and is "good"
	if(filedb_goodfile(my_table)){
		//if file is good
		//dont have to do anything more
	} else {
		//file isnt good or doesnt exist
		//thus it needs to be initialized and/or truncated
		filedb_setupfile(my_table);
	}


	(*my_table).get = filedb_get;
	(*my_table).put = filedb_put;
}

void filedb_open(DB_Table* my_table){
	filedb_data* storage = (*my_table).db_internal_storage;

	if((storage->filep = gzopen(storage.filename, "rb")) == NULL) {
		if((storage->filep = gzopen(storage.filename, "wb")) == NULL)
			ExitStageRightErrorString("Unable to create or open file database");
    }
}


int filedb_get(POSITION pos, RECORDTYPE type, char* data, DB_Table* my_table){
	filedb_data* storage = (*my_table).db_internal_storage;
	gzFile* filep = storage->filep;
	POSITION byte_loc,off_loc,new_loc;
	int numBytes;
	int error;

	byte_loc = gztell(filep);

	if(pos*4 < pos)
		ExitStageRightErrorString("Overflow not handled in databases for extreamly large positions (2^62 or higher).");

	off_loc = filedb_det_byteoff(byte_loc,pos,type,my_table);

	new_loc = gzseek(filep,off_loc,SEEK_CUR);

	if(new_loc != (byte_loc + off_loc))
		ExitStageRightErrorString("Error seeking to correct location in file");

	numBytes = db_num_bytes(type);
	error = gzread(filep,data,numBytes);

	if(error != numBytes)
		ExitStageRightErrorString("Unable to read all bytes from db file");

	filedb_align(char* data, numBytes, db_num_bits(type));

	return 0;
}
