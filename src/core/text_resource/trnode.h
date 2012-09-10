#ifndef TRNODE_H_
#define TRNODE_H_

#include "bstrlib.h"

//the different nodes from xml

enum TRNODETYPE {TRNODE_STRING, TRNODE_SWITCH, TRNODE_REPR, TRNODE_TEXT};

typedef struct trnode {
	enum TRNODETYPE type;
	struct trnode* children_list;

	//structural linked-list (old c development style =P, since we DON'T HAVE STL)
	struct trnode* next;

	//node specific data
	union {
		//we are string or case node
		bstring caseValue; //not null if we are actually a case

		//we are switch node
		bstring switchVar;

		//we are repr or cond node
		struct {
			bstring reprKey;

			bstring condKey; //not null if we are actually a cond
			bstring condValue;
		};

		//we are text node
		bstring textData;
	};
} trnode_t;

trnode_t* trnode_create(enum TRNODETYPE type);
trnode_t* trnode_createStringNode(const char* caseValue);
trnode_t* trnode_createSwitchNode(const char* var);
trnode_t* trnode_createReprNode(const char* reprKey, const char* condKey, const char* condValue);
trnode_t* trnode_createTextNode(bstring data);

void trnode_addLast(trnode_t* thisNode, trnode_t* toAdd);

void trnode_free(trnode_t*);



#endif /*TRNODE_H_*/
