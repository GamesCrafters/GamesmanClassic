#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/types.h"
#include "core/blobdb.h"

#define PAGE_BITS 12
#define N 4
#define CELLS (N * N)
#define FULL ((1ULL << CELLS) - 1ULL)

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

POSITION unhash(BITBOARD sh, uint64_t h) {
#if defined(__BMI2__)
    #include <immintrin.h>
    BITBOARD me = _pdep_u64(h, sh);
    BITBOARD occ = sh;
    POSITION result = { me & FULL, (occ ^ me) & FULL };
    return result;
#else
    BITBOARD me = 0;
    uint64_t m = sh;
    uint64_t src = h;

    while (m) {
        uint64_t b = m & -m;
        if (src & 1ULL) {
            me |= b;
        }
        src >>= 1;
        m ^= b;
    }

    BITBOARD occ = sh;
    POSITION result = { me & FULL, (occ ^ me) & FULL };
    return result;
#endif
}

int main() {
    // POSITION tests[] = {
    //     {0b0000010000100000, 0b0000001001000000},
    //     {0b0000000001000000, 0b0000011100100000},
    //     {0b0000011100000000, 0b0000000001110000},
    //     {0b0000000001000000, 0b0000011100110001},
    //     {0b0000010100110001, 0b0001001001000000},
    //     {0b0001000001000000, 0b0100011100110001},
    //     {0b0001000000000000, 0b0100011101111001}
    // };

    FILE* position_log = fopen("position_log.txt", "rb");
    if (!position_log) {
        fprintf(stderr, "Error: Could not open position_log.txt\n");
        return 1;
    }
    fseek(position_log, 0, SEEK_END);
    long file_size = ftell(position_log);
    fseek(position_log, 0, SEEK_SET);
    int num_tests = file_size / sizeof(POSITION);

    char filename[256];
    FILE *f;
    UINT64 info;
    VALUE value;
    REMOTENESS remoteness;

    // printf("Testing GetInfoFromBlobFile...\n\n");

    for (int k = 0; k < num_tests; k++) {        
        BITBOARD sh;
        uint32_t page_id, last;

        if (fread(&sh, 8, 1, position_log) != 1) {
            fprintf(stderr, "Error: Could not read shape for position %d\n", k);
            fclose(position_log);
            return 1;
        }
        if (fread(&page_id, 4, 1, position_log) != 1) {
            fprintf(stderr, "Error: Could not read page_id for position %d\n", k);
            fclose(position_log);
            return 1;
        }
        if (fread(&last, 4, 1, position_log) != 1) {
            fprintf(stderr, "Error: Could not read last word for position %d\n", k);
            fclose(position_log);
            return 1;
        }

        uint64_t h = ((uint64_t)page_id << PAGE_BITS) | (last & 0xFFF);

        POSITION testPos = unhash(sh, h);

        // printf("==== Test %d ====\n", k);

        GetBlobFileNameFromPosition(testPos, filename);
        // printf("File: %s\n", filename);

        f = fopen(filename, "rb");
        if (!f) {
            fprintf(stderr, "Error: Could not open file %s\n\n", filename);
            continue;  // move to next test instead of exiting
        }

        info = GetInfoFromBlobFile(testPos, f);

        value = GetPrimitiveFromInfo(info);
        remoteness = GetRemotenessFromInfo(info);

        // printf("Raw info: %llu (0x%llx)\n", 
        //        (unsigned long long)info, 
        //        (unsigned long long)info);
        // printf("Value: %d\n", value);
        // printf("Remoteness: %d\n\n", remoteness);

        fclose(f);
    }
    fclose(position_log);

    return 0;
}


// gcc -DTEST_ONLY -DUSE_ENCODING_POSITION -DNO_GRAPHICS \
//   -I/opt/homebrew/include \
//   -L/opt/homebrew/lib \
//   -o test_blob mothello4test.c mothellofour.c \
//   -lzstd