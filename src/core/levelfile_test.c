/************************************************************************
**
** NAME:    levelfile_test.c
**
** DESCRIPTION:    Level File Generation and Access Functions test
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

int main(int argc, char **argv)
{
	//File stuff
	char compressedFileName[80] = "./test/game_opt_tier.dat.gz";
	char compressedFileName0[80] = "./test/game_opt_tier_0.dat.gz";
	char compressedFileName1[80] = "./test/game_opt_tier_1.dat.gz";
	char compressedFileName2[80] = "./test/game_opt_tier_2.dat.gz";
	char compressedFileName3[80] = "./test/game_opt_tier_3.dat.gz";

	char textFileName0[80]="./test/game_opt_tier_0.txt";
	char textFileName1[80]="./test/game_opt_tier_1.txt";
	char textFileName2[80]="./test/game_opt_tier_2.txt";
	char textFileName3[80]="./test/game_opt_tier_3.txt";
	gzFile* compressed_fp;
	FILE* text_fp;
////TO CREATE A FILENAME
//     char game[10] = sprintf("%s",gameName);
//	char opt[4] = sprintf("%d",optName);
//	char tier[6] = sprintf("%d",tierName);
//     fileName = game + "_" + opt + "_" + tier + ".dat.gz";
//// END TO CREATE A FILENAME
//File input
	BITARRAY* compressedValues = (BITARRAY*) malloc(sizeof(BITARRAY)*2);
	BITARRAY* buffer = (BITARRAY*)malloc(sizeof(BITARRAY)*1);
	POSITION startIndex, endIndex;
	GMSTATUS status = 0;

	//file read
	int numRead = 0;

	//array values
	*compressedValues = 0x4F;              //0100 1111   ->11 , 14, 15, 16, 17
	*(compressedValues+1)  = 0x00;         //0000 0000 -> 21 22 24 25
	startIndex = 10;
	endIndex = 30;
// Write all Types
	status = WriteLevelFile(compressedFileName, compressedValues, startIndex, endIndex);
	printf("Compressed All\n\n");
// Type 0 Text
	compressed_fp = gzopen(compressedFileName0, "rb");
	text_fp = fopen(textFileName0, "wb");

	//read from compressed file and write to text file
	while ((numRead = gzread(compressed_fp, buffer, sizeof(buffer))) > 0)
	{
//        printf("write");
		fwrite(buffer, 1, numRead, text_fp);
	}
	gzclose(compressed_fp);
	fclose(text_fp);
	printf("0: Decompressed\n");

// Type 0 Read
	int type = getLevelFileType(compressedFileName0);
	int minHashValue = getLevelFileMinHashValue(compressedFileName0);
	int maxHashValue = getLevelFileMaxHashValue(compressedFileName0);
	int bitsPerPosition = getLevelFileBitsPerPosition(compressedFileName0);
	printf("0: This is type %d and min value %d max value %d  bpp %d\n", type, minHashValue, maxHashValue, bitsPerPosition);
	BITARRAY* answer = (BYTE *) malloc(sizeof(BYTE) * (ceil((maxHashValue - minHashValue) / 8)+1));
	printf("0: Beginning Read\n");
	ReadLevelFile(compressedFileName0, answer, maxHashValue-minHashValue);
	printf("0: Read\n0: The bit string is : \t");
	bitlib_print_bytes_in_bits(answer, ceil((maxHashValue - minHashValue) / 8)+1);
	printf("0: Printed\n\n");

// Type 1 Text
	compressed_fp = gzopen(compressedFileName1, "rb");
	text_fp = fopen(textFileName1, "wb");

	//read from compressed file and write to text file
	while ((numRead = gzread(compressed_fp, buffer, sizeof(buffer))) > 0)
	{
//        printf("write");
		fwrite(buffer, 1, numRead, text_fp);
	}
	gzclose(compressed_fp);
	fclose(text_fp);
	printf("1: Decompressed\n");

// Type 1 Read
	type = getLevelFileType(compressedFileName1);
	minHashValue = getLevelFileMinHashValue(compressedFileName1);
	maxHashValue = getLevelFileMaxHashValue(compressedFileName1);
	bitsPerPosition = getLevelFileBitsPerPosition(compressedFileName1);
	printf("1: This is type %d and min value %d max value %d  bpp %d\n", type, minHashValue, maxHashValue, bitsPerPosition);
	answer = (BYTE *) malloc(sizeof(BYTE) * (ceil((maxHashValue - minHashValue) / 8)+1));

	ReadLevelFile(compressedFileName1, answer, maxHashValue-minHashValue);
	printf("1: Read\n1: The bit string is : \t");
	bitlib_print_bytes_in_bits(answer, ceil((maxHashValue - minHashValue) / 8)+1);
	printf("1: Printed\n\n");

// Type 2 Text
	compressed_fp = gzopen(compressedFileName2, "rb");
	text_fp = fopen(textFileName2, "wb");

	//read from compressed file and write to text file
	while ((numRead = gzread(compressed_fp, buffer, sizeof(buffer))) > 0)
	{
//        printf("write");
		fwrite(buffer, 1, numRead, text_fp);
	}
	gzclose(compressed_fp);
	fclose(text_fp);
	printf("2: Decompressed\n");

// Type 2 Read
	type = getLevelFileType(compressedFileName2);
	minHashValue = getLevelFileMinHashValue(compressedFileName2);
	maxHashValue = getLevelFileMaxHashValue(compressedFileName2);
	bitsPerPosition = getLevelFileBitsPerPosition(compressedFileName2);
	printf("2: This is type %d and min value %d max value %d  bpp %d\n", type, minHashValue, maxHashValue, bitsPerPosition);
	answer = (BYTE *) malloc(sizeof(BYTE) * (ceil((maxHashValue - minHashValue) / 8)+1));

	ReadLevelFile(compressedFileName2, answer, maxHashValue-minHashValue);
	printf("2: Read\n2: The bit string is : \t");
	bitlib_print_bytes_in_bits(answer, ceil((maxHashValue - minHashValue) / 8)+1);
	printf("2: Printed\n\n");

// Type 3 Text
	compressed_fp = gzopen(compressedFileName3, "rb");
	text_fp = fopen(textFileName3, "wb");

	//read from compressed file and write to text file
	while ((numRead = gzread(compressed_fp, buffer, sizeof(buffer))) > 0)
	{
//        printf("write");
		fwrite(buffer, 1, numRead, text_fp);
	}
	gzclose(compressed_fp);
	fclose(text_fp);
	printf("3: Decompressed\n");

// Type 3 Read
	type = getLevelFileType(compressedFileName3);
	minHashValue = getLevelFileMinHashValue(compressedFileName3);
	maxHashValue = getLevelFileMaxHashValue(compressedFileName3);
	bitsPerPosition = getLevelFileBitsPerPosition(compressedFileName3);
	printf("3: This is type %d and min value %d max value %d  bpp %d\n", type, minHashValue, maxHashValue, bitsPerPosition);
	answer = (BYTE *) malloc(sizeof(BYTE) * (ceil((maxHashValue - minHashValue) / 8)+1));

	ReadLevelFile(compressedFileName3, answer, maxHashValue-minHashValue);
	printf("3: Read\n3: The bit string is : \t");
	bitlib_print_bytes_in_bits(answer, ceil((maxHashValue - minHashValue) / 8)+1);
	printf("3: Printed\n");

	printf("These are valid %d, %d, %d, %d", isValidLevelFile(compressedFileName0), isValidLevelFile(compressedFileName1), isValidLevelFile(compressedFileName2), isValidLevelFile(compressedFileName3));


	if(answer)
		free(answer);
	return 0;
}
