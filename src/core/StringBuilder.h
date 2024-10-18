/*
 * StringBuilder.h
 */
#ifndef __STRINGBUILDER_H__
#define __STRINGBUILDER_H__

#include <stdlib.h>

#define INITIALCAPACITY 10

typedef struct StringBuilderStruct
{
	unsigned int capacity;
	unsigned int length;
	char *string;
} STRINGBUILDER;

STRINGBUILDER* sbMake();
void sbFree(STRINGBUILDER *sb);
unsigned int sbLength(STRINGBUILDER *sb);
unsigned int sbCurrentCapacity(STRINGBUILDER *sb);
char sbGetChar(STRINGBUILDER *sb, unsigned int loc);
char *sbToString(STRINGBUILDER *sb);
void sbEnsureCapacity(STRINGBUILDER *sb, unsigned int additions);
void sbAppendChar(STRINGBUILDER *sb, char newchar);
void sbAppend(STRINGBUILDER *sb, char *newString);

#endif
