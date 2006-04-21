#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "gamesman.h"
#include <stdlib.h>

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
void itoa(int n, char s[]);
void parse(char url[], httpreq *req);
httpreq* newrequest(char url[]);
void addheader(httpreq *req, char name[], char value[]);
void settype(httpreq *req, char value[]);
httpres* post(httpreq *req, char body[], int bodyLength);
char* getheader(httpres *res, char name[]);
void freeresponse(httpres *res);
void readresponse(int sockFd, httpres *res);
void lcstrcpy(char to[], char from[]);

