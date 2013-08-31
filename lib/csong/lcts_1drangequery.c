/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen

   Contacts: vmakinen at cs helsinki fi

   Implements a data structure for one-dimensional dynamic semi-infinite
   range minimum queries for keys in range [0...n-1]. Query time is O(log n). 
   The structure is a complete binary tree with n leaves supporting 
   predecessor and successor queries, and insertions and deletions.

   Range query [-infty,x) is answered by calling i=predecessor(x) and
   taking value l(i) of i from an external array.

   Inserting a point x is done by calling insert(x) and then calling
   deleteGreaterSuccessors(x,values), which removes all successors
   of x that have greater value than x. This guarantees that range
   query [-infty,x) is answered correctly.


   This file is part of C-Brahms Engine for Musical Information Retrieval.

   C-Brahms Engine for Musical Information Retrieval is free software; 
   you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   C-Brahms Engine for Musical Information Retrieval is distributed 
   in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
   without even the implied warranty of MERCHANTABILITY or FITNESS 
   FOR A PARTICULAR PURPOSE. 
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with C-Brahms Engine for Musical Information Retrieval; 
   if not, write to the Free Software Foundation, Inc., 59 Temple Place, 
   Suite 330, Boston, MA  02111-1307  USA
*/
 

#include "stdlib.h"
#include "lcts.h"


/* Logarithm of base 2. */
inline unsigned int log_2(unsigned long n)
{
	doubleAndMask fm;
	fm.asDouble = (double) n;
	return (fm.asMask.exponentbias1023 - 1023);
}


/*
   Creates a complete binary tree with n leaves supporting 
   predecessor and successor queries, and insertions and deletions.
*/
treeNode *CreateCompleteBinaryTree(unsigned int leaves)
{
	unsigned int i;
	treeNode *treeAsArray=(treeNode*)malloc(sizeof(treeNode)*(leaves*2));
	/* no leafs in the tree at first */
	for (i=0;i<2*leaves;i++) 
	{
		treeAsArray[i].isLeft = 0;
		treeAsArray[i].isRight = 0;
	}
	return treeAsArray;         
}


/* Searchs for the predecessor of i in the tree. */
inline unsigned int predecessor(treeNode *A, unsigned int leaves, unsigned int index)
{
	/* in normal tree presentation:
	// x = "ith leaf"
	while (x!=NULL && (x==x->parent->left || !x->parent->isLeft))
	x = x->parent;
	// x->parent is the LCA of the original x and predesessor(x)	 
	if (x==NULL) return leafs;
	x = x->parent->left;
	while (!isLeaf(x)) 
	if (x->isRight) x = x->right;
	else x = x->left;
	return x; */	
   
	/* in array implementation: */
	unsigned int i = leaves+index;

	while (i>1 && ((i==(i>>1)<<1) || !A[i/2].isLeft)) i = i/2;

	if (i==1) return leaves;
	i = i-1;

	while (i<leaves) if (A[i].isRight) i = 2*i+1; else i = 2*i;

	return i-leaves;             
} 


/* Searchs for the successor of i in the tree. */
inline unsigned int successor(treeNode *A, unsigned int leaves, unsigned int index)
{
   
	/* in normal tree presentation:
	// x = "ith leaf"
	while (x!=NULL && (x==x->parent->right || !x->parent->isRight))
	x = x->parent;
	// x->parent is the LCA of the original x and successor(x)	 
	if (x==NULL) return leafs;
	x = x->parent->right;
	while (!isLeaf(x)) 
	if (x->isLeft) x = x->left;
	else x = x->right;
	return x; */	
   
	/* in array implementation: */
	unsigned int i = leaves+index;

	while (i>1 && ((i!=(i>>1)<<1) || !A[i/2].isRight)) i = i/2;

	if (i==1) return leaves;
	i = i+1;

	while (i<leaves) if (A[i].isLeft) i = 2*i; else i = 2*i+1;

	return i-leaves;             
} 


/* Inserts leaf 'index' and updates information to the root. */
inline void insertLeaf(treeNode *A, unsigned int leaves, unsigned int index)
{
	unsigned int i = leaves+index;
	while (i>1) 
	{
		if (i==(i>>1)<<1) if (A[i/2].isLeft) break; else A[i/2].isLeft = 1;
		else if (A[i/2].isRight) break; else A[i/2].isRight = 1;
		i = i/2;
	}
	return;
} 


/* Deletes leaf 'index' and updates information to the root. */
inline void deleteLeaf(treeNode *A, unsigned int leaves, unsigned int index)
{
	unsigned int i = leaves+index;
	while (i>1 && !A[i].isLeft && !A[i].isRight) 
	{
		if (i==(i>>1)<<1) A[i/2].isLeft = 0;
		else A[i/2].isRight = 0;
		i = i/2;
	}
	return;
} 


/* Deletes j = successor(index) until values[j] < values[index]. */ 
inline void deleteGreaterSuccessors(treeNode *A, unsigned int leaves, unsigned int index, int *values)
{
	unsigned int j = successor(A,leaves,index);
	while (j<leaves && values[j]>=values[index]) 
	{
		deleteLeaf(A,leaves,j);
		j = successor(A,leaves,index);
	}
	return;
} 


