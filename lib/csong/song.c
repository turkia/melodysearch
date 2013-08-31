/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Mika Turkia

   The functions defined in this class become methods of Song class
   when this library is loaded with require command from a Ruby program.
*/

#include "song.h"

VALUE cSong;
VALUE cMIR;

/*
   Function definitions for Song class. You can define initialization methods, scanning methods, post-scanning methods and other methods.
   The methods defined here become part of Song class and can be called directly from Ruby code. 

   When a search is started for a SongCollection instance with algorithm <name>, 
   method init_<name> is called first. Then method scan_<name> is called for each song in the collection.
   After the scanning post_<name> is called. Methods init_<name> and post_<name> are optional and their 
   existence is checked dynamically before calls.

   Other functions include preprocessing functions (e.g. for MonoPoly), histogram creation etc.
*/
void Init_Song()
{
	cMIR = rb_define_module("MIR");
	cSong = rb_define_class_under(cMIR, "Song", rb_cObject);

	/* preprocessing functions */
	rb_define_method(cSong, "preprocess_monopoly", c_monopoly_preprocess, 0);

	/* optional initialization functions; called before search if defined. */
	rb_define_module_function(cSong, "init_monopoly", c_monopoly_init, 1);
	rb_define_module_function(cSong, "init_shiftorand", c_shiftorand_init, 1);
	rb_define_module_function(cSong, "init_intervalmatching", c_intervalmatching_init, 1);
	rb_define_module_function(cSong, "init_geometric_p2", c_geometric_p2_init, 1);
	rb_define_module_function(cSong, "init_geometric_p3", c_geometric_p3_init, 1);

	/* scanning functions */
	rb_define_method(cSong, "scan_monopoly", c_monopoly_scan, 1);
	rb_define_method(cSong, "scan_shiftorand", c_shiftorand_scan, 1);
	rb_define_method(cSong, "scan_intervalmatching", c_intervalmatching_scan, 1);
	rb_define_method(cSong, "scan_geometric_p1", c_geometric_p1_scan, 1);
	rb_define_method(cSong, "scan_geometric_p2", c_geometric_p2_scan, 1);
	rb_define_method(cSong, "scan_geometric_p3", c_geometric_p3_scan, 1);
	rb_define_method(cSong, "scan_lcts", c_lcts_scan, 1);
	rb_define_method(cSong, "scan_splitting", c_splitting_scan, 1);
	rb_define_method(cSong, "scan_dynprog", c_dynprog_scan, 1);

	/* function to get chord data for playing a match */
	/* arguments: numbers of the first and the last chord */
	rb_define_method(cSong, "get_matched_chords", c_matchedchords, 2);
	rb_define_method(cSong, "pitch_histogram", c_pitch_histogram, 0);
	rb_define_method(cSong, "pitch_histogram_folded", c_pitch_histogram_folded, 0);
	rb_define_method(cSong, "pitch_interval_histogram", c_pitch_interval_histogram, 0);
	rb_define_method(cSong, "duration_histogram", c_duration_histogram, 0);
	rb_define_method(cSong, "lcts_distances", c_lcts_distances, 1);

	/* optional post-scan phase functions; called after search if defined. */
	/* rb_define_module_function(cSong, "post_<name>", c_<name>_post, 1); */
}
