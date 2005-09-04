
#ifndef GMCORE_MAIN_H
#define GMCORE_MAIN_H

VALUE	DetermineValue		(POSITION start);
void	InitializeDatabases	(void);
void	Initialize		(void);

/* Real main to be called by wrapper main() or by external linked modules */
int		gamesman_main(int argc, char *argv[]);

#endif /* GMCORE_MAIN_H */
