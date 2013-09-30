#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "SuffixArray.h"

int count=0;
int compare(HEAP_ELEMENT_TYPE* first, HEAP_ELEMENT_TYPE* second)
{
	return compareBinaryPrefix(&(first->data[0]), &(second->data[0]), first->header.info.seqLength, second->header.info.seqLength );
}

//some helper for debugging
void printMergedSuffix(SA_LCP_fileID *suffix, uint64_t *seq )
{
	char str64[65];
	bits64AsString (seq, str64);
	printf("%s\n",str64);
	printf("%d. Suffix length:%d lcpinbits:%d firstbitafterlcp:%d \n\n",count++,suffix->suffixLength, 
			suffix->lcpInBits, suffix->firstBitAfterLCP);
}

int convertHeapElementToOutputElement (EMMergeManager *merger, SuffixToMerge *inputElem, SA_LCP_fileID *outputElem)
{
	char firstBitAfterLCP = 1; //this is the most general case - the bit is 1 
	outputElem->suffixLength = inputElem->header.info.seqLength;
	outputElem->bwtChar = inputElem->header.info.bwtChar;
	outputElem->fileID = inputElem->header.info.fileID;
	outputElem->count = inputElem->header.info.count;
	
	if(merger->lastTransferred.header.info.bwtChar == 'n') //this happens only once - in the beginning of the processing
	{
		printf("FIRST ELEMENT\n");
		outputElem->lcpInBits=255;
		outputElem->firstBitAfterLCP=2;
	}
	else  //there is always a previous element - in 'merger->lastTransferred'
	{
		outputElem->lcpInBits = getLCPInBits(&(merger->lastTransferred.data[0]),&(inputElem->data[0]),
					merger->lastTransferred.header.info.seqLength, inputElem-> header.info.seqLength, &firstBitAfterLCP);
		//set bits
		outputElem->firstBitAfterLCP = firstBitAfterLCP;
		
	}

	if(LCP_TEST_MODE)
		printMergedSuffix(outputElem, &(inputElem->data[0]) );
	
	return 0;
}

uint32_t getFileID(HEAP_ELEMENT_TYPE *record)
{
	return record->header.info.fileID;
}

int mergeSuffixArraysIntoSA_LCP (char * inputFilePrefix, int numFiles, uint64_t memoryForInputBuffersBytes, uint64_t outputBufferCapacityNumberOfElements)  //initialize the merger
{
	char currentOutputFileName [MAX_PATH_LENGTH];
	char currentInputFileName [MAX_PATH_LENGTH];
	int i;
	EMMergeManager merger;
	uint64_t memoryPerFile;
	clock_t startclock, stopclock;
	double time_spent;

//***************timing total execution - timing starts ****************
startclock = clock();
	//init merger fields
	sprintf( merger.inputFileNamePrefix,"%s", inputFilePrefix);
	merger.lastTransferred.header.info.bwtChar = 'n';  //sign there is no lastTransferred yet
	
	memoryPerFile = memoryForInputBuffersBytes/numFiles;
	

	merger.inputBufferCapacity = memoryPerFile/sizeof(INPUT_TYPE); //tbd make it multiples of disk blocks
	printf("\n-----Started merge\n");
	printf("MEMORY REQUIRED: There are %lu bytes per each of total %d runs available for buffering which can hold %u input elements\n",memoryPerFile,numFiles,merger.inputBufferCapacity);
	merger.outputBufferCapacity = outputBufferCapacityNumberOfElements; //tbd make it multiples of disk blocks
	printf("There are %lu bytes for output buffer which can hold max %u elements of size %lu each\n",merger.outputBufferCapacity*sizeof(OUTPUT_TYPE),merger.outputBufferCapacity,sizeof(OUTPUT_TYPE));
	merger.heapCapacity = numFiles;
	
	merger.currentHeapSize=0;

	//open output file - stays open during the merge		
	sprintf( currentOutputFileName,"%s_MERGED", inputFilePrefix);
	
	if(!(merger.outputFP= fopen ( currentOutputFileName , "wb" )))
	{
		printf("Could not open file \"%s\" for writing merged suffix array \n", currentOutputFileName);
		return 1;
	}
		

	//***********MEMORY ALLOCATION
	//1. Allocate heap to hold numFiles elements
	if(!(merger.heap =(HEAP_ELEMENT_TYPE *)calloc(numFiles,sizeof (HEAP_ELEMENT_TYPE))))
	{
		printf("Unable to allocate memory for heap of size %lu bytes.\n",numFiles*sizeof(HEAP_ELEMENT_TYPE) );
		return 1;
	}

	//2. Allocate fileLengths, current file positions, currentbufferlengths, current buffer positions arrays to hold numFiles elements each
	if(!(merger.totalInputFileLengths =(int32_t *)calloc(numFiles,sizeof (int32_t))))
	{
		printf("Unable to allocate memory for an array of file lengths of size %d integers.\n",numFiles );
		return 1;
	}

	if(!(merger.currentInputFilePositions =(int32_t *)calloc(numFiles,sizeof (int32_t))))
	{
		printf("Unable to allocate memory for an array of current input file positions of size %d integers.\n",numFiles );
		return 1;
	}

	if(!(merger.currentInputBufferlengths =(int32_t *)calloc(numFiles,sizeof (int32_t))))
	{
		printf("Unable to allocate memory for an array of input buffer lengths of size %d integers.\n",numFiles );
		return 1;
	}	

	if(!(merger.currentInputBufferPositions =(int32_t *)calloc(numFiles,sizeof (int32_t))))
	{
		printf("Unable to allocate memory for an array of current buffer positions of size %d integers.\n",numFiles );
		return 1;
	}

	//3. allocate output buffer
	if(!(merger.outputBuffer =(OUTPUT_TYPE *)calloc(merger.outputBufferCapacity,sizeof (OUTPUT_TYPE))))
	{
		printf("Unable to allocate memory for an output buffer of size %lu bytes.\n",merger.outputBufferCapacity*sizeof (OUTPUT_TYPE) );
		return 1;
	}
	merger.currentPositionInOutputBuffer=0;

	//4. allocate input buffers - array of buffer arrays
	if(!(merger.inputBuffers =(INPUT_TYPE **)calloc(numFiles,sizeof (INPUT_TYPE *))))
	{
		printf("Unable to allocate memory for an array of input buffers of size %lu bytes.\n",numFiles*sizeof(INPUT_TYPE *) );
		return 1;
	}

	
	for(i=0; i<numFiles;i++)
	{
		merger.currentInputFilePositions[i] = 0;
		merger.currentInputBufferPositions[i] = 0;

		if(!(merger.inputBuffers[i] =(INPUT_TYPE *)calloc(merger.inputBufferCapacity,sizeof (INPUT_TYPE ))))
		{
			printf("Unable to allocate memory for input buffer %d of size %lu bytes.\n",i,merger.inputBufferCapacity*sizeof(INPUT_TYPE) );
			return 1;
		}
	}
	
	printf("Memory allocation completed.\n");
	//***********************end of initialization
	
	//************ collect input file lengths
	for(i=0;i<numFiles;i++)
	{
		sprintf( currentInputFileName,"%s%d_SA", inputFilePrefix,i);
		//open file for reading
		if(!(merger.inputFP= fopen ( currentInputFileName , "rb" )))
		{
			printf("Could not open file \"%s\" for determining length of suffix array %d \n", currentInputFileName,i);
			return (1);
		}
		fseek (merger.inputFP, 0, SEEK_END);   // non-portable ??
    		merger.totalInputFileLengths[i]=ftell (merger.inputFP)/sizeof(INPUT_TYPE);
    		fclose (merger.inputFP);	
	}
	

	//********** perform merge
	if(mergeRuns (&merger)!=0)
		return 1;

	fclose(merger.outputFP);
	//clean memory
	freeMemoryAfterMerge(&merger);
////***********timing total execution - timing ends *****************
stopclock = clock();
	time_spent = (double)(stopclock - startclock) / CLOCKS_PER_SEC;
	printf("\n Total %f seconds for merging suffixes in all files \n",time_spent);		
	return 0;
}


