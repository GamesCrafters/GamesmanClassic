#ifndef TEXTRESOURCE_H_
#define TEXTRESOURCE_H_

#include "ght_hash_table.h"

struct TextResource_struct {
	ght_hash_table_t *parameters;
	ght_hash_table_t *strings;
};

typedef struct TextResource_struct * TextResource;


//public functions

/**
 * Allocates a new text resource object.
 *
 * @param filename The resource file that gets loaded
 *
 * @return pointer to newly allocated TextResource object, NULL if file loading failed.
 */
TextResource TextResource_create(const char* filename);

/**
 * Deallocates this TextResource object, freeing all data in the process
 *
 * @param tr the object to be deallocated
 */
void TextResource_free(TextResource tr);

/**
 * Sets a parameter used for the conditionals when building the string.
 *
 * @param tr the object in question
 * @param key name of the parameter to set
 * @param value the value to set it to, this string is copied, therefore can safely be deallocated
 */
void TextResource_setParameter(TextResource tr, const char* key, const char* value);

/**
 * Evaluate the file for a string based on current parameters.
 *
 * @param tr the object in question
 * @param key the name of the entry in the resource file
 * @param maxSize the size of the return buffer (for the purpose of preventing overruns)
 * @param buffer a pointer to the character buffer that the result is written to
 *
 * @return the number of bytes written. If negative, the output cannot be fit into the buffer and is truncated.
 */
int TextResource_getString(TextResource tr, const char* key, unsigned int maxSize, char* buffer);


#endif /*TEXTRESOURCE_H_*/
