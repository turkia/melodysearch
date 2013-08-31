/*
	C-Brahms Engine for Musical Information Retrieval
	University of Helsinki, Department of Computer Science

	Version 0.2.4, May 15th, 2003

	Copyright Mika Turkia

	Contacts: turkia at cs helsinki fi

	Wrapper for splitting algorithm by Veli Makinen.


	This file is part of C-Brahms Engine for Musical Information Retrieval.

	C-Brahms Engine for Musical Information Retrieval is free software; 
	you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	C-Brahms Engine for Musical Information Retrieval is distributed 
	in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE. 
	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with C-Brahms Engine for Musical Information Retrieval; 
	if not, write to the Free Software Foundation, Inc., 59 Temple Place, 
	Suite 330, Boston, MA  02111-1307  USA
*/


#include "splitting.h"


/*
	Wrapper for splitting algorithm by Veli Makinen.
*/
VALUE c_splitting_scan(VALUE self, VALUE init_info)
{
	VALUE zero, result_list, matchednotes = Qnil, tracks_ary;
	char *chords, *preprocessed, *pattern;
	unsigned char *tracks[17]; /* assumes max 16 tracks */
	unsigned int i = 0, j,k,pattern_size, errors;
	unsigned int chordlen = 0, chords_size = 0, spos = 0, num_tracks = 0;

	tripleNode *node, *tempnode, *firstnode = NULL;
	int max_gap, songonce;
	int tp_invariance = 1;

	splittingResultStruct *process_results = NULL;

	/* Test for pattern and chord array sizes */
	pattern_size = NUM2UINT(rb_iv_get(init_info, "@pattern_notes"));
	chords = (char *) RSTRING(rb_iv_get(self, "@chords"))->ptr;
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	if (pattern_size > chords_size) return Qnil;

	/* Get the rest of parameters */
	zero = INT2FIX(0);
	result_list = rb_iv_get(init_info, "@matches");
	pattern = (char *) RSTRING(rb_iv_get(init_info, "@pattern_pitch_string"))->ptr;

	/* for matched notes, we need to get strt of the first chord, and we get the starting position of the chord */
	/* from preprocessed data by indexing with firstchord */
	preprocessed = (char *) RSTRING(rb_iv_get(self, "@preprocessed"))->ptr;

	max_gap = NUM2INT(rb_iv_get(init_info, "@gap"));
	errors = NUM2INT(rb_iv_get(init_info, "@errors"));
	songonce = NUM2INT(rb_iv_get(init_info, "@songonce"));

	/* we use track numbers for separating instruments, not program change events. */
	/* track number solution does not work for MIDI type 0 files. */
	/* program change does not work for e.g. piano with left and right hand on different tracks or multiple violins. */
	/* thus neither of these works in all cases. there is no general solution due to MIDI limitations. */
	num_tracks = NUM2UINT(rb_iv_get(self, "@num_tracks"));

	tracks_ary = rb_iv_get(self, "@tracks");
	for (i = 1; i <= num_tracks; i++) tracks[i] = (unsigned char *) RSTRING(RARRAY(tracks_ary)->ptr[i])->ptr;


	/* Call search function. now only non-ti; same in both cases. */
	if (tp_invariance) process_results = process_ti(pattern, tracks, pattern_size, chords_size, num_tracks, max_gap, songonce);
	else process_results = process(pattern, tracks, pattern_size, chords_size, num_tracks, max_gap, songonce);

	/* if songonce is requested, wrong number of all matches is reported since it is the length of results array. no fix at the moment. */
	 
	if (!songonce && tp_invariance)
	{
		for (k=1;k<=num_tracks;k++)
		{
			for (j=1;j<=chords_size;j++)
			{
				node = process_results->row_ti[k][j];
 			  
				if (node && node->kappa <= (int) errors)
				{
					/* generate matched notes */
					matchednotes = rb_ary_new2(pattern_size);

					/* optimal path can be extracted from node by following path node->prevTrace->prevTrace->... */
					/* however there is no spos information, just pitch, track number and chord index. */
					tempnode = node;

					while (tempnode)
					{
						/* we can't get references to notes in gaps. */ 
						/* there would be more point in reporting the notes if durations were reported. */
						/* get start of the chord by chordindex (tempnode->j - 1) */
						spos = *((unsigned int *) (preprocessed + (tempnode->j - 1) * PP_ITEM_SIZE));

						/* now we must find note with track number tempnode->k and same pitch. */
						chordlen = *((char *) (chords + spos));
						for (i = 0, spos += CHORDHEADERLEN; i < chordlen; i++, spos += NOTELEN)
					 	{
							if (*((char *) (chords + spos + 3)) == (tempnode->k - 1) && \
								*((char *) (chords + spos)) == tracks[tempnode->k][tempnode->j])
							{
								rb_ary_unshift(matchednotes, UINT2NUM(spos));
								break;
							}
						}

						firstnode = tempnode;
						tempnode = tempnode->prevTrace;
					}

					rb_ary_push(result_list, rb_ary_new3(7, self, INT2NUM(firstnode->j - 1), UINT2NUM(node->j - 1), \
					matchednotes, zero, zero, INT2FIX(node->kappa)));
				}

			}
		}
 	}
 	else
	{
		node = process_results->matchlist;

		while (node)
		{
			if (node->kappa <= (int) errors)
			{
				/* generate matched notes */
				matchednotes = rb_ary_new2(pattern_size);

				/* optimal path can be extracted from node by following path node->prevTrace->prevTrace->... */
				/* however there is no spos information, just pitch, track number and chord index. */
				tempnode = node;

				while (tempnode)
				{
					/* we can't get references to notes in gaps. */ 
					/* there would be more point in reporting the notes if durations were reported. */
					/* get start of the chord by chordindex (tempnode->j - 1) */
					spos = *((unsigned int *) (preprocessed + (tempnode->j - 1) * PP_ITEM_SIZE));

					/* now we must find note with track number tempnode->k and same pitch. */
					chordlen = *((char *) (chords + spos));
					for (i = 0, spos += CHORDHEADERLEN; i < chordlen; i++, spos += NOTELEN)
					{
						if (*((char *) (chords + spos + 3)) == (tempnode->k - 1) && \
							*((char *) (chords + spos)) == tracks[tempnode->k][tempnode->j])
						{
							rb_ary_unshift(matchednotes, UINT2NUM(spos));
							break;
						}
					}

					firstnode = tempnode;
					tempnode = tempnode->prevTrace;
				}

				rb_ary_push(result_list, rb_ary_new3(7, self, INT2NUM(firstnode->j - 1), UINT2NUM(node->j - 1), \
				matchednotes, zero, zero, INT2FIX(node->kappa)));
			}
	
			if (songonce) break;
			else node = node->next;
		}
	}

	/* free internal data structures of process function that must be freed after extracting the optimal path */
	if (tp_invariance) c_splitting_free_ti(process_results, MAX_TRANSPOSITION,num_tracks);
	else c_splitting_free(process_results, (int) pattern_size);
	return result_list;
}


