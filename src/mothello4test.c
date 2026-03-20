#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/types.h"
#include "core/blobdb.h"

// Forward declarations from mothellofour.c
POSITION GetCanonicalPosition(POSITION position);
BITBOARD shape(POSITION* p);
uint8_t tier_of(uint64_t shapeMask);
uint64_t hash(POSITION *p);
int owner_of_shape(uint64_t shape, int W);
UINT64 GetInfoFromBlobFile(POSITION p, FILE *f);
VALUE GetPrimitiveFromInfo(UINT64 info);
REMOTENESS GetRemotenessFromInfo(UINT64 info);
void GetBlobFileNameFromPosition(POSITION p, char *filename);

int main() {
    POSITION tests[] = {
        {0b0000010000100000, 0b0000001001000000},
        {0b0000000001000000, 0b0000011100100000},
        {0b0000011100000000, 0b0000000001110000},
        {0b0000000001000000, 0b0000011100110001},
        {0b0000010100110001, 0b0001001001000000},
        {0b0001000001000000, 0b0100011100110001},
        {0b0001000000000000, 0b0100011101111001}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    char filename[256];
    FILE *f;
    UINT64 info;
    VALUE value;
    REMOTENESS remoteness;

    printf("Testing GetInfoFromBlobFile...\n\n");

    for (int k = 0; k < num_tests; k++) {
        POSITION testPos = tests[k];

        printf("==== Test %d ====\n", k);

        GetBlobFileNameFromPosition(testPos, filename);
        printf("File: %s\n", filename);

        f = fopen(filename, "rb");
        if (!f) {
            fprintf(stderr, "Error: Could not open file %s\n\n", filename);
            continue;  // move to next test instead of exiting
        }

        info = GetInfoFromBlobFile(testPos, f);

        value = GetPrimitiveFromInfo(info);
        remoteness = GetRemotenessFromInfo(info);

        printf("Raw info: %llu (0x%llx)\n", 
               (unsigned long long)info, 
               (unsigned long long)info);
        printf("Value: %d\n", value);
        printf("Remoteness: %d\n\n", remoteness);

        fclose(f);
    }

    return 0;
}


// gcc -DTEST_ONLY -DUSE_ENCODING_POSITION -DNO_GRAPHICS \
//   -I/opt/homebrew/include \
//   -L/opt/homebrew/lib \
//   -o test_blob mothello4test.c mothellofour.c \
//   -lzstd