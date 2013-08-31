/*
   C-Brahms Engine for Musical Information Retrieval
   University of Helsinki, Department of Computer Science

   Version 0.2.4, May 15th, 2003

   Copyright Veli Makinen

   Contacts: vmakinen at cs helsinki fi

   This unit implements a sliding window version of Cartesian tree.
*/


#include <ruby.h>

#include <stdio.h>
#include "splitting.h"
#define max(x,y) ((x)>(y)?(x):(y))


TCartesianNode *newCartesianNode(int v, keyType k, TCartesianNode *l, TCartesianNode *r, TCartesianNode *p)
{
	TCartesianNode *temp = (TCartesianNode*) malloc(sizeof(TCartesianNode));
	temp->value = v;
	temp->key = k;
	temp->left = l;
	temp->right = r;
	temp->parent = p;
	temp->next = NULL;
	return temp;
}


TCartesianNode *Add(TCartesianNode *node, int v, keyType k)
{
	if (v<node->value && node->parent != NULL) 
	{
		// Proceed to the parent
		return Add(node->parent,v,k);
	}
	else if (v<node->value) 
	{  
		// Add new root
		TCartesianNode *newroot = newCartesianNode(v,k,node,NULL,NULL);
		node->parent = newroot;
		return newroot;
	} 
	else 
	{ 
		// Create new node with value v, add it to right son of current node
		TCartesianNode *newnode = newCartesianNode(v,k,node->right,NULL,node);
		if (node->right != NULL) node->right->parent = newnode;
		node->right = newnode;
		return newnode;
	};
}


TCartesianNode *Delete(TCartesianNode *node) 
{ 
	// must be called first->Delete()
	TCartesianNode *newroot = NULL;

	if (node->right != NULL) node->right->parent = node->parent;

	if (node->parent != NULL) node->parent->left = node->right;
	else newroot = node->right;

	return newroot;
};


/*
   This unit implements a sliding window version of Cartesian tree.
   The tree supports in worst case constant time operations:
   - Eject(): deletes the first item from the list,
   - findMin(): returns the minimum value of elements in the list,
   - findKeyOfMin(): returns the key associated with the min.
   In amortized constant time works the operation:
   - Push(value,key): inserts a new element as the last item

   Given values A[0]...A[n-1], an Cartesian tree on A is such that
   the root stores element A[m], where A[m]<=A[i], for all 0<=i<n,
   left son of root stores A[lm], where A[lm]<=A[i], for all 0<=i<m,
   right son of root stores A[rm], where A[rm]<=A[i], for all m<i<n, etc.

   Let rmn be the right-most node in the tree.
   The well-known linear time construction of an Cartesian tree proceeds by
   calling Push(value,key) from A[0] to A[n-1]. Operation Push(...) is
   implemented by adding (value,key) as a new node at the first place,
   say before node v, on the path from rmn to the root, where
   v->value < value. Then (value,key) becomes the right son of v
   (and the old right son becomes the left son of (value,key)). If such
   node v is not found, then (value,key) is the new root. In any case,
   (value,key) becomes the new rmn. The cost of a single Push(...)
   operation can be charged onto those that increase the rmn-root path,
   hence the overall complexity stays linear.

   A small adjustment converts this construction algorithm into a
   sliding window minima algorithm; it is easy to remove the tail in
   constant time. Hence, we can report in linear time minima in
   all windows of size alpha of A.

   Remark. It is easy to allow Inject() and Pop() operations as well in
   amortized constant time.
*/
TCartesianTree *newCartesianTree()
{
	TCartesianTree *temp = (TCartesianTree*) malloc(sizeof(TCartesianTree));
	temp->root = NULL;
	temp->first = NULL;
	temp->last = NULL;
	return temp;
}


void Push(TCartesianTree *CT, int v, keyType k)
{
	if (CT->first == NULL) 
	{
		CT->root = newCartesianNode(v,k,NULL,NULL,NULL);
		CT->first = CT->root;
		CT->last = CT->root;
	} 
	else 
	{
		TCartesianNode *newnode = Add(CT->last,v,k);
		CT->last->next = newnode;
		CT->last = newnode;
		if (newnode->parent == NULL) CT->root = newnode;
	};
}


void Eject(TCartesianTree *CT)
{
	TCartesianNode *temp = Delete(CT->first);
	if (temp != NULL) CT->root = temp; temp = CT->first;
	CT->first = CT->first->next;
	if (CT->first == NULL) CT->root = NULL;
	free(temp);
}


int isEmpty(TCartesianTree *CT) 
{
	if (CT->first==NULL) return 1;
	else return 0;
}


int findMin(TCartesianTree *CT) { return CT->root->value; };

keyType findKeyOfMin(TCartesianTree *CT) { return CT->root->key; };

keyType firstKey(TCartesianTree *CT) { return CT->first->key; };

keyType lastKey(TCartesianTree *CT) { return CT->last->key; };

void Empty(TCartesianTree *CT) { while (!isEmpty(CT)) Eject(CT); };


