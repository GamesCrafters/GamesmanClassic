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

#define compare(slot,piece,board) !memcmp(board+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+col)
#define getRow(slot) (slot/lBoard.cols)
#define getCol(slot) (slot%lBoard.rows)
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

featureEvaluatorLibrary getSEvalLibFnPtr(STRING fnName){
  return NULL;
}

fList parse_element(scew_element* element, fList tempList){
    scew_element* child = NULL;
    fList currFeature = NULL;
    
    printf("Parsing...\n");
    
    if (element == NULL)
    {
        return NULL;
    }
    
    if(strcmp(scew_element_name(element),"feature")==0){
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
      
	        if(strcmp(scew_attribute_name(attribute),"name")==0){
	          currFeature->name = (char *)SafeMalloc( (strlen(scew_attribute_value(attribute))+1) * sizeof(char));
	          strcpy(currFeature->name, scew_attribute_value(attribute));	  
	        }
	        else if(strcmp(scew_attribute_name(attribute),"type")==0)
	          currFeature->type = strcmp((char *)scew_attribute_value(attribute),"custom")==0? custom : library;
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
	        else if(strcmp(scew_attribute_name(attribute),"piece")==0){
	          if( (char *)scew_attribute_value(attribute)!=NULL && strlen((char *)scew_attribute_value(attribute))==1 )
	            currFeature->piece = ((char *)scew_attribute_value(attribute))[0];
	          else{
	            BadElse("parse_element");
	            printf("Piece must be one char long. Bad piece='%s'", (char *)scew_attribute_value(attribute));
	          }
	        }
	        else{
	          BadElse("parse_element");
	          printf("Bad attribute=\"%s\"",(char *)scew_attribute_value(attribute));
	        }
      }
      
      printf("Done Parsing attributes...");
      
      // Now we should have the name
      if( currFeature->type == custom )
	      currFeature->fEvalC = (featureEvaluatorCustom)getSEvalCustomFnPtr(currFeature->name);
      else
	      currFeature->fEvalL = (featureEvaluatorLibrary)getSEvalLibFnPtr(currFeature->name);
      
      if((currFeature->fEvalC == NULL) && (currFeature->fEvalL == NULL)){
	      printf("A function for feature \"%s\" could not be found!\n",currFeature->name);
	      return NULL;
      }
      
      currFeature->next = tempList;
      tempList = currFeature;
      //contents = scew_element_contents(element);
    }
    
    /**
     * Call parse_element function again for each child of the
     * current element.
     */
    child = NULL;
    while ((child = scew_element_next(element, child)) != NULL)
    {
      tempList = parse_element(child, tempList);
    }
    
    return tempList;
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

float evaluatePosition(POSITION p){
  fList features = featureList;
  float valueSum = 0;
  float weightSum = 0;

  while(features!=NULL){
    if(features->type == library) {
      void* board = (void*) gCustomUnhash(p); //temporary hack
      valueSum += features->weight * features->scale((*(features->fEvalL))(board,features->piece,features->evalParams),
						     features->scaleParams);  
    } else if(features->type == custom) {
      valueSum += features->weight * features->scale((*(features->fEvalC))(p),
						     features->scaleParams);
    } else {
      printf("ERROR, unknown feature type");
    }

    weightSum += features->weight;
    features = features->next;
  }
  
  return (valueSum/weightSum);
}

//Params are {-1 intersect, 1 intersect}
float linear(float value,int params[]){
  return (value < params[0]) ? -1 : ((value > params[1]) ? 1 : value);
}

//Params are {value*10 where y==1 for log10(abs(value*9)+1) (Default=1)}
float logarithmic(float value, int params[]){
  float absValue = abs(value);
  int sign = (int) value / absValue;
  float scaleFactor = (params==NULL || params[0]==0)? 1 : params[0];
  float ans = log10( (absValue*9+1) / scaleFactor );
  return sign * (absValue>1 || ans>1)? 1 : ans;
}

//Params are {shift,time scale} of sigmoid function
float logistic(float value, int params[]){
  float ans = (1/(1+exp(-params[1]*(value-params[0]))));
  return ans;
}

//Params are {isConcaveDown,-1 intersect,1 intersect}
float quadratic(float value, int params[]){
  float ans;
  float width = params[2] - params[1];
  if(params[0]==0) {
    ans = (2/(width*width))*((value - params[1]) * (value - params[1]));
  } else {
    ans = 1 - (2/(width*width))*((params[2] - value) * (params[2] - value));
  }
  
  ans = (value < params[1]) ? -1 : ((value > params[2]) ? 1 : ans);
  
  return ans;
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
  int i, count=0;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece,board)) {
      count++;
    }
  }
  return count;
}

/*Returns the sum of the distance of each piece from the given edge of
**the board. 0 = top, 1 = right, 2 = bottom, 3 = left.
*/
int NumFromEdge(void* board,void* piece,int edge) {
  int count=0,i;
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece,board)) {
      if (edge==0) {
	count+= (i/lBoard.cols);
      } else if (edge==1) {
	count+= (lBoard.cols-(i%lBoard.cols));
      } else if (edge==2) {
	count+= (lBoard.rows-(i/lBoard.cols));
      } else if (edge==3) {
	count+= (i%lBoard.cols);
      } else {
	printf("Error, unknown edge input");
      }
    }
  }
  return count;
}

/*Returns a measurement of the clustering of pieces on the board. First
**calculates the centroid, then measures the mean squared distance of all
**pieces from the centroid.
*/
double Clustering(void* board,void* piece) {
  int rowAverage=0,colAverage=0,numPieces=0,i=0;
  double value=0;
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece,board)) {
      rowAverage+=getRow(i);
      colAverage+=getCol(i);
      numPieces++;
    }
  }
  //compute actual averages
  rowAverage = rowAverage/numPieces;
  colAverage = colAverage/numPieces;
  
  for(i=0;i<lBoard.size;i++) {
    if(compare(i,piece,board)) {
      value = (getRow(i)-rowAverage)*(getRow(i)-rowAverage) + (getCol(i)-colAverage)*(getCol(i)-colAverage);
    }
  }
  value /= numPieces;
  return value;
  return 0;
}

/*Returns the number of connections between pieces on the board.
**A connection is defined as a string of blank spaces, possibly
**diagonal, between two of the given piece.
*/
int Connections(void* board,void* piece,void* blank, BOOLEAN diagonal) {
  int count=0,i,j,pathLength,loc,direction;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece,board)) {
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
	  loc+=lBoard.directionMap[direction];
	  if (compare(loc,piece,board)) {
	    count++;
	    break;
	  } else if (!compare(loc,blank,board)) {
	    break;
	  }
	}
	
      }
    }
  }
  return count;
  return 0;
}
