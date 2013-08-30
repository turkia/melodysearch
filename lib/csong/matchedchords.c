/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   Contacts: turkia at cs helsinki fi

   C language implementation of a method that returns a section of 
   the note data of a song. 


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


#include "song.h"


/*
   Returns note data between given chord indexes as an array. Used for creating a MIDI file of matched chords. 
*/
VALUE c_matchedchords(VALUE self, VALUE firstchordparam, VALUE lastchordparam)
{
	VALUE chordarray = Qnil, notearray = Qnil, strt;
	char *chords, *s;
	unsigned int i, j, offset, chords_size, firstchord, lastchord, chordlen = 0, spos;

	chords = (char *) RSTRING_PTR(rb_iv_get(self, "@chords"));
	chords_size = NUM2UINT(rb_iv_get(self, "@num_chords"));
	firstchord = NUM2UINT(firstchordparam);
	lastchord = NUM2UINT(lastchordparam);

	/* preprocessed string consists of (spos:4, intervaldata:2) pairs. used to get spos by chordindex */
	s = (char *) RSTRING_PTR(rb_iv_get(self, "@preprocessed"));

	/* validate chord indexes */
	if (firstchord >= chords_size || lastchord >= chords_size) return Qnil;

	/* array for chords */
	chordarray = rb_ary_new2(lastchord - firstchord);

	/* skip chords before the first chord */
	spos = *((unsigned int *) (s + firstchord * PP_ITEM_SIZE));

	/* strt of the first chord of the match; this is subtracted from strts 
	 * of the notes in the match so that the times start at zero */
	offset = *((unsigned int *) (chords + spos + 1));

	/* create a three-dimensional ruby array of the requested chords */
	for (i = 0; i <= lastchord - firstchord; i++)
	{
		/* get chord size */
		chordlen = chords[spos];

		/* get strt */
		strt = UINT2NUM(*((unsigned int *) (chords + spos + 1)) - offset);

		/* array for notes */
		notearray = rb_ary_new2(chordlen);

		/* get all notes in a chord */
		for (spos += 5, j = 0; j < chordlen; j++)
		{
			/* push an array representing a note to notearray (strt, ptch, dur) */
			rb_ary_push(notearray, rb_ary_new3(3, strt, CHR2FIX(chords[spos + j * NOTELEN]), \
						UINT2NUM((unsigned int) *((unsigned short *) (chords + spos + j * NOTELEN + 1)))));
		}
		spos += chordlen * NOTELEN;

		/* add note array to chord array */
		rb_ary_push(chordarray, notearray);
	}
	return chordarray;
}


