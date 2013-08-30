/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen
*/


#include <limits.h>
#include <stdlib.h>
#include <ruby.h>

typedef struct 
{
	unsigned int mantissa2 : 32;
	unsigned int mantissa1 : 20;
	unsigned int exponentbias1023 : 11;
	unsigned int signbit : 1;
} doubleMask;

typedef union
{
	double asDouble;
	doubleMask asMask;
} doubleAndMask;

typedef struct 
{
	unsigned int key;
	int strt;
	char ptch;
} treeNode;


unsigned int PQ_log_2(unsigned long n);
void PQ_updateValue(treeNode *A, unsigned int leaves, unsigned int key, int strt, char ptch);
void PQ_getMin(treeNode *A, unsigned int *key, int *strt, char *ptch);
treeNode *PQ_CreateCompleteBinaryTree(unsigned int leaves);
