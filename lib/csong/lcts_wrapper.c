/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Implements a wrapper for LCTS algorithm by Veli Makinen.
   Version that handles all tracks of a polyphonic song separately. 
*/


#include "song.h"
#include "lcts.h"


/*
   A wrapper for an algorithm by Veli Makinen, that allows for 
   searching the approximate transposition invariant
   occurrences of pattern P from text T. An occurrence chordind 
   is reported if d_{ID}(P+t,T_{chordind'...chordind}) <= errors, for some chordind' and transposition t. 
   Here d_{ID} is the edit distance under insertions and deletions (replacements are not allowed).
   This version handles all tracks of a polyphonic song separately. 
*/
VALUE c_lcts_scan(VALUE self, VALUE init_info)
{
	VALUE zero, result_list, tracks_ary = Qnil;
	char *p, *track, *temptrack, pitches[64], align_p[64], align_t[64]; /* NOTE: Fixed size. */
	unsigned int i = 0, chordind = 0, pattern_size, pind, trackind, tracklen;
	unsigned int chords_size = 0, num_tracks = 0, *mapping;
	int errors, j;
       	int startindex;
	occType *occ = NULL;
	
	/* Match set for each transposition -128,...,127 (mapped to 0...255) */
	matchList Mt[MAX_TRANSPOSITION];

	/* Test for pattern and chord array sizes */
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	if (pattern_size > chords_size) return Qnil;

	/* Get the rest of parameters */
	zero = INT2FIX(0);
	result_list = rb_iv_get(init_info, "@matches");
	p = (char *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_pitch_string"));
	errors = NUM2INT(rb_iv_get(init_info, "@errors"));

	num_tracks = NUM2UINT(rb_iv_get(self, "@num_tracks"));
	tracks_ary = rb_iv_get(self, "@tracks");

	temptrack = (char *) calloc(chords_size + 1, sizeof(char));
	mapping = (unsigned int *) calloc(chords_size + 1, sizeof(unsigned int));

	/* search each track separately. */
	for (trackind = 1; trackind <= num_tracks; trackind++)
	{
		track = (char *) RSTRING_PTR(RARRAY_PTR(tracks_ary)[trackind]);

		/* null characters indicate that there is no note in this chord on this track. */
		/* they are used by e.g. splitting algorithm. here we must remove them. */
		for (tracklen = 1, i = 1; i <= chords_size; i++) 
		{
			if ((unsigned char) track[i] != GAP_UNSIGNED)
			{
				temptrack[tracklen] = track[i];
				mapping[tracklen] = i;
				// printf("%d=%d/%d %d %d\n", tracklen, mapping[tracklen],chords_size, track[i], temptrack[tracklen]);
				tracklen++;
			}
		}
		tracklen--;
		//printf("lengths: %d/%d track=%s\n", tracklen, chords_size, temptrack+1);

		/* call search function */
		occ = searchAllTranspositions(Mt, p, temptrack, pattern_size, tracklen, errors);
		occ[0].value = INT_MAX;

		/* scan through results table (occ). */
		for (chordind = 1; chordind <= tracklen; chordind++)
		{
			/* occ table indicates for each chords if there is a match ending at that chord */
			if (occ[chordind].value <= errors && occ[chordind].value < occ[chordind - 1].value)
			{
				/* align function needs pitches as a char string. */
				for (pind = 0, i = max2((int) 0, (int) (chordind - pattern_size - errors)); i <= chordind; i++, pind++)
					pitches[pind] = temptrack[i];

				pitches[pind] = '\0';

				/* note: p starts from 1, align_p and align_t from zero. */
				j = align(p + 1, pitches+1, align_p, align_t, occ[chordind].t, &startindex);
				// printf("pind=%d t=%d p=%s pi=%s ap=%s at=%s \n", pind, occ[chordind].t, p+1,pitches+1, align_p, align_t);

				/* matched notes processing is not done in this version, since we would have to search for them 
				 * in polyphonic tracks across chord boundaries, which is too time consuming. */

				/* align strings are just added to the end of result array item (an array) */
				rb_ary_push(result_list, rb_ary_new3(8, self, INT2NUM(mapping[max2(0, (int) (chordind - pind + 1 + startindex))]), \
				UINT2NUM(mapping[chordind - 1]), Qnil, INT2FIX(occ[chordind].t), INT2FIX(occ[chordind].value), \
				rb_str_new2(align_p), rb_str_new2(align_t)));
			}
		}
		free(occ);
	}
	free(temptrack);

	return result_list;
}
