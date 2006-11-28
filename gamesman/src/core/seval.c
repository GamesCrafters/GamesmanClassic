 /************************************************************************
**
** NAME:        seval.c
**
** DESCRIPTION: Static evaluator to be used with gamesman modules.
**
** AUTHOR:      Michael Greenbaum
**              Brian Zimmer
**
** DATE:        Start: 19 September 2006
**
** UPDATE HIST: 
**              9/19/06- Initial draft of seval.
**              10/8/06- (Michael) Implemented 4 trait functions.
**              
**************************************************************************/

#include <scew/scew.h> // Include the XML Parser
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gamesman.h"

/*lBoard will contain board processing data. Requires call
  of LibInitialize by the module. For information see mlib.c and mlib.h.*/
#include "mlib.h"
#include "seval.h"

#define compare(slot,piece) !memcmp(lBoard+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+cols)
#define getRow(slot) (slot/lBoard.numCols)
#define getCol(slot) (slot%lBoard.numRows)
#define min(a,b) (a<=b ? a : b)

fList featureList;

/************************************************************************
**
** Initialization
** 
** -Read in the xml document for the game
** -Query the game for the function pointers for custom traits.
** -Set up array for trait weights, scaling, etc
** 
** Returns FALSE if the initialization failed
** 
************************************************************************/

featureEvaluator getSEvalLibFnPtr(STRING fnName){
  return NULL;
}

fList parse_element(scew_element* element, fList tempList){
    scew_element* child = NULL;
    fList currFeature = NULL;
    fList head = tempList;
    
    printf("Parsing...\n");
    
    if (element == NULL)
    {
        return NULL;
    }
    
    if(strcmp(scew_element_name(element),"feature")==0){
      STRING type = NULL;
      currFeature = SafeMalloc(sizeof(struct fNode));
      //contents = scew_element_contents(element);
      scew_attribute* attribute = NULL;
      
      printf("Parsing a feature...\n");
      
      
      // Default values for feature:
      currFeature->scale = &logistic;    
      currFeature->weight = 1;
      currFeature->perfect = FALSE;

      /**
       * Iterates through the element's attribute list
       */
      attribute = NULL;
      while ((attribute = scew_attribute_next(element, attribute)) != NULL)
      {
      
	printf("Parsing attributes...\n");
      
	if(strcmp(scew_attribute_name(attribute),"name")==0)
	  currFeature->name = (char *)scew_attribute_value(attribute);
	else if(strcmp(scew_attribute_name(attribute),"type")==0)
	  type = (char *)scew_attribute_value(attribute); // Not sure if we have the name yet
	else if(strcmp(scew_attribute_name(attribute),"weight")==0)
	  currFeature->weight = strtod(scew_attribute_value(attribute),NULL);
	else if(strcmp(scew_attribute_name(attribute),"scaling")==0){
	  if(strcmp(scew_attribute_name(attribute),"linear")==0)
	    currFeature->scale = &linear;
	  else if(strcmp(scew_attribute_name(attribute),"logarithmic")==0)
	    currFeature->scale = &logarithmic;
	  else if(strcmp(scew_attribute_name(attribute),"logistic")==0)
	    currFeature->scale = &logistic;
	  else if(strcmp(scew_attribute_name(attribute),"qaudratic")==0)
	    currFeature->scale = &quadratic;
	}
	else if(strcmp(scew_attribute_name(attribute),"perfect")==0)
	  currFeature->perfect = (strcmp(scew_attribute_name(attribute),"true")==0)?TRUE:FALSE;
      }
      
      
      printf("Done Parsing attributes...");
      
      // Now we should have the name
      if( strcmp(type, "custom")==0 )
	currFeature->evalFn = (featureEvaluator)getSEvalCustomFnPtr(currFeature->name);
      else
	currFeature->evalFn = (featureEvaluator)getSEvalLibFnPtr(currFeature->name);
      
      if(currFeature->evalFn == NULL){
	printf("A function for feature \"%s\" could not be found!\n",currFeature->name);
	return NULL;
      }
      
      if(tempList!=NULL)
	tempList->next = currFeature;
      else
	head = tempList = currFeature;
      //contents = scew_element_contents(element);
    }
    
    /**
     * Call parse_element function again for each child of the
     * current element.
     */
    child = NULL;
    while ((child = scew_element_next(element, child)) != NULL)
    {
      if( head==NULL )
	head = parse_element(child, tempList);
      else
	parse_element(child, tempList);
      
      // If we got a feature, advance the feature list
      if(tempList!=NULL && tempList->next!=NULL)
	tempList = tempList->next;
    }
    
    return head;
}

fList loadDataFromXML(STRING fileName){
  scew_tree* tree = NULL;
  scew_parser* parser = NULL;
  fList tempList = NULL;
  /**
   * Creates an SCEW parser. This is the first function to call.
   */
  parser = scew_parser_create();
  
  scew_parser_ignore_whitespaces(parser, 1);
  
  /* Loads an XML file */
  if (!scew_parser_load_file(parser, fileName))
  {
    scew_error code = scew_error_code();
    printf("Unable to load file (error #%d: %s)\n", code,
	   scew_error_string(code));
    if (code == scew_error_expat)
    {
      enum XML_Error expat_code = scew_error_expat_code(parser);
      printf("Expat error #%d (line %d, column %d): %s\n", expat_code,
	     scew_error_expat_line(parser),
	     scew_error_expat_column(parser),
	     scew_error_expat_string(expat_code));
    }
    return FALSE;
  }
  
  tree = scew_parser_tree(parser);
  
  /* Actually get the information */
  tempList = parse_element(scew_tree_root(tree), NULL);
  
  /* Remember to free tree (scew_parser_free does not free it) */
  scew_tree_free(tree);
  
  /* Frees the SCEW parser */
  scew_parser_free(parser);
  
  return tempList;

}

BOOLEAN initializeStaticEvaluator(STRING fileName){
  printf("Reading XML data from: %s\n", fileName);
  featureList = loadDataFromXML(fileName);
  return (featureList==NULL);
}

/************************************************************************
**
** The Evaluator
**
************************************************************************/

VALUE evaluatePosition(POSITION p){
  fList features = featureList;
  float valueSum = 0;
  float weightSum = 0;
  
  features->next = NULL;

  while(features!=NULL){
    valueSum += features->weight * (*(features->evalFn))(p);
    weightSum += features->weight;
    features = features->next;
  }
  
  return ( (valueSum/weightSum)>0.5 );
}

float linear(float value){
  return value;
}

float logarithmic(float value){
  return value;
}

float logistic(float value){
  return value;
}

float quadratic(float value){
  return value;
}

/************************************************************************
**
** Implementation of traits starts here.
**
** Current list:
**
** -Number of pieces (X)
** -Connections (how many pieces "see" each other)
** -NinaRow, AlmostNinaRow.
** -Side locality. (X)
** -Grouping, clustering. (X)
** -Weighting of pieces for above traits.
** -Piece centric traits- distance to piece, connections to piece
** -Slot centric traits- distance, connections, number on slots.
** -Remaining moves (requires module info)
** 
** Possible optimization: Store a list of piece locations for use by
** each instance of static evaluation. There is a lot of redundant 
** comparison to determine piece locations on the board.
**
************************************************************************/

//Returns the number of the given piece on the board.
int NumPieces(void* board,void* piece) {
  /*  int i, count=0;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece)) {
      count++;
    }
  }
  return count;*/
  return 0;
}

/*Returns the sum of the distance of each piece from the given edge of
**the board. 0 = top, 1 = right, 2 = bottom, 3 = left.
*/
int NumFromEdge(void* board,void* piece,int edge) {
  /*int count=0,i;
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece)) {
      if (edge==0) {
	count+= (i/lBoard.cols);
      } else if (edge==1) {
	count+= (lBoard.cols-(i%lBoard.cols));
      } else if (edge==2) {
	connt+= (lBoard.rows-(i/lBoard.cols));
      } else if (edge==3) {
	count+= (i%lBoard.cols);
      } else {
	printf("Error, unknown edge input");
      }
    }
  }
  return count;*/
  return 0;
}

/*Returns a measurement of the clustering of pieces on the board. First
**calculates the centroid, then measures the mean squared distance of all
**pieces from the centroid.
*/
double Clustering(void* board,void* piece) {
  /*int rowAverage=0,colAverage=0,numPieces=0,i=0;
  double value=0;
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece)) {
      rowAverage+=getRow(i);
      colAverage+=getCol(i);
      numPieces++;
    }
  }
  //compute actual averages
  rowAverage = rowAverage/numPieces;
  colAverage = colAverage/numPieces;
  
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece)) {
      value = (getRow(i)-rowAverage)*(getRow(i)-rowAverage) + (getCol(i)-colAverage)*(getCol(i)-colAverage);
    }
  }
  value /= numPieces;
  return value;*/
  return 0;
}

/*Returns the number of connections between pieces on the board.
**A connection is defined as a string of blank spaces, possibly
**diagonal, between two of the given piece.
*/
int Connections(void* board,void* piece,void* blank, BOOLEAN diagonal) {
  /*int count=0,i,j,pathLength,loc;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece)) {
      for(direction = 3;direction<7; direction+= (lBoard.diagonals ? 1 : 2)) {	

	//compute distance to edge of board along direction
	if (direction==3) {
	  pathLength = lBoard.cols-1-getCol(i);
	} else if (direction==4) {
	  pathLength = min(lBoard.cols-1-getCol(i),lBoard.rows-1-getRow(i));
	} else if (direction==5) {
	  pathLength = lBoard.rows-1-getRow(i);
	} else { //direction==6
	  pathLength = min(lBoard.rows-1-getRow(i),getCol(i));
	}
	
	loc =i;
	for(j=0;j<pathLength;j++) {
	  loc+=lBoard.directionMap(direction);
	  if (compare(loc,piece)) {
	    count++;
	    break;
	  } else if (!compare(loc,blank)) {
	    break;
	  }
	}
	
      }
    }
  }
  return count;*/
  return 0;
}
