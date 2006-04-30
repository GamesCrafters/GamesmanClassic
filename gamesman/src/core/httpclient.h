#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "gamesman.h"
//#include <string.h>
//#include <stdlib.h>

//#include <endian.h>
//#include <byteswap.h>


//byte conversion for 64 bit integers
#ifdef WORDS_BIGENDIAN
#define ntohll(x) (x)
#else

#define ntohll(x) (((long long) ntohl((x) & 0xffffffff)) << 32 | (((long long) ntohl((x) >> 32))))
#endif // WORDS_BIGENDIAN

#define htonll ntohll 



/* STRUCTS */
union sock
{
	struct sockaddr res;
	struct sockaddr_in req;
};

struct header_struct
{
	char *name;
	char *value;
	struct header_struct *next;
};
typedef struct header_struct header;

struct httpreq_struct
{
	union sock sock;
    struct hostent *serverAddr;
	char *hostName;
	char *path;
	int portNum;
	struct header_struct *headers;
};
typedef struct httpreq_struct httpreq;

struct httpres_struct
{
	char *status;
	char *body;
	int bodyLength;
	struct header_struct *headers;
};
typedef struct httpres_struct httpres;


/* FUNCTION DECLARATIONS */
void net_itoa(int n, char s[]); //convert int to char
void parse(char url[], httpreq *req); //parse (private)
httpreq* newrequest(char url[]); //insantiate a requesto with a url
void addheader(httpreq *req, char name[], char value[]); //add header
void settype(httpreq *req, char value[]); //set request type
httpres* post(httpreq *req, char body[], int bodyLength); //post request: response
char* getheader(httpres *res, char name[]); //get header
void freeresponse(httpres *res); //free response when done
void readresponse(int sockFd, httpres *res); //read response (private)
void lcstrcpy(char to[], char from[]);  //lower case copy

