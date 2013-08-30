/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Contacts: turkia at cs helsinki fi

   Implements an algorithm described in Esko Ukkonen, Kjell Lemstrom and Veli Makinen: 
   Sweepline the Music! In Computer Science in Perspective (LNCS 2598), 
   R. Klein, H.-W. Six, L. Wegner (Eds.), pp. 330-342, 2003.
   Consult the article for description. 
*/

/*
   Bugs: dummy 'infinity' value should be added to the end of source, so that the
   implementation could be simplified to follow pseudocode more closely.
   The decision to not add it made the algorithm overly complex. 
*/

#include "song.h"


/* Struct for q pointers that point to notes. */
/* it might be better to use unsigned int *q whose 
 * items were notepos values but this was meant to make code more readable. */
typedef struct {
	unsigned int strt;
	char ptch;
	/* needed for matched notes because cmatch.c function wants pointers to notes. */
	unsigned int notespos;
} q_vector;

/* Struct for difference vectors. */
typedef struct {
	int strt;
	char ptch;
} ivector;


static unsigned int next_note(char *chords, unsigned int *chords_size, unsigned int *chordind, unsigned int *chordspos, unsigned int *noteind, unsigned int *notespos);



/*
   Scanning phase of geometric algorithm P1. Described in Esko Ukkonen, Kjell Lemstrom and Veli Makinen: 
   Sweepline the Music! In Computer Science in Perspective (LNCS 2598), R. Klein, H.-W. Six, L. Wegner (Eds.), pp. 330-342, 2003.

   Unlike in the article, end of source is not detected by putting (infinity,infinity) to the end of source, but with indexes.
   Also because the source format consists of variable length chords the implementation is 
   fairly complex and differs somewhat from the pseudocode. 
*/
VALUE c_geometric_p1_scan(VALUE self, VALUE init_info)
{
	VALUE zero, result_list = Qnil, matchednotes_obj[MAX_PATTERN_NOTES];
	char *chords;
	unsigned int pi = 0, k = 0, pattern_size, end, quarternoteduration;
	unsigned int chords_size, num_notes, notes = 0;
	unsigned int spos, notespos, chordind, noteind;
	unsigned int tempspos, tempnotespos, tempchordind, tempnoteind;

	vector *pattern, p[MAX_PATTERN_NOTES];
	ivector f;
	q_vector q[MAX_PATTERN_NOTES];

	/* note: value infinity was added to the end of pattern in initialization */
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	if (pattern_size > chords_size || pattern_size > MAX_PATTERN_NOTES) return Qnil;
	
	pattern = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_polyphonic_vector"));
	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	num_notes = NUM2UINT(rb_iv_get(self, "@num_notes"));
	quarternoteduration = NUM2UINT(rb_iv_get(self, "@quarternoteduration"));
	zero = INT2FIX(0);


	/* initialize q array: last to infinity and others to minus infinity; also convert pattern time resolution. */
	for (pi = 0; pi < pattern_size; pi++)
	{ 
		q[pi].strt = 0;
		q[pi].ptch = CHAR_MIN;
		q[pi].notespos = 0;

		/* initially, pattern uses 960 MIDI division units per quarter note, and if source uses different
		   resolution, the pattern resolution is changed to correspond to source resolution.  */
		p[pi].strt = pattern[pi].strt * quarternoteduration / PNOTERESOLUTION;
		p[pi].ptch = pattern[pi].ptch;
	}
	q[pi].strt = UINT_MAX;
	q[pi].ptch = CHAR_MAX;
	q[pi].notespos = 0;


	/* scan all notes */
	for (spos = 0, notespos = CHORDHEADERLEN, noteind = 0, chordind = 0; notes++ <= num_notes - pattern_size; )
	{
		f.strt = (int) *((unsigned int *) (chords + spos + 1)) - (int) p[0].strt; 
		f.ptch = (int) chords[notespos] - (int) p[0].ptch;

		/* mark first matched note; other matched notes can be acquired from q[1]...q[pattern_size - 1] */
		q[0].notespos = notespos;

		pi = 1;
		end = 0;

		/* loop until q > p[pi] + f. */
		/* i.e. start finding points of the transposed pattern starting from this source position and do it until match cannot be found. */
		do
		{
			/* temporary indexes for while loop */
			tempspos = spos;
			tempnotespos = notespos;
			tempchordind = chordind;
			tempnoteind = noteind;

			/* move temp indexes to next note (don't start from the one from which f was calculated) */
			if (next_note(chords, &chords_size, &tempchordind, &tempspos, &tempnoteind, &tempnotespos)) { end = 1; break; }

			/* q[i] is the maximum of q[i] and t[j] */
			if (q[pi].strt < *((unsigned int *) (chords + tempspos + 1)) || \
				(q[pi].strt == *((unsigned int *) (chords + tempspos + 1)) && q[pi].ptch < chords[tempnotespos]))
		       	{ 
				q[pi].strt = *((unsigned int *) (chords + tempspos + 1));
				q[pi].ptch = chords[tempnotespos];
				q[pi].notespos = tempnotespos;
			}

			/* temp loop while q < p[pi] + f, i.e. move pointer over the notes that can't be part of a match for any remaining f. */
			while (((q[pi].strt < (p[pi].strt + f.strt)) || ((q[pi].strt == (p[pi].strt + f.strt)) && (q[pi].ptch < (p[pi].ptch + f.ptch)))))
			{
				/* move to next note: note that strt changes only when moving across chord boundary */
				/* if we hit end of source, we break both loops since there is nothing left to match with. */
				if (next_note(chords, &chords_size, &tempchordind, &tempspos, &tempnoteind, &tempnotespos)) { end = 1; break; }
				q[pi].strt = *((unsigned int *) (chords + tempspos + 1));
				q[pi].ptch = chords[tempnotespos];
				q[pi].notespos = tempnotespos;
			}
			/* after the loop we are at either a matching note or bigger, i.e. there is no matching note. */

		} while (!((q[pi].strt > p[pi].strt + f.strt) || ((q[pi].strt == p[pi].strt + f.strt) && (q[pi].ptch > p[pi].ptch + f.ptch)) || end)
			 && ++pi < pattern_size);

		if (pi == pattern_size && !end)
		{
			if (result_list == Qnil) result_list = rb_iv_get(init_info, "@matches");

			/* add match to result list */
			for (k = 0; k < pattern_size; k++) matchednotes_obj[k] = UINT2NUM(q[k].notespos);
			rb_ary_push(result_list, rb_ary_new3(6, self, UINT2NUM(chordind), UINT2NUM(chordind + pattern_size - 1), \
			rb_ary_new4(pattern_size, matchednotes_obj), INT2FIX((int) f.ptch), zero));
		}

		/* move main indexes to next note */
		next_note(chords, &chords_size, &chordind, &spos, &noteind, &notespos);
	}
	return result_list;
}



/* Move to next note. May cross chord boundary or not. */
unsigned int next_note(char *chords, unsigned int *chords_size, unsigned int *chordind, unsigned int *chordspos, unsigned int *noteind, unsigned int *notespos)
{
	/* at the end of source; return */
	if (*chordind + 1 == *chords_size && (unsigned int) *(chords + *chordspos) == *noteind + 1) return 1;

	*noteind += 1;

	/* at the end of current chord; must cross chord boundary. */
	if (*noteind == (unsigned int) *(chords + *chordspos))
	{
		*chordind += 1;
		*noteind = 0;
		*chordspos = *notespos + NOTELEN;
		*notespos += NOTELEN + CHORDHEADERLEN;
	}
	else *notespos += NOTELEN;
	return 0;
}


