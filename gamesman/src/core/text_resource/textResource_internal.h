#ifndef TEXTRESOURCE_INTERNAL_H
#define TEXTRESOURCE_INTERNAL_H

#include <libxml/tree.h>

typedef struct tres_file {
  struct tres_file* prev;
  xmlDocPtr doc;
} tres_file;

typedef struct {
  tres_file *last_file;
} tres;

tres* tres_create();
int tres_loadResource(tres* obj, char* file);
char* tres_lookupStr(tres* obj, char* key);

inline wchar_t* utf8towchar(char* utf8);

//static, call this once
void tres_system_init();

#endif
