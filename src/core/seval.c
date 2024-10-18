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

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include "gamesman.h"

/*lBoard will contain board processing data. Requires call
   of LibInitialize by the module. For information see mlib.c and mlib.h.*/
#include "mlib.h"
#include "seval.h"

#ifdef HAVE_XML
#include <scew/scew.h>
#endif

#define compare(slot,piece,board) !memcmp(board+slot*lBoard.eltSize,piece,lBoard.eltSize)
#define getSlot(row,col) (row*lBoard.cols+col)
#define getRow(slot) (slot/lBoard.cols)
#define getCol(slot) (slot%lBoard.rows)
#define min(a,b) (a<=b ? a : b)

seList evaluatorList;
seList currEvaluator = NULL;
char* LibraryTraits[] = {"Number of Pieces","Distance from Edge","Clustering","Connections"};
int numLibraryTraits = 4;
int numCustomTraits = 0;

//private functions
void printEvaluator();
void printFeature();
STRING getScaleFnName(fList feature);

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

	if(strcmp(fnName,"Number of Pieces")==0) {
		return &numPieces;
	} else if (strcmp(fnName,"Distance from Edge")==0) {
		return &numFromEdge;
	} else if (strcmp(fnName,"Clustering")==0) {
		return &(clustering);
	} else if (strcmp(fnName,"Connections")==0) {
		return &(connections);
	} else {
		printf("ERROR, unrecognized trait name in xml file: %s", fnName);
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
	if( head->featureList!=NULL ) // && head->featureList!=featureList
		freeFeatureList(head->featureList);
	SafeFree(head->name);
	freeEvaluatorList(head->next);
	SafeFree(head);
}

void chooseEvaluator(int numInEvaluatorList){
	seList temp = evaluatorList;
	while( numInEvaluatorList-- > 0 && temp!=NULL) temp = temp->next;

	if(temp!=NULL) {
		if(currEvaluator!=NULL)
			freeEvaluatorList(currEvaluator);
		currEvaluator = SafeMalloc(sizeof(struct seNode));
		memcpy(currEvaluator, temp, sizeof(struct seNode));
		currEvaluator->name = copyString(temp->name);
		currEvaluator->featureList = copyFeatureList(temp->featureList);
		currEvaluator->next = NULL;
		printf("\nThe Evaluator has been set to %s\n", currEvaluator->name);
		gSEvalLoaded = TRUE;
	}
}

seList findEvaluator(STRING name){
	seList temp = evaluatorList;
	while(temp!=NULL && strcmp(temp->name, name)!=0) temp = temp->next;
	return temp;
}

fList copyFeatureList(fList srcFeatureList){
	fList tempFeatureList=NULL;
	if(srcFeatureList!=NULL) {
		tempFeatureList = SafeMalloc(sizeof(struct fNode));
		memcpy(tempFeatureList, srcFeatureList, sizeof(struct fNode));
		tempFeatureList->name = copyString(srcFeatureList->name);
		tempFeatureList->next = copyFeatureList(srcFeatureList->next);
	}
	return tempFeatureList;
}

// Guaranteed upon entering that currEvaluator!=NULL
void updateCurrentEvaluator(){
	seList srcEvaluator = findEvaluator(currEvaluator->name);
	if(srcEvaluator==NULL) {
		freeEvaluatorList(currEvaluator);
		gSEvalLoaded = FALSE;
	}
	else{
		currEvaluator->variant = srcEvaluator->variant;
		currEvaluator->perfect = srcEvaluator->perfect;
		currEvaluator->next = NULL;
		if(currEvaluator->featureList!=NULL)
			freeFeatureList(currEvaluator->featureList);
		currEvaluator->featureList = copyFeatureList(srcEvaluator->featureList);
	}
}

BOOLEAN evaluatorExistsForVariant(int variant) {
	seList temp = evaluatorList;
	while( temp!=NULL ) {
		if( temp->variant == -1 || temp->variant == variant )
			return TRUE;
		temp = temp->next;
	}

	return FALSE;
}

/************************************************************************
**
** SCEW Related Functions
**
************************************************************************/

#ifdef HAVE_XML
// DON'T COMPILE THIS
// if XML not installed
// return types & args
// will create problems

fList parseFeature(scew_element* sEvalNode){
	int i;
	float tempEvalParams[4];
	scew_element* element = NULL;
	fList tempList = NULL;
	while ((element = scew_element_next(sEvalNode, element)) != NULL) {
		fList currFeature = SafeMalloc(sizeof(struct fNode));
		scew_attribute* attribute = NULL;


		// Default values for feature:
		currFeature->scale = &linear;
		currFeature->weight = 1;

		/**
		 * Iterates through the element's attribute list
		 */

		attribute = NULL;
		while ((attribute = scew_attribute_next(element, attribute)) != NULL)
		{
			if(strcmp(scew_attribute_name(attribute),"name")==0)
				currFeature->name = copyString((char *)scew_attribute_value(attribute));
			else if(strcmp(scew_attribute_name(attribute),"type")==0)
				currFeature->type = strcmp((char *)scew_attribute_value(attribute),"custom")==0 ? custom : library;
			else if(strcmp(scew_attribute_name(attribute),"scaling")==0) {
				if(strcmp(scew_attribute_value(attribute),"linear")==0) {
					currFeature->scale = &linear;
				} else if(strcmp(scew_attribute_value(attribute),"logistic")==0) {
					currFeature->scale = &logistic;
				} else if(strcmp(scew_attribute_value(attribute),"quadratic")==0) {
					currFeature->scale = &quadratic;
				} else if(strcmp(scew_attribute_value(attribute),"logarithmic")==0) {
					currFeature->scale = &logarithmic;
				} else if(strcmp(scew_attribute_value(attribute),"boolean")==0) {
					currFeature->scale = &boolean;
				}
			}
			else if(strcmp(scew_attribute_name(attribute),"functionParameters")==0) {
				setArrayFromString(tempEvalParams, (char *)scew_attribute_value(attribute));
				for(i=0; i<SEVAL_NUMEVALPARAMS; i++) {
					currFeature->evalParams[i] = (int) tempEvalParams[i];
				}
			}
			else if(strcmp(scew_attribute_name(attribute),"scalingParams")==0)
				setArrayFromString(currFeature->scaleParams, (char *)scew_attribute_value(attribute));
			else if(strcmp(scew_attribute_name(attribute),"piece")==0)
				currFeature->piece = atoi((char *)scew_attribute_value(attribute));
			else if(strcmp(scew_attribute_name(attribute),"weight")==0)
				currFeature->weight = atof((char *)scew_attribute_value(attribute));
			else {
				BadElse("parse_element");
				printf("Bad attribute=\"%s\"",(char *)scew_attribute_value(attribute));
			}
		}

		currFeature->fEvalC = NULL;
		currFeature->fEvalL = NULL;

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
	}

	return tempList;
}

void setArrayFromString(float* arr, STRING values){
	int i=0;
	char delims[] = ", ";
	char *value = NULL;
	value = strtok( values, delims );
	while( value != NULL ) {
		//printf( "value is \"%s\"\n", value );
		arr[i] = atof(value);
		value = strtok( NULL, delims );
		i++;
	}
}

seList parseEvaluators(scew_element* element, seList tempList) {
	scew_element* child = NULL;
	seList currEvaluator = NULL;

	if (element == NULL)
	{
		return NULL;
	}

	if(strcmp(scew_element_name(element),"seval")==0) {
		currEvaluator = SafeMalloc(sizeof(struct seNode));
		scew_attribute* attribute = NULL;
		currEvaluator->name = NULL;
		//currEvaluator->element = element;
		currEvaluator->variant = -1;
		currEvaluator->perfect = FALSE;
		while ((attribute = scew_attribute_next(element, attribute)) != NULL)
		{
			if(strcmp(scew_attribute_name(attribute),"name")==0)
				currEvaluator->name = copyString((char *)scew_attribute_value(attribute));
			else if(strcmp(scew_attribute_name(attribute),"variant")==0)
				currEvaluator->variant = atoi((char *)scew_attribute_value(attribute));
			else if(strcmp(scew_attribute_name(attribute),"perfect")==0) {
				currEvaluator->perfect = (strcmp(scew_attribute_value(attribute),"true")==0) ? TRUE : FALSE;
				if(currEvaluator->perfect) gSEvalPerfect = TRUE;
			} else{
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

scew_element* createEvaluatorNode(seList evaluator) {
	char placeHolderString[30];
	STRING scaleFnName=NULL;
	fList temp = evaluator->featureList;

	memset(placeHolderString,0,30);

	scew_element* evaluatorNode = scew_element_create("seval");
	scew_element_add_attr_pair(evaluatorNode, "name", evaluator->name);
	sprintf(placeHolderString,"%d", evaluator->variant);
	scew_element_add_attr_pair(evaluatorNode, "variant", placeHolderString);

	while ( temp!=NULL ) {
		scew_element* featureNode = scew_element_create("feature");

		scew_element_add_attr_pair(featureNode, "name", temp->name);

		sprintf(placeHolderString,"%.3f", temp->weight);
		scew_element_add_attr_pair(featureNode, "weight", placeHolderString);

		sprintf(placeHolderString,"%d", temp->piece);
		scew_element_add_attr_pair(featureNode, "piece", placeHolderString);

		sprintf(placeHolderString,"%.3f, %.3f, %.3f, %.3f", (double)temp->evalParams[0], (double)temp->evalParams[1], (double)temp->evalParams[2], (double)temp->evalParams[3]);
		scew_element_add_attr_pair(featureNode, "functionParameters", placeHolderString);

		scew_element_add_attr_pair(featureNode, "type", (temp->type==custom) ? "custom" : "library");

		scaleFnName = getScaleFnName(temp);
		scew_element_add_attr_pair(featureNode, "scaling", scaleFnName);

		sprintf(placeHolderString,"%.3f, %.3f, %.3f, %.3f", (double)temp->scaleParams[0], (double)temp->scaleParams[1], (double)temp->scaleParams[2], (double)temp->scaleParams[3]);
		scew_element_add_attr_pair(featureNode, "scalingParams", placeHolderString);

		scew_element_add_elem(evaluatorNode, featureNode);

		temp = temp->next;
	}
	return evaluatorNode;
}

scew_element* findEvaluatorNode(scew_element* root, STRING evaluatorName) {
	scew_element* child = NULL;
	scew_attribute* attribute = NULL;
	while ((child = scew_element_next(root, child)) != NULL)
	{
		while ((attribute = scew_attribute_next(child, attribute)) != NULL)
		{
			if(strcmp(scew_attribute_name(attribute),"name")==0) {
				if(strcmp(scew_attribute_value(attribute), evaluatorName)==0)
					return child;
			}
		}
	}
	return NULL;
}

#endif

void TryToLoadAnEvaluator(){
	char fileName[30];
	sprintf(fileName, "xml/%s.xml", kDBName);
	if(!FullInitializeSEval(fileName)) return;

	// If we don't have an evaluator defined, then
	// loop through them and pick one
	if(currEvaluator == NULL && evaluatorExistsForVariant(getOption())) {
		seList bestEvaluator = NULL;
		int bestEvalNum = -1, traverserNum=0;
		seList traverser = evaluatorList;

		while (traverser != NULL) {
			if( (traverser->variant==-1 || traverser->variant==getOption()) ) {
				if(bestEvaluator!=NULL) {
					if( bestEvaluator->perfect!=TRUE && traverser->perfect==TRUE) {
						bestEvaluator = traverser;
						bestEvalNum = traverserNum;
					}
				}
				else{
					bestEvaluator=traverser;
					bestEvalNum = traverserNum;
				}
			}

			if(bestEvaluator->perfect == TRUE) {
				// Set currEvaluator and break
				chooseEvaluator(bestEvalNum);
				return;
			}
			traverser = traverser->next;
			traverserNum++;
		}

		if(bestEvalNum!=-1)
			chooseEvaluator(bestEvalNum);
	}

	return;
}

STRING getScaleFnName(fList feature) {
	STRING scaleFnName;
	if( feature->scale==&linear )
		scaleFnName = "linear";
	else if( feature->scale==&logarithmic )
		scaleFnName = "logarithmic";
	else if( feature->scale==&logistic )
		scaleFnName = "logistic";
	else if( feature->scale==&quadratic )
		scaleFnName = "quadratic";
#ifdef WIN32
    else if( feature->scale==&booleanA )
#else
	else if( feature->scale==&boolean )
#endif
		scaleFnName = "boolean";
	else {
		scaleFnName = NULL;
	}
	return scaleFnName;
}


BOOLEAN loadDataFromXML(STRING fileName) {
  #ifdef HAVE_XML
	scew_tree* tree = NULL;
	scew_parser* parser = NULL;
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
	evaluatorList = parseEvaluators(scew_tree_root(tree), NULL);


	if(currEvaluator!=NULL)
		updateCurrentEvaluator();

	/* Remember to free tree (scew_parser_free does not free it) */
	scew_tree_free(tree);

	/* Frees the SCEW parser */
	scew_parser_free(parser);

	return TRUE;
  #else
	return FALSE;
  #endif

}

BOOLEAN createNewXMLFile(STRING fileName) {
  #ifdef HAVE_XML
	scew_tree* tree = scew_tree_create();
	scew_element* root = scew_tree_add_root(tree, "game");
	scew_element_add_attr_pair(root, "name", kDBName);
	return scew_writer_tree_file(tree, fileName);
  #else
	return FALSE;
  #endif
}

BOOLEAN writeEvaluatorToXMLFile(seList evaluator, STRING fileName) {
  #ifdef HAVE_XML
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
		printf("Evaluator to replace: %s", evaluatorToReplace);

	if( evaluatorToReplace!=NULL )
		scew_element_free(evaluatorToReplace);

	scew_element_add_elem(scew_tree_root(tree), createEvaluatorNode(evaluator));

	success = scew_writer_tree_file(tree, fileName);

	/* Remember to free tree (scew_parser_free does not free it) */
	scew_tree_free(tree);

	/* Frees the SCEW parser */
	scew_parser_free(parser);

	return success;
  #else
	return FALSE;
  #endif
}

/************************************************************************
**
** END SCEW Related Functions
**
************************************************************************/

BOOLEAN initializeStaticEvaluator(STRING fileName){
	numCustomTraits=0;
	if(gCustomTraits!=NULL) {
		while(strcmp(gCustomTraits[numCustomTraits], "")!=0) numCustomTraits++;
	}
  #ifdef HAVE_XML
	printf("Reading XML data from: %s\n", fileName);
	if(evaluatorList!=NULL)
		freeEvaluatorList(evaluatorList);
	evaluatorList = NULL;
	return loadDataFromXML(fileName);
  #else
	return TRUE;
  #endif
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
	fList features = NULL;
	float valueSum = 0;
	float weightSum = 0;
	void* board = NULL;

	if(currEvaluator!=NULL)
		features = currEvaluator->featureList;
	else{
		BadElse("evaluatePosition");
		printf("Tried to call evaluatePosition when currEvaluator==NULL");
	}


	if(linearUnhash != NULL)
		board = (*linearUnhash)(p);

	while(features!=NULL) {
		if(features->type == library) {
			if( linearUnhash!=NULL )
				valueSum += features->weight * // += weight * scale(evaluateTrait(board))
				            features->scale(features->fEvalL(board,(features->piece==initial ? lBoard.initialPlayerPiece : lBoard.opponentPlayerPiece)
				                                             ,features->evalParams),features->scaleParams);
			else{
				BadElse("evaluatePosition");
				printf("linearUnhash MUST be set if library functions are used!\n");
				printf("Results will be nondeterministic.\n");
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

	if(board!=NULL)
		SafeFree(board);

	return (valueSum/weightSum);
}

VALUE evaluatePositionValue(POSITION p) {
	float val = evaluatePosition(p);
	return (val==0) ? tie : (val>0 ? win : lose);
}

//Params are {-1 intersect, 1 intersect}
float linear(float value,float params[]){
	//return (value < params[0]) ? -1 : ((value > params[1]) ? 1 : value);
	return value;
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

#ifdef WIN32
float booleanA(float value,float params[]) {
    return (value>params[0] ? 1 : -1);
}
#else
float boolean(float value,float params[]) {
    return (value>params[0] ? 1 : -1);
}
#endif


//Params are {value*10 where y==1 for log10(abs(value*9)+1) (Default=1)}
float logarithmic(float value,float params[]){
	float absValue = fabs(value);
	int sign = (int) value / absValue;
	float scaleFactor = (params==NULL || params[0]==0) ? 1 : params[0];
	float ans = log10( (absValue*9+1) / scaleFactor );
	return (sign * (absValue>1 || ans>1)) ? 1 : ans;
}

/************************************************************************
**
** Text interface
**
************************************************************************/

void NewTraitMenu(STRING fileName) {
	char nextItem, libraryUpperBound;
	int traitNum;
	fList currFeature = NULL;
	char namePlaceHolder[30];
	char* defaultName = "default";
	int variant = 0;
	char choice=0;
	seList currEvaluator = NULL;

	printf("Enter a name for this evaluator ('' for 'default'): ");
	scanf("%s", namePlaceHolder);
	printf("Enter a variant for this evaluator (-1 for all): ");
	scanf("%d", &variant);
	while(getchar()!='\n') ; //absorb newline :(

	if(strcmp(namePlaceHolder, "")==0)
		strcpy(namePlaceHolder,defaultName);
	currEvaluator = SafeMalloc(sizeof(struct seNode));
	currEvaluator->name = copyString(namePlaceHolder);
	//currEvaluator->element = element;
	currEvaluator->variant = variant;
	currEvaluator->perfect = FALSE;

	while(TRUE) {

		printf("\nSelect a new trait to add to the static evaluator\n\n");

		nextItem = 'a';
		if(linearUnhash!=NULL) {
			nextItem = printList(LibraryTraits,numLibraryTraits,nextItem);
		} else {
			printf("\nWarning: Linear unhash function not found. May only select from custom traits\n\n");
		}
		libraryUpperBound = nextItem-1;
		nextItem = printList(gCustomTraits,numCustomTraits,nextItem);

		printf("%c) Done\n",nextItem);

		while(TRUE) {
			printf("\nTrait Choice: ");
			choice = GetMyChar();
			traitNum = (int) (choice-97);

			if(choice < 97 || choice > nextItem+1)
				printf("\nInvalid menu option, select again\n");
			else
				break;
		}

		if(choice == nextItem) //Done
			break;
		else {
			currFeature = ParameterizeTrait(traitNum, libraryUpperBound-97);
			currFeature->next = currEvaluator->featureList;
			currEvaluator->featureList = currFeature;
		}
	}


	//currEvaluator->featureList = copyFeatureList(featureList);
	currEvaluator->next = evaluatorList;
	evaluatorList = currEvaluator;
  #ifdef HAVE_XML
	writeEvaluatorToXMLFile(evaluatorList, fileName);
  #endif
}

fList ParameterizeTrait(int traitNum, int libraryUpperBound) {
	fList currFeature = SafeMalloc(sizeof(struct fNode));
	memset(currFeature, 0, sizeof(struct fNode));

	if(traitNum<=libraryUpperBound) {
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
	}

	if(traitNum > libraryUpperBound) {
		currFeature->type = custom;
		currFeature->name = copyString(gCustomTraits[traitNum-libraryUpperBound]);
		currFeature->fEvalC = (featureEvaluatorCustom)gGetSEvalCustomFnPtr(currFeature->name);

	} else {
		currFeature->type = library;
		currFeature->name = copyString(LibraryTraits[traitNum]);
		currFeature->fEvalL = (featureEvaluatorLibrary)getSEvalLibFnPtr(currFeature->name);
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

	for(i=0; i<length; i++) {
		printf("%c) %s\n",start,list[i]);
		start++;
	}
	return start;
}

void RequestPiece(fList currFeature) {
	char choice;
	char* listTemp[] = {"Initial player","Opponent player"};
	while(TRUE) {
		printf("\nWhose piece does this apply to?\n");
		printList(listTemp,2,'a');
		printf("\n\nSelect an option: ");
		switch(choice = GetMyChar()) {
		case 'a': case 'A':
			currFeature->piece = initial;
			return;
		case 'b': case 'B':
			currFeature->piece = opponent;
			return;
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	}
}

void RequestBoolean(STRING title,int paramNum,fList currFeature) {
	char choice;

	printf("%s",title);
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
 * Retrieves int input from user.
 * paramType=0 refers to evalParams, paramType=1 refers to scaleParams
 */
void RequestValue(STRING title,int paramNum,int paramType,fList currFeature) {
	int choice;
	float scaleChoice;
	char temp[80];

	printf("%s",title);


	if(paramType==0) {
		choice = GetMyInt();
	} else {
		GetMyStr(temp,80);
		scaleChoice = atof(temp);
	}

	if(paramType==0) {
		currFeature->evalParams[paramNum] = choice;
	} else if(paramType==1) {
		currFeature->scaleParams[paramNum] = scaleChoice;
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
	for(i=0; i<lBoard.size; i++) {
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

	for(i=0; i<lBoard.size; i++) {
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
	for(i=0; i<lBoard.size; i++) {
		if(compare(i,piece,board)) {
			rowAverage+=getRow(i);
			colAverage+=getCol(i);
			numPieces++;
		}
	}
	if(numPieces==0) {
		return 0;
	}
	//compute actual averages
	rowAverage = rowAverage/numPieces;
	colAverage = colAverage/numPieces;

	for(i=0; i<lBoard.size; i++) {
		if(compare(i,piece,board)) {
			value += (getRow(i)-rowAverage)*(getRow(i)-rowAverage) + (getCol(i)-colAverage)*(getCol(i)-colAverage);
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
	for(i=0; i<lBoard.size; i++) {
		if (compare(i,piece,board)) {
			for(direction = 3; direction<7; direction+= (diagonals ? 1 : 2)) {

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
				for(j=0; j<pathLength; j++) {
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

void printEvaluator(seList evaluator) {
	fList curFeature = NULL;
	if(evaluator!=NULL) {
		printf("\nEvaluator- %s\n",evaluator->name);
		printf("Variant: %d\tPerfect: %s\n",evaluator->variant,(evaluator->perfect ? "true" : "false"));
		printf("\nFeatures:\n");
		curFeature = evaluator->featureList;
		while(curFeature!=NULL) {
			printFeature(curFeature);
			curFeature = curFeature->next;
		}
	} else {
		printf("\nEvalutator - (null)\n");
	}
}

void printFeature(fList feature) {
	int i;
	printf("\n\t%s\n",feature->name);
	printf("\tType - %s\n",(feature->type==library ? "library" : "custom"));
	printf("\tWeight - %.2f\n",feature->weight);
	printf("\tPiece - %d\n",(int)feature->piece);
	//printf("\tScaling function: %s\n",getScaleFnName(feature)); CAN SOMEONE TELL ME WHY THIS LINE DOESN'T WORK???????????
	printf("\tScaling parameters: ");
	for(i=0; i<SEVAL_NUMSCALEPARAMS; i++) {
		printf("  %.2f",feature->scaleParams[i]);
	}
	printf("\n\tEvaluation parameters: ");
	for(i=0; i<SEVAL_NUMEVALPARAMS; i++) {
		printf("  %d",feature->evalParams[i]);
	}
	printf("\n");
}

BOOLEAN FullInitializeSEval(char *fileName){

  #ifdef HAVE_XML
	if( !gSEvalLoaded ) {
		// Check for existence of the file
		if(fopen(fileName, "r") == NULL) {
			// Make sure xml dir is there
			DIR *xml_dir = opendir("./xml");
			if (xml_dir == NULL) {
				if(mkdir ("./xml", 0755)==-1) {
					printf("SEval: Cannot create xml folder. Please create it youself. Returning to menu.\n\n");
					return FALSE;
				}
			}

			closedir (xml_dir);
			// If xml file doesn't already exist try to create it
			if(!createNewXMLFile(fileName)) {
				printf("SEval: Cannot create xml file. Returning to menu.\n\n");
				return FALSE;
			}
		}
	}
  #endif

	// By now, we at least have a file called kDBName.xml with a game element
	//  ---> Enough for the parser to work with
	// Regenerate list of defined Static Evaluators
	// Base Menu Choices off of that

	if(!initializeStaticEvaluator(fileName)) {
		printf("Sorry, the Static Evaluator failed to initialize.");
		return FALSE;
	}
	return TRUE;
}

USERINPUT StaticEvaluatorMenu()
{
	USERINPUT result = Configure;
	char c;
	char fileName[30];
	sprintf(fileName, "xml/%s.xml", kDBName);

	if(!FullInitializeSEval(fileName)) return result;

	do {

		// Now we have some sort of a list of evaluators (may be NULL)

		printf("\n\tStatic Evaluator Options:");
		printf("\n\t-------------------------\n");
		printf("\n\tn)\tCreate a (N)ew Static Evaluator\n");
		//Need to implement
		//printf("\te)\t(E)dit an existing Static Evaluator\n");
		if(evaluatorList!=NULL) {
			printf("\ts)\t(S)elect a Static Evaluator available for this variant\n");
			if(gSEvalLoaded)
				printf("\t\t(Currently \"%s\")\n", currEvaluator->name);
			else
				printf("\t\t(Currently None)\n");
		}
		if(gSEvalLoaded) {
			printf("\tp)\t(P)rint the current evaluator\n");
		}
		printf("\tb)\t(B)ack = Return to previous activity\n");
		printf("\n\tq)\t(Q)uit\n");
		printf("\n\nSelect an option: ");

		switch(c = GetMyChar()) {
		case 'n': case 'N':
			NewTraitMenu(fileName);
			break;
		case 'e': case 'E':
			printf("Coming soon to a Gamesman near you!");
			break;
		case 's': case 'S':
			if(evaluatorList!=NULL) {
				int variant = getOption();
				if( evaluatorExistsForVariant(variant) ) {
					do {
						seList temp = evaluatorList;
						int optionMax, option = 1;
						printf("\n\tStatic Evaluators For This Variant:");
						printf("\n\t-----------------------------------");
						while( temp!=NULL ) {
							if(temp->variant==-1)
								printf("\n\t%d)\t%s - all variants", option++, temp->name);
							else if(temp->variant==variant)
								printf("\n\t%d) %s - variants #%d", option++, temp->name, temp->variant);
							temp = temp->next;
						}
						printf("\n\n\tb)\t(B)ack = Return to previous activity");
						printf("\n\tq)\t(Q)uit\n");

						optionMax = option;
						printf("\n\nSelect an option: ");

						option = GetMyChar();
						if(option=='B' || option=='b')
							break;
						else if(option=='Q' || option=='q') {
							ExitStageRight();
							exit(0);
						}
						else if(option>=optionMax+'0' || option<=0+'0') {
							BadMenuChoice();
							HitAnyKeyToContinue();
						}
						else{
							chooseEvaluator(option-'1');
							if(gSEvalLoaded == TRUE)
								return result;
						}
					} while(TRUE);
				}
				else{
					BadMenuChoice();
					HitAnyKeyToContinue();
				}
			}
			else{
				BadMenuChoice();
				HitAnyKeyToContinue();
			}
			break;
		case 'P': case 'p':
			if(gSEvalLoaded) {
				printEvaluator(currEvaluator);
				HitAnyKeyToContinue();
			} else {
				BadMenuChoice();
				HitAnyKeyToContinue();
			}
			break;
		case 'B': case 'b':
			printf("\n\n");
			return result;
		case 'Q': case 'q':
			ExitStageRight();
			exit(0);
		default:
			BadMenuChoice();
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);

}
