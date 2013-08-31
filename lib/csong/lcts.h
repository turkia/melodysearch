/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen

   Contacts: vmakinen at cs helsinki fi

   This unit implements a data structure for one-dimensional dynamic semi-infinite
   range minimum queries for keys in range [0...n-1]. Query time is O(log n). 
   The structure is a complete binary tree with n leaves supporting 
   predecessor and successor queries, and insertions and deletions.

   Range query [-infty,x) is answered by calling i=predecessor(x) and
   taking value l(i) of i from an external array.

   Inserting a point x is done by calling insert(x) and then calling
   deleteGreaterSuccessors(x,values), which removes all successors
   of x that have greater value than x. This guarantees that range
   query [-infty,x) is answered correctly.
*/
 

#define max2(a,b) ((a)>(b)?(a):(b))
#define min2(a,b) ((a)<(b)?(a):(b))
#define MAX_TRANSPOSITION 256


typedef struct {
	unsigned int mantissa2 : 32;
	unsigned int mantissa1 : 20;
	unsigned int exponentbias1023 : 11;
	unsigned int signbit : 1;
} doubleMask;

typedef union {
	double asDouble;
	doubleMask asMask;
} doubleAndMask;

typedef struct {
	unsigned char isLeft : 1;
	unsigned char isRight : 1;
	unsigned char dummy : 6;
} treeNode;

typedef struct {
	int value;
	int t;
} occType;


/* moved here from lcts.c so that we can allocate mem in wrapper */
typedef struct {
	int i;
	int j;
} keyType;

typedef struct integerTag {
	struct integerTag *next;
	int x;
} integerNode;

typedef struct integerListTag {
	integerNode *first;
	integerNode *last;
} integerList;


typedef struct keyTypeNodeTag {
	keyType key;
	struct keyTypeNodeTag *next;
} keyTypeNode;


typedef struct keyTypeList {
	keyTypeNode *first;
	keyTypeNode *last;
} matchList;



inline unsigned int log_2(unsigned long n);
treeNode *CreateCompleteBinaryTree(unsigned int leaves);
unsigned int predecessor(treeNode *A, unsigned int leaves, unsigned int index);
unsigned int successor(treeNode *A, unsigned int leaves, unsigned int index);
void insertLeaf(treeNode *A, unsigned int leaves, unsigned int index);
void deleteLeaf(treeNode *A, unsigned int leaves, unsigned int index);
void deleteGreaterSuccessors(treeNode *A, unsigned int leaves, unsigned int index, int *values);

int computeAllTranspositions(matchList *Mt, char *A, char *B, int m, int n);
occType* searchAllTranspositions(matchList *Mt, char *P, char *T, int m, int n, int k);
int align(char *A, char *B, char *align_A, char * align_B,int t, int *startposition);

