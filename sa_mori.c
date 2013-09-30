#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "divsufsort.h"
#include "SuffixArray.h"

int create_suffix_array_mori(SABuildManager *manager)
{
	if(divsufsort(&manager->inputAsChars[0], (int *)(&manager->intSA[0]), (int)(manager->totalCharacters)) != 0) 
	{
    		printf("Failed to build suffix array by the Mori algorithm.\n");
    		return 1;
  	}
	return 0;
}

