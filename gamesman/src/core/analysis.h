
#ifndef GMCORE_ANALYSIS_H
#define GMCORE_ANALYSIS_H

/* Functions to output sets of data */

void	PrintRawGameValues		(BOOLEAN toFile);
void	PrintBadPositions		(char value, int maxPos, POSITIONLIST* badWinPos,
					 POSITIONLIST* badTiePos, POSITIONLIST* badLosePos);
void	PrintMexValues			(MEX value, int maxpos);
void	PrintValuePositions		(char value, int maxPos);
void	PrintGameValueSummary		();

/* Analysis output */

void	analyze				();
void	analyzer            ();
float   DetermineProbability    (POSITION position, VALUE value);
void	writeVarStat			(STRING statName, STRING text, FILE* out);
void	DatabaseCombVisualization	();

/* Analysis misc */

void	createAnalysisGameDir		();
void	createAnalysisVarDir		();
void	writeGameHTML			();
void	createVarTable			();
void	writeVarHTML			();
BOOLEAN	CorruptedValuesP		();

float	PercentDone			(STATICMESSAGE msg);

/* Analysis XML Support */

void	writeXML			(STATICMESSAGE msg);
FILE*	prepareXMLFile			();
void	closeXMLFile			(FILE* xmlFile);
void	writeXMLData			(FILE* xmlFile);

/* Analysis Data Structure */

typedef struct analysis_info
{
  int HashEfficiency;
  float AverageFanout;
  POSITION NumberOfPositions;
  POSITION TotalPositions;
  unsigned long TotalMoves;
  unsigned long WinCount;
  unsigned long LoseCount;
  unsigned long TieCount;
  unsigned long UnknownCount;
  unsigned long PrimitiveWins;
  unsigned long PrimitiveLoses;
  unsigned long PrimitiveTies;
  unsigned int  TimeToSolve;
  
  VALUE InitialPositionValue;
  float InitialPositionProbability;
} ANALYSIS;

extern ANALYSIS gAnalysis;

#endif /* GMCORE_ANALYSIS_H */

