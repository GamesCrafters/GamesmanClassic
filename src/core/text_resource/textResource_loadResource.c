#include "trnode.h"
#include "textResource.h"
#include "bstrlib.h"

#include <stdio.h>
#include <expat.h>
#include <string.h>
#include <assert.h>

//EXPAT LOADER
static trnode_t* gNodeStack;
static TextResource gCurrentResource;
static trnode_t* gLastNode; //important for text node concatenation

//helper function to die with a message
static void die(char* msg) {
	fprintf(stderr,"Resource file structure error: %s\n",msg);
	exit(-1);
}

//helper func for finding attr in attrlist
static const XML_Char* _trl_findAttr(const XML_Char** atts, const char* key) {
	int i=0;

	while (atts[i] != NULL) {
		if (!strcmp(atts[i],key))
			return atts[i+1];
		i+=2;
	}

	return NULL;

}

//helper func to push the node to top of stack
static void _trl_push(trnode_t* node) {
	node->next = gNodeStack;
	gNodeStack = node;
}

static trnode_t* _trl_pop() {
	trnode_t* node = gNodeStack;
	if (node != NULL) {
		gNodeStack = node->next;
		node->next = NULL;
	}
	return node;
}

static void _trl_elem_start(void *userData,
                            const XML_Char *name,
                            const XML_Char **atts) {
	gLastNode = NULL;
	if (gNodeStack) {
		switch(gNodeStack->type) {
		case TRNODE_STRING:                     //in it we expect cond and repr

			; //this semicolon is very important.
			trnode_t* node;

			if (!strcmp(name,"cond")) {
				const XML_Char* key = _trl_findAttr(atts,"key");
				if (!key) die("cond node must contain 'key' attribute");
				const XML_Char* var = _trl_findAttr(atts,"var");
				if (!var) die("cond node must contain 'var' attribute");
				const XML_Char* value = _trl_findAttr(atts,"value");
				if (!value) die("cond node must contain 'value' attribute");

				node = trnode_createReprNode(key,var,value);
			} else if (!strcmp(name,"repr")) {
				const XML_Char* key = _trl_findAttr(atts,"key");
				if (!key) die("repr node must contain 'key' attribute");

				node = trnode_createReprNode(key,NULL,NULL);
			} else {
				fprintf(stderr,"Got '%s': ",name);
				die("STRING/CASE nodes should "
				    "only contain \"cond\" or \"repr\" nodes");
			}
			_trl_push(node);
			break;
		case TRNODE_SWITCH:
			//in it we expect CASE
			if (!strcmp(name,"case")) {
				const XML_Char* value = _trl_findAttr(atts,"value");
				if (value == NULL)
					die("case elements must have a \"value\" attribute");
				trnode_t* node = trnode_createStringNode(value);
				_trl_push(node);
			} else {
				die("SWITCH should only contain \"case\" nodes");
			}
			break;
		case TRNODE_REPR:
			//we expect NOTHING
			die("REPR nodes should contain no contents");
			break;
		default:
			die("YOU SHOULD NOT SEE THIS.");
			break;
		}
	}

	//we are on root node
	if (!strcmp(name,"string")) {
		const XML_Char* key = _trl_findAttr(atts,"key");
		if (!key) die("string node must contain 'key' attribute");

		trnode_t* node = trnode_createStringNode(NULL);
		//put this in our TextResource hashtable
		ght_insert(gCurrentResource->strings,node,strlen(key),key);
		_trl_push(node);
	} else if(!strcmp(name,"switch")) {
		const XML_Char* key = _trl_findAttr(atts,"key");
		if (!key) die("switch node must contain 'key' attribute");
		const XML_Char* var = _trl_findAttr(atts,"var");
		if (!var) die("switch node must contain 'var' attribute");

		trnode_t* node = trnode_createSwitchNode(var);
		ght_insert(gCurrentResource->strings,node,strlen(key),key);
		_trl_push(node);
	}
}

static void _trl_elem_end(void *userData, const XML_Char *name) {
	//pop the node and stick it in the parent node if it exists
	gLastNode = NULL;
	trnode_t* node = _trl_pop();
	if (gNodeStack)
		trnode_addLast(gNodeStack,node);
	else { //this is a top level node

	}

}

static void _trl_elem_text(void *userData,const XML_Char *s,int len) {
	//this can be put somewhere better, so i don't have to keep killing it
	bstring newline = bfromcstr("\n");
	bstring tab = bfromcstr("\t");
	bstring space = bfromcstr(" ");

	//create a text node and stick it to the end of the current list, trim spaces of course
	if (gNodeStack && gNodeStack->type == TRNODE_STRING) {
		bstring str = blk2bstr(s,len);

		bfindreplace(str,newline,space,0);
		bfindreplace(str,tab,space,0);
		if (blength(str)) {
			if (gLastNode) {
				assert(gLastNode->type == TRNODE_TEXT);
				bconcat(gLastNode->textData,str);
			} else {
				trnode_t* node = trnode_createTextNode(str);
				trnode_addLast(gNodeStack,node);
				gLastNode = node;
			}
		}
		bdestroy(str);
	}

	bdestroy(newline);
	bdestroy(tab);
	bdestroy(space);
}


#define BUFFSIZE 8192
static char gBuff[BUFFSIZE];

int _TextResource_loadResource(TextResource tr, const char* filename) {
	gNodeStack = NULL;
	gCurrentResource = tr;
	gLastNode = NULL;

	XML_Parser p = XML_ParserCreate(NULL);

	if (p == NULL) {
		fprintf(stderr,"XML_Parser allocation fail\n");
	}

	XML_SetElementHandler(p,_trl_elem_start, _trl_elem_end);
	XML_SetCharacterDataHandler(p,_trl_elem_text);

	FILE* file = fopen(filename,"r");

	if (file == NULL)
		fprintf(stderr, "Text Resource File open failed.\n");

	for (;; ) {
		int done;
		int len;

		len = fread(gBuff, 1, BUFFSIZE, file);
		if (ferror(file)) {
			fprintf(stderr, "Text Resource File Read error\n");
			fclose(file);
			exit(-1);
		}
		done = feof(file);

		if (!XML_Parse(p, gBuff, len, done)) {
			fprintf(stderr, "Text Resource File Parse error at line %d:\n%s\n",
			        XML_GetCurrentLineNumber(p),
			        XML_ErrorString(XML_GetErrorCode(p)));
			fclose(file);
			exit(-1);
		}

		if (done)
			break;
	}
	fclose(file);
	XML_ParserFree(p);

	return 1;
}
