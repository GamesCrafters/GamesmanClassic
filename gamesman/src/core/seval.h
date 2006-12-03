#ifndef _SEVAL_H_
#define _SEVAL_H_

// Type Declarations

typedef float(*scalingFunction)(float,int[]);

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
  int scaleParams[4];
  int evalParams[4];
  scalingFunction scale;
  featureEvaluatorCustom fEvalC;
  featureEvaluatorLibrary fEvalL;
  struct fNode * next;
} *fList;

// Function Prototypes

int numPieces(void*,void*);
int numFromEdge(void*,void*,int);
double clustering(void*,void*);
int connections(void*,void*,void*,BOOLEAN);
fList loadDataFromXML(STRING);
fList parse_element(scew_element*, fList);
featureEvaluatorLibrary getSEvalLibFnPtr(STRING);
featureEvaluatorCustom getSEvalCustomFnPtr(STRING);
BOOLEAN initializeStaticEvaluator(STRING);
float evaluatePosition(POSITION p);
float linear(float,int[]);
float logarithmic(float,int[]);
float logistic(float,int[]);
float quadratic(float,int[]);

#endif
