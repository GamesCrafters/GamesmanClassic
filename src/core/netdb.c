/************************************************************************
**
** NAME:	netdb.c
**
** DESCRIPTION:	Accessor functions for the network database.
**
** AUTHOR:	GamesCrafters Networking Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-04-20
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


**
**  ERROR: is crash out with printf: We will do this for now until we think of something better
**


** Fixme: optimize resolution: dns request: get ip or something
**************************************************************************/

#include <zlib.h>
#include <netinet/in.h>
#include "gamesman.h"
#include "netdb.h"
#include "httpclient.h" //http requests

/*internal declarations and definitions */

#define FILEVER 1

typedef short cellValue;

void            netdb_close                     (){
};                                                    //nothing
/* Value */
VALUE           netdb_get_value                 (POSITION pos);

/* Remoteness */
REMOTENESS      netdb_get_remoteness            (POSITION pos);

/* Mex */
MEX             netdb_get_mex                   (POSITION pos);

/*bulk pos/remoteness request*/
void netdb_get_bulk (POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length);


/* saving to/reading from a file */
BOOLEAN         netdb_load_database             ();

/*LUI replacement algorithm using nth-chance clock*/
/*This would probably be faster as a hash, but for small cache_Size < 100
 * naive looping may be better*/
/*for smart db*/
#define CACHE_SIZE 100
#define CLOCK_LOOPS 5 //number of clock interations
POSITION p_cache[CACHE_SIZE];
cellValue v_cache[CACHE_SIZE];
uint8_t cache_sweep[CACHE_SIZE]; //0 = fresh; CLOCK_LOOPS = replace
short valid_num = 0; //index of first invalid entry
short c_clock =0;         //the clock hand

//cache prototypes:

//FIXME: exact LRU should be used!
//linked list?


//get cached position
BOOLEAN get_position (POSITION pos, cellValue * outcell);


//set cached position:
void set_position (POSITION pos, cellValue cv);

void checkResponseForErrors(httpres *res);

/*
** Code
*/

void netdb_init(DB_Table *new_db)
{
	//set function pointers
	new_db->put_value = NULL;
	new_db->put_remoteness = NULL;
	new_db->mark_visited = NULL;
	new_db->unmark_visited = NULL;
	new_db->put_mex = NULL;
	new_db->free_db = netdb_close;
	new_db->get_value = netdb_get_value;
	new_db->get_remoteness = netdb_get_remoteness;
	new_db->check_visited = NULL;
	new_db->get_mex = netdb_get_mex;
	new_db->get_bulk =  netdb_get_bulk; //bulk request
	new_db->save_database = NULL;
	new_db->load_database = netdb_load_database;
}


void error(char * reason, int code) /*fixme: handle better*/
{
	fprintf(stderr, "\nCRITICAL NETDB ERROR:\n");
	fprintf(stderr, "%s\n",reason);
	exit(1);
}

void badResponseCode(char * reason, int code)
{
	fprintf(stderr, "\nCRITICAL NETDB ERROR:\n");
	fprintf(stderr, "Server responded with HTTP status code: %d\n", code);
	exit(1);
}

//warning: syncronous net sending
//will need to rewrite to be asynch

void netdb_get_raw(POSITION * positions, cellValue * cells, int length){ //dispatch to get cells
	httpreq *req;
	httpres *res;
	char* errMsg;

	//byte found[] = malloc(sizeof(int)*length); //array to note found items
	//fixme: reduce server request based on a bitmap!

	//before we do anything, access the cache:
	BOOLEAN allcached = TRUE;
	int i;
	for (i = 0; i< length; i++) {
		if (!get_position(positions[i], cells + i))
			allcached = FALSE;
	}
	if (allcached) //done
		return;



	char * url = malloc(strlen(ServerAddress)+1);
	memcpy(url,ServerAddress,strlen(ServerAddress)+1);

	char option[32]; //option encoding
	char length_str[32]; //length encoding

	//connect to the server
	if (newrequest(url, &req, &errMsg) != 0)
	{
		fprintf(stderr, "Problem creating request: %s\n", errMsg);
		exit(1);
	}

	settype(req, HD_GET_VALUE_OF_POSITIONS);

	addheader(req, HD_GAME_NAME,  kDBName);

	net_itoa(getOption(),option);
	addheader(req, HD_GAME_VARIANT, option);

	net_itoa(length,length_str);
	addheader(req,HD_LENGTH,length_str);

	//now generate the body:
	POSITION * positions_copy = malloc(sizeof(POSITION)*length);
	//position is 64 bit.. must convert

	for (i=0; i<length; i++) {
		positions_copy[i] = htonll(positions[i]); //convert to net order
	}

	//now post
	if ((post(req,(char*)positions_copy,length*sizeof(POSITION), &res, &errMsg)) != 0)
	{
		fprintf(stderr, "Problem posting to the server: %s\n", errMsg);
		exit(1);
	}

	free (positions_copy);
	//now check errors:

	//debug:
	//printf("%s: %s\n", "Date", getheader(res, "date"));
	//	printf("%s: %s\n", "Content-Length", getheader(res, "Content-Length"));
	//	printf("%s: %s\n", "ReturnCode", getheader(res, HD_RETURN_CODE));
	//	printf("%s: %s\n", "ReturnMessage", getheader(res, HD_RETURN_MESSAGE));

	//normal:
	//FIXME: check status
	char* tmpVal;
	getheader(res,"date",&tmpVal);
	if (tmpVal == NULL) { //all real responses have this
		error("Server did not respond to http request",10);
	}

	char * ecode_str;
	getheader(res,HD_RETURN_CODE,&ecode_str);
	if (ecode_str == NULL) {
		error("Server sent back invalid response",11);
	}
	int ecode = atoi(ecode_str);
	// printf("return code - %d\n",ecode);
	if (ecode != 0) { //error
		getheader(res,HD_RETURN_MESSAGE,&tmpVal);
		error(tmpVal,ecode);
		//can quit
	}

	//verify server not broken
	char * len_str;
	getheader(res,HD_LENGTH,&len_str);
	if (len_str == NULL || length!=atoi(len_str) || res->bodyLength != length*sizeof(cellValue)) {
		error("Server sent back invalid response",10);
	}



	//now safe - just read out the body (length should be the same or we errored)
	cellValue * resvals = (cellValue*)(res->body);

	//must do byte conversion (16 bit)
	for (i=0; i<length; i++) {
		cells[i] = ntohs(resvals[i]);
	}


	for (i = 0; i< length; i++) //cache what we got back
		set_position(positions[i], cells[i]);



	//done - free response
	freeresponse(res);
	free(url);

}

void netdb_init_db()
{
	httpreq *req;
	httpres *res;
	char* errMsg;
	char *url;
	char option[32];

	// Create a new request
	url = malloc(strlen(ServerAddress)+1);
	memcpy(url, ServerAddress, strlen(ServerAddress)+1);
	if (newrequest(url, &req, &errMsg) != 0)
	{
		fprintf(stderr, "Problem creating request: %s\n", errMsg);
		exit(1);
	}
	settype(req, HD_INIT_DATABASE);

	// Set the gamename header
	addheader(req, HD_GAME_NAME,  kDBName);

	// Set the option header
	net_itoa(getOption(), option);
	addheader(req, HD_GAME_VARIANT, option);

	// Now post
	if ((post(req, NULL, 0, &res, &errMsg)) != 0)
	{
		fprintf(stderr, "Problem posting to the server: %s\n", errMsg);
		exit(1);
	}

	// Check for errors:
	checkResponseForErrors(res);

	// Done - free memory
	freeresponse(res);
	free(url);
}


void checkResponseForErrors(httpres *res)
{
	char *ecode_str;
	int ecode;

	// Check HTTP response code
	if (!res->statusCode || res->statusCode != 200)
	{
		// Bad server response
		badResponseCode(res->status, res->statusCode);
	}

	// Check GamesmanServlet return code/message
	getheader(res, HD_RETURN_CODE, &ecode_str);
	if (ecode_str == NULL)
		error("GamesmanServlet sent back invalid response. Missing return code.", 11);
	ecode = atoi(ecode_str);
	if (ecode != 0)
	{
		getheader(res, HD_RETURN_MESSAGE, &ecode_str);
		error(ecode_str, ecode);
	}
}


//cache support:
//return False if not found
//If true, set the cached value appropriately
BOOLEAN get_position (POSITION pos, cellValue * outcell){
	int i;
	for (i=0; i<valid_num; i++) {
		if (p_cache[i] == pos) {
			*outcell = v_cache[i];
			cache_sweep[i] = 0; //reset
			return TRUE;
		}
	}
	return FALSE;
}

//this will push entries to the cache
void set_position (POSITION pos, cellValue cv){
	//check if already cached
	if (get_position(pos,&cv))
		return;

	if (valid_num < CACHE_SIZE) {
		p_cache[valid_num] = pos;
		v_cache[valid_num] = cv;
		cache_sweep[valid_num] = 0;
		valid_num++;
	}
	//replacement (nth-chance clock)
	while (TRUE) {
		if (cache_sweep[c_clock] == CLOCK_LOOPS) { //replace
			cache_sweep[c_clock] = 0; //reset
			p_cache[c_clock] = pos;
			v_cache[c_clock] = cv;
			c_clock=(c_clock+1)%CACHE_SIZE;
			return;
		}
		//otherwise increment clock and sweep counter
		cache_sweep[c_clock]++;
		c_clock=(c_clock+1)%CACHE_SIZE;
	}
}

//FIXME: add a cache clear function


/*bulk request*/

//fine details about caching:
//All entries must be in the cache to abort the request
//Bandwidth is not an issue
//FIXME: This is dumb - refactor

void netdb_get_bulk (POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length){
	if (!length)
		return;
	cellValue * cells = malloc(length*sizeof(cellValue)); //for receiving data
	netdb_get_raw(positions,cells,length);
	int i;
	for (i=0; i<length; i++) {
		ValueArray[i] = ((VALUE)((int)cells[i] & VALUE_MASK));
		remotenessArray[i] = (REMOTENESS)((((int)cells[i] & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
	}
	free(cells);
}


//easy way to issue a single query
//really should be inlined
cellValue netdb_single_query(POSITION pos){
	cellValue cell;

	netdb_get_raw(&pos,&cell,1);

	return cell;
}


VALUE netdb_get_value(POSITION pos)
{
	cellValue cell = netdb_single_query(pos);

	return((VALUE)((int)cell & VALUE_MASK)); /* return pure value */
}

REMOTENESS netdb_get_remoteness(POSITION pos)
{
	cellValue cell = netdb_single_query(pos);

	return (REMOTENESS)((((int)cell & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}


MEX netdb_get_mex(POSITION pos)
{

	cellValue cell = netdb_single_query(pos);

	return (MEX)(((int)cell & MEX_MASK) >> MEX_SHIFT);
}


/*
**	Name: loadDatabase()
**
**      Description:Do things
************
***********/

BOOLEAN netdb_load_database()
{
	printf("\nInitializing net-db for %s...", kGameName);
	fflush(stdout);
	netdb_init_db();
	printf("done.\n");
	return TRUE;
}
