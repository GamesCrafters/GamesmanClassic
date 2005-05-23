/************************************************************************
**
** NAME:	db.c
**
** DESCRIPTION:	Generic Database Functions and Database Class Accessors
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

/*
** Needs to be built up to implement The new DB Class abstraction as is
** Found in the Expeimental directory. However we first need to make
** The existing functions abstract.
*/


#include "gamesman.h"
#include "db.h"


DB_Table *db_dbList;
int		  db_numDB = 0;
int		  db_sizeDB = 0;
void	 (*database_setup) (DB_Table *db);



/*************************
**
** db_intitialize Initializes the database and assigns it a database number.
**
**	Sets up the database of the specified type, with specifed records.
**	Uses any previous information that may be inferd about the database.
*************************/
int db_initialize(RECORDTYPE* rec_types, RECORDSIZE* rec_sizes,int num_rec, KNOWN_DATABSES db_type){
	int newdb;

	// Sets up the new database.
	newdb = db_SetUpDB();

	//Initializes the database.
	database_setup(db_dbList+newdb);

	return newdb;
}

int db_SetUpDB(RECORDTYPE* rec_types, RECORDSIZE* rec_sizes,int num_rec, KNOWN_DATABSES db_type){
	int thisNum,nameLength,i,j;
	RECORDSIZE temp;
	char* name;
	if(db_numDB >= db_sizeDB)
		db_size = db_incSize();

	thisNum = db_numDB = db_numDB + 1;

	db_dbList[thisNum].record_types = rec_types;
	db_dbList[thisNum].record_size = rec_sizes;
	db_dbList[thisNum].num_records = num_rec;
	db_dbList[thisNum].db_type = db_type;
	
	/**determine db name**/
	nameLength = strlen(kDBName) + 1;
	for(i=0;i<num_rec;i++){
		nameLength += strlen(rec_types[i]);
		temp = rec_sizes[i];
		for(j=0;temp != 0;j++) //how many characters in this number?
			temp /= 10;
		nameLength += (j + 2); //include room for spaces
	}

	name = (char*) SafeMalloc (sizeof(char) * nameLength);
	memset(name,NULL,nameLength);
	strcat(name,kDBName);
	for(i=0;i<num_rec;i++){
		strcat(name," ");
		strcat(name,rec_types[i]);
		strcat(name," ");
		temp = rec_sizes[i];
		for(j=0;temp != 0;j++){
			strcat(name,(char)(temp%10));
			temp /= 10;
		}
	}
	/** db name determined **/

	
	db_dbList[thisNum].db_name = name;



	database_setup = db_setup_functions[db_type];

	return thisNum;
}


// Helper DB functions


// Returns the number of bits this type of record takes up
int db_num_bits(RECORDTYPE recname,DB_Table* table){
	int i;
	for(i=0;i<(*table).num_records;i++){
		if(!strcmp(recname,(*table).record_types[i]))
			return (*table).record_size[i];
	}
	return -1;
}

int db_num_bytes(RECORDTYPE recname,DB_Table* table){
	int i;
	int num_bits = db_num_bits(recname,table);
	if(num_bits == -1)
		return -1;
	else
		return num_bits/8 + 1;
}

void db_free(){
    if(db_functions) {
	db_functions->free_db();
	SafeFree(db_functions);
    }
}
