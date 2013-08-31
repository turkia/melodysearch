/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003
   Version 0.2.2, December 17th, 2002

   Copyright Mika Turkia

   Implements an algorithm described in Kjell Lemstrom and Jorma Tarhio:  
   Transposition Invariant Pattern Matching for Multi-Track Strings. 
   Nordic Journal of Computing (to appear).
   Consult the article for description. 

   MonoPoly and IntervalMatching should produce identical results.
*/

#include "song.h"

VALUE c_matchcheck(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, VALUE result_list);
VALUE c_matchcheck_oe(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, VALUE result_list);
VALUE c_polycheck(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, unsigned int pattern_notes, VALUE result_list);

/*
   Pattern preprocessing and internal data structure initialization. 
   Stores data structures in init_info instance that is given as a parameter.
   Builds table t in linear time using arrays itable and ltable. 
   Array t has a column for every possible interval combination.
   Consult the article for details. 
*/
VALUE c_monopoly_init(VALUE self, VALUE init_info)
{
	unsigned int i, j = 0, itable[VOCSIZE], ltable[VOCSIZE], ones, e, em, mask, pattern_size, *t, tlen;
	int ii;
	vector *pattern;

	pattern = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_monophonic_vector"));
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	
	/* calculate values */
	e = pow(2, pattern_size - 1) - 1;
	em = ~0 - pow(2, pattern_size - 2);
	ones = pow(2, VOCSIZE) - 1;
	mask = pow(2, pattern_size) - 1;

	/* create and initialize array t */
	tlen = pow(2, VOCSIZE);
	t = (unsigned int *) ALLOCA_N(unsigned int, tlen);
	for (i = 0; i < tlen; i++) t[i] = mask;

	/* build itable and ltable */
	for (i = 0; i < VOCSIZE; i++)
	{
		itable[i] = ones - pow(2, i);
		ltable[i] = e;
	}

	for (i = 0; i < pattern_size - 1; i++)
	{
		ii = (pattern[i + 1].ptch - pattern[i].ptch) % VOCSIZE;
		while (ii < 0) ii += VOCSIZE;
		ltable[ii] -= pow(2, i);
	}

	/* build t */
	for (i = 0; i < ones; i++)
	{
		for (j = 0; j < VOCSIZE; j++)
		{
			if ((itable[j] | i) == itable[j]) t[i] &= ltable[j];
		}
	}

	/* save results to instance variables */
	rb_iv_set(init_info, "@t", rb_str_new((char *) t, tlen * sizeof(unsigned int)));
	rb_iv_set(init_info, "@e", UINT2NUM(e));
	rb_iv_set(init_info, "@em", UINT2NUM(em));
	rb_iv_set(init_info, "@mask", UINT2NUM(mask));
	return init_info;
}


/*
   Offline filtering algorithm. See Kjell Lemstrom and Jorma Tarhio:  
   Transposition Invariant Pattern Matching for Multi-Track Strings. 
   Nordic Journal of Computing (to appear), or Kjell Lemstrom:
   String Matching Techniques for Music Retrieval, PhD thesis, A-2000-4, 
   University of Helsinki, Department of Computer Science, November, 2000. 
   Implementation follows closely the pseudocode in the article.
   Compare also with the scanning phase of ShiftOrAnd algorithm. 

   Because this is a filtering algorithm, it calls either matchcheck or polycheck functions
   to check the candidate matches. 
*/
VALUE c_monopoly_scan(VALUE self, VALUE init_info)
{
	VALUE results = Qnil, result_list = Qnil;
	unsigned int e, em, mask, chordind, chords_size, pattern_size, pattern_notes, *t;
	unsigned short int *usptr;
	char *s, *chords, checkfunc;
	vector *pattern_mono, *pattern_poly;

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	pattern_mono = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_monophonic_vector"));
	pattern_poly = (vector *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_polyphonic_vector"));
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	pattern_notes = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));

	if (chords_size < pattern_size) return Qnil;

	/* get values from init_info */
	t = (unsigned int *) RSTRING_PTR(rb_iv_get(init_info, "@t"));
	e = NUM2UINT(rb_iv_get(init_info, "@e"));
	em = NUM2UINT(rb_iv_get(init_info, "@em"));
	mask = NUM2UINT(rb_iv_get(init_info, "@mask"));
	checkfunc = NUM2CHR(rb_iv_get(init_info, "@checkingfunction"));
	result_list = rb_iv_get(init_info, "@matches");

	/* preprocessed string consists of (spos:4, intervaldata:2) pairs */
	s = (char *) RSTRING_PTR(rb_iv_get(self, "@preprocessed"));

	for (chordind = 0; chordind < chords_size - 1; chordind++, s += PP_ITEM_SIZE)
	{
		/* get a pointer to the interval data in s */
		usptr = (unsigned short int *) (s + sizeof(unsigned int));

		e = ((e << 1) | t[ *usptr ]) & mask;

		if ((e | em) == em)
		{
			if (checkfunc == 1)
			{
				results = c_polycheck(self, chords, chordind - pattern_size + 2, \
				/* get the position of the first chord of the match in @chords */
				*((unsigned int *)(s - PP_ITEM_SIZE * (pattern_size - 2))), pattern_poly, pattern_size, pattern_notes, result_list);
			}
			else
			{
				results = c_matchcheck(self, chords, chordind - pattern_size + 2, \
				*((unsigned int *)(s - PP_ITEM_SIZE * (pattern_size - 2))), pattern_mono, pattern_size, result_list);
			}
		}
	}
	return result_list;
}


/*
   Right circularshift bit operator. E.g. "01" shifted by one becomes "10" due to wrapping. 
*/
static int rcs(int value, int width, int amount)
{
	return (((value << (width - amount)) & ~(~0 << width)) | ((value >> (amount)) & ~(~0 << (width - amount))));
}


/*
   Source preprocessing method. Returns an array consisting of structures with length of six bytes.
   First 4 bytes store the start position of the chord in source (offset from the start of source).
   Next 2 bytes are interval data, so that 12 bits represent the possible pitch intervals between notes 
   in two chords in when the vocabulary size is 12.
   If the corresponding bit is zero, interval is present. If it is one, interval is not present.
   
   Intervals are octave equivalent, so that for example when 72 - 60 = 12, we take 12 % 12 = 0. 
   Now also negative intervals wrap to positive; for example (60 - 70) % 12 = -10 % 12 = 2.
*/
VALUE c_monopoly_preprocess(VALUE self)
{
	char *chords, *s, *so;

	/* because VOCSIZE <= 16, unsigned short is enough */
	short int b, amount;
	unsigned short int tmp, ones, tpow, shifts, base;
	unsigned short int i, chordlen, nextchordlen, *usptr;

	/* note: unsigned short is not enough for spos */
	unsigned int spos, slen, nextchordspos, chordind, chords_size, *uiptr;

	/* get a pointer to chord data */
	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));

	/* s is the result of preprocessing. format: (spos:4, intervals:2). last uint is for storing the spos of the last chord. */
	slen = chords_size * (sizeof(unsigned int) + sizeof(unsigned short int)) + sizeof(unsigned int);
	so = s = (char *) ALLOCA_N(char, slen);

	ones = pow(2, VOCSIZE) - 1;

	/* scan all chords */
	for (spos = 0, chordind = 0; chordind < chords_size - 1; chordind++)
	{
		chordlen = chords[spos];
		nextchordspos = spos + CHORDHEADERLEN + chordlen * NOTELEN;
		nextchordlen = chords[nextchordspos];

		/* check that source position fits to unsigned int */
		if (spos > UINT_MAX)
		{
			printf("Error: too long source in a song: spos=%u\n", spos);
			return Qnil;
		}

		/* initialize fields: first 4 bytes are the start of the chord in source */ 
		/* next 2 bytes are interval data */
		uiptr = (unsigned int *) (s);
		*uiptr = spos;
		usptr = (unsigned short int *) (s + sizeof(unsigned int));
		*usptr = ones;
		
		/* skip chordlen and strt in both chords */
		spos += CHORDHEADERLEN;
		nextchordspos += CHORDHEADERLEN;

		/* get the pitch of the base note of the current chord */
		/* notes in a chord must be ordered */
		base = chords[spos];

		/* calculate intervals between base and all notes in the next chord */
		for (i = 0; i < nextchordlen; i++)
		{
			b = (chords[nextchordspos + i * NOTELEN] - base) % VOCSIZE;
			while (b < 0) b += VOCSIZE;

			tpow = pow(2, b);
			tmp = ones - tpow;
			if ((*usptr | tmp) != tmp) *usptr -= tpow;
		}

		/* shift the previous intervals by differences between the base and other notes */
		shifts = ones;
		for (i = 1; i < chordlen; i++)
		{
			amount = (chords[spos + i * NOTELEN] - base) % VOCSIZE;
			while (amount < 0) amount += VOCSIZE;
			shifts &= rcs(*usptr, VOCSIZE, amount);
		}

		/* add shifted intervals to base intervals */
		*usptr &= shifts;

		/* move to the next chord */
		spos += (NOTELEN * chordlen);
		s += PP_ITEM_SIZE;
	}
	/* write spos of the last chord to the end of string */
	uiptr = (unsigned int *) (s);
	*uiptr = spos;
	
	/* save results to instance variable */
	rb_iv_set(self, "@preprocessed", rb_str_new(so, slen));
	
	return self;
}
