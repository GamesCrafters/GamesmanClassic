#include "trnode.h"
#include <stdlib.h>

trnode_t* trnode_create(enum TRNODETYPE type) {
	//should only be called by the other create functions
	trnode_t* node = calloc(sizeof(trnode_t),1);
	node->type = type;
	node->children_list = NULL;
	node->next = NULL;

	return node;
}

trnode_t* trnode_createStringNode(const char* caseValue) {
	trnode_t* node = trnode_create(TRNODE_STRING);
	node->caseValue = bfromcstr(caseValue);

	return node;
}

trnode_t* trnode_createSwitchNode(const char* var) {
	trnode_t* node = trnode_create(TRNODE_SWITCH);
	node->switchVar = bfromcstr(var);

	return node;
}

trnode_t* trnode_createReprNode(const char* reprKey, const char* condKey, const char* condValue) {
	trnode_t* node = trnode_create(TRNODE_REPR);
	node->reprKey = bfromcstr(reprKey);
	node->condKey = bfromcstr(condKey);
	node->condValue = bfromcstr(condValue);

	return node;
}

trnode_t* trnode_createTextNode(bstring data) {
	trnode_t* node = trnode_create(TRNODE_TEXT);
	node->textData = bstrcpy(data);

	return node;
}

void trnode_free(trnode_t* node) {
	//face it, we don't have to, since program quits anyway
	//unless we do, so if anyone's extending this they should implement this
}

void trnode_addLast(trnode_t* thisNode, trnode_t* toAdd) {
	trnode_t* tail = thisNode->children_list;
	toAdd->next = NULL;

	if (!tail) {
		thisNode->children_list = toAdd;
	} else {
		while(tail->next)
			tail = tail->next;
		tail->next = toAdd;
	}
}

