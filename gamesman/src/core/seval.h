#ifndef _SEVAL_H_
#define _SEVAL_H_


// Type Declarations

typedef float(*scalingFunction)(float,float[]);

typedef float(*featureEvaluatorLibrary)(void*,void*,int[]);

typedef enum f_type {
  library, custom
} TYPE;

/**I decided not to use a union for the function pointers because
 *it really provides no additional benefit, besides saving 4 bytes
 *of space. In either implementation, we still have to store which
 *type of function we're using.
 */
typedef struct fNode{
  STRING name;
  float weight;
  TYPE type; //custom or library
  void* piece; //the piece that this trait centers on
  float scaleParams[4];
  int evalParams[4];
  scalingFunction scale;
  featureEvaluatorCustom fEvalC;
  featureEvaluatorLibrary fEvalL;
  struct fNode * next;
} *fList;

typedef struct seNode{
  scew_element* element;
  STRING name;
  int variant;
  BOOLEAN perfect;
  fList featureList;
  struct seNode * next;
} *seList;

extern seList evaluatorList;

// Function Prototypes

extern BOOLEAN initializeStaticEvaluator(STRING);
extern void chooseEvaluator(int);
extern BOOLEAN evaluatorExistsForVariant(int);
extern BOOLEAN createNewXMLFile(STRING);

float numPieces(void*,void*,int[]);
float numFromEdge(void*,void*,int[]);
float clustering(void*,void*,int[]);
float connections(void*,void*,int[]);

void initializeStaticEvaluatorBoard(int,int,int,BOOLEAN,void*,void*,void*);

void freeFeatureList(fList);
void freeEvaluatorList(seList);
STRING copyString(STRING);
fList parseFeature(scew_element*);
seList parseEvaluators(scew_element*, seList);
scew_element* findEvaluatorNode(scew_element*, STRING);
scew_element* createEvaluatorNode(seList);
seList loadDataFromXML(STRING);
BOOLEAN writeEvaluatorToXMLFile(seList, STRING);
featureEvaluatorLibrary getSEvalLibFnPtr(STRING);
float evaluatePosition(POSITION);
VALUE evaluatePositionValue(POSITION);
float linear(float,float[]);
float logarithmic(float,float[]);
float logistic(float,float[]);
float quadratic(float,float[]);

void NewTraitMenu();
fList ParameterizeTrait(int);
void ParameterizeScalingFunction(fList);
void ParameterizeWeightingFunction(fList);
char printList(STRING[],int,char);
void RequestPiece(fList);
void RequestBoolean(STRING,int,fList);
void RequestValue(STRING,int,int,fList);
#endif
