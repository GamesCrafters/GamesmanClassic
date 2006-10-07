#include "bpdb_misc.h"

/* create new scheme list */
Scheme_List scheme_list_new() {
    return NULL;
}

/* add scheme to list */
Scheme_List scheme_list_add(Scheme_List sl, int schemenum, UINT64 (*read_varnum)( dbFILE *inFile, BYTE **curBuffer, BYTE *inputBuffer, UINT32 length, UINT8 *offset, BOOLEAN alreadyReadFirstBit ), BOOLEAN (*write_varnum)( dbFILE *outFile, BYTE **curBuffer, BYTE *outputBuffer, UINT32 length, UINT8 *offset, UINT64 consecutiveSkips ), BOOLEAN indicator) {
    Scheme_List cur = sl;
    Scheme_List temp;

    if(sl == NULL) {
        // if no schemes, add first scheme
        temp = (Scheme_List) malloc(sizeof(struct Schemelist));
        temp->next = NULL;
        temp->scheme = schemenum;
        temp->read_varnum = read_varnum;
        temp->write_varnum = write_varnum;
        temp->indicator = indicator;
        return temp;
    } else {
        // find end of list
        while(cur->next != NULL) {
            cur = cur->next;
        }

        // add scheme
        cur->next = (Scheme_List) malloc(sizeof(struct Schemelist));
        cur->next->next = NULL;
        cur->next->scheme = schemenum;
        cur->next->read_varnum = read_varnum;
        cur->next->write_varnum = write_varnum;
        cur->next->indicator = indicator;
        return sl;
    }
    
    // return pointer to start of list
    return sl;
}

UINT8 scheme_list_size( Scheme_List sl ) {
    UINT8 size = 0;

    while(NULL != sl) {
        size++;
        sl = sl->next;
    }

    return size;
}
