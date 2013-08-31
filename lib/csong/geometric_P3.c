/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.5, June 25th, 2003

   Copyright Mika Turkia
   Priority queue functions copyright Veli Makinen, modified by Mika Turkia

   Contacts: turkia at cs helsinki fi, vmakinen at cs helsinki fi

   Implements an algorithm P3 described in Esko Ukkonen, Kjell Lemstrom and Veli Makinen: 
   Sweepline the Music! In Computer Science in Perspective (LNCS 2598), 
   R. Klein, H.-W. Six, L. Wegner (Eds.), pp. 330-342, 2003.
   Consult the article for description. 


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

#include "geometric_P3_priority_queue.h"

VALUE c_geometric_p3_init(VALUE self, VALUE init_info)
{
	//unsigned int pattern_notes;
	VerticalTranslationTableItem *verticaltranslationtable = (VerticalTranslationTableItem *) calloc(256, sizeof(VerticalTranslationTableItem));
	rb_iv_set(init_info, "@t", rb_str_new((char *) verticaltranslationtable, 256 * sizeof(VerticalTranslationTableItem)));
	//pattern_notes = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));
	//for (i = 0; i < 256; i++) verticaltranslationtable[i].matchednotes = (unsigned int *) malloc(pattern_notes * sizeof(unsigned int)); 
	//this did not work, unclear why.
	free(verticaltranslationtable);
	return init_info;
}


/*
   Scanning phase of geometric algorithm P3. Described in Esko Ukkonen, Kjell Lemstrom and Veli Makinen: 
   Sweepline the Music! In Computer Science in Perspective (LNCS 2598), R. Klein, H.-W. Six, L. Wegner (Eds.), pp. 330-342, 2003.

   Algorithm described in this article had a flaw that allowed total duration to exceed the total duration of the pattern.
   This problem has been solved by merging overlapping segments in the note data during the calculation of turning points. 
   However overlapping notes in the pattern are currently not checked for.
   The problem can also be solved using counters; see the prototypes folder in the distribution for an example.

   Rough description of the algorithm:
   First a priority queue is created, which will give translations in sorted order. Translation vectors for first source note and
   all pattern notes are inserted to the queue.
   The priority queue is then processed with the help of array of vertical translations, 
   which stores value, slope and previous x translation for each vertical translation y. 
   For each vector extracted from the priority queue, we check the type of the turning point associated with this vector and
   adjust the slope and value accordingly. Then we move on in the source, and add to the queue a new translation vector for the new source note and 
   the pattern note associated with the previous translation vector. This loop is repeated until the end of source.
   After the processing we have acquired the longest common time for this pattern and source.
   This method returns only the best match for each song.  
   Consult the article for details.
*/
VALUE c_geometric_p3_scan(VALUE self, VALUE init_info)
{
	unsigned int loopind, pattern_chords, num_tpoints, num_tpoints_minusone, pattern_notes, i, j;
	unsigned int quarternoteduration, chords_size, num_loops;
	unsigned int endchordind = 0;
	unsigned int startchordind = 0;
	unsigned int matchednotes[MAX_PATTERN_NOTES];
	int best = 0, dursum = 0, halfdursum;
	int transposition = INT_MAX;
	priority_queue *pq = NULL;
	vector *p = NULL, pattern[MAX_PATTERN_NOTES];
	treenode min;
	VerticalTranslationTableItem *verticaltranslationtable = NULL, *item = NULL;
	TurningPoint *startpoints = NULL, *endpoints = NULL;
	TurningPointPointer turningpointpointers[MAX_PATTERN_NOTES], *tpp = NULL;
	memset((void *) turningpointpointers, 0, MAX_PATTERN_NOTES);

	/* note: value infinity was added to the end of pattern in initialization */
	pattern_chords = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	pattern_notes = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));

	if (pattern_chords > chords_size || pattern_notes >= MAX_PATTERN_NOTES) return Qnil;

	num_tpoints = NUM2UINT(rb_iv_get(self, "@preprocessed_p3_num_turningpoints"));
	quarternoteduration = NUM2UINT(rb_iv_get(self, "@quarternoteduration")) ;
	p = (vector *) RSTRING(rb_iv_get(init_info, "@pattern_polyphonic_vector"))->ptr;

	for (j=0; j<pattern_notes;j++) matchednotes[j]=0;

	/* create priority queue */
	pq = p3_create_priority_queue(pattern_notes * 4);

	/* initialize Y array */
	verticaltranslationtable = (VerticalTranslationTableItem *) RSTRING(rb_iv_get(init_info, "@t"))->ptr;
	for (i = 0; i < 256; i++)
	{
		verticaltranslationtable[i].value = 0;
		verticaltranslationtable[i].slope = 0;
		verticaltranslationtable[i].prev_x = 0;
	}

	/* convert strt values in the pattern */
	dursum = 0;
	for (i = 0; i < pattern_notes; i++)
	{
		pattern[i].strt = p[i].strt * quarternoteduration / PNOTERESOLUTION;
		pattern[i].ptch = p[i].ptch;

		/* NOTE: pattern vector structs are padded wrong; this is not stylish but works */
		pattern[i].dur = *((unsigned short *) ((char *)(&p[i]) + 5)) * quarternoteduration / PNOTERESOLUTION;
		dursum += pattern[i].dur;
	}
	/* used in match reporting; matches with duration at least half of the pattern duration are accepted. */
	halfdursum = dursum * 0.75;

	startpoints = (TurningPoint *) RSTRING(rb_iv_get(self, "@preprocessed_p3_startpoints"))->ptr;
	endpoints = (TurningPoint *) RSTRING(rb_iv_get(self, "@preprocessed_p3_endpoints"))->ptr;


	/* create an array whose items have two pointers each: one for startpoints and one for endpoints. */
	/* each item points to turning point array item */
	tpp = &turningpointpointers[0];
	for (i = 0, loopind = 0; i < pattern_notes; i++)
	{
		//printf("loop %d\n", i);
		turningpointpointers[i].startpoint = &startpoints[0];
		turningpointpointers[i].endpoint = &endpoints[0];

		/* also populate priority queue with initial items. */
		min.vector.tpindex = 0;
		min.vector.patternindex = i;
		min.vector.y = (long int) tpp->startpoint->y - (long int) pattern[i].ptch;


		/* add translation vectors calculated from startpoint */
		min.vector.text_is_start = 1;

		min.vector.x =  (long int) tpp->startpoint->x - (long int) (pattern[i].strt + pattern[i].dur);
		min.vector.pattern_is_start = 0;
		min.key = loopind++;
		p3_update_value(pq, &min);
		//printf("(%d,%d)\n", min.vector.x, min.vector.y);

		min.vector.x =  (long int) tpp->startpoint->x - (long int) pattern[i].strt;
		min.vector.pattern_is_start = 1;
		min.key = loopind++;
		p3_update_value(pq, &min);
		//printf("(%d,%d)\n", min.vector.x, min.vector.y);


		/* add translation vectors calculated from endpoint */
		min.vector.text_is_start = 0;

		min.vector.x =  (long int) tpp->endpoint->x - (long int) (pattern[i].strt + pattern[i].dur);
		min.vector.pattern_is_start = 0;
		min.key = loopind++;
		p3_update_value(pq, &min);
		//printf("(%d,%d)\n", min.vector.x, min.vector.y);
		
		min.vector.x =  (long int) tpp->endpoint->x - (long int) pattern[i].strt;
		min.vector.pattern_is_start = 1;
		min.key = loopind++;
		p3_update_value(pq, &min);
		//printf("(%d,%d)\n", min.vector.x, min.vector.y);
	}

	/* create translation vectors. prev_x should be x on the first loop but since slope is zero it doesn't matter. */
	best = 0;
	num_loops = pattern_notes * num_tpoints * 4;
	num_tpoints_minusone = num_tpoints - 1;

	for (loopind = 0; loopind < num_loops; loopind++)
	{
		/* get minimum element */
		min = pq->tree[1];

		/* update value */
		item = &verticaltranslationtable[127 + min.vector.y];
		item->value += item->slope * (min.vector.x - item->prev_x);
		item->prev_x = min.vector.x;

		/* adjust slope */
		if (min.vector.text_is_start != min.vector.pattern_is_start) item->slope++;
		else item->slope--;

		/* check for best match */
		if (item->value > best || (item->value == best && abs(min.vector.y) < abs(transposition)))
		{
			transposition = min.vector.y;
			best = item->value;
			if (min.vector.text_is_start) endchordind = startpoints[min.vector.tpindex].textchordind;
			else endchordind = endpoints[min.vector.tpindex].textchordind;
		}

		/* move pointer and insert new translation vector according to turning point type. */
		if (min.vector.tpindex < num_tpoints_minusone)
		{
			min.vector.tpindex++;
			tpp = &turningpointpointers[min.vector.patternindex];
			
			if (min.vector.text_is_start)
			{
				tpp->startpoint = &startpoints[min.vector.tpindex];

				min.vector.y = (long int) tpp->startpoint->y - (long int) pattern[min.vector.patternindex].ptch;

				if (min.vector.pattern_is_start)
				{
					min.vector.x =  (long int) tpp->startpoint->x - (long int) pattern[min.vector.patternindex].strt;
					p3_update_value(pq, &min);
				}
				else
				{
					min.vector.x =  (long int) tpp->startpoint->x - (long int) \
						(pattern[min.vector.patternindex].strt + pattern[min.vector.patternindex].dur);
					p3_update_value(pq, &min);
				}
			}
			else
			{
				tpp->endpoint = &endpoints[min.vector.tpindex];

				min.vector.y = tpp->endpoint->y - pattern[min.vector.patternindex].ptch;

				if (min.vector.pattern_is_start)
				{
					min.vector.x =  (long int) tpp->endpoint->x - (long int) pattern[min.vector.patternindex].strt;
					p3_update_value(pq, &min);
				}
				else
				{
					min.vector.x =  (long int) tpp->endpoint->x - (long int) \
						(pattern[min.vector.patternindex].strt + pattern[min.vector.patternindex].dur);
					p3_update_value(pq, &min);
				}
			}
		}
		else
		{
			/* 'remove' translation vector by marking it to be the largest */
			/* it won't be extracted since there are only as many loops as real vectors */
			min.vector.x = INT_MAX;
			p3_update_value(pq, &min);
		}
	}

	/* report best match, i.e. longest common time. scaled to pattern division. common time must be at least half of the pattern time. */
	if (best > halfdursum)
	{
		/* rough approximation */
		startchordind = max2(0, (int) endchordind - (int) pattern_notes);
		endchordind = min2(chords_size, endchordind + pattern_notes);

		/* save match */
		rb_ary_push(rb_iv_get(init_info, "@matches"), rb_ary_new3(7, self, UINT2NUM(startchordind), UINT2NUM(endchordind), \
			Qnil, INT2NUM(transposition), INT2FIX(0), INT2NUM(best / quarternoteduration)));
	}

	/* delete priority queue */
	free(pq->tree);
	free(pq);

	return Qnil;
}


inline static int compare_turningpoints(const void *aa, const void *bb)
{       
	/* sort by x coordinate of the translation vector */
	TurningPoint *a, *b;
	a = (TurningPoint *) aa;
	b = (TurningPoint *) bb;
	if (a->x < b->x) return -1;
	else if (a->x > b->x) return 1;
	else return 0;
}
 

/* Returns a node with minimum translation vector (vector.x,vector.y) and deletes it from the tree. */
inline static treenode p3_get_min(priority_queue *pq)
{
	/* the minimum is always in the root */
	treenode e[1], n =  pq->tree[1];

	/* "delete" from tree by replacing values with pseudo infinity so that it will be extracted last */
	/* other fiels are left as is */
	e->key = n.key;
	e->vector.x = INT_MAX;
	p3_update_value(pq, e);

	return n;
}


/* Creates a priority queue into which a number of items indicated by 'size' parameter can be added. 
   Programmer must take care of not adding too many items; overflows are not checked.
*/
inline static priority_queue *p3_create_priority_queue(unsigned int size)
{
	unsigned int i;
	doubleAndMask fm;
	priority_queue *pq = (priority_queue *) calloc(1, sizeof(priority_queue));

	fm.asDouble = (double) size;
	pq->leaves = 1 << (fm.asMask.exponentbias1023 - 1023 + 1);	/* 1 << (log2(size) + 1) */
	pq->tree = (treenode *) calloc(pq->leaves * 2 , sizeof(treenode));

	/* no leafs in the tree at first */
	for (i = 0; i < 2 * pq->leaves; i++) pq->tree[i].vector.x = INT_MAX;
	return pq;
}


/* Adds or replaces node with given key. */
inline static void p3_update_value(priority_queue *pq, treenode *n)
{
	/* update priority from leaf 'key' to the root */
	unsigned int i = pq->leaves + n->key;
	unsigned int j;

	*(pq->tree + i) = *n;

	while (i > 1)
	{
		/* if i is even, then j is the next odd number larger than i. else j is the previous even number before i. */
		if (i == (i >> 1) << 1) j = i + 1;
		else j = i - 1;

		/* i is the left and j is the right son of i/2. choose the smaller of the sons. */
		if (pq->tree[i].vector.x < pq->tree[j].vector.x)
		{
			if (pq->tree[i>>1].vector.x == pq->tree[i].vector.x && pq->tree[i>>1].vector.y == pq->tree[i].vector.y && pq->tree[i>>1].key == pq->tree[i].key) break; 
			*(pq->tree + (i>>1)) = *(pq->tree + i);
		}
		else
		{ 
			if (pq->tree[i>>1].vector.x == pq->tree[j].vector.x && pq->tree[i>>1].vector.y == pq->tree[j].vector.y && pq->tree[i>>1].key == pq->tree[j].key) break;
			*(pq->tree + (i>>1)) = *(pq->tree + j);
		}

		/* it is possible to break the loop before root with these conditions but testing them takes probably more time than copying, therefore disabled.
		 * put first in if branch and second in else branch of the previous if-else clause before the current statement if you want to use it. */

		/* proceed to the parent */
		i >>= 1;
	}
}


