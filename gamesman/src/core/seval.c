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
#include <scew/scew.h>

#define compare(slot,piece,board) !memcmp(board+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+col)
#define getRow(slot) (slot/lBoard.cols)
#define getCol(slot) (slot%lBoard.rows)
#define min(a,b) (a<=b ? a : b)

seList evaluatorList;
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
/*
int* board;

void* linearUnhash(int p) {
  return (void*) board;
}

//Try something like BlankOX

int XPiece = 2;
int OPiece = 1;
int BlankPiece = 0;

int main(int argc,char* argv[]) {
  float value;
  int i;

  InitializeStaticEvaluator(4,3,3,0,&XPiece,&OPiece,&BlankPiece);

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
*/


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
  
  if(strcmp(fnName,"Number of Pieces")) {
    return &numPieces;
  } else if (strcmp(fnName,"Distance from Edge")) {
    return &numFromEdge;
  } else if (strcmp(fnName,"Clustering")) {
    return &(clustering);
  } else if (strcmp(fnName,"Connections")) {
    return &(connections);
  } else {
    printf("ERROR, unrecognized trait name in xml file");
    return NULL;
  }
  
}

STRING copyString(STRING toCopy){
  STRING toReturn = SafeMalloc((strlen(toCopy)+1) * sizeof(char));
  strcpy(toReturn, toCopy);
  return toReturn;
}

void freeFeatureList(fList head){
  if( head==NULL )
    return;
  SafeFree(head->name);
  freeFeatureList(head->next);
  SafeFree(head);
}

void freeEvaluatorList(seList head){
  if( head==NULL )
    return;
  if( head->featureList!=NULL && head->featureList!=featureList )
    freeFeatureList(head->featureList);
  SafeFree(head->name);
  freeEvaluatorList(head->next);
  SafeFree(head);
}

void chooseEvaluator(int numInEvaluatorList){
  seList temp = evaluatorList;
  while( numInEvaluatorList-- > 0 ) temp = temp->next;
  featureList = temp->featureList;
  printf("\nThe Evaluator has been set to %s\n", temp->name);
  gSEvalLoaded = TRUE;
}

BOOLEAN evaluatorExistsForVariant(int variant) {
  seList temp = evaluatorList;
  while( temp!=NULL ){
    if( temp->variant == -1 || temp->variant == variant )
      return TRUE;
    temp = temp->next;
  }
  
  return FALSE;
}

fList parseFeature(scew_element* sEvalNode){
    scew_element* element = NULL;
    fList tempList = NULL;
    while ((element = scew_element_next(sEvalNode, element)) != NULL){
      fList currFeature = SafeMalloc(sizeof(struct fNode));
      scew_attribute* attribute = NULL;
      
      //printf("Parsing a feature...\n");
      
      
      // Default values for feature:
      currFeature->scale = &linear;    
      currFeature->weight = 1;

      /**
       * Iterates through the element's attribute list
       */

      attribute = NULL;
      while ((attribute = scew_attribute_next(element, attribute)) != NULL)
      {
      
	      //printf("Parsing attributes...\n");
      
	      if(strcmp(scew_attribute_name(attribute),"name")==0) 
      	  currFeature->name = copyString((char *)scew_attribute_value(attribute));
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
      	else if(strcmp(scew_attribute_name(attribute),"piece")==0) {
      	  if( (char *)scew_attribute_value(attribute)!=NULL && strlen((char *)scew_attribute_value(attribute))==1 ){
      	    STRING attrValue = (char *)scew_attribute_value(attribute);
      	    currFeature->piece = (void*) &attrValue[0];
      	  }
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
      
      
      //printf("Done Parsing attributes...");
      
      // Now we should have the name
      if( currFeature->type == custom && gGetSEvalCustomFnPtr!=NULL)
	      currFeature->fEvalC = (featureEvaluatorCustom)gGetSEvalCustomFnPtr(currFeature->name);
      else
	      currFeature->fEvalL = (featureEvaluatorLibrary)getSEvalLibFnPtr(currFeature->name);
      
      if((currFeature->fEvalC == NULL) && (currFeature->fEvalL == NULL)) {
	       printf("A function for feature \"%s\" could not be found!\n",currFeature->name);
	       return NULL;
      }
      
      currFeature->next = tempList;
      tempList = currFeature;
      //contents = scew_element_contents(element);
    }
    
    return tempList;
}

seList parseEvaluators(scew_element* element, seList tempList) {
    scew_element* child = NULL;
    seList currEvaluator = NULL;
    
    //printf("Parsing...\n");
    
    if (element == NULL)
    {
        return NULL;
    }
    
    if(strcmp(scew_element_name(element),"seval")==0) {
      currEvaluator = SafeMalloc(sizeof(struct seNode));
      scew_attribute* attribute = NULL;
      currEvaluator->name = NULL;
      currEvaluator->element = element;
      currEvaluator->variant = -1;
      currEvaluator->perfect = FALSE;
      while ((attribute = scew_attribute_next(element, attribute)) != NULL)
      {
	      if(strcmp(scew_attribute_name(attribute),"name")==0) 
      	  currEvaluator->name = copyString((char *)scew_attribute_value(attribute));
      	else if(strcmp(scew_attribute_name(attribute),"variant")==0)
      	  currEvaluator->variant = atoi((char *)scew_attribute_value(attribute));
      	else if(strcmp(scew_attribute_name(attribute),"perfect")==0){
      	  currEvaluator->perfect = (strcmp(scew_attribute_name(attribute),"true")==0)?TRUE:FALSE;
      	  if(currEvaluator->perfect) gSEvalPerfect = TRUE;
      	}
      	else{
      	  BadElse("parseEvaluators");
      	  printf("Bad attribute=\"%s\"",(char *)scew_attribute_value(attribute));
      	}
      }
      
      if( currEvaluator->name==NULL )
        currEvaluator->name = copyString("default");
      
      currEvaluator->featureList = parseFeature(element);
    
      currEvaluator->next = tempList;
      tempList = currEvaluator;
    }
    
    /**
     * Call parse_element function again for each child of the
     * current element.
     */
    
    child = NULL;
    while ((child = scew_element_next(element, child)) != NULL)
    {
      tempList = parseEvaluators(child, tempList);
    }
    
    return tempList;
}

seList loadDataFromXML(STRING fileName) {
  scew_tree* tree = NULL;
  scew_parser* parser = NULL;
  seList tempList = NULL;
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
  tempList = parseEvaluators(scew_tree_root(tree), NULL);
  
  /* Remember to free tree (scew_parser_free does not free it) */
  scew_tree_free(tree);
  
  /* Frees the SCEW parser */
  scew_parser_free(parser);
  
  return tempList;

}

BOOLEAN createNewXMLFile(STRING fileName) { 
  scew_tree* tree = scew_tree_create();
  scew_element* root = scew_tree_add_root(tree, "game");
  scew_element_add_attr_pair(root, "name", kDBName);
  return scew_writer_tree_file(tree, fileName);
}

scew_element* findEvaluatorNode(scew_element* root, STRING evaluatorName) {
  scew_element* child = NULL;
  scew_attribute* attribute = NULL;
  while ((child = scew_element_next(root, child)) != NULL)
  {
    while ((attribute = scew_attribute_next(child, attribute)) != NULL)
    {
	    if(strcmp(scew_attribute_name(attribute),"name")==0) {
    	  if(strcmp(scew_attribute_value(attribute), evaluatorName))
    	    return child;
    	}
    }
  }
  return NULL;
}

scew_element* createEvaluatorNode(seList evaluator) {
  char placeHolderString[30];
  STRING scaleFnName=NULL;
  fList temp = evaluator->featureList;
  scew_element* evaluatorNode = scew_element_create("seval");
  scew_element_add_attr_pair(evaluatorNode, "name", evaluator->name);
  sprintf(placeHolderString,"%d", evaluator->variant);
  scew_element_add_attr_pair(evaluatorNode, "variant", placeHolderString);
  
  while ( temp!=NULL ){
    scew_element* featureNode = scew_element_create("feature");
    
    scew_element_add_attr_pair(featureNode, "name", temp->name);
    
    
    sprintf(placeHolderString,"%.3f, %.3f, %.3f, %.3f", (double)temp->evalParams[0], (double)temp->evalParams[1], (double)temp->evalParams[2], (double)temp->evalParams[3]);
    scew_element_add_attr_pair(featureNode, "functionParameters", placeHolderString);
    
    scew_element_add_attr_pair(featureNode, "type", (temp->type==custom)?"custom":"library");
    
    if( temp->scale==&linear )
      scaleFnName = "linear";
    else if( temp->scale==&logarithmic )
      scaleFnName = "logarithmic";
    else if( temp->scale==&logistic )
      scaleFnName = "logistic";
    else if( temp->scale==&quadratic )
      scaleFnName = "quadratic";
    scew_element_add_attr_pair(featureNode, "scaling", scaleFnName);
    
    sprintf(placeHolderString,"%.3f, %.3f, %.3f, %.3f", (double)temp->scaleParams[0], (double)temp->scaleParams[1], (double)temp->scaleParams[2], (double)temp->scaleParams[3]);
    scew_element_add_attr_pair(featureNode, "scalingParams", placeHolderString);
    
    scew_element_add_elem(evaluatorNode, featureNode);
  }
  return evaluatorNode;
}

BOOLEAN writeEvaluatorToXMLFile(seList evaluator, STRING fileName) {
  scew_tree* tree = NULL;
  scew_parser* parser = NULL;
  BOOLEAN success = FALSE;
  scew_element* evaluatorToReplace = NULL;
  
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
  evaluatorToReplace = findEvaluatorNode(scew_tree_root(tree), evaluator->name);
  
  if( evaluatorToReplace!=NULL )
    scew_element_free(evaluatorToReplace);
  
  scew_element_add_elem(scew_tree_root(tree), createEvaluatorNode(evaluator));
  
  success = scew_writer_tree_file(tree, fileName);
  
  /* Remember to free tree (scew_parser_free does not free it) */
  scew_tree_free(tree);
  
  /* Frees the SCEW parser */
  scew_parser_free(parser);
  
  return success;
}

BOOLEAN initializeStaticEvaluator(STRING fileName){
  printf("Reading XML data from: %s\n", fileName);
  if(evaluatorList!=NULL)
    freeEvaluatorList(evaluatorList);
  evaluatorList = NULL;
  if(featureList!=NULL)
    freeFeatureList(featureList);
  featureList = NULL;
  
  evaluatorList = loadDataFromXML(fileName);
  return (evaluatorList!=NULL);
}


void initializeStaticEvaluatorBoard(int eltSize,int rows, int cols, BOOLEAN diagonals,void* initialPlayerPiece, void* opponentPlayerPiece, void* blankPiece) {
  
  LibInitialize(eltSize,rows,cols,diagonals);
  
  lBoard.initialPlayerPiece = initialPlayerPiece;
  lBoard.opponentPlayerPiece = opponentPlayerPiece;
  lBoard.blankPiece = blankPiece;
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
  void* board = NULL;
  
  if(linearUnhashPtr != NULL) 
    board = (*linearUnhashPtr)(p);

  while(features!=NULL){
    if(features->type == library) {
      if( linearUnhashPtr!=NULL )
        valueSum += features->weight * features->scale(features->fEvalL(board,features->piece,features->evalParams),features->scaleParams);
	    else{
	      printf("linearUnhash MUST be set if library functions are used!");
	      return -2;
	    }
    } else if(features->type == custom) {
      valueSum += features->weight * features->scale(features->fEvalC(p),features->scaleParams);
    } else {
      printf("ERROR, unknown feature type");
    }

    weightSum += features->weight;
    features = features->next;
  }
  
  SafeFree(board);
  
  return (valueSum/weightSum);
}

VALUE evaluatePositionValue(POSITION p) {
  float val = evaluatePosition(p);
  return (val==0)? tie : (val>0?win:lose);
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
  fList currFeature;

  while(TRUE) {

    printf("\nSelect a new trait to add to the static evaluator\n\n");
  
    nextItem = printList(LibraryTraits,numLibraryTraits,'a');
    nextItem = printList(CustomTraits,numCustomTraits,nextItem);

    printf("%c) Done\n",nextItem);
    
    while(TRUE) {
      choice = GetMyChar();
      traitNum = (int) (choice-97);

      if(choice < 97 || choice > nextItem+1) {
	printf("\nInvalid menu option, select again\n");
      } else {
	break;
      }
    }

    if(choice == nextItem) { //Done
      break;
    } else {
      currFeature = ParameterizeTrait(traitNum);
      currFeature->next = featureList;
      featureList = currFeature;
    }
  } 
}

fList ParameterizeTrait(int traitNum) {
  fList currFeature = SafeMalloc(sizeof(struct fNode));

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
  char choice;
  
  printf("\nWhat type of scaling function should be used for this trait?\n\n");
  printf("A scaling function is a function that maps raw trait values to the range [-1,1]\n\n");
  printf("a) Linear - A line clipped at y=1 and y=-1\nb) Logistic - A shifted and scaled sigmoid function\nc) Quadratic - A parabola with vertex at either y=-1 (concave up) or y=1 (concave down)\n");
  
  while(TRUE) {
    choice = GetMyChar();
    if(choice < 'a' || choice > 'c') {
      printf("Invalid menu option, select again\n");
    } else {
      break;
    }
  }

  switch(choice) {
  case 'a':
    currFeature->scale = &linear;
    RequestValue("\nEnter value under which the evaluator will assign -1:\n",0,1,currFeature);
    RequestValue("\nEnter value above which the evaluator will assign 1:\n",1,1,currFeature);
    break;
  case 'b':
    currFeature->scale = &logistic;
    RequestValue("\nEnter time shift of sigmoid function:\n",0,1,currFeature);
    RequestValue("\nEnter time scale of sigmoid function:\n",1,1,currFeature);
    break;
  case 'c':
    currFeature->scale = &quadratic;
    RequestValue("\nEnter 0 for a concave down function, otherwise function will be concave up:\n",0,1,currFeature);
    RequestValue("\nEnter value under which evaluator will assign -1:\n",1,1,currFeature);
    RequestValue("\nEnter value above which evaluator will assign 1:\n",2,1,currFeature);  
    break;
  }

}

void ParameterizeWeightingFunction(fList currFeature) {
  float choice;
  char temp[80];

  printf("\nEnter the weighting of this trait to be used in calculating the final result\n");
  
  GetMyStr(temp,80);
  choice = atof(temp);

  currFeature->weight = choice;
}

char printList(STRING list[],int length, char start) {
  int i;

  for(i=0;i<length;i++) {
    printf("%c) %s\n",start,list[i]);
    start++;
  }
  return start;
}

void RequestPiece(fList currFeature) {
  char choice;

  printf("\nDoes this pertain to the initial player's piece or opponent player's piece?\n");
  printf("(Type 'o' for opponent, anything else for initial player)\n");

  choice = GetMyChar();

  if (choice == 'o') {
    currFeature->piece = lBoard.opponentPlayerPiece;
  } else {
    currFeature->piece = lBoard.initialPlayerPiece;
  }
}

void RequestBoolean(STRING title,int paramNum,fList currFeature) {
  char choice;

  printf(title);
  printf("(0 for false, 1 for true)\n");
 
  while(TRUE) {
    choice = GetMyChar();
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
    choice = GetMyInt();
  } else {
    GetMyStr(temp,80);
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
float numPieces(void* board,void* piece, int params[]) {
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
float numFromEdge(void* board,void* piece,int params[]) {
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
float clustering(void* board,void* piece,int params[]) {
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
float connections(void* board,void* piece,int params[]) {
  int count=0,i,j,pathLength,loc,direction;
  BOOLEAN diagonals = params[0];
  void* blank = lBoard.blankPiece;
  
  

  for(i=0;i<lBoard.size;i++) {
    if (compare(i,piece,board)) {
      for(direction = 3;direction<7; direction+= (diagonals ? 1 : 2)) {	

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
