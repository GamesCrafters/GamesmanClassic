/************************************************************************
**
** NAME:    levelfile_generator.c
**
** DESCRIPTION:    Level File Generation and Access Functions
**
** AUTHOR:    Deepa Mahajan
**        GamesCrafters Research Group, UC Berkeley
**        Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:    2006-07-20
**
** LICENSE:    This file is part of GAMESMAN,
**        The Finite, Two-person Perfect-Information Game Generator
**        Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/
#include "levelfile_generator.h"

// GLOBAL VARIABLES
gzFile*        compressed_filep;
gzFile*        compressed_filep_type0;
gzFile*        compressed_filep_type1;
gzFile*        compressed_filep_type2;
gzFile*        compressed_filep_type3;
char           filename_type0[82];
char           filename_type1[82];
char           filename_type2[82];
char           filename_type3[82];

/****************************************************************************
* Description                                                               *
*      WriteLevelFile reads an array of hash values and returns a           *
*      level file. This file may be of any format type. The function        *
*      optimizes for size by generating all types sequentially and          *
*      comparing size of file after each file is generated. If the endIndex *
*      is greater than the startIndex then the function knows that the      *
*      array has been corrupted at some other level                         *
* Arguments                                                                 *
*      char* compressed_filename                                            *
*      BITARRAY *array                                                      *
*      POSITION startIndex                                                  *
*      POSITION endIndex                                                    *
* Return Values                                                             *
*      int 0 for success -1 for error                                       *
****************************************************************************/
int WriteLevelFile(char* compressed_filename, BITARRAY* array,
 POSITION startIndex, POSITION endIndex)
{
    UINT64 minHashValue, maxHashValue;
    minHashValue = startIndex + findMinValueFromArray(array, endIndex-startIndex);
    maxHashValue = startIndex + findMaxValueFromArray(array, endIndex-startIndex);
    int type = 0;
    
 	if(endIndex < startIndex)
	{
         return -1;
	}
	
 	UINT8 bitsPerPosition = log2(maxHashValue - minHashValue) + 1; 
// 	UINT64 length = ceil((maxHashValue - minHashValue)/BITSINBYTE);
 	
	while(type < NUMOFTYPES)// create the four file types
	{
         if(type == 0)
         {
          strncpy(filename_type0, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
          strcat(filename_type0, "_0.dat.gz");
          printf(" Compressed_filename is:  %s \n", filename_type0);
          compressed_filep_type0 = gzopen(filename_type0, "wb");
          if(!compressed_filep_type0)
          {
               printf("couldn't open file");
               return -1;
          }
          writeHeader(compressed_filep_type0, minHashValue, maxHashValue, 0, type);
          printf("Header is Written to File\n");
          ArrayToType0Write(array, startIndex, maxHashValue);

          printf("Body is Written to File\n");
          gzclose(compressed_filep_type0);
         }

         if(type == 1)
         {
          strncpy(filename_type1, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
          strcat(filename_type1, "_1.dat.gz");
          printf("1: Compressed_filename is:  %s \n", filename_type1);
          compressed_filep_type1 = gzopen(filename_type1, "wb");
          if(!compressed_filep_type1)
          {
               printf("1: couldn't open file");
               return -1;
          }
          writeHeader(compressed_filep_type1, minHashValue, maxHashValue, bitsPerPosition, type);
          printf("1: Header is Written to File\n");
          ArrayToType1Write(array, startIndex, maxHashValue, bitsPerPosition, minHashValue);
          printf("1: Body is Written to File\n");
          gzclose(compressed_filep_type1);
         }
         
          if(type == 2)
          {
               strncpy(filename_type2, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
               strcat(filename_type2, "_2.dat.gz");

               printf("2: Compressed_filename is:  %s \n", filename_type2);
               compressed_filep_type2 = gzopen(filename_type2, "wb");
               if(!compressed_filep_type2)
               {
                    printf("2: couldn't open file");
                    return -1;
               }
               writeHeader(compressed_filep_type2, minHashValue, maxHashValue, bitsPerPosition, type);
               printf("2: Header is Written to File\n");
               ArrayToType2Write(array, startIndex, maxHashValue, bitsPerPosition, minHashValue);

               printf("2: Body is Written to File\n");
               gzclose(compressed_filep_type2);
          }
         
          if(type == 3)
          {
               strncpy(filename_type3, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
               strcat(filename_type3, "_3.dat.gz");
               printf("3: Compressed_filename is:  %s \n", filename_type3);
               compressed_filep_type3 = gzopen(filename_type3, "wb");
               if(!compressed_filep_type3)
               {
                    printf("3: couldn't open file");
                    return -1;
               }
               writeHeader(compressed_filep_type3, minHashValue, maxHashValue, 0, type);
               printf("3: Header is Written to File\n");
 		      
               ArrayToType3Write(array, minHashValue, maxHashValue);
               printf("3: Body is Written to File\n");
              
               gzclose(compressed_filep_type3);
          }
         
          type++;
     }
     return 0;
}

/****************************************************************************
* Description                                                               *
*      writeHeader writes the header information and header comments        *
*      to the file                                                          *
* Arguments                                                                 *
*      gzFile *file                                                         *
*      UINT64 minHashValue                                                  *
*      UINT64 maxHashValue                                                  *
*      UINT8 bitsPerPosition                                                *
*      int type                                                             *
* Return Values                                                             *
*      0 upon success or 1 upon error                                       *
****************************************************************************/
int writeHeader(gzFile *file, UINT64 minHashValue, UINT64 maxHashValue, UINT8 bitsPerPosition, int type)
{
     int goodCompression;
     char* check = "1\n";
     char comment1[80];
     int status;
     char* comment2 = "### Doc on level file formats in doc/LevelFiles.txt\n";
     char* typeH = (char*)malloc((1+log(UINT8_MAX))+ sizeof(char));
     char* header = (char*)malloc((log(UINT64_MAX)+1)*3 + sizeof(UINT8) + 3*sizeof(char));

     sprintf(comment1,"### GAMESMAN Bagh Chal Type%d Level Files\n", type);
     sprintf(typeH, "%d\n", type);
     if(bitsPerPosition == 0)
     {
          sprintf(header, "%llu %llu %c", minHashValue, maxHashValue, 0xC);
     }
     else
     {
          sprintf(header, "%llu %llu %d %c", minHashValue, maxHashValue, bitsPerPosition, 0xC);
     }

     status = bitlib_file_write_bytes(file, check, strlen(check));
     status = bitlib_file_write_bytes(file, comment1, strlen(comment1));
     status = bitlib_file_write_bytes(file, comment2, strlen(comment2));
     status = bitlib_file_write_bytes(file, typeH, strlen(typeH));
     status = bitlib_file_write_bytes(file, header, strlen(header));
     if(!status)
     {
          return 0;
     }
     return -1;
}

/****************************************************************************
* Description                                                               *
*      ArrayToType0Write reads in an array of BYTE values and               *
*      creates a Type 0 level File.                                         *
* Arguments                                                                 *
*      BITARRAY *array                                                      *
*      UINT64 startIndex                                                    * 
*      UINT64 maxHashValue                                                  * 
* Return Values                                                             *
*      0 upon success or 1 upon error                                       *
****************************************************************************/
int ArrayToType0Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue)
{
     POSITION counter = startIndex;
     UINT8 bitcounter = 0, currentByte;
     int success;
     BYTE* buffer = (BYTE*)malloc(sizeof(UINT64));
     GMSTATUS status = STATUS_SUCCESS;
     char positionString[22];

     while(counter <= maxHashValue)
     {
          bitcounter=0;
          currentByte = *array;
          while(counter <= maxHashValue && bitcounter < BITSINBYTE)
          {
               if(getBitValue(currentByte, bitcounter))
               {
                    sprintf(positionString, "%llu ", counter);
                    status = bitlib_file_write_bytes(compressed_filep_type0, positionString, strlen(positionString));
               }
               bitcounter++;
               counter++;
          }
          array++;
     }
     return 0;
}

/****************************************************************************
* Description                                                               *
*      ArrayToType1Write reads in an array of BYTE values and               *
*      creates a Type 0 level File.                                         *
* Arguments                                                                 *
*      BYTE *array                                                          *
*      UINT64 startIndex                                                    * 
*      UINT64 maxHashValue                                                  * 
*      UINT8 bitsPerPosition                                                * 
*      UINT64 offset                                                        * 
* Return Values                                                             *
*      0 upon success or 1 upon error                                       *
****************************************************************************/
int ArrayToType1Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue, UINT8 bitsPerPosition, UINT64 offset)
{
     POSITION counter = startIndex;
     UINT8 bitcounter = 0, currentByte;
     int success;
     BYTE bufferPrev = 0;
     BYTE* bufferCurr = (BYTE*)malloc(sizeof(UINT64));

     GMSTATUS status = STATUS_SUCCESS;
     UINT8 leftOffset=0, rightOffset=0;
 	
 	/*
 	take the first one, bitshift by bpp. Then print the floor(first bpp/8) Bytes. 
	 set leftOffset to 8-(bpp%8) and take the next Byte and set to buffer Prev
	take 2nd value, shift left bpp - leftOffset
	 or byte 1 with buffer Prev, Print all bytes up to floor(bitsPerPosition+leftOffset)/8
	 set leftOffset to 8-(bpp+leftOffset)%8 set bufferPrev to the next Byte 
	*/  

     while(counter <= maxHashValue)
     {
          bitcounter=0;
          currentByte = *array;
          while(counter <= maxHashValue && bitcounter < BITSINBYTE)
          {
          if(counter >= offset && getBitValue(currentByte, bitcounter))
          {
               UINT64 offsetValue = counter - offset;
               int leftShift = 8*8 - leftOffset - bitsPerPosition;
               positionToByteArray(bufferCurr, (offsetValue << leftShift), bufferPrev);
//             printf(" for value %d , %d\n", (int)offsetValue,  leftShift);
               status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, floor((bitsPerPosition + leftOffset)/8));
//             status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, sizeof(UINT64));
               bufferPrev = *(bufferCurr + (UINT8)floor((leftOffset+bitsPerPosition)/8));
               leftOffset = ((bitsPerPosition+leftOffset)%8);
//			printf("offset %d leftover value %x\n", leftOffset, bufferPrev);
//             printf("printed a value in 2\n");
          }
          bitcounter++;
          counter++;
          }
          if(bufferPrev !=0)
          {
               status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, 1);
          }
          array++;
     }
     return 0;
}

/****************************************************************************
* Description                                                               *
*      positionToByteArray takes a 64 bit integer and puts it in a          *
*      BITARRAY. It also takes in a bufferPrev, which will bit-wise OR the  *
*      first BYTE of the return value with the value of bufferPrev          *
* Arguments                                                                 *
*      BITARRAY* buffer                                                     *
*      UINT64 value                                                         * 
*      BYTE bufferPrev                                                      * 
* Return Values                                                             *
*      0 upon success or 1 upon error                                       *
****************************************************************************/
int positionToByteArray(BITARRAY* buffer, UINT64 value, BYTE bufferPrev)
{
     int byteNum = 0;
     BYTE oneByte;
     while(byteNum < sizeof(UINT64))
     {
          UINT8 bitsTillDesiredByte, bitsInSevenBytes;
          bitsTillDesiredByte = byteNum*BITSINBYTE;
          bitsInSevenBytes = 56;//(sizeof(UINT64)-1)*BITSINBYTE;
          oneByte = (BYTE)((value << bitsTillDesiredByte) >> bitsInSevenBytes);
//        printf("%x ", oneByte);
          if(byteNum == 0)
          {
               oneByte = oneByte|bufferPrev;
          }
          *buffer = oneByte;
          buffer++;
          byteNum += 1;
     }
//   printf(" the bytes for the number %llu\n", value);
     return 0;
}

/****************************************************************************
* Description                                                               *
*      ArrayToType2Write reads in an array of BYTE values and               *
*      creates a Type 1 level File.                                         *
* Arguments                                                                 *
*      BITARRAY *array                                                      *
*      UINT64 startIndex                                                    * 
*      UINT64 maxHashValue                                                  * 
*      UINT8 bitsPerPosition                                                *
*      UINT64 offset                                                        *
* Return Values                                                             *
*      0 upon success or 1 upon error                                       *
****************************************************************************/
int ArrayToType2Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue, UINT8 bitsPerPosition, UINT64 offset)
{
     POSITION counter = startIndex;
     UINT8 bitcounter = 0, currentByte;
     int success;
     BYTE bufferPrev = 0;
     BYTE* bufferCurr = (BYTE*)malloc(sizeof(UINT64));

     GMSTATUS status = STATUS_SUCCESS;
     UINT8 leftOffset=0, rightOffset=0;
 	
 	/*
 	take the first one, bitshift by bpp. Then print the floor(first bpp/8) Bytes. 
	 set leftOffset to 8-(bpp%8) and take the next Byte and set to buffer Prev
	take 2nd value, shift left bpp - leftOffset
	 or byte 1 with buffer Prev, Print all bytes up to floor(bitsPerPosition+leftOffset)/8
	 set leftOffset to 8-(bpp+leftOffset)%8 set bufferPrev to the next Byte 
	*/  
 	while(counter <= maxHashValue)
     {
          bitcounter=0;
          currentByte = *array;
          while(counter <= maxHashValue && bitcounter < BITSINBYTE)
          {
               if(counter >= offset && !getBitValue(currentByte, bitcounter))
               {
                    UINT64 offsetValue = counter - offset;
                    int leftShift = 8*8 - leftOffset - bitsPerPosition;
                    positionToByteArray(bufferCurr, (offsetValue << leftShift), bufferPrev);
//                  printf(" for value %d , %d\n", (int)offsetValue,  leftShift);
                    status = bitlib_file_write_bytes(compressed_filep_type2, bufferCurr, floor((bitsPerPosition + leftOffset)/8));
//                  status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, sizeof(UINT64));
                    bufferPrev = *(bufferCurr + (UINT8)floor((leftOffset+bitsPerPosition)/8));
                    leftOffset = ((bitsPerPosition+leftOffset)%8);
//			     printf("offset %d leftover value %x\n", leftOffset, bufferPrev);
//                  printf("printed a value in 2\n");
               }
               bitcounter++;
               counter++;
          }
          if(bufferPrev !=0)
          {
               status = bitlib_file_write_bytes(compressed_filep_type2, bufferCurr, 1);
          }
          array++;
     }
     return 0;
}

/****************************************************************************
* Description                                                               *
*      ArrayToType3Write reads in an array of BYTE values and               *
*      creates a Type 2 level File.                                         *
* Arguments                                                                 *
*      BITARRAY *array                                                      *
*      UINT64 minHashValue                                                  * 
*      UINT64 maxHashValue                                                  * 
* Return Values                                                             *
*      0 upon success or 1 upon error                                       *
****************************************************************************/
int ArrayToType3Write(BITARRAY *array, UINT64 minHashValue, UINT64 maxHashValue)
{
     POSITION counter = minHashValue;
     UINT8 bitcounter = 0, currentByte;
     int success;
     BYTE* buffer = (BYTE*)malloc(sizeof(UINT64));
     GMSTATUS status = STATUS_SUCCESS;
     char positionString[1];

     while(counter <= maxHashValue)
     {
          status = bitlib_file_write_bytes(compressed_filep_type3, array, 1);
          counter+=8;
          array++;
     }
     return 0;
}

/****************************************************************************
* Description                                                               *
*      getBitValue determines whether the bitnum bit in the byte currentByte*
*      is a 0 or 1                                                          *
* Arguments                                                                 *
*      BYTE* currentByte                                                    * 
*      UINT8 bitnum                                                         *
* Return Values                                                             *
*      UINT8 value                                                          *
****************************************************************************/
UINT8 getBitValue(BYTE currentByte, UINT8 bitnum)
{
     UINT8 x = currentByte << bitnum;
     x = x >> (BITSINBYTE - 1);
     return x;
}

/****************************************************************************
* Description                                                               *
*      findMinValueFromArray determines the minimum bitIndex of a one in    *
*      the array.                                                           *
* Arguments                                                                 *
*      BITARRAY* array                                                      * 
*      UINT64 length                                                        *
* Return Values                                                             *
*      UINT64 minIndex                                                      *
****************************************************************************/
UINT64 findMinValueFromArray(BITARRAY* array, UINT64 length)
{
     UINT64 counter = 0;
     int bitcounter;
     BYTE currentByte;
     UINT8 value;
     while(counter < length)
     {
          bitcounter=0;
          currentByte = *array;
          while(counter <= length && bitcounter < BITSINBYTE)
          {
               value = getBitValue(currentByte, bitcounter);
               if(value ==1)
               {
                    return counter;
               }
               bitcounter++;
               counter++;
          }
          array++;
     }
     return length;
}
/****************************************************************************
* Description                                                               *
*      findMaxValueFromArray determines the maximum bitIndex of a one in    *
*      the array.                                                           *
* Arguments                                                                 *
*      BITARRAY* array                                                      * 
*      UINT64 length                                                        *
* Return Values                                                             *
*      UINT64 maxIndex                                                      *
****************************************************************************/
UINT64 findMaxValueFromArray(BITARRAY* array, UINT64 length)
{
     UINT64 counter=0, lastVal;
     int bitcounter;
     BYTE currentByte;
     UINT8 value;

     while(counter < length)
     {
          bitcounter=0;
          currentByte = *array;
          while(counter <= length && bitcounter < BITSINBYTE)
	     {
               value = getBitValue(currentByte, bitcounter);
               if(value ==1)
               {
                    lastVal = counter;
               }
               bitcounter++;
               counter++;
          }
          array++;
     }
     return lastVal;
}
