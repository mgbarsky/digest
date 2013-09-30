#include <stdint.h>

int numBitsOneByte = 8;
int numBitsOneLong = 64;

unsigned char masks_array8[8]={
	(unsigned char)  (1 << 0),
	(unsigned char)  (1 << 1),
	(unsigned char)  (1 << 2),
	(unsigned char)  (1 << 3),
	(unsigned char)  (1 << 4),
	(unsigned char)  (1 << 5),
	(unsigned char)  (1 << 6),
	(unsigned char)  (1 << 7),
};

uint64_t masks_array64[64]={
	(uint64_t)  (1LL << 0),
	(uint64_t)  (1LL << 1),
	(uint64_t)  (1LL << 2),
	(uint64_t)  (1LL << 3),
	(uint64_t)  (1LL << 4),
	(uint64_t)  (1LL << 5),
	(uint64_t)  (1LL << 6),
	(uint64_t)  (1LL << 7),
(uint64_t)  (1LL << 8),
(uint64_t)  (1LL << 9),
(uint64_t)  (1LL << 10),
(uint64_t)  (1LL << 11),
(uint64_t)  (1LL << 12),
(uint64_t)  (1LL << 13),
(uint64_t)  (1LL << 14),
(uint64_t)  (1LL << 15),
	(uint64_t)  (1LL << 16),
	(uint64_t)  (1LL << 17),
	(uint64_t)  (1LL << 18),
	(uint64_t)  (1LL << 19),
	(uint64_t)  (1LL << 20),
	(uint64_t)  (1LL << 21),
	(uint64_t)  (1LL << 22),
	(uint64_t)  (1LL << 23),
(uint64_t)  (1LL << 24),
(uint64_t)  (1LL << 25),
(uint64_t)  (1LL << 26),
(uint64_t)  (1LL << 27),
(uint64_t)  (1LL << 28),
(uint64_t)  (1LL << 29),
(uint64_t)  (1LL << 30),
(uint64_t)  (1LL << 31),
	(uint64_t)  (1LL << 32),
	(uint64_t)  (1LL << 33),
	(uint64_t)  (1LL << 34),
	(uint64_t)  (1LL << 35),
	(uint64_t)  (1LL << 36),
	(uint64_t)  (1LL << 37),
	(uint64_t)  (1LL << 38),
	(uint64_t)  (1LL << 39),
(uint64_t)  (1LL << 40),
(uint64_t)  (1LL << 41),
(uint64_t)  (1LL << 42),
(uint64_t)  (1LL << 43),
(uint64_t)  (1LL << 44),
(uint64_t)  (1LL << 45),
(uint64_t)  (1LL << 46),
(uint64_t)  (1LL << 47),
	(uint64_t)  (1LL << 48),
	(uint64_t)  (1LL << 49),
	(uint64_t)  (1LL << 50),
	(uint64_t)  (1LL << 51),
	(uint64_t)  (1LL << 52),
	(uint64_t)  (1LL << 53),
	(uint64_t)  (1LL << 54),
	(uint64_t)  (1LL << 55),
(uint64_t)  (1LL << 56),
(uint64_t)  (1LL << 57),
(uint64_t)  (1LL << 58),
(uint64_t)  (1LL << 59),
(uint64_t)  (1LL << 60),
(uint64_t)  (1LL << 61),
(uint64_t)  (1LL << 62),
(uint64_t)  (1LL << 63)
};

int getBit64(uint64_t  *sequence, int pos)
{
	return ((*sequence & masks_array64[numBitsOneLong-pos-1])!=0);
}

void setBit64(uint64_t  *sequence, int pos)
{
	(*sequence)|=masks_array64[numBitsOneLong-pos-1];
}

int getBit8(unsigned char *sequence, int pos)
{
	return ((*sequence & masks_array8[numBitsOneByte-pos-1])!=0);
}

void setBit8(unsigned char *sequence, int pos)
{
	(*sequence)|=masks_array8[numBitsOneByte-pos-1];
}

uint64_t getPrefix64(uint64_t  *sequence, int newStartPos, int prefixLen ) //returns new long where 'prefixLen' first bits are extracted and shifted to begin at pos 'newStartPos'  
{
	uint64_t result=((*sequence)>>(numBitsOneLong - prefixLen));
	result<<=(numBitsOneLong - prefixLen - newStartPos);
	return result;
}

uint64_t getSuffix64(uint64_t  *sequence, int newStartPos, int suffixStart) //returns new long where last bits starting from suffixStart are extracted and shifted to begin at pos 'newStartPos'  
{
	uint64_t result=((*sequence)<<(suffixStart));
	result>>=newStartPos;
	return result;
}

uint64_t getBinarySubstringFromTwo64(uint64_t  *sequence1, uint64_t  *sequence2, int startInSeq1)
{
	uint64_t bitseqSuffix;
	uint64_t result = 0LL;
	
	if(!sequence1)
		return result;
	result |= ((*sequence1)<<(startInSeq1));
	if(!sequence2)
		return result;
	bitseqSuffix = getPrefix64(sequence2, numBitsOneLong - startInSeq1, startInSeq1 );
	return (result|bitseqSuffix);	
}

unsigned char getPrefix8(unsigned char  *sequence, int newStartPos, int prefixLen ) //returns new byte where 'prefixLen' first bits are extracted and shifted to begin at pos 'newStartPos'  
{
	unsigned char result=((*sequence)>>(numBitsOneByte - prefixLen));
	result<<=(numBitsOneByte - prefixLen - newStartPos);
	return result;
}

unsigned char getSuffix8(unsigned char  *sequence, int newStartPos, int suffixStart) //returns new long where last bits starting from suffixStart are extracted and shifted to begin at pos 'newStartPos'  
{
	unsigned char result=((*sequence)<<(numBitsOneByte - suffixStart));
	result>>=newStartPos;
	return result;
}


void bits64AsString (uint64_t  *bitseq, char *str)
{
	int i=0;
	for(i=0;i<numBitsOneLong;i++)
	{		
		int bit=getBit64(bitseq ,i);
		str[i]=(bit==0)?'O':'I';
	}
	str[i]='\0';
}

void bits8AsString (unsigned char *bitseq, char *str)
{
	int i=0;
	for(i=0;i<numBitsOneByte;i++)
	{		
		int bit=getBit8(bitseq ,i);
		str[i]=(bit==0)?'O':'I';
	}
	str[i]='\0';
}



