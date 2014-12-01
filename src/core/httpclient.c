#include "httpclient.h"
#include <errno.h>
#include "globals.h"
#include <string.h>

/* FUNCTIONS */

/* TEST FUNCTION ONLY */

/*
   int main(int argc, char *argv[])
   {
        httpreq *req;
        httpres *res;
        char *body;
        int r, i;
        //char blah[] = "136.152.170.158:8080/gamesman/GamesmanServlet";
        char * url = malloc(strlen(argv[1])+1);

        // Read the args
    if (argc < 2)
    {
       fprintf(stderr,"usage %s url\n", argv[0]);
       exit(1);
    }

    while (1){
        memcpy(url,argv[1],strlen(argv[1])+1);
        printf("trying to resolve %s\n",url);
        req = newrequest(url);
        addheader(req, "type", "MATT");
        addheader(req, "header1", "val1");
        addheader(req, "Header2", "monkey monkey doo doo face");

        printf("%s:%d%s\n", req->hostName, req->portNum, req->path);

        body = "abcdefghijklmnopqrstuvwxyz";

        res = post(req, body, 27);

        printf("%s: %s\n", "Date", getheader(res, "date"));
        printf("%s: %s\n", "Content-Length", getheader(res, "Content-Length"));
        printf("%s: %s\n", "ReturnCode", getheader(res, "ReturnCode"));
        printf("%s: %s\n", "ReturnMessage", getheader(res, "ReturnMessage"));

        r = res->bodyLength;
        printf("response: ");
        for (i=0; i<r; i++)
                printf("%d ", res->body[i]);

        printf("\ndone\n");
        freeresponse(res);
    }
        return 0;
   }
 */

#ifndef htonll
/**
 * Converts an unsigned long long from host byte order to network byte order
 * depending on the endian-ness of the host system.
 *
 * n - unsigned long long (usually a POSITION) in host's byte order
 * returns an unsigned long long in network byte order
 */
unsigned long long htonll(unsigned long long n)
{
	short w = 0x4321;
	if ((*(char *)&w) != 0x21 )
		return n;
	else
		return (((unsigned long long)htonl(n)) << 32) + htonl(n >> 32);
}
#endif

/**
 * Copies the http response status into the status handle.
 *
 * res - httpres struct to read the status from
 * status - a handle to the status string
 */
void getstatus(httpres *res, char** status)
{
	*status = NULL;
	if (res != NULL && res->status != NULL)
	{
		if ((*status = malloc(strlen(res->status)+1)) == NULL)
		{
			fprintf(stderr,"ERROR, could not allocate memory for response status\n");
			return;
		}
		strcpy(*status, res->status);
	}
}

/**
 * Copies the value of the specified header into the hdrVal string. The hdrVal
 * string may be NULL if no header exists with the specified name.
 *
 * res - httpres struct to read header values from
 * name - name of the header for which we want the value
 * hdrVal - handle to the header value string
 */
void getheader(httpres *res, char name[], char** hdrVal)
{
	char *lname;
	char *lhname;
	*hdrVal = NULL;
	header *currHdr;

	if (res == NULL)
		return;

	// Lower case the name so we can do case-insensitive comparisons
	if ((lname = malloc(strlen(name)+1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for header name comparision\n");
		return;
	}
	lcstrcpy(lname, name);

	// Loop through the headers until we find one
	currHdr = res->headers;
	while (currHdr != NULL)
	{
		if (currHdr->name != NULL)
		{
			if ((lhname = malloc(strlen(currHdr->name)+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for header name comparision\n");
				return;
			}
			lcstrcpy(lhname, currHdr->name);
			if (strcmp(lhname, lname) == 0)
			{
				if (currHdr->value != NULL)
				{
					if ((*hdrVal = malloc(strlen(currHdr->value)+1)) == NULL)
					{
						fprintf(stderr,"ERROR, could not allocate memory for header value\n");
						return;
					}
					strcpy(*hdrVal, currHdr->value);
				}
				free(lhname);
				break;
			}
			free(lhname);
		}
		currHdr = currHdr->next;
	}
	free(lname);
}

/**
 * Copies the from string to the to string and lower cases it
 * in the process.
 *
 * to - char array to copy into
 * from - char array to copy from
 */
void lcstrcpy(char to[], char from[])
{
	int i;
	int len = strlen(from);
	for (i=0; i<len; i++)
		to[i] = tolower(from[i]);
	to[i] = '\0';
}

/**
 * Frees the specified httpres struct when no longer needed. Not
 * using this function will result in a memory leak.
 *
 * res - httpres struct to free
 */
void freeresponse(httpres *res)
{
	header *currHdr;
	header *tmpHdr;

	if (res == NULL)
		return;

	// Free the malloc'd memory
	currHdr = res->headers;
	tmpHdr = NULL;
	while (currHdr != NULL)
	{
		if (tmpHdr != NULL)
			free(tmpHdr);
		tmpHdr = currHdr;
		if (currHdr->name != NULL)
			free(currHdr->name);
		if (currHdr->value != NULL)
			free(currHdr->value);
		currHdr = currHdr->next;
	}
	if (tmpHdr != NULL)
		free(tmpHdr);
	if (res->body != NULL)
		free(res->body);
	free(res);
}

/**
 * POST's the specified httpreq to it's preconfigured url with
 * all preconfigured headers and the body content (if any). Frees
 * the httpreq and all its request headers. Populates the httpres struct
 * representing the HTTP response data (which must be freed later using
 * freeresponse). Returns 0 if successful. If not successful, returns
 * a non-zero value and populates the errMsg string.
 *
 * req - httpreq struct to POST to it's url/addr
 * body - content for the body of the HTTP POST, can be NULL
 * bodyLength - length of the body content, can be 0 if body is NULL
 * res - handle to the httpres struct
 * errMsg - handle to the error message string
 * returns httpres struct representing the HTTP response data
 */
int post(httpreq *req, char body[], int bodyLength, httpres** res, char** errMsg)
{
	header *currHdr = NULL;
	header *tmpHdr = NULL;
	char buffer[64];
	int sockFd;
	int n;
	*res = NULL;
	*errMsg = NULL;

	// Check the request
	if (req == NULL)
	{
		mallocstrcpy(errMsg, "NULL httpreq struct pointer");
		return 1;
	}

	// Add the content-length header
	net_itoa(bodyLength, buffer);
	addheader(req, "Content-Length", buffer);

	// Create a socket
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		connecterror(buffer);
		mallocstrcpyext(errMsg, "ERROR, creating socket: ", buffer);
		return errno;
	}

	// Connect to the socket
	if (connect(sockFd, &(req->sock.res), sizeof(struct sockaddr_in)) < 0)
	{
		connecterror(buffer);
		mallocstrcpyext(errMsg, "ERROR, opening socket: ", buffer);
		return errno;
	}

	// Submit the http request
	n = write(sockFd, "POST ", 5);
	n = write(sockFd, req->path, strlen(req->path));
	n = write(sockFd, " HTTP/1.1\r\n", 11);
	// Add the headers
	currHdr = req->headers;
	while (currHdr != NULL)
	{
		n = write(sockFd, currHdr->name, strlen(currHdr->name));
		n = write(sockFd, ": ", 2);
		n = write(sockFd, currHdr->value, strlen(currHdr->value));
		n = write(sockFd, "\r\n", 2);
		currHdr = currHdr->next;
	}

	// Add the extra line to separate headers from body
	n = write(sockFd, "\r\n", 2);
	// Add the body (if any)
	if (bodyLength > 0)
		n = write(sockFd, body, bodyLength);

	// Create the response
	if ((*res = malloc(sizeof(httpres))) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http response\n");
		return 1;
	}

	// NULL out the initial values
	(*res)->headers = NULL;
	(*res)->status = NULL;
	(*res)->body = NULL;
	(*res)->statusCode = 0;
	(*res)->bodyLength = 0;

	// Read the response
	readresponse(sockFd, *res);
	close(sockFd);
	shutdown(sockFd,2);

	// Free the malloc'd memory
	currHdr = req->headers;
	while (currHdr != NULL)
	{
		if (currHdr->name != NULL)
			free(currHdr->name);
		if (currHdr->value != NULL)
			free(currHdr->value);
		if (tmpHdr != NULL)
			free(tmpHdr);
		tmpHdr = currHdr;
		currHdr = currHdr->next;
	}
	if (tmpHdr != NULL)
		free(tmpHdr);
	if (req->hostName != NULL)
		free(req->hostName);
	if (req->path != NULL)
		free(req->path);
	free(req);

	return 0;
}

/**
 * Reads the HTTP response from the specified socket and populates the specified
 * httpres struct accordingly.
 *
 * sockFd - socket file descriptor from which to read
 * res - httpres struct to populate
 */
void readresponse(int sockFd, httpres *res)
{
	header *currHdr = NULL;
	header *tmpHdr = NULL;
	char *pos = NULL;
	char buffer[512];
	char c[1];
	int p;
	int n;

	if (res == NULL)
		return;

	// Read each line at a time
	while (1)
	{
		p = 0;
		while ((n = read(sockFd, c, 1)) != EOF)
		{
			if (c[0] == '\r')
				continue;
			else if (c[0] == '\n')
				break;
			else
				buffer[p++] = c[0];
		}
		buffer[p] = '\0';

		// Stop reading line-by-line when we encounter a blank line
		if (strlen(buffer) == 0)
			break;

		if (res->status == NULL)
		{
			// First line is the HTTP status line
			if ((res->status = malloc(strlen(buffer)+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for http response status\n");
				return;
			}
			strcpy(res->status, buffer);

			// Parse out the status code
			pos = strtok(buffer, " ");
			if (pos != NULL)
			{
				if ((pos = strtok(NULL, " ")) != NULL)
					res->statusCode = atoi(pos);
			}
		}
		else
		{
			// Make a header for this line
			if ((currHdr = malloc(sizeof(header))) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for http response header\n");
				return;
			}

			// NULL out the initial values
			currHdr->next = NULL;
			currHdr->name = NULL;
			currHdr->value = NULL;

			if ((pos = strchr(buffer, ':')))
			{
				// Line has a ':' char
				*pos = '\0';
				p = strlen(buffer);
				if ((currHdr->name = malloc(p+1)) == NULL)
				{
					fprintf(stderr,"ERROR, could not allocate memory for header name\n");
					return;
				}
				strcpy(currHdr->name, buffer);

				// Skip spaces and tabs
				*pos = ' ';
				while (p < 512 && (buffer[p] == ' ' || buffer[p] == '\t'))
				{
					p++;
					pos++;
				}

				// Now read to the end
				if ((currHdr->value = malloc(strlen(pos)+1)) == NULL)
				{
					fprintf(stderr,"ERROR, could not allocate memory for header value\n");
					return;
				}
				strcpy(currHdr->value, pos);
			}
			else
			{
				// Line had no ':' so add the whole line as just a header name, no value
				if ((currHdr->name = malloc(strlen(buffer)+1)) == NULL)
				{
					fprintf(stderr,"ERROR, could not allocate memory for header name\n");
					return;
				}
				strcpy(currHdr->name, buffer);
			}

			// Add the header
			if (tmpHdr != NULL)
				tmpHdr->next = currHdr;
			else
				res->headers = currHdr;
			tmpHdr = currHdr;
			//printf("currHdr: %s = %s\n", currHdr->name, currHdr->value);
		}
	}

	// Now read any body content
	getheader(res, "Content-Length", &pos);
	if (pos != NULL)
	{
		p = atoi(pos);
		if (p > 0)
		{
			if ((res->body = malloc(p+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for response body\n");
				return;
			}
			res->bodyLength = read(sockFd, res->body, p);
			res->body[p] = '\0';
			//printf("expected: %d read: %d body: '%s'\n", p, res->bodyLength, res->body);
		}
	}
}

/**
 * Converts the specified int into a character equivalent and places the
 * value in the specified char array. Assumes the char array has enough
 * space to support the number of converted digits.
 *
 * n - int to convert
 * s - char array that will hold the result of the conversion
 */
void net_itoa(int n, char s[])
{
	int c, i, j;
	if ((c = n) < 0)
		n = -n;
	i =0;
	do
	{
		s[i++] = n % 10 + '0';
	}
	while ((n /= 10) > 0);
	if (c < 0)
		s[i++] = '-';
	s[i] = '\0';

	// Now reverse
	for (i=0, j=strlen(s)-1; i<j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/**
 * Sets the type header to the specified httpreq struct. The specified
 * value is copied (by value) into the struct. Thus, changes to that
 * character array will not have any further affect.
 *
 * req - httpreq struct to modify
 * value - value for the header t
 */
void settype(httpreq *req, char value[])
{
	if (req == NULL)
		return;
	addheader(req, "TYPE", value);
}

/**
 * Adds the specified header to the specified httpreq struct. The
 * specified name/value are copied (by value) into the struct. Thus,
 * changes to these character arrays will not have any further affect.
 *
 * req - httpreq struct to modify
 * name - name of the header to add
 * value - value for the header to add
 */
void addheader(httpreq *req, char name[], char value[])
{
	header *hdr = NULL;
	header *currHdr = NULL;

	// Create a header
	if ((hdr = malloc(sizeof(header))) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request header\n");
		return;
	}
	hdr->next = NULL;
	hdr->name = NULL;
	hdr->value = NULL;

	if ((hdr->name = malloc(strlen(name) + 1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request header name\n");
		return;
	}
	strcpy(hdr->name, name);
	if ((hdr->value = malloc(strlen(value) + 1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request header value\n");
		return;
	}
	strcpy(hdr->value, value);

	// Add to the httpreq
	if (req->headers == NULL)
	{
		req->headers = hdr;
	}
	else
	{
		currHdr = req->headers;
		while (currHdr->next != NULL)
			currHdr = currHdr->next;
		currHdr->next = hdr;
	}
}

/**
 * Creates a new httpreq to represent the http request to make and
 * populates the handle to the httpreq struct for later using in the
 * post function. Returns 0 if successful. Otherwise, returns a non-zero
 * value and populates the errMsg string.
 *
 * WARNING: clobbers url
 *
 * url - url the http request will use (minus the http:// prefix)
 * req - handle to the httpreq struct
 * errMsg - handle to the error message string
 */
int newrequest(char url[], httpreq** req, char** errMsg)
{
	*req = NULL;
	*errMsg = NULL;

	if (req == NULL)
	{
		mallocstrcpy(errMsg, "NULL httpreq struct pointer");
		return 1;
	}

	if (url == NULL || strlen(url) == 0)
	{
		mallocstrcpy(errMsg, "Cannot specify a NULL url for a request");
		return 1;
	}

	// Create the httpreq
	if ((*req = malloc(sizeof(httpreq))) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request\n");
		return 1;
	}

	// NULL out the initial values
	(*req)->headers = NULL;
	(*req)->serverAddr = NULL;
	(*req)->hostName = NULL;
	(*req)->path = NULL;
	(*req)->portNum = 80;

	// Parse the url and add the info to the httpreq
	if (parse(url, *req, errMsg) != 0)
		return 1;

	// Lookup the host addr and validate it
	if (((*req)->serverAddr = gethostbyname((*req)->hostName)) == NULL)
	{
		mallocstrcpyext(errMsg, "ERROR, no such host: ", (*req)->hostName);
		return 1;
	}

	// Setup the socket address
	memcpy(&((*req)->sock.req.sin_addr.s_addr), *((*req)->serverAddr->h_addr_list), sizeof(struct in_addr));
	(*req)->sock.req.sin_family = AF_INET;
	(*req)->sock.req.sin_port = htons((*req)->portNum);

	// Add the required headers
	addheader(*req, "Host", (*req)->hostName); // Required by HTTP 1.1
	addheader(*req, "Connection", "close"); // Don't keep the conn open afterwards
	addheader(*req, "User-Agent", "Gamesman/1.0"); // So we can identify ourselves
	addheader(*req, "Content-Type", "application/octet-stream"); // Body content will be binary

	// Ready to go
	return 0;
}

/**
 * Parses the specified url into the hostName, path, and port
 * and assigns the values into the specified httpreq struct.
 * Returns 0 if successful. If unsuccessful, populates the errMsg
 * error message string and returns non-zero.
 *
 * url - url to parse
 * req - pointer to httpreq
 * errMsg - handle to the error message string
 * returns 0 if succesful, non-zero otherwise
 */
int parse(char url[], httpreq *req, char** errMsg)
{
	char *pos1 = NULL;
	char *pos2 = NULL;
	int n;
	*errMsg = NULL;

	// Parse the url
	if ((pos1 = strchr(url, ':')))
	{
		// Url has a port number specified
		// Read just the host name, up to the ':'
		*pos1 = '\0';
		n = strlen(url);
		if ((req->hostName = malloc(n+1)) == NULL)
		{
			fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
			return 1;
		}
		strcpy(req->hostName, url);
		// Move past the ':' char
		pos1++;
		if ((pos2 = strchr(pos1,'/')))
		{
			// Url has a '/' char
			// Read just the port, up to the '/'
			*pos2 = '\0';
			req->portNum = atoi(pos1);
			// Now read from the '/' to the end
			*pos2 = '/';
			n = strlen(pos2);
			if ((req->path = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for path\n");
				return 1;
			}
			strcpy(req->path, pos2);
		}
		else
		{
			// Just host name and port, but use malloc calls
			// so we can free later as we will need to in
			// other cases
			req->portNum = atoi(pos1);
			// Default path to '/'
			if ((req->path = malloc(1+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for path\n");
				return 1;
			}
			strcpy(req->path, "/");
		}
	}
	else
	{
		req->portNum = 80;
		if ((pos1 = strchr(url, '/')))
		{
			// Url has a '/' char
			// Read just the host name, up to the '/'
			*pos1 = '\0';
			n = strlen(url);
			if ((req->hostName = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
				return 1;
			}
			strcpy(req->hostName, url);
			// Now read from the '/' to the end
			*pos1 = '/';
			n = strlen(pos1);
			if ((req->path = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for path\n");
				return 1;
			}
			strcpy(req->path, pos1);
		}
		else
		{
			// Just the host name, but use malloc calls
			// so we can free later as we will need to in
			// other cases
			n = strlen(url);
			if ((req->hostName = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
				return 1;
			}
			strcpy(req->hostName, url);
			// Default path to '/'
			if ((req->path = malloc(1+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
				return 1;
			}
			strcpy(req->path, "/");
		}
	}

	return 0;
}

/**
 * Checks the http response to see if it is valid. If it is, returns 0 and copies
 * no value to the errMsg string. If it is not valid, returns a non-zero value
 * and copies an error message into the errMsg string.
 *
 * res - the httpres struct to check
 * errMsg - handle to the error message string
 * returns 0 if everything checks out normal, non-zero otherwise
 */
int responseerrorcheck(httpres* res, char** errMsg)
{
	int errCode = 0;
	*errMsg = NULL;

	if (res == NULL)
	{
		mallocstrcpy(errMsg, "NULL httpres struct pointer");
		return 1;
	}

	if (res->statusCode != 200)
	{
		getstatus(res, errMsg);
		if (*errMsg == NULL)
		{
			mallocstrcpy(errMsg, "No response received from server.");
			return 1;
		}
		errCode = 1;

		// Print out the body (if any) to stderr
		if (res->bodyLength > 0)
			fprintf(stderr, "http response status code: %d %s\n%s", res->statusCode, res->status, res->body);
	}
	else
	{
		char* ecode_str;
		getheader(res, HD_RETURN_CODE, &ecode_str);
		if (ecode_str == NULL)
		{
			mallocstrcpy(errMsg, "Server sent back invalid response");
			errCode = 1;
		}
		else
		{
			int ecode = atoi(ecode_str);
			free(ecode_str);
			if (ecode != 0)
			{
				getheader(res,HD_RETURN_MESSAGE, errMsg);
				errCode = ecode;
			}
		}
	}
	return errCode;
}

/**
 * Copies the network error message for the current error into the specified errMsg
 * string.
 *
 * errMsg - error message string
 */
void connecterror(char errMsg[])
{
	if (errno == EOPNOTSUPP)
		strcpy(errMsg, "Operation not supported on transport endpoint.");
	else if (errno == EPFNOSUPPORT)
		strcpy(errMsg, "Protocol family not supported.");
	else if (errno == ECONNRESET)
		strcpy(errMsg, "Connection reset by peer.");
	else if (errno == ENOBUFS)
		strcpy(errMsg, "No buffer space available.");
	else if (errno == EAFNOSUPPORT)
		strcpy(errMsg, "Address family not supported by protocol family.");
	else if (errno == EPROTOTYPE)
		strcpy(errMsg, "Protocol wrong type for socket.");
	else if (errno == ENOTSOCK)
		strcpy(errMsg, "Socket operation on non-socket.");
	else if (errno == ENOPROTOOPT)
		strcpy(errMsg, "Protocol not available.");
	else if (errno == ESHUTDOWN)
		strcpy(errMsg, "Can't send after socket shutdown.");
	else if (errno == ECONNREFUSED)
		strcpy(errMsg, "Connection refused.");
	else if (errno == EADDRINUSE)
		strcpy(errMsg, "Address already in use.");
	else if (errno == ECONNABORTED)
		strcpy(errMsg, "Connection aborted.");
	else if (errno == ENETUNREACH)
		strcpy(errMsg, "Network is unreachable.");
	else if (errno == ENETDOWN)
		strcpy(errMsg, "Network interface is not configured.");
	else if (errno == ETIMEDOUT)
		strcpy(errMsg, "Connection timed out.");
	else if (errno == EHOSTDOWN)
		strcpy(errMsg, "Host is down.");
	else if (errno == EHOSTUNREACH)
		strcpy(errMsg, "Host is unreachable.");
	else if (errno == EINPROGRESS)
		strcpy(errMsg, "Connection already in progress.");
	else if (errno == EALREADY)
		strcpy(errMsg, "Socket already connected.");
	else if (errno == EDESTADDRREQ)
		strcpy(errMsg, "Destination address required.");
	else if (errno == EMSGSIZE)
		strcpy(errMsg, "Message too long.");
	else if (errno == EPROTONOSUPPORT)
		strcpy(errMsg, "Unknown protocol.");
	else if (errno == ESOCKTNOSUPPORT)
		strcpy(errMsg, "Socket type not supported.");
	else if (errno == EADDRNOTAVAIL)
		strcpy(errMsg, "Address not available.");
	else if (errno == ENETRESET)
		strcpy(errMsg, "Network interface reset.");
	else if (errno == EISCONN)
		strcpy(errMsg, "Socket is already connected.");
	else if (errno == ENOTCONN)
		strcpy(errMsg, "Socket is not connected.");
	else if (errno == ENOTSUP)
		strcpy(errMsg, "Not supported.");
	else if (errno == EMULTIHOP)
		strcpy(errMsg, "Multihop attempted.");
	else if (errno == EPROTO)
		strcpy(errMsg, "Protocol error.");
	else if (errno == ENOLINK)
		strcpy(errMsg, "The link has been severed.");
	else if (errno == EREMOTE)
		strcpy(errMsg, "The object is remote.");
	else if (errno == ENOSR)
		strcpy(errMsg, "Out of streams resources.");
	else if (errno == ETIME)
		strcpy(errMsg, "Timer expired.");
	else if (errno == ENODATA)
		strcpy(errMsg, "No data (for no delay io).");
	else if (errno == ENOSTR)
		strcpy(errMsg, "Device not a stream.");
	else if (errno == EIDRM)
		strcpy(errMsg, "Identifier removed.");
	else if (errno == ENOMSG)
		strcpy(errMsg, "No message of desired type.");
	else if (errno == EPIPE)
		strcpy(errMsg, "Broken pipe.");
	else if (errno == EMLINK)
		strcpy(errMsg, "Too many links.");
	else if (errno == EMFILE)
		strcpy(errMsg, "Too many open files.");
	else if (errno == ENFILE)
		strcpy(errMsg, "Too many open files in system.");
	else if (errno == EINVAL)
		strcpy(errMsg, "Invalid argument.");
	else if (errno == ENODEV)
		strcpy(errMsg, "No such device.");
	else if (errno == EXDEV)
		strcpy(errMsg, "Cross-device link.");
	else if (errno == ENOTBLK)
		strcpy(errMsg, "Block device required.");
	else if (errno == EFAULT)
		strcpy(errMsg, "Bad address.");
	else if (errno == EACCES)
		strcpy(errMsg, "Permission denied.");
	else if (errno == EBADF)
		strcpy(errMsg, "Bad file number.");
	else if (errno == ENXIO)
		strcpy(errMsg, "No such device or address.");
	else if (errno == EIO)
		strcpy(errMsg, "I/O error.");
	else if (errno == EINTR)
		strcpy(errMsg, "Interrupted system call.");
	else if (errno == EPERM)
		strcpy(errMsg, "Not super-user.");
	else
		sprintf(errMsg, "Unknown error %d.", errno);
}

/**
 * Malloc's enough memory to copy the string specified by msg into the handle specified
 * by errMsg. Returns 0 if successful, 1 otherwise.
 *
 * errMsg - handle that will point to a malloc'd copy of the msg string
 * msg - string to copy
 * returns 0 if successful, 1 otherwise
 */
int mallocstrcpy(char** errMsg, const char msg[])
{
	if ((*errMsg = malloc(strlen(msg)+1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for error message\n");
		return 1;
	}
	strcpy(*errMsg, msg);
	return 0;
}

/**
 * Malloc's enough memory to copy the strings specified by msg1 and msg2 into the handle
 * specified by errMsg. Returns 0 if successful, 1 otherwise.
 *
 * errMsg - handle that will point to a malloc'd copy of the msg1 + msg2 string
 * msg1 - first string to copy
 * msg2 - second string to copy
 * returns 0 if successful, 1 otherwise
 */
int mallocstrcpyext(char** errMsg, char msg1[], char msg2[])
{
	if ((*errMsg = malloc(strlen(msg1)+strlen(msg2)+1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for error message\n");
		return 1;
	}
	strcpy(*errMsg, msg1);
	strcat(*errMsg, msg2);
	return 0;
}
