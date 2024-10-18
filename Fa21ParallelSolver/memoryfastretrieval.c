#include "memory.h"
#include <string.h>


/*Struct defined for solving a game*/
struct solverdata
{
    unsigned char size;
    unsigned char data[];
};

/*Struct defined for playing a game*/
struct playerdata {
    unsigned char size;
    unsigned char data[];
};

/*Initializes the data structure for solver, which acts as a dictionary with
keys as keylen-bit integers and value of one (nonzero) byte each. Returns NULL if error,
and assumes keylen <= 64*/
solverdata* initializesolverdata(int keylen)
{
    solverdata* s = (solverdata*) calloc(1l + (1l << keylen), 1);
    if(!s) {return NULL;}
    s->size = keylen;
    return s;
}

/*Inserts the given (key,val) pair to a solver. It is assumed that any insert either occurs only once
or occurs with the same value for a given key, and assumes that val != 0*/
void solverinsert(solverdata* data, uint64_t key, unsigned char val)
{
    data->data[key] = val;
}

/*Reads a data value at the given key, returning 0 if the value has not been inserted*/
unsigned char solverread(solverdata* data, uint64_t key)
{
    return data->data[key];
}


/* Writes to output the shard of (2^SIZE) length. Returns -n if n is the unique nonzero value in
   the shard, 0 if the shard is empty (contains all zeros), or the number of bytes written
   otherwise.

   Format of this file structure:
   Each block of size 2^n begins with a pointer.
   If pointer == 0, then the next byte contains the unique value stored in the given block.
   If pointer == 1, then left and right blocks are identical, and what follows is the
   structure for a block of size 2^(n-1)
   Otherwise, the left and right blocks are different. Then the pointer lists the length of
   the left block. What follows is the left block, then the right block.
   Since all blocks contain at least one pointer of length at least one byte, and at least
   one byte of data, subblocks are at least 2 bytes long.
*/
static int64_t solversavefragment(int size, unsigned char* data, unsigned char* output) {
    /* Base case: data is 1 byte long. Return that inverse of that unique nonzero value. */
    if (size == 0) {
        return -*data;
    }
    int64_t leftlength = solversavefragment(size - 1, data, output + 1l);
    if (leftlength > 0) {
        /* Left tree contains multiple values. */
        int64_t rightlength = solversavefragment(size - 1, data + (1l << (size - 1)), output + 1l + leftlength);
        if (rightlength > 0) {
            /* Right tree contains multiple values. */
            output[0] = 2;
            return leftlength + rightlength + 1l;
        } else if (rightlength == 0) {
            /* Right tree contains all zeros, treat right tree as identical to the left tree. */
            output[0] = 1;
            return leftlength + 1l;
        } else {
            /* Right tree contains a unique nonzero value. */
            output[0] = 2;
            output[1l + leftlength] = 0;
            *(output + leftlength + 2l) = -rightlength;
            return leftlength + 3l;
        }
    } else if (leftlength == 0) {
        /* Left tree contains all zeros. */
        int64_t rightlength = solversavefragment(size - 1, data + (1l << (size - 1)), output + 1l);
        if (rightlength <= 0) {
            /* Right tree contains a unique value (possibly zero). */
            return rightlength;
        }
        /* Right tree contains multiple values, treat left tree as identical to the right tree. */
        output[0] = 1;
        return rightlength + 1l;
    } else {
        /* Left tree contains a unique nonzero value. */
        int64_t rightlength = solversavefragment(size - 1, data + (1l << (size - 1)), output + 3l);
        if (rightlength > 0) {
            /* Right tree contains multiple values. */
            output[0] = 2;
            output[1] = 0;
            output[2] = -leftlength;
            return rightlength + 3l;
        } else if (rightlength == 0) {
            /* Right tree contains all zeros. */
            return leftlength;
        } else {
            /* Right tree contains a unique nonzero value. */
            if (rightlength == leftlength) {
                return leftlength;
            }
            output[0] = 2;
            output[1] = 0;
            output[2] = -leftlength;
            output[3] = 0;
            output[4] = -rightlength;
            return 5l;
        }
    }
}

/*Writes to the given file (assumed to be opened and readable) the current solver state.
Is not required to leave the solverdata object unmodified.
The output file is designed to be used with the playerdata object,
but only guarantees values for stored keys; any key not set is set to a random value*/
void solversave(solverdata* data, FILE* fp)
{
    /* Why is this safe? */
    /*In any cases we care about, we'll get significant memory improvements anyway.*/
    unsigned char* result = calloc(1l << (data->size-2), sizeof(unsigned char));
    if (result == NULL) {
        printf("Memory allocation error\n");
        return;
    }
    int length = solversavefragment(data->size, data->data, result);
    fwrite(&(data->size), sizeof(unsigned char), 1, fp);
    if(length <= 0) {
        printf("Compression complete. New length: %d bytes\n", 2);
        result[0] = 0;
        result[1] = -length;
        fwrite(result, sizeof(unsigned char), 2, fp);
    } else {
        printf("Compression complete. New length: %d bytes\n", length);
        fwrite(result, sizeof(unsigned char), length, fp);
    }
    free(result);
}

/*Frees a solver*/
void freesolver(solverdata* data) {
    free(data);
}

static void initializesegment(char* data, FILE* file, int size) {
    int64_t ptr = 0;
    fread(&ptr, 1, 1, file);
    //printf("%llx %d\n", ptr, size);
    //fflush(stdout);
    if (ptr == 0) {
        char c;
        fread(&c, 1, 1, file);
        memset(data, c, 1l<<size);
    }
    else if(ptr == 1) {
        initializesegment(data, file, size-1);
        memcpy(data+(1l<<(size-1)), data, (1l<<(size-1)));
    }
    else {
        initializesegment(data, file, size-1);
        initializesegment(data+(1l<<(size-1)), file, size-1);
    }
}

/*Initializes the data structure for a player, read from a given filename*/
playerdata* initializeplayerdata(int keylen, char* filename)
{
    //printf("Here\n");
    //fflush(stdout);
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    playerdata* s = (playerdata*) calloc(1l + (1l << keylen), 1);
    //printf("%p\n", s);
    if(!s) {return NULL;}
    s->size = keylen;
    char size;
    fread(&size, sizeof(unsigned char), 1, file);
    //printf("%d %d\n", keylen, size);
    //fflush(stdout);
    initializesegment(s->data, file, size);
    fclose(file);
    return s;
}


/* Reads a data value at the given key. Returns a garbage value
   if the key had not received a defined value in the corresponding
   solver. */
unsigned char playerread(playerdata* data, uint64_t key) {
    return data->data[key];
}

/*Frees a player*/
void freeplayer(playerdata* data)
{
    free(data);
}

bool verifyPlayerData(solverdata* sd, playerdata* pd) {
    int size = (int)sd->size;
    for (uint64_t i = 0; i < (1ul << size); ++i) {
        if (i % 1000000 == 0) {
            printf("verifying %lld\n", i);
            fflush(stdout);
        }
        unsigned char solverVal = solverread(sd, i);
        if (solverVal != 0) {
            unsigned char playerVal = playerread(pd, i);
            if (solverVal != playerVal) {
                printf("inconsistent value at key %llx: solver value %d and player value %d\n", i, solverVal, playerVal);
                fflush(stdout);
                return false;
            }
        }
    }
    return true;
}
