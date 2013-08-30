/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia
*/

#include "song.h"

/*
   Simple unpublished exact polyphonic pattern checking algorithm for MonoPoly. 
   Assumes that pitches of the pattern and source are sorted in ascending order. 
   Exact matching; matched notes need not be stored or evaluated.
*/
VALUE c_polycheck(VALUE self, char *chords, unsigned int chordind, unsigned int spos, vector *pattern, unsigned int pattern_size, unsigned int pattern_notes, VALUE result_list)
{
	VALUE zero = Qnil;
	unsigned int pi, ni, chordlen;

	pi = 0;
	ni = 0;
	chordlen = chords[spos];


	while (pi < pattern_notes)
	{

		if (chords[spos + CHORDHEADERLEN + ni * NOTELEN] > pattern[pi].ptch)
		{
			/* Source pitch is greater than pattern pitch, i.e. match can't be found. 1 case. */
			return Qnil;
		}
		else if (chords[spos + CHORDHEADERLEN + ni * NOTELEN] < pattern[pi].ptch)
		{
			/* Pattern pitch is greater than source pitch: 2 cases. */

			/* Case 2a. We are at the end of source chord and did not find the pattern pitch, i.e. search failed. */
			if (ni == chordlen - 1) return Qnil;

			/* Case 2b. We are not at the end of source chord. Try to find the pattern pitch later in the source chord. */
			ni++;
		}
		else
		{
			/* Pattern pitch and source pitch are equal: 4 cases. */

			/* Last note matched: succeeded. */
			if (pi == pattern_notes - 1)
			{ 
				break;
			}
			else if (pattern[pi].strt != pattern[pi + 1].strt)
			{
				/* Case 3a. We are at end of pattern chord; all notes in the pattern chord matched. */
				/* Move to the next note in the pattern and the next chord in the source. */
				pi++;
				ni = 0;
				spos += CHORDHEADERLEN + chordlen * NOTELEN;
				chordlen = chords[spos];
			}
			else
			{
				/* Case 3c. We are at the end of source chord but not at the end of pattern chord: search failed. */
				if (ni == chordlen - 1) return Qnil;

				/* Case 3d. We are not at end of any chord: move on in both */
				ni++;
				pi++;
			}
		}
	}


	/* If we got here, all pattern notes were found, i.e. match was found. */

	zero = INT2FIX(0);

	rb_ary_push(result_list, rb_ary_new3(6, self, UINT2NUM(chordind), UINT2NUM(chordind + pattern_size - 1), Qnil, zero, zero));

	return result_list;
}
