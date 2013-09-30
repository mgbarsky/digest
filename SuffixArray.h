#define MIN(a, b) ((a)<=(b) ? (a) : (b))
#define MAX(a, b) ((a)>=(b) ? (a) : (b))

#define MAX_PATH_LENGTH 500  //to access files of a predefined path
#define MAX_CHARS_PER_SUFFIX 127 //at most 127 meaningful characters, 1 additional for a termination char
#define MAX_CHARS_PER_LINE 10000  //to define an array to read each line of an input

#define LCP_TEST_MODE 0
#define MERGE_TEST_MODE 0
#define SA_SORTING_TEST_MODE 0

/**
Part 1. Lexicographical sorting of suffixes in each separate input file
**/

#define NUM_SUFFIXES_IN_OUTPUT_BUFFER 1000 //40 bytes per suffix - makes 1000 suffixes ~ 40 MB - to collect prefixed suffixes and to flush them to disk 
#define MAX_COUNT 32000 //this is max counts per 1 organism - depends on the type of value used for counts

//Information about the suffix is stored in 1 8-byte integer
typedef union
{
    struct {
        unsigned char seqLength;
        unsigned char bwtChar;
	unsigned short fileID;
	unsigned short count;
        unsigned char reserve[2];  //not currently in use
    } info;
    uint64_t as_64bits; //for alignment
} SuffixInfo;

//Struct represents suffix information and 32 Bytes for binary encoding of the suffix: can hold 256 bits, for DNA alphabet of size 4 makes 128 characters max 
typedef struct SuffixToMerge  //5 8bytes = 40 bytes per (unique) suffix position
{
	SuffixInfo header;
	uint64_t data[4];
} SuffixToMerge;

//bookkeeping struct - keeps all necessary info required for single-file suffix sorting
typedef struct SABuildManager  //5 8bytes = 40 bytes per (unique) suffix position
{
	unsigned char maxLineLength; //what is the maximum line length in this file
	uint32_t numberOfLines; //what is total number of lines	
	uint32_t totalCharacters; //total number of characters in concatenated input, including termination chars
	SuffixToMerge *prefixedSuffixesBuffer;  //Buffer - here it is going to collect 40x prefixed suffixes from sorted array to flush them to disk
	unsigned char *inputAsChars; //line input converted into a single string just by concatenating all lines and putting a termination char '?' at the end of each line
	uint32_t *intSA; //Here is an array of positions in the input file, each position represents a start of a suffix, and by the end of processing, these suffixes are lexicographically sorted	
	unsigned short currentFileID;  //id of a file currently being processed	
} SABuildManager;

//goes in a loop from 0 to totalInputFiles-1 and creates suffix arrays for each file
//allocates memory once for all and frees it at the end
int buildSuffixArraysForAllFiles(char *inputFileNamePrefix, int totalInputFiles); //suffix_array_all_files.c
int freeMemoryAfterSAForAll(SABuildManager* manager); //suffix_array_all_files.c

//builds suffix array for a single file - with memory allocated and all parameters set in the previous loop
int buildSuffixArrayForSingleFile (SABuildManager *manager, FILE *inputFP, FILE *outputFP); //suffix_array_single_file.c

//collects one input stats: num of lines, max line length
int collectOneInputStats (FILE *inputFP, uint32_t *maxNumberOfLines, unsigned char *maxLinelength ); //suffix_array_single_file.c

//calls divsufsort of mori algorithm to build suffix array in memory - with input and output ready
int create_suffix_array_mori(SABuildManager *manager); //sa_mori.c

//when suffix sorting is complete - records to file resulting information with attached binary prefixes, uses buffering of prefixed suffixes
int recordOrderedSuffixesToFile(SABuildManager *manager,FILE *outputFP); //suffix_array_single_file.c

//bit operations are defined in file 'bit_operations.c'
int getBit64 (uint64_t  *sequence, int pos);
void setBit64 (uint64_t  *sequence, int pos);
int getBit8 (unsigned char *sequence, int pos);
void setBit8 (unsigned char *sequence, int pos);
uint64_t getBinarySubstringFromTwo64(uint64_t  *sequence1, uint64_t  *sequence2, int startInSeq1);
uint64_t getPrefix64 (uint64_t  *sequence, int newStartPos, int prefixLen);
uint64_t getSuffix64 (uint64_t  *sequence, int newStartPos, int suffixStart);
unsigned char getPrefix8 (unsigned char  *sequence, int newStartPos, int prefixLen);
unsigned char getSuffix8(unsigned char  *sequence, int newStartPos, int suffixStart) ;
void bits64AsString (uint64_t  *bitseq, char *str); //For printing 64-bit sequence : str should be of length 65
void bits8AsString (unsigned char *bitseq, char *str); //For printing 8-bit sequence : str should be of length 9
 
//compares an array of unsigned longs attached to two suffixes, needs to know what is the suffix lengths to determine how many bits to consider
int compareBinaryPrefix(uint64_t *startData1, uint64_t *startData2, unsigned short len1, unsigned short len2 ); // prefix_compare.c
//gets lcp in bits - including termination char
int getLCPInBits (uint64_t *startData1, uint64_t *startData2, 
	unsigned short len1, unsigned short len2, char *firstBitAfterLCP) ; // prefix_compare.c
/**
Part 2. Multiway merge sort - external
**/

//specific macros for suffix merge - in order for merge-sort to work with any type of data
#define INPUT_TYPE SuffixToMerge
#define HEAP_ELEMENT_TYPE SuffixToMerge
#define OUTPUT_TYPE SA_LCP_fileID

typedef struct SA_LCP_fileID  //Output type - what will be written into output buffer - 64-bits per character input - SPECIFIC
{
	unsigned char suffixLength;
    	unsigned char bwtChar;
	char firstBitAfterLCP; //0-bit, 1-bit and -1 for empty char
	unsigned char lcpInBits; 
	unsigned short fileID;
	unsigned short count;
} SA_LCP_fileID;

typedef struct EMMergeManager  //Bookkeeping: keeps track of all necessary variables during external merge - GENERAL
{
	HEAP_ELEMENT_TYPE *heap;  //keeps 1 from each buffer in top-down order - smallest on top (according to compare function)
	HEAP_ELEMENT_TYPE lastTransferred; //last suffix transferred from heap to output buffer
	FILE *inputFP; //stays closed, opens each time we need to reupload some amount of data from disk
	char inputFileNamePrefix [MAX_PATH_LENGTH];  //thats why we need to know the file prefix to open the necessary file	
	FILE *outputFP;  //stays open to flush output buffer when full
	OUTPUT_TYPE* outputBuffer; //buffer to store output elements until they are flushed to disk
	int32_t currentPositionInOutputBuffer;  //where to add element in output buffer
	int32_t outputBufferCapacity; //how many elements max can it hold
	INPUT_TYPE **inputBuffers; //array of buffers to hold part of runs
	int32_t inputBufferCapacity; //how many elements max can each hold
	int32_t *currentInputFilePositions; //current position in each file, -1 if the run is complete
	int32_t *totalInputFileLengths; //total number of elements in each file
	int32_t *currentInputBufferPositions; //position in current input buffer, if no need - -1
	int32_t *currentInputBufferlengths;  //number of elements currently in input buffer
	int32_t currentHeapSize;
	int32_t heapCapacity;  //corresponds to the total number of files	
} EMMergeManager;

//---------GENERAL functions - for any external multiway merge sort
int mergeRuns (EMMergeManager *merger); //merges all runs into a single sorted list
int initialFillBuffers(EMMergeManager *merger); //initial fill of input buffers with elements of each run
int initialInsertIntoHeap(EMMergeManager *merger); //inserts into heap one element from each buffer - to keep them sorted
int getNextInputElement (EMMergeManager *merger, int32_t fileID, HEAP_ELEMENT_TYPE *result); //reads the next element from an input buffer, 
// uploads next part of a run from disk if necessary by calling refillBuffer
int refillBuffer(EMMergeManager *merger, int32_t fileID);

int insertIntoHeap (EMMergeManager *merger, HEAP_ELEMENT_TYPE *newHeapElement); //inserts next element into heap
int getTopHeapElement (EMMergeManager *merger, HEAP_ELEMENT_TYPE *result); //removes smallest element from the heap, and restores heap order
int addToOutputBuffer(EMMergeManager *merger, OUTPUT_TYPE* newElement); //adds next smallest element to the output buffer, flushes buffer if full by calling flushOutputBuffer
int flushOutputBuffer(EMMergeManager *merger);

int convertHeapElementToOutputElement (EMMergeManager *merger, HEAP_ELEMENT_TYPE *inputElem, OUTPUT_TYPE *outputElem); //implementation-specific - in this case adds lcp
int compare(HEAP_ELEMENT_TYPE* first, HEAP_ELEMENT_TYPE* second); //implementation-specific: varies depending on the merged elements
uint32_t getFileID(HEAP_ELEMENT_TYPE *record); //extracts run ID from the currently transferred heap element -  to add a new element from the same run - implementation specific  - heap type differes
//-----End of general external multiway merge sort

//-----SPECIFIC - for merging suffixes
//allocates memory for all the buffers and then calls mergeRuns
int mergeSuffixArraysIntoSA_LCP (char * inputFilePrefix, int numFiles, uint64_t memoryForInputBuffersBytes, uint64_t outputBufferCapacityNumberOfElements);
int freeMemoryAfterMerge(EMMergeManager *merger);
