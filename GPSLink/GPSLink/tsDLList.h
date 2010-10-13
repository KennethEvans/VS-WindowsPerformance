/*
 *      $Id: tsDLList.h,v 1.1 1999/06/17 14:04:37 evans Exp $
 *
 *	type safe doubly linked list templates
 *
 *      Author  Jeffrey O. Hill
 *              johill@lanl.gov
 *              505 665 1831
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 *
 * History
 * $Log: tsDLList.h,v $
 * Revision 1.1  1999/06/17 14:04:37  evans
 * Most of the capability except saving files has been done and appears
 * to work, though more testing is necessary.  Jeff's tsDLList.h has been
 * copied here to avoid any dependence on epics.  Some checking on 8-bit
 * displays has been done.
 *
 * Revision 1.16  1999/05/03 15:39:57  jhill
 * made compatible with visual C++ 6.0
 *
 * Revision 1.15  1999/02/01 21:49:04  jhill
 * removed redundant API
 *
 * Revision 1.14  1998/10/23 00:20:41  jhill
 * attempted to clean up HP-UX warnings
 *
 * Revision 1.13  1998/06/16 03:01:44  jhill
 * cosmetic
 *
 * Revision 1.12  1998/05/29 17:25:47  jhill
 * allow use of epicsAssert.h
 *
 * Revision 1.11  1998/05/05 18:06:57  jhill
 * rearranged to allow compilation by g++ 2.8.1
 *
 * Revision 1.10  1998/02/05 23:28:21  jhill
 * fixed hp sompiler warnings
 *
 * Revision 1.9  1997/06/13 09:21:52  jhill
 * fixed compiler compatibility problems
 *
 * Revision 1.8  1997/04/11 20:49:48  jhill
 * added no arg reset() to bwd iter
 *
 * Revision 1.7  1997/04/10 19:43:10  jhill
 * API changes
 *
 * Revision 1.6  1997/01/22 21:13:49  jhill
 * fixed class decl order for VMS
 *
 * Revision 1.5  1996/11/02 01:07:19  jhill
 * many improvements
 *
 * Revision 1.4  1996/08/14 12:32:09  jbk
 * added first() to list class, added first()/last() to iterator.
 *
 * Revision 1.3  1996/07/25 18:01:41  jhill
 * use pointer (not ref) for list in iter class
 *
 * Revision 1.2  1996/07/24 22:12:03  jhill
 * added remove() to iter class + made node's prev/next private
 *
 * Revision 1.1.1.1  1996/06/20 22:15:55  jhill
 * installed  ca server templates
 *
 *
 */

#ifndef tsDLListH_include
#define tsDLListH_include

#ifndef UNDER_CE
#ifndef assert // allow use of epicsAssert.h
#include <assert.h>
#endif
#endif

template <class T> class tsDLList;
template <class T> class tsDLIterBD;
template <class T> class tsDLIter;
template <class T> class tsDLFwdIter;
template <class T> class tsDLBwdIter;

//
// tsDLNode<T>
// NOTE: T must derive from tsDLNode<T>
//
template <class T>
class tsDLNode {
friend class tsDLList<T>;
friend class tsDLIterBD<T>;
friend class tsDLIter<T>;
friend class tsDLFwdIter<T>;
friend class tsDLBwdIter<T>;
public:
	tsDLNode() : pNext(0), pPrev(0) {}
	//
	// when someone copies in a class deriving from this
	// do _not_ change the node pointers
	//
	void operator = (const tsDLNode<T> &) {}

protected:
	T	*getNext(void) { return pNext; }
	T	*getPrev(void) { return pPrev; }
private:
	T	*pNext;
	T	*pPrev;
};

//
// tsDLList<T>
// NOTE: T must derive from tsDLNode<T>
//
template <class T>
class tsDLList {
friend class tsDLIter<T>;
friend class tsDLFwdIter<T>;
friend class tsDLBwdIter<T>;
private:
	//
	// clear()
	//
	void clear()
	{
		this->pFirst = 0;
		this->pLast = 0;
		this->itemCount = 0u;
	}

public:

	tsDLList () 
	{
		clear();
	}

	//
	// count()
	// (returns the number of items on the list)
	//
	unsigned count() const
	{
		return this->itemCount; 
	}

	//
	// add() - 
	// adds addList to the end of the list
	// (and removes all items from addList)
	//
	void add (tsDLList<T> &addList);

	//
	// add() 
	// (add an item to the end of the list)
	//
	void add (T &item);

	//
	// get ()
	//
	T * get();

	//
	// insertAfter()
	// (place item in the list immediately after itemBefore)
	//
	void insertAfter (T &item, T &itemBefore);

	//
	// insertBefore ()
	// (place item in the list immediately before itemAfter)
	//
	void insertBefore (T &item, T &itemAfter);

	//
	// remove ()
	//
	void remove (T &item);

	//
	// pop ()
	// (returns the first item on the list)
	T * pop();

	//
	// push ()
	// (add an item at the beginning of the list)
	//
	void push (T &item);
	
	//
	// find 
	// returns -1 if the item isnt on the list
	// and the node number (beginning with zero if
	// it is)
	//
	int find(T &item) const;

	T *first(void) const { return this->pFirst; }
	T *last(void) const { return this->pLast; }

private:
	T		*pFirst;
	T		*pLast;
	unsigned	itemCount;

	//
	// past functionality incompatible
	// with strict definition of a copy
	// constructor so this has been eliminated
	//
	// copying one list item into another and
	// ending up with to list headers pointing
	// at the same list is always a questionable
	// thing to do.
	//
	// therefore, this is intentionally _not_
	// implemented.
	//
	tsDLList (const tsDLList &);
};


//
// tsDLIterBD<T>
// (a bi-directional iterator in the style of the STL)
//
template <class T>
class tsDLIterBD {
public:
	tsDLIterBD () : 
		pEntry(0) {}

	tsDLIterBD (T *pInitialEntry) : 
		pEntry(pInitialEntry) {}

	//
	// This is apparently required by some compiler, causes 
	// trouble with MS Visual C 6.0, but should not be 
	// required by any compiler. I am assuming that the 
	// other compiler is a past version of MS Visual C.
	//	
#	if defined(_MSC_VER) && _MSC_VER < 1200
		template <class T>
		tsDLIterBD (const class tsDLIterBD<T> &copyIn) :
			pEntry(copyIn.pEntry) {}
#	endif

	tsDLIterBD<T> & operator = (T *pNewEntry)
	{
		this->pEntry = pNewEntry;
		return *this;
	}

	tsDLIterBD<T> &operator = (const tsDLIterBD<T> &copyIn)
	{
		this->pEntry = copyIn.pEntry;
		return *this;
	}

	int operator == (const tsDLIterBD<T> &rhs) const
	{
		return (this->pEntry == rhs.pEntry);
	}

	int operator != (const tsDLIterBD<T> &rhs) const
	{
		return (this->pEntry != rhs.pEntry);
	}

	T & operator * () const
	{
		return *this->pEntry;
	}

	T * operator -> () const
	{
		return this->pEntry;
	}

	operator T* () const
	{
		return this->pEntry;
	}

	//
	// prefix ++
	//
	T *operator ++ () 
	{
		tsDLNode<T> *pNode = this->pEntry;
		return this->pEntry = pNode->pNext;
	}

	//
	// postfix ++
	//
	T *operator ++ (int) 
	{
		T *pE = this->pEntry;
		tsDLNode<T> *pNode = this->pEntry;
		this->pEntry = pNode->pNext;
		return pE;
	}

	//
	// prefix -- 
	//
	T *operator -- () 
	{
		tsDLNode<T> *pEntryNode = pEntry;
		return this->pEntry = pEntryNode->pPrev;
	}

	//
	// postfix -- 
	//
	T *operator -- (int) 
	{
		T *pE = this->pEntry;
		tsDLNode<T> *pEntryNode = pEntry;
		this->pEntry = pEntryNode->pPrev;
		return pE;
	}
private:
	T *pEntry;
};

//
// tsDLIter<T>
//
// Notes:
// 2) This iterator does not allow for removal
//	of an item in order to avoid problems 
//	resulting when we remove an item (and 
//	then dont know whether to make pCurrent 
//	point at the item before or after the 
//	item removed
//
template <class T>
class tsDLIter {
public:
	tsDLIter (tsDLList<T> & listIn) : 
		pCurrent(0), pList(&listIn) {}

	void reset ()
	{
		this->pCurrent = 0;
	}

	void reset (tsDLList<T> &listIn)
	{
	    this->reset();
	    this->pList = &listIn;
	}

	void operator = (tsDLList<T> &listIn)
	{
	    this->reset(listIn);
	}

	T * next ();

	T * prev ();

	T * first()
	{
		this->pCurrent = this->pList->pFirst;
		return this->pCurrent;
	}

	T * last()
	{
	    this->pCurrent = this->pList->pLast;
	    return this->pCurrent;
	}

	T * operator () ();

protected:
	T			*pCurrent;
	tsDLList<T>	*pList;
};

//
// tsDLFwdIter<T>
//
// Notes:
// 1) No direct access to pCurrent is provided since
// 	this might allow for confusion when an item
//	is removed (and pCurrent ends up pointing at
//	an item that has been seen before)
//
// 2) This iterator only moves forward in order to 
//	avoid problems resulting when we remove an
//	item (and then dont know whether to make 
//	pCurrent point at the item before or after
// 	the item removed
//
template <class T>
class tsDLFwdIter: private tsDLIter<T> {
public:
	tsDLFwdIter (tsDLList<T> &listIn) : 
		tsDLIter<T>(listIn) {}

	void reset ()
	{
		this->tsDLIter<T>::reset();
	}

	void reset (tsDLList<T> &listIn)
	{
	    this->tsDLIter<T>::reset(listIn);
	}

	void operator = (tsDLList<T> &listIn)
	{
	    this->tsDLIter<T>::reset(listIn);
	}

	T * operator () ();

	T * next ();

	T * first()
	{
		tsDLIter<T> &iterBase = *this;
		return iterBase.first();
	}

	//
	// remove ()
	// (and move current to be the item
	// pointed to by pPrev - the item seen
	// by the iterator before the current one -
	// this guarantee that the list will be
	// accessed sequentially even if an item
	// is removed)
	//
	void remove ();
};

//
// tsDLBwdIter<T>
//
// Notes:
// 1) No direct access to pCurrent is provided since
// 	this might allow for confusion when an item
//	is removed (and pCurrent ends up pointing at
//	an item that has been seen before)
//
// 2) This iterator only moves backward in order to 
//	avoid problems resulting when we remove an
//	item (and then dont know whether to make 
//	pCurrent point at the item before or after
// 	the item removed
//
template <class T>
class tsDLBwdIter : private tsDLIter<T> {
public:
	tsDLBwdIter(tsDLList<T> &listIn) : 
		tsDLIter<T>(listIn) {}

	void reset ()
	{
		this->tsDLIter<T>::reset();
	}

	void reset (tsDLList<T> &listIn)
	{
	    this->tsDLIter<T>::reset(listIn);
	}

	void operator = (tsDLList<T> &listIn)
	{
	    this->tsDLIter<T>::reset(listIn);
	}

	T * operator () ();

	T * prev ();

	T * last()
	{
		return this->tsDLIter<T>::last();
	}

	//
	// remove ()
	// remove current item 
	// (and move current to be the item
	// pointed to by pNext - the item seen
	// by the iterator before the current one -
	// this guarantee that the list will be
	// accessed sequentially even if an item
	// is removed)
	//
	void remove ();
};

//
// tsDLList<T>::remove ()
//
template <class T>
void tsDLList<T>::remove (T &item)
{
	tsDLNode<T> &node = item;

	if (this->pLast == &item) {
		this->pLast = node.pPrev;
	}
	else {
		tsDLNode<T> *pNextNode = node.pNext;
		pNextNode->pPrev = node.pPrev; 
	}

	if (this->pFirst == &item) {
		this->pFirst = node.pNext;
	}
	else {
		tsDLNode<T> *pPrevNode = node.pPrev;
		pPrevNode->pNext = node.pNext;
	}
	
	this->itemCount--;
}

//
// tsDLList<T>::get ()
//
template <class T>
inline T * tsDLList<T>::get()
{
	T *pItem = this->pFirst;

	if (pItem) {
		this->remove (*pItem);
	}
	
	return pItem;
}

//
// tsDLList<T>::pop ()
// (returns the first item on the list)
template <class T>
inline T * tsDLList<T>::pop()
{
	return this->get();
}

//
// add() - 
// adds addList to the end of the list
// (and removes all items from addList)
//
template <class T>
void tsDLList<T>::add (tsDLList<T> &addList)
{
	//
	// NOOP if addList is empty
	//
	if (addList.itemCount==0u) {
		return;
	}

	if (this->itemCount==0u) {
		//
		// this is empty so just init from 
		// addList 
		//
		*this = addList;
	}
	else {
		tsDLNode<T> *pLastNode = this->pLast;
		tsDLNode<T> *pAddListFirstNode = addList.pFirst;

		//
		// add addList to the end of this
		//
		pLastNode->pNext = addList.pFirst;
		pAddListFirstNode->pPrev = addList.pLast;
		this->pLast = addList.pLast;
		this->itemCount += addList.itemCount;
	}

	//
	// leave addList empty
	//
	addList.clear();
}

//
// add() 
// (add an item to the end of the list)
//
template <class T>
void tsDLList<T>::add (T &item)
{
	tsDLNode<T> &node = item;

	node.pNext = 0;
	node.pPrev = this->pLast;

	if (this->itemCount) {
		tsDLNode<T> *pLastNode = this->pLast;
		pLastNode->pNext = &item;
	}
	else {
		this->pFirst = &item;
	}

	this->pLast = &item;

	this->itemCount++;
}

//
// insertAfter()
// (place item in the list immediately after itemBefore)
//
template <class T>
void tsDLList<T>::insertAfter (T &item, T &itemBefore)
{
	tsDLNode<T> &node = item;
	tsDLNode<T> &nodeBefore = itemBefore;

	node.pPrev = &itemBefore;
	node.pNext = nodeBefore.pNext;
	nodeBefore.pNext = &item;

	if (node.pNext) {
		tsDLNode<T> *pNextNode = node.pNext;
		pNextNode->pPrev = &item;
	}
	else {
		this->pLast = &item;
	}

	this->itemCount++;
}

//
// insertBefore ()
// (place item in the list immediately before itemAfter)
//
template <class T>
void tsDLList<T>::insertBefore (T &item, T &itemAfter)
{
	tsDLNode<T> &node = item;
	tsDLNode<T> &nodeAfter = itemAfter;

	node.pNext = &itemAfter;
	node.pPrev = nodeAfter.pPrev;
	nodeAfter.pPrev = &item;

	if (node.pPrev) {
		tsDLNode<T> *pPrevNode = node.pPrev;
		pPrevNode->pNext = &item;
	}
	else {
		this->pFirst = &item;
	}

	this->itemCount++;
}

//
// push ()
// (add an item at the beginning of the list)
//
template <class T>
void tsDLList<T>::push (T &item)
{
	tsDLNode<T> &node = item;
	node.pPrev = 0;
	node.pNext = this->pFirst;

	if (this->itemCount) {
		tsDLNode<T> *pFirstNode = this->pFirst;
		pFirstNode->pPrev = &item;
	}
	else {
		this->pLast = &item;
	}

	this->pFirst = &item;

	this->itemCount++;
}

//
// tsDLIter<T>::next ()
//
template <class T> 
T * tsDLIter<T>::next () 
{
	T *pCur = this->pCurrent;
	if (pCur==0) {
		pCur = this->pList->pFirst;
	}
	else {
		tsDLNode<T> *pCurNode = pCur;
		pCur = pCurNode->pNext;
	}
	this->pCurrent = pCur;
	return pCur;
}

//
// tsDLIter<T>::prev ()
//
template <class T>
T * tsDLIter<T>::prev ()
{
	T *pCur = this->pCurrent;
	if (pCur==0) {
		pCur = this->pList->pLast;
	}
	else {
		tsDLNode<T> *pCurNode = pCur;
		pCur = pCurNode->pPrev;
	}
	this->pCurrent = pCur;
	return pCur;
}

//
// tsDLIter<T>::operator () ()
//
template <class T>
inline T * tsDLIter<T>::operator () () 
{
	return this->next();
}

//
// remove ()
// remove current item 
// (and move current to be the item
// pointed to by pNext - the item seen
// by the iterator before the current one -
// this guarantee that the list will be
// accessed sequentially even if an item
// is removed)
//
template <class T>
void tsDLBwdIter<T>::remove ()
{
	T *pCur = this->pCurrent;

	if (pCur) {
		tsDLNode<T> *pCurNode = pCur;

		//
		// strip const (we didnt declare the
		// list const in the constructor)
		//
		tsDLList<T> * pMutableList = 
			(tsDLList<T> *) this->pList;

		//
		// Move this->pCurrent to the item after the
		// item being deleted 
		//
		this->pCurrent = pCurNode->pNext;

		//
		// delete current item
		//
		pMutableList->remove(*pCur);
	}
}

//
// tsDLBwdIter<T>::operator () ()
//
template <class T>
inline T * tsDLBwdIter<T>::operator () ()
{
	return this->tsDLIter<T>::prev();
}

//
// tsDLBwdIter<T>::prev () 
//
template <class T>
inline T * tsDLBwdIter<T>::prev () 
{
	return this->tsDLIter<T>::prev();
}

//
// tsDLFwdIter<T>::remove ()
// (and move current to be the item
// pointed to by pPrev - the item seen
// by the iterator before the current one -
// this guarantee that the list will be
// accessed sequentially even if an item
// is removed)
//
template <class T>
void tsDLFwdIter<T>::remove ()
{
	T *pCur = this->pCurrent;

	if (pCur) {
		tsDLNode<T> *pCurNode = pCur;

		//
		// Move this->pCurrent to the previous item
		//
		this->pCurrent = pCurNode->pPrev;

		//
		// delete current item
		//
		this->pList->remove(*pCur);
	}
}

//
// tsDLFwdIter<T>::next ()
// 
template <class T>
inline T * tsDLFwdIter<T>::next () 
{
	tsDLIter<T> &iterBase = *this;
	return iterBase.next();
}

//
// tsDLFwdIter<T>::operator () ()
// 
template <class T>
inline T * tsDLFwdIter<T>::operator () () 
{
	return this->next();
}

//
// tsDLList<T>::find () 
// returns -1 if the item isnt on the list
// and the node number (beginning with zero if
// it is)
//
template <class T>
int tsDLList<T>::find(T &item) const
{
	tsDLFwdIter<T>	iter(*this);
	tsDLNode<T>	*pItem;
	int		itemNo=0;

	while ( (pItem = iter.next()) ) {
		if (pItem == &item) {
			return itemNo;
		}
		itemNo++;
	}
	return -1;
}

#endif // tsDLListH_include
