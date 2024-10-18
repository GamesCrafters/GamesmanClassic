#ifndef GMCORE_UNIVHT_H
#define GMCORE_UNIVHT_H

#include "gamesman.h"

/*
   Type declarations
 */

typedef BOOLEAN (*univht_equal)(void *, void*);
typedef unsigned long long (*univht_hashcode)(void *);
typedef void (*univht_destructor)(void *);
typedef void *(*univht_visitor)(void *, void *);

typedef struct {

	/* Number of slots in hash table */
	unsigned long int slots;

	/* Number of entries in hash table */
	unsigned long int entries;

	/* The prime modulus */
	unsigned long int modulus;

	/* The linear coefficient to ax + b */
	unsigned long int a;

	/* The constant coefficient to ax + b */
	unsigned long int b;

	/* Load factor allowed until insert causes resizing */
	float load_factor;

	/* Function to compare two table entries */
	univht_equal equal;

	/* Function to calculate hash code for entry */
	univht_hashcode hashcode;

	/* Function to destroy entry */
	univht_destructor destructor;

	/* Array storage */
	void **table;

	/* Offset of chain within entry object */
	int entry_chain_offset;

	/***
	    Statistical entries
	 ***/

	/* Length of longest chain */
	unsigned long int stat_max_chain_length;

	/* Chain length on average (excludes 0 length chains) */
	double stat_avg_chain_length;

	/* Number of lookups for most looked up entry in table */
	unsigned long int stat_max_entry_lookup;

	/* Average number of lookups for entry in table */
	double stat_avg_entry_lookup;

	/* Entry size in bytes (constant across all entries, but is only used for statistical purposes) */
	unsigned int stat_entry_size;

	/* Number of chains (occupied slots) in hash table */
	unsigned long int stat_chains;

} univht;

/*
   Function prototypes
 */

/* Generation of random prime between number and 2 * number */
unsigned long int find_next_random_prime(unsigned long int number);

/* Raise b to the e-th power, mod m */
unsigned long int expt(unsigned long int b, unsigned long int e, unsigned long int m);

/* Multiply a and b, mod m */
unsigned long int mul(unsigned long int a, unsigned long int b, unsigned long int m);

/* Add a and b, mod m */
unsigned long int add(unsigned long int a, unsigned long int b, unsigned long int m);

/* Hash-table creation */
univht *univht_create(unsigned int slots, float load_factor, univht_equal equal, univht_hashcode hashcode, univht_destructor destructor, unsigned int entry_size, unsigned int entry_chain_offset);

/* Hash-table destruction */
void univht_destroy(univht *ht);

/* Hash-table insertion */
unsigned long int univht_insert(univht *ht, void *entry);

/* Hash-table lookup */
void *univht_lookup(univht *ht, void *object);

/* Hash-table traversal */
void univht_traverse(univht *ht, univht_visitor visitor, void *state);

#endif
