/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen
   Modifications by Mika Turkia (removal of static variables; malloc->calloc)

   Contacts: vmakinen at cs helsinki fi

   Implements an O(|M|) time algorithm described in
   K. Lemström and V. Mäkinen: On Minimum Splitting of Pattern
   in Multi-Track String Matching, Accepted to CPM'2003,
   Morelia, Mexico, June 25-27, 2003.

   Here M={(i,j,k) | p_i=t^k_j}, that is the set of matching character
   pairs between pattern string P and each text T^k, 1<=k<=K.
   The algorithm tries to find a splitting of P into kappa pieces so
   that each piece is found in some text track T^k, and the occurrences
   of the pieces are consecutive having at most an alpha-gap between
   two pieces. The objective is to minimize kappa.
*/


/* To handle alpha-gaps, we use a linear time sliding window minima algorithm. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>

#include <splitting.h>

#define max2(a,b) ((a)>(b)?(a):(b))
#define min2(a,b) ((a)<(b)?(a):(b))


static tripleNode  *newtripleNode()
{
	tripleNode *node = (tripleNode*) calloc(1, sizeof(tripleNode));
	// correct values for first row!
	// calloc takes care of these: node->next = NULL; node->prevD = NULL; node->kappa = 0; node->prevTrace = NULL;
	return node;
}


static tripleNode  *newtripleNode2(int i, int j, int k, int kappa, tripleNode *next)
{
   tripleNode *node = (tripleNode*) calloc(1, sizeof(tripleNode));
   node->i = i; node->j = j; node->k = k; node->kappa = kappa; node->next = next;
   node->prevTrace = NULL;
   return node;
}


static tripleNode *rowCopy(tripleNode *node, int i)
{
   return newtripleNode2(i,node->j,node->k,node->kappa,node->next);
};


/*
   Assuming integer alphabet Sigma, each symbol keeps a list of its occurrences in multi-track text T.
   m: length of pattern
   n: length of multi-track text
   K: number of tracks
*/

splittingResultStruct *process(unsigned char *P,unsigned char **T, int m, int n, int K, int alpha, int songonce)
{
   matchList Sigma[128];
   tripleNode *temp, *temp2, *optTrace = NULL;
   matchList *row = (matchList*) calloc(m+1, sizeof(matchList));
   TCartesianTree *CT = newCartesianTree();
   tripleNode **track = (tripleNode**) calloc(K+1, sizeof(tripleNode*));
   keyType key;
   int kappa;
   splittingResultStruct *process_results = NULL;
   int **gap_counter = (int**)calloc(K+1,sizeof(int*));
   int i,j,k,s;

   /***********************************************************************
    * this produces the match set M in row-by-row order,                  *
    * and does the computation on-the-fly                                 *
    ***********************************************************************/
//   printf("--start\n"); for (j=1;j<=n;j++) for (k=1;k<=K;k++) printf("T[%d][%d]=%d\n", k,j, T[k][j]); printf("--end\n");


   process_results = (splittingResultStruct *) calloc(1, sizeof(splittingResultStruct));
   process_results->row = row;

   for (k=1;k<=K; k++)
      gap_counter[k] = (int*)calloc(n+1,sizeof(int));
   
   for (j=0;j<=n;j++)
      for (k=1;k<=K;k++)
         gap_counter[k][j] = 0;
      
   for (s=0;s<128;s++) {
      Sigma[s].first = NULL;
      Sigma[s].last = NULL;
   }

   // the following collects the list of occurrences of each character in T
   for (j=1;j<=n;j++)
      for (k=1;k<=K;k++) {
	 if (T[k][j] == GAP_UNSIGNED) {
	    gap_counter[k][j] = gap_counter[k][j - 1] + 1;
	    continue;
	 } else
	    gap_counter[k][j] = 0;   
         temp = newtripleNode();
         temp->next = NULL;
         temp->j = j;
         temp->k = k;
	 temp->kappa = INT_MAX;
         if (Sigma[(unsigned int)T[k][j]].last != NULL) {
            Sigma[(unsigned int)T[k][j]].last->next = temp;
            Sigma[(unsigned int)T[k][j]].last = temp;
         }
         else {
            Sigma[(unsigned int)T[k][j]].first = temp;
            Sigma[(unsigned int)T[k][j]].last = temp;
         }
      }

   // the following constructs the match set for each row separately
   for (i=1;i<=m;i++) {
      temp = Sigma[(unsigned int)P[i]].first;
      if (temp == NULL) return process_results; // no occurrences possible
      row[i].first = rowCopy(temp,i);
      row[i].last = row[i].first;
      temp = temp->next;
      while (temp!=NULL) {
         row[i].last->next = rowCopy(temp,i);
         row[i].last = row[i].last->next;
         temp = temp->next;
      }
   }

   // let's assign the prevD pointers by simulating the
   // merging of each two consecutive rows
   // !!! No need for this anymore, see the modification below.
   /*
   for (i=2;i<=m;i++) {
      temp = row[i-1].first;
      temp2 = row[i].first;
      while (temp!=NULL && temp2!=NULL) {
         if (temp->j == temp2->j-1 && temp->k == temp2->k) {
            temp2->prevD = temp;
            temp = temp->next;
            temp2 = temp2->next;
         }
         else if (temp->j < temp2->j-1 || (temp->j == temp2->j-1 && temp->k < temp2->k))
            temp = temp->next;
         else
            temp2 = temp2->next;
      }
   }*/

   // Ready to do the computation!
   // Have to make similar simulation of merging as above to sweep
   // through two consecutive rows simultaneously.
   // Also sliding window minima need to be maintained.

   // A small modification to the algorithm in CPM03:
   // We allow skipping over characters '-' for free,
   // i.e. over those that are just added to align the tracks.
   // This is done by keeping for each row/track pointer to the
   // previous match in track[k].
   for (i=2;i<=m;i++) {
      for (k=1; k<=K; k++)
         track[k] = NULL;
      temp = row[i-1].first;
      temp2 = row[i].first;
      while (temp2!=NULL) {
         // add values to min-queue
         while (temp != NULL && temp->j < temp2->j) {
            //key.x = temp->j; key.y = temp->k;
            key = temp;
            Push(CT,temp->kappa,key);
            track[temp->k] = temp;
            temp = temp->next;
         }
         // remove value from min-queue
         while (!isEmpty(CT) && firstKey(CT)->j<temp2->j-alpha-1)
            Eject(CT);
         // now, value of temp2->kappa is minimum of temp2->prevD->kappa and
         // CT->findMin()+1
         // modification: track[k] instead of temp2->prevD->kappa
         if (!isEmpty(CT)) {
            temp2->kappa = findMin(CT)+1;
            temp2->prevTrace = findKeyOfMin(CT);
         }
         else temp2->kappa = m+1; // no splitting up to this point

//printf("T[%d][%d]=%d p[%d]=%d\n", temp2->k, temp2->j, T[temp2->k][temp2->j], temp2->i, P[temp2->i]);

         if (track[temp2->k] != NULL && track[temp2->k]->j >= temp2->j-gap_counter[temp2->k][temp2->j-1]-1 && track[temp2->k]->kappa<temp2->kappa) {
            temp2->kappa = track[temp2->k]->kappa;
            temp2->prevTrace = track[temp2->k];
         }
         temp2 = temp2->next;
      }
      Empty(CT);
   }

   
   if (songonce)
   {
           // find the smallest kappa at the last row
           temp = row[m].first;
           optTrace = NULL;
	   kappa = m+1;
	   while (temp!=NULL) {
	      if (temp->kappa < kappa) {kappa = temp->kappa; optTrace = temp;};
	      temp = temp->next;
	   }
   	   //containing min kappa if songonce
           process_results->matchlist = optTrace;
   }
   // else return first match; all matches will be processed in wrapper function.
   else process_results->matchlist = row[m].first;

   // let's empty all data structures
   free(CT);
   free(track);

   for (s=0;s<128;s++) {
      temp = Sigma[s].first;
      while (temp != NULL) {
         temp2 = temp;
         temp = temp->next;
         free(temp2);
      }
   }

for (k=1; k <= K; k++) free(gap_counter[k]);
free(gap_counter);

   return process_results; 
}

static void preProcessAllTranspositions(unsigned char *P,unsigned char **T, int m, int n, int K, matchList *Mt, tripleNode ***lastrow) {

	 tripleNode *temp;
   /**************************************************************
    * this produces all the match sets M_{t} in row-by-row order *
    **************************************************************/

   int i,j,k,t;
   for (i=1;i<=m;i++)   
      for (j=1;j<=n;j++)
         for (k=1;k<=K;k++) {
	        if (T[k][j] == GAP_UNSIGNED) continue;  
            temp = newtripleNode();
            temp->next = NULL;
            temp->i = i;
            temp->j = j;
            temp->k = k; 
            // initialize first row
            if (temp->i == 1) temp->kappa = 0;
	    else temp->kappa = INT_MAX;
            // store last row to array (for reporting) 
            lastrow[temp->k][temp->j] = temp; 
            t = (int)T[k][j]-(int)P[i]+MAX_TRANSPOSITION/2;           
            if (Mt[t].last != NULL) {
               Mt[t].last->next = temp;
               Mt[t].last = temp;
            }
            else {
               Mt[t].first = temp;
               Mt[t].last = temp;
            }
         }
   return;
}

splittingResultStruct *process_ti(unsigned char *P,unsigned char **T, int m, int n, int K, int alpha, int songonce)
{
   tripleNode *temp, *temp2;
   tripleNode ***lastrow = (tripleNode***) calloc(K+1, sizeof(tripleNode**));
   matchList *row = (matchList*) calloc(m+1, sizeof(matchList));
   matchList *Mt = (matchList*) calloc(MAX_TRANSPOSITION, sizeof(matchList));
   TCartesianTree *CT = newCartesianTree();
   tripleNode **track = (tripleNode **) calloc(K+1, sizeof(tripleNode *));
   keyType key;
   int kappa;
   splittingResultStruct *process_results = NULL;
   int **gap_counter = (int**)calloc(K+1,sizeof(int*));
   int i,j,k,t;

   /***********************************************************************
    * this produces the match set M in row-by-row order,                  *
    * and does the computation on-the-fly                                 *
    ***********************************************************************/

   for (k=1;k<=K;k++)
      lastrow[k] = (tripleNode**) calloc(n+1, sizeof(tripleNode*)); 

   for (k=1;k<=K; k++)
      gap_counter[k] = (int*)calloc(n+1,sizeof(int));
      
   process_results = (splittingResultStruct *) calloc(1, sizeof(splittingResultStruct));
   process_results->row_ti = lastrow;
   process_results->Mt = Mt;
   process_results->matchlist = NULL;

   // construct match sets for all transpositions
   preProcessAllTranspositions(P,T,m,n,K,Mt,lastrow);

   for (j=0;j<=n;j++)
      for (k=1;k<=K;k++) 
         gap_counter[k][j] = 0;
	     
   for (j=1;j<=n;j++)
      for (k=1;k<=K;k++) 
	 if (T[k][j] == GAP_UNSIGNED) gap_counter[k][j] = gap_counter[k][j - 1] + 1;
	 else gap_counter[k][j] = 0;   
   
   // smallest splitting found
   kappa = m+1;    

   // compute in each transposition t
   for (t=0;t<MAX_TRANSPOSITION; t++) {
      // the following constructs the match set for each row separately
      for (i=1; i<=m; i++) {
         row[i].first = NULL;
         row[i].last = NULL;
      }
      temp = Mt[t].first;

	while (temp != NULL) 
	{
		if (row[temp->i].first == NULL) 
		{
			row[temp->i].first = temp;
			row[temp->i].last = temp;
		}
		temp = temp->next;
	}

      // Ready to do the computation!

      // A small modification to the algorithm in CPM03:
      // We allow skipping over characters '-' for free,
      // i.e. over those that are just added to align the tracks.
      // This is done by keeping for each row/track pointer to the
      // previous match in track[k].
       
      for (i=2;i<=m;i++) {
         temp = row[i-1].first;
         for (k=1;k<=K;k++)
            track[k] = NULL;
         temp2 = row[i].first;
	 while (temp2!=NULL && temp2->i == i) {
	   // add values to min-queue
	   while (temp != NULL && temp->i == i-1 && temp->j < temp2->j) {
               //key.x = temp->j; key.y = temp->k;
               key = temp;
               Push(CT,temp->kappa,key);
               track[temp->k] = temp;
               temp = temp->next;
            }
            // remove value from min-queue
            while (!isEmpty(CT) && firstKey(CT)->j<temp2->j-alpha-1)
               Eject(CT);
	        // now, value of temp2->kappa is minimum of temp2->prevD->kappa and
	        // CT->findMin()+1
	        // modification: track[k] instead of temp2->prevD->kappa
	        if (!isEmpty(CT)) {
	           temp2->kappa = findMin(CT)+1;
	           temp2->prevTrace = findKeyOfMin(CT);
	        }
	        else temp2->kappa = m+1; // no splitting up to this point

//if (track[temp2->k]) printf("temp2k=%d temp2j=%d gapcounter=%d kappa=%d trackkappa=%d i=%d trackj=%d \n", 
//	temp2->k, temp2->j, gap_counter[temp2->k][temp2->j-1]-1, temp2->kappa,  track[temp2->k]->kappa, i, track[temp2->k]->j);

	        if (track[temp2->k] != NULL && track[temp2->k]->j >= temp2->j-gap_counter[temp2->k][temp2->j-1]-1 && track[temp2->k]->kappa<temp2->kappa) {
	           temp2->kappa = track[temp2->k]->kappa;
	           temp2->prevTrace = track[temp2->k];
	        }
	        temp2 = temp2->next;
	     }
	     Empty(CT);
	  }

	  if (songonce)
	  {
	       // find the smallest kappa at the last row
	       temp = row[m].first;
		   while (temp!=NULL) {
		      if (temp->kappa < kappa) {
		         kappa = temp->kappa; 
                         //containing min kappa if songonce
	                 process_results->matchlist = temp;
		      };
		      temp = temp->next;
		   }

	   }
   }   
   

   // let's empty all data structures
   free(CT);
   free(track);
   for (k=1; k <= K; k++) free(gap_counter[k]);
   free(gap_counter);
   free(row);

   if (!songonce) {
	  
	  process_results->row_ti = lastrow;
	   // else return first match; all matches will be processed in wrapper function. 

   }
   return process_results; 
}


void c_splitting_free(splittingResultStruct *process_results, int m)
{
	int i;
	tripleNode *temp, *temp2;
	
	/* These should be emptied only after the optimal path is extracted */
	for (i = 1; i <= m; i++)
	{
		temp = process_results->row[i].first;
		while (temp != NULL)
		{
			temp2 = temp;
			temp = temp->next;
			free(temp2);
		}
	}
	
	free(process_results->row);
	free(process_results);
}

void c_splitting_free_ti(splittingResultStruct *process_results, int max_transposition, int K)
{
	int t,k;
	tripleNode *temp, *temp2;
	
	/* These should be emptied only after the optimal path is extracted */
	for (t = 0; t < max_transposition; t++)
	{
		temp = process_results->Mt[t].first;
		while (temp != NULL)
		{
			temp2 = temp;
			temp = temp->next;
			free(temp2);
		}
	}
	
	free(process_results->Mt);
	
	for (k=1;k<=K; k++) free(process_results->row_ti[k]);
	   
	free(process_results->row_ti);
	free(process_results);
}

