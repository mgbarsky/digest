digest
======
This is a program which builds suffix array with LCP and BWT for large collection of files, where each file consists of short lines

Each suffix in each line is indexed

The main feature is that program uses as little memory as possible and stores inputs, intermediate data and outputs on disk

This makes it suitable for very large text collections

In particular, it is designed to handle short reads of DNA - sequences up to 100 characters over alphabet {a,c,g,t}, but it can easily be modified to handle any alphabet.

The software runs on any operating system.

However, the first step is preprocessing of files into numbered cleaned collection is unix-only (GNU), as it uses library <fts.h>. This header file is missing on some platforms: AIX 5.1, HP-UX 11, IRIX 6.5, OSF/1 5.1, Solaris 11 2011-11, mingw, MSVC 9, BeOS.

Compile:
make

**********************
How to use
**********************

1. Preprocess your files placed in a particular folder "folder_path/folder_name". Create folder "inputs" and run:
./prepareonunix folder_path/folder_name
This creates a collection of files in folder "inputs" which are cleaned and prepared for further processing


Software consists of two major steps:
Step 1: Sortig suffixes of each line in each separate file and writing them on disk, together with the binary representation of each suffix. Each file with sorted suffixes, file numbers and BWT chars is a run.
Step 2: External memory multiway merge sort for runs produces a single output file, which contains all suffixes of the collection in lexicographic order, with attached file numbers and BWT chars.

****************
Single machine processing
***************

The steps can be run separately:
./buildsaallfiles inputs/input_ <num_files>
./mergesasinglefile inputs/input_ 'num_files' 'memory_bytes_for_all_input_buffers' 'elements_in_output_buffer'

or as a single program
./buildandmerge inputs/input_ 'num_files' 'memory_bytes_for_all_input_buffers' 'elements_in_output_buffer'

two last parameters define memory usage during the merge


