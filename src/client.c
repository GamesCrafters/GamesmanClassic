#include "gamesman.h"

////// SUNIL's CLIENT/SERVER CODE
#define MAXLINE 4096

#define max(a,b) (((a)>(b)) ? (a) : (b))

int sockfd;

int gameCount;
int gameIndex;
char ** gameKeys;
char ** gameNames;

short getLength(void * line)
{
	short ret;
	memcpy(&ret, line, 2);
	ret = ntohs(ret);
	return ret;
}

void setLength(void * line, short length)
{
	length = htons(length);
	memcpy(line, &length, 2);
}

int stuff(char * source, char * dest)
{
	int a = strlen(source);
	memcpy(dest+2, source, a);
	setLength(dest, a+2);
	return a+2;
}

main(int argc, char ** argv)
{
	short port;
	char userinput[256];
	char buffer[256];
	int a;

	int InitClient();

	sscanf(argv[2], "%hd", &port);
	InitClient(argv[1], port);

	// Test the server
	do
	{
		gets(userinput);
		if(strcmp(userinput, "quit"))
		{
			int i;
			a = stuff(userinput, buffer);
			writen(sockfd, buffer, a);
			readn(sockfd, buffer, 2);
			a = getLength(buffer);
			a -= 2;
			readn(sockfd, buffer, a);
			buffer[a] = (char)NULL;
			printf("'%s'\n", buffer);
		}
	}
	while(strcmp(userinput, "quit"));
}

int InitClient(char * hostname, short port)
{
	struct sockaddr_in servaddr;
	struct hostent * he;
	char inbuf[MAXLINE];

	// Now for the client-specific stuff
	int maxfdp1, val;
	ssize_t n, nwritten;
	fd_set rset, wset;

	char mychar;
	int logon_msg_len;
	int a;
	short option;
	short type, length;

	char gameList[1024];
	char returnstring[1024];
	char countstring[64];
	char * string;
	int i;

	if((he = gethostbyname(hostname)) == NULL)
	{
		printf("sunil: gethostbyname failed\n");
		return 0;
	}
	if(port <= 0)
	{
		return 0;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(servaddr.sin_zero), '\0', 8);   // zero the rest of the structure

	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		printf("sunil: connect failed\n");
		return 0;
	}
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char      *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (errno == EINTR)
				nwritten = 0;           /* and call write() again */
			else
				return(-1);                     /* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char    *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
				nread = 0;              /* and call read() again */
			else
				return(-1);
		}
		else if (nread == 0)
			break;                          /* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);              /* return >= 0 */
}

void Gameline()
{
	printf("Welcome to gameline!!\n");
	//GPlayAgainstHuman() ;
	printf("goodbye now\n");
	close(sockfd);
	exit(1);
}
