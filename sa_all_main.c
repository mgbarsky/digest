#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SuffixArray.h"

int 
main(int argc, char *argv[])
{
	char inputFileNamePrefix [MAX_PATH_LENGTH];
	
	int result;

	int totalInputFiles;

	//need parameters for the following
	//int mergeSuffixArraysIntoSA_LCP (char * inputFilePrefix, int numFiles, uint64_t memoryForInputBuffersBytes, uint64_t outputBufferCapacityNumberOfElements)
	if(argc<3)
	{
		printf("./sortsasimple <inputfilenamefullpathprefix> <total_files> \n");
		return 1;
	}
	
	

	sprintf(inputFileNamePrefix,"%s", argv[1]);
	totalInputFiles = atoi(argv[2]);
	
	
	result = buildSuffixArraysForAllFiles(inputFileNamePrefix,  totalInputFiles);
	if(result!=0)
		return 1;
	return 0;
	
}
