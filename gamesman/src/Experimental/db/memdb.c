
/*
** Memory-based Database Class
** John Jordan (jjjordan@berkeley.edu)
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "gamesman.h"
#include "db_local.h"
#include "bits.h"


/*
** Local types/definitions
*/

struct memdb_internal {
	int		record_size;
	POSITION	record_count;
	char*		data_ptr;
};

static DATABASE_CLASS	MemoryDB;


/*
** Local function prototypes
*/

static int		memdb_initf		( DATABASE*, POSITION, int );
static int		memdb_free		( DATABASE* );
static int		memdb_get_byte		( DATABASE*, POSITION, void* );
static int		memdb_put_byte		( DATABASE*, POSITION, void* );
static int		memdb_putn_byte		( DATABASE*, POSITION, void*, int );
static int		memdb_get_bits		( DATABASE*, POSITION, void* );
static int		memdb_put_bits		( DATABASE*, POSITION, void* );
static int		memdb_putn_bits		( DATABASE*, POSITION, void*, int );
static int		memdb_transfer		( DATABASE*, DATABASE* );
static DATABASE*	memdb_new		( STRING );



/*
** Member functions
*/

/**
 *  This initializes the database with record_count records of size
 *  record_size.  Returns 0 on success.
 */

static int memdb_initf ( DATABASE* db, POSITION record_count, int record_size )
{
	struct memdb_internal*	ptr;
	
	if (db -> internal) {
		ptr = db -> internal;
		if (ptr -> data_ptr)
			safe_free(ptr -> data_ptr);
		safe_free(db -> internal);
	}
	
	db -> internal = safe_malloc(sizeof(struct memdb_internal));
	ptr = (struct memdb_internal*) db -> internal;
	ptr -> data_ptr = (char*) safe_malloc(((record_count >> 3) + 1) * record_size);
	ptr -> record_count = record_count;
	ptr -> record_size = record_size;
	
	table_put_position(&db -> properties, "RecordCount", record_count);
	table_put_int(&db -> properties, "RecordSize", record_size);
	
	/* Optimize if we're dealing with whole bytes */
	
	if (!(ptr -> record_size & 7)) {
		db -> get = memdb_get_byte;
		db -> put = memdb_put_byte;
		db -> putn = memdb_putn_byte;
	} else {
		db -> get = memdb_get_bits;
		db -> put = memdb_put_bits;
		db -> putn = memdb_putn_bits;
	}
	
	return 0;
}


/**
 *  Closes down the database, and frees the memory used.  Returns 0 on succes.
 */

static int memdb_free ( DATABASE* db )
{
	struct memdb_internal* ptr;
	
	if ((ptr = (struct memdb_internal*) db -> internal)) {
		if (ptr -> data_ptr)
			safe_free(ptr -> data_ptr);
		safe_free(db -> internal);
	}
	
	if (db -> properties)
		table_free(&db -> properties);
	
	db -> internal = NULL;
	db -> properties = NULL;
	
	safe_free(db);
	
	return 0;
}


/**
 *  Retrieves an entry p from the database.  This entry will be
 *  stored in the memory pointed to by ptr.  Therefore, ptr should
 *  point to PRE-ALLOCATED space.  Returns 0 on success.
 */

static int memdb_get_bits ( DATABASE* db, POSITION p, void* ptr )
{
	struct memdb_internal*	internal;
	int			sz;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size;
	
	if (p >= internal -> record_count || p < 0)
		return -1;
	
	get_from_octet(sz, OCTET_INDEX(p), &internal -> data_ptr[sz * OCTET_NUMBER(p)], ptr);
	
	return 0;
}


/**
 *  Same as the above, but in the special case that the record_size is a byte
 *  multiple.
 */

static int memdb_get_byte ( DATABASE* db, POSITION p, void* ptr )
{
	struct memdb_internal*	internal;
	int			sz;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size >> 3;
	
	if (p >= internal -> record_count || p < 0)
		return -1;
	
	memcpy(ptr, &internal -> data_ptr[sz * p], sz);
	
	return 0;
}


/**
 *  Stores a record pointed to by ptr in the database with the
 *  key p.  Returns 0 on success.
 */

static int memdb_put_bits ( DATABASE* db, POSITION p, void* ptr )
{
	struct memdb_internal*	internal;
	int			sz;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size;
	
	if (p >= internal -> record_count || p < 0)
		return -1;
	
	put_to_octet(sz, OCTET_INDEX(p), &internal -> data_ptr[sz * OCTET_NUMBER(p)], ptr);
	
	return 0;
}


/**
 *  Same as above, except for the special byte-multiple case.
 */

static int memdb_put_byte ( DATABASE* db, POSITION p, void* ptr )
{
	struct memdb_internal*	internal;
	int			sz;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size >> 3;
	
	if (p >= internal -> record_count || p < 0)
		return -1;
	
	memcpy(&internal -> data_ptr[sz * p], ptr, sz);
	
	return 0;
}


/**
 *  Stores n records pointed to by ptr in the database with the
 *  key p.  Returns 0 on success.
 *
 *  ptr should point to data that is stored in the same manner as it is
 *  actually stored in the database (first entry starts on the MSB of the
 *  first byte).  It is most optimal to align octets (direct copying as
 *  opposed to translating).
 */

static int memdb_putn_bits ( DATABASE* db, POSITION p, void* ptr, int n )
{
	struct memdb_internal*	internal;
	POSITION		i;
	char			buff[128];
	int			sz;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size;
	
	if ((p + n) > internal -> record_count || p < 0 || n < 1)
		return -1;
	
	if (!(p & 7)) {
		/* Aligned to an octet: copy an octet at a time */
		for (i = OCTET_NUMBER(p); n > 7; n -= 8, i++) {
			memcpy(&internal -> data_ptr[sz * i], ptr, sz);
			ptr += sz;
		}
		
		p += (i << 3);
	}
	
	for (i = 0; i < n; i++, p++) {
		get_from_octet(sz, OCTET_INDEX(i), ptr + (sz * OCTET_NUMBER(i)), buff);
		put_to_octet(sz, OCTET_INDEX(p), &internal -> data_ptr[sz * OCTET_NUMBER(p)], buff);
	}
	
	return 0;
}


/**
 *  Same as above, except for the special byte case.
 */

static int memdb_putn_byte ( DATABASE* db, POSITION p, void* ptr, int n )
{
	struct memdb_internal*	internal;
	int			sz;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size >> 3;
	
	if (p >= internal -> record_count || p < 0)
		return -1;
	
	memcpy(&internal -> data_ptr[sz * p], ptr, sz * n);
	
	return 0;
}


/**
 *  Transfers all records from db to dest using dest's Put
 *  method.
 */

static int memdb_transfer ( DATABASE* db, DATABASE* dest )
{
	struct memdb_internal*	internal;
	POSITION		cnt;
	int			sz;
	void*			ptr;
	
	internal = (struct memdb_internal*) db -> internal;
	sz = internal -> record_size;
	cnt = internal -> record_count;
	ptr = internal -> data_ptr;
	
	(*dest -> init)(dest, cnt, sz);
	return (*dest -> putn)(dest, 0, ptr, cnt);
}


/**
 *  Creates a new, empty, uninitialized memory database
 */

static DATABASE* memdb_new ( STRING nm )
{
	DATABASE*	ret;
	
	ret = (DATABASE*) safe_malloc(sizeof(DATABASE));
	ret -> internal = NULL;
	ret -> properties = NULL;
	
	ret -> init = memdb_initf;
	ret -> free = memdb_free;
	ret -> get = memdb_get_bits;
	ret -> put = memdb_put_bits;
	ret -> putn = memdb_putn_bits;
	ret -> transfer = memdb_transfer;
	
	table_put_string(&ret -> properties, "Type", "memory");
	
	return ret;
}


/**
 *  Returns a table of information about the memory db.
 */

DATABASE_CLASS* memdb_init ( void )
{
	MemoryDB . class_name = "memdb";
	MemoryDB . prefix = "memory";
	MemoryDB . properties = NULL;
	MemoryDB . create = memdb_new;
	
	table_put_string(&MemoryDB . properties, "Name", "Memory Database");
	table_put_string(&MemoryDB . properties, "Authors", "John Jordan");
	
	return &MemoryDB;
}

