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
gzFile        compressed_filep;
gzFile        compressed_filep_type0;
gzFile        compressed_filep_type1;
gzFile        compressed_filep_type2;
gzFile        compressed_filep_type3;

/********************************************************************************
 * Description
 *      WriteLevelFile reads an array of hash values and returns a
 *      level file. This file may be of any format type. The function
 *      optimizes for size by generating all types sequentially and
 *      comparing size of file after each file is generated. If the endIndex
 *      is greater than the startIndex then the function knows that the
 *      array has been corrupted at some other level
 * Arguments
 *      char* compressed_filename
 *      BITARRAY *array
 *      POSITION startIndex
 *      POSITION endIndex
 * Return Values
 *      int 0 for success -1 for error
 *********************************************************************************/
int WriteLevelFile(char* compressed_filename, BITARRAY* array, POSITION startIndex, POSITION endIndex)
{
	char filename_type0[82];
	char filename_type1[82];
	char filename_type2[82];
	char filename_type3[82];
	UINT64 minHashValue, maxHashValue;
	char* check ="1";
	minHashValue = startIndex + findMinValueFromArray(array, endIndex-startIndex);
	maxHashValue = startIndex + findMaxValueFromArray(array, endIndex-startIndex);

	int type = 0;

	if(endIndex < startIndex)
	{
		return 1;
	}
	UINT64 bitsPerPosition = log2(maxHashValue - minHashValue) + 1;

	//      UINT64 length = ceil((maxHashValue - minHashValue)/BITSINBYTE);

	while(type < NUMOFTYPES) // create the four file types
	{
		if(type == 0)
		{
			strncpy(filename_type0, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
			filename_type0[strlen(compressed_filename)-LENGTHOFEXT] = '\0';
			strcat(filename_type0, "_0.dat.gz");
			printf("0: Compressed_filename is:  %s \n", filename_type0);
			compressed_filep_type0 = gzopen(filename_type0, "wb");
			if(!compressed_filep_type0)
			{
				printf("0: Couldn't open file");
				return 1;
			}
			writeHeader(compressed_filep_type0, minHashValue, maxHashValue, 0, type);
			printf("0: Header is Written to File\n");
			ArrayToType0Write(array, startIndex, maxHashValue);
			printf("0: Body is Written to File\n");
			bitlib_file_write_bytes(compressed_filep_type0, (BYTE *)check, strlen(check));
			gzclose(compressed_filep_type0);

		}

		if(type == 1)
		{
			strncpy(filename_type1, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
			filename_type1[strlen(compressed_filename)-LENGTHOFEXT] = '\0';
			strcat(filename_type1, "_1.dat.gz");
			printf("1: Compressed_filename is:  %s \n", filename_type1);
			compressed_filep_type1 = gzopen(filename_type1, "wb");
			if(!compressed_filep_type1)
			{
				printf("1: couldn't open file");
				return 1;
			}
			writeHeader(compressed_filep_type1, minHashValue, maxHashValue, 0,  type);
			printf("1: Header is Written to File\n");
			ArrayToType1Write(array, startIndex, maxHashValue, bitsPerPosition, minHashValue);
			printf("1: Body is Written to File\n");
			bitlib_file_write_bytes(compressed_filep_type1, (BYTE *)check, strlen(check));
			gzclose(compressed_filep_type1);
		}

		if(type == 2)
		{
			strncpy(filename_type2, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
			filename_type2[strlen(compressed_filename)-LENGTHOFEXT] = '\0';
			strcat(filename_type2, "_2.dat.gz");

			printf("2: Compressed_filename is:  %s \n", filename_type2);
			compressed_filep_type2 = gzopen(filename_type2, "wb");
			if(!compressed_filep_type2)
			{
				printf("2: couldn't open file");
				return 1;
			}
//			printf("bpppppp %llu %llu %llu", bitsPerPosition, maxHashValue, minHashValue);
			writeHeader(compressed_filep_type2, minHashValue, maxHashValue, findLastZero(array, maxHashValue-minHashValue) - findMinValueFromArray(array, endIndex-startIndex), type);
			printf("2: Header is Written to File\n");
			ArrayToType2Write(array, startIndex, maxHashValue, bitsPerPosition, minHashValue);

			printf("2: Body is Written to File\n");
			bitlib_file_write_bytes(compressed_filep_type2, (BYTE *)check, strlen(check));
			gzclose(compressed_filep_type2);
		}

		if(type == 3)
		{
			strncpy(filename_type3, compressed_filename, strlen(compressed_filename)-LENGTHOFEXT);
			filename_type3[strlen(compressed_filename)-LENGTHOFEXT] = '\0';
			strcat(filename_type3, "_3.dat.gz");
			printf("3: Compressed_filename is:  %s \n", filename_type3);
			compressed_filep_type3 = gzopen(filename_type3, "wb");
			if(!compressed_filep_type3)
			{
				printf("3: couldn't open file");
				return 1;
			}
			writeHeader(compressed_filep_type3, minHashValue, maxHashValue, 0, type);
			printf("3: Header is Written to File\n");

			ArrayToType3Write(array, startIndex, minHashValue, maxHashValue);
			printf("3: Body is Written to File\n");
			bitlib_file_write_bytes(compressed_filep_type3, (BYTE *)check, strlen(check));
			gzclose(compressed_filep_type3);
		}

		type++;
	}

	char* fnameChosen = NULL; /*s
	                             stat(filename_type0, &fileinfo0);
	                             stat(filename_type1, &fileinfo1);
	                             stat(filename_type2, &fileinfo2);
	                             stat(filename_type3, &fileinfo3);
	                             if(fileinfo1.st_size > fileinfo0.st_size)
	                             {
	                             if(fileinfo2.st_size > fileinfo1.st_size)
	                             {
	                             if(fileinfo3.st_size > fileinfo2.st_size)
	                             {
	                             compressed_filep = compressed_filep_type3;
	                             fnameChosen = filename_type3;
	                             remove(filename_type0);
	                             remove(filename_type1);
	                             remove(filename_type2);
	                             printf("type 3a");
	                             }
	                             else
	                             {
	                             compressed_filep = compressed_filep_type2;
	                             fnameChosen = filename_type2;
	                             remove(filename_type0);
	                             remove(filename_type1);
	                             remove(filename_type3);
	                             printf("type 2a");
	                             }
	                             }
	                             else
	                             {
	                             if(fileinfo3.st_size > fileinfo1.st_size)
	                             {
	                             compressed_filep = compressed_filep_type3;
	                             fnameChosen = filename_type3;
	                             remove(filename_type0);
	                             remove(filename_type1);
	                             remove(filename_type2);
	                             printf("type 3b");
	                             }
	                             else
	                             {
	                             compressed_filep = compressed_filep_type1;
	                             fnameChosen = filename_type1;
	                             remove(filename_type0);
	                             remove(filename_type3);
	                             remove(filename_type2);
	                             printf("type 1a");
	                             }
	                             }
	                             }
	                             else
	                             {
	                             if(fileinfo2.st_size > fileinfo0.st_size)
	                             {
	                             if(fileinfo3.st_size > fileinfo2.st_size)
	                             {
	                             compressed_filep = compressed_filep_type3;
	                             fnameChosen = filename_type3;
	                             remove(filename_type0);
	                             remove(filename_type1);
	                             remove(filename_type2);
	                             printf("type 3c");
	                             }
	                             else
	                             {
	                             compressed_filep = compressed_filep_type2;
	                             fnameChosen = filename_type2;
	                             remove(filename_type0);
	                             remove(filename_type1);
	                             remove(filename_type3);
	                             printf("type 2b");
	                             }
	                             }
	                             else
	                             {
	                             if(fileinfo3.st_size > fileinfo0.st_size)
	                             {
	                             compressed_filep = compressed_filep_type3;
	                             fnameChosen = filename_type3;
	                             remove(filename_type0);
	                             remove(filename_type1);
	                             remove(filename_type2);
	                             printf("type 3d");
	                             }
	                             else
	                             {
	                             compressed_filep = compressed_filep_type0;
	                             fnameChosen = filename_type0;
	                             remove(filename_type3);
	                             remove(filename_type1);
	                             remove(filename_type2);
	                             printf("type 0a");
	                             }
	                             }
	                             }*/
	compressed_filep = compressed_filep_type0;
	fnameChosen = filename_type0;
	remove(filename_type3);
	remove(filename_type1);
	remove(filename_type2);
	rename(fnameChosen, compressed_filename);

	return 0;
}

/****************************************************************************
* Description
*      writeHeader writes the header information and header comments
*      to the file
* Arguments
*      gzFile *file
*      UINT64 minHashValue
*      UINT64 maxHashValue
*      int type
* Return Values
*      0 upon success or 1 upon error
****************************************************************************/
int writeHeader(gzFile file, UINT64 minHashValue, UINT64 maxHashValue, UINT64 lastZero, int type)
{
	char* check= "1\n";
	char comment1[80];
	char comment2[80];
	int status;
	char* typeH = (char*)malloc((1+log(UINT8_MAX))+ sizeof(char));
	char* header = (char*)malloc((log(UINT64_MAX)+1)*3 + sizeof(UINT8) + 3*sizeof(char));

	sprintf(comment1,"### GAMESMAN Bagh Chal Type %d Level Files\n", type);
	sprintf(comment2,"### Doc on level file formats in doc/LevelFiles.txt %c\n", 0x0C);
	sprintf(typeH, "%d\n", type);
	if(type != 2)
		sprintf(header, "%llu %llu %c\n", minHashValue, maxHashValue, 0x0C);
	else
		sprintf(header, "%llu %llu %llu %c\n", minHashValue, maxHashValue, lastZero, 0x0C);

	status = bitlib_file_write_bytes(file, (BYTE *)check, strlen(check));
	status = bitlib_file_write_bytes(file, (BYTE *)comment1, strlen(comment1));
	status = bitlib_file_write_bytes(file, (BYTE *)comment2, strlen(comment2));
	status = bitlib_file_write_bytes(file, (BYTE *)typeH, strlen(typeH));
	status = bitlib_file_write_bytes(file, (BYTE *)header, strlen(header));
	if(typeH)
		free(typeH);
	if(header)
		free(header);
	if(status)
	{
		return 1;
	}
	return 0;
}


//returns 0 if valid and 1 if not valid
int isValidLevelFile(char* compressed_filename)
{
	//read bytes until two 0xC 0xA have been found. Then read next bit, must be 1
	int status;
	int firstValid = 0;
	int endSectionCounter = 0;
	int sawx0C = 0;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	compressed_filep = gzopen(compressed_filename, "rb");
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	if((BYTE)(*buffer)=='1')
	{
		firstValid = 1;
	}
	while(status == STATUS_SUCCESS && endSectionCounter < 3)
	{
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
		if(status)
		{
			return 1;
		}
		if(*buffer == 0x0C)
		{
			sawx0C = 1;
		}
		else if (sawx0C)
		{
			sawx0C = 0;
			if(*buffer == 0x0A)
			{
				endSectionCounter++;
			}
		}
	}
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	if((BYTE)(*buffer)!='1')
	{
		if(buffer)
			free(buffer);
		gzclose(compressed_filep);
		return 1;
	}
	else
	{
		if(buffer)
			free(buffer);
		gzclose(compressed_filep);
		return 0;
	}
	gzclose(compressed_filep);
	return 1;
}
/****************************************************************************
* Description
*      ArrayToType0Write reads in an array of BYTE values and
*      creates a Type 0 level File.
* Arguments
*      BITARRAY *array
*      UINT64 startIndex
*      UINT64 maxHashValue
* Return Values
*      0 upon success or 1 upon error
****************************************************************************/
int ArrayToType0Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue)
{
	POSITION counter = startIndex;
	UINT8 bitcounter = 0, currentByte;
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
				status = bitlib_file_write_bytes(compressed_filep_type0, (BYTE *)positionString, strlen(positionString));
			}
			bitcounter++;
			counter++;
		}
		array++;
	}
	char endline = 0x0C;
	sprintf(positionString, "%c\n", endline);
	status = bitlib_file_write_bytes(compressed_filep_type0, (BYTE *)positionString, strlen(positionString));
	if(buffer)
		free(buffer);
	return 0;
}

/****************************************************************************
* Description
*      ArrayToType1Write reads in an array of BYTE values and
*      creates a Type 0 level File.
* Arguments
*      BYTE *array
*      UINT64 startIndex
*      UINT64 maxHashValue
*      UINT8 bitsPerPosition
*      UINT64 offset
* Return Values
*      0 upon success or 1 upon error
****************************************************************************/
int ArrayToType1Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue, UINT64 bitsPerPosition, UINT64 offset)
{

	POSITION counter = startIndex;
	UINT8 bitcounter = 0, currentByte;
	BYTE bufferPrev = 0;
	BYTE* bufferCurr = (BYTE*)malloc(sizeof(UINT64));
	char positionString[22];
	GMSTATUS status = STATUS_SUCCESS;
	UINT8 leftOffset=0;
	int numBytesInCurrent = 0;
	UINT64 offsetValue;
	int leftShift;
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
		while(status == STATUS_SUCCESS && counter <= maxHashValue && bitcounter < BITSINBYTE)
		{
			if(counter >= offset && getBitValue(currentByte, bitcounter)) // you have a position
			{
				offsetValue = counter - offset;
				leftShift = 8*8 - leftOffset - bitsPerPosition;
				positionToByteArray(bufferCurr, offsetValue << leftShift, bufferPrev);
				status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, floor((bitsPerPosition + leftOffset)/8));
//				bitlib_print_bytes_in_bits(bufferCurr, floor((bitsPerPosition + leftOffset)/8));
//				printf("counter %d amountt %d", counter, (UINT8)(bitsPerPosition));
				numBytesInCurrent = floor((leftOffset + bitsPerPosition)/8);
				bufferPrev = *(bufferCurr + (UINT8)floor((leftOffset+bitsPerPosition)/8));
				leftOffset = ((bitsPerPosition+leftOffset)%8);
			}
			bitcounter++;
			counter++;
		}
		if(bufferPrev !=0)
		{
			status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, 1);
			//bitlib_print_bytes_in_bits(bufferCurr, 1);

		}
		array++;
	}

	sprintf(positionString, "%c\n", 0x0C);
	status = bitlib_file_write_bytes(compressed_filep_type0, (BYTE *)positionString, strlen(positionString));
	if(bufferCurr)
		free(bufferCurr);
	return 0;
}

/****************************************************************************
* Description
*      positionToByteArray takes a 64 bit integer and puts it in a
*      BITARRAY. It also takes in a bufferPrev, which will bit-wise OR the
*      first BYTE of the return value with the value of bufferPrev
* Arguments
*      BITARRAY* buffer
*      UINT64 value
*      BYTE bufferPrev
* Return Values
*      0 upon success or 1 upon error
****************************************************************************/
int positionToByteArray(BITARRAY* buffer, UINT64 value, BYTE bufferPrev)
{
	int byteNum = 0;
	BYTE oneByte;
//	printf("value %x	\n", bufferPrev);
	while(byteNum < sizeof(UINT64))
	{
		UINT8 bitsTillDesiredByte, bitsInSevenBytes;
		bitsTillDesiredByte = byteNum*BITSINBYTE;
		bitsInSevenBytes = 56; //(sizeof(UINT64)-1)*BITSINBYTE;
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
* Description
*      ArrayToType2Write reads in an array of BYTE values and
*      creates a Type 1 level File.
* Arguments
*      BITARRAY *array
*      UINT64 startIndex
*      UINT64 maxHashValue
*      UINT8 bitsPerPosition
*      UINT64 offset
* Return Values
*      0 upon success or 1 upon error
****************************************************************************/
int ArrayToType2Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue, UINT64 bitsPerPosition, UINT64 offset)
{
	POSITION counter = startIndex;
	UINT8 bitcounter = 0, currentByte;
	BYTE bufferPrev = 0;
	BYTE* bufferCurr = (BYTE*)malloc(sizeof(UINT64));
	char positionString[22];
	GMSTATUS status = STATUS_SUCCESS;
	UINT8 leftOffset=0;
	int numBytesInCurrent = 0;
	UINT64 offsetValue;
	int leftShift;
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
		while(status == STATUS_SUCCESS && counter <= maxHashValue && bitcounter < BITSINBYTE)
		{
			if(counter >= offset && !getBitValue(currentByte, bitcounter)) // you have a position
			{
				offsetValue = counter - offset;
				leftShift = 8*8 - leftOffset - bitsPerPosition;
				positionToByteArray(bufferCurr, offsetValue << (64 - bitsPerPosition-leftOffset), bufferPrev);
				status = bitlib_file_write_bytes(compressed_filep_type1, bufferCurr, floor((bitsPerPosition + leftOffset)/8));
				numBytesInCurrent = floor((leftOffset + bitsPerPosition)/8);
				bufferPrev = *(bufferCurr + (UINT8)floor((leftOffset+bitsPerPosition)/8));
				leftOffset = ((bitsPerPosition+leftOffset)%8);
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

	sprintf(positionString, "%c\n", 0x0C);
	status = bitlib_file_write_bytes(compressed_filep_type0, (BYTE *)positionString, strlen(positionString));
	if(bufferCurr)
		free(bufferCurr);
	return 0;
}

/****************************************************************************
* Description
*      ArrayToType3Write reads in an array of BYTE values and
*      creates a Type 2 level File.
* Arguments
*      BITARRAY *array
*      UINT64 minHashValue
*      UINT64 maxHashValue
* Return Values
*      0 upon success or 1 upon error
****************************************************************************/
int ArrayToType3Write(BITARRAY *array, UINT64 startIndex, UINT64 minHashValue, UINT64 maxHashValue)
{
	POSITION counter = minHashValue;
	BYTE* buffer = (BYTE*)malloc(sizeof(UINT8));
	GMSTATUS status = STATUS_SUCCESS;
	char positionString[1];
	UINT64 temp = minHashValue - startIndex;
	int offset = temp % 8;
	array += (temp / 8);



// Get this to just rite the 8 bits skipping the 0's between start index and minHashValue
	while(counter <= maxHashValue)
	{
		*buffer = (*array << offset) + (*(array+1) >> (8-offset));
		status = bitlib_file_write_bytes(compressed_filep_type3, buffer, 1);
		counter+=8;
		array++;

	}

	sprintf(positionString, "%c\n", 0x0C);
	status = bitlib_file_write_bytes(compressed_filep_type0, (BYTE *)positionString, strlen(positionString));
	if(buffer)
		free(buffer);
	return 0;
}

/****************************************************************************
* Description
*      ReadLevelFile takes a pointer to a bitarray and a compressed file
*      and creates a bitarray starting at the files minHashValue until
*      its maxHashValue (0=not valid, 1 = valid
* Arguments
*      char* compressed_filename
*      BITARRAY *array
*      int length   (maxHashValue-minHashValue)
* Return Values
*      UINT8 status
****************************************************************************/
int ReadLevelFile(char* compressed_filename, BITARRAY *array, int length)
{
	int status;
	//length++;
	int type = getLevelFileType(compressed_filename);
	if(type == 0)
	{
		status = readLevelFileType0(compressed_filename, array, length);
	}
	else if(type == 1)
	{
		status = readLevelFileType1(compressed_filename, array, length);
	}
	else if(type == 2)
	{
		status = readLevelFileType2(compressed_filename, array, length);
	}
	else if(type == 3)
	{
		status = readLevelFileType3(compressed_filename, array, length);
	}
	else
	{
		return 1;
	}
	return 0;
}
/****************************************************************************
* Description
*      readLevelFileType0 takes a pointer to a bitarray and a compressed file
*      and creates a bitarray starting at the files minHashValue until
*      its maxHashValue (0=not valid, 1 = valid
* Arguments
*      char* compressed_filename
*      BITARRAY *array
*      int length   (maxHashValue-minHashValue)
* Return Values
*      UINT8 status
****************************************************************************/
int readLevelFileType0(char* compressed_filename, BITARRAY *array, int length)
{
	int index=0;
	UINT64 minHashValue = getLevelFileMinHashValue(compressed_filename);
	UINT64 temp = 0;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	int bitValue=0;
	int status = STATUS_SUCCESS;
	int endSectionCounter = 0;
	int diff  = 0;
	compressed_filep = gzopen(compressed_filename, "rb");
	//length ++;
	//skips header section which has two 0xC (at end of header) and all other ASCII chars
	while(status == STATUS_SUCCESS && endSectionCounter < 2)
	{
		//printf("Begining Header Parse");
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
		if(status)
		{
			return 1;
		}
		if(*buffer == 0x0C)
		{
			endSectionCounter++;
		}
	}
	//copies data into the bitarray
	temp = 0;
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1); // new line
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	while(index <= length && *buffer!= 0x0C)
	{
		//   printf("buffer %d", *buffer);
		if((*buffer) != 0x20) //create number
		{
			temp = temp * 10 + ((*buffer) - '0');
//		  printf(" number %d  --", temp);
		}
		else //number done, put in array
		{
			diff = temp - minHashValue;
			while(index < diff)
			{
				bitValue = index % 8;
				if(bitValue ==0)
				{
					(*array) = 0;
				}
				index++;
				if(bitValue == 7)
				{
//					printf("== %d %x==", bitValue, *array);
					array++;
				}
			}
			if(index == diff)
			{
				bitValue = index % 8;
//				printf("\nputting in a 1 %d , %d,  %d bitValue", temp, bitValue, index);
				if(bitValue ==0)
				{
					(*array) = 0;
				}
				(*array) = (*array) | (1 << (7-bitValue));
				index++;
			}
			if(bitValue == 7)
			{
//				printf("== %d %x==", bitValue, *array);
				array++;
			}
			temp = 0;
		}
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	}
	printf("== %d %x==", bitValue, *array);

	bitlib_print_bytes_in_bits(array, 2);
	if(buffer)
		free(buffer);
	gzclose(compressed_filep);
	return 0;
}

/****************************************************************************
* Description
*      readLevelFileType1 takes a pointer to a bitarray and a compressed
*      file and creates a bitarray starting at the files minHashValue until
*      its maxHashValue (0=not valid, 1 = valid
* Arguments
*      char* compressed_filename
*      BITARRAY *array
*      int length   (maxHashValue-minHashValue)
* Return Values
*      UINT8 status
****************************************************************************/
int readLevelFileType1(char* compressed_filename, BITARRAY *bitArray, int length)
{
	int index = 0;
	UINT64 minHashValue = getLevelFileMinHashValue(compressed_filename);
	UINT64 maxHashValue = getLevelFileMaxHashValue(compressed_filename);
	UINT64 bitsPerPosition = log2(maxHashValue - minHashValue) + 1;
	BYTE*     buffer = (BYTE*)malloc(sizeof(BYTE));
	int status = STATUS_SUCCESS;
	int endSectionCounter = 0;

	//    BITARRAY* bitArray = (BITARRAY*)malloc(sizeof(BITARRAY)*BITSINPOS/BITSINBYTE);
	int currentIndex = 0;
	BYTE*     currentByte = (BYTE*)malloc(sizeof(BYTE));

	compressed_filep = gzopen(compressed_filename, "rb");
	(*bitArray) = 0;
	//skips header section which has two 0xC 's(at end of header) and all other ASCII chars
	while(status == STATUS_SUCCESS && endSectionCounter < 2)
	{
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
		if(status)
		{
			return status;
		}
		if(*buffer == 0x0C)
		{
			endSectionCounter++;
		}
	}
	//copies data into the bitarray
	int tempBitIndexInByte = 1;
	UINT8 tempNumber = 0;
	int bitValue;
	int counter = 0;
	int lastTempNumber = -1;
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	while((lastTempNumber+1) < length)
	{
		if(tempBitIndexInByte != 1) //stll evaluating same byte
		{
			printf("case1\n");
			printf("--- %x %d\n", *buffer, currentIndex);
			if((8 - (tempBitIndexInByte - 1)) > bitsPerPosition) // doesn't use up rest of byte
			{
				printf(" == %d %x==", tempBitIndexInByte, *buffer);
				tempNumber += (UINT8)((UINT8)((*buffer) << (tempBitIndexInByte - 1)) >> (8 - (bitsPerPosition)));
				tempBitIndexInByte += bitsPerPosition;
				//add number to bitArray
				lastTempNumber = tempNumber;
				printf("1tempNumber %d\n", tempNumber);
				///ADDNUMBERTOBITARRAY
				if(counter > length)
				{
					gzclose(compressed_filep);
					if(buffer)
						free(buffer);
					if(currentByte)
						free(currentByte);
					return 0;
				}
				else
				{
					while(counter < tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						counter++;
					}
					if(counter == tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						(*bitArray) = (*bitArray) | (1 << (7-bitValue));
						counter++;
					}
					if(bitValue == 7)
					{
						bitArray++;
					}
				}
				//ENDADDNUMBERTOBITARRAY
				tempNumber = 0;
				currentIndex =0;
			}
			else // uses up rest of Byte
			{
				tempNumber += (UINT8)((UINT8)((*buffer) << (tempBitIndexInByte - 1)) >> (tempBitIndexInByte - 1));
				printf(" *** %d %d ", currentIndex, tempBitIndexInByte);
				currentIndex += (8-(tempBitIndexInByte-1));
				tempBitIndexInByte = 1;
				if(currentIndex >= bitsPerPosition) //add number to bitArray
				{
//					printf("add case1b to array ");
					///ADDNUMBERTOBITARRAY
					printf("2tempNumber %d\n", tempNumber);
					lastTempNumber = tempNumber;
					if(counter > length)
					{
						gzclose(compressed_filep);
						if(buffer)
							free(buffer);
						if(currentByte)
							free(currentByte);
						return 0;
					}
					else
					{
						while(counter < tempNumber)
						{
//							printf("counter %d %d", counter, tempNumber);
							bitValue = counter % 8;
							if(bitValue ==0)
							{
								(*bitArray) = 0;
							}
							counter++;
							if(bitValue == 7)
							{
								bitArray++;
							}
						}
						if(counter == tempNumber)
						{
							bitValue = counter % 8;
							if(bitValue ==0)
							{
								(*bitArray) = 0;
							}
							(*bitArray) = (*bitArray) | (1 << (7-bitValue));
							counter++;
						}
						if(bitValue == 7)
						{
							bitArray++;
						}
					}
					//ENDADDNUMBERTOBITARRAY
					currentIndex = 0;
					tempNumber = 0;
				}
			}
		}
		else if((currentIndex + 8) < bitsPerPosition) //if using whole Byte
		{
			printf("case2\n");
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			printf("--- %x %d\n", *buffer, currentIndex);
			index++;
			tempNumber += ((*buffer) << (bitsPerPosition - currentIndex));
			tempBitIndexInByte = 1;
			currentIndex += 8;
			if(currentIndex >= bitsPerPosition) //add number to bitArray
			{
				printf("3tempNumber %d\n", tempNumber);
				///ADDNUMBERTOBITARRAY
				lastTempNumber = tempNumber;
				if(counter > length)
				{
					gzclose(compressed_filep);
					if(buffer)
						free(buffer);
					if(currentByte)
						free(currentByte);
					return 0;
				}
				else
				{
					while(counter < tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						if(bitValue == 7)
						{
							bitArray++;
						}
						counter++;
					}
					if(counter == tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						(*bitArray) = (*bitArray) | (1 << (7-bitValue));
						counter++;
					}
					if(bitValue == 7)
					{
						bitArray++;
					}
				}
				//ENDADDNUMBERTOBITARRAY
				currentIndex = 0;
				tempNumber = 0;
			}
		}
		else // getting new byte but only using some of it
		{
			printf("case3\n");
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			printf("--- %x %d\n", *buffer, currentIndex);
			index++;
			if(currentIndex)
				tempNumber = (tempNumber << (bitsPerPosition - currentIndex))  + ((*buffer) >> (8-(bitsPerPosition-currentIndex)));
			else
				tempNumber = (((*buffer) << (tempBitIndexInByte - 1))  >> (8 - bitsPerPosition));

			tempBitIndexInByte = (bitsPerPosition - currentIndex + 1);
//			printf("tempNumber %x %d %d\n", *buffer, bitsPerPosition, tempBitIndexInByte);
			currentIndex = bitsPerPosition;
			if(currentIndex >= bitsPerPosition) //add number to bitArray
			{
				///ADDNUMBERTOBITARRAY
				lastTempNumber = tempNumber;
				printf("4tempNumber %d\n", tempNumber);
				if(counter > length)
				{
					gzclose(compressed_filep);
					if(buffer)
						free(buffer);

					if(currentByte)
						free(currentByte);
					return 0;
				}
				else
				{
					while(counter < tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						counter++;
						if(bitValue == 7)
						{
							bitArray++;
						}
					}
					if(counter == tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						(*bitArray) = (*bitArray) | (1 << (7-bitValue));
						counter++;
					}
					if(bitValue == 7)
					{
						bitArray++;
					}
				}
				//ENDADDNUMBERTOBITARRAY
				currentIndex = 0;
				tempNumber = 0;
			}
			//printf("bitInByte %d  CurrentIndex %d \n", tempBitIndexInByte, currentIndex);
		}
	}
	// read bits and form number until bits per postition, then increment index

	gzclose(compressed_filep);

	if(buffer)
		free(buffer);

	if(currentByte)
		free(currentByte);

	return 0;
}
/****************************************************************************
* Description
*      readLevelFileType2 takes a pointer to a bitarray and a compressed
*      file and creates a bitarray starting at the files minHashValue until
*      its maxHashValue (0=not valid, 1 = valid
* Arguments
*      char* compressed_filename
*      BITARRAY *array
*      int length   (maxHashValue-minHashValue)
* Return Values
*      UINT8 status
****************************************************************************/
int readLevelFileType2(char* compressed_filename, BITARRAY *bitArray, int length)
{
	int index = 0;
	UINT64 minHashValue = getLevelFileMinHashValue(compressed_filename);
	UINT64 maxHashValue = getLevelFileMaxHashValue(compressed_filename);
	UINT64 lastZero = getLastZero(compressed_filename);
	UINT64 bitsPerPosition = log2(maxHashValue - minHashValue) + 1;
	BYTE*     buffer = (BYTE*)malloc(sizeof(BYTE));
	int status = STATUS_SUCCESS;
	int endSectionCounter = 0;

	//    BITARRAY* bitArray = (BITARRAY*)malloc(sizeof(BITARRAY)*BITSINPOS/BITSINBYTE);
	int currentIndex = 0;
	BYTE*     currentByte = (BYTE*)malloc(sizeof(BYTE));

	compressed_filep = gzopen(compressed_filename, "rb");
	(*bitArray) = 0;
	//skips header section which has two 0xC 's(at end of header) and all other ASCII chars
	while(status == STATUS_SUCCESS && endSectionCounter < 2)
	{
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
		if(status)
		{
			return 1;
		}
		if(*buffer == 0x0C)
		{
			endSectionCounter++;
		}
	}
	//copies data into the bitarray
	int tempBitIndexInByte = 1;
	UINT8 tempNumber = 0;
	int bitValue;
	int counter = 0;
	int lastTempNumber = -1;
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	while(counter <= lastZero)
	{
		if(tempBitIndexInByte != 1) //stll evaluating same byte
		{
//			printf("case1\n");
			if((8 - (tempBitIndexInByte - 1)) > bitsPerPosition) // doesn't use up rest of byte
			{
				tempNumber += (UINT8)((UINT8)((*buffer) << (tempBitIndexInByte - 1)) >> (8 - (bitsPerPosition)));
				tempBitIndexInByte += bitsPerPosition;
				//add number to bitArray
				lastTempNumber = tempNumber;
				///ADDNUMBERTOBITARRAY
				if(counter > length)
				{
					gzclose(compressed_filep);
					if(buffer)
						free(buffer);
					if(currentByte)
						free(currentByte);
					return 0;
				}
				else
				{
					while(counter < tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						(*bitArray) = (*bitArray) | (1 << (7-bitValue));
						counter++;
					}
					if(counter == tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						counter++;
					}
					if(bitValue == 7)
					{
						bitArray++;
					}
				}
				//ENDADDNUMBERTOBITARRAY
				tempNumber = 0;
				currentIndex =0;
			}
			else // uses up rest of Byte
			{
				tempNumber += (UINT8)((UINT8)((*buffer) << (tempBitIndexInByte - 1)) >> (tempBitIndexInByte - 1));
				currentIndex = (8-(tempBitIndexInByte-1));
				tempBitIndexInByte = 1;
				if(currentIndex >= bitsPerPosition) //add number to bitArray
				{
					///ADDNUMBERTOBITARRAY
					lastTempNumber = tempNumber;
					if(counter > length)
					{
						gzclose(compressed_filep);
						if(buffer)
							free(buffer);
						if(currentByte)
							free(currentByte);
						return 0;
					}
					else
					{
						while(counter < tempNumber)
						{
							bitValue = counter % 8;
							if(bitValue ==0)
							{
								(*bitArray) = 0;
							}
							(*bitArray) = (*bitArray) | (1 << (7-bitValue));
							counter++;
						}
						if(counter == tempNumber)
						{
							bitValue = counter % 8;
							if(bitValue ==0)
							{
								(*bitArray) = 0;
							}
							counter++;
						}
						if(bitValue == 7)
						{
							bitArray++;
						}

						currentIndex = 0;
						tempNumber = 0;
					}
					//ENDADDNUMBERTOBITARRAY

				}
			}
		}
		else if((currentIndex + 8) < bitsPerPosition) //if using whole Byte
		{
//			printf("case2\n");
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == 0x0C)
				endSectionCounter = 1;
			else
			{
				if(endSectionCounter == 1 && (*buffer) == 0x0A)
				{
					///ADDNUMBERTOBITARRAY
					lastTempNumber = tempNumber;
					if(counter > length)
					{
						gzclose(compressed_filep);
						if(buffer)
							free(buffer);
						if(currentByte)
							free(currentByte);
						return 0;
					}
					else
					{
						while(counter <= length)
						{
							bitValue = counter % 8;
							if(bitValue ==0)
							{
								(*bitArray) = 0;
							}
							(*bitArray) = (*bitArray) | (1 << (7-bitValue));
							counter++;
						}
						if(bitValue == 7)
						{
							bitArray++;
						}
					}
					//ENDADDNUMBERTOBITARRAY
				}
				endSectionCounter = 0;
			}
			index++;
			tempNumber += ((*buffer) << (bitsPerPosition - currentIndex));
			tempBitIndexInByte = 1;
			currentIndex += 8;
			if(currentIndex >= bitsPerPosition) //add number to bitArray
			{
				///ADDNUMBERTOBITARRAY
				lastTempNumber = tempNumber;
				if(counter > length)
				{
					gzclose(compressed_filep);
					if(buffer)
						free(buffer);
					if(currentByte)
						free(currentByte);
					return 0;
				}
				else
				{
					while(counter < tempNumber)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						(*bitArray) = (*bitArray) | (1 << (7-bitValue));
						counter++;
					}
					if(counter == tempNumber)
					{

						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
						counter++;
					}
					if(bitValue == 7)
					{
						bitArray++;
					}
				}
				//ENDADDNUMBERTOBITARRAY
				currentIndex = 0;
				tempNumber = 0;
			}
		}
		else // getting new byte but only using some of it
		{
//			printf("case3\n");
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == 0x0C)
			{
				printf("ksljfd?");
				endSectionCounter = 1;
			}
			else
			{
				if(endSectionCounter == 1 && (*buffer) != 0x0A)
				{

					if(counter > length)
					{
						gzclose(compressed_filep);
						if(buffer)
							free(buffer);
						if(currentByte)
							free(currentByte);
						return 0;
					}
					else
					{
						lastTempNumber = tempNumber;
//						printf("tempnumber %d", tempNumber);
						while(counter <= length)
						{
							bitValue = counter % 8;
							if(bitValue ==0)
							{
								(*bitArray) = 0;
							}
							(*bitArray) = (*bitArray) | (1 << (7-bitValue));
							printf(" conter bitvalue, bitarray %d, %d, %d", counter, bitValue, *bitArray);
							counter++;
							if(bitValue == 7)
							{
								bitArray++;
							}
						}
						if(bitValue == 7)
						{
							bitArray++;
						}
						endSectionCounter = 0;
					}
				}
				else
				{
					while(counter <= length)
					{
						bitValue = counter % 8;
						if(bitValue ==0)
						{
							(*bitArray) = 0;
						}
//						printf("count %d", counter);
						(*bitArray) = (*bitArray) | (1 << (7-bitValue));
						counter++;
						if(bitValue == 7)
						{
							bitArray++;
						}
					}
				}
			}

			//ENDADDNUMBERTOBITARRAY
			//printf("bitInByte %d  CurrentIndex %d \n", tempBitIndexInByte, currentIndex);
		}
	}

	// read bits and form number until bits per postition, then increment index

	gzclose(compressed_filep);

	if(buffer)
		free(buffer);

	if(currentByte)
		free(currentByte);

	return 0;
}

/****************************************************************************
* Description
*      readLevelFileType3 takes a pointer to a bitarray and a compressed file
*      and creates a bitarray starting at the files minHashValue until
*      its maxHashValue (0=not valid, 1 = valid
* Arguments
*      char* compressed_filename
*      BITARRAY *array
*      int length   (maxHashValue-minHashValue)
* Return Values
*      UINT8 status
****************************************************************************/
int readLevelFileType3(char* compressed_filename, BITARRAY *array, int length)
{
	int index=0;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	int status = STATUS_SUCCESS;
	compressed_filep = gzopen(compressed_filename, "rb");
	int endSectionCounter = 0;

	//skips header section which has one 0xC (at end of header) and all other ASCII chars
	while(status == STATUS_SUCCESS && endSectionCounter < 2)
	{
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
		if(*buffer == 0x0C)
		{
			endSectionCounter++;
		}
	}
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1); //newline
	//copies data into the bitarray
	while(status == STATUS_SUCCESS && index <= length)
	{
		status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
		*array = *buffer;
		index += 8;
		array++;
	}

	gzclose(compressed_filep);
	if(buffer)
		free(buffer);
	return 0;
}

/****************************************************************************
* Description
*      getType returns the levelfile type of the argument file
* Arguments
*      char* compressed_filename
* Return Values
*      UINT8 status
****************************************************************************/
int getLevelFileType(char* compressed_filename)
{
	int status;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	compressed_filep = gzopen(compressed_filename, "rb");
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	if((BYTE)(*buffer)!='1')
	{
		gzclose(compressed_filep);
		return -1; //check bit
	}
	else
	{
		while(status == STATUS_SUCCESS)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0x0A && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0x0A) {} //IGNORE EXTRA NEWLINES
			else
			{
				gzclose(compressed_filep);
				return (BYTE)(*buffer)-'0';
			}
		}
	}
	gzclose(compressed_filep);
	if(buffer)
		free(buffer);
	return -1;
}

/****************************************************************************
* Description
*      getMinHashValue returns the minHashValue of the file
* Arguments
*      char* compressed_filename
* Return Values
*      UINT8 status
****************************************************************************/
UINT64 getLevelFileMinHashValue(char* compressed_filename)
{
	int status;
	UINT64 minHashValue;
	int pastType = 0;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	compressed_filep = gzopen(compressed_filename, "rb");
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	if((BYTE)(*buffer)!='1')
	{
		gzclose(compressed_filep);
		return -1; //check bit
	}
	else
	{
		while(status == STATUS_SUCCESS && pastType ==0)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0x0A && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0xA) {} //IGNORE EXTRA NEWLINES
			else
			{
				pastType = 1;
			}
		}

		while(status == STATUS_SUCCESS && pastType ==1)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0x0A && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0x0A) {} //IGNORE EXTRA NEWLINES
			else
			{
				minHashValue = (BYTE)(*buffer)-'0';
				while(*buffer>='0' && *buffer <='9'&& status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
					if(*buffer>='0' && *buffer <='9'&& status == STATUS_SUCCESS)
					{
						minHashValue = minHashValue*10 + ((BYTE)(*buffer)-'0');
					}
				}

				gzclose(compressed_filep);
				return minHashValue;
			}
		}

	}
	gzclose(compressed_filep);
	if(buffer)
		free(buffer);
	return -1;
}
/****************************************************************************
* Description
*      getMaxHashValue returns the maxHashValue of the file
* Arguments
*      char* compressed_filename
* Return Values
*      UINT8 status
****************************************************************************/
UINT64 getLevelFileMaxHashValue(char* compressed_filename)
{
	int status;
	UINT64 maxHashValue;
	int pastMin = 0;
	int pastType = 0;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	compressed_filep = gzopen(compressed_filename, "rb");
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	if((BYTE)(*buffer)!='1')
	{
		gzclose(compressed_filep);
		return -1; //check bit
	}
	else
	{
		while(status == STATUS_SUCCESS && pastType ==0)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0xA && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0xA) {} //IGNORE EXTRA NEWLINES
			else
			{
				pastType = 1;
			}
		}
		while(status == STATUS_SUCCESS && pastType ==1)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0xA && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0xA) {} //IGNORE EXTRA NEWLINES
			else
			{

				while(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
					pastMin=1;
				}
				status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				maxHashValue = (BYTE)(*buffer)-'0';
				while(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS&& pastMin == 1)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
					if(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS)
					{
						maxHashValue = maxHashValue*10 + ((BYTE)(*buffer)-'0');
					}
				}
//                    printf("max = %d \n", maxHashValue);
				gzclose(compressed_filep);
				return maxHashValue;
			}
		}

	}
	gzclose(compressed_filep);
	if(buffer)
		free(buffer);
	return -1;

}
/****************************************************************************
* Description
*      getBitsPerPosition returns the bitsPerPosition of the file
* Arguments
*      char* compressed_filename
* Return Values
*      UINT8 status
****************************************************************************/
UINT64 getLevelFileBitsPerPosition(char* compressed_filename)
{
	UINT64 max = getLevelFileMaxHashValue(compressed_filename);
	UINT64 min = getLevelFileMinHashValue(compressed_filename);
	UINT64 maxValue = max - min;
	printf("maxvalue %llu %llu %llu", maxValue, max, min);
	return (log2(maxValue) + 1);
}

/****************************************************************************
* Description
*      getBitValue determines whether the bitnum bit in the byte currentByte
*      is a 0 or 1
* Arguments
*      BYTE* currentByte
*      UINT8 bitnum
* Return Values
*      UINT8 value
****************************************************************************/
UINT8 getBitValue(BYTE currentByte, UINT8 bitnum)
{
	UINT8 x = currentByte << bitnum;
	x = x >> (BITSINBYTE - 1);
	return x;
}

/****************************************************************************
* Description
*      findMinValueFromArray determines the minimum bitIndex of a one in
*      the array.
* Arguments
*      BITARRAY* array
*      UINT64 length
* Return Values
*      UINT64 minIndex
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
	return 0;
}
/****************************************************************************
* Description
*      findMaxValueFromArray determines the maximum bitIndex of a one in
*      the array.
* Arguments
*      BITARRAY* array
*      UINT64 length
* Return Values
*      UINT64 maxIndex
****************************************************************************/
UINT64 findMaxValueFromArray(BITARRAY* array, UINT64 length)
{
	UINT64 counter=0, lastVal = 0;
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
	printf("!!!!!!!!!!!!!!!! max %llu", lastVal);
	return lastVal;
}

/****************************************************************************
* Description
*      findMaxValueFromArray determines the maximum bitIndex of a one in
*      the array.
* Arguments
*      BITARRAY* array
*      UINT64 length
* Return Values
*      UINT64 maxIndex
****************************************************************************/
UINT64 findLastZero(BITARRAY* array, UINT64 length)
{
	UINT64 counter=0, lastVal=0;
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
			if(value ==0)
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
/****************************************************************************
* Description
*      getLastZero returns the findLastZero of the file
* Arguments
*      char* compressed_filename
* Return Values
*      UINT8 status
****************************************************************************/
int getLastZero(char* compressed_filename)
{
	int status;
	int lastZero=0;
	int pastMin = 0;
	int pastMax = 0;
	int pastType = 0;
	BYTE* buffer = (BYTE*)malloc(sizeof(BYTE));
	compressed_filep = gzopen(compressed_filename, "rb");
	status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
	if((BYTE)(*buffer)!='1')
	{
		gzclose(compressed_filep);
		return -1; //check bit
	}
	else
	{
		while(status == STATUS_SUCCESS && pastType ==0)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0xA && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0xA) {} //IGNORE EXTRA NEWLINES
			else
			{
				pastType = 1;
			}
		}
		while(status == STATUS_SUCCESS && pastType ==1)
		{
			status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
			if(*buffer == '#') //IGNORE COMMENTS TILL NEWLINE
			{
				while(*buffer!=0xA && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
			}
			else if(*buffer==0xA) {} //IGNORE EXTRA NEWLINES
			else
			{

				while(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
				if(*buffer == 0x20)
					pastMin=1;
				status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				while(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				}
				if(*buffer == 0x20)
					pastMax=1;
				status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
				lastZero = (BYTE)(*buffer)-'0';
				while(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS&& pastMax == 1)
				{
					status = bitlib_file_read_bytes(compressed_filep, buffer, 1);
					if(*buffer>='0' && *buffer <='9' && status == STATUS_SUCCESS)
					{
						lastZero = lastZero*10 + ((BYTE)(*buffer)-'0');
					}
				}
//                    printf("max = %d \n", maxHashValue);
				gzclose(compressed_filep);
				return lastZero;
			}
		}

	}
	gzclose(compressed_filep);
	if(buffer)
		free(buffer);
	return -1;
}
