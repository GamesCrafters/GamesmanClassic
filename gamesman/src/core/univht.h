#ifndef GMCORE_UNIVHT_H
#define GMCORE_UNIVHT_H

#include "gamesman.h"

/*
  Type declarations
*/

typedef BOOLEAN (*univht_equal) (void *, void*);
typedef unsigned long long (*univht_hashcode) (void *);
typedef void (*univht_destructor) (void *);

typedef struct _univht_entry{
  void *object;
  struct _univht_entry *chain;
} univht_entry;

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
  univht_entry **table;

} univht;

/*
  Function prototypes
*/

/* Generation of first prime >= number */
unsigned long int find_next_prime(unsigned long int number);

/* Raise b to the e-th power, mod m */
unsigned long int expt(unsigned long int b, unsigned long int e, unsigned long int m);

/* Multiply a and b, mod m */
unsigned long int mul(unsigned long int a, unsigned long int b, unsigned long int m);

/* Add a and b, mod m */
unsigned long int add(unsigned long int a, unsigned long int b, unsigned long int m);

/* Hash-table creation */
univht *univht_create(int slots, float load_factor, univht_equal equal, univht_hashcode hashcode, univht_destructor destructor);

/* Hash-table destruction */
void univht_destroy(univht *ht);

/* Hash-table insertion */
unsigned long int univht_insert(univht *ht, void *object);

/* Hash-table lookup */
void *univht_lookup(univht *ht, void *object);


#endif
