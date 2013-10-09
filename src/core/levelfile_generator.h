/************************************************************************
**
** NAME:    levelfile_generator.h
**
** DESCRIPTION:    level file generation utility
**
** AUTHOR:    Deepa Mahajan
**        GamesCrafters Research Group, UC Berkeley
**        Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:    2006-08-07
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

#ifndef GMCORE_LEVELFILE_GENERATOR_H
#define GMCORE_LEVELFILE_GENERATOR_H

#include "gamesman.h"
#include "levelfile_generator.h"
#include "bpdb_bitlib.h"
#include <sys/stat.h>
#include <math.h>

#define BITSINBYTE 8
#define BITSINPOS 64
#define NUMOFTYPES 4
#define LENGTHOFEXT 7 //.dat.gz
// gcc levelfile_generator.c levelfile_generator.h bpdb_bitlib.h bpdb_bitlib.c levelfile_test.c -lZ
// hexedit to view binary characters
typedef unsigned char BITARRAY;


int WriteLevelFile(char* compressed_filename, BITARRAY *array, POSITION minHashValue, POSITION maxHashValue);
int writeHeader(gzFile file, UINT64 minHashValue, UINT64 maxHashValue, UINT64 lastZero, int type);
int ArrayToType0Write(BITARRAY *array, UINT64 minHashValue, UINT64 maxHashValue);
int ArrayToType1Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue, UINT64 bitsPerPosition, UINT64 offset);
int ArrayToType2Write(BITARRAY *array, UINT64 startIndex, UINT64 maxHashValue, UINT64 bitsPerPosition, UINT64 offset);
int ArrayToType3Write(BITARRAY *array, UINT64 startIndex, UINT64 minHashValue, UINT64 maxHashValue);

int ReadLevelFile(char* compressed_filename, BITARRAY *array, int length);
int getLevelFileType(char* compressed_filename);
UINT64 getLevelFileMinHashValue(char* compressed_filename);
UINT64 getLevelFileMaxHashValue(char* compressed_filename);
UINT64 getLevelFileBitsPerPosition(char* compressed_filename);
int isValidLevelFile(char* compressed_filename);
int readLevelFileType0(char* compressed_filename, BITARRAY *array, int length);
int readLevelFileType1(char* compressed_filename, BITARRAY *bitArray, int length);
int readLevelFileType2(char* compressed_filename, BITARRAY *bitArray, int length);
int readLevelFileType3(char* compressed_filename, BITARRAY *array, int length);
UINT8 getBitValue(BYTE currentByte, UINT8 bitnum);
UINT64 findMinValueFromArray(BITARRAY* array, UINT64 length);
UINT64 findMaxValueFromArray(BITARRAY* array, UINT64 length);
UINT64 findLastZero(BITARRAY* array, UINT64 length);
int positionToByteArray(BITARRAY* buffer, UINT64 value, BYTE bufferPrev);
int getLastZero(char* compressed_filename);
#endif /* GMCORE_LEVELFILEGENERATOR_H */
