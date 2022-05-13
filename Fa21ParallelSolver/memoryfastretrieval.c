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

static int getpointerlength(int size)
{
    return (size + 9) >> 3;
}

static void storepointer(int64_t pointer, int size, unsigned char* output) {
    for(int i = 0; i < size; i++) {
        output[i] = pointer;
        pointer >>= 8;
    }
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
    int pointerlength = getpointerlength(size);
    /* Base case: data is 1 byte long. Return that inverse of that unique nonzero value. */
    if (size == 0) {
        return -*data;
    }
    int64_t leftlength = solversavefragment(size - 1, data, output + pointerlength);
    if (leftlength > 0) {
        /* Left tree contains multiple values. */
        int64_t rightlength = solversavefragment(size - 1, data + (1l << (size - 1)), output + pointerlength + leftlength);
        if (rightlength > 0) {
            /* Right tree contains multiple values. */
            storepointer(leftlength, pointerlength, output);
            return leftlength + rightlength + pointerlength;
        } else if (rightlength == 0) {
            /* Right tree contains all zeros, treat right tree as identical to the left tree. */
            storepointer(1l, pointerlength, output);
            return leftlength + pointerlength;
        } else {
            /* Right tree contains a unique nonzero value. */
            int subpointerlength = getpointerlength(size - 1);
            storepointer(leftlength, pointerlength, output);
            storepointer(0, subpointerlength, output + pointerlength + leftlength);
            *(output + pointerlength + leftlength + subpointerlength) = -rightlength;
            return leftlength + pointerlength + subpointerlength + 1l;
        }
    } else if (leftlength == 0) {
        /* Left tree contains all zeros. */
        int64_t rightlength = solversavefragment(size - 1, data + (1l << (size - 1)), output + pointerlength);
        if (rightlength <= 0) {
            /* Right tree contains a unique value (possibly zero). */
            return rightlength;
        }
        /* Right tree contains multiple values, treat left tree as identical to the right tree. */
        storepointer(1l, pointerlength, output);
        return rightlength + pointerlength;
    } else {
        /* Left tree contains a unique nonzero value. */
        int subpointerlength = getpointerlength(size - 1);
        int64_t rightlength = solversavefragment(size - 1, data + (1l << (size - 1)), output + pointerlength + subpointerlength + 1l);
        if (rightlength > 0) {
            /* Right tree contains multiple values. */
            storepointer(subpointerlength + 1l, pointerlength, output);
            storepointer(0l, subpointerlength, output + pointerlength);
            *(output + pointerlength + subpointerlength) = -leftlength;
            return rightlength + pointerlength + subpointerlength + 1l;
        } else if (rightlength == 0) {
            /* Right tree contains all zeros. */
            return leftlength;
        } else {
            /* Right tree contains a unique nonzero value. */
            if (rightlength == leftlength) {
                return leftlength;
            }
            storepointer(subpointerlength + 1l, pointerlength, output);
            storepointer(0l, subpointerlength, output + pointerlength);
            *(output + pointerlength + subpointerlength) = -leftlength;
            storepointer(0l, subpointerlength, output + pointerlength + subpointerlength + 1);
            *(output + pointerlength + (subpointerlength << 1) + 1) = -rightlength;
            return pointerlength + (subpointerlength << 1) + 2;
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
    unsigned char* result = calloc(1l << (data->size - 2), sizeof(unsigned char));
    if (result == NULL) {
        printf("Memory allocation error\n");
        return;
    }
    int length = solversavefragment(data->size, data->data, result);
    fwrite(&(data->size), sizeof(unsigned char), 1, fp);
    if(length <= 0) {
        printf("Compression complete. New length: %d bytes\n", getpointerlength(data->size)+1);
        for(int i = 0; i < getpointerlength(data->size); i++) {
            result[i] = 0;
        }
        result[getpointerlength(data->size)] = -length;
        fwrite(result, sizeof(unsigned char), getpointerlength(data->size)+1, fp);
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
    fread(&ptr, 1, getpointerlength(size), file);
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
