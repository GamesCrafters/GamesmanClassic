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
void    PrintGameValueSummary           (void);
void    PrintDetailedGameValueSummary   (void);
void    PrintDetailedOpenSummary        (void);

/* Analysis output */

void    analyze                         (void);
VALUE   AnalyzePosition                 (POSITION thePosition, VALUE value);
void    AnalysisCollation               (void);
float   DetermineProbability            (POSITION position, VALUE value);
void    writeVarStat                    (STRING statName, STRING text, FILE* out);
void    DatabaseCombVisualization       (void);

/* Analysis save/load */
BOOLEAN LoadAnalysis(void);
void    SaveAnalysis(void);

/* Analysis misc */

void    createAnalysisGameDir           (void);
void    createAnalysisVarDir            (void);
void    writeGameHTML                   (void);
void    createVarTable                  (void);
void    writeVarHTML                    (void);
BOOLEAN CorruptedValuesP                (void);

float   PercentDone                     (STATICMESSAGE msg);
float   PercentLoaded                   (STATICMESSAGE msg);
void    InitializeAnalysis(void);

/* Analysis XML Support */

void    writeXML                        (STATICMESSAGE msg);
void    prepareXMLFile                  (void);
void    prepareXMLVarFile               (void);
void    closeXMLFile                    (void);
void    closeXMLVarFile                 (void);
void    writeXMLData                    (void);
void    writeXMLVarData                 (void);

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

