#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SuffixArray.h"

int 
main(int argc, char *argv[])
{
	char inputFileNamePrefix [MAX_PATH_LENGTH];
	
	uint32_t availmemoryInputBuffersBytes;
	uint32_t maxOutputBufferElements;
	
	int result;

	int totalInputFiles;

	//need parameters for the following
	//int mergeSuffixArraysIntoSA_LCP (char * inputFilePrefix, int numFiles, uint64_t memoryForInputBuffersBytes, uint64_t outputBufferCapacityNumberOfElements)
	if(argc<5)
	{
		printf("./buildandmerge <inputfilenamefullpathprefix> <total_files> <memory_bytes_for_inputbuffers>  <max_elements_output_buffer>\n");
		return 1;
	}
	
	

	sprintf(inputFileNamePrefix,"%s", argv[1]);
	totalInputFiles = atoi(argv[2]);
	availmemoryInputBuffersBytes = atoi(argv[3]);
	maxOutputBufferElements = atoi(argv[3]);
	
	
	result = buildSuffixArraysForAllFiles(inputFileNamePrefix, totalInputFiles);
	if(result!=0)
		return 1;
	return mergeSuffixArraysIntoSA_LCP (inputFileNamePrefix, totalInputFiles, 
			availmemoryInputBuffersBytes, maxOutputBufferElements);	
}
