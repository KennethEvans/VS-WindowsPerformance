// ListView implementation

#include "stdafx.h"

#define DEBUG_BLOCK 0
#define DEBUG_CHECKCONSISTENCY 1
#define ITEM_LEN 40
#define COL_MULT 9
#define RTE_NCOLS 7

#include "GPSLink.h"
#include "resource.h"

// Function ptototypes

// Global variables
static TCHAR rteColLabel[RTE_NCOLS][ITEM_LEN]={
	_T("Name"),_T("Indx"),_T("Latitude"),_T("Longitude"),_T("Alt"),
	_T("Distance"),_T("Symbol")
};
// width of column in pixels    !!!!!!!!!!!!!!!!!!!!!!!
static TCHAR rteColWidth[RTE_NCOLS]={8,4,9,9,6,7,7};

void rteCreateLV(HWND hWndParent)                                     
{
	RECT rect;
	LV_COLUMN lvCol;
	LVITEM lvI;
	int icol,isub,ret;

	if(hRteLV) {
		// Delete all the items
		ListView_DeleteAllItems(hRteLV);
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
		hRteLV=CreateWindowEx(0L,
			WC_LISTVIEW,                // List view class
			TEXT(""),                   // No default text
			WS_VISIBLE|WS_CHILD|WS_BORDER|WS_EX_CLIENTEDGE|	// Styles
			WS_CLIPSIBLINGS|LVS_REPORT|LVS_SHOWSELALWAYS,
			rect.top,rect.left,
			rect.right-rect.left,rect.bottom-rect.top,
			hWndParent,
			(HMENU)ID_RTE_LISTVIEW,
			hInst,
			NULL);

		if(hRteLV == NULL ) return;

		// Set extended styles
		ListView_SetExtendedListViewStyle(hRteLV,
			LVS_EX_FULLROWSELECT|
			LVS_EX_CHECKBOXES|
			LVS_EX_HEADERDRAGDROP);

		// Initialize the LV_COLUMN structure.
		lvCol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
		lvCol.fmt=LVCFMT_LEFT;  // left-align column
		lvCol.pszText=NULL;

		// Add the columns.
		for(icol=0; icol < RTE_NCOLS; icol++) {
			lvCol.iSubItem=icol;
			lvCol.pszText=rteColLabel[icol];
			lvCol.cx=rteColWidth[icol]*COL_MULT;
			ret=ListView_InsertColumn(hRteLV,icol,&lvCol);
			if(ret == -1) {
				errMsg(_T("Could not insert column %d"),icol);
				return;
			}
		}
	}

	// Get the item count and set indices and distance
	showRouteWaypointsOld=showRouteWaypoints;
	{
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		int count=0;
		int rteIndex=1;
		while(iter != eol) {
			int index=1;
			double rteDistance=0.0;
			double rteAltitude=0.0;
			CBlock *pBlock=iter;
			CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			count++;
			if(showRouteWaypoints) count+=blockList->getCount();
			// Do the waypoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			CBlock *pBlock0=NULL;
			while(iter1 != eol) {
				double distance=0.0;
				CBlock *pBlock1=iter1;
				double altitude=pBlock1->getAltitude();
				if(rteAltitude < altitude) rteAltitude=altitude;
				pBlock1->setIndex(index++);
				if(pBlock0) {
					// Distance is zero for the first waypoint
					// Calculate distance from block0 after the first
					distance=getDistance(pBlock1,pBlock0);
				}
				rteDistance+=distance;
#if 1
				pBlock1->setDistance(distance);
#else
				pBlock1->setDistance(rteDistance);
#endif
				pBlock0=pBlock1;
				++iter1;
			}
			// Route distance is the total
			pBlock->setIndex(rteIndex++);
			pBlock->setDistance(rteDistance);
			((CRoute *)pBlock)->setAltitude(rteAltitude);
			++iter;
		}
		ListView_SetItemCount(hRteLV,count);
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
	tsDLIterBD<CBlock> iter(routeList.first());
	tsDLIterBD<CBlock> eol;

	int index=0;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
		lvI.iItem=index;
		lvI.iSubItem=0;
		lvI.pszText=LPSTR_TEXTCALLBACK;
		lvI.cchTextMax=ITEM_LEN;
		lvI.lParam=(LPARAM)pBlock;
		BOOL checked=pBlock->isChecked();

		ret=ListView_InsertItem(hRteLV,&lvI);
#if DEBUG_BLOCK
		lbprintf(_T("InsertItem %d: lParam=%x"),index,lvI.lParam);
#endif
		if(ret == -1) {
			errMsg(_T("Error inserting route %d"),index);
			return;
		}
		// Set if it is checked or not.  Use the original value since
		// the listview will have overwritten it
		ListView_SetCheckState(hRteLV,index,checked);

		// Insert the subitems as callbacks
		for(isub=1; isub < RTE_NCOLS; isub++) {
			ListView_SetItemText(hRteLV,index,isub,LPSTR_TEXTCALLBACK);
		}
		index++;

		// Do the waypoints
		if(showRouteWaypoints) {
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				lvI.iItem=index;
				lvI.iSubItem=0;
				lvI.pszText=LPSTR_TEXTCALLBACK;
				lvI.cchTextMax=ITEM_LEN;
				lvI.lParam=(LPARAM)pBlock1;
				BOOL checked1=pBlock1->isChecked();

				ret=ListView_InsertItem(hRteLV,&lvI);
#if DEBUG_BLOCK
				lbprintf(_T("InsertItem %d: lParam=%x"),index,lvI.lParam);
#endif
				if(ret == -1) {
					errMsg(_T("Error inserting route waypoint %d"),index);
					return;
				}
				// Set if it is checked or not.  Use the original value since
				// the listview will have overwritten it
				ListView_SetCheckState(hRteLV,index,checked1);
				// Insert the subitems as callbacks
				for(isub=1; isub < RTE_NCOLS; isub++) {
					ListView_SetItemText(hRteLV,index,isub,LPSTR_TEXTCALLBACK);
				}
				index++;
				++iter1;
			}
		}
		++iter;
	}

	// Update the window 
	UpdateWindow(hRteLV);
}

LRESULT rteNotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
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

	if(wParam != ID_RTE_LISTVIEW) return 0L;

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
			lbprintf(_T("rteNotifyHandler: code=%d"),hdr->code);
	}
#endif

	// Put the branch on route or waypoint inside each case, because
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
				pBlock=lvGetBlockFromItem(hRteLV,pNmia->iItem);
				if(!pBlock) {
					lbprintf(_T("Could not get block for list view item"));
					lbprintf(_T("  iItem=%d iSubItem=%d"),
						pNmia->iItem,pNmia->iSubItem);
					break;
				}

				// Set checked
				pBlock->setChecked(ListView_GetCheckState(hRteLV,pNmia->iItem));
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
		ScreenToClient(hRteLV,&point);
		pBlock=lvGetBlockFromPoint(hRteLV,point);
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
		pBlock=lvGetBlockFromItem(hRteLV,pNmia->iItem);
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
		if(pBlock->getType() == TYPE_RTE) {
			CRoute *pRoute=(CRoute *)pBlock;
			if(!pRoute) return 0L;
			// Route
			switch(pLvdi->item.iSubItem) {
	case 0:     // Name
		unicodeString=NULL;
		ansiToUnicode(pRoute->getIdent(),&unicodeString);
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
		_stprintf(szText,_T("%d"),pRoute->getIndex());
		pLvdi->item.pszText=szText;
		break;
	case 4:     // Altitude
		_stprintf(szText,_T("%4.0f"),pRoute->getAltitude());
		pLvdi->item.pszText=szText;
		break;
	case 5:     // Distance
		_stprintf(szText,_T("%7.2f"),pRoute->getDistance());
		pLvdi->item.pszText=szText;
		break;
	case 2:     // Latitude
	case 3:     // Longitude
	case 6:     // Symbol
		pLvdi->item.pszText=_T("");
		break;
	default:
		break;
			}
		} else if(pBlock->getType() == TYPE_WPT) {
			CWaypoint *pWaypoint=(CWaypoint *)pBlock;
			// Waypoint
			switch(pLvdi->item.iSubItem) {
	case 0:     // Name
		unicodeString=NULL;
		ansiToUnicode(pWaypoint->getIdent(),&unicodeString);
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
		_stprintf(szText,_T("%d"),pWaypoint->getIndex());
		pLvdi->item.pszText=szText;
		break;
	case 2:     // Latitude
		latitude=pWaypoint->getLatitude();
		_stprintf(szText,_T("%9.6f"),latitude);
		pLvdi->item.pszText=szText;
		break;
	case 3:     // Longitude
		longitude=pWaypoint->getLongitude();
		_stprintf(szText,_T("%9.6f"),longitude);
		pLvdi->item.pszText=szText;
		break;
	case 4:     // Altitude
		_stprintf(szText,_T("%4.0f"),pWaypoint->getAltitude());
		pLvdi->item.pszText=szText;
		break;
	case 5:     // Distance
		_stprintf(szText,_T("%7.2f"),pWaypoint->getDistance());
		pLvdi->item.pszText=szText;
		break;
	case 6:     // Symbol
		unicodeString=NULL;
		ansiToUnicode(getSymbolName(pWaypoint->getSymbol()),
			&unicodeString);
		if(unicodeString) {
			_tcsncpy(szText,unicodeString,ITEM_LEN);
			szText[ITEM_LEN-1]='\0';
			free(unicodeString);
		} else {
			_tcscpy(szText,_T("<Error>"));
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
			if(ansiString) {
				pBlock->setIdent(ansiString);
				free(ansiString);
			} else {
				pBlock->setIdent(ansiString);
			}
			break;
		}
	default:
		break;
	}

	return 0L;
}

// Set the selections into the blocks (Could do this by iterating over
// the listview also)
void rteGetSelections(void)
{
	tsDLIterBD<CBlock> iter(routeList.first());
	tsDLIterBD<CBlock> eol;
	BOOL checkState;

	if(!hRteLV) return;

	int index=0;
	while(iter != eol) {
		CBlock *pBlock=iter;
		checkState=ListView_GetCheckState(hRteLV,index);
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
		// Do the waypoints
		CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
		tsDLIterBD<CBlock> iter1(blockList->first());
		while(iter1 != eol) {
			pBlock=iter1;
			checkState=ListView_GetCheckState(hRteLV,index);
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
