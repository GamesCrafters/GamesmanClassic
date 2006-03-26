/*Mem db tester
  By Aaron STaley*/

//#include "gamesman.h"

#include <inttypes.h>
#include <stdio.h>
#include <zlib.h>
#include <netinet/in.h>

int op;
char * kDBName;

gzFile* 	filep;
int		dirty=1;
long long	CurrentPosition=0;
short	CurrentValue=0;

short* memdb_get_raw_file(long long pos)
{
        if(dirty || (pos != CurrentPosition)) {
		dirty = 0;
		CurrentPosition = pos;
                z_off_t offset = sizeof(short) + sizeof(long long) + sizeof(short)*pos;
                z_off_t zoffset = gztell(filep);
                if(offset >= zoffset)
                        gzseek(filep, offset-zoffset, SEEK_CUR);
                else
                        gzseek(filep, offset, SEEK_SET);
                gzread(filep, &CurrentValue, sizeof(short));
		CurrentValue = ntohs(CurrentValue);
		//printf("reading pos = "POSITION_FORMAT", value = %u\n", pos, CurrentValue);

        }
        return &CurrentValue;
}



int main(int agv, char ** agc){
  //  printf("%i\n",sizeof(long long));
  if (agv!=2){
    printf("jacked options\n");
    return 1;
  }
  
  if((filep = gzopen(agc[1], "rb")) == NULL){
    printf("could not load file\n");
     return 1;
  }
  printf("give me positions:\n");
  while (1){
    int i;
    int j;
    scanf("%d",&i);
    j=*(int*) memdb_get_raw_file((long long)i);
    printf("pos is %d\n",j);
  }
  //memdb_close_file();
  return 0;  


}
