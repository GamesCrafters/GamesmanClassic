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

//#include <scew/scew.h> // Include the XML Parser
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gamesman.h"

/*lBoard will contain board processing data. Requires call
  of LibInitialize by the module. For information see mlib.c and mlib.h.*/
#include "mlib.h"
#include "seval.h"
//#include <scew/scew.h>

#define compare(slot,piece,board) !memcmp(board+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+col)
#define getRow(slot) (slot/lBoard.cols)
#define getCol(slot) (slot%lBoard.rows)
#define min(a,b) (a<=b ? a : b)

fList featureList;
char* LibraryTraits[] = {"Number of pieces","Distance from edge","Clustering","Connections"};
int numLibraryTraits = 4;

/************************************************************************
**
** TEST
**
************************************************************************/
int numCustomTraits = 2;
char* CustomTraits[] = {"Hell","Yeah"};

int* board;

void* linearUnhash(int p) {
  return (void*) board;
}

//Try something like BlankOX

int XPiece = 2;
int OPiece = 1;
int BlankPiece = 0;

void* getInitialPlayerPiece() {
  return &XPiece;
}

void* getOpponentPlayerPiece() {
  return &OPiece;
}

void* getBlankPiece() {
  return &BlankPiece;
}

int main(int argc,char* argv[]) {
  float value;
  int i;

  NewTraitMenu();

  printf("Static evaluator parameterized\n");
  
  board = malloc(sizeof(int) * 9);

  while(TRUE) {
    
    for(i=0;i<9;i++) {
      scanf("%d",board+i);
    }
    
    value = evaluatePosition(0);
  
  }

  return 0;
}



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
	currFeature->fEvalC = (featureEvaluatorCustom)getSEvalCustomFnPtr(currFeature->name);
      else
	currFeature->fEvalL = (featureEvaluatorLibrary)getSEvalLibFnPtr(currFeature->name);
      
      if((currFeature->fEvalC == NULL) && (currFeature->fEvalL == NULL)){
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
*/
/************************************************************************
**
** The Evaluator
**
************************************************************************/

float evaluatePosition(POSITION p){
  fList features = featureList;
  float valueSum = 0;
  float weightSum = 0;
  
  void* board = (void*) linearUnhash(p); //temporary hack

  while(features!=NULL){
    if(features->type == library) {
      valueSum += features->weight * features->scale((*(features->fEvalL))(board,features->piece,features->evalParams),features->scaleParams);  
    } else if(features->type == custom) {
      valueSum += features->weight * features->scale((*(features->fEvalC))(p),features->scaleParams);
    } else {
      printf("ERROR, unknown feature type");
    }

    weightSum += features->weight;
    features = features->next;
  }
  
  return (valueSum/weightSum);
}

//Params are {-1 intersect, 1 intersect}
float linear(float value,float params[]){
  return (value < params[0]) ? -1 : ((value > params[1]) ? 1 : value);
}

//Params are {value*10 where y==1 for log10(abs(value*9)+1) (Default=1)}
float logarithmic(float value,float params[]){
  float absValue = abs(value);
  int sign = (int) value / absValue;
  float scaleFactor = (params==NULL || params[0]==0)? 1 : params[0];
  float ans = log10( (absValue*9+1) / scaleFactor );
  return sign * (absValue>1 || ans>1)? 1 : ans;
}

//Params are {shift,time scale} of sigmoid function
float logistic(float value,float params[]){
  float ans = (1/(1+exp(-params[1]*(value-params[0]))));
  return ans;
}

//Params are {isConcaveDown,-1 intersect,1 intersect}
float quadratic(float value,float params[]){
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
** Text interface
**
************************************************************************/

void NewTraitMenu() {
  char nextItem;
  char choice;
  int traitNum;

  while(TRUE) {

    printf("\nSelect a new trait to add to the static evaluator\n");
  
    nextItem = printList(LibraryTraits,numLibraryTraits,'a');
    nextItem = printList(CustomTraits,numCustomTraits,nextItem);

    printf("%c) Done\n",nextItem);
    
    while(TRUE) {
      choice = getMyChar();
      traitNum = (int) (choice-97);

      if(choice < 97 || choice > nextItem+1) {
	printf("\nInvalid menu option, select again\n");
      } else {
	break;
      }
    }

    if(choice = nextItem) { //Done
      break;
    } else {
      ParameterizeTrait(traitNum);
    }
  } 
}

fList ParameterizeTrait(int traitNum) {
  fList currFeature = SafeMalloc(sizeof(struct fNode));
  char choice;

  switch(traitNum) {
  case 0:
    RequestPiece(currFeature);
    break;
  case 1:
    RequestPiece(currFeature);
    RequestValue("\nWhich edge should this pertain to?\n\n0) Top\n1) Right\n2) Bottom\nAnything else) Left\n",0,0,currFeature);
    break;
  case 2:
    RequestPiece(currFeature);
    break;
  case 3:
    RequestPiece(currFeature);
    RequestBoolean("Should diagonals be considered?\n",0,currFeature);
    break;
  }    

  if(traitNum > numLibraryTraits-1) {
    currFeature->type = custom;
    currFeature->name = CustomTraits[traitNum-numLibraryTraits];
  } else {
    currFeature->type = library;
    currFeature->name = LibraryTraits[traitNum];
  }

  ParameterizeScalingFunction(currFeature);
  ParameterizeWeightingFunction(currFeature);

  return currFeature;
}

void ParameterizeScalingFunction(fList currFeature) {
  STRING valueChoice;
  char choice;
  
  printf("\nWhat type of scaling function should be used for this trait?\n");
  printf("A scaling function is a function that maps raw trait values to the range [-1,1]");
  printf("\na) Linear - A line clipped at y=1 and y=-1\nb)Logistic - A shifted and scaled sigmoid function\nc)Quadratic - A parabola with vertex at either y=-1 (concave up) or y=1 (concave down)\n");
  
  while(TRUE) {
    choice = getMyChar();
    if(choice < 'a' || choice > 'c') {
      printf("Invalid menu option, select again\n");
    } else {
      break;
    }
  }

  switch(choice) {
  case 'a':
    currFeature->scale = &linear;
    RequestValue("\nEnter value under which the evaluator will assign -1\n",0,1,currFeature);
    RequestValue("\nEnter value above which the evaluator will assign 1\n",1,1,currFeature);
    break;
  case 'b':
    currFeature->scale = &logistic;
    RequestValue("\nEnter time shift of sigmoid function\n",0,1,currFeature);
    RequestValue("\nEnter time scale of sigmoid function\n",1,1,currFeature);
    break;
  case 'c':
    currFeature->scale = &quadratic;
    RequestValue("\nEnter 0 for a concave down function, otherwise function will be concave up\n",0,1,currFeature);
    RequestValue("\nEnter value under which evaluator will assign -1\n",1,1,currFeature);
    RequestValue("\nEnter value above which evaluator will assign 1\n",2,1,currFeature);  
    break;
  }

}

void ParameterizeWeightingFunction(fList currFeature) {
  float choice;
  char temp[80];

  printf("\nEnter the weighting of this trait to be used in calculating the final result\n");
  
  getMyStr(temp,80);
  choice = atof(temp);

  currFeature->weight = choice;
}

char printList(STRING list[],int length, char start) {
  int i;

  for(i=0;i<length;i++) {
    printf("%c) %s",start,list[i]);
    start++;
  }
  return start;
}

void RequestPiece(fList currFeature) {
  char choice;

  printf("\nDoes this pertain to the initial player's piece or opponent player's piece?\n");
  printf("(Type 'o' for opponent, anything else for initial player)\n");

  choice = getMyChar();

  if (choice == 'o') {
    currFeature->piece = getOpponentPlayerPiece();
  } else {
    currFeature->piece = getInitialPlayerPiece();
  }
}

void RequestBoolean(STRING title,int paramNum,fList currFeature) {
  char choice;

  printf(title);
  printf("(0 for false, 1 for true)\n");
 
  while(TRUE) {
    choice = getMyChar();
    if(choice!='0' && choice!='1') {
      printf("Invalid menu option, select again\n");
    } else {
      if(choice=='0') {
	currFeature->evalParams[paramNum] = FALSE;
      } else {
	currFeature->evalParams[paramNum] = TRUE;
      }
      break;
    }
  }   
}

/**
 *Retrieves int input from user.
 *paramType=0 refers to evalParams, paramType=1 refers to scaleParams
 */
void RequestValue(STRING title,int paramNum,int paramType,fList currFeature) {
  int choice;
  float scaleChoice;
  char temp[80];

  printf(title);
 
 
  if(paramType==0) {
    choice = getMyInt();
  } else {
    getMyStr(temp,80);
    scaleChoice = atof(temp);
  }

  if(paramType==0) {
    currFeature->evalParams[paramNum] = choice;
  } else if(paramType==1) {
    currFeature->scaleParams[paramNum] = choice;
  } else {
    printf("ERROR, unknown param type\n");
  }
  
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
float NumPieces(void* board,void* piece, int params[]) {
  int i, count=0;
  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece,board)) {
      count++;
    }
  }
  return (float) count;
}

/*Returns the sum of the distance of each piece from the given edge of
**the board. 0 = top, 1 = right, 2 = bottom, 3 = left.
*/
float NumFromEdge(void* board,void* piece,int params[]) {
  int count=0,i;
  int edge = params[0];

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
  return (float)count;
}

/*Returns a measurement of the clustering of pieces on the board. First
**calculates the centroid, then measures the mean squared distance of all
**pieces from the centroid.
*/
float Clustering(void* board,void* piece,int params[]) {
  int rowAverage=0,colAverage=0,numPieces=0,i=0;
  float value=0;
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
}

/*Returns the number of connections between pieces on the board.
**A connection is defined as a string of blank spaces, possibly
**diagonal, between two of the given piece.
*/
float Connections(void* board,void* piece,int params[]) {
  int count=0,i,j,pathLength,loc,direction;
  BOOLEAN diagonals = params[0];
  void* blank = getBlankPiece();
  
  

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
  return (float) count;
}
