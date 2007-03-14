#ifndef GMCORE_SOLVERETROGRADE_H
#define GMCORE_SOLVERETROGRADE_H

VALUE DetermineRetrogradeValue(POSITION);
POSITION InitTierGamesman();

void RemoteInitialize();
TIERLIST* RemoteGetTierSolveOrder();
TIERPOSITION RemoteGetTierSize(TIER);
int RemoteGetTierDependencies(TIER);
BOOLEAN RemoteCanISolveTier(TIER);
void RemoteSolveTier(TIER, TIERPOSITION, TIERPOSITION);
BOOLEAN RemoteIsTierReadyToMerge(TIER);
BOOLEAN RemoteMergeToMakeTierDB(TIER);

#endif /* GMCORE_SOLVERETROGRADE_H */
