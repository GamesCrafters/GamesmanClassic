
#ifndef GM_SOLVER_H
#define GM_SOLVER_H

void		solver_init	( void );
SOLVER_CLASS*	solver_get	( STRING );
SOLVER*		solver_new	( STRING, GAME*, DATABASE* );

#endif /* GM_SOLVER_H */
