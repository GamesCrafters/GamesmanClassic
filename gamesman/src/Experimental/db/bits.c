
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


void get_from_octet ( int bits, int idx, char* octet, char* dest )
{
	int	last_bit;
	int	bit_offset;
	int	last_byte;
	int	mask;
	int	bytes;
	int	byte;
	int	i;
	
	bytes = (bits >> 3) + (bits & 7 ? 1 : 0);
	last_bit = bits * (idx + 1) - 1;
	bit_offset = ~last_bit & 7;
	last_byte = last_bit >> 3;
	mask = (1 << bit_offset) - 1;
	
	for (i = bytes; i > 0; i--) {
		byte = i + last_byte - bytes;
		
		dest[i - 1] = (((~mask & octet[byte]) & 0xff) >> bit_offset)
			    | (byte > 0 ? ((mask & octet[byte - 1]) << (8 - bit_offset)) : 0);
	}
	
	if (bits & 7)
		dest[0] &= (1 << (bits & 7)) - 1;
}


void put_to_octet ( int bits, int idx, char* octet, char* src )
{
	int	last_bit;
	int	bit_offset;
	int	last_byte;
	int	mask;
	int	bytes;
	int	byte;
	int	b1;
	int	b2;
	int	i;
	
	bytes = (bits >> 3) + (bits & 7 ? 1 : 0);
	last_bit = bits * (idx + 1) - 1;
	bit_offset = ~last_bit & 7;
	last_byte = last_bit >> 3;
	mask = (1 << bit_offset) - 1;
	
	for (i = bytes; i > 1; i--) {
		byte = i + last_byte - bytes;
		
		octet[byte] = (mask & octet[byte]) | (~mask & (src[i - 1] << bit_offset));
		if (byte > 0)
			octet[byte - 1] = (~mask & octet[byte - 1]) | ((src[i - 1] >> (8 - bit_offset)) & mask);
	}
	
	byte = 1 + last_byte - bytes;
	
	if (bits & 7) {
		mask = ((1 << (bits & 7)) - 1) << bit_offset;
		b1 = mask & 0xff;
		b2 = (mask & 0xff00) >> 8;
		
		octet[byte] = (octet[byte] & ~b1) | ((src[0] << bit_offset) & b1);
		if (b2)
			octet[byte - 1] = (octet[byte - 1] & ~b2) | ((src[0] >> (8 - bit_offset)) & b2);
	} else {
		octet[byte] = src[0];
	}
}
