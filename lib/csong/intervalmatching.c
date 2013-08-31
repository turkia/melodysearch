/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Implements an algorithm described in Kjell Lemstrom and Jorma Tarhio:  
   Transposition Invariant Pattern Matching for Multi-Track Strings. 
   Nordic Journal of Computing (to appear).
   Consult the article for description. 

   MonoPoly and OLF (Online Filter) should produce identical results.
*/


#include "song.h"


VALUE c_matchcheck(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, VALUE result_list);
VALUE c_matchcheck_oe(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, VALUE result_list);

/*
   Pattern preprocessing and internal data structure initialization. 
   Consult the article for details.
*/
VALUE c_intervalmatching_init(VALUE self, VALUE init_info)
{
	unsigned char i, pattern_size;
	unsigned int e, t[13];
	int ii = 0;
	vector *pattern;

	pattern = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_monophonic_vector"));
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));

	e = (unsigned int) pow(2, pattern_size - 1) - 1;

	for (i = 0; i <= VOCSIZE; i++) t[i] = e;

	for (i = 1; i < pattern_size; i++)
	{
		ii = (pattern[i].ptch - pattern[i - 1].ptch) % VOCSIZE;
		while (ii < 0) ii += VOCSIZE;
		t[ii] = t[ii] - (unsigned int) pow(2, i - 1);
	}

	rb_iv_set(init_info, "@pattern_size", INT2NUM(pattern_size));
	rb_iv_set(init_info, "@t", rb_str_new((char *) t, 13 * sizeof(unsigned int)));
	rb_iv_set(init_info, "@e", UINT2NUM(e));
	rb_iv_set(init_info, "@em", INT2NUM(pow(2, pattern_size) - 1 - pow(2, pattern_size - 2)));

	return init_info;
}


/*
   Online filtering algorithm. Described in Kjell Lemstrom and Jorma Tarhio:  
   Transposition Invariant Pattern Matching for Multi-Track Strings. Nordic Journal of Computing (to appear).
   Implementation follows closely the pseudocode presented in the article.
   Calculates the intervals on-the-fly, whereas MonoPoly uses precomputed intervals. 
   Scanning phase is similar. Results are identical to results of MonoPoly. 
*/
VALUE c_intervalmatching_scan(VALUE self, VALUE init_info)
{
	VALUE result_list = Qnil, result;
	int ii = 0;
	unsigned int pattern_size, i = 0, k = 0, chords_size, spos;
	unsigned int tmp = 0, mask = 0, e = 0, em = 0, *t;
	unsigned int chordind, chordlen, prevchordlen, prevchordspos;
	char *chords, *s;
	vector *pattern;

	pattern = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_monophonic_vector"));
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));

	mask = e = NUM2UINT(rb_iv_get(init_info, "@e"));
	em = NUM2UINT(rb_iv_get(init_info, "@em"));
	t = (unsigned int *) RSTRING_PTR(rb_iv_get(init_info, "@t"));

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));

	/* preprocessed string consists of (spos:4, intervaldata:2) pairs */
	s = (char *) RSTRING_PTR(rb_iv_get(self, "@preprocessed"));

	result_list = rb_iv_get(init_info, "@matches");

	prevchordlen = chords[0];
	prevchordspos = 0;

	/* scan all chords, start from the second chord */
	for (spos = CHORDHEADERLEN + chords[0] * NOTELEN, chordind = 1; chordind < chords_size; chordind++, spos += CHORDHEADERLEN + NOTELEN * prevchordlen)
	{
		tmp = mask;

		/* for each note in the current chord */
		for (chordlen = chords[spos], i = spos + CHORDHEADERLEN; i < spos + CHORDHEADERLEN + chordlen * NOTELEN; i += NOTELEN)
		{
			/* for each note in the previous chord */
			for (k = prevchordspos + CHORDHEADERLEN; k < prevchordspos + CHORDHEADERLEN + prevchordlen * NOTELEN; k += NOTELEN)
			{
				/* calculate interval */
				ii = (chords[i] - chords[k]) % VOCSIZE;
				while (ii < 0) ii += VOCSIZE;

				/* bit-and does not care if same interval is and'd more then once. no need to have separate table of intervals. */
				tmp &= t[ii];
			}
		}
		prevchordlen = chordlen;
		prevchordspos = spos;

		e = ((e << 1) | tmp) & mask;

		if ((e | em) == em)
		{
			result = c_matchcheck(self, chords, chordind - pattern_size + 1, *((unsigned int *)(s + PP_ITEM_SIZE * (chordind - pattern_size + 1))), \
				pattern, pattern_size, result_list);
		}
	}
	return result_list;
}


