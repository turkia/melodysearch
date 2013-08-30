/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   C language function for converting matches to HTML.
*/


#include <ruby.h>
#include <string.h>
#include <math.h>
#include "../csong/song.h"

/* defined also in align.c for lcts align string gap marker */
#define GAP -2

/* definitions specific to result processing */
#define ALIGNMAXLEN 512
#define NOTEMAXLEN 256 
#define ROWMAXLEN 2048


VALUE cServer;
VALUE cMIR;

/*
   Generates HTML string from result array.
*/
static VALUE c_matches_to_html(VALUE self, VALUE matches)
{
	VALUE s, notes, match, song, transp_num, scoreurl_rb, timesignatures, timesig;
	int transp, ptch, ti;
	unsigned int i, len, j, firstchord, lastchord, notes_len, quarternoteduration, bar, strt, sigstrt;
	char notestr[NOTEMAXLEN + 1], *nastr, *playstr, *t1, *name = NULL, *notestrp, scoreurl[256], pdfurl[256], *midiurl = NULL, *chords, *preprocessed;
	char *align_p, *align_t, *alignstrp, alignstr[ALIGNMAXLEN + 1];
	int stringlen = 0;
	unsigned int nom, denom;

	char *rowstr = (char *) ALLOCA_N(char, ROWMAXLEN);

	nastr = "n/a";
	playstr = "(play)";

	len = RARRAY_LEN(matches);

	/* create main result string with table start header and column headers */
	s = rb_str_new2("<table><tr><td>Number</td><td>Composer</td><td>Title</td><td>Opus</td><td>Date</td><td>Style</td><td>Play</td><td>Metadata</td><td>Score&nbsp;&nbsp;&nbsp;&nbsp;</td><td>Appr.Bar #</td><td>Matched Notes</td><td>Transposition</td><td>Errors</td><td>Splits/Duration</td><td>Aligns</td></tr>");


	/* loop through all results */
	for (i = 0; i < len; i++)
	{
		match = RARRAY_PTR(matches)[i];
		song = RARRAY_PTR(match)[0];
		chords = (char *) RSTRING_PTR(rb_iv_get(song, "@chords"));
		firstchord = NUM2UINT(RARRAY_PTR(match)[1]);
		lastchord = NUM2UINT(RARRAY_PTR(match)[2]);
		timesignatures = rb_iv_get(song, "@timesignatures");

		transp_num = RARRAY_PTR(match)[4];
		if (transp_num == Qnil) transp = 0; else transp = NUM2INT(transp_num);

		scoreurl_rb = rb_iv_get(song, "@scoreurl");
		if (scoreurl_rb == Qnil || RSTRING_LEN(scoreurl_rb) == 0)
		{
			scoreurl[0] = '\0';
			pdfurl[0] = '\0';
		}
		else 
		{
			snprintf(scoreurl, 254, "<a href='%s '>PS</a>", RSTRING_PTR(scoreurl_rb));
			 /* pdf url is the same as ps url except last two letters are different. */
			snprintf(pdfurl, 254, "<a href='%s '>PDF</a>", RSTRING_PTR(scoreurl_rb));
			pdfurl[RSTRING_LEN(scoreurl_rb) + 8] = 'd';
			pdfurl[RSTRING_LEN(scoreurl_rb) + 9] = 'f';
		}

		notes = RARRAY_PTR(match)[3];
		if (notes != Qnil) notes_len = (unsigned int) RARRAY_LEN(notes);
		else notes_len = 0;

		/* midiurl can't be Qnil */
		midiurl = (strlen(t1 = RSTRING_PTR(rb_iv_get(song, "@midiurl"))) == 0) ? nastr : t1;

		/* calculate bar number of the start of the match */
		quarternoteduration = NUM2UINT(rb_iv_get(song, "@quarternoteduration"));
		preprocessed = (char *) RSTRING_PTR(rb_iv_get(song, "@preprocessed"));

		/* we need to get strt of the first chord, and we get the starting position of the chord */
		/* from preprocessed data by indexing with firstchord */
		strt = *((unsigned int *) (chords + *((unsigned int *) (preprocessed + firstchord * PP_ITEM_SIZE)) + 1));
		if (RARRAY_LEN(timesignatures) == 0) bar = strt / (quarternoteduration * 4) + 1; 
		else
		{
			bar = 1;
			for (ti = RARRAY_LEN(timesignatures) - 1; ti >= 0; ti--)
			{
				timesig = RARRAY_PTR(timesignatures)[ti];
				sigstrt = NUM2UINT(RARRAY_PTR(timesig)[0]);

				if (strt > sigstrt)
				{
					nom = NUM2UINT(RARRAY_PTR(timesig)[1]);
					denom =	(unsigned int) pow(2, NUM2UINT(RARRAY_PTR(timesig)[2]));

					bar += (strt - sigstrt) / (4 * quarternoteduration * ((float) nom / denom));
					strt -= (strt - sigstrt);
				}
			}
		}


		/* matched notes */
		notestr[0] = '\0';
		notestrp = notestr;
		stringlen = 0;

		for (j = 0; j < notes_len; j++)
		{
			ptch = (int) chords[NUM2INT(RARRAY_PTR(notes)[j])];

			/* convert midi value to english note name. inlined to improve performance. */
			switch (ptch % VOCSIZE)
			{
				case 0: name = "C"; break;
				case 1: name = "C#"; break;
				case 2: name = "D"; break;
				case 3: name = "D#"; break;
				case 4: name = "E"; break;
				case 5: name = "F"; break;
				case 6: name = "F#"; break;
				case 7: name = "G"; break;
				case 8: name = "G#"; break;
				case 9: name = "A"; break;
				case 10: name = "A#"; break;
				case 11: name = "B"; break;
			}

			/* symbolic durations to be added */

			/* print to string */
			if (stringlen + 7 < NOTEMAXLEN) snprintf(notestrp, 7, "%s%d ", name, ptch / VOCSIZE);
			else break;

			if (ptch / VOCSIZE < 10)
			{ 
				notestrp += strlen(name) + 2;
				stringlen += strlen(name) + 2;
			}
			else
			{ 
				notestrp += strlen(name) + 3;
				stringlen += strlen(name) + 3;
			}
		}


		/* align strings for lcts */
		if (RARRAY_LEN(match) == 8)
		{
			stringlen = 0;
			alignstrp = alignstr;
			align_p =  RSTRING_PTR(RARRAY_PTR(match)[6]);
			align_t =  RSTRING_PTR(RARRAY_PTR(match)[7]);

			for (j = 0; j < strlen(align_p); j++)
			{
				if (align_p[j] == GAP) name = "- ";
				else
				{
					switch (align_p[j] % VOCSIZE)
					{
						case 0: name = "C "; break;
						case 1: name = "C#"; break;
						case 2: name = "D "; break;
						case 3: name = "D#"; break;
						case 4: name = "E "; break;
						case 5: name = "F "; break;
						case 6: name = "F#"; break;
						case 7: name = "G "; break;
						case 8: name = "G#"; break;
						case 9: name = "A "; break;
						case 10: name = "A#"; break;
						case 11: name = "B "; break;
					}
				}

				if (stringlen + 8 < ALIGNMAXLEN)
				{
					snprintf(alignstrp, 9, "%s&nbsp;", name);
					alignstrp += 8;
					stringlen += 8;
				}
				else break;
			}

			if (stringlen + 4 < ALIGNMAXLEN)
			{ 
				snprintf(alignstrp, 5, "<br> ");
				alignstrp += 4;
			}

			for (j = 0; j < strlen(align_t); j++)
			{
				if (align_t[j] == GAP) name = "- ";
				else
				{
					switch (align_t[j] % VOCSIZE)
					{
						case 0: name = "C "; break;
						case 1: name = "C#"; break;
						case 2: name = "D "; break;
						case 3: name = "D#"; break;
						case 4: name = "E "; break;
						case 5: name = "F "; break;
						case 6: name = "F#"; break;
						case 7: name = "G "; break;
						case 8: name = "G#"; break;
						case 9: name = "A "; break;
						case 10: name = "A#"; break;
						case 11: name = "B "; break;
					}
				}
				if (stringlen + 8 < ALIGNMAXLEN)
				{	
					snprintf(alignstrp, 9, "%s&nbsp;", name);
					alignstrp += 8;
					stringlen += 8;
				}
				else break;
			}
			alignstrp = "\0";
		}


		/* get values of other fields and print all */
		snprintf(rowstr, ROWMAXLEN - 1, "<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td><a href='%s'>MIDI</a></td><td><a href='histogram?filepath=%s'>Histogram</a></td><td>%s %s</td><td>%u</td><td><a href='midi?filepath=%s&firstchord=%u&lastchord=%u'>%s</a></td><td>%d</td><td>%ld</td><td>%ld</td><td>%s</td><tr>\n", \
		i + 1, \
		(strlen(t1 = RSTRING_PTR(rb_iv_get(song, "@composer"))) == 0) ? nastr : t1, \
		(strlen(t1 = RSTRING_PTR(rb_iv_get(song, "@title"))) == 0) ? nastr : t1, \
		(strlen(t1 = RSTRING_PTR(rb_iv_get(song, "@opus"))) == 0) ? nastr : t1, \
		(strlen(t1 = RSTRING_PTR(rb_iv_get(song, "@date"))) == 0) ? nastr : t1, \
		(strlen(t1 = RSTRING_PTR(rb_iv_get(song, "@style"))) == 0) ? nastr : t1, \
		midiurl, midiurl, \
		(strlen(scoreurl) == 0) ? nastr : scoreurl, \
		(strlen(pdfurl) == 0) ? "" : pdfurl, \
		bar, midiurl, firstchord, lastchord, \
		(strlen(notestr) == 0) ? playstr : notestr, transp, (long) NUM2INT(RARRAY_PTR(match)[5]), \
		(RARRAY_LEN(match) == 7) ? (long) NUM2INT(RARRAY_PTR(match)[6]): 0, \
		(RARRAY_LEN(match) == 8) ? alignstr : "");

		/* add this row to main result string */
		rb_str_cat2(s, rowstr);
	}

	/* add table end header to main result string */
	rb_str_concat(s, rb_str_new2("</table>"));

	return s;
}


/*
   function definitions
*/
void Init_Server()
{
	cMIR = rb_define_module("MIR");
	cServer = rb_define_class_under(cMIR, "Server", rb_cObject);

	/*rb_define_module_function(cServer, "matches_to_html", c_matches_to_html, 1);*/
	rb_define_method(cServer, "matches_to_html", c_matches_to_html, 1);
}
