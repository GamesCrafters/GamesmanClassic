
#ifndef GM_MEMDB_H
#define GM_MEMDB_H

/* Value */
VALUE		GetValueOfPosition	(POSITION pos);
VALUE		StoreValueOfPosition	(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	Remoteness		(POSITION pos);
void		SetRemoteness		(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		Visited			(POSITION pos);
void		MarkAsVisited		(POSITION pos);
void		UnMarkAsVisited		(POSITION pos);

/* Mex */
void		MexStore		(POSITION pos, MEX mex);
MEX		MexLoad			(POSITION pos);


#endif /* GM_MEMDB_H */
