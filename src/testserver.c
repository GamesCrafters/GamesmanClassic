#include "gamesman.h"

////// SUNIL's CLIENT/SERVER CODE
#define MAXLINE 4096

#define max(a,b) (((a)>(b)) ? (a) : (b))

int sockfd;

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

main(int argc, char ** argv)
{
	short port;
	char userinput[256];
	char buffer[256];
	int a;

	sscanf(argv[2], "%hd", &port);
	InitClient(argv[1], port);

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

	a = stuff("GameList", gameList);

	writen(sockfd, gameList, a);

	printf("Wrote %d bytes\n", a);

	readn(sockfd, gameList, 2);
	a = getLength(gameList);
	a -= 2;
	readn(sockfd, gameList, a);
	gameList[a] = (char)NULL;

	printf("'%s'\n", gameList);

	return 1;
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

/*
   int Value(int position)
   {
        char buffer[256] ;
        char string[256] ;
        char len ;
        sprintf(string, "%d", position) ;

        len = strlen(string) ;

        setType(buffer, PKT_VALUE_QUERY) ;
        setLength(buffer, 5+len) ;
        memcpy(buffer+4, &len, 1) ;
        memcpy(buffer+5, string, len) ;

        writen(sockfd, buffer, 5+len) ;
        readn(sockfd, buffer, 8) ;
        memcpy(&position, buffer+4, 4) ;
        position = ntohl(position) ;

        return position ;
   }

   int Values(int * position, int * values, int n)
   {
        char buffer[1024];
        short temp ;
        int i ;
        int cur_offset = 0 ;
        setType(buffer, PKT_VALUES_QUERY) ;
        temp = htons((short)n) ;
        memcpy(buffer+4, &temp, 2) ;
        cur_offset = 6 ;
        for(i = 0 ; i < n ; i++)
        {
                char the_position[256] ;
                char len ;
                sprintf(the_position, "%d", position[i]) ;
                len = strlen(the_position) ;
                memcpy(buffer+cur_offset, &len, 1) ;
                cur_offset++ ;
                memcpy(buffer+cur_offset, the_position, len) ;
                cur_offset += len ;
        }
        setLength(buffer, cur_offset) ;
        writen(sockfd, buffer, cur_offset) ;
        readn(sockfd, buffer, 6+4*n) ;
        cur_offset = 6 ;
        for(i = 0 ; i < n ; i++)
        {
                int value ;
                memcpy(&value, buffer+6+4*i, 4) ;
                value = ntohl(value) ;
                values[i] = value ;
        }
   }

   MOVELIST * GenerateMoves(POSITION position)
   {
        MOVELIST * head = NULL ;
        char buf[256];
        char string[256] ;
        char len ;
        char offset = 2 ;
        short n ;
        int i ;

        char input[5000] ;
        short input_length ;

        sprintf(string, "%d", position) ;
        len = strlen(string) ;

        memcpy(buf+4, &len, 1) ;
        memcpy(buf+5, string, len) ;
        setType(buf, PKT_GENERATE_MOVES_QUERY) ;
        setLength(buf, len+5) ;

        writen(sockfd, buf, len+5) ;

        readn(sockfd, input, 4) ;

        input_length = getLength(input) ;

        readn(sockfd, input, input_length - 4) ;

        n = getType(input) ;

        for(i = 0 ; i < n ; i++)
        {
                char move[256] ;
                char movelength ;
                int imove ;
                memcpy(&movelength, input+offset, 1) ;
                offset++ ;
                memcpy(move, input+offset, movelength) ;
                move[movelength] = (char)NULL ;
                offset += movelength ;
                sscanf(move, "%d", &imove) ;
                head = CreateMovelistNode(imove, head) ;
        }

        return head ;
   }

   POSITION DoMove(POSITION position, MOVE move)
   {
        char packet[256] ;
        char movestring[256] ;
        char movelength ;
        char positionstring[256] ;
        char positionlength ;
        short packetlength ;

        setType(packet, PKT_DO_MOVE_QUERY) ;

        sprintf(movestring, "%d", move) ;
        movelength = strlen(movestring) ;
        sprintf(positionstring, "%d", position) ;
        positionlength = strlen(positionstring) ;

        memcpy(packet+4, &positionlength, 1) ;
        memcpy(packet+5, positionstring, positionlength) ;
        memcpy(packet+5+positionlength, &movelength, 1) ;
        memcpy(packet+5+positionlength+1, movestring, movelength) ;

        setLength(packet, positionlength+movelength+6) ;

        writen(sockfd, packet, positionlength+movelength+6) ;

        readn(sockfd, packet, 4) ;
        packetlength = getLength(packet) ;
        readn(sockfd, packet, packetlength-4) ;
        memcpy(&positionlength, packet, 1) ;
        memcpy(positionstring, packet+1, positionlength) ;

        sscanf(positionstring, "%d", &position) ;

        return position ;
   }

   VALUE Primitive(POSITION position)
   {
        char packet[256] ;
        char packetlength ;
        char positionstring[256] ;
        char positionlength ;

        setType(packet, PKT_PRIMITIVE_QUERY) ;

        sprintf(positionstring, "%d", position) ;
        positionlength = strlen(positionstring) ;

        memcpy(packet+4, &positionlength, 1) ;
        memcpy(packet+5, positionstring, positionlength) ;

        setLength(packet, positionlength+5) ;

        writen(sockfd, packet, positionlength+5) ;

        readn(sockfd, packet, 5) ;

        memcpy(&positionlength, packet+4, 1) ;

        return (VALUE)positionlength ;
   }

   PlayAgainstHuman()
   {
   POSITION currentPosition;
   MOVE theMove;
   VALUE Primitive();
   UNDO *undo = NULL, *InitializeUndo(), *HandleUndoRequest(), *UpdateUndo();
   BOOLEAN playerOneTurn = TRUE, error, abort;
   USERINPUT userInput, GetAndPrintPlayersMove();
   char line[256] ;

   currentPosition = gInitialPosition;

   PrintPosition(currentPosition, gPlayerName[playerOneTurn], kHumansTurn);

   while(GPrimitive(currentPosition) == undecided)
   {
     gets(line);
     theMove = GTextToMove(
     //GetAndPrintPlayersMove(currentPosition,&theMove, gPlayerName[playerOneTurn]) ;

    if (!gGoAgain(currentPosition,theMove))
      playerOneTurn = !playerOneTurn;

    PrintPosition(currentPosition = DoMove(currentPosition,theMove),
                  gPlayerName[!playerOneTurn], kHumansTurn);
   }

   if(Primitive(currentPosition) == tie)
    printf("The match ends in a draw. Excellent strategies, %s and %s.\n\n",
           gPlayerName[0], gPlayerName[1]);
   else if(Primitive(currentPosition) == lose)
    printf("\n%s (player %s) Wins!\n\n", gPlayerName[!playerOneTurn],
           playerOneTurn ? "two" : "one");
   else if(Primitive(currentPosition) == win)
    printf("\n%s (player %s) Wins!\n\n", gPlayerName[playerOneTurn],
           playerOneTurn ? "one" : "two");
   else if(userInput == Abort || abort)
    printf("Your abort command has been received and successfully processed!\n");
   else
    BadElse("GPlayAgainstHuman");
   }

   PlayAgainstComputer()
   {
   POSITION thePosition;
   MOVE theMove, GetComputersMove();
   VALUE Primitive();
   UNDO *undo, *InitializeUndo(), *HandleUndoRequest(), *UpdateUndo();
   BOOLEAN usersTurn, error, abort;
   USERINPUT userInput, GetAndPrintPlayersMove();

   thePosition = gInitialPosition;
   undo = InitializeUndo();
   usersTurn = gHumanGoesFirst;

   printf("\nOk, %s and %s, let us begin.\n\n",
         gPlayerName[kPlayerOneTurn], gPlayerName[kPlayerTwoTurn]);

   #ifndef X
   printf("Type '?' if you need assistance...\n\n");
   #endif

   PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);

   while(Primitive(thePosition) == undecided) {

    if(usersTurn) {

      while((userInput = GetAndPrintPlayersMove(thePosition,
                                                &theMove,
                                                gPlayerName[usersTurn])) == Undo) {
        undo = HandleUndoRequest(&thePosition,undo,&error);
        PrintPosition(thePosition,gPlayerName[usersTurn],usersTurn);
      }

    }
    else {
        theMove = GetComputersMove(thePosition);
      PrintComputersMove(theMove,gPlayerName[usersTurn]);
    }
    if(userInput == Abort)
      break;

    if (!gGoAgain(thePosition,theMove))
      usersTurn = !usersTurn;

    PrintPosition(thePosition = DoMove(thePosition,theMove),
                  gPlayerName[usersTurn],usersTurn);

    undo = UpdateUndo(thePosition, undo, &abort);
    if(abort)
      break;

   }
   if((Primitive(thePosition) == lose && usersTurn) ||
     (Primitive(thePosition) == win && !usersTurn))
    printf("\n%s wins. Nice try, %s.\n\n", gPlayerName[kComputersTurn],
           gPlayerName[kHumansTurn]);
   else if((Primitive(thePosition) == lose && !usersTurn) ||
          (Primitive(thePosition) == win && usersTurn))
    printf("\nExcellent! You won!\n\n");
   else if(Primitive(thePosition) == tie)
    printf("The match ends in a draw. Excellent strategy, %s.\n\n",
           gPlayerName[kHumansTurn]);
   else if(userInput == Abort)
    printf("Your abort command has been received and successfully processed!\n");
   else
    BadElse("PlayAgainstHuman");

   ResetUndoList(undo);
   }

   MOVELIST * GenerateMovesWrapper(int thePosition)
   {
        if(gameline) return GGenerateMoves(thePosition) ;
        return GenerateMoves(thePosition) ;
   }

   MOVE GTextToMove(char * text)
   {
        char packet[256] ;
        char textlength ;

        setType(packet, PKT_TEXT_TO_MOVE_QUERY) ;
        setLength(packet, 5+strlen(text)) ;

        textlength = strlen(text) ;

        memcpy(packet+4, &positionlength, 1) ;

        memcpy(packet+5, text, textlength) ;

        writen(sockfd, packet, textlength+5) ;

        readn(sockfd, packet, 5) ;

        memcpy(&textlength, packet, 1) ;

        readn(sockfd, packet, textlength) ;
        packet[textlength] = '\0' ;

        sscanf(packet, "%d", &textlength) ;

        return (MOVE)textlength ;
   }
 */
