#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "SuffixArray.h"



int 
buildSuffixArrayForSingleFile (SABuildManager *manager, FILE *inputFP, FILE *outputFP)
{
	char current_line[MAX_CHARS_PER_LINE];
	unsigned char currLen;
	clock_t begin, end;
	double time_spent;
	int currentPosInConcatInput = 0;
	
begin = clock();

	//we fill in the input as one concatenation of all lines, and we add termination char '?' at the end of each line
	while( fgets (current_line, MAX_CHARS_PER_LINE-10, inputFP)!=NULL ) 
	{
		currLen = strlen(current_line);		
		if(currLen>3)  //avoid empty lines
		{
			int k=0;
			int endOfLine = 0;
			for(k=0;k<currLen && !endOfLine;k++)
			{
				if(current_line[k]==10 ||current_line[k]==32 ||current_line[k]=='\0') //padding
				{
					endOfLine =1;
					manager->inputAsChars[currentPosInConcatInput++]='?';  //end of line					
				}
				else
					manager->inputAsChars[currentPosInConcatInput++]=current_line[k];
			}
			if(!endOfLine)
				manager->inputAsChars[currentPosInConcatInput++]='?';
		}	
	}
	
	//finally-finally, putting string termination char for the entire total string
	manager->inputAsChars[currentPosInConcatInput]='\0';
	
	manager->totalCharacters = currentPosInConcatInput;
	
	//now we create a suffix array for this input file	
	if(create_suffix_array_mori(manager)!=0)		
		return 1;	

	if(LCP_TEST_MODE)
		printf("%s\n",manager->inputAsChars);
	//now we iterate over suffix array, attach prefix to a valid suffix, count equal suffixes, write them into an output buffer, when it is full - append to an output file
	if(recordOrderedSuffixesToFile(manager,outputFP)!=0)
		return 1;

end = clock();
time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
printf("In memory suffix sorting took %f seconds \n",time_spent);	
	return 0;
}

int collectOneInputStats (FILE *inputFP, uint32_t *maxNumberOfLines, unsigned char *maxLinelength )
{
	uint32_t numLines=0;
	unsigned char maxLine = 0;

	char current_line[MAX_CHARS_PER_LINE];
	//reading lines
	while( fgets (current_line, MAX_CHARS_PER_LINE-10, inputFP)!=NULL ) 
	{
		int lineLength = strlen(current_line);
		if(lineLength>maxLine)
			maxLine=lineLength;
		numLines++;			
	}
	*maxNumberOfLines = numLines;
	*maxLinelength = maxLine;
	return 0;
}

int getSuffixLengthWithEncoding(int suffixStart, unsigned char *sequence, int seqLen, uint64_t *bitArray)
{
	int suffixLength=0;
	int endOfLine =0;
	int i;
	int currentLongID=0;
	int currentBit=0;
	bitArray[currentLongID] = 0LL;

	for(i=suffixStart; i< seqLen && !endOfLine;i++)
	{
		unsigned char currentChar = sequence[i];
		suffixLength++;
		switch( currentChar ) 
		{
		    case 'a': //00				
			break;
		    case 'c': //01
			setBit64(&(bitArray[currentLongID]), currentBit+1);
			break;
		    case 'g': //10
			setBit64(&(bitArray[currentLongID]), currentBit);
			break;
		    case 't': //11
			setBit64(&(bitArray[currentLongID]), currentBit);
			setBit64(&(bitArray[currentLongID]), currentBit+1);
			break;
		    default :		
			endOfLine=1;
			break;
		}
		if(currentBit+2>=64) //need next long
		{
			currentBit=0;
			currentLongID++;
			bitArray[currentLongID] = 0LL;
		}
		else
		{
			currentBit+=2; //stay in the same long
		}	
	}
	return suffixLength;
}

void printSuffixWithBinaryEncoding(SuffixToMerge* suffix, int startPos)
{
	char chars64[65];
	int i;
	
	printf("Suffix starts:%d, length:%d, bwt:%c\n", startPos, 
		suffix->header.info.seqLength, suffix->header.info.bwtChar );
	for(i=0; i<2;i++)
	{
		bits64AsString (&(suffix->data[i]), chars64);
		printf("%s\n",chars64);
	}

}

int recordOrderedSuffixesToFile(SABuildManager *manager, FILE *outputFP)
{
	int posInSA; //iterator over suffix array
	int posInOutputSuffixBuffer = 0;
	int p;//,k; //iterators over 4 longs in the array of long binary encodings of each SuffixToMerge
	//char strOfBits[65]; //TBR - for debugging
	
	int totalRecordedSuffixes=0;
	uint64_t currentEncoding[4];	
	int addNewsuffix;
	int currentSuffixStart,suffixLength,numberOfLongs;
	unsigned char bwtChar;

	for(posInSA=0;posInSA<manager->totalCharacters;posInSA++)
	{
		if(posInOutputSuffixBuffer==NUM_SUFFIXES_IN_OUTPUT_BUFFER-1) //buffer is full
		{			
			int written=fwrite(manager->prefixedSuffixesBuffer, sizeof (SuffixToMerge), posInOutputSuffixBuffer-1, outputFP);
			if(written!=posInOutputSuffixBuffer-1)
			{
				printf("Not all suffix array to merge has been written\n");
				return 1;
			}
			totalRecordedSuffixes+=(posInOutputSuffixBuffer-1);
			//transfer last element to the 0-position
			manager->prefixedSuffixesBuffer[0] = manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer-2];
			//reset buffer counter
			posInOutputSuffixBuffer=1;	
		}
		
		currentSuffixStart = manager->intSA[posInSA];
		suffixLength = getSuffixLengthWithEncoding(currentSuffixStart, &(manager->inputAsChars[0]),manager->totalCharacters, &currentEncoding[0]);
		if(suffixLength == 0)
		{
			printf("UNEXPECTED ERROR while extracting binary encoding\n");
			return 1;
		}
		numberOfLongs = (2*suffixLength)/64;
		if((2*suffixLength)%64 >0)
			numberOfLongs++;
		
		addNewsuffix=0;
		if(posInSA>0)
		{
			int compResult = compareBinaryPrefix(&(manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer-1].data[0]), 
					&(currentEncoding[0]), 
					manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer-1].header.info.seqLength, 
					suffixLength );
			if(compResult>0)
			{
				printf("Invalid suffix order\n");
				return 1;
			}
			if(compResult == 0)  //dont add - update count- only if bwtChars are the same, otherwise we loose information
			{
				bwtChar = (currentSuffixStart==0)?'?':manager->inputAsChars[currentSuffixStart-1];
				if(bwtChar==manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer-1].header.info.bwtChar)
					manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer-1].header.info.count++;
				else
					addNewsuffix=1;
			}
			else
				addNewsuffix=1;
		}
		else
			addNewsuffix=1;
		if(addNewsuffix)
		{
			manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer].header.info.seqLength = suffixLength;  //IN CHARS
				
			//record character preceeding current suffix - BWT
			manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer].header.info.bwtChar = (currentSuffixStart==0)?'?':manager->inputAsChars[currentSuffixStart-1];
			manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer].header.info.fileID = manager->currentFileID;
			manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer].header.info.count =1;

			//transfer attached encoding
			for(p=0; p< numberOfLongs;p++)
				manager->prefixedSuffixesBuffer[posInOutputSuffixBuffer].data[p]= currentEncoding[p];
			
			posInOutputSuffixBuffer++;
		}
	}

	//if something remaining in a non-full output buffer, flush to disk
	if(posInOutputSuffixBuffer>0) //buffer is full
	{
		int written=fwrite(manager->prefixedSuffixesBuffer, sizeof (SuffixToMerge), posInOutputSuffixBuffer, outputFP);
		if(written!=posInOutputSuffixBuffer)
		{
			printf("Not all final chunk of suffix array to merge has been written\n");
			return 1;
		}
		totalRecordedSuffixes+=posInOutputSuffixBuffer;			
	}	
	return 0;
}



