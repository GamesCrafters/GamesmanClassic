#ifndef _SEVAL_H_
#define _SEVAL_H_


#ifdef HAVE_XML
#include <scew/scew.h>
#endif

// Constants
#define SEVAL_NUMSCALEPARAMS 4
#define SEVAL_NUMEVALPARAMS 4

// Type Declarations

typedef float (*scalingFunction)(float,float[]);

typedef float (*featureEvaluatorLibrary)(void*,void*,int[]);

typedef enum f_type {
	library, custom
} TYPE;

typedef enum p_type {
	initial, opponent
} PIECE_TYPE;

/**I decided not to use a union for the function pointers because
 * it really provides no additional benefit, besides saving 4 bytes
 * of space. In either implementation, we still have to store which
 * type of function we're using.
 */
typedef struct fNode {
	STRING name;
	float weight;
	TYPE type; //custom or library
	PIECE_TYPE piece; //the piece that this trait centers on
	float scaleParams[SEVAL_NUMSCALEPARAMS];
	int evalParams[SEVAL_NUMEVALPARAMS];
	scalingFunction scale;
	featureEvaluatorCustom fEvalC;
	featureEvaluatorLibrary fEvalL;
	struct fNode * next;
} *fList;

typedef struct seNode {
	//scew_element* element;
	STRING name;
	int variant;
	BOOLEAN perfect;
	fList featureList;
	struct seNode * next;
} *seList;

extern seList evaluatorList;

// Function Prototypes

extern USERINPUT StaticEvaluatorMenu();
extern float evaluatePosition(POSITION);
extern VALUE evaluatePositionValue(POSITION);
extern void TryToLoadAnEvaluator();

BOOLEAN FullInitializeSEval(char*);
float numPieces(void*,void*,int[]);
float numFromEdge(void*,void*,int[]);
float clustering(void*,void*,int[]);
float connections(void*,void*,int[]);

void initializeStaticEvaluatorBoard(int,int,int,BOOLEAN,void*,void*,void*);

#ifdef HAVE_XML
scew_element* findEvaluatorNode(scew_element*, STRING);
scew_element* createEvaluatorNode(seList);
seList parseEvaluators(scew_element*, seList);
fList parseFeature(scew_element*);
#endif
void setArrayFromString(float*, STRING);
void freeFeatureList(fList);
void freeEvaluatorList(seList);
STRING copyString(STRING);
fList copyFeatureList(fList);
void updateCurrentEvaluator();
BOOLEAN loadDataFromXML(STRING);
BOOLEAN writeEvaluatorToXMLFile(seList, STRING);
BOOLEAN initializeStaticEvaluator(STRING);
void chooseEvaluator(int);
BOOLEAN evaluatorExistsForVariant(int);
BOOLEAN createNewXMLFile(STRING);
featureEvaluatorLibrary getSEvalLibFnPtr(STRING);
float linear(float,float[]);
float logarithmic(float,float[]);
float logistic(float,float[]);
float quadratic(float,float[]);
#ifdef WIN32
    float booleanA(float, float[]);
#else
    float boolean(float,float[]);
#endif

void NewTraitMenu(STRING);
fList ParameterizeTrait(int, int);
void ParameterizeScalingFunction(fList);
void ParameterizeWeightingFunction(fList);
char printList(STRING[],int,char);
void RequestPiece(fList);
void RequestBoolean(STRING,int,fList);
void RequestValue(STRING,int,int,fList);
#endif
