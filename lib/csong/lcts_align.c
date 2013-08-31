/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen

   Contacts: vmakinen at cs helsinki fi

   Computes min_{j} d_{ID}(A+t,B_{j...n}) at given transposition t and
   prints the trace.


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


#include <string.h>
#include <stdlib.h>

#define GAP (char)-2


typedef struct {
	int x;
	int l;  
} cell;


/* 
   Gets A, B, and transposition t as parameters and
   returns edit trace of A and B_{j...n} in align_A and align_B, 
   as well as the start position of the alignment in startposition.
   Returns the number of errors.
*/
int align(char *A, char *B, char *align_A, char * align_B,int t, int *startposition)
{
	int m = strlen(A), n = strlen(B);
	int i,j,k,ii, diag;
	char temp;

	cell **d = (cell**)malloc((m+1)*sizeof(cell*));

	for (i=0;i<=m;i++)
	d[i] = (cell*)malloc((n+1)*sizeof(cell));
      
	/* initialize */
	for (i=0;i<=m;i++) 
	{
		d[i][0].x = i;
		d[i][0].l = 2;
	}
	
	for (j=0;j<=n;j++) d[0][j].x = 0;

	/* compute and store trace */   
	for (i=1;i<=m;i++)
		for (j=1;j<=n;j++) 
		{
			if ((int)A[i-1]+t==(int)B[j-1]) diag = d[i-1][j-1].x; 
			else diag = d[i-1][j-1].x+3; 

			if (diag <= d[i][j-1].x+1 && diag <= d[i-1][j].x+1) 
			{
				d[i][j].l = 0;
				d[i][j].x = diag;
			} 
			else if (d[i][j-1].x+1<=d[i-1][j].x+1) 
			{
				d[i][j].l = 1;
				d[i][j].x = d[i][j-1].x+1;
			} 
			else 
			{
				d[i][j].l = 2;
				d[i][j].x = d[i-1][j].x+1;
			}
		}

	/* trace optimal path */
	i = m; j = n;
	ii = 0;

	while (i>0) 
	{
		if (d[i][j].l == 0) 
		{
			align_A[ii] = A[i-1];
			align_B[ii++] = B[j-1];
			i--;j--;
		} 
		else if (d[i][j].l == 1) 
		{
			align_A[ii] = GAP;         
			align_B[ii++] = B[j-1];
			j--;
		} 
		else if (d[i][j].l == 2) 
		{
			align_A[ii] = A[i-1];         
			align_B[ii++] = GAP;
			i--;
		}
	}

	*startposition = j;
	align_A[ii] = '\0';
	align_B[ii] = '\0';
    
	/* reverse alignments */
	for (i=0;i<ii/2;i++) 
	{
		temp = align_A[i];
		align_A[i] = align_A[ii-i-1];
		align_A[ii-i-1] = temp;   
	}   

	for (i=0;i<ii/2;i++) 
	{
		temp = align_B[i];
		align_B[i] = align_B[ii-i-1];
		align_B[ii-i-1] = temp;   
	}   

	/* free up */
	k = d[m][n].x;

	for (i=0;i<=m;i++) free(d[i]);
	free(d);
   
	return k;  /* return the number of errors */  
}


