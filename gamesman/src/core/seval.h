#ifndef _SEVAL_H_
#define _SEVAL_H_

// Type Declarations


typedef float(*scalingFunction)(float,float[]);

typedef float(*featureEvaluatorCustom)(POSITION);
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
  BOOLEAN perfect;
  TYPE type; //custom or library
  void* piece; //the piece that this trait centers on
  float scaleParams[4];
  int evalParams[4];
  scalingFunction scale;
  featureEvaluatorCustom fEvalC;
  featureEvaluatorLibrary fEvalL;
  struct fNode * next;
} *fList;

// Function Prototypes

//test
void* linearUnhash(int);
void* getInitialPlayerPiece();
void* getOpponentPlayerPiece();
void* getBlankPiece();
//end test

int numPieces(void*,void*,int[]);
int numFromEdge(void*,void*,int[]);
float clustering(void*,void*,int[]);
int connections(void*,void*,int[]);
//fList loadDataFromXML(STRING);
//fList parse_element(scew_element*, fList);
//featureEvaluatorLibrary getSEvalLibFnPtr(STRING);
//featureEvaluatorCustom getSEvalCustomFnPtr(STRING);
//BOOLEAN initializeStaticEvaluator(STRING);
float evaluatePosition(POSITION p);
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
