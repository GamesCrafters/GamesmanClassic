#include "bpdb_misc.h"


SCHEME scheme_new(
                UINT32 id,
                UINT8 (*varnum_gap_bits) ( UINT64 consecutiveSkips ),
                UINT8 (*varnum_size_bits) ( UINT8 leftBits ),
                UINT64 (*varnum_implicit_amt) ( UINT8 leftBits ),
                void (*varnum_init) ( ),
                BOOLEAN indicator
                )
{
    SCHEME s = (SCHEME) malloc( sizeof(struct dbscheme) );
    s->id = id;
    s->varnum_gap_bits = varnum_gap_bits;
    s->varnum_size_bits = varnum_size_bits;
    s->varnum_implicit_amt = varnum_implicit_amt;
    s->indicator = indicator;

    if(NULL != varnum_init) {
        varnum_init();
    }

    return s;
}

//void scheme_free( SCHEME s )
//{
//    SAFE_FREE( s );
//}

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
        temp = (SLIST) malloc(sizeof(struct dbscheme));
        temp->next = NULL;
        temp->obj = obj;
        return temp;
    } else {
        // find end of list
        while(cur->next != NULL) {
            cur = cur->next;
        }

        // add scheme
        cur->next = (SLIST) malloc(sizeof(struct dbscheme));
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
