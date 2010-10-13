// Edit routines for GPSLink

#include "stdafx.h"

#define DEBUG_CLIPBOARD 0
#define DEBUG_SELECTED 0

#include "GPSLink.h"
#include "resource.h"

static CBlock *getPasteTarget(UINT selectedCount, BOOL homogenous);

BOOL deleteCheckedBlocks(BOOL val)
{
	CBlock *pBlock,*pBlock1;
	BOOL resetTracks=FALSE;

	// Cannot use the bidirectional iterator with remove

	// Waypoints
	tsDLFwdIter<CBlock> iterWpt(waypointList);
	while(pBlock = iterWpt()) {
		if(pBlock->isChecked() == val) {
			// Remove it from its list without deleting it
			iterWpt.remove();
			// Set its parentList back to NULL
			pBlock->setParentList(NULL);
			delete pBlock;
		}
	}

	// Routes
	tsDLFwdIter<CBlock> iterRte(routeList);
	while(pBlock = iterRte()) {
		if(pBlock->isChecked() == val) {
			// Remove it from its list without deleting it
			iterRte.remove();
			// Set its parentList back to NULL
			pBlock->setParentList(NULL);
			delete pBlock;
		} else {
			// Do the waypoints
			CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			tsDLFwdIter<CBlock> iterBlock(*blockList);
			while(pBlock1 = iterBlock()) {
				if(pBlock1->isChecked() == val) {
					// Remove it from its list without deleting it
					iterBlock.remove();
					// Set its parentList back to NULL
					pBlock1->setParentList(NULL);
					delete pBlock1;
				}
			}
		}
	}

	// Tracks
	tsDLFwdIter<CBlock> iterTrk(trackList);
	while(pBlock = iterTrk()) {
		if(pBlock->isChecked() == val) {
			// Remove it from its list without deleting it
			iterTrk.remove();
			// Set its parentList back to NULL
			pBlock->setParentList(NULL);
			delete pBlock;
		} else {
			// Do the trackpoints
			CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			tsDLFwdIter<CBlock> iterBlock(*blockList);
			while(pBlock1 = iterBlock()) {
				if(pBlock1->isChecked() == val) {
					// Remove it from its list without deleting it
					iterBlock.remove();
					// Set its parentList back to NULL
					pBlock1->setParentList(NULL);
					delete pBlock1;
					resetTracks=TRUE;
				}
			}
		}
	}

	// Insure all tracks start with a start trackpoint
	if(resetTracks) {
		trackListSetStart();
	}

	// Refresh the list views
	if(hWptLV) refreshLV(hWptLV);
	if(hRteLV) refreshLV(hRteLV);
	if(hTrkLV) refreshLV(hTrkLV);

	return TRUE;
}

BOOL removeSelectedBlocks(EditType editType)
{
	UINT selectedCount;
	CBlock *pBlock;
	CBlockList *blockList;
	BOOL resetTracks=FALSE;
	// Use a tsDLList so it won't change the parentList
	tsDLList<CBlock> selectedList;
	// Cannot use the bidirectional iterator with remove
	tsDLFwdIter<CBlock> iterF(selectedList);
	int index;

	// The current window must be one of the list view windows
	if(!hCurWnd) return FALSE;
	if(hCurWnd != hWptLV  && hCurWnd != hRteLV && 
		hCurWnd != hTrkLV) {
			errMsg(_T("Nothing selected"));
			return FALSE;
	}

	// See if there is anything selected
	selectedCount=ListView_GetSelectedCount(hCurWnd);
#if DEBUG_SELECTED
	lbprintf(_T("remove: clipboard: %d selected: %d"),
		clipboardList.getCount(),selectedCount);
#endif
	if(!selectedCount) {
		errMsg(_T("Nothing selected"));
		return FALSE;
	}

	// Get the first index
	index=ListView_GetNextItem(hCurWnd,-1,LVNI_SELECTED);
	if(index < 0) return FALSE;
#if DEBUG_SELECTED
	pBlock=lvGetBlockFromItem(hCurWnd,index);
	if(pBlock) {
		LPWSTR unicodeString=NULL;

		ansiToUnicode(pBlock->getIdent(),&unicodeString);
		if(unicodeString) {
			lbprintf(_T("First selected: %s"),unicodeString);
			free(unicodeString);
		} else {
			lbprintf(_T("First selected: <Convert error>"));
		}
	} else {
		lbprintf(_T("First selected: NULL"));
	}
#endif

	// Get the selected items and put them in the selected list
	do {
		pBlock=lvGetBlockFromItem(hCurWnd,index);
		if(pBlock) {
			blockList=pBlock->getParentList();
			if(editType != EDIT_COPY) {
				// Is a cut or delete.  Remove it from its list but
				// don't delete it
				blockList->removeBlock(pBlock,FALSE);
				// Restore its parentList that was nulled
				pBlock->setParentList(blockList);
				// Add it to the selected list
				selectedList.add(*pBlock);
			} else {
				// Is a copy
				pBlock=pBlock->clone();
				pBlock->setParentList(blockList);
				selectedList.add(*pBlock);
			}
		}
		index=ListView_GetNextItem(hCurWnd,index,LVNI_SELECTED);
	} while(index >= 0);

	// Copy the list into the clipboard
	if(editType == EDIT_CUT || editType == EDIT_COPY) {
		clipboardList=selectedList;
#if DEBUG_CLIPBOARD
		lbprintf(_T("clipboardList: %d selectedList : %d"),
			clipboardList.getCount(),selectedList.count());
		clipboardList.dump();
#endif
	}

	// If it is not a copy, delete the blocks in the selected list
	if(editType != EDIT_COPY) {
		// First loop over the selected list and delete items in
		// sublists so we don't ever try to remove items that have
		// already been removed
		while(pBlock = iterF()) {
			int type=pBlock->getType();
			if(type == TYPE_TPT) {
				// Flag resetting tracks to begin with a start trackpoint
				resetTracks=TRUE;
				// Remove it from the selected list and delete it
				iterF.remove();
				// Set its parentList back to NULL
				pBlock->setParentList(NULL);
				delete pBlock;
			} else if(type == TYPE_WPT) {
				if(pBlock->getParentList() != &waypointList) {
					// Remove it from the selected list and delete it
					iterF.remove();
					// Set its parentList back to NULL
					pBlock->setParentList(NULL);
					delete pBlock;
				}
			}
		}

		// Now remove the waypoints, routes, or tracks minus any
		// subitems that may have been checked
		iterF.reset();
		while(pBlock = iterF()) {
			// Remove it from the selected list and delete it
			iterF.remove();
			// Set its parentList back to NULL
			pBlock->setParentList(NULL);
			delete pBlock;
		}
	}

	// Insure all tracks start with a start trackpoint
	if(resetTracks) {
		trackListSetStart();
	}

	// Refresh the list view (must be current to have selection)
	refreshLV(hCurWnd);

	return TRUE;
}

BOOL pasteBlocks(WPARAM wParam)
{
	BOOL waypoints=FALSE;
	BOOL routes=FALSE;
	BOOL tracks=FALSE;
	BOOL trackpoints=FALSE;
	int editType=LOWORD(wParam); 
	UINT selectedCount;
	BOOL first=TRUE;
	BOOL homogenous=TRUE;
	int dataType;
	CBlock *pBlock,*pBlock1;
	CBlockList *blockList;

	// See if there is anything in the clipboard
	if(!clipboardList.getCount()) {
		errMsg(_T("Nothing to paste"));
		return FALSE;
	}

	// See if there is anything selected
	selectedCount=ListView_GetSelectedCount(hCurWnd);
#if DEBUG_SELECTED
	lbprintf(_T("paste: clipboard: %d selected: %d"),
		clipboardList.getCount(),selectedCount);
	if(selectedCount) {
		int index;
		index=ListView_GetNextItem(hCurWnd,-1,LVNI_SELECTED);
		if(index >= 0) {
			pBlock=lvGetBlockFromItem(hCurWnd,index);
			if(pBlock) {
				LPWSTR unicodeString=NULL;

				ansiToUnicode(pBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					lbprintf(_T("First selected: %s at %d"),
						unicodeString,index);
					free(unicodeString);
				} else {
					lbprintf(_T("First selected: <Convert error> at %d"),
						index);
				}
			} else {
				lbprintf(_T("First selected: NULL at %d"),index);
			}
		} else {
			lbprintf(_T("First selected: %d (invalid)"),index);
		}
	}
#endif

	// See what is in the clipboard
	tsDLIterBD<CBlock> iter(clipboardList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		switch(pBlock->getType()) {
	case TYPE_WPT:
		waypoints=TRUE;
		break;
	case TYPE_RTE:
		routes=TRUE;
		break;
	case TYPE_TRK:
		tracks=TRUE;
		break;
	case TYPE_TPT:
		trackpoints=TRUE;
		break;
		}
		if(first) {
			dataType=pBlock->getType();
			first=FALSE;
		} else {
			if(dataType != pBlock->getType()) homogenous=FALSE;
		}
		iter++;
	}	

	// Check if paste is meaningful
	if(tracks && trackpoints) {
		errMsg(_T("Cannot paste both tracks and trackpoints"));
		return FALSE;
	}

	switch(editType) {
	case ID_EDIT_PASTE_ATTOP:
		if(!selectedCount) {
			// Put things at the top of the relevant list
			if(trackpoints) {
				errMsg(_T("Do not know where to put trackpoints"));
				return FALSE;
			}
			iter=clipboardList.last();
			while(iter != eol) {
				pBlock=iter;
				pBlock=pBlock->clone();
				switch(pBlock->getType()) {
	case TYPE_WPT:
		if(hCurWnd != hWptLV) {
			errMsg(_T("To paste waypoints at top go to the ")
				_T("waypoint view or select a route"));
			return FALSE;
		}
		waypointList.insertBlockBefore(pBlock,waypointList.first());
		break;
	case TYPE_RTE:
		if(hCurWnd != hRteLV) {
			errMsg(_T("To paste routes at top go to the ")
				_T("route view"));
			return FALSE;
		}
		routeList.insertBlockBefore(pBlock,routeList.first());
		break;
	case TYPE_TRK:
		if(hCurWnd != hTrkLV) {
			errMsg(_T("To paste tracks at top go to the ")
				_T("track view"));
			return FALSE;
		}
		trackList.insertBlockBefore(pBlock,trackList.first());
		break;
				}
				iter--;
			}
		} else {
			// Put things at the top of the selected block's blocklist
			pBlock1=getPasteTarget(selectedCount,homogenous);
			if(!pBlock1) return FALSE;
			switch(pBlock1->getType()) {
	case TYPE_RTE:
		if(dataType != TYPE_WPT) {
			errMsg(_T("Can only paste waypoints into a route"));
			return FALSE;
		}
		blockList=((CRoute *)pBlock1)->getBlockList();
		iter=clipboardList.last();
		while(iter != eol) {
			pBlock=iter;
			pBlock=pBlock->clone();
			blockList->insertBlockBefore(pBlock,blockList->first());
			iter--;
		}
		break;
	case TYPE_TRK:
		if(dataType != TYPE_TPT) {
			errMsg(_T("Can only paste trackpoints into a track"));
			return FALSE;
		}
		blockList=((CTrack *)pBlock1)->getBlockList();
		iter=clipboardList.last();
		while(iter != eol) {
			pBlock=iter;
			pBlock=pBlock->clone();
			blockList->insertBlockBefore(pBlock,blockList->first());
			iter--;
		}
		break;
	case TYPE_WPT:
		errMsg(_T("Selected item must be a route"));
		return FALSE;
	case TYPE_TPT:
		errMsg(_T("Selected item must be a track"));
		return FALSE;
	default:
		errMsg(_T("Selection is not meaningful"));
		return FALSE;
			}
		}
		break;
	case ID_EDIT_PASTE_ATBOTTOM:
		if(!selectedCount) {
			// Put things at the bottom of the relevant list
			if(trackpoints) {
				errMsg(_T("Do not know where to put trackpoints"));
				return FALSE;
			}
			iter=clipboardList.first();
			while(iter != eol) {
				pBlock=iter;
				pBlock=pBlock->clone();
				switch(pBlock->getType()) {
	case TYPE_WPT:
		if(hCurWnd != hWptLV) {
			errMsg(_T("To paste waypoints at bottom go to the ")
				_T("waypoint view or select a route"));
			return FALSE;
		}
		waypointList.insertBlockAfter(pBlock,waypointList.last());
		break;
	case TYPE_RTE:
		if(hCurWnd != hRteLV) {
			errMsg(_T("To paste routes at bottom go to the ")
				_T("route view"));
			return FALSE;
		}
		routeList.insertBlockAfter(pBlock,routeList.last());
		break;
	case TYPE_TRK:
		if(hCurWnd != hTrkLV) {
			errMsg(_T("To paste tracks at bottom go to the ")
				_T("track view"));
			return FALSE;
		}
		trackList.insertBlockAfter(pBlock,trackList.last());
		break;
				}
				iter++;
			}
		} else {
			// Put things at the top of the selected block's blocklist
			pBlock1=getPasteTarget(selectedCount,homogenous);
			if(!pBlock1) return FALSE;
			switch(pBlock1->getType()) {
	case TYPE_RTE:
		if(dataType != TYPE_WPT) {
			errMsg(_T("Can only paste waypoints into a route"));
			return FALSE;
		}
		blockList=((CRoute *)pBlock1)->getBlockList();
		iter=clipboardList.first();
		while(iter != eol) {
			pBlock=iter;
			pBlock=pBlock->clone();
			blockList->insertBlockAfter(pBlock,blockList->last());
			iter++;
		}
		break;
	case TYPE_TRK:
		if(dataType != TYPE_TPT) {
			errMsg(_T("Can only paste trackpoints into a track"));
			return FALSE;
		}
		blockList=((CTrack *)pBlock1)->getBlockList();
		iter=clipboardList.first();
		while(iter != eol) {
			pBlock=iter;
			pBlock=pBlock->clone();
			blockList->insertBlockAfter(pBlock,blockList->last());
			iter++;
		}
		break;
	case TYPE_WPT:
		errMsg(_T("Selected item must be a route"));
		return FALSE;
	case TYPE_TPT:
		errMsg(_T("Selected item must be a track"));
		return FALSE;
	default:
		errMsg(_T("Selection is not meaningful"));
		return FALSE;
			}
		}
		break;
	case ID_EDIT_PASTE_BEFORE:
		pBlock1=getPasteTarget(selectedCount,homogenous);
		if(!pBlock1) return FALSE;
		if(pBlock1->getType() != dataType) {
			errMsg(_T("Clipboard items must be the same type ")
				_T("as the selected item"));
			return FALSE;
		}
		blockList=pBlock1->getParentList();
		iter=clipboardList.first();
		while(iter != eol) {
			pBlock=iter;
			pBlock=pBlock->clone();
			blockList->insertBlockBefore(pBlock,pBlock1);
			iter++;
		}
		break;
	case ID_EDIT_PASTE_ON:
	case ID_EDIT_PASTE_AFTER:
		pBlock1=getPasteTarget(selectedCount,homogenous);
		if(!pBlock1) return FALSE;
		if(pBlock1->getType() != dataType) {
			errMsg(_T("Clipboard items must be the same type ")
				_T("as the selected item"));
			return FALSE;
		}
		blockList=pBlock1->getParentList();
		iter=clipboardList.last();
		while(iter != eol) {
			pBlock=iter;
			pBlock=pBlock->clone();
			blockList->insertBlockAfter(pBlock,pBlock1);
			iter--;
		}
		// Remove the selected block if it is paste on
		if(editType == ID_EDIT_PASTE_ON) {
			blockList->removeBlock(pBlock1,TRUE);
		}
		break;
	}

	// Insure all tracks start with a start trackpoint
	trackListSetStart();

	// Refresh the list view (must be current to paste)
	refreshLV(hCurWnd);

	return TRUE;
}

static CBlock *getPasteTarget(UINT selectedCount, BOOL homogenous)
{
	int index;
	CBlock *pBlock=NULL;

	if(!selectedCount) {
		errMsg(_T("Select a place to paste"));
		return NULL;
	}
	if(selectedCount  != 1) {
		errMsg(_T("Select only one place to paste"));
		return NULL;
	}
	if(!homogenous) {
		errMsg(_T("Clipboard can only contain one type of item")
			_T(" when there is a selection."));
		return NULL;
	}
	// Get the first index
	index=ListView_GetNextItem(hCurWnd,-1,LVNI_SELECTED);
	if(index < 0) {
		errMsg(_T("Error finding place to paste"));
		return NULL;
	}
	// Get the block
	pBlock=lvGetBlockFromItem(hCurWnd,index);
	if(!pBlock) {
		errMsg(_T("Invalid item at place to paste"));
		return NULL;
	}

	return pBlock;
}

BOOL checkSelectedBlocks(BOOL checkState)
{
	UINT selectedCount;
	CBlock *pBlock;
	CBlockList *blockList;
	int index;

	// The current window must be one of the list view windows
	if(!hCurWnd) return FALSE;
	if(hCurWnd != hWptLV  && hCurWnd != hRteLV && 
		hCurWnd != hTrkLV) {
			errMsg(_T("Nothing selected"));
			return FALSE;
	}

	// See if there is anything selected
	selectedCount=ListView_GetSelectedCount(hCurWnd);
#if DEBUG_SELECTED
	lbprintf(_T("remove: clipboard: %d selected: %d"),
		clipboardList.getCount(),selectedCount);
#endif
	if(!selectedCount) {
		errMsg(_T("Nothing selected"));
		return FALSE;
	}

	// Get the first index
	index=ListView_GetNextItem(hCurWnd,-1,LVNI_SELECTED);
	if(index < 0) return FALSE;
#if DEBUG_SELECTED
	pBlock=lvGetBlockFromItem(hCurWnd,index);
	if(pBlock) {
		LPWSTR unicodeString=NULL;

		ansiToUnicode(pBlock->getIdent(),&unicodeString);
		if(unicodeString) {
			lbprintf(_T("First selected: %s"),unicodeString);
			free(unicodeString);
		} else {
			lbprintf(_T("First selected: <Convert error>"));
		}
	} else {
		lbprintf(_T("First selected: NULL"));
	}
#endif

	// Check the selected items
	do {
		pBlock=lvGetBlockFromItem(hCurWnd,index);
		if(pBlock) {
			blockList=pBlock->getParentList();
			pBlock->setChecked(checkState);
		}
		index=ListView_GetNextItem(hCurWnd,index,LVNI_SELECTED);
	} while(index >= 0);

	// Refresh the list view (must be current to have selection)
	refreshLV(hCurWnd);
	return TRUE;
}

BOOL checkBlocks(DataType type, BOOL checkState, BOOL doList, BOOL doParent)
{
	tsDLIterBD<CBlock> iter;
	tsDLIterBD<CBlock> eol;
	CBlock *pBlock;

	// Determine the list
	switch(type) {
	case TYPE_WPT:
		iter=waypointList.first();
		doList=FALSE;
		break;
	case TYPE_RTE:
		iter=routeList.first();
		break;
	case TYPE_TRK:
		iter=trackList.first();
		break;
	default:
		return FALSE;
	}

	// Loop over the list
	while(iter != eol) {
		pBlock=iter;
		if(doParent) {
			pBlock->setChecked(checkState);
		}
		// Loop over the list
		if(doList) {
			CBlockList *blockList;
			if(type == TYPE_RTE) {
				blockList=((CRoute *)pBlock)->getBlockList();
			} else {
				blockList=((CTrack *)pBlock)->getBlockList();
			}
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				pBlock=iter1;
				pBlock->setChecked(checkState);
				iter1++;
			}
		}
		iter++;
	}

	// Redisplay the list view
	switch(type) {
	case TYPE_WPT:
		if(hWptLV) refreshLV(hWptLV);
		break;
	case TYPE_RTE:
		if(hRteLV) refreshLV(hRteLV);
		break;
	case TYPE_TRK:
		if(hTrkLV) refreshLV(hTrkLV);
		break;
	}

	// Redisplay the map
	if(hMap) {
		InvalidateRect(hMap,NULL,FALSE);
		UpdateWindow(hMap);
	}

	return TRUE;
}

BOOL deleteBlocks(DataType type)
{
	CBlockList *list;

	// Determine the list
	switch(type) {
	case TYPE_WPT:
		list=&waypointList;
		break;
	case TYPE_RTE:
		list=&routeList;
		break;
	case TYPE_TRK:
		list=&trackList;
		break;
	default:
		return FALSE;
	}

	// Clear the list and delete the blocks
	while(list->getCount() > 0) {
		CBlock *pLast=list->last();
		list->removeBlock(pLast,TRUE);
	}

	// Redisplay the list view
	switch(type) {
	case TYPE_WPT:
		if(hWptLV) refreshLV(hWptLV);
		break;
	case TYPE_RTE:
		if(hRteLV) refreshLV(hRteLV);
		break;
	case TYPE_TRK:
		if(hTrkLV) refreshLV(hTrkLV);
		break;
	}

	// Redisplay the map
	if(hMap) {
		InvalidateRect(hMap,NULL,FALSE);
		UpdateWindow(hMap);
	}

	return TRUE;
}

void trackListSetStart()
{
	if(!trackList.getCount()) return;

	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
		if(blockList->getCount() > 0) {
			tsDLIterBD<CBlock> iter1(blockList->first());
			CBlock *pBlock1=iter1;
			((CTrackpoint *)pBlock1)->setIdent("S");
		}
		iter++;
	}	
}

void refreshLV(HWND hLV)
{
	void (*createLV)(HWND hWndParent)=NULL;

	if(hLV == hWptLV) {
		createLV=wptCreateLV;
	} else if(hLV == hRteLV) {
		createLV=wptCreateLV;
	} else if(hLV == hTrkLV) {
		createLV=trkCreateLV;
	} else {
		return;
	}

	// Try to keep what is visible still visible.  (There is no
	// SetTopIndex.)  Essentially try to make the old top and bottom
	// visible.  Since we are at the beginning after the create, making
	// the top visible will bring it to the bottom of the page.  Then,
	// making the bottom visible, will scroll it back to the top.
	if(hLV) {
#if 0
		int top=ListView_GetTopIndex(hLV)+ListView_GetCountPerPage(hLV)-1;
		if(createLV) createLV(hWndMain);
		ListView_EnsureVisible(hLV,top,TRUE);
#else
		int top=ListView_GetTopIndex(hLV);
		int countPerPage=ListView_GetCountPerPage(hLV);
		int bottom=ListView_GetTopIndex(hLV)+countPerPage-1;

		if(createLV) createLV(hWndMain);

		int last=ListView_GetItemCount(hLV)-1;
		if(last < 0) last=0;
		if(bottom > last) bottom=last;
		ListView_EnsureVisible(hLV,top,TRUE);
		ListView_EnsureVisible(hLV,bottom,TRUE);
#endif
	}
}
