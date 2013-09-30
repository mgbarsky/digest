#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "SuffixArray.h"

// with memcmp - does not work correctly
/*
int compareBinaryPrefix(uint64_t *startData1, uint64_t *startData2, unsigned short len1, unsigned short len2 )
{
	int minLength = 2*MIN(len1, len2);  //minimum bits in binary seq
	int howManyLongsToCompare = minLength/64;

	
	if(minLength%64 >0)
		howManyLongsToCompare++;
	
	int cmpResult =memcmp ((unsigned char *)startData1, (unsigned char *)startData2,howManyLongsToCompare*sizeof(uint64_t)); 
	
	if(cmpResult!=0)
	{
		printf("minBitLengthTo Compare=%d  cmpResult = %d\n",minLength,cmpResult);
		return cmpResult;
	}
	if(len1>len2)
		return 1;
	if(len1<len2)
		return -1;
	return 0;
}*/


int compareBinaryPrefix(uint64_t *startData1, uint64_t *startData2, unsigned short len1, unsigned short len2 )
{
	int i;
	int minLength = 2*MIN(len1, len2);  //minimum bits in binary seq
	int howManyLongsToCompare = minLength/64;

	
	if(minLength%64 >0)
		howManyLongsToCompare++;
	
	
	for(i=0;i<howManyLongsToCompare;i++)
	{
		if(startData1[i]>startData2[i])
			return 1;
		if(startData1[i]<startData2[i])
			return -1;
		
	}
	
	if(len1>len2)
		return 1;
	if(len1<len2)
		return -1;
	return 0;
}

int getLCPInBits (uint64_t *startData1, uint64_t *startData2, 
	unsigned short len1, unsigned short len2, char *firstBitAfterLCP) 
{
	int lcp=0;
	int i;
	int bit;
	int differenceFound = 0;
	
	int minLength =2*MIN(len1, len2);  //minimum bits in binary seq
	int howManyLongsToCompare = minLength/64;
	int minLengthInLastLong = minLength%64;	
	if(minLengthInLastLong>0)
		howManyLongsToCompare++;
	else
		minLengthInLastLong=64;
	
	i=0;
	while(i<howManyLongsToCompare-1 && !differenceFound) //we compare k-1 first numbers for equality
	{
		if(startData1[i]==startData2[i])  
		{
			lcp+=64;
			i++;
		}
		else
			differenceFound=1;					
	}
	
	if(differenceFound)  //difference found before last long has been reached
	{
		//comparing the non-last long where the 2 sequences of bits differ - neither length falls into this long
		int equals=0;
		int shiftCount=0;
		while(!equals && shiftCount<=64)
		{
			shiftCount++;
			if((startData1[i]>>shiftCount) == (startData2[i]>>shiftCount))
			{
				equals=1;
				lcp+=(64-shiftCount);
			}
		}
		return lcp;  //TBD make error handling
	}
	else  //we are now at last long, all previous were equals
	{
		if(startData1[i]==startData2[i])  //the numbers are equal, but actual number of common bits is minlength in last long
		{
			lcp+=minLengthInLastLong;
			//here may be several cases:
			//case 1. length1=length2. - the suffixes are equal, and first bit after lcp is empty string
			if(len1==len2)
				*firstBitAfterLCP =-1; //empty character
			//case 2. first is shorter than the next - first terminated, the next can be any character
			else if(len1<len2)			
			{
				//case 2a. minLengthInLastLong<64 bits - look in the same long of seq2 to find next bit
				if(minLengthInLastLong<64)
				{
					bit = getBit64(&startData2[i], minLengthInLastLong);
					if(bit == 0)
						*firstBitAfterLCP=0; 
				}
				//case 2b. minLengthInLastLong=64 bits, the second is longer than that so we need the first bit in the next long
				else
				{
					if(i+1>=4) //error, only 4 longs are attached
					{
						printf("Unexpected error while extracting lcp from bit sequence. Trying to access more than 4 longs\n");
						exit(1);
					}
					bit = getBit64(&startData2[i+1], 0);
					if(bit == 0)
						*firstBitAfterLCP=0;
				}
			}
			//case3. First is longer than the next. There is empty char after lcp
			else
			{
				*firstBitAfterLCP=-1;
			}
		}
		else //check minLengthInLastLong bits - until they are equal
		{
			uint64_t prefix1= (startData1[i]>>(64-minLengthInLastLong));
			uint64_t prefix2= (startData2[i]>>(64-minLengthInLastLong));
			
			int equals = 0;
			int shiftCount = 0;
			while(shiftCount<minLengthInLastLong && !equals)
			{
				if((prefix1>>shiftCount) == (prefix2>>shiftCount))
				{
					equals=1;
					lcp+=(minLengthInLastLong-shiftCount);

					if(shiftCount == 0) //this means all minlength is lcp - either of sequences is exhausted until the end
					{
						//here may be several cases:
						//case 1. length1=length2. - the suffixes are equal, and first bit after lcp is empty string
						if(len1==len2)
							*firstBitAfterLCP =-1; //empty character
						//case 2. first is shorter than the next - first terminated, the next can be any character
						else if(len1<len2)			
						{
							//case 2a. minLengthInLastLong<64 bits - look in the same long of seq2 to find next bit
							if(minLengthInLastLong<64)
							{
								bit = getBit64(&startData2[i], minLengthInLastLong);
								if(bit == 0)
									*firstBitAfterLCP=0; 
							}
							//case 2b. minLengthInLastLong=64 bits, the second is longer than that so we need the first bit in the next long
							else
							{
								if(i+1>=4) //error, only 4 longs are attached
								{
									printf("Unexpected error while extracting lcp from bit sequence. Trying to access more than 4 longs\n");
									exit(1);
								}
								bit = getBit64(&startData2[i+1], 0);
								if(bit == 0)
									*firstBitAfterLCP=0;
							}
						}
						//case3. First is longer than the next. There is empty char after lcp
						else
						{
							*firstBitAfterLCP=-1;
						}
					}
				}
				shiftCount++;
			}
		}
	}	
	
	return lcp;
}
