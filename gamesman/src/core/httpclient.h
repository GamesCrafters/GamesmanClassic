#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "gamesman.h"

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
	int statusCode;
	int bodyLength;
	struct header_struct *headers;
};
typedef struct httpres_struct httpres;


/* FUNCTION DECLARATIONS */
unsigned long long htonll(unsigned long long n); // convert from host to network byte order for long longs
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
void connecterror(FILE *stream); // prints the error message corresponding to the errno

