/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen
   Modified by Mika Turkia (convert matchList *Mt to non-static, Sigma removed completely etc.)

   Contacts: vmakinen at cs helsinki fi

   Implements the computation of transposition invariant LCS(A,B) in 
   O(mn log m) time. The corresponding search problem is also solved.
   See Makinen, Navarro, Ukkonen: "Transposition invariant string matching", 
   STACS 2003 (to appear), for details.

   Usage:
   To compare two songs: computeAllTranspositions(....)
   To search for a pattern in a song: searchAllTranspositions(....)

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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include "lcts.h"


/* Produces all the match sets M_{t} in reverse column-by-column order. */
static void preProcessAllTranspositions(matchList *Mt, char *A,char *B, int m, int n)
{
	int i,j,t;

	for (t=0; t<MAX_TRANSPOSITION; t++)
	{
		Mt[t].first = (keyTypeNode*)malloc(sizeof(keyTypeNode));
		Mt[t].last = Mt[t].first;
		Mt[t].last->next = NULL;
		Mt[t].last->key.i = 0;
		Mt[t].last->key.j = 0;
	}

	for (j=1; j<=n; j++) 
		for (i=m; i>=1; i--) 
		{
			t = (int)B[j]-(int)A[i]+MAX_TRANSPOSITION/2;
			Mt[t].last->next = (keyTypeNode*)malloc(sizeof(keyTypeNode));
			Mt[t].last = Mt[t].last->next;
			Mt[t].last->next = NULL;
			Mt[t].last->key.i = i;
			Mt[t].last->key.j = j;	 
		}   
   
	/* let's add (m+1, n+1) */
	for (t=0; t<MAX_TRANSPOSITION; t++)
	{
		Mt[t].last->next = (keyTypeNode*)malloc(sizeof(keyTypeNode));
		Mt[t].last = Mt[t].last->next;
		Mt[t].last->next = NULL;
		Mt[t].last->key.i = m+1;
		Mt[t].last->key.j = n+1;	 
	}
	return;
}
 

/*
   Computes d_{ID}(A+t,B) using one-dimensional range searching.
   Time complexity O(|M|\log m).
*/
static int processSparseFast(int m, int n, matchList *M)
{
	keyTypeNode *match; 
	int i,d;
	int *values=(int*)malloc(sizeof(int)*(m+2));
	unsigned int leaves = 1<<(log_2(m)+1);
	treeNode *A = CreateCompleteBinaryTree(leaves);
   
	values[0] = 0;
	insertLeaf(A,leaves, 0); 
	for (i=1;i<=m+1;i++) values[i]=INT_MAX;
	match = M->first->next;

	while (match != NULL)
	{
		i = predecessor(A,leaves,match->key.i);
		insertLeaf(A,leaves, match->key.i); 

		if (values[i]-2<values[match->key.i]) 
		{
			values[match->key.i] = values[i]-2;
			deleteGreaterSuccessors(A,leaves,match->key.i,values);
		}	 
		match = match->next;
	}
	d = values[m+1]+m+n+2;
	free(values);
	free(A);
	return d;
}


/*
   Reports {j} such that d_{ID}(A + t, T_{j'...j}) <= k.
   Time complexity is O(|M|\log m).
*/
static void searchOccurrences(int m, int k, int t, matchList *M, occType* occ)
{
	keyTypeNode *match; 
	int i,d,value;
	int *values=(int*)malloc(sizeof(int)*(m+2));
	unsigned int leaves = 1<<(log_2(m)+1);
	treeNode *A = CreateCompleteBinaryTree(leaves);
   
	values[0] = 0;
	insertLeaf(A,leaves, 0); 
	for (i=1;i<=m+1;i++) values[i]=INT_MAX;
	match = M->first->next;

	/* discard the pair (m+1,n+1) since it is only used in the distance computation (hence using match->next != NULL) */
	while (match->next != NULL) 
	{
		i = predecessor(A,leaves,match->key.i);

		/* let's check if cheeper to start a new occurrence */
		d = min2(values[i]-2,-match->key.j-1);
		insertLeaf(A,leaves, match->key.i); 

		if (d<values[match->key.i]) 
		{
			values[match->key.i] = d;
			deleteGreaterSuccessors(A,leaves,match->key.i,values);
		}	 

		/* We should report an interval [key.j,j] on the last row, 
		where the current point induces occurrences
		d_{ID}(A+t,T_{j''...j'})<=k, where j' in [key.j,j].
		However, since d_{ID}(A+t,T_{j''...key.j}) will be the best
		occurrence induced by the current point, let's just report it. */
		value = d+match->key.j+m;

		if (value <= k && value < occ[match->key.j].value) 
		{
			occ[match->key.j].value = value;
			occ[match->key.j].t = t;
		}
		match = match->next;
	}
	free(values);
	free(A);
}


/* Frees memory. */
static void cleanUp(matchList *Mt)
{
	int i;
	keyTypeNode *knode,*ktemp;

	for (i=0;i<MAX_TRANSPOSITION;i++)
	{
		knode = Mt[i].first;
		while (knode!=NULL) 
		{
			ktemp = knode;
			knode = knode->next;
			free(ktemp); 
		}
	}
	return;
} 


/*
   Given two music pieces, this function will tell their distance
   min = min_{t\in T} {d_{ID}(A+t,B) = m+n-2*LCS(A+t,B)}
*/
int computeAllTranspositions(matchList *Mt, char *A, char *B, int m, int n)
{
	int t,min=INT_MAX, d;
      
	preProcessAllTranspositions(Mt, A,B,m,n);
    
	for (t=0;t<MAX_TRANSPOSITION;t++)
	{
		if (Mt[t].first->next == Mt[t].last) continue;
		d = processSparseFast(m,n,&(Mt[t]));
		if (d<min) min = d;
	}
	cleanUp(Mt);
	return min;
}


/*
   This function will search all approximate occurrences of P in T.
   An occurrence is at j iff d_{ID}(P+t,T_{j'...j}) <= k for some j', t.
   We will only report minimal occurrences 
   (those that can not be derived from other (better) occurrences.
*/
occType* searchAllTranspositions(matchList *Mt, char *P, char *T, int m, int n, int k)
{
	int j, t;
	occType *occ = (occType*)malloc((n+1)*sizeof(occType));
      
	preProcessAllTranspositions(Mt, P,T,m,n);
	for (j=1; j<=n; j++)
	{
		occ[j].value = INT_MAX;
		occ[j].t = 0;
	}
    
	for (t=0;t<MAX_TRANSPOSITION;t++)
	{
		if (Mt[t].first->next == Mt[t].last) continue;
		searchOccurrences(m,k,t - MAX_TRANSPOSITION / 2,&(Mt[t]),occ);
	}
	cleanUp(Mt);
	return occ; /* returns the array occ[1...n] of occurrences */
}


