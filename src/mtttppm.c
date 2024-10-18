/************************************************************************
**
** NAME:        mtttppm.c
**
** DESCRIPTION: Tic-Tac-Toe PPM file output subroutines
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1999. All rights reserved.
**
** DATE:        1999-02-03
**
** UPDATE HIST:
**
**
** Last Updated: $Id: mtttppm.c,v 1.6 2006-12-19 20:00:51 arabani Exp $
**************************************************************************/

/*************************************************************************
**
** These are just local variables to set up the PIXEL array
**
**************************************************************************/

#include "gamesman.h"
#include <stdio.h>


extern POSITION gInitialPosition;  /* The initial position of the game */

int gR[] = {   0, 139, 255, 0 };   /* Change the last number to 255 - white */
int gG[] = { 255,   0, 255, 0 };
int gB[] = {   0,   0,   0, 0 };

char *gPScmd[] = { "W", "L", "T", "U" }; /* String of commands to PS */

typedef enum color_enum {
	c_win, c_lose, c_tie, c_invalid
} COLOR;

typedef struct pixelbuffer_struct {
	char *bufptr;
	int x,y;
} PIXELBUFFER;

/* S(0) = 1; S(n) = 3 * S(n-1) + 2; */
/////int gS[] = {  1, 5, 17, 53, 161, 485, 1457, 4373, 13121, 39365 };/////////

/* S(0) = 1; S(1) = 5; S(n) = 3 * S(n-1) + (2/3)(S(n-1)) */
int gS[] = {  1, 5, 17, 63, 231, 847, 3106, 11389, 41760, 153120 };
int gX[] = {  -1, 0, 1, -1, 0, 1, -1, 0, 1 };
int gY[] = {  -1, -1, -1, 0, 0, 0, 1, 1, 1 };


/***** Prototypes *****/
PIXELBUFFER *CreatePixelBuffer();
PIXELBUFFER *CreatePixelBufferAtDepth();
COLOR GetBuffer();

void tttppm(int,int);
void WritePreamblePS(FILE *,int,int);
void tttppmrecur(FILE *,PIXELBUFFER *,int,int,POSITION,int,int,int);
void DrawColor(FILE *,PIXELBUFFER *,int,int,int,int,POSITION,int);

COLOR SwapWinLoseColor(COLOR,int);
int GetOffset(int,int);

void PrintPPMBufferbinary(FILE *,PIXELBUFFER *);
void PrintPPMBufferAscii(FILE *,PIXELBUFFER *);
void PrintBuffer(PIXELBUFFER *);
void WriteBuffer(PIXELBUFFER *,int,int,COLOR);
/***** End Prototypes *****/


/************************************************************************
**
** NAME:        tttppm
**
** DESCRIPTION: Start the PPM process, build a buffer, fill it, print it
**
** INPUTS:      int toPS, toFile : Whether to write to (PPM,PS) and
**                               : whether to write to (screen,file)
**
** CALLS:       CreatePixelBufferAtDepth()
**              tttppmrecur()
**              PrintPPMBufferBinary()
**
************************************************************************/

void tttppm(toPS,toFile)
int toPS, toFile;
{
	FILE *fp;
	char filename[80];
	PIXELBUFFER *pixbuf;
	int cx, cy, depth, scale;

	printf("%s Creation time!\n\n",toPS ? "Postscript" : "PPM");
	printf("Depth : ");
	scanf("%d",&depth);

	if(toPS) {
		printf("Scale : ");
		scanf("%d",&scale);
	}

	if(toFile) {
		printf("File  : ");
		scanf("%s",filename);

		if((fp = fopen(filename, "w")) == NULL)
			ExitStageRightErrorString("Couldn't open file, sorry.");
	} else
		fp = stdout;

	if(!toPS)
		printf("Creating/Initializing buffer...\n");

	pixbuf = toPS ? (PIXELBUFFER *)NULL : CreatePixelBufferAtDepth(depth);
	cx = cy = GetOffset(depth,0);

	printf("Filling %s...\n",toPS ? (toFile ? "file" : "screen") : "buffer");

	if(toPS && toFile) {
		WritePreamblePS(fp,scale,depth);

	}

	tttppmrecur(fp,pixbuf,cx,cy,gInitialPosition,depth,0,toPS);

	if(!toPS)
		printf("Dumping buffer to file...\n");

	if(!toPS) {
		if(toFile)
			PrintPPMBufferAscii(fp,pixbuf);
		// PrintPPMBufferAscii(fp,pixbuf,gS[depth],gS[depth]);
		// This was an incorrect call... -Elmer
		/* PrintPPMBufferBinary(fp,pixbuf,gS[depth],gS[depth]); */
		else
			PrintBuffer(pixbuf);
	} else
	if(toFile)
		fprintf(fp,"showpage\n");

	if(toPS && toFile) {
		fclose(fp);
		int z;
		int x;
		const char *myFirst = "pstoedit -f plot-svg ";
		char command[256];
		int k;
		for (k =0; k<20; k++) {
			command[k] = myFirst[k];
		}
		for (k = 21; k<101; k++) {
			command[20] = " ";
			command[k] = (char)filename[k-21];
		}
		for (k = 102; k<182; k++) {
			command[101] = " ";
			command[k+1] = (char)filename[k-102];
		}
		command[185] = ".";
		command[186] = "s";
		command[187] = "v";
		command[188] = "g";
		printf(command);
		system(command);
	}

	printf("done...\n");

	if (!toPS)
		SafeFree(pixbuf);

	if(toFile && !toPS)
		fclose(fp);
}

/************************************************************************
**
** NAME:        WritePreamblePS
**
** DESCRIPTION: Write the Postscript header to the file fp
**
** INPUTS:      FILE *fp            : File pointer to write PPP to
**              int depth           : Depth requested from user
**              int scale           : Scale requested from user
**
************************************************************************/

void WritePreamblePS(fp,scale,depth)
FILE *fp;
int scale,depth;
{
	fprintf(fp,"%%!-Adobe-1.0\n\
\n\
/pagewidthinches 8.5 def\n\
/pageheightinches 11 def\n\
/dpi 72 def\n\
pagewidthinches dpi mul 2 div pageheightinches dpi mul 2 div translate\n\
\n\
%d dup scale\n\
\n\
0.5 dup translate\n\
%d -2 div dup translate\n\
\n\
/d 1 def\n\
d setlinewidth\n\
/h d 2 div def\n\
clear\n\
\n\
%% Winning, Tieing, Losing moves\n\
/L { .53 0 0 setrgbcolor } def\n\
/T { 1 1 0 setrgbcolor } def\n\
/W { 0 1 0 setrgbcolor } def\n\
/U { 0 0 0 setrgbcolor } def\n\
\n\
/B { %% (s x y) on stack to moveto, B=box\n\
newpath moveto\n\
/t exch 1 sub def\n\
t 0 rlineto 0 t  rlineto t -1 mul 0 rlineto\n\
closepath stroke clear\n\
} def\n\
\n\
/F { %% (s x y) on stack F = fill\n\
newpath h sub exch h sub exch moveto\n\
/t exch def\n\
t 0 rlineto 0 t rlineto t  -1 mul 0 rlineto\n\
closepath fill clear\n\
} def\n\
\n\
/P { %% (x y) on stack to moveto P = point\n\
newpath moveto\n\
h -1 mul 0 rmoveto \n\
d 0 rlineto\n\
closepath stroke clear\n\
} def\n\
\n\
U %d 0 0 F\n\
",scale,gS[depth],gS[depth]);
}


/************************************************************************
**
** NAME:        tttppmrecur
**
** DESCRIPTION: Recursively go down the game tree and modify the buffer
**
** INPUTS:      FILE *fp            : File pointer to write PPP to
**              PIXELBUFFER *pixbuf : Pixel buffer
**              int          cx, cy : Center in the image of new piece
**              POSITION        pos : The position to write
**              int depth           : Depth requested from user
**              int topdowndepth    : Depth we're at now
**              int toPS            : Are we writing to a PS file?
**
** CALLS:       DrawColor(), GenerateMoves(), DoMove(), tttppmrecur()
**              FreeMoveList()
**
************************************************************************/

void tttppmrecur(fp,pixbuf,cx,cy,pos,depth,topdowndepth,toPS)
FILE *fp;
PIXELBUFFER *pixbuf;
int cx,cy,depth,topdowndepth,toPS;
POSITION pos;
{
	MOVELIST *ptr, *head, *GenerateMoves();
	POSITION theChild, DoMove();
	MOVE theMove;

	/* Draw this level */
	DrawColor(fp,pixbuf,cx,cy,depth,topdowndepth,pos,toPS);

	/* If we're not at the bottom level */
	if (depth != topdowndepth) {
		head = ptr = GenerateMoves(pos);
		while(ptr != NULL) {
			theMove = ptr->move;
			theChild = DoMove(pos,theMove);

			/* Recursively call ourselves */
			tttppmrecur(fp,
			            pixbuf,
			            cx+gX[(int)theMove]*gS[depth-topdowndepth-1],
			            cy+gY[(int)theMove]*gS[depth-topdowndepth-1],
			            theChild,
			            depth,
			            topdowndepth+1,
			            toPS);
			ptr = ptr->next;
		}
		FreeMoveList(head);
	}
}

/************************************************************************
**
** NAME:        DrawColor
**
** DESCRIPTION: Draw the color of the pixel into the buffer
**
** INPUTS:      FILE *fp            : File pointer to write PPP to
**              PIXELBUFFER *pixbuf : Pixel buffer
**              int          cx, cy : Center in the image of new piece
**              POSITION        pos : The position to write
**              int depth           : Depth requested from user
**              int topdowndepth    : Depth we're at now
**              int toPS            : Are we writing to a PS file?
**
** CALLS:       GetValueOfPosition(), SwapWinLoseColor(), WriteBuffer()
**
************************************************************************/

void DrawColor(fp,pixbuf,cx,cy,depth,topdowndepth,pos,toPS)
FILE *fp;
PIXELBUFFER *pixbuf;
int cx,cy,depth,topdowndepth,toPS;
POSITION pos;
{
	VALUE Primitive(), GetValueOfPosition();
	int i,j,edge,halfedge,primitivep;
	COLOR SwapWinLoseColor(), color;

	color = SwapWinLoseColor((COLOR)GetValueOfPosition(pos),topdowndepth);
	primitivep = (Primitive(pos) != undecided);
	edge = gS[depth-topdowndepth];
	halfedge = (edge-1)/2;

	if(depth == topdowndepth) /* lowest depth, single pixel */
		if(toPS)
			fprintf(fp,"%s %d %d P\n",gPScmd[(int)color],cx,cy);
		else
			WriteBuffer(pixbuf,cx,cy,color);
	else if (primitivep) /* Primitive, fill in w/single color */
		if(toPS)
			fprintf(fp,"%s %d %d %d F\n",gPScmd[(int)color],edge,cx-halfedge,cy-halfedge);
		else
			for(i=0; i<edge; i++)
				for(j=0; j<edge; j++)
					WriteBuffer(pixbuf,cx-halfedge+i,cy-halfedge+j,color);
	else /* Not primitive and not the bottom of tree yet */
	if(toPS)
		fprintf(fp,"%s %d %d %d B\n",gPScmd[(int)color],edge,cx-halfedge,cy-halfedge);
	else
		for(i=0; i<edge; i++) {
			for(j=0; j<edge/17; j++) {
			WriteBuffer(pixbuf,cx-halfedge+i,cy-halfedge+j,color);
			WriteBuffer(pixbuf,cx-halfedge+j,cy-halfedge+i,color);
			WriteBuffer(pixbuf,cx-halfedge+i,cy+halfedge-j,color);
			WriteBuffer(pixbuf,cx+halfedge-j,cy-halfedge+i,color);
			}
 }
}

/************************************************************************
**
** NAME:        SwapWinLoseColor
**
** DESCRIPTION: Swap the value of the array for all positions
**              w <-> l because we want the user to see the values he
**              sees in GAMESMAN.
**
** INPUTS:      COLOR color         : Color of the position
**              int topdowndepth    : Depth we're at now
**
** OUTPUTS:     COLOR : The updated color after the swap
**
************************************************************************/

COLOR SwapWinLoseColor(color,topdowndepth)
COLOR color;
int topdowndepth;
{
	if(topdowndepth != 0) { /* Don't swap colors for the first position */
		if(color == c_lose)
			return(c_win);
		else if(color == c_win)
			return(c_lose);
		else
			return(color);
	}
	return(c_invalid);
}

/************************************************************************
**
** NAME:        GetOffset
**
** DESCRIPTION: Get the offset into the center of the array
**
** INPUTS:      int depth           : Depth requested from user
**              int topdowndepth    : Depth we're at now
**
************************************************************************/

int GetOffset(depth,topdowndepth)
int depth,topdowndepth;
{
	return((gS[depth - topdowndepth]-1)/2);
}

/************************************************************************
**
** NAME:        PrintPPMBufferBinary
**
** DESCRIPTION: Write Buffer to file in PPM Binary format
**
** INPUTS:      FILE *fp            : File pointer to write PPP to
**              PIXELBUFFER *pixbuf : Buffer to dump to file
**
** CALLS:       GetBuffer()
**
************************************************************************/

void PrintPPMBufferBinary(fp,pixbuf)
FILE *fp;
PIXELBUFFER *pixbuf;
{
	int i,j,ci;

	fprintf(fp,"P3\n#foo.ppm\n%d %d\n255\n",pixbuf->x,pixbuf->y);

	for(j=0; j<pixbuf->y; j++) {
		for(i=0; i<pixbuf->x; i++) {
			ci = (int)GetBuffer(pixbuf,i,j);
			fprintf(fp,"%c%c%c",(char)gR[ci],(char)gG[ci],(char)gB[ci]);
		}
	}
}

/************************************************************************
**
** NAME:        PrintPPMBufferAscii
**
** DESCRIPTION: Write Buffer to file in PPM Ascii format
**
** INPUTS:      FILE *fp            : File pointer to write PPP to
**              PIXELBUFFER *pixbuf : Buffer to dump to file
**
** CALLS:       GetBuffer()
**
************************************************************************/

void PrintPPMBufferAscii(fp,pixbuf)
FILE *fp;
PIXELBUFFER *pixbuf;
{
	int i,j,ci;

	fprintf(fp,"P3\n#foo.ppm\n%d %d\n255\n",pixbuf->x,pixbuf->y);

	for(j=0; j<pixbuf->y; j++) {
		for(i=0; i<pixbuf->x; i++) {
			ci = (int)GetBuffer(pixbuf,i,j);
			fprintf(fp,"%3d %3d %3d  ",gR[ci],gG[ci],gB[ci]);
		}
		fprintf(fp,"\n");
	}
}

/************************************************************************
**
** NAME:        PrintBuffer
**
** DESCRIPTION: Write Buffer out to stdout
**
** INPUTS:      PIXELBUFFER *pixbuf : Buffer to dump to file
**
** CALLS:       GetBuffer()
**
************************************************************************/

void PrintBuffer(pixbuf)
PIXELBUFFER *pixbuf;
{
	int i,j;

	for(j=0; j<pixbuf->y; j++) {
		for(i=0; i<pixbuf->x; i++)
			printf("%d",(int)GetBuffer(pixbuf,i,j));
		printf("\n");
	}
}

/************************************************************************
**
** NAME:        CreatePixelBufferAtDepth
**
** DESCRIPTION: Build buffer at a size appropriate for the depth
**
** INPUTS:      int depth : Depth user has requested
**
** OUTPUTS:     PIXELBUFFER *pixbuf : Pixel Buffer
**
************************************************************************/

PIXELBUFFER *CreatePixelBufferAtDepth(depth)
int depth;
{
	return(CreatePixelBuffer(gS[depth],gS[depth]));
}

/************************************************************************
**
** NAME:        CreatePixelBuffer
**
** DESCRIPTION: Build buffer of size x,y
**              initialize it with the invalid color
**
** INPUTS:      int x,y : Size user has requested
**
** OUTPUTS:     PIXELBUFFER *pixbuf : Pixel Buffer
**
************************************************************************/

PIXELBUFFER *CreatePixelBuffer(x,y)
int x,y;
{
  #ifndef MEMWATCH
	GENERIC_PTR SafeMalloc();
  #endif
	PIXELBUFFER *pixbuf;
	int i,j;

	pixbuf = (PIXELBUFFER *) SafeMalloc (sizeof(PIXELBUFFER));
	pixbuf->bufptr = (char *) SafeMalloc (sizeof(char) * x * y);
	pixbuf->x = x;
	pixbuf->y = y;

	for(j=0; j<y; j++)
		for(i=0; i<x; i++)
			WriteBuffer(pixbuf,i,j,c_invalid);

	return(pixbuf);
}

/************************************************************************
**
** NAME:        WriteBuffer
**
** DESCRIPTION: Write the pixel into the (i,j) position in the buffer
**
** INPUTS:      PIXELBUFFER *pixbuf : Pixel buffer
**              int          i,j    : The pixel to write
**              COLOR        color  : The color to write
**
** CALLS:       ExitStageRightErrorString()
**
************************************************************************/

void WriteBuffer(pixbuf,i,j,color)
PIXELBUFFER *pixbuf;
int i,j;
COLOR color;
{
	if(i >= pixbuf->x)
		ExitStageRightErrorString("index i greater than max width in WriteBuffer");

	if(j >= pixbuf->y)
		ExitStageRightErrorString("index j greater than max height in WriteBuffer");

	*(pixbuf->bufptr + j*pixbuf->x + i) = (char) color;
}

/************************************************************************
**
** NAME:        GetBuffer
**
** DESCRIPTION: Get the color at the (i,j) position in pixbuf
**
** INPUTS:      PIXELBUFFER *pixbuf : Pixel buffer
**              int          i,j    : The pixel to write
**
** OUTPUTS:     COLOR : The color at the (i,j) coordinate in the buffer
**
************************************************************************/

COLOR GetBuffer(pixbuf,i,j)
PIXELBUFFER *pixbuf;
int i,j;
{
	if(i >= pixbuf->x)
		ExitStageRightErrorString("index i greater than max width in WriteBuffer");

	if(j >= pixbuf->y)
		ExitStageRightErrorString("index j greater than max height in WriteBuffer");

	return((COLOR) *(pixbuf->bufptr + j*pixbuf->x + i));
}

/* Changelog:
   $Log: not supported by cvs2svn $
   Revision 1.5  2006/02/03 06:08:39  hevanm
   fixed warnings. I will leave the real bugs to retro hehehehe.

   Revision 1.4  2005/05/04 22:54:15  ogren
   prototyped void functions properly.  Fixed a call to PrintPPMBufferAscii(fp,pixbuf) -Elmer

   Revision 1.3  2004/12/01 22:31:29  ogren
   added prototypes for COLOR SwapWinLoseColor, int GetOffset. Must still decide between typing file/buffer access functions as void or prototyping them as int. -Elmer

 */
