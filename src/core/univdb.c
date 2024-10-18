/************************************************************************
**
** NAME:	univdb.c
**
** DESCRIPTION:
               Implementation of a dynamically-resizable database,
               based on resizable 2-universal hash table
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-11-12
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
#include "univdb.h"
#include "univht.h"
#include "db.h"

univht *ht;

extern FILE *strdbg;

#define MAX_INIT_SLOTS 200

DB_Table *univdb_init() {

	BOOLEAN univdb_equal_entries(void *left, void* right);
	POSITION univdb_hashcode(void *object);
	void univdb_destroy_entry(void *entry);

	POSITION slots;
	DB_Table *db;

	strdbg = stderr;

	db = (DB_Table *) SafeMalloc(sizeof(DB_Table));
	db->get_value = univdb_get_value;
	db->put_value = univdb_put_value;
	db->get_remoteness = univdb_get_remoteness;
	db->put_remoteness = univdb_put_remoteness;
	db->check_visited = univdb_check_visited;
	db->mark_visited = univdb_mark_visited;
	db->unmark_visited = univdb_unmark_visited;
	db->get_mex = univdb_get_mex;
	db->put_mex = univdb_put_mex;
	db->free_db = univdb_free;
	db->save_database = univdb_save_database;
	db->load_database = univdb_load_database;

	/* Decide how many slots the database will have initially.
	   It is the maximum of gNumberOfPosition or MAX_INIT_SLOTS
	 */
	slots = (gNumberOfPositions > MAX_INIT_SLOTS) ? MAX_INIT_SLOTS : gNumberOfPositions;

	/* Create hash table for database */
	ht = univht_create((unsigned long int) slots, 0.75, univdb_equal_entries, univdb_hashcode, univdb_destroy_entry, sizeof(univdb_entry), offsetof(univdb_entry, chain));

	/* Return newly created table of database callbacks */
	return db;

}

BOOLEAN univdb_equal_entries(void *left, void* right) {

	return (((univdb_entry *) left)->position == ((univdb_entry *) right)->position);

}

POSITION univdb_hashcode(void *object) {

	return ((univdb_entry *) object)->position;

}

void univdb_destroy_entry(void *entry) {

	SafeFree(entry);

}

univdb_entry *univdb_create_entry(POSITION position) {

	univdb_entry *entry;

	entry = (univdb_entry *) SafeMalloc(sizeof(univdb_entry));
	entry->position = position;
	entry->flags = undecided;

	/* Insert newly created entry into hash table */
	univht_insert(ht, (void *)entry);

	/* Return newly created entry */
	return entry;
}

univdb_entry *univdb_lookup_entry(POSITION position) {

	univdb_entry entry;

	entry.position = position;
	return (univdb_entry *)univht_lookup(ht, &entry);

}

void univdb_free() {
	/* Destroy hash table */
	univht_destroy(ht);
	fprintf(stderr, "destroying hash\n");
	ht = NULL;
}


VALUE univdb_get_value (POSITION position) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, value is undecided */
	if (entry == NULL) {

		return undecided;

	}
	/* Else extract value from the flags bit-array */
	else {

		return (entry->flags & VALUE_MASK);

	}

}

VALUE univdb_put_value (POSITION position, VALUE value) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, create new one */
	if (entry == NULL) {

		/* Create new hash table entry for position */
		entry = univdb_create_entry(position);

	}

	/* Set new flags to entry to include for updated value */
	entry->flags = (entry->flags & ~VALUE_MASK) | (value & VALUE_MASK);

	/* Return filtered value */
	return (entry->flags & VALUE_MASK);

}

REMOTENESS univdb_get_remoteness (POSITION position) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, remoteness is 0 */
	if (entry == NULL) {

		return 0;

	}

	/* Else extract remoteness from the flags bit-array */
	else {

		return (entry->flags & REMOTENESS_MASK) >> REMOTENESS_SHIFT;

	}

}

void univdb_put_remoteness (POSITION position, REMOTENESS remoteness) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, create new one */
	if (entry == NULL) {

		/* Create new hash table entry for position */
		entry = univdb_create_entry(position);

	}

	/* Set new flags to entry to include for updated remoteness */
	entry->flags = (entry->flags & ~REMOTENESS_MASK) | (remoteness << REMOTENESS_SHIFT);

}

void univdb_put_mex (POSITION position, MEX mex) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, create new one */
	if (entry == NULL) {

		/* Create new hash table entry for position */
		entry = univdb_create_entry(position);

	}

	/* Set new flags to entry to include for updated remoteness */
	entry->flags = (VALUE) ((entry->flags & ~MEX_MASK) | (mex << MEX_SHIFT));

}

MEX univdb_get_mex (POSITION position) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, mex value is 0 */
	/* NOTE: Is that the correct thing to do? */
	if (entry == NULL) {

		return 0;

	}

	/* Else extract remoteness from the flags bit-array */
	else {

		return (MEX) ((entry->flags & MEX_MASK) >> MEX_SHIFT);

	}

}

BOOLEAN univdb_check_visited (POSITION position) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If no entry in hash-table, entry is not visited */
	if (entry == NULL) {

		return FALSE;

	}
	/* Else extract visited mark from the flags bit-array */
	else {

		return (entry->flags & VISITED_MASK) == VISITED_MASK;

	}

}

void univdb_mark_visited (POSITION position) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If entry in hash-table */
	if (entry != NULL) {

		/* Set visited flag */
		entry->flags |= VISITED_MASK;

	}

}

void univdb_unmark_visited (POSITION position) {

	univdb_entry *entry;

	/* Obtain entry from hash-table */
	entry = univdb_lookup_entry(position);

	/* If entry in hash-table */
	if (entry != NULL) {

		/* Unset visited flag */
		entry->flags &= ~VISITED_MASK;

	}

}

BOOLEAN univdb_save_database () {

	return FALSE;

}

BOOLEAN univdb_load_database() {

	return FALSE;

}
