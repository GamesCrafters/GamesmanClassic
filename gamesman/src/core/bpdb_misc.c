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
