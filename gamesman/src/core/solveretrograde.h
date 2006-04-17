#ifndef GMCORE_SOLVERETROGRADE_H
#define GMCORE_SOLVERETROGRADE_H

/* Solver procs */
VALUE DetermineRetrogradeValue(POSITION);
void SolveTier(int);
int SolveWithDelgadilloAlgorithm(POSITION, POSITION, POSITION);

/* Error handling */
BOOLEAN ConfirmAction(char);
void HandleErrorAndExit();
void FileOpenError();
void FileWriteError();
void FileSyntaxError();
void FileCloseError();

/* Reading files */
POSITION readPos(FILE*);
int readSolveFile(FILE*);
void skipLineSolveFile(FILE*);

/* Writing files */
void writeChildrenToFile(FILE*, POSITIONLIST*);
void writeCorruptedWinToFile(FILE*, POSITION, POSITIONLIST*);
void writeCorruptedLoseToFile(FILE*, POSITION, POSITIONLIST*, REMOTENESS);
void writeUnknownToFile(FILE*, POSITION, POSITIONLIST*, REMOTENESS, REMOTENESS, BOOLEAN);

/* Init and Cleanup for files */
void initFiles();
void removeFiles();

/* Solver Progress files */
void SaveProgressToFile(int);
int LoadProgressFromFile();
void SaveDBToFile();
void LoadDBFromFile();

/* HAXX for comparing two databases */
void writeCurrentDBToFile();
void compareTwoFiles(char*, char*);
void skipToNewline(FILE*);

#endif /* GMCORE_SOLVERETROGRADE_H */
