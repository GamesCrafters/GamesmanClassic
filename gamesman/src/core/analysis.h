
#ifndef GMCORE_ANALYSIS_H
#define GMCORE_ANALYSIS_H

/* Functions to output sets of data */

void	PrintRawGameValues	(BOOLEAN toFile);
void	PrintBadPositions	(char value, int maxPos, POSITIONLIST* badWinPos,
				 POSITIONLIST* badTiePos, POSITIONLIST* badLosePos);
void	PrintMexValues		(MEX value, int maxpos);
void	PrintValuePositions	(char value, int maxPos);
void	PrintGameValueSummary	();

/* Analysis output */

void	analyze			();
void	analyzer		();
void	writeVarStat		(STRING statName, STRING text, FILE* out);

/* Analysis misc */

void	createAnalysisGameDir	();
void	createAnalysisVarDir	();
void	writeGameHTML		();
void	createVarTable		();
void	writeVarHTML		();
BOOLEAN	CorruptedValuesP	();

float	percentDone		(STATICMESSAGE msg);

#endif /* GMCORE_ANALYSIS_H */

