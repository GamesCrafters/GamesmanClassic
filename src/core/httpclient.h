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
#ifndef htonll
unsigned long long htonll(unsigned long long n); // convert from host to network byte order for long longs */
#endif
void net_itoa(int n, char s[]); //convert int to char
int parse(char url[], httpreq *req, char** errMsg); //parse (private)
int newrequest(char url[], httpreq**req, char** errMsg);  //insantiate a request with a url
void addheader(httpreq *req, char name[], char value[]); //add header
void settype(httpreq *req, char value[]); //set request type
int post(httpreq *req, char body[], int bodyLength, httpres**res, char** errMsg);  //post request: response
void getheader(httpres *res, char name[], char** hdrVal); //get header
void getstatus(httpres *res, char** status); // get a copy of the status
void freeresponse(httpres *res); //free response when done
void readresponse(int sockFd, httpres *res); //read response (private)
void lcstrcpy(char to[], char from[]);  //lower case copy
int responseerrorcheck(httpres *res, char** errMsg); // checks for bad server response and returns an error code and message
void connecterror(char errMsg[]); // copies the error message corresponding to the errno into the specified buffer
int mallocstrcpy(char** errMsg, const char msg[]); // copies a string into a malloc'd area of memory
int mallocstrcpyext(char** errMsg, char msg1[], char msg2[]); // copies and concatenates 2 strings into a malloc' area of memory

/* HEADERS AND VALUES */
#define HD_GET_VALUE_OF_POSITIONS "GetValueOfPositions"
#define HD_INIT_DATABASE "InitDatabase"
#define HD_LOGON_USER "LogonUser"
#define HD_LOGOFF_USER "LogoffUser"
#define HD_CREATE_USER "CreateUser"
#define HD_GET_USERS "GetUsers"
#define HD_REGISTER_GAME "RegisterGame"
#define HD_UNREGISTER_GAME "UnregisterGame"
#define HD_JOIN_GAME "JoinGame"
#define HD_RECEIVED_CHALLENGE "ReceivedChallenge"
#define HD_ACCEPT_CHALLENGE "AcceptChallenge"
#define HD_GET_GAME_STATUS "GetGameStatus"
#define HD_ACCEPTED_CHALLENGE "AcceptedChallenge"
#define HD_DESELECT_CHALLENGER "DeselectChallenger"
#define HD_SELECT_CHALLENGER "SelectChallenger"
#define HD_GET_LAST_MOVE "GetLastMove"
#define HD_SEND_MOVE "SendMove"
#define HD_GAME_OVER "GameOver"
#define HD_RESIGN "Resign"

#define HD_USERNAME  "Username"
#define HD_PASSWORD "Password"
#define HD_SECRET_KEY "SecretKey"
#define HD_LENGTH "Length"
#define HD_GAME_ID "GameId"
#define HD_GAME_NAME "GameName"
#define HD_GAME_VARIANT "GameVariant"
#define HD_GAME_DESCRIPTION "GameDescription"
#define HD_GAME_HOST_MOVES_FIRST "GameHostMovesFirst"
#define HD_STATUS "Status"
#define HD_ACCEPT "Accept"
#define HD_MOVE_FIRST "MoveFirst"
#define HD_CHALLENGER_USERNAME "ChallengerUsername"
#define HD_LAST_MOVE "LastMove"
#define HD_MOVE "Move"

#define HD_RETURN_CODE "ReturnCode"
#define HD_RETURN_MESSAGE "ReturnMessage"
