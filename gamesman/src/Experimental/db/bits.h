
#ifndef GM_BITS_H
#define GM_BITS_H

void	get_from_octet	( int, int, char*, char* );
void	put_to_octet	( int, int, char*, char* );

#define	OCTET_NUMBER(p)	((p) >> 3)
#define	OCTET_INDEX(p)	((p) & 7)

#endif /* GM_BITS_H */
