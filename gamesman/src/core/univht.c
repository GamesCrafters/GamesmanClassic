/*
  Author: Mario Tanev @ GamesCrafters
  Credits:
    Professor Marty Weissman (Math 115, Fall 2005) for primaility testing lecture
    Professor Mike Jordan (CS170, Fall 2005) for 2-universal hashing lecture
    Professor Jonathan Shewchuck (CS61B, Spring 2005) for hashing concepts
*/

/* Include support for bignums */
#include <gmp.h>

/* Include support for gamesman constants and types */
#include "gamesman.h"

/* Include support for random generator, malloc and free */
#include <stdlib.h>

/* Include assert support */
#include <assert.h>

/* Include prototypes */
#include "univht.h"

FILE *strdbg = NULL;

univht *univht_create(int slots, float load_factor, univht_equal equal, univht_hashcode hashcode, univht_destructor destructor) {

  void univht_generate_function(univht *ht);

  univht *ht = (univht *) malloc(sizeof(univht));

  /* Set the initial number of entries to 0 */
  ht->entries = 0;

  /* Set the initial number of slots in the hash table */
  ht->slots = slots;

  /* Set the load factor of the hash table */
  ht->load_factor = load_factor;

  /* Set the equality test callback */
  ht->equal = equal;

  /* Set the hashcode producer callback */
  ht->hashcode = hashcode;

  /* Set the object destructor callback */
  ht->destructor = destructor;

  /* Create NULL slots */
  ht->table = (univht_entry **) calloc(ht->slots, sizeof(univht_entry *));

  /* Generate the random function */
  univht_generate_function(ht);

  /* Return newly created hash-table */
  return ht;

}

void univht_generate_function(univht *ht) {

  /* Generate the prime modulus */
  ht->modulus = find_next_prime(ht->slots);

  /* Generate the linear coefficient */
  ht->a = rand() % ht->modulus;

  /* Generate the constant coefficient */
  ht->b = rand() % ht->modulus;

  /* Diagnostic message */
  fprintf(strdbg, "univht: generated randomized function h(x) = %lux + %lu (mod %lu)\n", ht->a, ht->b, ht->modulus);

}

unsigned long int univht_insert(univht *ht, void *object) {

  void univht_resize(univht *ht);
  unsigned long int univht_key(univht *ht, void *object);

  unsigned long int key;
  univht_entry **slot;

  /* Resize the hash table if needed */
  univht_resize(ht);
  
  /* Obtain the key for the object */
  key = univht_key(ht, object);
  
  /* Locate the proper spot in the chain for this entry */
  for (slot = &ht->table[key];
      *slot;
      slot = &((*slot)->chain));

  /* Allocate new hashtable entry and place it in the chain */
  *slot = (univht_entry *) malloc(sizeof(univht_entry));

  /* Set the entry to the given object */
  (*slot)->object = object;

  /* This entry ends the chain, set its chain to NULL */
  (*slot)->chain = NULL;

  /* Increment number of entries in hash-table */
  ht->entries++;

  /* Diagnostic message */
  // fprintf(strdbg, "univht: inserted object with key %d\n", key);

  /* Return key at which entry was inserted */
  return key;

}

void *univht_lookup(univht *ht, void *object) {

  unsigned long int univht_key(univht *ht, void *object);

  unsigned long int key;
  univht_entry **slot;

  /* Obtain the key for the object */
  key = univht_key(ht, object);

  /* Locate the slot in the chain for this entry */
  for (slot = &ht->table[key];
       *slot && !(ht->equal((*slot)->object, object));
       slot = &((*slot)->chain));
  
  /* If slot found, move it to the head of the chain and return the object */
  if (*slot) {
    
    univht_entry *entry;
    
    /* Extract the entry from slot */
    entry = *slot;
    
    /* Set slot to next entry in chain */
    *slot = entry->chain;
    
    /* Make entry's chain point to the head of the chain */
    entry->chain = ht->table[key];
    
    /* Make entry the head of the chain */
    ht->table[key] = entry;
    
    /* Return entry's object */
    return entry->object;
    
  } else {
    
    /* Return NULL if this object was not found in the hash-table */
    return NULL;
    
  }

}

void univht_resize(univht *ht) {

  float load = (float) ht->entries / (float) ht->slots;
  if (load >= ht->load_factor) {
    
    int slot;
    univht *new_ht;
    
    /* Diagnostic message */
    fprintf(strdbg, "univht: load factor of %f reached, resizing database from %lu to %lu slots\n", load, ht->slots, ht->slots * 2);
    
    /* Allocate new hash table to accomodate the moved entries */
    new_ht = univht_create(ht->slots * 2, ht->load_factor, ht->equal, ht->hashcode, ht->destructor);
    
    for (slot = 0; ht->entries; slot++) {
      
      /* Traverse the chain */
      while (ht->table[slot]) {
	
	univht_entry *entry = ht->table[slot];
	
	/* Insert object at entry of chain into new hash table */
	univht_insert(new_ht, entry->object);
	
	/* Link the slot to chain of object moved */
	ht->table[slot] = entry->chain;
	
	/* Free old chain entry */
	free(entry);
	
	/* Decrement number of entries in hash table */
	ht->entries--;
	
      }
      
    }
    
    /* Free slots of hash table */
    free(ht->table);
    
    /* Transfer all fields of new hash table into original hash table
       Fields equal(), hashcode(), load_factor must be invariant */
    
    assert(ht->load_factor == new_ht->load_factor);
    assert(ht->equal == new_ht->equal);
    assert(ht->hashcode == new_ht->hashcode);
    assert(ht->destructor == new_ht->destructor);
    
    ht->slots = new_ht->slots;
    ht->entries = new_ht->entries;
    ht->modulus = new_ht->modulus;
    ht->a = new_ht->a;
    ht->b = new_ht->b;
    ht->table = new_ht->table;
   
    /* Dispose of the new hash table */
    free(new_ht);
  }
}

unsigned long int univht_key(univht *ht, void *object) {

  unsigned long long hashcode, key;

  /* Obtain the hashcode for the object */
  hashcode = ht->hashcode(object);
  
  /* Produce the key */
  key = add(mul(hashcode, ht->a, ht->modulus), ht->b, ht->modulus) % ht->slots;
  //key = (((hashcode % ht->modulus) * ht->a + ht->b) % ht->modulus) % ht->slots;

  /* Diagnostic message */
  //fprintf(strdbg, "univht: created key %u for hashcode %u\n", key, hashcode);

  return key;
  
}

void univht_destroy(univht *ht) {
  int slot;

  for (slot = 0; ht->entries; slot++) {
    
    /* Traverse the chain */
    while (ht->table[slot]) {
      
      univht_entry *entry = ht->table[slot];
      
      /* Link the slot to chain of object moved */
      ht->table[slot] = entry->chain;
      
      /* Invoke entry object destructor */
      ht->destructor(entry->object);
      
      /* Free old chain entry */
      free(entry);
      
      /* Decrement number of entries in hash table */
      ht->entries--;
      
    }
    
  }
  
  /* Free slots of hash table */
  free(ht->table);

  /* Free actual hash table */
  free(ht);

}

/* Raise b to the e-th power, mod m
   Return: b^e (mod m)
*/
unsigned long int expt(unsigned long int b, unsigned long int e, unsigned long int m) {

  mpz_t base, exponent, modulus, result;
  unsigned long int r;

  mpz_init(result);
  mpz_init_set_ui(base, b);
  mpz_init_set_ui(exponent, e);
  mpz_init_set_ui(modulus, m);

  mpz_powm(result, base, exponent, modulus);
  
  r = mpz_get_ui(result);

  mpz_clear(result);
  mpz_clear(base);
  mpz_clear(exponent);
  mpz_clear(modulus);

  return r;
  
}

/* Add a and b, mod m
   Return: a+b (mod m)
*/
unsigned long int add(unsigned long int a, unsigned long int b, unsigned long int m) {

  mpz_t left, right, modulus, result;
  unsigned long int r;

  mpz_init(result);
  mpz_init_set_ui(left, a);
  mpz_init_set_ui(right, b);
  mpz_init_set_ui(modulus, m);

  mpz_add(result, left, right);
  mpz_mod(result, result, modulus);

  r = mpz_get_ui(result);

  mpz_clear(result);
  mpz_clear(left);
  mpz_clear(right);
  mpz_clear(modulus);

  return r;
  
}

/* Add a and b, mod m
   Return: a+b (mod m)
*/
unsigned long int mul(unsigned long int a, unsigned long int b, unsigned long int m) {

 mpz_t left, right, modulus, result;
 unsigned long int r;
 
 mpz_init(result);
 mpz_init_set_ui(left, a);
 mpz_init_set_ui(right, b);
 mpz_init_set_ui(modulus, m);
 
 mpz_mul(result, left, right);
 mpz_mod(result, result, modulus);
 
 r = mpz_get_ui(result);
 
 mpz_clear(result);
 mpz_clear(left);
 mpz_clear(right);
 mpz_clear(modulus);

  return r;

}

BOOLEAN rabin_miller(unsigned long int number, short witness) {

  int i, k;
  int q = number - 1;
  int result;
  BOOLEAN prime = FALSE;

  /* Shift q right and increment k while q is even
     At the end k and q will be such that number - 1 = q*(2^k)
  */
  for (k = 0; !(q & 1); k++, q >>= 1);

  for (i = 0, result = expt(witness, q, number);
       i < k && !prime;
       i++, result = expt(result, 2, number)) {
    
      prime = (result == number - 1) || (result == 1 && i == 0);

  }
  return prime;
  
}

BOOLEAN prime_p(unsigned long int number) {

  int witnesses[] = { 2, 3, 5, 7, 11, 13, 17, 19 };

  /* If number is odd and not equal to 1, perform Rabin-Miller test */
  if ((number & 1) && (number ^ 1)) {

    int witness;      
    
    /* Try with witnesses 2 through min(9, number) */
    for (witness = 0; witness < 8 && witnesses[witness] < number; witness++) {
      
      /* If Rabin-Miller fails on this witness, the number is not prime */
      if (!rabin_miller(number, witnesses[witness])) {
	return FALSE;
      }
    }

    /* If Rabin-Miller didn't fail on any of the witnesses, assume it is prime */
    return TRUE;
    
  } else {

    /* If number is not odd or is 1, it is only prime if it is 2 */
    return (number == 2);

  }

}

unsigned long int find_next_prime(unsigned long int number) {
  
  while (!prime_p(number)) {
    number++;
  }
  return number;
}



/*
BOOLEAN equal(void *left, void *right) {
  
  return (int)left == (int)right;
  
}

unsigned long long hashcode(void *object) {

  return (int)object;

}


int main() {

  strdbg = stderr;
  int i = 2;
  void *a;

  univht *ht = univht_create(1000000, 0.75, equal, hashcode);

  for (i = 0; i < 2000000; i++) {
    int returned;
    int key = univht_insert(ht, (void *)i);
    //fprintf(strdbg, "Inserted %u at key %u\n", i, key);
    returned = (int)univht_lookup(ht, (void *)i);
    if (returned != i) {
      fprintf(strdbg, "Inserted %u, but received %u\n", i, returned);
    }
  }
  univht_destroy(ht);

}
*/
