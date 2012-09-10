#include "gamesman.h"
#include <errno.h>

#define SA struct sockaddr

#define MAXNUMCLIENTS 256

#define MAXNAMELENGTH 100

#define MAXLINE 4096
#define LISTENQ 1024
#define max(a,b) (((a)>(b)) ? (a) : (b))

void execute(char * command, char * output)
{
	FILE * filep;
	int i = 0;
	char thechar;
	strcpy(output, "");
	strcat(command, " > .tempOut");
	system(command);
	if(!(filep = fopen(".tempOut", "r")))
		printf("Cannot open temp file for reading, quitting.\n"), exit(1);

	while((thechar = fgetc(filep)) != EOF)
		output[i++] = thechar;
	output[i] = (char)NULL;
	fclose(filep);
	remove(".tempOut");
}

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

void unstuff(char * source, char * dest)
{
	int a = getLength(source);
	memcpy(dest, source+2, a-2);
	dest[a-2] = (char)NULL;
}

char fromsocket[MAXNUMCLIENTS][MAXLINE];
char tosockbuf[MAXNUMCLIENTS][MAXLINE];

char * tosockbegptr[MAXNUMCLIENTS];
char * tosockendptr[MAXNUMCLIENTS];

char * stdoutendptr;
char * stdoutbegptr;
char stdoutbuf[MAXLINE];

int clients[MAXNUMCLIENTS];
int j[MAXNUMCLIENTS];
short lengths[MAXNUMCLIENTS];
int joins[MAXNUMCLIENTS];

void ActUponClientInput(char * buffer, int row, int sockfd);

int paslength;
char username[50][256];
char password[50][256];

FILE * fileps[MAXNUMCLIENTS];

int main (int argc, char ** argv)
{
	int listenfd, serv_port, i, maxfd, clilen, connfd, val;
	struct sockaddr_in cliaddr, servaddr;
	fd_set rset, wset;
	struct timeval tval;
	FILE * pass;

	stdoutbegptr = stdoutendptr = stdoutbuf;
	for(i = 0; i < MAXNUMCLIENTS; i++)
	{
		tosockbegptr[i] = tosockendptr[i] = tosockbuf[i];
		clients[i] = -1;
		j[i] = 0;
		lengths[i] = 0;
		joins[i] = 0;
		fileps[i] = NULL;
	}

	if (argc != 2) printf("Usage: %s <server port>\n", argv[0]), exit(1);
	sscanf(argv[1], "%d", &serv_port);
	if(serv_port <= 0) printf("Usage: port number must be a positive integer\n"), exit(1);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(serv_port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(servaddr.sin_zero), '\0', 8); //zero rest of struct

	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	val = fcntl(listenfd, F_GETFL, 0);
	fcntl(listenfd, F_SETFL, val | O_NONBLOCK);

	val = fcntl(STDOUT_FILENO, F_GETFL, 0);
	fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	maxfd = max(listenfd, STDOUT_FILENO);

	while(1)
	{
		tval.tv_sec = 0;
		tval.tv_usec = 0;

		FD_ZERO(&rset);
		FD_ZERO(&wset);

		FD_SET(listenfd, &rset);

		if(stdoutendptr > stdoutbegptr) FD_SET(STDOUT_FILENO, &wset);
		for(i = 0; i < MAXNUMCLIENTS; i++)
		{
			if(clients[i] > 0)
			{
				if(tosockendptr[i] > tosockbegptr[i])
					FD_SET(clients[i], &wset);
				FD_SET(clients[i], &rset);
			}
		}

		select(maxfd + 1, &rset, &wset, NULL, &tval);

		if(FD_ISSET(listenfd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
			if(connfd < 0)
			{
				if(errno != EWOULDBLOCK)
					fprintf(stderr, "internal socket failure. Exitting\n"), exit(1);
			}
			else
			{
				for(i = 0; i < MAXNUMCLIENTS; i++)
				{
					if(clients[i] < 0)
					{
						clients[i] = connfd;
						val = fcntl(connfd, F_GETFL, 0);
						fcntl(connfd, F_SETFL, val | O_NONBLOCK);
						joins[i] = 0;
						break;
					}
				}
				if(i == MAXNUMCLIENTS)
				{
					// Eventually handle some error here
				}
				else if(connfd > maxfd) maxfd = connfd;
				printf("Just accepted a connection\n");
			}
		}
		if(FD_ISSET(STDOUT_FILENO, &wset))
		{
			int nwritten = write(STDOUT_FILENO, stdoutbegptr, stdoutendptr - stdoutbegptr);
			if(nwritten < 0)
			{
				if(errno != EWOULDBLOCK)
					fprintf(stderr, "Non-Blocking IO Error\n"), exit(1);
			}
			else
			{
				stdoutbegptr += nwritten;
				if(stdoutendptr == stdoutbegptr)
					stdoutbegptr = stdoutendptr = stdoutbuf;
			}
		}
		for(i = 0; i < MAXNUMCLIENTS; i++)
		{
			if(clients[i] < 0) continue;

			if(FD_ISSET(clients[i], &wset))
			{
				int n = write(clients[i], tosockbegptr[i], tosockendptr[i] - tosockbegptr[i]);
				tosockbegptr[i] += n;
				if(tosockbegptr[i] == tosockendptr[i])
					tosockbegptr[i] = tosockendptr[i] = tosockbuf[i];
			}
			if(FD_ISSET(clients[i], &rset))
			{
				char schar;
				int n;
				if((n=read(clients[i], &schar, 1)) < 0)
				{
					if(errno != EWOULDBLOCK)
						fprintf(stderr, "Non-Blocking IO Error\n"), exit(1);
				}
				else if(n == 0)
				{
					close(clients[i]);
					stdoutendptr += sprintf(stdoutendptr, "disconnected: client %d closed connection\n", i+1);
					clients[i] = -1;
					lengths[i] = 0;
					j[i] = 0;
					fclose(fileps[i]);
					continue;
				}
				else if(n > 0)
				{
					fromsocket[i][j[i]] = schar;
					j[i] = j[i] + 1;
					if(j[i] == 2)
					{
						memcpy(lengths+i, fromsocket[i], 2);
						lengths[i] = ntohs(lengths[i]);
					}
					/// no else if here...
					if(j[i] == lengths[i])
					{
						fromsocket[i][j[i]] = (char)NULL;
						ActUponClientInput(fromsocket[i], i, clients[i]);
						j[i] = 0;
						lengths[i] = 0;
					}
					else
						fromsocket[i][j[i]] = (char)NULL;
				}
			}
		}
	}

	return 0;
}

void ActUponClientInput(char * inputstart, int row, int sockfd)
{
	int i, a;
	char commandName[256];
	char gameName[256];
	char optionstring[64];
	int option;
	char response[1024];
	char buffer[1024];
	char * input = inputstart;

	input += 2;

	for(i = 0; *input != ' ' && *input; i++, input++)
		commandName[i] = *input;
	commandName[i] = (char)NULL;
	while(*input == ' ') input++;

	if(!strcmp(commandName, "GameList"))
	{
		FILE * filep;
		if((filep = fopen("gamelist.txt", "r")) == NULL)
		{
			printf("Unable to open gamelist.txt\n");
			close(sockfd);
			return;
		}
		fgets(response, 1020, filep);
		response[strlen(response)-1] = (char)NULL;
		fclose(filep);
	}
	else if(!strcmp(commandName, "NumberOfOptions"))
	{
		char syscommand[256];
		for(i = 0; input[i] != ' ' && input[i]; i++)
			gameName[i] = input[i];
		gameName[i] = (char)NULL;
		sprintf(syscommand, "./%s -%s", gameName, "numberOfOptions");
		execute(syscommand, response);
	}
	else
	{
		char syscommand[256];
		strcpy(syscommand, "./");
		strcat(syscommand, inputstart+2);
		execute(syscommand, response);
	}

	a = stuff(response, buffer);
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
