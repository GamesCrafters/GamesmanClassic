#ifndef GMCORE_DB_H
#define GMCORE_DB_H


/************************
** Databases store different types of data of variable width (i.e. size)
** Because each type is uniquely specifed by a number of bits
** maxium compression is accomplished. For Example, if we want to store
** value, which is two bits, we only use two bits to store the value, where
** and how the database stores these two bits of data is up to the database
** so long as it is retrievable by specifying the position and type.
**
** get and put take the same arguments. They do essentially the same thing,
**	1) Find the record for the given position of the given type.
**	2) a)Get takes the data from the internal storage and writes it into the
**		 data pointer.
**	   b)Put takes the data in the data pointer and places it into the
**		 internal storage.
**	3) The data will be aligned to the least significant bit of the data pointer
**		(see bits.c for a visual example)
**  4) Returns 0 on success.
**
*************************/

#define RECORDTYPE char*
#define RECORDSIZE int


typedef struct DB {
  char*			db_name;
  KNOWN_DATABASES db_type;

  int			(*get)(POSITION pos, RECORDTYPE type, char* data);
  int			(*put)(POSITION pos, RECORDTYPE type, char* data);
  
  RECORDTYPE*	record_types;
  RECORDSIZE*	record_size;
  int			num_records;
  int			(*free_db)();

  void*			db_internal_storage;

} DB_Table;

/*************************
** The current types of data and bit width of said data types
** currently supported by gamesman
**/
int			db_num_known_rec_types = 4;
RECORDTYPE* DATA_TYPES = {"value", "remoteness", "visited", "mex"};
RECORDSIZE* DATA_SIZES = {		2,			  8,		 1,		8};


/*************************
** Current databases supported and implemented
*************************/
typedef enum known_db_enum
{
	MemDB, FileDB //MemDB = 0, FileDB = 1
} KNOWN_DATABASES;

void(*fn)(void) db_setup_functions = {memdb_setup(DB_Table*),filedb_setup(DB_Table*)};


/* General */ 
void            db_free();
void            db_initialize();

/**************
**	known database setup functions
**  These should be the ONLY external functions available from the actual
**  DB implementation. (Needed or otherwise).
**  This is the reason there is no filedb.h or memdb.h
**************/
void			memdb_setup(DB_Table*);
void			filedb_setup(DB_Table*);

#endif /* GMCORE_DB_H */
