/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen

   Contacts: vmakinen at cs helsinki fi


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

#include <stdlib.h>
#include "song.h"

#define MAX_TRANSPOSITION 256

struct tNode;

/* An element of sparse matrix M, that is a subset of {(i,j,k)}. */
typedef struct tNode {
	
	/* next element on row */
	struct tNode *next; 

	/* prev element on diagonal */
	// not needed anymore: struct tNode *prevD; 

	/* prev match in the optimal path (trace) */
	struct tNode *prevTrace;

	/* row number */
	int i;

	/* column number (chord index + 1?) */
	int j;

	/* track number */
	int k;

	/* splits so far */
	int kappa;
} tripleNode;



/* typedefs for splitting_sidemin.c */
struct TCNode;
typedef tripleNode* keyType;

typedef struct TCNode {
	struct TCNode *left;
	struct TCNode *right;
	int value;
	keyType key;
	struct TCNode *next;
	struct TCNode *parent;
} TCartesianNode;

typedef struct {
	TCartesianNode *root;
	TCartesianNode *first;
	TCartesianNode *last;
} TCartesianTree;

typedef struct {
	tripleNode *first;
	tripleNode *last;
} matchList ;

typedef struct {
	tripleNode *matchlist;
	matchList *row;
	tripleNode ***row_ti;
	matchList *Mt;
} splittingResultStruct;
 


/* functions in splitting_sidemin.c */
void Push(TCartesianTree *CT, int v, keyType k);
void Eject(TCartesianTree *CT);
int isEmpty(TCartesianTree *CT);
int findMin(TCartesianTree *CT);
keyType findKeyOfMin(TCartesianTree *CT);
keyType firstKey(TCartesianTree *CT);
keyType lastKey(TCartesianTree *CT);
void Empty(TCartesianTree *CT);
TCartesianNode *newCartesianNode(int v, keyType k, TCartesianNode *l, TCartesianNode *r, TCartesianNode *p);
TCartesianNode *Add(TCartesianNode *node, int v, keyType k);
TCartesianNode *Delete(TCartesianNode *node);
TCartesianTree *newCartesianTree();
splittingResultStruct *process(unsigned char *P, unsigned char **T, int m, int n, int K, int gap, int songonce);
splittingResultStruct *process_ti(unsigned char *P,unsigned char **T, int m, int n, int K, int alpha, int songonce);
void c_splitting_free_ti(splittingResultStruct *process_results, int max_transposition, int K);
void c_splitting_free(splittingResultStruct *process_results, int m);

