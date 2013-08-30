/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen 
   Modified by Mika Turkia

   Contacts: vmakinen at cs helsinki fi

   Implements a priority queue using a binary tree.
   The idea is that each leaf stores (key,p) and the 
   internal nodes keep that (key,p) whose priority p is the minimum in its subtree.
   This way the key having minimum priority is stored in the root.
   To change the priority of a key, it is enough to locate the leaf holding that key, 
   and change the priority at all nodes from the leaf to the root. 
   The binary tree is implemented as an array.
   (Here p is a pair, and it is used for storing vectors of note onset time and pitch.)
*/

 
#include "priority_queue.h"


unsigned int PQ_log_2(unsigned long n) 
{
	doubleAndMask fm;
	fm.asDouble = (double) n;
	return (fm.asMask.exponentbias1023 - 1023);
}


treeNode *PQ_CreateCompleteBinaryTree(unsigned int leaves)
{
	unsigned int i;

	treeNode *treeAsArray = (treeNode *) calloc(leaves * 2, sizeof(treeNode));

	/* no leafs in the tree at first */
	for (i = 0; i < 2 * leaves; i++)
	{ 
		treeAsArray[i].strt = INT_MAX;
		treeAsArray[i].ptch = CHAR_MAX;
	}

	return treeAsArray;         
}


void PQ_updateValue(treeNode *A, unsigned int leaves, unsigned int key, int strt, char ptch)
{
	/* update priority 'p' from leaf 'key' to the root */
	unsigned int i = leaves + key;
	unsigned int j;

	A[i].strt = strt;
	A[i].ptch = ptch;
	A[i].key = key;

	while (i > 1) 
	{
		/* if i even then j next odd else j previous even */
		if (i == (i >> 1) << 1) j = i + 1;
		else j = i - 1;

		/* i and j left and right son of i/2. choose the minimum p from the sons. */
		if (A[i].strt < A[j].strt || (A[i].strt == A[j].strt && A[i].ptch < A[j].ptch) || \
			(A[i].strt == A[j].strt && A[i].ptch == A[j].ptch && A[i].key < A[j].key)) A[i/2] = A[i];
		else A[i/2] = A[j];

		/* proceed to the parent */
		i = i/2;
	}
}


void PQ_getMin(treeNode *A, unsigned int *key, int *strt, char *ptch)
{
	/* the minimum is always in the root */
	*key = A[1].key;
	*ptch = A[1].ptch;
	*strt = A[1].strt;
} 
