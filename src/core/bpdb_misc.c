/************************************************************************
**
** NAME:    bpdb_misc.c
**
** DESCRIPTION:    Implementation of miscellaneous functions for the
**                 Bit-Perfect Database
**
** AUTHOR:    Ken Elkabany
**        GamesCrafters Research Group, UC Berkeley
**        Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:    2006-05-01
**
** LICENSE:    This file is part of GAMESMAN,
**        The Finite, Two-person Perfect-Information Game Generator
**        Released under the GPL:
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

#include "bpdb_misc.h"

// create new slist
SLIST slist_new() {
	return NULL;
}

SLIST slist_add(
        SLIST sl,
        void *obj
        )
{
	SLIST cur = sl;
	SLIST temp;

	if(sl == NULL) {
		// if no schemes, add first scheme
		temp = (SLIST) malloc(sizeof(struct singlylinkedlist));
		temp->next = NULL;
		temp->obj = obj;
		return temp;
	} else {
		// find end of list
		while(cur->next != NULL) {
			cur = cur->next;
		}

		// add scheme
		cur->next = (SLIST) malloc(sizeof(struct singlylinkedlist));
		cur->next->next = NULL;
		cur->next->obj = obj;
		return sl;
	}

	// return pointer to start of list
	return sl;
}


UINT32 slist_size(
        SLIST sl
        )
{
	UINT32 size = 0;

	while(NULL != sl) {
		size++;
		sl = sl->next;
	}

	return size;
}

void
slist_free(
        SLIST sl
        )
{
	if( NULL != sl ) {
		slist_free(sl->next);
	}

	SAFE_FREE(sl);
}

// create new slist
HTABLE_SLIST htable_slist_new() {
	return NULL;
}

HTABLE_SLIST htable_slist_add(
        HTABLE_SLIST sl,
        UINT32 key,
        UINT32 value
        )
{
	HTABLE_SLIST cur = sl;
	HTABLE_SLIST temp;

	if(sl == NULL) {
		// if no schemes, add first scheme
		temp = (HTABLE_SLIST) malloc(sizeof(struct hashtable_singlylinkedlist));
		temp->next = NULL;
		temp->key = key;
		temp->value = value;
		return temp;
	} else {
		// find end of list
		while(cur->next != NULL) {
			cur = cur->next;
		}

		// add scheme
		cur->next = (HTABLE_SLIST) malloc(sizeof(struct hashtable_singlylinkedlist));
		cur->next->next = NULL;
		cur->next->key = key;
		cur->next->value = value;
		return sl;
	}

	// return pointer to start of list
	return sl;
}


UINT32 htable_slist_size(
        HTABLE_SLIST sl
        )
{
	UINT32 size = 0;

	while(NULL != sl) {
		size++;
		sl = sl->next;
	}

	return size;
}

void
htable_slist_free(
        HTABLE_SLIST sl
        )
{
	if( NULL != sl ) {
		htable_slist_free(sl->next);
	}

	SAFE_FREE(sl);
}

HTABLE
htable_new(
        UINT32 size
        )
{
	HTABLE ht = calloc( 1, sizeof(struct hashtable) );
	ht->size = size;
	ht->buckets = calloc( size, sizeof(UINT32) );

	return ht;
}

void
htable_add(
        HTABLE ht,
        UINT32 key,
        UINT32 value
        )
{
	UINT32 hkey = ((key + 103699) * 103703) % ht->size;
	ht->buckets[hkey] = htable_slist_add(ht->buckets[hkey], key, value);
}

void
htable_set(
        HTABLE ht,
        UINT32 key,
        UINT32 value
        )
{
	UINT32 hkey = ((key + 103699) * 103703) % ht->size;
	HTABLE_SLIST hs = ht->buckets[hkey];
	BOOLEAN found = FALSE;

	while( NULL != hs) {
		if( hs->key == key ) {
			hs->value = value;
			found = TRUE;
			break;
		}
		hs = hs->next;
	}

	if( !found ) {
		ht->buckets[hkey] = htable_slist_add(ht->buckets[hkey], key, value);
	}
}

BOOLEAN
htable_contains(
        HTABLE ht,
        UINT32 key
        )
{
	UINT32 hkey = ((key + 103699) * 103703) % ht->size;
	HTABLE_SLIST hs = ht->buckets[hkey];
	while( NULL != hs) {
		if( hs->key == key ) return TRUE;
		hs = hs->next;
	}

	return FALSE;
}

UINT32
htable_get(
        HTABLE ht,
        UINT32 key
        )
{
	UINT32 hkey = ((key + 103699) * 103703) % ht->size;
	HTABLE_SLIST hs = ht->buckets[hkey];
	while( NULL != hs) {
		if( hs->key == key ) return hs->value;
		hs = hs->next;
	}

	return 0;
}

void
htable_free(
        HTABLE ht
        )
{
}
