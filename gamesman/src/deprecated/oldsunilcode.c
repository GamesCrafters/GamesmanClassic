
//old huffman tree prototypes & defines

#define FREQTABLESIZE   256
#define HUFFTREESIZE    550
/* reserved value for parent, lChild, rChild fields */
#define NONE            -1
#define lessThan    -1
#define equalTo      0
#define greaterThan  1
#define mMax(A,B) ((A) > (B) ? (A) : (B))
#define mMin(A,B) ((A) > (B) ? (B) : (A))
#define MAGIC        0xC0FFEE
int  BuildFreqTable(char fileName[], unsigned int freqTable[FREQTABLESIZE]);
int  LocateTwoSmallestNodes(HuffTree *huffTree, int *first, int *second);
void BuildHuffmanTree(unsigned int freqTable[FREQTABLESIZE], HuffTree *huffTree);
int  CompressFile(char inFileName[], char outFileName[]);
int  UncompressFile(char inFileName[], char outFileName[]);

/*Sunil huffmantree functions.*/

int writeDatabase()
{
	POSITION i ;
	POSITION badcount ;
	FILE * filep ;
	STRING tempfilename = "gamesman.dump" ;
	char outfilename[256] ;

	mkdir("data", 0755) ;

	sprintf(outfilename, "./data/m%s_%d.dat", kDBName, getOption()) ;

	if((filep = fopen(tempfilename, "w")) == NULL) // ... if we are  unable to create a temporary file
	{
		printf("Unable to create temporary file %s", outfilename) ;
		return 0;
	}

	// start our compression algorithm
	for(i = 0, badcount = 0 ; i < gNumberOfPositions ; i++)
	{
		if(gDatabase[i] % 4 == 3)
			badcount++ ;
		else
		{
			if(badcount > 0)
			{
				badcount = 4*badcount + 3 ;
				fwrite(&badcount, sizeof(int), 1, filep) ;
				badcount = 0 ;
			}
			fwrite(gDatabase+i, sizeof(int), 1, filep) ;
		}
	}
	if(badcount > 0)
	{
		badcount = 4*badcount + 3 ;
		fwrite(&badcount, sizeof(int), 1, filep) ;
	}
	fwrite(&gNumberOfPositions, sizeof(int), 1, filep) ;

	fclose(filep) ;

	CompressFile(tempfilename, outfilename) ;
	remove(tempfilename) ;
	return 1;
}

int loadDatabase()
{
        POSITION i;
        int temp ;
	FILE * filep ;
	STRING tempfilename = "/tmp/gamesman/gamesman.dump" ;
	char outfilename[256] ;
	sprintf(outfilename, "./data/m%s_%d.dat", kDBName, getOption()) ;
	if((filep = fopen(outfilename, "r")) == NULL) return 0 ;
	fclose(filep) ;

	mkdir("/tmp/gamesman", 0755) ;

	UncompressFile(outfilename, tempfilename) ;

	if((filep = fopen(tempfilename, "r")) == NULL) return 0 ;

	i = 0 ;
	while(i < gNumberOfPositions && !feof(filep))
	{
		fread(&temp, sizeof(int), 1, filep) ;
		if(temp % 4 == 3)
			i += temp/4 ;
		else
			gDatabase[i++] = temp ;
	}

	fclose(filep) ;

	remove(tempfilename) ;

	return 1 ;
}

int BuildFreqTable(char fileName[], unsigned int freqTable[FREQTABLESIZE])
{
	FILE *fileHandle;
	long byteCount = 0;

	if((fileHandle = fopen(fileName, "r")) == NULL) return FALSE ;

	memset(freqTable, 0, FREQTABLESIZE*sizeof(unsigned int));

	while (!feof(fileHandle))
	{
		freqTable[fgetc(fileHandle)]++;
		++byteCount;
	}
	
	fclose(fileHandle);
	
	return byteCount-1;
}


int LocateTwoSmallestNodes(HuffTree *huffTree, int *first, int *second)
{
	int smallNodesFound = 0;
	int execFlag;
	int loopCounter;

	loopCounter = 0;
	execFlag = TRUE;

	while (execFlag == TRUE)
	{
		if (loopCounter == huffTree->nodeCount)
			execFlag = FALSE;

		if (  (huffTree->tree[loopCounter].parent == NONE) 
		   && (huffTree->tree[loopCounter].nodeUsed == TRUE) )
		{
			*first = loopCounter;
			execFlag = FALSE;
			smallNodesFound = 1;
		} 
		++loopCounter;
	}

	if (smallNodesFound == 0)
		return 0;

	execFlag = TRUE;

	while (execFlag == TRUE)
	{
		if (loopCounter == huffTree->nodeCount)
			execFlag = FALSE;

		if ((execFlag == TRUE)
		  && (huffTree->tree[loopCounter].parent == NONE)
		  && (huffTree->tree[loopCounter].nodeUsed == TRUE))
		{
			if ((smallNodesFound == 1) 
			  && (huffTree->tree[loopCounter].frequency
			   > huffTree->tree[*first].frequency))
			{
				*second = loopCounter;
				smallNodesFound = 2;

			}
			else if (huffTree->tree[loopCounter].frequency
			         <= huffTree->tree[*first].frequency)
			{
				*second = *first;
				*first = loopCounter;
				smallNodesFound = 2;

			}
			else if ((huffTree->tree[loopCounter].frequency
				 <= huffTree->tree[*second].frequency)
				 && (smallNodesFound == 2))
			{
				*second = loopCounter;
			}
		} 
		++loopCounter;
	}

	return smallNodesFound;
}


void BuildHuffmanTree(unsigned int freqTable[FREQTABLESIZE], HuffTree *huffTree)
{
	int loopCounter;
	int smallest, secondSmallest;
	int index;

	memset(huffTree, 0, sizeof(HuffTree));

	for (loopCounter = 0; loopCounter < FREQTABLESIZE; ++loopCounter)
	{
		if (freqTable[loopCounter])
		{
			huffTree->tree[huffTree->nodeCount].nodeUsed = TRUE;
			huffTree->tree[huffTree->nodeCount].parent = NONE;
			huffTree->tree[huffTree->nodeCount].lChild = NONE;
			huffTree->tree[huffTree->nodeCount].rChild = NONE;
			huffTree->tree[huffTree->nodeCount].character = loopCounter;
			huffTree->tree[huffTree->nodeCount].frequency =
				freqTable[loopCounter];
			huffTree->encoderLUT[loopCounter] = huffTree->nodeCount;
			++huffTree->nodeCount;
		}
	}

	while (LocateTwoSmallestNodes(huffTree, &smallest, &secondSmallest) == 2)
	{
		index = huffTree->nodeCount; /* pos. of new node */
		huffTree->tree[index].nodeUsed = TRUE;
		huffTree->tree[smallest].parent = index;
		huffTree->tree[secondSmallest].parent = index;
		huffTree->tree[index].lChild = smallest;
		huffTree->tree[index].rChild = secondSmallest;
		huffTree->tree[index].parent = NONE;
		huffTree->tree[index].frequency = 
			huffTree->tree[smallest].frequency
		      + huffTree->tree[secondSmallest].frequency;
		++huffTree->nodeCount;
	}

	huffTree->rootNode = smallest;
}


int CompressFile(char inFileName[], char outFileName[])
{
	FILE *inFileHandle;
	FILE *outFileHandle;

	unsigned int freqTable[FREQTABLESIZE];   /* the frequency data */
	HuffTree huffTree;                       /* Huffman tree */
	long bytesInInputFile;                   /* stored in output file */
	int bitPointer;        /* used to track when to write a byteful of data out */
	char byteToBeWritten = 0;
	int loopCounter;
	unsigned char charToEncode;   /* the character read in, to be encoded */
	int nodePointer;         /* Huffman tree pointer used in encoding chars */
	int parentNode;          /* the parent of the current node */
	char buffer[512];
	char bufferPointer;

	long magicNumber = MAGIC;  /* used to verify files are huff-compressed */


	bitPointer = 0;

	inFileHandle = fopen(inFileName, "r");
	if (inFileHandle == '\0') return FALSE;

	outFileHandle = fopen(outFileName, "w");
	if (outFileHandle == '\0') return FALSE;

	bytesInInputFile = BuildFreqTable(inFileName, freqTable);
	if (bytesInInputFile == 0) return FALSE;

	BuildHuffmanTree(freqTable, &huffTree);

	fwrite(&magicNumber, sizeof(long), 1, outFileHandle);          /* magic */
	fwrite(&bytesInInputFile, sizeof(long), 1, outFileHandle);     /* orig. file size */
	fwrite(freqTable, sizeof(int), FREQTABLESIZE, outFileHandle);  /* freq table */

	while (!feof(inFileHandle))
	{
		charToEncode = fgetc(inFileHandle);

		nodePointer = huffTree.encoderLUT[(int)charToEncode];

		bufferPointer = 0;    /* init buffer ptr */

		while (huffTree.tree[nodePointer].parent != NONE)
		{
			parentNode = huffTree.tree[nodePointer].parent;

			if (nodePointer == huffTree.tree[parentNode].lChild)
			{
				buffer[(int)bufferPointer++] = FALSE;
			}
			else
			{
				assert(nodePointer == huffTree.tree[parentNode].rChild);
				buffer[(int)bufferPointer++] = TRUE;
			}
			nodePointer = parentNode;  /* move up one node */
		}

		for (loopCounter = bufferPointer-1; loopCounter >= 0; --loopCounter)
		{
			byteToBeWritten = byteToBeWritten << 1;  /* bitwise shift left */

			if (buffer[loopCounter])
				byteToBeWritten = byteToBeWritten | 1;
			++bitPointer;
			if (bitPointer == 8)
			{
				fputc(byteToBeWritten, outFileHandle);
				byteToBeWritten = 0;
				bitPointer = 0;
			}
		}
	}

	fclose(inFileHandle);
	fclose(outFileHandle);

	return TRUE;
}

int UncompressFile(char inFileName[], char outFileName[])
{
	FILE *inFileHandle;
	FILE *outFileHandle;

	HuffTree huffTree;                      /* Huffman tree */
	unsigned int freqTable[FREQTABLESIZE];  /* freq table */
	long bytesInOutputFile;     /* loaded in from input file */
	int bitPointer;   /* used to track when to read in the next byte */
	char outChar;
	char charToDecode;    /* the character to be decoded and written out */
	int nodePointer;      /* Huffman tree pointer used in decoding chars */
	long magicNumber;    /* used to verify input file is huff-compressed */

	inFileHandle = fopen(inFileName, "r");
	if (inFileHandle == '\0') return FALSE;

	outFileHandle = fopen(outFileName, "w");
	if (outFileHandle == '\0') return FALSE;

	fread(&magicNumber, sizeof(long), 1, inFileHandle);
	if (magicNumber != MAGIC) return FALSE;

	fread(&bytesInOutputFile, sizeof(long), 1, inFileHandle); /* size of output file */

	fread(freqTable, sizeof(int), FREQTABLESIZE, inFileHandle);

	BuildHuffmanTree(freqTable, &huffTree);

	charToDecode = fgetc(inFileHandle);
	bitPointer = 0;

	while ((!feof(inFileHandle)) && (bytesInOutputFile > 0))
	{
		nodePointer = huffTree.rootNode;

		while ((huffTree.tree[nodePointer].lChild != NONE) ||
		       (huffTree.tree[nodePointer].rChild != NONE))
		{
			if (charToDecode & 128)
			{
				nodePointer = huffTree.tree[nodePointer].rChild;
			}
			else
			{
				nodePointer = huffTree.tree[nodePointer].lChild;
			}

			charToDecode <<= 1;   /* shift, to get the next bit */
			++bitPointer;

			/* have we run out of bits in this byte?  Get another byte */
			if (bitPointer == 8)
			{
				bitPointer = 0;
				charToDecode = fgetc(inFileHandle);
			}

		}
		outChar = huffTree.tree[nodePointer].character;
		fputc(outChar, outFileHandle);
		--bytesInOutputFile;
	}

	fclose(inFileHandle);
	fclose(outFileHandle);

	return TRUE;
}






/* sunil server/client internal function prototypes */
int     getLength(void * line);
void    setLength(void * line, short length);
int     stuff(char * source, char * dest);
int     InitClient();
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readn(int fd, void *vptr, size_t n);


//// Gameline Information
int gameline = 0 ;
int sockfd = -1 ;

// Start Gameline functions

////// SUNIL's CLIENT/SERVER CODE
#define MAXLINE 4096

#define max(a,b) (((a)>(b))?(a):(b))

int getLength(void * line)
{
	short ret ;
	memcpy(&ret, line, 2) ;
	ret = ntohs(ret) ;
	return (int)ret ;
}

void setLength(void * line, short length)
{
	length = htons(length) ;
	memcpy(line, &length, 2) ;
}

int stuff(char * source, char * dest)
{
	int a = strlen(source) ;
	memcpy(dest+2, source, a) ;
	setLength(dest, a+2) ;
	return a+2 ;
}

int InitClient()
{
	struct sockaddr_in servaddr ;
	struct hostent * he ;
	char hostname[256] ;
	short port ;
	int iport ;
	FILE * filep ;

	if(!(filep = fopen(hostname, "r")))
		printf("Unable to open server.txt. Please reinstall Gameline\n"), exit(1) ;

	fgets(hostname, 254, filep) ;
	if(hostname[strlen(hostname)-1] == (char)NULL)
		hostname[strlen(hostname)-1] = (char)NULL ;
	fscanf(filep, "%d", &iport) ;
	port = (short)iport ;

	fclose(filep) ;

	if((he = gethostbyname(hostname)) == NULL)
	{
		printf("sunil: gethostbyname failed\n") ;
		return 0 ;
	}
	if(port <= 0)
	{
		printf("sunil: port <= 0\n") ;
		return 0 ;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0) ;

	bzero(&servaddr, sizeof(servaddr)) ;
	servaddr.sin_family = AF_INET ;
	servaddr.sin_port = htons(port) ;
	servaddr.sin_addr = *((struct in_addr *)he->h_addr) ;
	memset(&(servaddr.sin_zero), '\0', 8) ;  // zero the rest of the structure

	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		printf("gameline: connect failed\n") ;
		return 0 ;
	}
	return 1;
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		}
		else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}


//Random Stuff removed from code to clean it up (functions included for refrence).

VALUE GetValueOfPosition(position)
     POSITION position;
{
  if(gameline)
  {
	char buffer[256] ;
	char userinput[256] ;
	int a ;

	if(sockfd < 0)
		InitClient() ;

	sprintf(userinput, "%s -RawRead %d " POSITION_FORMAT, kDBName, getOption(), position) ;
	a = stuff(userinput, buffer) ;
	writen(sockfd, buffer, a) ;
	readn(sockfd, buffer, 2) ;
	a = getLength(buffer) ;
	a -= 2 ;
	readn(sockfd, buffer, a) ;
	buffer[a] = (char)NULL ;
	//printf("'%s'\n", buffer) ;
	sscanf(buffer, POSITION_FORMAT, &position) ;
	return position % 4;
  }
  else
  {
	VALUE *GetRawValueFromDatabase(), *ptr;
	ptr = GetRawValueFromDatabase(position);
	return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
  }
}

REMOTENESS Remoteness(position)
     POSITION position;
{
  if(gameline)
  {
	char buffer[256] ;
	char userinput[256] ;
	int a ;

	if(sockfd < 0)
		InitClient() ;

	sprintf(userinput, "%s -RawRead %d " POSITION_FORMAT, kDBName, getOption(), position) ;
	a = stuff(userinput, buffer) ;
	writen(sockfd, buffer, a) ;
	readn(sockfd, buffer, 2) ;
	a = getLength(buffer) ;
	a -= 2 ;
	readn(sockfd, buffer, a) ;
	buffer[a] = (char)NULL ;
	//printf("'%s'\n", buffer) ;
	sscanf(buffer, POSITION_FORMAT, &position) ;
	return ( (position & REMOTENESS_MASK) >> REMOTENESS_SHIFT );
  }
  else
  {
	VALUE *GetRawValueFromDatabase(), *ptr;
	ptr = GetRawValueFromDatabase(position);
	return((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
  }
}

MEX MexLoad(position)
POSITION position;
{
  if(gameline)
  {
	char buffer[256] ;
	char userinput[256] ;
	int a ;

	if(sockfd < 0)
		InitClient() ;

	sprintf(userinput, "%s -RawRead %d " POSITION_FORMAT, kDBName, getOption(), position) ;
	a = stuff(userinput, buffer) ;
	writen(sockfd, buffer, a) ;
	readn(sockfd, buffer, 2) ;
	a = getLength(buffer) ;
	a -= 2 ;
	readn(sockfd, buffer, a) ;
	buffer[a] = (char)NULL ;
	//printf("'%s'\n", buffer) ;
	sscanf(buffer, POSITION_FORMAT, &position) ;
	return((position /8)%32);
  }
  else
  {
	return (gDatabase[position]/8) % 32 ;
  }
}