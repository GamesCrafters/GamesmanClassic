ifndef _SEVAL_H_
#define _SEVAL_H_

//function prototypes

int numPieces(void*,void*);
int numFromEdge(void*,void*,int);
double clustering(void*,void*);
int connections(void*,void*,void*,boolean);

//structs and enums

typedef enum scaling_enum {
  Linear, Logistic;
} SCALING

//encapsulates parameters associated with a trait
typedef struct trait {
  int id;
  int weight;
  SCALING scaleType;
  int scaleParams[2];
  int additionalParams[4];
} TRAIT;

//trait linked list
typedef struct traitlist_item {
  TRAIT item;
  struct traitlist_item* next;
} *TRAITLIST;

//static evaluator instance
typedef struct seval_struct {
  String name;
  TRAITLIST traits;
} EVALUATOR;

#endif
