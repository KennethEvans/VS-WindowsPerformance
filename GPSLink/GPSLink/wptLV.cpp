// ListView implementation

#include "stdafx.h"

#define DEBUG_BLOCK 0
#define DEBUG_SELECT 0
#define DEBUG_CHECKCONSISTENCY 0

#define ITEM_LEN 40
#define COL_MULT 9
#define WPT_NCOLS 5

#include "GPSLink.h"
#include "resource.h"

// Function ptototypes
static int CALLBACK wptLVCompareProc(LPARAM lParam1, LPARAM lParam2,
									 LPARAM lParamSort);


// Global variables
static TCHAR wptColLabel[WPT_NCOLS][ITEM_LEN]={
	_T("Name"),_T("Latitude"),_T("Longitude"),_T("Alt"),_T("Symbol")
};
// width of column in pixels    !!!!!!!!!!!!!!!!!!!!!!!
static TCHAR wptColWidth[WPT_NCOLS]={15,9,9,5,7};
static BOOL wptReverse;

void wptCreateLV(HWND hWndParent)                                     
{
	RECT rect;
	LV_COLUMN lvCol;
	LVITEM lvI;
	int icol,isub,ret;

	if(hWptLV) {
		// Delete all the items
		ListView_DeleteAllItems(hWptLV);
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
		hWptLV=CreateWindowEx(0L,
			WC_LISTVIEW,                // List view class
			TEXT(""),                   // No default text
			WS_VISIBLE|WS_CHILD|WS_BORDER|WS_EX_CLIENTEDGE|	// Styles
			WS_CLIPSIBLINGS|LVS_REPORT|LVS_EDITLABELS|
			LVS_SHOWSELALWAYS,
			rect.top,rect.left,
			rect.right-rect.left,rect.bottom-rect.top,
			hWndParent,
			(HMENU)ID_WPT_LISTVIEW,
			hInst,
			NULL);

		if(hWptLV == NULL ) return;

		// Set extended styles
		ListView_SetExtendedListViewStyle(hWptLV,
			LVS_EX_FULLROWSELECT|
			LVS_EX_CHECKBOXES|
			LVS_EX_HEADERDRAGDROP);

		// Initialize the LV_COLUMN structure.
		lvCol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
		lvCol.fmt=LVCFMT_LEFT;  // left-align column
		lvCol.pszText=NULL;

		// Add the columns.
		for(icol=0; icol < WPT_NCOLS; icol++) {
			lvCol.iSubItem=icol;
			lvCol.pszText=wptColLabel[icol];
			lvCol.cx=wptColWidth[icol]*COL_MULT;
			ret=ListView_InsertColumn(hWptLV,icol,&lvCol);
			if(ret == -1) {
				errMsg(_T("Could not insert column %d"),icol);
				return;
			}
		}
	}

	// Set the item count
	ListView_SetItemCount(hWptLV,waypointList.getCount());

	// Add the items
	// The mask specifies the the pszText, iImage, lParam and state
	// members of the LV_ITEM structure are valid.
	lvI.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
	lvI.state=0;
	lvI.stateMask=(UINT)(-1);  // Modify all bits
	tsDLIterBD<CBlock> iter(waypointList.first());
	tsDLIterBD<CBlock> eol;

	int index=0;
	while(iter != eol) {
		CBlock *pBlock=iter;
		lvI.iItem=index;
		lvI.iSubItem=0;
		lvI.pszText=LPSTR_TEXTCALLBACK;
		lvI.cchTextMax=ITEM_LEN;
		lvI.lParam=(LPARAM)pBlock;
		BOOL checked=pBlock->isChecked();

		ret=ListView_InsertItem(hWptLV,&lvI);
		if(ret == -1) {
			errMsg(_T("Error inserting waypoint %d"),index);
			return;
		}
		// Set if it is checked or not.  Use the original value since
		// the listview will have overwritten it
		ListView_SetCheckState(hWptLV,index,checked);
		// Insert the subitems as callbacks
		for(isub=1; isub < WPT_NCOLS; isub++) {
			ListView_SetItemText(hWptLV,index,isub,LPSTR_TEXTCALLBACK);
		}
		index++;
		++iter;
	}

	// Initialize the sort order
	wptReverse=FALSE;

	// Update the window 
	UpdateWindow(hWptLV);
}

LRESULT notifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case ID_WPT_LISTVIEW:
		return wptNotifyHandler(hWnd,uMsg,wParam,lParam);
	case ID_RTE_LISTVIEW:
		return rteNotifyHandler(hWnd,uMsg,wParam,lParam);
	case ID_TRK_LISTVIEW:
		return trkNotifyHandler(hWnd,uMsg,wParam,lParam);
	default:
		return 0L;
	}
}

LRESULT wptNotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
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
	double latitude,longitude,height;
	static TCHAR szText[ITEM_LEN];
	TCHAR *unicodeString;
	POINT menuPoint,point;

	if(wParam != ID_WPT_LISTVIEW) return 0L;

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
			lbprintf(_T("wptNotifyHandler: code=%d"),hdr->code);
	}
#endif

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
				pBlock=lvGetBlockFromItem(hWptLV,pNmia->iItem);
				if(!pBlock) {
					lbprintf(_T("Could not get block for list view item"));
					lbprintf(_T("  iItem=%d iSubItem=%d"),
						pNmia->iItem,pNmia->iSubItem);
					break;
				}

				// Set checked
				pBlock->setChecked(ListView_GetCheckState(hWptLV,pNmia->iItem));
#if DEBUG_SELECT
				{
					LPWSTR unicodeString=NULL;

					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
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
		ScreenToClient(hWptLV,&point);
#if 0
		CheckMenuItem(hLvPopupMenu,ID_MAPOPTIONS_SHOWCOMPASS,
			showCompass?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hLvPopupMenu,ID_MAPOPTIONS_TRACKPOINTS,
			drawTrackpoints?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hLvPopupMenu,ID_MAPOPTIONS_CALIBRATIONPOINTS,
			showCalibrationPoints?MF_CHECKED:MF_UNCHECKED);
#endif
		pBlock=lvGetBlockFromPoint(hWptLV,point);
		if(!pBlock) {
			lbprintf(_T("Could not get block for tap hold"));
			break;
		}
#if DEBUG_BLOCK
		message=_T("GN_CONTEXTMENU");
		pNmia=(NMLISTVIEW *)lParam;
		lbprintf(_T("  x=%ld y=%ld"),
			pNmrg->ptAction.x,pNmrg->ptAction.y);
#endif
		pMenuBlock=pBlock;
		TrackPopupMenu(hLvPopupMenu,
			TPM_LEFTALIGN|TPM_VCENTERALIGN,
			point.x,point.y,0,hWnd,NULL);
		break;
#else
	case NM_RCLICK:
		pNmia=(NMLISTVIEW *)lParam;
#if DEBUG_BLOCK
		message=_T("NM_RCLICK");
		pNmia=(NMLISTVIEW *)lParam;
		lbprintf(_T(" %s: iItem=%d iSubItem=%d"),
			message,pNmia->iItem,pNmia->iSubItem);
		lbprintf(_T("  x=%ld y=%ld"),
			pNmia->ptAction.x,pNmia->ptAction.y);
#endif
		menuPoint=pNmia->ptAction;
		point=menuPoint;
		ClientToScreen(hWnd,&point);
		pBlock=lvGetBlockFromItem(hWptLV,pNmia->iItem);
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
#if DEBUG_BLOCK
	case NM_CLICK:
		pNmia=(NMLISTVIEW *)lParam;
#if DEBUG_BLOCK
		message=_T("NM_CLICK");
		pNmia=(NMLISTVIEW *)lParam;
		lbprintf(_T(" %s: iItem=%d iSubItem=%d"),
			message,pNmia->iItem,pNmia->iSubItem);
		lbprintf(_T("  x=%ld y=%ld"),
			pNmia->ptAction.x,pNmia->ptAction.y);
#endif
		menuPoint=pNmia->ptAction;
		break;
#endif
#if DEBUG_BLOCK
	case NM_DBLCLK:
		pNmia=(NMLISTVIEW *)lParam;
#if DEBUG_BLOCK
		message=_T("NM_DBLCLK");
		pNmia=(NMLISTVIEW *)lParam;
		lbprintf(_T(" %s: iItem=%d iSubItem=%d"),
			message,pNmia->iItem,pNmia->iSubItem);
		lbprintf(_T("  x=%ld y=%ld"),
			pNmia->ptAction.x,pNmia->ptAction.y);
#endif
		menuPoint=pNmia->ptAction;
		break;
#endif
	case LVN_GETDISPINFO:
		pLvdi=(LV_DISPINFO *)lParam;
		pBlock=(CBlock *)(pLvdi->item.lParam);
		if(pBlock->getType() == TYPE_WPT) {
			CWaypoint *pWaypoint=(CWaypoint *)(pLvdi->item.lParam);
			if(!pWaypoint) return 0L;
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
	case 1:     // Latitude
		latitude=pWaypoint->getLatitude();
		_stprintf(szText,_T("%9.6f"),latitude);
		pLvdi->item.pszText=szText;
		break;
	case 2:     // Longitude
		longitude=pWaypoint->getLongitude();
		_stprintf(szText,_T("%9.6f"),longitude);
		pLvdi->item.pszText=szText;
		break;
	case 3:     // Altitude
		height=pWaypoint->getAltitude();
		_stprintf(szText,_T("%4.0f"),height);
		pLvdi->item.pszText=szText;
		break;
	case 4:     // Symbol
		unicodeString=NULL;
		ansiToUnicode(getSymbolName(pWaypoint->getSymbol()),&unicodeString);
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
	case LVN_COLUMNCLICK:
		pNmia=(NMLISTVIEW *)lParam;
		// The user clicked a column header - sort by this criterion.
		ListView_SortItems(pNm->hdr.hwndFrom,wptLVCompareProc,
			(LPARAM)(pNm->iSubItem));
		// Reverse the order for next time
		wptReverse=!wptReverse;
		break;
	default:
		break;
	}

	return 0L;
}

static int CALLBACK wptLVCompareProc(LPARAM lParam1, LPARAM lParam2,
									 LPARAM lParamSort)
{
	CWaypoint *pWaypoint1=(CWaypoint *)lParam1;
	CWaypoint *pWaypoint2=(CWaypoint *)lParam2;
	LPSTR lpStr1, lpStr2;
	int iResult;

	if(pWaypoint1 && pWaypoint2) {
		switch(lParamSort) {
	case 0:     // Name
		lpStr1=pWaypoint1->getIdent();
		lpStr2=pWaypoint2->getIdent();
		// WCE does not support strcmpi, stricmp
		// lstrcmpi is supported
		iResult=strcmp(lpStr1,lpStr2);
		break;
	case 1:     // Latitude
		if(pWaypoint1->getLatitude() > pWaypoint2->getLatitude())
			iResult=1;
		else if(pWaypoint1->getLatitude() < pWaypoint2->getLatitude())
			iResult=-1;
		else iResult=0;
		break;
	case 2:     // Longitude
		if(pWaypoint1->getLongitude() > pWaypoint2->getLongitude())
			iResult=1;
		else if(pWaypoint1->getLongitude() < pWaypoint2->getLongitude())
			iResult=-1;
		else iResult=0;
		break;
	case 3:     // Altitude
		if(pWaypoint1->getAltitude() > pWaypoint2->getAltitude())
			iResult=1;
		else if(pWaypoint1->getAltitude() < pWaypoint2->getAltitude())
			iResult=-1;
		else iResult=0;
		break;
	case 4:     // Symbol
		lpStr1=getSymbolName(pWaypoint1->getSymbol());
		lpStr2=getSymbolName(pWaypoint2->getSymbol());
		// WCE does not support strcmpi, stricmp
		// lstrcmpi is supported
		iResult=strcmp(lpStr1,lpStr2);
		break;
	default:
		iResult=0;
		break;
		}

	}

	if(wptReverse) iResult=-iResult;
	return(iResult);
}

// Find the block corresponding to the index
CBlock *lvGetBlockFromItem(HWND hLV, int iItem)
{
	LVITEM lvI;
	BOOL status;
	CBlock *pBlock;

	// Fill in the index and mask and use 0 for the subindex
	lvI.iItem=iItem;
	lvI.iSubItem=0;
	lvI.mask=LVIF_PARAM;
	// Get the LVITEM filled in
	status=ListView_GetItem(hLV,&lvI);
	if(!status) {
		return(NULL);
	}
	// The user data is the lParam
	pBlock=(CBlock *)(lvI.lParam);
	if(!pBlock) {
		return(NULL);
	}

	return(pBlock);
}

// Find the block corresponding to the hit point
CBlock *lvGetBlockFromPoint(HWND hLV, POINT point)
{
	LVHITTESTINFO htI;
	LVITEM lvI;
	BOOL status;
	CBlock *pBlock;
	int index;

	// Get the index of the item
	htI.pt=point;
	index=ListView_HitTest(hLV,&htI);
	if(index == -1) return NULL;

	// Get the lparam of the item
	lvI.iItem=index;
	lvI.iSubItem=0;
	lvI.mask=LVIF_PARAM;
	// Get the LVITEM filled in
	status=ListView_GetItem(hLV,&lvI);
	if(!status) {
		return(NULL);
	}
	// The user data is the lParam
	pBlock=(CBlock *)(lvI.lParam);
	if(!pBlock) {
		return(NULL);
	}

	return(pBlock);
}

// Find the hit flag corresponding to the POINT
UINT lvGetHitFlagFromPoint(HWND hLV, POINT point)
{
	LVHITTESTINFO htI;
	int index;

	// Fill in the POINT
	htI.pt=point;
	index=ListView_SubItemHitTest(hLV,&htI);

	if(index == -1) return 0;
	else return(htI.flags);
}

// Set the selections into the blocks (Could do this by iterating over
// the listview also)
void wptGetSelections(void)
{
	tsDLIterBD<CBlock> iter(waypointList.first());
	tsDLIterBD<CBlock> eol;
	BOOL checkState;

	if(!hWptLV) return;

	int index=0;
	while(iter != eol) {
		CBlock *pBlock=iter;
		checkState=ListView_GetCheckState(hWptLV,index);
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
		++iter;
	}
}
