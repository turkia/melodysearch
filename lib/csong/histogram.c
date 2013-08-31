/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Returns a histogram of absolute note pitches. 
*/

#include "song.h"


/*
   Creates a histogram of absolute note pitches.
   Returns an array containing count for each note (0-127).
*/
VALUE c_pitch_histogram(VALUE self)
{
	VALUE pitches_obj[128];
	char *chords, *cp;
	unsigned int i, j, chords_size, chordlen, pitches[128];

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	for (i = 0; i < 128; i++) pitches[i] = 0;

	/* scan all chords */
	for (cp = chords, i = 0; i < chords_size; i++)
	{
		/* get chord size */
		chordlen = *cp;
		cp += CHORDHEADERLEN;

		/* scan all notes in a chord */
		for (j = 0; j < chordlen; j++, cp += NOTELEN) pitches[(int) *cp] += 1;
	}
	
	for (i = 0; i < 128; i++) pitches_obj[i] = UINT2NUM(pitches[i]);

	return rb_ary_new4(128, pitches_obj);
}

/*
   Returns a pitch histogram where pitches are first mapped to pitch classes and then mapped to the circle of fifths (folded).
   Tzanetakis, G., Ermolinskyi, A., Cook, P. Picth Histograms in Audio and Symbolic Music Information Retrieval. Proc. ISMIR 2002.
*/
VALUE c_pitch_histogram_folded(VALUE self)
{
	VALUE pitches_obj[12];
	char *chords, *cp;
	unsigned int i, j, chords_size, chordlen, pitches[12];

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	for (i = 0; i < 12; i++) pitches[i] = 0;

	/* scan all chords */
	for (cp = chords, i = 0; i < chords_size; i++)
	{
		/* get chord size */
		chordlen = *cp;
		cp += CHORDHEADERLEN;

		/* scan all notes in a chord */
		for (j = 0; j < chordlen; j++, cp += NOTELEN) pitches[(7 * (((int) *cp) % 12)) % 12] += 1;
	}
	
	for (i = 0; i < 12; i++) pitches_obj[i] = UINT2NUM(pitches[i]);

	return rb_ary_new4(12, pitches_obj);
}


										  

/*
   Creates a histogram of note intervals.
   Track information is ignored.
   Intervals are calculated between notes in two chords.
   Note that the definition of a chord in this software is
   that notes with same onset time (strt) belong to same chord.
   
   MIDI pitch values have a minimum value of 0 and maximum value of 127.
   Therefore the minimum possible interval is 0 - 127 = -127 and
   maximum interval is 127 - 0 = 127. Thus there are 127 + 1 + 127 = 255 
   possible intervals.

   Returns an array containing count for each interval (-127-127).
*/
VALUE c_pitch_interval_histogram(VALUE self)
{
	VALUE intervals_obj[255];
	char *chords, *cp, *pp, *temp;
	unsigned int i, j, k, chords_size, chordlen, prevchordlen, intervals[255];

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	for (i = 0; i < 255; i++) intervals[i] = 0;

	/* scan all chords, start from the second chord */
	pp = chords;
	prevchordlen = *pp;
	pp += CHORDHEADERLEN;
	cp = chords + CHORDHEADERLEN + chords[0] * NOTELEN;

	for (i = 0; i < chords_size; i++)
	{
		/* get chord size */
		chordlen = *cp;
		cp += CHORDHEADERLEN;
		temp = cp;

		/* scan all notes in a chord */
		for (j = 0; j < chordlen; j++, cp += NOTELEN)
		{
			/* scan all notes in the previous chord */
			for (k = 0; k < prevchordlen; k++, pp += NOTELEN)
			{
				/* calculate interval */
				intervals[127 + (int) *cp - (int) *pp] += 1;
			}
		}
		prevchordlen = chordlen;
		pp = temp;
	}

	for (i = 0; i < 255; i++) intervals_obj[i] = UINT2NUM(intervals[i]);
	return rb_ary_new4(255, intervals_obj);
}


/*
   Returns an array of length 11, where items are counts for the following duration classes:
   0: unknown duration
   1: (inf,1)
   2: [1,1/2)
   3: [1/2,1/4)
   4: [1/4,1/8)
   5: [1/8,1/16)
   6: [1/16,1/32)
   7: [1/32,1/64)
   8: [1/64,1/128)
   9: [1/128,1/256)
   10: [1/256,1/512)
*/
VALUE c_duration_histogram(VALUE self)
{
	VALUE durations_obj[11];
	char *chords, *cp;
	unsigned int i, j, k, chords_size, chordlen;
	double dur = 0, fullnoteduration, temp, durations[11];

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	fullnoteduration = 4 * NUM2DBL(rb_iv_get(self, "@quarternoteduration"));
	for (i = 0; i < 11; i++) durations[i] = 0;

	for (cp = chords, i = 0; i < chords_size; i++)
	{
		chordlen = *cp;
		cp += CHORDHEADERLEN;
		//printf("chord %u/%u: len=%u\n", i, chords_size, chordlen);

		for (j = 0; j < chordlen; j++, cp += NOTELEN)
		{ 
			dur = (double) *((unsigned short *) (cp + 1));
			//printf("chord %u/%u: note %u/%u: pitch=%d:dur=%f\n", i, chords_size, j, chordlen, *cp, dur);

			temp = fullnoteduration;
			for (k = 1; k < 11; k++, temp /= 2)
			{
				//printf("  k=%u: dur=%f >= %f\n", k, dur, temp);
				if (dur > temp)
				{
					durations[k] += 1; break;
				}
			}
			if (k == 11) durations[0] += 1;
		}
	}

	for (i = 0; i < 11; i++) durations_obj[i] = UINT2NUM(durations[i]);

	return rb_ary_new4(11, durations_obj);
}


