
/*
** Functions to deal with bit addressing
** John Jordan (jjjordan@berkeley.edu)
**
** BIT ADDRESSING:
**
** Databases have record sizes specified in bits, in order to conserve space.
** This type of addressing is -hard-, though, so this file contains functions
** to help along the database writer.
**
** Say the size of a record is 3 bits, we end up with something like this in
** our database:
**
** AAABBBCC CDDDEEEF FFGGGHHH
**
** If it were 7 bits, we end up with this:
**
** AAAAAAAB BBBBBBCC CCCCCDDD DDDDEEEE EEEFFFFF FFGGGGGG GHHHHHHH
**
** Each of the above examples is composed of X bytes (where X is the number of
** bits per entry), and also has 8 entries.  It is easy to see that this
** pattern follows for any size of X bits.  Every 8 entries will be X bytes
** long.  This group of 8 entries/X bytes is called an 'octet'.
**
** Therefore, databases should store all of their entries in these octets
** (groups of 8) since the beginning of an octet will always be aligned to a
** byte.  In order to access the data in these octets, the following two
** functions can be used.
**
** Since we deal with odd numbers of bits (as in not 8), a word should be said
** about how they should be stored outside of their context in octets.  Entries
** will span multiple bytes in big-endian format, and align their LSB with the
** LSB of the last byte.  Examples:
**
**		  	1st byte 2nd byte 3rd byte
**	3  bits:	00000XXX
**	5  bits:	000XXXXX
**	8  bits:	XXXXXXXX
**	11 bits:	00000XXX XXXXXXXX
**	18 bits:	000000XX XXXXXXXX XXXXXXXX
**
** The functions:
**
** get_from_octet(bits, idx, octet, dest)
**	- bits: The number of bits per entry (and the number of bytes per octet)
**	- idx: The index of the entry that we want to retrieve (range: 0 thru 7)
**	- octet: Pointer to the beginning of the octet we want to read from
**	- dest: Pointer to the memory where the entry will be stored (in the
**	        above format)
**
** put_to_octet(bits, idx, octet, src)
**	- bits: Number of bits per entry
**	- idx: The index of the entry we want to set.
**	- octet: Pointer to the beginning of the octet to write to
**	- src: Pointer to the entry to write (in above format)
**
** Note that these functions do _no_ error checking whatsoever.
*/

#include <stdlib.h>
#include <string.h>

#include "gamesman.h"


/*
** Local types / variables
*/

typedef struct {
	int	first_byte;		// last_byte - bytes;
	int	bytes;			// bytes
	int	bit_offset;		// bit_offset
	int	obit_offset;		// ~bit_offset & 7
	int	mask;			// mask
	int	imask;			// ~mask
	int	pmask;			// ((1 << (bits & 7)) - 1) << bit_offset
} bits_data;

static bits_data**	memo_table = NULL;
static int		memo_count = 0;


/*
** Code
*/

static void compute_memo ( int bits )
{
	int	i;
	
	if (memo_count < bits) {
		bits_data**	new_table;
		
		new_table = (bits_data**) safe_malloc(sizeof(bits_data*) * bits);
		memset(new_table, 0, sizeof(bits_data*) * bits);
		memcpy(new_table, memo_table, sizeof(bits_data*) * memo_count);
		free(memo_table);
		memo_table = new_table;
		memo_count = bits;
	}
	
	memo_table[bits - 1] = (bits_data*) safe_malloc(sizeof(bits_data) * 8);
	
	for (i = 0; i < 8; i++) {
		bits_data*	entry = &memo_table[bits - 1][i];
		int		last_bit;
		
		last_bit = bits * (i + 1) - 1;
		entry -> bytes = (bits >> 3) + (bits & 7 ? 1 : 0);
		entry -> bit_offset = ~last_bit & 7;
		entry -> obit_offset = 8 - (entry -> bit_offset);
		entry -> first_byte = (last_bit >> 3) - (entry -> bytes);
		entry -> mask = (1 << (entry -> bit_offset)) - 1;
		entry -> imask = (~(entry -> mask) & 0xff);
		entry -> pmask = ((1 << (bits & 7)) - 1) << (entry -> bit_offset);
	}
}

inline bits_data* get_memo ( int bits, int idx )
{
	if (!memo_table) {
		int	i;
		
		for (i = 32; i > 0; i--)
			compute_memo(i);
	}
	
	if (memo_count < bits || !memo_table[bits - 1])
		compute_memo(idx);
	
	return &memo_table[bits - 1][idx];
}

void get_from_octet ( int bits, int idx, char* octet, char* dest )
{
	bits_data*	tab;
	int		i;
	int		byte;
	int		mask;
	int		imask;
	int		bit_offset;
	int		obit_offset;
	
	tab = get_memo(bits, idx);
	mask = tab -> mask;
	imask = tab -> imask;
	bit_offset = tab -> bit_offset;
	obit_offset = (8 - bit_offset);
	
	for (i = tab -> bytes, byte = tab -> bytes + tab -> first_byte; i > 0; i--, byte--)
		dest[i - 1] = ((imask & octet[byte]) >> bit_offset)
			      | (byte > 0 ? ((mask & octet[byte - 1]) << obit_offset) : 0);
	
	if (bits & 7)
		dest[0] &= (1 << (bits & 7)) - 1;
}


void put_to_octet ( int bits, int idx, char* octet, char* src )
{
	bits_data*	tab;
	int		i;
	int		byte;
	int		bit_offset;
	int		obit_offset;
	int		mask;
	int		imask;
	
	tab = get_memo(bits, idx);
	bit_offset = tab -> bit_offset;
	obit_offset = tab -> obit_offset;
	mask = tab -> mask;
	imask = tab -> imask;
	
	for (i = tab -> bytes, byte = tab -> bytes + tab -> first_byte; i > 1; i--, byte--) {
		octet[byte] = (mask & octet[byte]) | (imask & (src[i - 1] << bit_offset));
		if (byte > 0)
			octet[byte - 1] = (imask & octet[byte - 1]) | ((src[i - 1] >> obit_offset) & mask);
	}
	
	if (bits & 7) {
		unsigned char	b1;
		unsigned char	b2;
		
		b1 = (tab -> pmask & 0xff);
		b2 = (tab -> pmask & 0xff00) >> 8;
		
		octet[byte] = (octet[byte] & ~b1) | ((src[0] << bit_offset) & b1);
		if (b2)
			octet[byte - 1] = (octet[byte - 1] & ~b2) | ((src[0] >> obit_offset) & b2);
	} else
		octet[byte] = src[0];
}

