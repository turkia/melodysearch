/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   C language implementation of checking algorithm that finds
   transposition invariant matches.
*/


#include "song.h"


/*
   Checks that the candidates found by filtering methods are real occurrences.
   Finds transposition invariant octave equivalent matches.
*/
VALUE c_matchcheck(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, VALUE result_list)
{
	VALUE zero = Qnil, matchednotes_obj[31];
	int pitch, x, y, transposition = 0;
	unsigned int matchednotes[31], noteindex, nextindex, current_spos, next_spos, i;
	unsigned char current_len, next_len, pattern_index, found, yfound;

	/* for all notes in the first chord do */
	for (noteindex = 0; noteindex < (unsigned char) chords[spos]; noteindex++)
	{
		pattern_index = 0;
		found = 1;
		current_spos = spos;
		current_len = chords[current_spos];

		/* get the pitch of the current note */
		pitch = (int) chords[spos + CHORDHEADERLEN + noteindex * NOTELEN];

		matchednotes[pattern_index] = current_spos + CHORDHEADERLEN + noteindex * NOTELEN;

		/* try to find a consequtive sequence of pitches in the following chords */
		/* starting from c and corresponding to the intervals in the pattern */
		while (found == 1 && pattern_index < pattern_size - 1)
		{
			/* must find x in the next chord */
			x = pitch + (int) pattern[pattern_index + 1].ptch - (int) pattern[pattern_index].ptch;

			/* position of the next chord (relative to current_spos) */
			next_spos = current_spos + CHORDHEADERLEN + current_len * NOTELEN;
			next_len = chords[next_spos];

			/* try to find xx in ychord */
			for (yfound = 0, nextindex = 0; nextindex < next_len; nextindex++)
			{
				y = chords[next_spos + CHORDHEADERLEN + nextindex * NOTELEN];
				while (y < 0) y += VOCSIZE;

				if (x == y)
				{
					yfound = 1;
					break;
				}
			}

			if (yfound == 1)
			{
				/* ynote matched: we can move to the next chord */
				pitch = x;
				matchednotes[pattern_index + 1] = next_spos + CHORDHEADERLEN + nextindex * NOTELEN;
			}
			else found = 0;

			/* move to the next chord */
			current_spos += CHORDHEADERLEN + current_len * NOTELEN;
			current_len = chords[current_spos];
			pattern_index++;
		}

		if (found == 1)
		{
			/* Calculates the amount of transposition from the first note of the pattern and the first matched note. */
			/* Assumes that the algorithm is exact, which implies that all notes are transposed the same amount. */
			transposition = chords[matchednotes[0]] - pattern[0].ptch;
			if (zero == Qnil) zero = UINT2NUM(0);

			/* Conversion of matched notes to object format */
			for (i = 0; i < pattern_size; i++) matchednotes_obj[i] = UINT2NUM(matchednotes[i]);

			/* Add match to result list. */
			rb_ary_push(result_list, rb_ary_new3(6, self, UINT2NUM(chordind), UINT2NUM(chordind + pattern_size - 1), \
				rb_ary_new4(pattern_index + 1, matchednotes_obj), INT2NUM(transposition), zero));

			/* There may be overlapping matches, so we do not stop after the first match. */
			/* But if there are multiple overlapping matches with only the last note different, only one is found. */
			/* -> the best match may not be reported. */
		}
	}
	return result_list;
}


