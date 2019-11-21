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

//headers:
#define HD_LENGTH "length"
#define HD_GAME  "game"
#define HD_VARIANT "variant"
#define HD_TYPE "GetValueOfPositions"
#define HD_RETURN_CODE "ReturnCode"
#define HD_RETURN_MESSAGE "ReturnMessage"

typedef short	cellValue;

void       	netdb_close 			(){}; //nothing
/* Value */
VALUE		netdb_get_value	        	(POSITION pos);

/* Remoteness */
REMOTENESS	netdb_get_remoteness		(POSITION pos);

/* Mex */
MEX		netdb_get_mex			(POSITION pos);

/*bulk pos/remoteness request*/
void netdb_get_bulk (POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length);


/* saving to/reading from a file */
BOOLEAN		netdb_load_database		();

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
  printf("CRITICAL NETDB ERROR:\n");
  printf("%s\n",reason);
  exit(1);
}



//warning: syncronous net sending
//will need to rewrite to be asynch

void netdb_get_raw(POSITION * positions, cellValue * cells, int length){ //dispatch to get cells
  httpreq *req;
  httpres *res;
 
  char option[32]; //option encoding
  char length_str[32]; //length encoding
	
  req = newrequest(ServerAddress); //connect to the server
  settype(req,HD_TYPE); //mr. jacobson: is type lowercase?
  
  addheader(req, HD_GAME,  kDBName);
  
  net_itoa(getOption(),option);
  addheader(req, HD_VARIANT, option);
  
  net_itoa(length,length_str);
  addheader(req,HD_LENGTH,length_str); 

  //now generate the body:
  POSITION * positions_copy = malloc(sizeof(POSITION));
  //position is 64 bit.. must convert
  int i;
  for (i=0;i<length;i++){
    positions_copy[i] = htonll(positions[i]); //convert to net order
  }
  
  //now post
  res = post(req,(char*)positions_copy,length*sizeof(POSITION));

  free (positions_copy);
  //now check errors:

  //debug:
	printf("%s: %s\n", "Date", getheader(res, "date"));
	printf("%s: %s\n", "Content-Length", getheader(res, "Content-Length"));
	printf("%s: %s\n", "ReturnCode", getheader(res, HD_RETURN_CODE));
	printf("%s: %s\n", "ReturnMessage", getheader(res, HD_RETURN_MESSAGE));

     //normal:
	//FIXME: check status
    if (!getheader(res,"Content-Length")){ //all real responses have this
      error("Server did not respond to http request",10);
     }

    char * ecode_str = getheader(res,HD_RETURN_CODE);
    if (!ecode_str){
      error("Server sent back invalid response",11);
    }
    int ecode = atoi(ecode_str);
    printf("return code - %d\n",ecode);
    if (ecode != 0){ //error
       error(getheader(res,HD_RETURN_MESSAGE),ecode);
       //can quit
    }
    
    //verify server not broken
    char * len_str = getheader(res,HD_LENGTH);
    if (!len_str || length!=atoi(len_str) || res->bodyLength != length*sizeof(cellValue)){
      error("Server sent back invalid response",10);
    }

    

    //now safe - just read out the body (length should be the same or we errored)
    cellValue * resvals = (cellValue*)(res->body);

    //must do byte conversion (16 bit)
    for (i=0;i<length;i++){
      cells[i] = ntohs(resvals[i]);
    }
    //done
    
}

/*bulk request*/
void netdb_get_bulk (POSITION* positions, VALUE* ValueArray, REMOTENESS* remotenessArray, int length){
  if (!length)
    return;
  cellValue * cells = malloc(length*sizeof(cellValue)); //for receiving data
  netdb_get_raw(positions,cells,length);
  int i;
  for (i=0;i<length;i++){
    ValueArray[i] = ((VALUE)((int)cells[i] & VALUE_MASK));
    remotenessArray[i] = (REMOTENESS)((((int)cells[i] & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
  }
  free(cells);
}

VALUE netdb_get_value(POSITION pos)
{
        cellValue cell;

	netdb_get_raw(&pos,&cell,1);

        return((VALUE)((int)cell & VALUE_MASK)); /* return pure value */
}

REMOTENESS netdb_get_remoteness(POSITION pos)
{
        cellValue cell;

	netdb_get_raw(&pos,&cell,1);

        return (REMOTENESS)((((int)cell & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
}


MEX netdb_get_mex(POSITION pos)
{

        cellValue cell;

	netdb_get_raw(&pos,&cell,1);


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
	printf("\nNetwork Testing\n"); 
	printf("Using URL: %s\n",ServerAddress);
	//ping or something
	return TRUE; 
}
	//g_Network_DB
