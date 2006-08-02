#ifndef GMCORE_SOLVERETROGRADE_H
#define GMCORE_SOLVERETROGRADE_H

// Solver procs
VALUE DetermineRetrogradeValue(POSITION);
void SolveLoopyTier();
void SolveWithLoopyAlgorithm();
void LoopyParentsHelper(POSITIONLIST*, VALUE, REMOTENESS);

// Error handling
BOOLEAN ConfirmAction(char);
void HandleErrorAndExit();
void FileOpenError();
void FileWriteError();
void FileSyntaxError();
void FileCloseError();

// Solver ChildCounter and Hashtable functions
void rInitFRStuff();
void rFreeFRStuff();
POSITIONLIST* rRemoveFRList(VALUE);
void rInsertFR(VALUE, POSITION, REMOTENESS);

// Reading files
POSITION readPos(FILE*);
//int readSolveFile(FILE*);

// Writing files
void writeChildrenToFile(FILE*, POSITIONLIST*);
void writeCorruptedWinToFile(FILE*, POSITION, POSITIONLIST*);
void writeCorruptedLoseToFile(FILE*, POSITION, POSITIONLIST*, REMOTENESS);
void writeUnknownToFile(FILE*, POSITION, POSITIONLIST*, REMOTENESS, REMOTENESS, BOOLEAN);

// DB Save files
void initDB();
void SaveTierDBToFile();

// Debug
POSITION GetMyPosition();
void debugMenu();

// HAXX for comparing two databases
void writeCurrentDBToFile();
void compareTwoFiles(char*, char*);
void skipToNewline(FILE*);

#endif /* GMCORE_SOLVERETROGRADE_H */
