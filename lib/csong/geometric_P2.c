/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Implements an algorithm described in Esko Ukkonen, Kjell Lemstrom and Veli Makinen: 
   Sweepline the Music! In Computer Science in Perspective (LNCS 2598), 
   R. Klein, H.-W. Six, L. Wegner (Eds.), pp. 330-342, 2003.
   Consult the article for description. 
*/

#include "song.h"
#include "priority_queue.h"


/* Struct for items of pointer array q. */
typedef struct {
	/* pointers to chord, note inside the chord and how many notes to next chord (excluding current) */
	unsigned int chordspos;
	unsigned int notespos;
	unsigned int notesleft;
	unsigned int chordind;
} qitem;


/* Struct for difference vectors. */
typedef struct {
	int strt;
	char ptch;
} ivector;



/*
   Initialization phase of geometric algorithm P2: creates priority queue.
*/
VALUE c_geometric_p2_init(VALUE self, VALUE init_info)
{
	unsigned int leaves, pattern_notes;
	treeNode *tree = NULL;

	pattern_notes = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));

	leaves = 1 << (PQ_log_2(pattern_notes) + 1);
	tree = PQ_CreateCompleteBinaryTree(leaves);

	rb_iv_set(init_info, "@t", rb_str_new((char *) tree, leaves * 2 * sizeof(treeNode)));
	free(tree); // rb_str_new seems to make a copy so we can free this here
	rb_iv_set(init_info, "@e", UINT2NUM(leaves));

	return init_info;
}


/*
   Scanning phase of geometric algorithm P2. Described in Esko Ukkonen, Kjell Lemstrom and Veli Makinen: 
   Sweepline the Music! In Computer Science in Perspective (LNCS 2598), R. Klein, H.-W. Six, L. Wegner (Eds.), pp. 330-342, 2003.

   Unlike in the article, end of source is not detected by putting (infinity,infinity) to the end of source, but with indexes.
   Also because the source format consists of variable length chords the implementation is 
   fairly complex and differs somewhat from the pseudocode. 
   Consult the article for details.
*/
VALUE c_geometric_p2_scan(VALUE self, VALUE init_info)
{
	VALUE result_list = Qnil, matchednotes_obj[MAX_PATTERN_NOTES];
	char *chords;
	unsigned int matchednotes[MAX_PATTERN_NOTES];
	unsigned int loopind, num_loops, i = 0, j, pattern_chords, num_notes, pattern_notes, c, errors, min_pattern_size;
	unsigned int quarternoteduration, chords_size, min_key, leaves, minchordind = 0, maxchordind = 0;

	treeNode *tree = NULL;
	vector *p, pattern[MAX_PATTERN_NOTES];
	qitem q[MAX_PATTERN_NOTES];
	ivector prev, min;

	/* note: value infinity was added to the end of pattern in initialization */
	pattern_chords = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	pattern_notes = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));

	if (pattern_chords > chords_size || pattern_notes > MAX_PATTERN_NOTES) return Qnil;

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	num_notes = NUM2UINT(rb_iv_get(self, "@num_notes"));
	quarternoteduration = NUM2UINT(rb_iv_get(self, "@quarternoteduration")) ;

	p = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_polyphonic_vector"));
	errors = NUM2UINT(rb_iv_get(init_info, "@errors"));
	min_pattern_size = pattern_notes - errors;

	/* priority queue: get and initialize. cannot init with memset due to data types. */
	leaves = NUM2UINT(rb_iv_get(init_info, "@e"));
	tree = (treeNode *) RSTRING_PTR(rb_iv_get(init_info, "@t"));
	for (i = 0; i < 2 * leaves; i++)
	{
		tree[i].strt = INT_MAX;
		tree[i].ptch = CHAR_MAX;
	}

	c = 1;	/* value is insignificant since at the start, loop will branch to else due to prev=-infinity. */
	prev.strt = INT_MIN;
	prev.ptch = CHAR_MIN;

	for (i = 0; i < pattern_notes; i++)
	{
		pattern[i].strt = p[i].strt * quarternoteduration / PNOTERESOLUTION;
		pattern[i].ptch = p[i].ptch;

		/* initialize q array: all point to the first note of the source */
		q[i].chordind = 0;
		q[i].chordspos = 0;		/* relative pointer. refer to start of chord with (chords + q[i].chordspos).*/
		q[i].notespos = CHORDHEADERLEN; /* relative pointer. refer to actual note with chords[q[i].notespos] */
		q[i].notesleft = chords[0] - 1;

		/* add translation vectors to the priority queue */
		/* MIDI division in different songs may differ. pattern uses 960 units per quarter note, */
		/* and if source uses different resolution, the pattern resolution is changed to correspond to source resolution. */
		PQ_updateValue(tree, leaves, i, (int) *((unsigned int *) (chords + q[0].chordspos + 1)) - \
			pattern[i].strt, chords[q[i].notespos] - pattern[i].ptch);
	}

	num_loops =  num_notes * pattern_notes;

	/* main loop: how many times you can take items away from the priority queue. */
	/* pattern_notes items are added before, pattern_notes * num_notes - pattern_notes items are added in the loop. */
	for (loopind = 0; loopind < num_loops; loopind++)
	{
		/* get the smallest translation vector */
		/* min_key refers to the index of the pattern note being handled. */
		/* equal difference vectors come out of priority queue in min_key order. */
		PQ_getMin(tree, &min_key, &min.strt, &min.ptch);


		/* update counter */
		if (prev.strt == min.strt && prev.ptch == min.ptch)
		{
			/* this is second matching note */
			maxchordind = q[min_key].chordind;
			c++;
			matchednotes[c - 1] = (unsigned int) q[min_key].notespos;
		}
		else
		{
			/* check match */
			if (c >= min_pattern_size)
			{ 
				/* add match to result list */
				for (j = 0; j < c; j++)
					matchednotes_obj[j] = UINT2NUM(matchednotes[j]);
				if (result_list == Qnil) result_list = rb_iv_get(init_info, "@matches");

				rb_ary_push(result_list, rb_ary_new3(6, self, UINT2NUM(minchordind), UINT2NUM(maxchordind), \
					//Qnil, INT2FIX((int) prev.ptch), INT2FIX((int) pattern_notes - c)));
					rb_ary_new4(c, matchednotes_obj), INT2FIX((int) prev.ptch), INT2FIX((int) pattern_notes - c)));
			}
	
			prev.strt = min.strt;
			prev.ptch = min.ptch;
			minchordind = maxchordind = q[min_key].chordind;
			matchednotes[0] = (unsigned int) (q[min_key].notespos);
			c = 1;
		}


		/* update q pointer: q[h] = next(q[h]): move to next position in the source (from the current position pointed by q). */
		if (q[min_key].notesleft > 0)
		{
			q[min_key].notesleft--;
			q[min_key].notespos += NOTELEN;
			PQ_updateValue(tree, leaves, min_key, (int) *((unsigned int *) (chords + q[min_key].chordspos + 1)) - \
					pattern[min_key].strt, chords[q[min_key].notespos] - pattern[min_key].ptch);
		}
		else if (q[min_key].chordind < chords_size - 1 || (q[min_key].chordind == chords_size - 1 && q[min_key].notesleft > 0))
		{
			/* the current pointer is not at the end of source: move to next chord */ 
			q[min_key].chordind++;
			q[min_key].chordspos = q[min_key].notespos + NOTELEN;
			q[min_key].notespos = q[min_key].chordspos + CHORDHEADERLEN;
			q[min_key].notesleft = *((char *) (chords + q[min_key].chordspos)) - 1;

			/* add difference vector corresponding to pattern note min_key. */
			PQ_updateValue(tree, leaves, min_key,  (int) *((unsigned int *) (chords + q[min_key].chordspos + 1)) - \
					pattern[min_key].strt, chords[q[min_key].notespos] - pattern[min_key].ptch);
		}
		else 
		{
			/* current pointer is at the end of source; remove difference vector from priority queue. */
			PQ_updateValue(tree, leaves, min_key, INT_MAX, CHAR_MAX);
		}
	}

	return result_list;
}


