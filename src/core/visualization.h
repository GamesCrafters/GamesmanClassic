#ifndef GMCORE_VISUALIZATION_H
#define GMCORE_VISUALIZATION_H

#define INI_EDGES_PER_LEVEL 1000

#define LOSE_COLOR "#8B0000"
#define WIN_COLOR  "#00FF00"
#define TIE_COLOR  "#FFFF00"
#define DRAW_COLOR "#FFCC00"
#define DEFAULT_SHAPE "circle"
#define DRAW_SHAPE "diamond"
#define FRINGE_SHAPE "hexagon"

typedef struct tree_edge
{
	POSITION Parent;
	POSITION Child;
} EDGE;

typedef struct edge_list
{
	EDGE **edges;
	POSITION *edgesPerLevel;
	POSITION *nextEdgeInLevel;
	int NumberOfLevels;
	POSITION **nodeRanks;
	POSITION *nextNodeInRank;
	POSITION *nodesPerRank;
	REMOTENESS maxRank;
} EDGELIST;

/* Public functions */
BOOLEAN PrepareTree(EDGELIST *);        // Prepare the in-memory write buffer
void CleanupTree(EDGELIST *);   // Cleanup the in-memory write buffer
void Visualize();               // Visualize game tree
void Write(FILE *, EDGELIST *); // Write a string to a file

/* Private functions */
FILE *PrepareDOTFile();
BOOLEAN PrepareRankList(EDGELIST *);
void CloseDOTFile(FILE *);
STRING PositionColor(POSITION);
STRING PositionShape(POSITION);
STRING PositionValue(POSITION);
STRING MoveColor(EDGE);
void PopulateEdgelist(EDGELIST *);
void WriteLevel(EDGELIST *, int);
void WriteNode(FILE *, POSITION, int, EDGELIST *);
void WriteRanks(FILE *, EDGELIST *);
void WriteBoards();
void UpdateRankList(EDGELIST *, POSITION, REMOTENESS);
#endif
