#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SuffixArray.h"

int 
main(int argc, char *argv[])
{
	char inputFileNamePrefix [MAX_PATH_LENGTH];
	int numFiles;
	uint32_t availmemoryInputBuffersBytes;
	uint32_t maxOutputBufferElements;
	
	if(argc<5)
	{
		printf("./mergesa <inputfilenamefullpathprefix> <total_files> <memory_bytes_for_inputbuffers>  <max_elements_output_buffer>\n");
		return 1;
	}	

	sprintf(inputFileNamePrefix,"%s", argv[1]);
	numFiles = atoi(argv[2]);
	availmemoryInputBuffersBytes = atoi(argv[3]);
	maxOutputBufferElements = atoi(argv[4]);

	return mergeSuffixArraysIntoSA_LCP (inputFileNamePrefix, numFiles, 
			availmemoryInputBuffersBytes, maxOutputBufferElements);	
}
