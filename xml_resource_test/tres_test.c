#include <stdlib.h>
#include <stdio.h>
#include "tres.h"

int main(int argc, char** argv) {
  tres_system_init();

  tres* res = tres_create();
  tres_loadResource(res, "data/data1.xml");
  tres_loadResource(res, "data/data1_zh_cn.xml");
  char* str = tres_lookupStr(res, argv[1]);
  if ( str == NULL )
    printf("String not found: %s\n",argv[1]);
  else
    printf("%s\n",str);
  
  return 0;
}
