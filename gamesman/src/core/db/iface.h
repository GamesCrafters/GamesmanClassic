
#ifndef GMCORE_iface_H
#define GMCORE_iface_H


typedef struct iface {

        /* These 7 functions need to be reduced to the 2 that are specified
            in the iface Class. Like this for ease of implementation/switchover
            a single get and put is really all we need. */

  VALUE		(*get_value)(POSITION pos);
  VALUE		(*put_value)(POSITION pos, VALUE data);
  REMOTENESS	(*get_remoteness)(POSITION pos);
  void    	(*put_remoteness)(POSITION pos, REMOTENESS data);
  BOOLEAN     (*check_visited)(POSITION pos);
  void        (*mark_visited)(POSITION pos);
  void        (*unmark_visited)(POSITION pos);
	       
  void        (*free_iface)();
} IFACE_Table;



/* General */ 
void            iface_free();
void            iface_initialize();
VALUE*		GetRawValueFromDatabase		(POSITION pos);	// TODO: Move to misc

/* Value */
VALUE		GetValueOfPosition	(POSITION pos);
VALUE		StoreValueOfPosition	(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	Remoteness		(POSITION pos);
void		SetRemoteness		(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		Visited			    (POSITION pos);
void		MarkAsVisited		(POSITION pos);
void		UnMarkAsVisited		(POSITION pos);
void        UnMarkAllAsVisited  ();

/* Mex */
void		MexStore		(POSITION pos, MEX mex);
MEX		MexLoad			(POSITION pos);


#endif /* GMCORE_iface_H */
