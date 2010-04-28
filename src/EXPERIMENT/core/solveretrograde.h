#ifndef GMCORE_SOLVERETROGRADE_H
#define GMCORE_SOLVERETROGRADE_H

VALUE DetermineRetrogradeValue(POSITION);
POSITION InitTierGamesman();

// ODeepaBlue (parallelization)
void RemoteInitialize();
TIERLIST* RemoteGetTierSolveOrder();
TIERPOSITION RemoteGetTierSize(TIER);
int RemoteGetTierDependencies(TIER);

BOOLEAN RemoteCanISolveTier(TIER);
void RemoteSolveTier(TIER, TIERPOSITION, TIERPOSITION);
BOOLEAN RemoteMergeToMakeTierDBIfCan(TIER);

BOOLEAN RemoteCanISolveLevelFile(TIER);
void RemoteSolveLevelFile(TIER, TIERPOSITION, TIERPOSITION);
BOOLEAN RemoteMergeToMakeLevelFileIfCan(TIER);

#endif /* GMCORE_SOLVERETROGRADE_H */
