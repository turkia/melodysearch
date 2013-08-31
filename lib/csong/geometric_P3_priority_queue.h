/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen
   Modified by Mika Turkia

   Contacts: vmakinen at cs helsinki fi
*/

#include <limits.h>
#include <stdlib.h>
#include <ruby.h>
#include "geometric_P3.h"

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
	/* for referring to values in the priority queue */
	unsigned int key;

	/* data is a translation vector */
	TranslationVector vector;
} treenode;


typedef struct 
{
	treenode *tree;
	unsigned int leaves;
} priority_queue;


inline static priority_queue *p3_create_priority_queue(unsigned int size);
inline static void p3_update_value(priority_queue *pq, treenode *n);
inline static treenode p3_get_min(priority_queue *pq);

