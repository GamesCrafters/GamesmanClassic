/************************************************************************
**
** NAME:	visualization.c
**
** DESCRIPTION: Contains functions to output the game tree in the DOT
				language, allowing it to be visualized using Graphviz.
**
** AUTHORS:	GamesCrafters Research Group, UC Berkeley
			Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-04-11
**
** LICENSE:	This file is part of GAMESMAN,
**			The Finite, Two-person Perfect-Information Game Generator
**			Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gamesman.h"
#include "db.h"
#include "misc.h"
#include "openPositions.h"
#include "visualization.h"


/**
 * Globals
 */

/**
 * Local Variables
 */
FILE *DOTFile;
EDGELIST GameTree;
 
/**
 * Code
 */

BOOLEAN PrepareTree(EDGELIST *tree) {
	int level;
	tree->NumberOfLevels = 1;
	tree->edges = (EDGE **) SafeMalloc(tree->NumberOfLevels*sizeof(EDGE *));
	tree->edgesPerLevel = (POSITION *) SafeMalloc(tree->NumberOfLevels*sizeof(POSITION));
	tree->nextEdgeInLevel = (POSITION *) SafeMalloc(tree->NumberOfLevels*sizeof(POSITION));
	
	for(level = 0; level < tree->NumberOfLevels; level++) {
		tree->edgesPerLevel[level] = INI_EDGES_PER_LEVEL;
		tree->nextEdgeInLevel[level] = 0;
		tree->edges[level] = (EDGE *) SafeMalloc(INI_EDGES_PER_LEVEL*sizeof(EDGE));
		memset((tree->edges)[level], 0, INI_EDGES_PER_LEVEL*sizeof(EDGE));
	}
	UnMarkAllAsVisited();
	return TRUE;	
}

void CleanupTree(EDGELIST *tree) {
	int level;
	SafeFree(tree->edgesPerLevel);
	SafeFree(tree->nextEdgeInLevel);
	
	for(level = 0; level < tree->NumberOfLevels; level++) {
		SafeFree(tree->edges[level]);
	}
	SafeFree(tree->edges);
	UnMarkAllAsVisited();
}

void ResizeTree(EDGELIST *tree, int newSize) {
	printf("\nResizing tree\n");
	printf("Current size: %d, new size: %d\n", tree->NumberOfLevels, newSize+1);
	int level;
	tree->edges = (EDGE **) SafeRealloc(tree->edges, (newSize+1) * sizeof(EDGE *));
	tree->edgesPerLevel = (POSITION *) SafeRealloc(tree->edgesPerLevel, (newSize+1) * sizeof(POSITION));
	tree->nextEdgeInLevel = (POSITION *) SafeRealloc(tree->nextEdgeInLevel, (newSize+1) * sizeof(POSITION));
	
	for(level = tree->NumberOfLevels; level < (newSize+1); level++) {
		tree->edges[level] = (EDGE *) SafeMalloc(INI_EDGES_PER_LEVEL*sizeof(EDGE));
		tree->edgesPerLevel[level] = INI_EDGES_PER_LEVEL;
		tree->nextEdgeInLevel[level] = 0;
	}
	tree->NumberOfLevels = newSize+1;
	printf("Done resizing tree\n");
}

void Visualize() {	
	printf("\nGenerating visualization for %s...", kDBName);
	Stopwatch();
	
	printf("\nPreparing tree");
	PrepareTree(&GameTree);

	printf("\nPreparing file");
	PrepareDOTFile();

	printf("\nFilling tree");
	PopulateEdgelist(&GameTree);

	printf("\nWriting file");
	Write(DOTFile, &GameTree);
	
	printf("\nClosing file");
	CloseDOTFile(DOTFile);
	printf("\nCleaning up tree");
	CleanupTree(&GameTree);
	
	printf("done in %u seconds!", Stopwatch());
	printf("\nVisualization saved to directory \"visualization\\m%s\"", kDBName);
	printf("\nPlease use Graphviz's 'dot' tool to render the DOT files as images.\n");
	return;
}

void PopulateEdgelist(EDGELIST *tree) {
	int level;
	POSITION parent, child;
	MOVELIST *childMoves;
	MOVE theMove;
	OPEN_POS_DATA pdata, cdata;
	int resizeCount = 0;
	int resizeLevelCount = 0;
	
	if(!kLoopy)
		level = 0;
	
	for(parent=0; parent < gNumberOfPositions; parent++) {
		//printf("Getting value of parent %llu\n", parent);
		if(GetValueOfPosition(parent) == undecided &&
			Remoteness(parent) != REMOTENESS_MAX) {
				continue;
		}
		
		if(Primitive(parent) != undecided) {
			//printf("Parent is primitive\n");
			continue;
		} else {
			//printf("Generating moves on parent\n");
			childMoves = GenerateMoves(parent);
		}
		
		while(1) {
			//printf("Doing move\n");
			theMove = childMoves->move;
			child = DoMove(parent, theMove);
			
			if(kLoopy) {
				pdata = GetOpenData(parent);
				cdata = GetOpenData(child);
				level = GetLevelNumber(pdata);
			}
			
			/* If the level is more than the number in tree, resize to handle more levels */

			if(level > (GameTree.NumberOfLevels - 1)) {
				resizeCount++;
				ResizeTree(&GameTree, level);
			}
			
			//printf("Setting edge values\n");
			
			(((GameTree.edges)[level])[(GameTree.nextEdgeInLevel)[level]]).Parent = parent;
			(((GameTree.edges)[level])[(GameTree.nextEdgeInLevel)[level]]).Child = child;
			(GameTree.nextEdgeInLevel)[level] += 1;
			//printf("Next edge: %llu\n", GameTree.nextEdgeInLevel[level]);
			//printf("Edges in level %d: %llu\n", level, GameTree.edgesPerLevel[level]);
			
			/* Resize an individual level if no room left */

			if(((GameTree.edgesPerLevel)[level] - 2) <= (GameTree.nextEdgeInLevel)[level]) {
				//printf("\nResizing level\n");
				//printf("Max: %llu, Current: %llu\n", GameTree.edgesPerLevel[level], GameTree.nextEdgeInLevel[level]);
				resizeLevelCount++;
				(GameTree.edges)[level] = SafeRealloc((GameTree.edges)[level], (GameTree.edgesPerLevel)[level] * 2 * sizeof(EDGE));
				(GameTree.edgesPerLevel)[level] = (GameTree.edgesPerLevel)[level] * 2;
				//printf("Done resizing.\n");
				//printf("Max: %llu, Current: %llu\n", GameTree.edgesPerLevel[level], GameTree.nextEdgeInLevel[level]);
			}
			
			if((childMoves = childMoves->next) == NULL) {
				break;
			}
		}
	}
	printf("Levels resized: %d, tree resized: %d\n", resizeLevelCount, resizeCount);
}

void Write(FILE *fp, EDGELIST *tree) {
	int currentLevel;
		
	for(currentLevel = tree->NumberOfLevels - 1; currentLevel > -1; currentLevel--) {
		WriteLevel(tree, currentLevel);
	}
}

void WriteLevel(EDGELIST *tree, int currentLevel) {
	FILE *fp;
	POSITION currentEdge;
	char filename[80];
	EDGE theEdge;

	sprintf(filename, "visualization/m%s/m%s_%d_level_%d_vis.dot", kDBName, kDBName, getOption(), currentLevel);
	fp = fopen(filename, "w+");
		
	fprintf(fp, "/* Visualization of game tree of %s, variant %d*/\n", kGameName, getOption());
	fprintf(fp, "digraph g {\n");
	fprintf(fp, "\tlabel = \"%s, variant %d\"\n", kGameName, getOption());
	fprintf(fp, "\tlabelloc = \"top\"\n");
	fprintf(fp, "\tranksep = 1\n");
	fprintf(fp, "\tnode [fixedsize = \"true\", width = .75, height = .75]\n");

	fprintf(fp, "\tsubgraph cluster_%d {\n", currentLevel);
	fprintf(fp, "\t\tlabel = \"Level %d\"\n", currentLevel);
	fprintf(fp, "\t\tcolor = \"blue\"\n\n");
	
	/* Draw legend */
	fprintf(fp, "\t\tsubgraph cluster_toc {\n");
	fprintf(fp, "\t\t\trank = \"max\"\n");
	fprintf(fp, "\t\t\tlabel = \"Legend\"\n");
	fprintf(fp, "\t\t\tcolor = \"blue\"\n");
	
	/* Meaning of shapes */
	fprintf(fp, "\t\t\tsubgraph cluster_shape {\n");
	fprintf(fp, "\t\t\t\trank = \"same\"\n");
	fprintf(fp, "\t\t\t\tlabel = \"Position Shapes\"\n");
	fprintf(fp, "\t\t\t\tdefault [shape = \"%s\", label = \"Non-draw\"]\n", DEFAULT_SHAPE);
	fprintf(fp, "\t\t\t\tdraw [shape = \"%s\", label = \"Draw\"]\n", DRAW_SHAPE);
	fprintf(fp, "\t\t\t\tfringe [shape = \"%s\", label = \"Fringe\"]\n", FRINGE_SHAPE);
	fprintf(fp, "\t\t\t}\n");
	
	/* Meaning of colors */
	fprintf(fp, "\t\t\tsubgraph cluster_color {\n");
	fprintf(fp, "\t\t\t\trank = \"same\"\n");
	fprintf(fp, "\t\t\t\tlabel = \"Position Colors\"\n");
	fprintf(fp, "\t\t\t\twin [label = \"Win\", shape = \"%s\", style = \"filled\", color = \"%s\"]\n", DEFAULT_SHAPE, WIN_COLOR);
	fprintf(fp, "\t\t\t\ttie [label = \"Tie\", shape = \"%s\", style = \"filled\", color = \"%s\"]\n", DEFAULT_SHAPE, TIE_COLOR);
	fprintf(fp, "\t\t\t\tlose [label = \"Lose\", shape = \"%s\", style = \"filled\", color = \"%s\"]\n", DEFAULT_SHAPE, LOSE_COLOR);
	fprintf(fp, "\t\t\t\tdraw2 [label = \"Draw\", shape = \"%s\", style = \"filled\", color = \"%s\"]\n", DEFAULT_SHAPE, DRAW_COLOR);
	fprintf(fp, "\t\t\t}\n");
	fprintf(fp, "\t\t}\n");
	
	for(currentEdge = 0; currentEdge < (tree->nextEdgeInLevel)[currentLevel]; currentEdge++) {
		theEdge = (tree->edges)[currentLevel][currentEdge];
		WriteNode(fp, theEdge.Parent, currentLevel);
		WriteNode(fp, theEdge.Child, currentLevel);
				
		fprintf(fp, "\t\t%llu -> %llu [color = \"%s\"]\n", theEdge.Parent, theEdge.Child, MoveColor(theEdge));
	}
	
	fprintf(fp, "\t}\n");
	fprintf(fp, "}\n");
	fclose(fp);
	UnMarkAllAsVisited();
}

void WriteNode(FILE *fp, POSITION node, int level) {
	OPEN_POS_DATA pdata;
	char label[50];
	
	pdata = GetOpenData(node);
	if(!Visited(node)) {
		if(level != GetLevelNumber(pdata)) {
			fprintf(fp, "\t\tsubgraph cluster%llu {\n", node);
			fprintf(fp, "\t\t\tlabel = \" \"\n");
			fprintf(fp, "\t\t\tcolor = \"blue\"\n\t");
			sprintf(label, "\\nlvl %d", GetLevelNumber(pdata));
		} else {
			sprintf(label, " ");
		}
		
		if(node == gInitialPosition) {
			if(level == GetLevelNumber(pdata)) {
				fprintf(fp, "\t\tsubgraph cluster%llu {\n", node);
				fprintf(fp, "\t\t\tcolor = \"blue\"\n\t");
			}
			fprintf(fp, "\t\tlabel = \"Initial Position\"\n\t");
		}
		
		if(GetLevelNumber(pdata) == 0) {
			MarkAsVisited(node);
			fprintf(fp, "\t\t%llu [color = \"%s\", style = \"filled\", shape = \"%s\", label = \"%llu%s\"]", node, PositionColor(node), PositionShape(node), node, label);
		} else if(GetLevelNumber(pdata) > 0) {
			MarkAsVisited(node);
			fprintf(fp, "\t\t%llu [color = \"%s\", style = \"filled\", peripheries = %d, shape = \"%s\", label = \"%llu%s\"]", node, PositionColor(node), GetCorruptionLevel(pdata) + 1, PositionShape(node), node, label);
		} else {
			BadElse("WriteNode");
		}
		//fprintf(fp, "/* Value: %s, level: %d */\n", PositionValue(node), GetLevelNumber(pdata));
		
		if(level != GetLevelNumber(pdata) || node == gInitialPosition) {
			fprintf(fp, "\n\t\t}\n");
		} else {
			fprintf(fp, "\n");
		}
	}
}

STRING PositionValue(POSITION thePosition) {
	VALUE theValue = GetValueOfPosition(thePosition);
	OPEN_POS_DATA pdata;
	int level = 0;
	
	if(kLoopy) {
		pdata = GetOpenData(thePosition);
		level = GetLevelNumber(pdata);
	}
	
	if(level == 0) {
		if(theValue == win) {
			return "win";
		} else if(theValue == lose) {
			return "win";
		} else if(theValue == tie) {
			return "win";
		} else {
			BadElse("PositionValue");
			exit(0);
		}
	} else if(level > 0) {
		theValue = GetDrawValue(pdata);
		if(theValue == win) {
			return "draw win";
		} else if(theValue == lose) {
			return "draw lose";
		} else if(theValue == undecided) {
			return "draw draw";
		} else {
			BadElse("PositionValue");
			exit(0);
		}
	} else {
		BadElse("PositionValue");
		exit(0);
	}
}

STRING PositionColor(POSITION thePosition) {
	VALUE theValue = GetValueOfPosition(thePosition);
	OPEN_POS_DATA pdata;
	int level = 0;
	
	if(kLoopy) {
		pdata = GetOpenData(thePosition);
		level = GetLevelNumber(pdata);
	}
	
	if(level == 0) {
		if(theValue == win) {
			return WIN_COLOR;
		} else if(theValue == lose) {
			return LOSE_COLOR;
		} else if(theValue == tie) {
			return TIE_COLOR;
		} else {
			BadElse("PositionColor");
			exit(0);
		}
	} else if(level > 0) {
		theValue = GetDrawValue(pdata);
		if(theValue == win) {
			return WIN_COLOR;
		} else if(theValue == lose) {
			return LOSE_COLOR;
		} else if(theValue == undecided) {
			return DRAW_COLOR;
		} else {
			BadElse("PositionColor");
			exit(0);
		}
	} else {
		BadElse("PositionColor");
		exit(0);
	}
}

STRING PositionShape(POSITION thePosition) {
	OPEN_POS_DATA pdata;
	int level = 0;
	
	if(kLoopy) {
		pdata = GetOpenData(thePosition);
		level = GetLevelNumber(pdata);
	}
	
	if(level == 0) {
		return DEFAULT_SHAPE;
	} else if(GetFringe(pdata)) {
		return FRINGE_SHAPE;
	} else if(level > 0) {
		return DRAW_SHAPE;
	} else {
		BadElse("PositionShape");
		exit(0);
	}
}

STRING MoveColor(EDGE theEdge) {
	VALUE theValue = GetValueOfPosition(theEdge.Child);
	OPEN_POS_DATA pdata;
	int level = 0;
	
	if(kLoopy) {
		pdata = GetOpenData(theEdge.Child);
		level = GetLevelNumber(pdata);
	}
	
	if(level == 0) {
		if(theValue == win) {
			return LOSE_COLOR;
		} else if(theValue == lose) {
			return WIN_COLOR;
		} else if(theValue == tie) {
			return TIE_COLOR;
		} else {
			BadElse("MoveColor");
			exit(0);
		}
	} else if(level > 0) {
		theValue = GetDrawValue(pdata);
		if(theValue == win) {
			return LOSE_COLOR;
		} else if(theValue == lose) {
			return WIN_COLOR;
		} else if(theValue == undecided) {
			return DRAW_COLOR;
		} else {
			BadElse("MoveColor");
			exit(0);
		}
	} else {
		BadElse("MoveColor");
		exit(0);
	}
}

FILE *PrepareDOTFile() {
	FILE *fp;
	char fileName[256], dirName[256];
	
	sprintf(dirName, "visualization/m%s", kDBName);
	mkdir(dirName, 0755);
	/*
	sprintf(fileName, "visualization/m%s_%d_vis.dot", kDBName, getOption());
	
	if((DOTFile = fopen(fileName, "wb")) == NULL) {
		printf("\nFailed to open DOT file for writing!");
		exit(0);
	}
	*/
	//fprintf(DOTFile, "/* Visualization of game tree of %s, variant %d*/\n", kDBName, getOption());
	//fprintf(DOTFile, "digraph g {\n");
	//fprintf(DOTFile, "\tlabel = \"%s, variant %d\"\n", kDBName, getOption());
	//fprintf(DOTFile, "\tlabelloc = \"top\"\n");
	//fprintf(DOTFile, "\tnode [shape = \"circle\"]\n");
	
	return fp;
}

void CloseDOTFile(FILE *fp) {
	if(fp) {
		printf("\nWriting last line");
		fprintf(DOTFile, "}\n");
		printf("\nActually closing");
		fclose(DOTFile);
	}
}
