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
   calculating the LCTS edit distance between two monophonic tracks. 
   This wrapper calculates the distance between all track combinations and returns them as an array.
*/
VALUE c_lcts_distances(VALUE self, VALUE song2)
{
	VALUE result_list, tracks_ary1, tracks_ary2, tracklengths1, tracklengths2;
	char *track1, *track2;
	unsigned int trackind1, trackind2, num_tracks1, num_tracks2, track1_len, track2_len;
	
	/* Match set for each transposition -128,...,127 (mapped to 0...255) */
	matchList Mt[MAX_TRANSPOSITION];

	num_tracks1 = NUM2UINT(rb_iv_get(self, "@num_tracks"));
	tracks_ary1 = rb_iv_get(self, "@tracks");

	num_tracks2 = NUM2UINT(rb_iv_get(song2, "@num_tracks"));
	tracks_ary2 = rb_iv_get(song2, "@tracks");

	tracklengths1 = rb_iv_get(self, "@tracklengths");
	tracklengths2 = rb_iv_get(song2, "@tracklengths");

	result_list = rb_ary_new();

	/* compare each track of this song to each track of song2. */
	for (trackind1 = 1; trackind1 <= num_tracks1; trackind1++)
	{
		track1 = (char *) RSTRING_PTR(RARRAY_PTR(tracks_ary1)[trackind1]);
		track1_len = NUM2UINT(RARRAY_PTR(tracklengths1)[trackind1]);

		for (trackind2 = 1; trackind2 <= num_tracks2; trackind2++)
		{
			track2 = (char *) RSTRING_PTR(RARRAY_PTR(tracks_ary2)[trackind2]);
			track2_len = NUM2UINT(RARRAY_PTR(tracklengths2)[trackind2]);
			rb_ary_push(result_list, INT2NUM(computeAllTranspositions(Mt, track1, track2, track1_len, track2_len)));
		}
	}
	return result_list;
}
