#ifndef GMCORE_DB_H
#include "db.h"
#endif

#ifndef GMCORE_ANALYSIS_H
#define GMCORE_ANALYSIS_H
/*
   #ifndef ORIGINAL_PUT_VALUE_PTR
   VALUE   (*original_put_value) (POSITION pos, VALUE val);
   #define ORIGINAL_PUT_VALUE_PTR
   #endif
 */

#define ANALYSIS_FILE_VER 4
#define PRIMITIVE_INTERESTINGNESS 0.01

/* Functions to output sets of data */

void    PrintRawGameValues              (BOOLEAN toFile);
void    PrintBinaryGameValuesToFile     (char * filename);
void    PrintBadPositions               (char value, int maxPos, POSITIONLIST* badWinPos,
                                         POSITIONLIST* badTiePos, POSITIONLIST* badLosePos);
void    PrintMexValues                  (MEX value, int maxpos);
void    PrintValuePositions             (char value, int maxPos);
void    PrintGameValueSummary           ();
void    PrintDetailedGameValueSummary();
void    PrintDetailedOpenSummary();

/* Analysis output */

void    analyze                         ();
VALUE   AnalyzePosition(POSITION thePosition, VALUE value);
void    AnalysisCollation();
float   DetermineProbability    (POSITION position, VALUE value);
void    writeVarStat                    (STRING statName, STRING text, FILE* out);
void    DatabaseCombVisualization       ();

/* Analysis save/load */
BOOLEAN LoadAnalysis();
void    SaveAnalysis();

/* Analysis misc */

void    createAnalysisGameDir           ();
void    createAnalysisVarDir            ();
void    writeGameHTML                   ();
void    createVarTable                  ();
void    writeVarHTML                    ();
BOOLEAN CorruptedValuesP                ();

float   PercentDone                     (STATICMESSAGE msg);
float   PercentLoaded                   (STATICMESSAGE msg);
void    InitializeAnalysis();

/* Analysis XML Support */

void    writeXML                        (STATICMESSAGE msg);
void    prepareXMLFile                  ();
void    prepareXMLVarFile               ();
void    closeXMLFile                    ();
void    closeXMLVarFile                 ();
void    writeXMLData                    ();
void    writeXMLVarData                 ();

/* Interestingness */
void DetermineInterestingness(POSITION position);
void DetermineInterestingnessDFS(POSITION position);

/* Analysis Data Structure */

typedef struct analysis_info
{
	POSITION NumberOfPositions;
	POSITION TotalPositions;
	POSITION TotalMoves;
	POSITION TotalPrimitives;

	POSITION WinCount;
	POSITION LoseCount;
	POSITION TieCount;
	POSITION Draws;

	POSITION UnknownCount;
	POSITION PrimitiveWins;
	POSITION PrimitiveLoses;
	POSITION PrimitiveTies;

	POSITION F0EdgeCount;
	POSITION F0NodeCount;
	POSITION F0DrawEdgeCount;

	VALUE InitialPositionValue;
	REMOTENESS LargestFoundRemoteness;
	unsigned int TimeToSolve;
	int HashEfficiency;
	float AverageFanout;
	float InitialPositionProbability;

	POSITION DetailedPositionSummary[REMOTENESS_MAX+1][3]; // Table for counting wins(1) and losses(2) and ties(3)
	// at each remoteness between 0 and REMOTENESS_MAX-1.
	POSITION DetailedOpenSummary[11][11][REMOTENESS_MAX+1][4]; /* [Level][Corruption][FRemoteness][drawvalue] */
	POSITION OpenSummary[4];

	POSITION DrawWinCount;
	POSITION DrawLoseCount;
	POSITION DrawTieCount;
	POSITION DrawDraws;
	POSITION TotalOpenPositions;

	REMOTENESS LargestFoundLevel;
	REMOTENESS LargestFoundFRemoteness;
	REMOTENESS LargestFoundCorruption;

	float* Interestingness;
	float MaxInterestingness;
	POSITION MostInteresting;

} ANALYSIS;

extern ANALYSIS gAnalysis;

extern UINT32 gValueSlot;

#endif /* GMCORE_ANALYSIS_H */

