 #include <langinfo.h>
 
 int main()
 {
   printf("%s\n",nl_langinfo(CODESET));
 }
