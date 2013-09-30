CC = gcc
CFLAGOPT = -O3 -Wall 
CFLAGS = -D_LARGEFILE_SOURCE
CFLAGS += -fno-exceptions
CFLAGS += -finline-functions
CFLAGS += -funroll-loops
CFLAGOFFSET = -D_FILE_OFFSET_BITS=64

all: buildandmerge mergesasinglefile buildsaallfiles prepareonunix
buildandmerge:
	$(CC) $(CFLAGOPT) $(CFLAGOFFSET) $(CFLAGS) suffix_array_all_files.c suffix_array_single_file.c bit_operations.c sa_mori.c divsufsort.c prefix_compare.c merge_external.c merge_sa_lcp_filenames.c main.c -o buildandmerge
mergesasinglefile:
	$(CC) $(CFLAGOPT) $(CFLAGOFFSET) $(CFLAGS) bit_operations.c prefix_compare.c merge_external.c merge_sa_lcp_filenames.c merge_sa_main.c -o mergesasinglefile
buildsaallfiles:
	$(CC) $(CFLAGOPT) $(CFLAGOFFSET) $(CFLAGS)  suffix_array_all_files.c suffix_array_single_file.c bit_operations.c sa_mori.c divsufsort.c prefix_compare.c sa_all_main.c -o buildsaallfiles
prepareonunix:
	$(CC) $(CFLAGOPT) $(CFLAGS)  prepare_inputs.c -o prepare
clean:  
	rm buildandmerge mergesasinglefile buildsaallfiles prepare
