#ifndef GMCORE_TIERDB_H
#define GMCORE_TIERDB_H

/* General */
void    tierdb_init     (DB_Table*);
BOOLEAN tierdb_reinit (DB_Table*);
void tierdb_free_childpositions();
int CheckTierDB     (TIER, int);
BOOLEAN tierdb_load_minifile (char*);

#endif /* GMCORE_TIERDB_H */
