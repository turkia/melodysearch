
#include "song.h"

/* Struct for items in the vertical translation table. 
   It stores value, slope and previous x for each vertical translation (y). */
typedef struct {
	int slope;
	int value;
	unsigned int prev_x;
} VerticalTranslationTableItem;


/*
   Startpoints (strt,ptch) and endpoints (strt+dur,ptch) are called turning points. 
   They are precalculated and stored in separate arrays of this struct. 
   This adds to space requirements but makes algorithm simpler. 
   (startpoints could be traversed from source data but endpoint 
   order must be stored somehow anyway.) */
typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int textchordind;
} TurningPoint;


typedef struct {
	/* first pointer is for storing startpoints and the second for endpoints. */
	/* one pointer for each pattern item. */
	TurningPoint *startpoint;
	TurningPoint *endpoint;
} TurningPointPointer;


typedef struct {
	/* index to turning point array; which turning point is associated with this translation vector. */
	unsigned int tpindex;
	unsigned int patternindex;
	long int x;
	long int y;
	char text_is_start;
	char pattern_is_start;
} TranslationVector;
 

