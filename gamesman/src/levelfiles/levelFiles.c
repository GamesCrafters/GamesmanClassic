#include <stdio.h>
#include <stdlib.h>
FILE* type0Totype1Converter(FILE* type0, FILE* type1);
FILE* type0Totype2Converter(FILE* type0, FILE* type2);
FILE* type0Totype3Converter(FILE* type0, FILE* type3);

int asciiToBinary(int asciiNum);
int binaryLog2(int asciiNum);
int asciiLog2(int asciiNum);
void formatHashValue(int value, char* formattedString, int bitsPerPosition);

int main(){
	FILE* type0;
	FILE* type1;
	FILE* type2;
	FILE* type3;
	type0 = fopen("type0-0-100.txt", "r");
	type1 = fopen("type1-0-100.txt", "wt");
	type2 = fopen("type2-0-100.txt", "wt");
	type3 = fopen("type3-0-100.txt", "wt");
	fseek(type0, 0, 0);
	fseek(type1, 0, 0);
	type1 = type0Totype1Converter(type0, type1);

	fseek(type0, 0, 0);
	fseek(type2, 0, 0);
	type2 = type0Totype2Converter(type0, type2);

	fseek(type0, 0, 0);
	fseek(type3, 0, 0);
	type3 = type0Totype3Converter(type0, type3);
	fclose(type0);
	fclose(type1);
	fclose(type2);
	fclose(type3);
	return 0;
}
/*
Reads in a file pointer to a level file in type 1 format and returns a pointer to a file in type 1 format. 
Type 0:
0, minHashValue, maxHashValue
Ascii positions separated by spaces
Type 1:
1, minHashValue, maxHashValue, bitsPerPosition, offset
positions in binary but not separated w/ spaces. Leading zeros for 
each position so each one is bitsPerPosition long.
*/
FILE* type0Totype1Converter(FILE* type0, FILE* type1){
	int type=0, minHashValue=0, maxHashValue=0, currentHashValue=0, globalOffset=0, bitsPerPosition=0;
	char currentHashValueFormatted[64];
	char ch, tempchar;
	while(((ch = getc(type0)) != EOF) && (ch != ',')){
		type = type * 10 + (ch - '0');
	}
	if(type != 0){
		printf("Incorrect input file type %d", type);
		return type1;
	}//check type
	while(((ch = getc(type0)) != EOF) && (ch != ','))
	{
		if((ch >= '0') && (ch <= '9')){
			minHashValue = minHashValue*10 + (ch - '0');
		}
	}//set minHashValue
	while(((ch = getc(type0)) != EOF) && (ch != ',') && (ch != '\n') && (ch != '\r')){
		if((ch >= '0') && (ch <= '9')){
			maxHashValue = maxHashValue*10 + (ch - '0');
		}
	}//set maxHashValue
	while(((ch = getc(type0)) != EOF) && (ch != '\n') && (ch != '\r')){
	}// wait for positions to begin

	// type 1 formatting
	fprintf(type1, "1,%d,%d,", minHashValue, maxHashValue);
	ch = getc(type0);
	currentHashValue = 0;
	while((ch != EOF) && (ch >= '0') && (ch <= '9'))
	{	    
		currentHashValue = 10 * currentHashValue + (ch - '0');
		ch = getc(type0);
	}
	globalOffset = currentHashValue;
	while(ch !=EOF)
	{
		if(ch>= '0' && ch <= '9')
		{
			currentHashValue = currentHashValue * 10 + (ch - '0');
		}
		else
		{
			currentHashValue = 0;
		}
		//	  ungetc(ch,type0);
		ch = getc(type0);
		if(ch == EOF){
			bitsPerPosition = asciiLog2(currentHashValue);
		}
	}

	fprintf(type1,"%d,%d\n", bitsPerPosition, globalOffset);
	if(fseek(type0, 0, SEEK_SET) == 0)
	{
		ch = getc(type0);
		while(ch != EOF && ch != '\n' && ch != '\r')
		{
			ch = getc(type0);
		}
		ch = getc(type0);
		while(ch != EOF)
		{
			if(ch >= '0' && ch <= '9')
			{
				currentHashValue = currentHashValue *10 + (ch - '0');

			}
			else if(ch == ' ')
			{
				currentHashValue = asciiToBinary(currentHashValue-globalOffset);
				formatHashValue(currentHashValue, currentHashValueFormatted, bitsPerPosition);
				fprintf(type1,"%s", currentHashValueFormatted);
				currentHashValue = 0;
			}
			else
			{
				currentHashValue = 0;
			}

			ch = getc(type0);
		}
		if(currentHashValue !=0)
		{
			currentHashValue = asciiToBinary(currentHashValue-globalOffset);
			formatHashValue(currentHashValue, currentHashValueFormatted, bitsPerPosition);
			fprintf(type1,"%s", currentHashValueFormatted);
			currentHashValue = 0;
		}

	}
	return type1;
}
/*Reads in a file pointer to a level file in type 2 format and returns a pointer to a file in type 2 format. 
Type 0:
0, minHashValue, maxHashValue
Ascii positions separated by spaces
Type 1:
2, minHashValue, maxHashValue, bitsPerPosition, offset
all positions from the offset onwards that are not contained in the level type0 file. The positions are
stored in binary but not separated w/ spaces. Leading zeros for 
each position so each one is bitsPerPosition long.
*/

FILE* type0Totype2Converter(FILE* type0, FILE* type2){
	int type=0, minHashValue=0, maxHashValue=0, currentHashValue=0, currentBinaryValue=0, globalOffset=-1, bitsPerPosition=0, value=0, minHashTemp = 0;
	char currentHashValueFormatted[64];
	char ch, tempchar;
	while(((ch = getc(type0)) != EOF) && (ch != ',')){
		type = type * 10 + (ch - '0');
	}
	if(type != 0){
		printf("Incorrect input file type %d", type);
		return type2;
	}//check type
	while(((ch = getc(type0)) != EOF) && (ch != ','))
	{
		if((ch >= '0') && (ch <= '9')){
			minHashValue = minHashValue*10 + (ch - '0');
		}
	}//set minHashValue
	while(((ch = getc(type0)) != EOF) && (ch != ',') && (ch != '\n') && (ch != '\r')){
		if((ch >= '0') && (ch <= '9')){
			maxHashValue = maxHashValue*10 + (ch - '0');
		}
	}//set maxHashValue
	while(((ch = getc(type0)) != EOF) && (ch != '\n') && (ch != '\r')){
	}// wait for positions to begin

	// type 1 formatting
	fprintf(type2, "2,%d,%d,", minHashValue, maxHashValue);

	ch = getc(type0);
	currentHashValue = 0;
	minHashTemp = minHashValue;
	while(ch != EOF && globalOffset == -1)
	{
		if(ch >= '0' && ch <= '9')
		{
			currentHashValue = currentHashValue *10 + (ch - '0');

		}
		else if(ch == ' ')
		{
			if(currentHashValue > minHashTemp){
				globalOffset = minHashTemp;
			}
			else{
				minHashTemp +=1;
			}
			currentHashValue = 0;
		}
		else
		{
			currentHashValue = 0;
		}

		ch = getc(type0);
	}
	bitsPerPosition = asciiLog2(maxHashValue);

	fprintf(type2,"%d,%d\n", bitsPerPosition, globalOffset);
	if(fseek(type0, 0, SEEK_SET) == 0)
	{
		ch = getc(type0);
		while(ch != EOF && ch != '\n' && ch != '\r')
		{
			ch = getc(type0);
		}
		ch = getc(type0);
		while(ch != EOF)
		{
			if(ch >= '0' && ch <= '9')
			{
				currentHashValue = currentHashValue *10 + (ch - '0');

			}
			else if(ch == ' ')
			{
				for(value = minHashValue; value < currentHashValue && minHashValue >= globalOffset; value++){
					currentBinaryValue = asciiToBinary(value-globalOffset);
					formatHashValue(currentBinaryValue, currentHashValueFormatted, bitsPerPosition);
					fprintf(type2,"%s", currentHashValueFormatted);
				}

				minHashValue = currentHashValue + 1;
				currentHashValue = 0;
			}
			else
			{
				currentHashValue = 0;
			}

			ch = getc(type0);
		}
		if(currentHashValue != maxHashValue)
		{
			currentHashValue += 1;
			minHashValue = currentHashValue;

			for(value = minHashValue; value <= maxHashValue; value++){
					currentBinaryValue = asciiToBinary(value-globalOffset);
					formatHashValue(currentBinaryValue, currentHashValueFormatted, bitsPerPosition);
					fprintf(type2,"%s", currentHashValueFormatted);
			}

		}
	}

return type2;
}

/*Reads in a file pointer to a level file in type 2 format and returns a pointer to a file in type 3 format. 
Type 0:
0, minHashValue, maxHashValue
Ascii positions separated by spaces
Type 3:
3, minHashValue, maxHashValue, offset
starts from globalOffset and puts a 0 if that number is not there in type0 and a 1 if it is there
*/

FILE* type0Totype3Converter(FILE* type0, FILE* type3){
	int type=0, minHashValue=0, maxHashValue=0, currentHashValue=0, currentBinaryValue=0, globalOffset=-1, bitsPerPosition=0, value=0;
	char currentHashValueFormatted[64];
	char ch, tempchar;
	while(((ch = getc(type0)) != EOF) && (ch != ',')){
		type = type * 10 + (ch - '0');
	}
	if(type != 0){
		printf("Incorrect input file type %d", type);
		return type3;
	}//check type
	while(((ch = getc(type0)) != EOF) && (ch != ','))
	{
		if((ch >= '0') && (ch <= '9')){
			minHashValue = minHashValue*10 + (ch - '0');
		}
	}//set minHashValue
	while(((ch = getc(type0)) != EOF) && (ch != ',') && (ch != '\n') && (ch != '\r')){
		if((ch >= '0') && (ch <= '9')){
			maxHashValue = maxHashValue*10 + (ch - '0');
		}
	}//set maxHashValue
	while(((ch = getc(type0)) != EOF) && (ch != '\n') && (ch != '\r')){
	}// wait for positions to begin

	// type 1 formatting
	fprintf(type3, "3,%d,%d,", minHashValue, maxHashValue);

	ch = getc(type0);
	currentHashValue = 0;
	while((ch != EOF) && (ch >= '0') && (ch <= '9'))
	{	    
		currentHashValue = 10 * currentHashValue + (ch - '0');
		ch = getc(type0);
	}
	globalOffset = currentHashValue;
	fprintf(type3,"%d\n", globalOffset);
	if(fseek(type0, 0, SEEK_SET) == 0)
	{
		ch = getc(type0);
		while(ch != EOF && ch != '\n' && ch != '\r')
		{
			ch = getc(type0);
		}
		ch = getc(type0);
		while(ch != EOF)
		{
			if(ch >= '0' && ch <= '9')
			{
				currentHashValue = currentHashValue *10 + (ch - '0');

			}
			else if(ch == ' ')
			{
				for(value = minHashValue; value < currentHashValue && minHashValue >= globalOffset; value++){
					fprintf(type3,"0");
				}
				fprintf(type3,"1");
				minHashValue = currentHashValue + 1;
				currentHashValue = 0;
			}
			else
			{
				currentHashValue = 0;
			}

			ch = getc(type0);
		}
		for(value = minHashValue; value < currentHashValue; value++){
					fprintf(type3,"0");
			}
		if(currentHashValue != maxHashValue)
		{
			currentHashValue += 1;
			minHashValue = currentHashValue;
			fprintf(type3,"1");
			for(value = minHashValue; value <= maxHashValue; value++){
					fprintf(type3,"0");
			}

		}
	}

return type3;
}

int asciiLog2(int asciiNum)
{
	int i = 0, val = 1;
	if(asciiNum < 0){
		return 0;
	}
	else if(asciiNum == 0)
	{
		return 1;
	}
	for(i=0; val<=asciiNum; i++)
	{
		val *=2;
	}
	return i;
}
int binaryLog2(int asciiNum)
{
	int i = 0, val = 1;
	if(asciiNum < 0){
		return 0;
	}
	else if(asciiNum == 0)
	{
		return 1;
	}
	for(i=0; asciiNum > 0; i++)
	{
		asciiNum /= 10;
	}
	return i;
}

int asciiToBinary(int asciiNum)
{
	int binary=1, temp = asciiNum, answer = 0;
	if(asciiNum <= 0)
	{
		return 0;
	}
	for(binary = 1; binary <= asciiNum; binary*=2)
	{
	}
	for(temp = asciiNum; binary > 0;)
	{
		if((temp - binary) < 0)
		{
			answer *=10;
			binary /=2;
		}
		else
		{
			temp = temp - binary;
			binary /=2;
			answer = answer * 10 + 1;
		}
	}
	return answer;

}

void formatHashValue(int value, char formattedString[], int bitsPerPosition)
{
	int length, i;
	length = binaryLog2(value);
	for(i=0; i<bitsPerPosition - length; i++)
	{
		formattedString[i]='0';
	}
	for(i=bitsPerPosition-1; i > bitsPerPosition - length -1; i--)
	{
		formattedString[i] = (value % 2) + '0';
		value = value /10;
	}
	formattedString[bitsPerPosition] = '\0';
}
