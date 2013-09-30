#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "SuffixArray.h"
int 
buildSuffixArraysForAllFiles(char *inputFileNamePrefix, int totalInputFiles)
{	
	char currentInputFileName [MAX_PATH_LENGTH];
	char currentOutputFileName [MAX_PATH_LENGTH];	
	
	int subscriptFrom = 0;
	
	unsigned char maxLineLenAllFiles=0;
		
	SABuildManager manager;	
	clock_t startclock, stopclock;
	double time_spent;
	FILE * inputFP;
	FILE * outputFP;
	int i;
	int result;
	uint64_t totalCharsMaxFile;
	uint32_t maxNumLines=0;
	uint32_t maxNumLinesAllFiles=0;
	unsigned char maxLineLen=0;
	
	
//***************timing total execution - timing starts ****************
startclock = clock();
	
	//we scan all input files and collect input statistics in order to allocate all necessary arrays only once
	for(i=subscriptFrom; i< totalInputFiles; i++)
	{
		//create input file name, based on prefix and subscript
		sprintf(currentInputFileName,"%s%d", inputFileNamePrefix,i);
		//open file for reading
		if(!(inputFP= fopen ( currentInputFileName , "r" )))
		{
			printf("Could not open file \"%s\" for reading input lines \n", currentInputFileName);
			return (1);
		}		
		
		maxLineLen = 0;
		maxNumLines = 0;
		collectOneInputStats (inputFP, &maxNumLines, &maxLineLen );
		
		if(maxLineLen>MAX_CHARS_PER_SUFFIX)
		{
			printf("Unable to process file %s with lines longer than %d DNA characters.\n", currentInputFileName,MAX_CHARS_PER_SUFFIX);
			printf("Need to redefine prefix buffers in SuffixArray.h and the comparison routine.\n");
			fclose(inputFP);
			return 1;
		}

		if(maxLineLen>maxLineLenAllFiles)
			maxLineLenAllFiles=maxLineLen;
		if(maxNumLines>maxNumLinesAllFiles)
			maxNumLinesAllFiles=maxNumLines;

		fclose(inputFP);
	}

	totalCharsMaxFile = maxNumLinesAllFiles*(maxLineLenAllFiles+1);	
	printf("------Max number of lines per file for all inputs is %u and max line length is %u\n",maxNumLinesAllFiles,maxLineLenAllFiles);
	printf("------Allocating memory based on the max concatenated string: %lu bytes\n", totalCharsMaxFile);

	//allocating arrays for the largest possible memory required	
	//1. we can allocate memory once for all for an output buffer - it has a constant size
	if(!(manager.prefixedSuffixesBuffer =(SuffixToMerge *)calloc(NUM_SUFFIXES_IN_OUTPUT_BUFFER,sizeof (SuffixToMerge))))
	{
		printf("Unable to allocate memory for the output buffer.\n");
		return 1;
	}
	
	//2. allocate char buffer to hold input string where each line is added with termination char '?' 	
	if(!(manager.inputAsChars =(unsigned char *)calloc(totalCharsMaxFile+1,sizeof (unsigned char))))
	{
		printf("Unable to allocate memory for concatenated input string.\n");
		return 1;
	}

	//3. allocate memory to hold resulting suffix array 
	if(!(manager.intSA =(uint32_t *)calloc(totalCharsMaxFile+1,sizeof (uint32_t))))
	{
		printf("Unable to allocate memory to store SA integer array.\n");
		return 1;
	}

	//memory allocated, now we process each file in turn into a suffix array with bwt, prefix, suffixlen
	//looping through the input files, creating suffix arrays, and outputing the results (prefixed and with BWT) 
	//to a disk file - for subsequent merge
	for(i=subscriptFrom; i< totalInputFiles; i++)
	{
		manager.currentFileID = i;
		//create input file name, based on prefix and subscript
		sprintf(currentInputFileName,"%s%d", inputFileNamePrefix,i);
		//open file for reading
		if(!(inputFP= fopen ( currentInputFileName , "r" )))
		{
			printf("Could not open file \"%s\" for reading input lines \n", currentInputFileName);
			return (1);
		}
		
		sprintf(currentOutputFileName,"%s_SA", currentInputFileName);
		//open file for writing suffix array
		if(!(outputFP= fopen ( currentOutputFileName , "wb" )))
		{
			printf("Could not open file \"%s\" for writing suffix array \n", currentOutputFileName);
			return (1);
		}
		
		printf("\n ^^^^^^^^ Started processing file \"%s\" into suffix array \"%s\" \n", currentInputFileName, currentOutputFileName);

		//pass to another file for processing a single file
		result = buildSuffixArrayForSingleFile (&manager, inputFP, outputFP );		
		
		//close file pointers
		fclose(inputFP);
		fclose(outputFP);

		if(result!=0)
		{
			printf("Processing of file \"%s\" into suffix array failed \n", currentInputFileName);
			return result;
		}
		else
		{
			printf("Processed file \"%s\" into suffix array \"%s\" ^^^^^^^^^ \n", currentInputFileName, currentOutputFileName);
		}
	}	
	

	freeMemoryAfterSAForAll(&manager);

////***********timing total execution - timing ends *****************
stopclock = clock();
	time_spent = (double)(stopclock - startclock) / CLOCKS_PER_SEC;
	printf("\n Total %f seconds for sorting suffixes in all files \n",time_spent);	
	return 0;
}

int 
freeMemoryAfterSAForAll(SABuildManager* manager)
{
	if(manager->prefixedSuffixesBuffer!=NULL)
	{
		free(manager->prefixedSuffixesBuffer);
		manager->prefixedSuffixesBuffer = NULL;
	}
	if(manager->inputAsChars!=NULL)
	{
		free(manager->inputAsChars);
		manager->inputAsChars = NULL;
	}
	
	if(manager->intSA!=NULL)
	{
		free(manager->intSA);
		manager->intSA = NULL;
	}
	return 0;
}
int 
allocateArraysForLarsson(SABuildManager *manager, int seqlen)
{
	return 0;
}

int 
allocateArraysForMori(SABuildManager *manager, int seqlen)
{
	return 0;
}
