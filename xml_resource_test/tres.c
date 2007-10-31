#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

#include "tres.h"

xmlDocPtr __tres_load_file(char* filename);
xmlChar* __lookup_str_in_doc(xmlDocPtr doc, char* key);
xmlNodePtr __lookup_matching_node_in_doc(xmlDocPtr doc, char* key);

inline wchar_t* utf8towchar(char* utf8) {
  //static cp = iconv_open("UTF-8","WCHAR_T")
  return NULL;
}

/**
 * Find a matching node in the document
 * 
 * @return the string node with specified key  
 */ 
xmlNodePtr __lookup_matching_node_in_doc(xmlDocPtr doc, char* key) {
  static char tmp[512];
  
  xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
  
  //beware of buffer overflows!
  sprintf(tmp,"//string[@key='%s']",key);

  //and proper escaping!
  context = xmlXPathNewContext(doc);
	result = xmlXPathEvalExpression((xmlChar*)tmp, context);
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    return NULL;
  }
  //take first result
  xmlNodePtr ret = result->nodesetval->nodeTab[0];
  xmlXPathFreeObject(result);
  xmlXPathFreeContext(context);
  return ret;
}

xmlChar* __lookup_str_in_doc(xmlDocPtr doc, char* key) {
  xmlNodePtr node = __lookup_matching_node_in_doc(doc,key);
  if (node == NULL)
    return NULL;
  //for now lets just grab the contents
  xmlChar* str = node->children->content;
  //this str and that node shouldn't need to be freed, they are read from tree
  return str;
}

xmlDocPtr __tres_load_file(char* filename) {
  xmlDocPtr doc = xmlParseFile(filename);
  xmlNodePtr node;
  if (doc == NULL) {
    fprintf(stderr,"Text resource invalid: %s\n",filename);
    return NULL;
  }

  node = xmlDocGetRootElement(doc);
  if ( node == NULL ) {
    fprintf(stderr,"Text resource empty: %s\n",filename);
    xmlFreeDoc(doc);
    return NULL;
  }

  if (xmlStrcmp(node->name, (const xmlChar *) "resource")) {
    fprintf(stderr, "Text resource root node != \"resource\": %s\n", filename);
    xmlFreeDoc(doc);
    return NULL;
  }
  return doc;
}

int tres_loadResource(tres* obj, char* file) {
  xmlDocPtr doc =  __tres_load_file(file);
  if ( doc == NULL )
    return 0;

  tres_file* fo = (tres_file*)malloc(sizeof(tres_file));
  fo->prev = obj->last_file;
  fo->doc = doc;
  obj->last_file = fo;

  return 1;
}

tres* tres_create() {
  tres* obj = (tres*)malloc(sizeof(tres));
  obj->last_file = NULL;
  return obj;
}

char* tres_lookupStr(tres* obj, char* key) {
  xmlChar* result = NULL;
  tres_file* file = obj->last_file;
  while ( file != NULL ) {
    result = __lookup_str_in_doc(file->doc,key);
    if ( result != NULL )
      goto after;
    file = file -> prev;
  }

  after:
  if ( result == NULL )
    return NULL;
  
  //do substitutions later
  return (char*)result;
}

void tres_system_init() {
  static int done = 0;
  if (done)
    return; //alert them? maybe.

  LIBXML_TEST_VERSION

}
