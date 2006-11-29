#ifndef _SEVAL_H_
#define _SEVAL_H_

// Type Declarations

typedef float(*scalingFunction)(float);

typedef float(*featureEvaluator)(POSITION);

typedef struct fNode{
  STRING name;
  float weight;
  scalingFunction scale;
  BOOLEAN perfect;
  featureEvaluator evalFn;
  struct fNode * next;
} *fList;

// Function Prototypes

int numPieces(void*,void*);
int numFromEdge(void*,void*,int);
double clustering(void*,void*);
int connections(void*,void*,void*,BOOLEAN);
fList loadDataFromXML(STRING);
fList parse_element(scew_element* , fList);
featureEvaluator getSEvalLibFnPtr(STRING);
featureEvaluator getSEvalCustomFnPtr(STRING);
BOOLEAN initializeStaticEvaluator(STRING);
VALUE evaluatePosition(POSITION p);
float linear(float);
float logarithmic(float);
float logistic(float);
float quadratic(float);

#endif
