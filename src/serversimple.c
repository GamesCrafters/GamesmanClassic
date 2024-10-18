#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <signal.h>

#define SERV_PORT 2020
#define LISTENQ 1024
#define SA struct sockaddr

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
	setLength(dest, a);
	return a+2;
}

void go(int fd)
{
	char input[2048];
	int a;
	readn(fd, input, 2);
	a = getLength(input);
	readn(fd, input, a-2);
	ActUponClientInput(input, fd);
}

void ActUponClientInput(char * input, int sockfd)
{
	int i, a;
	char commandName[256];
	char gameName[256];
	char optionstring[64];
	int option;
	char response[1024];
	char buffer[1024];

	input += 2;

	for(i = 0; input[i] != ' '; i++)
		commandName[i] = input[i];
	commandName[i] = (char)NULL;
	input++;

/*	for(i = 0 ; input[i] != ' ' ; i++)
                gameName[i] = input[i] ;
        gameName[i] = (char)NULL ;
        input++ ;
        for(i = 0 ; input[i] != ' ' ; i++)
                optionstring[i] = input[i] ;
        optionstring[i] = (char)NULL ;
        input++ ;
        sscanf(optionstring, "%d", &option) ;
 */
	if(!strcmp(commandName, "GameList"))
	{
		FILE * filep;
		printf("Just read a GameList packet\n");
		if((filep = fopen("gamelist.txt", "r")) == NULL)
		{
			close(sockfd);
			return;
		}
		fgets(response+2, 1020, filep);
		response[strlen(response)-1] = (char)NULL;
		fclose(filep);
	}
	else if(!strcmp(commandName, "GameOptions"))
	{
		//char syscommand[256] ;
	}
	else if(!strcmp(commandName, "Primitive"))
	{
	}
	else if(!strcmp(commandName, "DoMove"))
	{
	}
	else if(!strcmp(commandName, "Value"))
	{
	}
	else if(!strcmp(commandName, "Analyze"))
	{
	}
	else if(!strcmp(commandName, "GenerateMoves"))
	{
	}
	else if(!strcmp(commandName, "PrintPosition"))
	{
	}
	else if(!strcmp(commandName, "PrintMove"))
	{
	}
	else if(!strcmp(commandName, "ConvertTextInputToMove"))
	{
	}

	a = stuff(buffer, response);
	writen(sockfd, buffer, a);
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

int main(int argc, char **argv)
{
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	for (;; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

		if ( (childpid = fork()) == 0) {        /* child process */
			close(listenfd);        /* close listening socket */
			go(connfd);     /* process the request */
			exit(0);
		}
		close(connfd);                  /* parent closes connected socket */
	}
}
