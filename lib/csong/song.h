/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003
   Copyright Mika Turkia
*/

#include <ruby.h>
#include <math.h>

#define VOCSIZE 12
#define NOTELEN 4
#define CHORDHEADERLEN 5
#define PP_ITEM_SIZE (sizeof(unsigned int) + sizeof(unsigned short int))
#define PNOTERESOLUTION 960
#define MAX_PATTERN_NOTES 40
#define GAP_UNSIGNED 255
#define GAP_SIGNED -127

#define max2(a,b) ((a)>(b)?(a):(b))
#define min2(a,b) ((a)<(b)?(a):(b))


/* Two-dimensional vector struct for storing patterns. */
typedef struct {
	unsigned int strt;
	char ptch;
	unsigned short dur;
} vector;


VALUE c_shiftorand_init(VALUE self, VALUE init_info);
VALUE c_shiftorand_scan(VALUE self, VALUE init_info);

VALUE c_monopoly_preprocess(VALUE self);
VALUE c_monopoly_init(VALUE self, VALUE init_info);
VALUE c_monopoly_scan(VALUE self, VALUE init_info);

VALUE c_intervalmatching_init(VALUE self, VALUE init_info);
VALUE c_intervalmatching_scan(VALUE self, VALUE init_info);

VALUE c_matchedchords(VALUE self, VALUE firstchord, VALUE lastchord);
VALUE c_pitch_histogram(VALUE self);
VALUE c_pitch_histogram_folded(VALUE self);
VALUE c_pitch_interval_histogram(VALUE self);
VALUE c_duration_histogram(VALUE self);

VALUE c_geometric_p1_scan(VALUE self, VALUE init_info);
VALUE c_geometric_p2_init(VALUE self, VALUE init_info);
VALUE c_geometric_p2_scan(VALUE self, VALUE init_info);

VALUE c_geometric_p3_init(VALUE self, VALUE init_info);
VALUE c_geometric_p3_scan(VALUE self, VALUE init_info);

VALUE c_splitting_scan(VALUE self, VALUE init_info);

VALUE c_lcts_scan(VALUE self, VALUE init_info);
VALUE c_lcts_distances(VALUE self, VALUE song2);

VALUE c_dynprog_scan(VALUE self, VALUE init_info);
