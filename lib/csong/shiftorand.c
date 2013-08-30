/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Contacts: turkia at cs helsinki fi

   Implements an algorithm described in Kjell Lemstrom and Jorma Tarhio:  
   Transposition Invariant Pattern Matching for Multi-Track Strings. 
   Nordic Journal of Computing, Volume 10 Issue 3, September 2003, Pages 185-205.
   Consult the article for description. 
*/


#include "song.h"


/*
   Initializes table t. Items of t refer to pitch values.
   Values of items indicate which positions of the pattern contain the corresponding pitch value.
   Item size is 32 bits; therefore pattern size is restricted to 32. 
   Each bit corresponds to pattern position. If a bit is zero, there is a note with this pitch
   in the pattern in a position indicated by the number of the bit in the word. Otherwise the value of the bit is one.
   Consult the article for details.
*/
VALUE c_shiftorand_init(VALUE self, VALUE init_info)
{
	/*unsigned int i, e, em, mask, pattern_size, t[128];*/
	unsigned int i, e, em, mask, pattern_size, t[256];
	vector *pattern;

	/* note: there must not be notes with same pitch in same source chord, or values of t will be confused. */
	/* therefore we must use monophonic version of the pattern. */
	pattern = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_monophonic_vector"));
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));

	/* calculate values */
	mask = e = pow(2, pattern_size) - 1;
	em = mask - pow(2, pattern_size - 1);

	/* initialize array t */
	/*for (i = 0; i < 128; i++) t[i] = mask;*/
	for (i = 0; i < 256; i++) t[i] = mask;

	/* calculate values of t */
	for (i = 0; i < pattern_size; i++) t[(int) pattern[i].ptch] -= (unsigned int) pow(2, i);

	/* save results to instance variables */
	/*rb_iv_set(init_info, "@t", rb_str_new((char *) t, 128 * sizeof(unsigned int)));*/
	rb_iv_set(init_info, "@t", rb_str_new((char *) t, 256 * sizeof(unsigned int)));
	rb_iv_set(init_info, "@e", UINT2NUM(e));
	rb_iv_set(init_info, "@em", UINT2NUM(em));
	rb_iv_set(init_info, "@mask", UINT2NUM(mask));
	return init_info;
}


/*
   Scanning phase. Implementation follows closely the pseudocode presented in the article.
*/
VALUE c_shiftorand_scan(VALUE self, VALUE init_info)
{
	VALUE zero, result_list;
	char *chords;
	unsigned int i, pattern_size, tmp, mask, e, em, *t;
	unsigned int chordlen = 0;
	unsigned int chord, chords_size;
	unsigned int spos;

	zero = INT2FIX(0);
	e = NUM2UINT(rb_iv_get(init_info, "@e"));
	em = NUM2UINT(rb_iv_get(init_info, "@em"));
	mask = NUM2UINT(rb_iv_get(init_info, "@mask"));
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	t = (unsigned int *) RSTRING_PTR(rb_iv_get(init_info, "@t"));
	result_list = rb_iv_get(init_info, "@matches");

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));

	/* scan all chords */
	for (spos = 0, chord = 0; chord < chords_size; chord++, spos += (NOTELEN * chordlen))
	{
		tmp = mask;
		chordlen = chords[spos];

		/* skip chordlen and strt */
		spos += CHORDHEADERLEN;

		/* scan all notes in a chord */
		for (i = 0; i < chordlen; i++)
		{
			/* it does not matter for bit-and if there are many notes with same pitch in the source chord. */
			tmp &= t[(int) chords[spos + i * NOTELEN]];
		}

		e = ((e << 1) | tmp) & mask;

		if ((e | em) == em)
		{
			rb_ary_push(result_list, rb_ary_new3(6, self, INT2FIX(chord - pattern_size + 1), \
				INT2FIX(chord), Qnil, zero, zero));
		}
	}
	return result_list;
}
