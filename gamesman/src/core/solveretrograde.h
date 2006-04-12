#ifndef GMCORE_SOLVERETROGRADE_H
#define GMCORE_SOLVERETROGRADE_H

VALUE DetermineRetrogradeValue(POSITION position);
void initFiles();
void removeFiles();
void SaveDBToFile();
BOOLEAN LoadDBFromFile();
void SolveTier(int tier);
int SolveWithDelgadilloAlgorithm(POSITION tierSize, POSITION numSolved, POSITION numCorrupted);

int tierMax;
char filename[80];
POSITIONLIST* childlist;
REMOTENESS maxUncorruptedWinRem;
BOOLEAN seenDraw;

#endif /* GMCORE_SOLVERETROGRADE_H */