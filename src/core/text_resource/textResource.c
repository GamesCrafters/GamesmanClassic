
#include "textResource.h"
#include "trnode.h"
#include <expat.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int _TextResource_loadResource(TextResource tr, const char* filename);

TextResource TextResource_create(const char* filename) {
	TextResource tr = calloc(sizeof(struct TextResource_struct), 1);
	tr->parameters = ght_create(32);
	tr->strings = ght_create(64);

	int result = _TextResource_loadResource(tr,filename);

	if (!result) {
		TextResource_free(tr);
		return NULL;
	} else {
		return tr;
	}
}

void TextResource_free(TextResource tr) {
	ght_iterator_t iterator;
	const void *p_key;
	void *p_e;

	for (p_e = ght_first(tr->parameters, &iterator, &p_key); p_e;
	     p_e = ght_next(tr->parameters, &iterator, &p_key)) {
		free(p_e);
	}


	ght_finalize(tr->parameters);

	for (p_e = ght_first(tr->strings, &iterator, &p_key); p_e;
	     p_e = ght_next(tr->strings, &iterator, &p_key)) {

		free(p_e);
	}

	ght_finalize(tr->strings);

	free(tr);
}

void TextResource_setParameter(TextResource tr, const char* key, const char* value) {
	bstring val = ght_remove(tr->parameters,strlen(key),key);
	if (val) bdestroy(val);

	val = bfromcstr(value);
	ght_insert(tr->parameters,val,strlen(key),key);
}

static bstring tr_eval(TextResource tr, trnode_t* node);

static bstring tr_evalKey(TextResource tr, const char* key) {
	trnode_t* node = ght_get(tr->strings,strlen(key),key);
	if (!node)
		return bformat("<'%s' not found>",key);
	return tr_eval(tr,node);
}

static bstring tr_eval(TextResource tr, trnode_t* node) {
	//internally we work with bstrings



	trnode_t* tmpNode;

	switch(node->type) {
	case TRNODE_TEXT:
		return bstrcpy(node->textData);
	case TRNODE_REPR:
		; int match=0;
		if (node->condKey) {
			assert(node->condValue);
			//conditional
			char* condkey = bstr2cstr(node->condKey,'\0');
			bstring val = ght_get(tr->parameters,strlen(condkey),condkey);
			bcstrfree(condkey);

			if (val && !bstrcmp(node->condValue, val)) {
				//matches
				match = 1;
			}
		} else {
			//unconditional, so go too
			match=1;
		}

		if (match) {
			return tr_evalKey(tr,node->reprKey->data);
		} else
			return bfromcstr("");
	case TRNODE_STRING:
		//these guys have a blob of nodes that needs to be catted together smartly
		tmpNode = node->children_list;
		bstring tmpStr = bfromcstralloc(32,"");
		while(tmpNode) {
			bstring childStr = tr_eval(tr,tmpNode);
			bconchar(tmpStr,' ');
			bconcat(tmpStr,childStr);
			bdestroy(childStr);
			tmpNode = tmpNode->next;
		}
		return tmpStr;
	case TRNODE_SWITCH:
		//look through the children for matching choice
		; char* switchVar = bstr2cstr(node->switchVar,'\0');
		bstring val = ght_get(tr->parameters,strlen(switchVar),switchVar);
		bcstrfree(switchVar);

		tmpNode = node->children_list;
		while(tmpNode) {
			assert(tmpNode->type == TRNODE_STRING);
			assert(tmpNode->caseValue);

			if (!bstrcmp(tmpNode->caseValue,val)) {
				//we match, return this
				return tr_eval(tr,tmpNode);
			}
			//try next
			tmpNode = tmpNode->next;
		}
		return bfromcstr("");
	default:
		assert(0);
	}

	assert(0);
	return NULL;
}


static void normalize_space(bstring str) {
	int i=0;
	while(i<blength(str)) {
		int spaceStart = bstrchrp(str,' ',i);
		if (spaceStart == BSTR_ERR)
			break;
		//we found it, now look for the end of this space sequence
		int spaceLen = 1; //exclusive
		while(str->data[spaceStart+spaceLen] == ' ')
			spaceLen++;
		if (spaceLen > 1) {
			//multiple spaces
			bdelete(str,spaceStart+1,spaceLen-1);
		}

		i = spaceStart+1;
	}
}

int TextResource_getString(TextResource tr, const char* key, unsigned int maxSize, char* buffer) {

	bstring str = tr_evalKey(tr, key);
	normalize_space(str);
	char* cstr = bstr2cstr(str,'0');

	//copy no more than maxSize bytes
	int copysize = strlen(cstr) + 1;
	int rv = copysize;
	if (copysize > maxSize) {
		copysize = maxSize-1;
		buffer[maxSize-1] = '\0'; //null terminate the buffer
		rv = -maxSize; //negative signals we got truncated
	}

	strncpy(buffer,cstr,copysize);
	bcstrfree(cstr);
	bdestroy(str);

	return rv;
}

