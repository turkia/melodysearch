/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.5b, July 4th, 2003

   Copyright Mika Turkia

   Implements an equation (2) for calculating weighted edit distance 
   from article Kjell Lemstrom and Gonzalo Navarro: Flexible and Efficient 
   Bit-Parallel Techniques for Transposition Invariant Approximate Matching 
   in Music Retrieval.
*/


#include "song.h"
#include <string.h>


/*
	Naive dynamic programming algorithm for comparison purposes. Transposition invariant.
	Handles each track separately.
*/
VALUE c_dynprog_scan(VALUE self, VALUE init_info)
{
	VALUE tracks_ary = Qnil, result_list;
	char *p, *track;
	unsigned int pattern_size, trackind, num_chords = 0, num_notes, num_tracks = 0, i, j, ip, jp;
	int errors, tp, len;
	/* ID = cost of indel operation; sigma = vocabulary size (here size of MIDI pitch range) */
	int sigma = 128, ID = 1;
	int mindistance = INT_MAX, minchordind = 0, mintp = 0, min1, min2, min3;
	int columna[MAX_PATTERN_NOTES + 1], oldcolumna[MAX_PATTERN_NOTES + 1], initcolumn[MAX_PATTERN_NOTES + 1], *temp, *oldcolumn, *column;

	/* Test for pattern and chord array sizes */
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_size"));
	num_chords = NUM2UINT(rb_iv_get(self, "@num_chords"));
	num_notes = NUM2UINT(rb_iv_get(self, "@num_notes"));
	if (pattern_size > num_chords) return Qnil;

	/* Get the rest of parameters */
	p = (char *) RSTRING_PTR(rb_iv_get(init_info, "@pattern_pitch_string"));
	errors = NUM2INT(rb_iv_get(init_info, "@errors"));
	tracks_ary = rb_iv_get(self, "@tracks");
	num_tracks = NUM2UINT(rb_iv_get(self, "@num_tracks"));
	result_list = rb_iv_get(init_info, "@matches");

	/* initialize first column */
	for (i = 0; i <= pattern_size; i++) initcolumn[i] = i * ID;
	len = sizeof(int) * (pattern_size + 1);
	oldcolumn = oldcolumna;
	column = columna;

	/* for each track */
	for (trackind = 1; trackind <= num_tracks; trackind++)
	{
		track = (char *) RSTRING_PTR(RARRAY_PTR(tracks_ary)[trackind]);

		/* for each transposition */
		for (tp = -sigma + 1; tp < sigma; tp++)
		{
			/* initialize left column (old column) */
			for (i = 0; i <= pattern_size; i++) { oldcolumn[i] = initcolumn[i]; column[i] = 0; }

			/* for each note on the track */
			for (j = 0, jp = 1; j < num_chords; j++, jp++)
			{
				column[0] = j * ID;
				
				for (i = 0, ip = 1; i < pattern_size; i++, ip++)
				{
					min1 = abs(track[jp] - p[ip] - tp) + oldcolumn[i];
					min2 = ID + column[i];
					min3 = ID + oldcolumn[ip];

					if (min1 < min2)
					{
						if (min1 < min3) column[ip] = min1;
						else column[ip] = min3;
					}
					else
					{
						if (min2 < min3) column[ip] = min2;
						else column[ip] = min3;
					}
				}

				if (column[pattern_size] <= mindistance)
				{
					mindistance = column[pattern_size];
					minchordind = j;
					mintp = tp;
				}

				temp = oldcolumn;
				oldcolumn = column;
				column = temp;
			}
		}
	}

	/* Process results */
	if (mindistance <= errors * ID) rb_ary_push(result_list, rb_ary_new3(6, self, INT2NUM(max2(minchordind - (int) pattern_size + 1, 0)), INT2NUM(minchordind), Qnil, INT2FIX(mintp), INT2FIX(mindistance)));

	return result_list;
}
