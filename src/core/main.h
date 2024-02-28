
#ifndef GMCORE_MAIN_H
#define GMCORE_MAIN_H

void    Initialize              (void);

void    StartGame               (STRING executableName);
void    SolveAndStore           (void);
VALUE   DetermineValue          (POSITION start);
void    HandleArguments         (int argc, char *argv[]);
int     main                    (int argc, char *argv[]);

/* Real main to be called by wrapper main() or by external linked modules */
int     gamesman_main           (STRING executableName);

#endif /* GMCORE_MAIN_H */
