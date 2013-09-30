#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SuffixArray.h"


int mergeRuns (EMMergeManager *merger)  //merger should be already set up at this point
{
	int  result;
	//1. go in the loop through all input files and fill-in initial buffers
	if(initialFillBuffers(merger)!=0)
		return 1;
	printf("Init buffers completed.\n");

	
	//2. Initialize heap content with 1 element from each buffer
	result = initialInsertIntoHeap(merger);
	if(result!=0)
		return 1;
	printf("Init heap completed. Heap contains %d elements \n", merger->currentHeapSize);
	
	while (merger->currentHeapSize>0)  //heap is not empty
	{
		HEAP_ELEMENT_TYPE smallest;
		HEAP_ELEMENT_TYPE next; //here next is of input_type
		OUTPUT_TYPE outputElem;
		int result;
		if(getTopHeapElement (merger, &smallest)!=0)
			return 1;
		
		printf("merger->lastTransferred.header.info.fileID=%d\n",merger->lastTransferred.header.info.fileID);
		if(convertHeapElementToOutputElement (merger, &smallest, &outputElem)!=0)
			return 1;

		merger->outputBuffer[merger->currentPositionInOutputBuffer++]=outputElem;
		merger->lastTransferred = smallest;

		

		result = getNextInputElement (merger, getFileID(&smallest), &next);

		if(result==0) //next element exists
		{
			if(insertIntoHeap (merger, &next)!=0)
				return 1;
		}
		if(result==1) //error
			return 1;
		
		if(merger->currentPositionInOutputBuffer == merger-> outputBufferCapacity ) //staying on the last slot of the output buffer - next will cause overflow
		{
			if(flushOutputBuffer(merger)!=0)
				return 1;			
			merger->currentPositionInOutputBuffer=0;
		}	
	}
	//flush what remains in output buffer
	if(merger->currentPositionInOutputBuffer >0) //there are elements in the output buffer
	{
		if(flushOutputBuffer(merger)!=0)
			return 1;
	}

	return 0;	
}



int initialInsertIntoHeap(EMMergeManager *merger)
{
	INPUT_TYPE first;
	int i;
	for(i=0;i<merger->heapCapacity;i++)
	{	
		//get first suffix from each buffer		
		if(getNextInputElement (merger,i, &first)!=0)
			return 1;
		
		//tbd - if needed convert INPUT_TYPE to HEAP_ELEMENT
		//insert it into heap
		if(insertIntoHeap (merger, &first)!=0)
			return 1;
	}
	return 0;	
}

int initialFillBuffers(EMMergeManager *merger)
{
	int i;
	for(i=0;i<merger->heapCapacity;i++)
	{
		if(refillBuffer(merger,i)!=0)
		{
			printf("Failed to fill initial buffer %d\n",i);
			return 1;
		}
	}
	
	return 0;
}

int flushOutputBuffer(EMMergeManager *merger)  //tbd - record information about each flushed chunk, split blocks of the output
{
	int written = fwrite(merger->outputBuffer, sizeof (OUTPUT_TYPE), merger->currentPositionInOutputBuffer, merger->outputFP);
	if(written!=merger->currentPositionInOutputBuffer)
	{
		printf("Not all output elements were written\n");
		return 1;
	}
		
	return 0;
}


int  getTopHeapElement (EMMergeManager *merger, HEAP_ELEMENT_TYPE *result)
{
	HEAP_ELEMENT_TYPE item;
	int child, parent;

	if(merger->currentHeapSize == 0)
	{
		printf( "UNEXPECTED ERROR: popping top element from an empty heap\n");
		return 1;
	}

	*result=merger->heap[0];  //to be returned

	//now we need to reorganize heap - keep the smallest on top
	item = merger->heap [--merger->currentHeapSize]; // to be reinserted 

	parent =0;

	while ((child = (2 * parent) + 1) < merger->currentHeapSize) 
	{
		// if there are two children, compare them 
		if (child + 1 < merger->currentHeapSize && (compare(&(merger->heap[child]),&(merger->heap[child + 1]))>0)) 
		{
			++child;
		}
		// compare item with the larger 
		if (compare(&item, &(merger->heap[child]))>0) 
		{
			merger->heap[parent] = merger->heap[child];
			parent = child;
		} 
		else 
		{
			break;
		}
	}
	merger->heap[parent] = item;
	
	return 0;
}

int insertIntoHeap (EMMergeManager *merger, HEAP_ELEMENT_TYPE *newHeapElement)
{
	int child, parent;
	if (merger->currentHeapSize == merger->heapCapacity) 
	{
		printf( "Unexpected ERROR: heap is full\n");
		return 1;
	}
	
  	
	child = merger->currentHeapSize++; /* the next available slot in the heap */
	
	while (child > 0) 
	{
		parent = (child - 1) / 2;
		if (compare(&(merger->heap[parent]),newHeapElement)>0) 
		{
			merger->heap[child] = merger->heap[parent];
			child = parent;
		} 
		else 
		{
			break;
		}
	}
	merger->heap[child]= *newHeapElement;	
	return 0;
}

int getNextInputElement (EMMergeManager *merger, int32_t fileID, HEAP_ELEMENT_TYPE *result)
{
	if(merger->currentInputBufferPositions[fileID] == -1) //run is complete
		return -1; //empty
	
	if(merger->currentInputBufferPositions[fileID]<merger->currentInputBufferlengths[fileID]) //there are elements in the buffer
	{		
		*result=  merger->inputBuffers[fileID][merger->currentInputBufferPositions[fileID]++];		
	}
	else
	{
		int refillResult = refillBuffer(merger,fileID);
		if(refillResult==0)			
			return getNextInputElement (merger,  fileID, result);
		else 
			return refillResult;
	}
	return 0; //success
}


int refillBuffer(EMMergeManager *merger, int32_t fileID)
{
	char currentInputFileName[MAX_PATH_LENGTH];
	int toRead;
	int result;

	if(merger->currentInputFilePositions[fileID] == -1)
	{
		merger->currentInputBufferPositions[fileID] = -1; //signifies no more elements
		return -1; //run is complete - no more elements in the input file
	}

	sprintf(currentInputFileName,"%s%d_SA", merger->inputFileNamePrefix,fileID);
	
	if(!(merger->inputFP=fopen(currentInputFileName,"rb")))
	{
		printf("Cannot open input suffix array File <%s> for reading\n",currentInputFileName);
		return 1;
	}

	//move to the corresponding position 
	fseek(merger->inputFP,merger-> currentInputFilePositions[fileID]*sizeof(INPUT_TYPE),SEEK_SET);  //fseek can fail - TBD replace with XXL
	
	//calculate how many elements to read from the input file
	toRead = MIN(merger->inputBufferCapacity,merger->totalInputFileLengths[fileID] - merger-> currentInputFilePositions[fileID]);

	result=fread (merger->inputBuffers[fileID],sizeof(INPUT_TYPE),toRead,merger->inputFP);
	if(result!=toRead)
	{
		printf("Error in refilling buffer %d: wanted to read %d and in fact read %d\n",fileID,toRead,result);
		return 1;
	}
	fclose(merger->inputFP);

	merger->currentInputBufferlengths[fileID] = toRead;
	merger->currentInputBufferPositions[fileID]=0;

	//move start position in file - for the next read
	merger->currentInputFilePositions[fileID]+=toRead;

	if(merger->currentInputFilePositions[fileID]>=merger->totalInputFileLengths[fileID])//finished processing file  tbd - check how it can be >
	{
		merger->currentInputFilePositions[fileID] = -1; //no next time refilling
	}
	
	return 0;
}

int freeMemoryAfterMerge(EMMergeManager *merger)
{
	int i;
	if(merger->heap)
		free(merger->heap);
	if(merger->totalInputFileLengths)
		free(merger->totalInputFileLengths);
	if(merger->currentInputFilePositions)
		free(merger->currentInputFilePositions);
	if(merger->currentInputBufferlengths)
		free(merger->currentInputBufferlengths);
	if(merger->currentInputBufferPositions)
		free(merger->currentInputBufferPositions);
	if(merger->outputBuffer)
		free(merger->outputBuffer);

	for(i=0;i<merger->heapCapacity;i++)
	{
		if(merger->inputBuffers[i])
			free(merger->inputBuffers[i]);
	}
	
	if(merger->inputBuffers)
		free(merger->inputBuffers);
	return 0;
}
 
