// ListView implementation

#include "stdafx.h"

#define DEBUG_BLOCK 0
#define DEBUG_CHECKCONSISTENCY 1

// Use incremental or acucumulated distance for trackpoints
#define USE_INCREMENTAL_DISTANCE 0
#define INCLUDE_CONTINUE_TO_START_DISTANCE 0

#define ITEM_LEN 40
#define COL_MULT 9
#define TRK_NCOLS 7

#include "GPSLink.h"
#include "resource.h"

// Function ptototypes

// Global variables
static TCHAR trkColLabel[TRK_NCOLS][ITEM_LEN]={
	_T("Name"),_T("Indx"),_T("Latitude"),_T("Longitude"),_T("Alt"),
	_T("Distance"),_T("Time")
};
// width of column in pixels    !!!!!!!!!!!!!!!!!!!!!!!
static TCHAR trkColWidth[TRK_NCOLS]={12,4,9,9,6,9,17};

void trkCreateLV(HWND hWndParent)                                     
{
	RECT rect;
	LV_COLUMN lvCol;
	LVITEM lvI;
	int icol,isub,ret;

	if(hTrkLV) {
		// Delete all the items
		ListView_DeleteAllItems(hTrkLV);
	} else {
		// Create the control
		// Ensure that the common control DLL is loaded.
#if 1    
		INITCOMMONCONTROLSEX init;
		init.dwSize=sizeof(INITCOMMONCONTROLSEX);
		init.dwICC=ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&init);
#else
		InitCommonControls();
#endif

		// Get the size and position of the parent window.
		GetClientRect(hWndParent,&rect);

		// Create the list view window that starts out in details view
		// and supports label editing.
		hTrkLV=CreateWindowEx(0L,
			WC_LISTVIEW,                // List view class
			TEXT(""),                   // No default text
			WS_VISIBLE|WS_CHILD|WS_BORDER|WS_EX_CLIENTEDGE|	// Styles
			WS_CLIPSIBLINGS|LVS_REPORT|LVS_SHOWSELALWAYS,
			rect.top,rect.left,
			rect.right-rect.left,rect.bottom-rect.top,
			hWndParent,
			(HMENU)ID_TRK_LISTVIEW,
			hInst,
			NULL);

		if(hTrkLV == NULL ) return;

		// Set extended styles
		ListView_SetExtendedListViewStyle(hTrkLV,
			LVS_EX_FULLROWSELECT|
			LVS_EX_CHECKBOXES|
			LVS_EX_HEADERDRAGDROP);

		// Initialize the LV_COLUMN structure.
		lvCol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
		lvCol.fmt=LVCFMT_LEFT;  // left-align column
		lvCol.pszText=NULL;

		// Add the columns.
		for(icol=0; icol < TRK_NCOLS; icol++) {
			lvCol.iSubItem=icol;
			lvCol.pszText=trkColLabel[icol];
			lvCol.cx=trkColWidth[icol]*COL_MULT;
			ret=ListView_InsertColumn(hTrkLV,icol,&lvCol);
			if(ret == -1) {
				errMsg(_T("Could not insert column %d"),icol);
				return;
			}
		}
	}

	// Set the column order
	if(showTrackpoints) {
		int colOrder[TRK_NCOLS]={0,1,2,3,4,5,6};
		ListView_SetColumnOrderArray(hTrkLV,TRK_NCOLS,colOrder);
	} else {
		int colOrder[TRK_NCOLS]={0,1,5,6,4,2,3};
		ListView_SetColumnOrderArray(hTrkLV,TRK_NCOLS,colOrder);
	}

	// Get the item count and set indices and distance
	showTrackpointsOld=showTrackpoints;
	{
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		int count=0;
		int trkIndex=1;
		while(iter != eol) {
			int index=1;
			double trkDistance=0.0;
			double trkAltitude=0.0;
			CBlock *pBlock=iter;
			CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			count++;
			if(showTrackpoints) count+=blockList->getCount();
			// Do the trackpoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			CBlock *pBlock0=NULL;
			while(iter1 != eol) {
				double distance=0.0;
				CBlock *pBlock1=iter1;
				double altitude=pBlock1->getAltitude();
				if(trkAltitude < altitude) trkAltitude=altitude;
				pBlock1->setIndex(index++);
				if(pBlock0) {
					// Distance is zero for the first trackpoint
					// Calculate distance from block0 after the first
					distance=getDistance(pBlock1,pBlock0);
				}
#if INCLUDE_CONTINUE_TO_START_DISTANCE
				// Include the distance between every trackpoint
				trkDistance+=distance;
#else
				// Don't include the distance before a Start point
				CTrackpoint *pTrackpoint=(CTrackpoint *)pBlock1;
				if(!pTrackpoint->isNewTrack()) {
					trkDistance+=distance;
				}
#endif
#if USE_INCREMENTAL_DISTANCE
				pBlock1->setDistance(distance);
#else
				pBlock1->setDistance(trkDistance);
#endif
				pBlock0=pBlock1;
				++iter1;
			}
			// Track distance is the total
			pBlock->setIndex(trkIndex++);
			pBlock->setDistance(trkDistance);
			((CTrack *)pBlock)->setAltitude(trkAltitude);
			++iter;
		}
		ListView_SetItemCount(hTrkLV,count);
#if DEBUG_BLOCK
		lbprintf(_T("ItemCount=%d"),count);
#endif
	}

	// Add the items
	// The mask specifies the the pszText, iImage, lParam and state
	// members of the LV_ITEM structure are valid.
	lvI.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
	lvI.state=0;
	lvI.stateMask=(UINT)(-1);  // Modify all bits
	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;

	int index=0;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
		lvI.iItem=index;
		lvI.iSubItem=0;
		lvI.pszText=LPSTR_TEXTCALLBACK;
		lvI.cchTextMax=ITEM_LEN;
		lvI.lParam=(LPARAM)pBlock;
		BOOL checked=pBlock->isChecked();

		ret=ListView_InsertItem(hTrkLV,&lvI);
#if DEBUG_BLOCK
		lbprintf(_T("InsertItem %d: lParam=%x"),index,lvI.lParam);
#endif
		if(ret == -1) {
			errMsg(_T("Error inserting track %d"),index);
			return;
		}
		// Set if it is checked or not.  Use the original value since
		// the listview will have overwritten it
		ListView_SetCheckState(hTrkLV,index,checked);
		// Insert the subitems as callbacks
		for(isub=1; isub < TRK_NCOLS; isub++) {
			ListView_SetItemText(hTrkLV,index,isub,LPSTR_TEXTCALLBACK);
		}
		index++;

		// Do the trackpoints
		if(showTrackpoints) {
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				lvI.iItem=index;
				lvI.iSubItem=0;
				lvI.pszText=LPSTR_TEXTCALLBACK;
				lvI.cchTextMax=ITEM_LEN;
				lvI.lParam=(LPARAM)pBlock1;
				BOOL checked1=pBlock1->isChecked();

				ret=ListView_InsertItem(hTrkLV,&lvI);
#if DEBUG_BLOCK
				lbprintf(_T("InsertItem %d: lParam=%x"),index,lvI.lParam);
#endif
				if(ret == -1) {
					errMsg(_T("Error inserting trackpoint %d"),index);
					return;
				}
				// Set if it is checked or not.  Use the original value since
				// the listview will have overwritten it
				ListView_SetCheckState(hTrkLV,index,checked1);
				// Insert the subitems as callbacks
				for(isub=1; isub < TRK_NCOLS; isub++) {
					ListView_SetItemText(hTrkLV,index,isub,LPSTR_TEXTCALLBACK);
				}
				index++;
				++iter1;
			}
		}
		++iter;
	}

	// Update the window 
	UpdateWindow(hTrkLV);
}

LRESULT trkNotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
						 LPARAM lParam)
{
	// Presumably all lParms will point to something with a NMHDR
	// pointer as the first part
	NMHDR *hdr=(NMHDR *)lParam;
	// NMLVDISPINFO is LV_DISPINFO in WCE
	LV_DISPINFO *pLvdi;
	// NMLISTVIEW is replaced by NMITEMACTIVATE in WIN32 for WIN_IE >=
	// 0x400.  The latter has an extra uKeyFlags member.  (See
	// commctrl.h for WIN32.)  NMITEMACTIVATE is not mentioned in
	// commctrl.h for WCE.  NMLISTVIEW apparently works both places.
	NMLISTVIEW *pNmia;
	NM_LISTVIEW *pNm=(NM_LISTVIEW *)lParam;
#ifdef UNDER_CE
	NMRGINFO *pNmrg;
#endif
	CBlock *pBlock;
	double latitude,longitude;
	static TCHAR szText[ITEM_LEN];
	TCHAR *unicodeString;
	POINT menuPoint,point;

	if(wParam != ID_TRK_LISTVIEW) return 0L;

#if DEBUG_BLOCK
	// NM_ message are generic and go negative from NM_FIRST=0
	// LVN_ messages go negative from LVN_FIRST=100u
	TCHAR *message;
	// lParam may be a LV_DISPINFO, or NMLISTVIEW pointer.  Both have a
	// hdr.  Use pLvdi here.
	if(hdr->code != LVN_GETDISPINFO &&
		hdr->code != LVN_ITEMCHANGING &&
		hdr->code != LVN_ITEMCHANGED &&
		hdr->code != LVN_INSERTITEM) {
			lbprintf(_T("trkNotifyHandler: code=%d"),hdr->code);
	}
#endif

	// Put the branch on track or trackpoint inside each case, because
	// there are WM_NOTIFY that do not have an lParam that is a CBlock
	// Block->getType() will reference inappropriate memory
	switch(hdr->code) {
	case LVN_ITEMCHANGED:
		pNmia=(NMLISTVIEW *)lParam;
#if DEBUG_BLOCK && 0
		lbprintf(_T(" LVN_ITEMCHANGED: iItem=%d iSubItem=%d"),
			message,pNmia->iItem,pNmia->iSubItem);
#endif
		// Check if it was the state image that changed
		if((pNmia->uChanged&LVIF_STATE) &&
			(pNmia->uNewState&LVIS_STATEIMAGEMASK) != 
			(pNmia->uOldState&LVIS_STATEIMAGEMASK)) {
				// Find the block corresponding to the index
				pBlock=lvGetBlockFromItem(hTrkLV,pNmia->iItem);
				if(!pBlock) {
					lbprintf(_T("Could not get block for list view item"));
					lbprintf(_T("  iItem=%d iSubItem=%d"),
						pNmia->iItem,pNmia->iSubItem);
					break;
				}

				// Set checked
				pBlock->setChecked(ListView_GetCheckState(hTrkLV,pNmia->iItem));
#if DEBUG_SELECT
				{
					LPWSTR unicodeString=NULL;

					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(sunicodeString) {
						lbprintf(_T("%s: checked=%s"),unicodeString,
							pBlock->isChecked()?_T("True"):_T("False"));
						free(unicodeString);
					} else {
						lbprintf(_T("<Unknown>: checked=%s"),
							pBlock->isChecked()?_T("True"):_T("False"));
					}
				}
#endif
		}
		break;
#ifdef UNDER_CE
	case GN_CONTEXTMENU:
		pNmrg=(NMRGINFO *)lParam;
		point=pNmrg->ptAction;
		menuPoint=point;
		ScreenToClient(hTrkLV,&point);
		pBlock=lvGetBlockFromPoint(hTrkLV,point);
		if(!pBlock) {
			lbprintf(_T("Could not get block for tap hold"));
			break;
		}
		pMenuBlock=pBlock;
		TrackPopupMenu(hLvPopupMenu,
			TPM_LEFTALIGN|TPM_VCENTERALIGN,
			point.x,point.y,0,hWnd,NULL);
		break;
#else
	case NM_RCLICK:
		pNmia=(NMLISTVIEW *)lParam;
		menuPoint=pNmia->ptAction;
		point=menuPoint;
		ClientToScreen(hWnd,&point);
		pBlock=lvGetBlockFromItem(hTrkLV,pNmia->iItem);
		if(!pBlock) {
			lbprintf(_T("Could not get block for list view item"));
			lbprintf(_T("  iItem=%d iSubItem=%d"),
				pNmia->iItem,pNmia->iSubItem);
			break;
		}
		pMenuBlock=pBlock;
		TrackPopupMenu(hLvPopupMenu,
			TPM_LEFTALIGN|TPM_VCENTERALIGN,
			point.x,point.y,0,hWnd,NULL);
		break;
#endif
	case LVN_GETDISPINFO:
		pLvdi=(LV_DISPINFO *)lParam;
		pBlock=(CBlock *)(pLvdi->item.lParam);
		if(pBlock->getType() == TYPE_TRK) {
			CTrack *pTrack=(CTrack *)pBlock;
			// Track
			switch(pLvdi->item.iSubItem) {
	case 0:     // Name
		unicodeString=NULL;
		ansiToUnicode(pTrack->getIdent(),&unicodeString);
		if(unicodeString) {
			_tcsncpy(szText,unicodeString,ITEM_LEN);
			szText[ITEM_LEN-1]='\0';
			free(unicodeString);
		} else {
			_tcscpy(szText,_T("<Error>"));
		} 
		pLvdi->item.pszText=szText;
		break;
	case 1:     // Index
		_stprintf(szText,_T("%d"),pTrack->getIndex());
		pLvdi->item.pszText=szText;
		break;
	case 4:     // Altitude
		_stprintf(szText,_T("%4.0f"),pTrack->getAltitude());
		pLvdi->item.pszText=szText;
		break;
	case 5:     // Distance
		_stprintf(szText,_T("%7.2f"),pTrack->getDistance());
		pLvdi->item.pszText=szText;
		break;
	case 2:     // Latitude
	case 3:     // Longitude
		pLvdi->item.pszText=_T("");
		break;
	case 6:     // Time
		{
			CBlockList *blockList=pTrack->getBlockList();
			if(blockList->getCount() > 0) {
				CTrackpoint *pFirst=(CTrackpoint *)(blockList->first());
				CTrackpoint *pLast=(CTrackpoint *)(blockList->last());
				long timeFirst=pFirst->getGTime();
				long timeLast=pLast->getGTime();
				if(timeFirst > 0 && timeLast > 0) {
					printElapsedTime(szText,timeLast-timeFirst);
				} else {
					_tcscpy(szText,_T("GarminTime0"));
				}
				pLvdi->item.pszText=szText;
			} else {
				pLvdi->item.pszText=_T("");
			}
			break;
		}
	default:
		break;
			}
		} else if(pBlock->getType() == TYPE_TPT) {
			CTrackpoint *pTrackpoint=(CTrackpoint *)pBlock;
			// Trackpoint
			switch(pLvdi->item.iSubItem) {
	case 0:     // Name
		if(pTrackpoint->isNewTrack()) {
			pLvdi->item.pszText=_T("Start");
		} else {
			pLvdi->item.pszText=_T("Continue");
		}
		break;
	case 1:     // Index
		_stprintf(szText,_T("%d"),pTrackpoint->getIndex());
		pLvdi->item.pszText=szText;
		break;
	case 2:     // Latitude
		latitude=pTrackpoint->getLatitude();
		_stprintf(szText,_T("%9.6f"),latitude);
		pLvdi->item.pszText=szText;
		break;
	case 3:     // Longitude
		longitude=pTrackpoint->getLongitude();
		_stprintf(szText,_T("%9.6f"),longitude);
		pLvdi->item.pszText=szText;
		break;
	case 4:     // Altitude
		_stprintf(szText,_T("%4.0f"),pTrackpoint->getAltitude());
		pLvdi->item.pszText=szText;
		break;
	case 5:     // Distance
		_stprintf(szText,_T("%7.2f"),pTrackpoint->getDistance());
		pLvdi->item.pszText=szText;
		break;
	case 6:     // Time
		if(pTrackpoint->getGTime() <= 0) {
			_tcscpy(szText,_T("GarminTime0"));
		} else {
			printGarminTime(szText,pTrackpoint->getGTime(),Format_AMPM);
		}
		pLvdi->item.pszText=szText;
		break;
	default:
		break;
			}
		}
		break;
	case LVN_BEGINLABELEDIT:
		{
			HWND hWndEdit;

			// Get the handle to the edit box.
			hWndEdit=(HWND)SendMessage(hWnd,LVM_GETEDITCONTROL,0,0);
			// Limit the amount of text that can be entered
			SendMessage(hWndEdit, EM_SETLIMITTEXT, (WPARAM)NAMELIMIT, 0);
			break;
		}
	case LVN_ENDLABELEDIT:
		{
			pLvdi=(LV_DISPINFO *)lParam;
			pBlock=(CBlock *)(pLvdi->item.lParam);
			char *ansiString=NULL;
			DWORD status;

			// Check
			if(pLvdi->item.iItem == -1 || pLvdi->item.pszText == NULL) {
				break;
			}

			// Convert to ANSI
			status=unicodeToAnsi(pLvdi->item.pszText,&ansiString);
			pBlock->setIdent(ansiString);
			if(ansiString) free(ansiString);
			// Made it refresh since the distance may have changed
			if(hTrkLV) refreshLV(hTrkLV);
			break;
		}
	default:
		break;
	}

	return 0L;
}

// Set the selections into the blocks (Could do this by iterating over
// the listview also)
void trkGetSelections(void)
{
	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	BOOL checkState;

	if(!hTrkLV) return;

	int index=0;
	while(iter != eol) {
		CBlock *pBlock=iter;
		checkState=ListView_GetCheckState(hTrkLV,index);
#if DEBUG_CHECKCONSISTENCY
		if(checkState != pBlock->isChecked()) {
			LPWSTR unicodeString=NULL;

			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T("!%s: sel=%s chk=%s"),unicodeString,
					pBlock->isChecked()?_T("True"):_T("False"),
					checkState?_T("True"):_T("False"));
				free(unicodeString);
			} else {
				lbprintf(_T("<Unknown>: checked=%s"),
					pBlock->isChecked()?_T("True"):_T("False"));
			}
		}
#endif	
		pBlock->setChecked(checkState);
		index++;
		// Do the trackpoints
		CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
		tsDLIterBD<CBlock> iter1(blockList->first());
		while(iter1 != eol) {
			pBlock=iter1;
			checkState=ListView_GetCheckState(hTrkLV,index);
#if DEBUG_CHECKCONSISTENCY
			if(checkState != pBlock->isChecked()) {
				LPWSTR unicodeString=NULL;

				ansiToUnicode(pBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					lbprintf(_T("!%s: sel=%s chk=%s"),unicodeString,
						pBlock->isChecked()?_T("True"):_T("False"),
						checkState?_T("True"):_T("False"));
					free(unicodeString);
				} else {
					lbprintf(_T("<Unknown>: checked=%s"),
						pBlock->isChecked()?_T("True"):_T("False"));
				}
			}
#endif	
			pBlock->setChecked(checkState);
			index++;
			++iter1;
		}
		++iter;
	}
}

void trkConvertS(void)
{
	CTrackpoint *pTrackpoint=(CTrackpoint *)pMenuBlock;
	if(pTrackpoint->isNewTrack()) {
		pTrackpoint->setIdent("Continue");
	}

	if(hTrkLV) refreshLV(hTrkLV);
}

void trkConvertAllS(int type)
{
	CBlock *pBlockSel=NULL;
	CBlockList *pTrackBlockList=NULL;
	CTrack *pTrack=NULL;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	// Get the track corresponding to the trackpoint
	if(type == TYPE_TPT) {
		pTrack=(CTrack *)getTrackFromTrackpoint(pMenuBlock);
		if(!pTrack) {
			errMsg(_T("Cannot associate track with selected trackpoint"));
			goto FINISH;
		}
	} else if(type == TYPE_TRK) {
		pTrack=(CTrack *)pMenuBlock;
	} else {
		errMsg(_T("Invalid type"));
		goto FINISH;
	}
	pTrackBlockList=pTrack->getBlockList();

	// Loop over the blocklist
	{
		tsDLIterBD<CBlock> iter1(pTrackBlockList->first());
		tsDLIterBD<CBlock> eol;
		while(iter1 != eol) {
			CBlock *pBlock=iter1;
			CTrackpoint *pTrackpoint=(CTrackpoint *)pBlock;
			if(pTrackpoint->isNewTrack()) {
				pTrackpoint->setIdent("Continue");
			}
			++iter1;
		}
	}

FINISH:
	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;

	if(hTrkLV) refreshLV(hTrkLV);
}

void trkNextS(Direction direction)
{
	CBlock *pBlockSel=NULL;
	CBlockList *pTrackBlockList0=NULL;
	int sel=-1,index=0,count=0,count0=0,countCheck=0;
	BOOL found=FALSE,trackpointFound=FALSE;
	int index1=-1,index2=-1,inc=1;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	// Get the track corresponding to the trackpoint
	CTrack *pTrack0=(CTrack *)getTrackFromTrackpoint(pMenuBlock);
	if(!pTrack0) {
		errMsg(_T("Cannot associate track with selected trackpoint"));
		goto FINISH;
	}
	pTrackBlockList0=pTrack0->getBlockList();
	count0=pTrackBlockList0->getCount();
	// Get the item count
	count=ListView_GetItemCount(hTrkLV);
	if(count <= 0) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		goto FINISH;
	}

	// Find the indices of this track
	index1=index2=-1;
	for(index=0; index < count; index++) {
		CBlock *pBlock=lvGetBlockFromItem(hTrkLV,index);
		if(pBlock->getType() == TYPE_TRK) {
			// Is a track
			CTrack *pTrack=(CTrack *)pBlock;
			if(index1 >= 0) {
				// Track has been found, this is the next track
				index2=index;
				break;
			}
			if(pTrack == pTrack0) {
				// This is the track for the trackpoint
				index1=index;
			}
		}
	}
	// See if it was found
	if(index1 < 0) {
		errMsg(_T("Cannot find track for selected trackpoint"));
		goto FINISH;
	}
	// See if the next track was found
	if(index2 < 0) index2=count;
	// Check there are tracks
	countCheck=index2-index1-1;
	if(countCheck <= 0 || countCheck != count0) {
		errMsg(_T("Index error for selected track\n")
			_T("check=%d index1=%d index2=%d count0=%d"),
			countCheck,index1,index2,count0);
		goto FINISH;
	}

	// Switch the indices if searching back
	if(direction == FORWARD) {
		index1++;
		inc=1;
	} else {
		int temp=index1;
		index1=index2-1;
		index2=temp;
		inc=-1;
	}

	// Loop over the trackpoints in this track
	found=trackpointFound=FALSE;
	for(index=index1; index != index2; index=index+inc) {
		CBlock *pBlock=lvGetBlockFromItem(hTrkLV,index);
		if(pBlock->getType() != TYPE_TPT) {
			errMsg(_T("Expected trackpoint, found track"));
			goto FINISH;
		}
		CTrackpoint *pTrackpoint=(CTrackpoint *)pBlock;
		if(trackpointFound) {
			if(pTrackpoint->isNewTrack()) {
				found=TRUE;
				sel=index;
				pBlockSel=pBlock;
				break;
			}
		}
		if(pTrackpoint == pMenuBlock) {
			// This is the selected trackpoint;
			trackpointFound=TRUE;
		}
	}
	if(!found) {
		infoMsg(_T("No Start trackpoint found"));
		goto FINISH;
	}

	// Unselect everything
	ListView_SetItemState(hTrkLV,-1,0x0000,
		LVIS_SELECTED|LVIS_FOCUSED|LVIS_CUT|LVIS_DROPHILITED);

	// Set the the item to be selected and make it visible
	ListView_SetItemState(hTrkLV,
		sel,LVIS_SELECTED|LVIS_FOCUSED,
		LVIS_SELECTED|LVIS_FOCUSED);
	ListView_EnsureVisible(hTrkLV,sel,FALSE);

FINISH:
	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;

	viewWindow(hTrkLV);
}
