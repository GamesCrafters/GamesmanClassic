/*
 * StringBuilder.c
 * A C StringBuilder Analogous to Java's StringBuilder
 */

#include "StringBuilder.h"

/* Creates a STRINGBUILDER structure */
STRINGBUILDER* sbMake()
{
	STRINGBUILDER* sb = (STRINGBUILDER *)malloc(sizeof(STRINGBUILDER));
	sb->capacity = INITIALCAPACITY;
	sb->length = 0;
	sb->string = (char *)malloc(INITIALCAPACITY * sizeof(char));
	return sb;
}

/* Takes a STRINGBUILDER structure and frees the memory */
void sbFree(STRINGBUILDER *sb)
{
	free(sb->string);
	free(sb);
}

/* Flushes the contents of a STRINGBUILDER */
void sbFlush(STRINGBUILDER *sb)
{
	char *oldString = sb->string;
	sb->capacity = INITIALCAPACITY;
	sb->length = 0;
	sb->string = (char *)malloc(INITIALCAPACITY * sizeof(char));
	free(oldString);
}

/* Returns the length of the string in STRINGBUILDER */
unsigned int sbLength(STRINGBUILDER *sb)
{
	return sb->length;
}

/* Returns the current capacity of STRINGBUILDER */
unsigned int sbCurrentCapacity(STRINGBUILDER *sb)
{
	return sb->capacity;
}

/* Returns the character at a specified location of a STRINGBUILDER
   If out of bounds, it returns '\0' */
char sbGetChar(STRINGBUILDER *sb, unsigned int loc)
{
	if(loc < sbLength(sb))
	{
		return sb->string[loc];
	}
	else
	{
		return '\0';
	}
}

/* Returns the char * representation of the STRINGBUILDER */
char *sbToString(STRINGBUILDER *sb)
{
	char *returnString = (char *)malloc((sbLength(sb) + 1) * sizeof(char));
	int i = 0;

	for (i = 0; i < sbLength(sb); i++)
	{
		returnString[i] = sbGetChar(sb, i);
	}
	returnString[i] = '\0';

	return returnString;
}

/* Utiltiy function to ensure that the STRINGBUILDER is large enough.
   Used by AppendChar */
void sbEnsureCapacity(STRINGBUILDER *sb, unsigned int additions)
{
	char *newString;
	char *oldString;
	int i = 0;
	unsigned int newCapacity = sbCurrentCapacity(sb);

	if(sbLength(sb) + additions >= newCapacity)
	{
		while(sbLength(sb) + additions >= newCapacity)
		{
			newCapacity *= 2;
		}
		newString = (char *) malloc(newCapacity * sizeof(char));

		for(i = 0; i < sbCurrentCapacity(sb); i++)
		{
			newString[i] = sbGetChar(sb, i);
		}
		sb->capacity = newCapacity;
		oldString = sb->string;
		sb->string = newString;
		free(oldString);
	}
}

/* Appends NEWCHAR to the end of a string in STRINGBUILDER */
void sbAppendChar(STRINGBUILDER *sb, char newchar)
{
	sbEnsureCapacity(sb, 1);
	sb->string[sbLength(sb)] = newchar;
	sb->length++;
}

/* Takes a NEWSTRING and appends it to the end of a string in STIRNGBUILDER */
void sbAppend(STRINGBUILDER *sb, char *newString)
{
	while(*newString != '\0')
	{
		sbAppendChar(sb, *newString);
		newString++;
	}
}
