#include <stdio.h>
#include "gamesman.h"

__attribute__((weak))
void GetBlobFileNameFromPosition(POSITION p, char *filename) {
    (void)p;
    if (filename) filename[0] = '\0';
}

__attribute__((weak))
UINT64 GetInfoFromBlobFile(POSITION p, FILE *f) {
    (void)p; (void)f;
    return 0;
}

__attribute__((weak))
STRING GetPrimitiveFromInfo(UINT64 info) {
    (void)info;
    return 0;
}

__attribute__((weak))
REMOTENESS GetRemotenessFromInfo(UINT64 info) {
    (void)info;
    return 0;
}

__attribute__((weak))
void StartingPositionToString(char* buf) {
    (void)buf;
    return;
}