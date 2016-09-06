// TODO: Complete review This File!

//*********************************************************//
// dialoghelper.cpp                                        //
//                                                         //
// CDialogHelper: Class to aid dialog template disassembly //
// Author: Mark McGuill                                    //
// Date Of Birth: 11/04/2002                               //
// Used by: CDlgView And Doc and CRawstructdoc             //
//                                                         //
//                                                         //
//*********************************************************//

///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "Studio.h"
#include "dialoghelper.h"

///////////////////////////////////////////////////////////////////////////////

#define MARGIN_FOR_EDIT 0xFF // Extra space in memory to play with template

///////////////////////////////////////////////////////////////////////////////

CDialogHelper::CDialogHelper()
{
	m_bEx = FALSE;
	m_pTemplate = NULL;
	m_dwSize = 0;
}

///////////////////////////////////////////////////////////////////////////////

CDialogHelper::~CDialogHelper()
{
	if(m_pTemplate) delete[] m_pTemplate;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDialogHelper::Init(BYTE* pData, DWORD dwSize)
{
	m_pTemplate= new unsigned char[dwSize];
	if(!m_pTemplate) return FALSE;

	CopyMemory(m_pTemplate,pData,dwSize);
	m_dwSize = dwSize;

	m_bEx = ((*(DWORD*)m_pTemplate) == 0xFFFF0001);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDialogHelper::CreateChildDialogCopy(CDialog* pAttach, CWnd* pParent)
{
	if(!m_dwSize) return FALSE;

	BYTE* pNew = new unsigned char[m_dwSize + MARGIN_FOR_EDIT]; // Create good bit of space
	if(!pNew) return FALSE;
	FillMemory(pNew,m_dwSize + MARGIN_FOR_EDIT,0); //Zero all
	BYTE* pCur = pNew;

	if(m_bEx){
		CopyMemory(pCur,m_pTemplate,SIZE_OF_DLGTEMPLATEEX);
		pCur+=SIZE_OF_DLGTEMPLATEEX;
	}
	else{
		CopyMemory(pCur,m_pTemplate,SIZE_OF_DLGTEMPLATE);
		pCur+=SIZE_OF_DLGTEMPLATE;
	}

	// Menu
	*(WORD*)pCur = 0x0000; //No Menu
	pCur+=2;

	//Class
	*(WORD*)pCur = 0x0000; //No Class
	pCur+=2;

	// Title
	BYTE* pOrig = (BYTE*)(m_pTemplate + (m_bEx ? SIZE_OF_DLGTEMPLATEEX : SIZE_OF_DLGTEMPLATE));

	if(*(WORD*)pOrig != 0xFFFF && *(WORD*)pOrig != 0x0000){
		pOrig = __SkipString(pOrig);// => Menu
	}
	else if(*(WORD*)pOrig == 0xFFFF){
		pOrig+=4; //skip 0xFFFF and Ordinal => Menu
	}
	else{
		pOrig+=2; //Skip 0x0000
	}

	if(*(WORD*)pOrig != 0xFFFF && *(WORD*)pOrig != 0x0000){
		pOrig = __SkipString(pOrig);// => Menu
	}
	else if(*(WORD*)pOrig == 0xFFFF){
		pOrig+=4; //skip 0xFFFF and Ordinal => Menu
	}
	else{
		pOrig+=2; //Skip 0x0000
	}

	// Copy Original title
	int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pOrig,-1,NULL,0,NULL,NULL);	
	CopyMemory(pCur,pOrig,(nCh*2));
	pCur += (nCh*2);
	pOrig += (nCh*2);

	// Font ??
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(pDlg->style & DS_SETFONT){
			// Copy point,weight and italic 6 bytes, pTitle now points to orig font size etc
			CopyMemory(pCur,pOrig,6);
			pCur+=6;
			pOrig+=6;

			// Now Font face name
			nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pOrig,-1,NULL,0,NULL,NULL);	
			CopyMemory(pCur,pOrig,(nCh*2));
			pCur+=(nCh*2);
			pOrig+=(nCh*2);
		}
	}
	else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(pDlg->style & DS_SETFONT){
			// Copy point 2 bytes
			*(WORD*)pCur = *(WORD*)pOrig;
			pCur+=2;
			pOrig+=2;

			// Now Font face name
			nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pOrig,-1,NULL,0,NULL,NULL);	
			CopyMemory(pCur,pOrig,(nCh*2));
			pCur+=(nCh*2);
			pOrig+=(nCh*2);
		}
	}
	
	// the children, Wont somebody please think of the children!!
	pCur = __Dword_Align(pCur);
	pOrig = __Dword_Align(pOrig);
	__ModifyChildControlsCopy(pNew,pCur,(BYTE*)pOrig);
	
	// Change to child
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)pNew;
		if(!(pDlg->style & WS_CHILD)){
			pDlg->style &= (~(WS_POPUP));
			pDlg->style |= WS_CHILD;
		}
	}
	else{
		LPDLGTEMPLATE pDlg = (LPDLGTEMPLATE)pNew;
		if(!(pDlg->style & WS_CHILD)){
			pDlg->style &= (~(WS_POPUP));
			pDlg->style |= WS_CHILD;
		}
	}

	// Create
	if(!pAttach->CreateIndirect(pNew,pParent)){
		delete[] pNew;
		return FALSE;
	}

	delete[] pNew;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::__ModifyChildControlsCopy(BYTE *pNewTemplate, BYTE *pNewCurControl, BYTE *pOrigCurControl)
{
	DLGTEMPLATEEX* pDlgEx = (DLGTEMPLATEEX*)pNewTemplate;
	DLGTEMPLATE* pDlg = (DLGTEMPLATE*)pNewTemplate;
	int nCtrlCount = (m_bEx ? pDlgEx->cdit : pDlg->cdit);

	for(int x=0;x<nCtrlCount;x++){
		if(m_bEx)	CopyMemory(pNewCurControl,pOrigCurControl,SIZE_OF_DLGITEMTEMPLATEEX);
		else		CopyMemory(pNewCurControl,pOrigCurControl,SIZE_OF_DLGITEMTEMPLATE);

		//Make visible
		DWORD* pdwItemStyle = (DWORD*)(m_bEx ? (pNewCurControl + 8) : pNewCurControl); // +8 to move past helpId & exStyle
		*pdwItemStyle |= WS_VISIBLE;

		// Move to Class
		pNewCurControl+= (m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);
		pOrigCurControl+=(m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);
		if(m_bEx){
			pNewCurControl = __Dword_Align(pNewCurControl);
			pOrigCurControl = __Dword_Align(pOrigCurControl);
		}
		else{
			pNewCurControl = __Word_Align(pNewCurControl);
			pOrigCurControl = __Word_Align(pOrigCurControl);
		}

		__ModifyControlClassCopy(&pNewCurControl,&pOrigCurControl,pdwItemStyle);
		

		//Title ==> Copy
		if(*(WORD*)pOrigCurControl != 0xFFFF){
			int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pOrigCurControl,-1,NULL,0,NULL,NULL);	
			CopyMemory(pNewCurControl,pOrigCurControl,(nCh*2));
			pNewCurControl+=(nCh*2);
			pOrigCurControl+=(nCh*2);
		}
		else{
			*(DWORD*)pNewCurControl = *(DWORD*)pOrigCurControl; // Copy 0xFFFF and title ord
			pNewCurControl+=4; //Past 0xFFFF and Ord
			pOrigCurControl+=4;
		}

		// Creation Data - not sure bout this
		if(*(WORD*)pOrigCurControl != 0x0000){ // There is creation Data
			int ncount = *(WORD*)pOrigCurControl;
			*(WORD*)pNewCurControl = *(WORD*)pOrigCurControl;
			pNewCurControl+=2;pOrigCurControl+=2;

			//Copy
			CopyMemory(pNewCurControl,pOrigCurControl,ncount);
			pNewCurControl+=ncount;
			pOrigCurControl+=ncount;
		}
		else{
			*(WORD*)pNewCurControl = 0x0000;
			pNewCurControl+=2; //Leave zeroed
			pOrigCurControl+=2;
		}

		pNewCurControl = __Dword_Align(pNewCurControl);
		pOrigCurControl= __Dword_Align(pOrigCurControl);
	}
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::__ModifyControlClassCopy(BYTE** ppCur, BYTE** ppOrig, DWORD* pdwItemStyle)
{
	// Load
	BYTE* pOrig = *ppOrig;
	BYTE* pCur = *ppCur;
	
	if(*(WORD*)pOrig != 0xFFFF)
	{ //String
		TCHAR szClass[256];

		#ifdef UNICODE
		lstrcpyn(szClass,(LPCWSTR)pOrig,256);
		#else
		WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pOrig,-1,szClass,256,NULL,NULL);
		#endif

		int nCh = _tcslen(szClass) + 1;
		
		// Does class exist??
		WNDCLASSEX wndClass;
		if(!GetClassInfoEx(NULL,szClass,&wndClass))
		{ //No
			*(WORD*)pCur = 0xFFFF;
			pCur+=2;
			*(WORD*)pCur = 0x0082; // Static
			*pdwItemStyle = WS_CHILD| WS_VISIBLE | SS_GRAYRECT | SS_LEFT; // easily visible
			pCur+=2;
			pOrig+=(nCh*2); //word pointer original past original class
		}
		else
		{ //yes class exists
			//Copy into
			CopyMemory(pCur,pOrig,(nCh*2));
			pCur+=(nCh*2); // Ok Class => skip class
			pOrig+=(nCh*2); // original past original class
		}
	}
	else{
		*(DWORD*)pCur = *(DWORD*)pOrig; // Copy 0xFFFF and class ord
		pCur+=2;
		// Remove ownerdraw from buttons
		if(*(WORD*)pCur == 0x0080){
			if((*pdwItemStyle & BS_OWNERDRAW) == BS_OWNERDRAW) 
				*pdwItemStyle &= (~BS_OWNERDRAW);
		}
		if(*(WORD*)pCur == 0x0082){
			if((*pdwItemStyle & SS_ICON) == SS_ICON){
				*pdwItemStyle &= (~SS_ICON);
				*pdwItemStyle |= SS_BLACKFRAME;
			}
			if((*pdwItemStyle & SS_BITMAP) == SS_BITMAP){
				*pdwItemStyle &= (~SS_BITMAP);
				*pdwItemStyle |= SS_BLACKFRAME;
			}
		}
		pCur+=2; //Past 0xFFFF and Ord of system class
		pOrig+=4;
	}

	// Restore
	*ppOrig = pOrig;
	*ppCur = pCur;
}

///////////////////////////////////////////////////////////////////////////////

BYTE* CDialogHelper::__SkipString(BYTE *pStr)
{
	int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pStr,-1,NULL,0,NULL,NULL);	
	pStr += (nCh*2);
	return pStr;
}

///////////////////////////////////////////////////////////////////////////////

BYTE* CDialogHelper::__Dword_Align(BYTE* pAl)
{
	if((DWORD_PTR)pAl % 4){
		int nPad = 4 - (DWORD)((DWORD_PTR)pAl%4);
		for(int x =0;x<nPad;x++,pAl++);
	}
	return pAl;
}

///////////////////////////////////////////////////////////////////////////////

BYTE* CDialogHelper::__Word_Align(BYTE* pAl)
{
	if((DWORD_PTR)pAl % 2) pAl++;
	return pAl;
}

///////////////////////////////////////////////////////////////////////////////

BYTE* CDialogHelper::__GetFirstItem()
{
	WORD* pTmp = (WORD*)(m_pTemplate + (m_bEx ? SIZE_OF_DLGTEMPLATEEX : SIZE_OF_DLGTEMPLATE));
	// Menu
	if(*pTmp == 0xFFFF){
		pTmp+=2;
	}
	else if(*pTmp != 0x0000){
		pTmp = (WORD*)__SkipString((BYTE*)pTmp);
	}
	else{
		pTmp++;
	}

	// Class
	if(*pTmp == 0xFFFF){
		pTmp+=2;
	}
	else if(*pTmp != 0x0000){
		pTmp = (WORD*)__SkipString((BYTE*)pTmp);
	}
	else{
		pTmp++;
	}

	// Title
	if(*pTmp != 0x0000){
		pTmp = (WORD*)__SkipString((BYTE*)pTmp);
	}
	else pTmp++;

	// Font ??
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(pDlg->style & DS_SETFONT){
			pTmp+=3;
			pTmp += WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pTmp,-1,NULL,0,NULL,NULL);	
		}
	}
	else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(pDlg->style & DS_SETFONT){
			pTmp++;
			pTmp += WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pTmp,-1,NULL,0,NULL,NULL);	
		}
	}
	
	pTmp = (WORD*)__Dword_Align((BYTE*)pTmp);
	return (BYTE*)pTmp;
}

///////////////////////////////////////////////////////////////////////////////

BYTE* CDialogHelper::__GetNextItem(BYTE* pCur)
{
	pCur += (m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);

	if(m_bEx)	pCur = __Dword_Align(pCur);
	else		pCur = __Word_Align(pCur);

	// Class
	if(*(WORD*)pCur == 0xFFFF){
		pCur+=4;
	}
	else if(*(WORD*)pCur != 0x0000){
		pCur = __SkipString(pCur);
	}
	else{
		pCur+=2;
	}

	//Title ==> Copy
	if(*(WORD*)pCur != 0xFFFF){
		int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pCur,-1,NULL,0,NULL,NULL);
		pCur += (nCh*2);
	}
	else pCur += 4; //Past 0xFFFF and Ord

	// Creation Data - not sure bout this
	if(*(WORD*)pCur != 0x0000){ // There is creation Data
		int ncount = *(WORD*)pCur;
		pCur += 2;
		pCur += ncount;
	}
	else pCur+=2; 
	pCur = __Dword_Align(pCur);
	return pCur;
}

//////////////////////////////////////////////////////////////

// Accessors

BOOL CDialogHelper::GetMenu(WORD* pwMenuOrd, CString& strMenu)
{
	if(!m_dwSize) return FALSE;

	WORD* pRaw = (WORD*)(m_pTemplate + (m_bEx ? SIZE_OF_DLGTEMPLATEEX : SIZE_OF_DLGTEMPLATE));
	
	*pwMenuOrd = 0;
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF){ // Next word specifies ordinal of menu in resource
			pRaw++;
			*pwMenuOrd = (*(pRaw));
		}
		else{ // Its a Unicode string specifying menu resource!!
			char szMenu[MAX_PATH];
			int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pRaw,-1,szMenu,MAX_PATH,NULL,NULL);	
			szMenu[nCh] = 0;
			strMenu = szMenu;
		}
	}
	else return FALSE;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDialogHelper::GetClass(WORD *pwOrd, CString &strClass)
{
	if(!m_dwSize) return FALSE;

	WORD* pRaw = (WORD*)(m_pTemplate + (m_bEx ? SIZE_OF_DLGTEMPLATEEX : SIZE_OF_DLGTEMPLATE));
	*pwOrd = 0;

	//Skip Menu
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF)	pRaw+=2;
		else				pRaw = (WORD*)__SkipString((BYTE*)pRaw);
	}
	else pRaw++;
	
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF){ // Next word specifies ordinal of menu in resource
			pRaw++;
			*pwOrd = (*(pRaw));
		}
		else{ // Its a Unicode string specifying menu resource!!
			char szMenu[MAX_PATH];
			int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pRaw,-1,szMenu,MAX_PATH,NULL,NULL);	
			szMenu[nCh] = 0;
			strClass = szMenu;
		}
	}else return FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDialogHelper::GetTitle(CString &strTitle)
{
	if(!m_dwSize) return FALSE;

	WORD* pRaw = (WORD*)(m_pTemplate + (m_bEx ? SIZE_OF_DLGTEMPLATEEX : SIZE_OF_DLGTEMPLATE));
	
	//Skip Menu
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF)	pRaw+=2;
		else				pRaw = (WORD*)__SkipString((BYTE*)pRaw);
	}
	else pRaw++;

	//Skip Class
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF)	pRaw+=2;
		else				pRaw = (WORD*)__SkipString((BYTE*)pRaw);
	}
	else pRaw++;

	if(*pRaw != 0x0000){
		// Its a Unicode string specifying menu resource!!
		char szMenu[MAX_PATH];
		int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pRaw,-1,szMenu,MAX_PATH,NULL,NULL);	
		szMenu[nCh] = 0;
		strTitle = szMenu;
	}else return FALSE;
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDialogHelper::GetFont(WORD *pwPoint, WORD *pwWeight, WORD *pwItalic, CString &strFace)
{
	if(!m_dwSize) return FALSE;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(!(pDlg->style & DS_SETFONT)) return FALSE;
	}
	else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(!(pDlg->style & DS_SETFONT)) return FALSE;
	}

	//////////////////////////////////////////////////
	WORD* pRaw = (WORD*)(m_pTemplate + (m_bEx ? SIZE_OF_DLGTEMPLATEEX : SIZE_OF_DLGTEMPLATE));
	
	//Skip Menu
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF)	pRaw+=2;
		else				pRaw = (WORD*)__SkipString((BYTE*)pRaw);
	}
	else pRaw++;

	//Skip Class
	if(*pRaw != 0x0000){
		if(*pRaw == 0xFFFF)	pRaw+=2;
		else				pRaw = (WORD*)__SkipString((BYTE*)pRaw);
	}
	else pRaw++;

	//Skip Title
	if(*pRaw != 0x0000){
		pRaw = (WORD*)__SkipString((BYTE*)pRaw);
	}
	else pRaw++;

	*pwPoint = *pRaw;
	pRaw++;

	*pwWeight = (m_bEx ? (*pRaw) : 0);
	pRaw += (m_bEx ? 1:0);
	*pwItalic = (m_bEx ? (*pRaw & 0x00FF) : 0);
	pRaw +=(m_bEx ? 1:0);

	char szMenu[MAX_PATH];
	int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pRaw,-1,szMenu,MAX_PATH,NULL,NULL);	
	szMenu[nCh] = 0;
	strFace = szMenu;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::GetRect(CRect rctDlg)
{
	if(!m_dwSize) return;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		rctDlg.SetRect(pDlg->x,pDlg->y,pDlg->x + pDlg->cx,pDlg->y + pDlg->cy);
	}
	else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		rctDlg.SetRect(pDlg->x,pDlg->y,pDlg->x + pDlg->cx,pDlg->y + pDlg->cy);
	}
}

///////////////////////////////////////////////////////////////////////////////

WORD CDialogHelper::GetNumControls()
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		return pDlg->cdit;
	}
	else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		return pDlg->cdit;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::GetTemplate(DLGHELP_TEMPLATE *pTemplate)
{
	if(!m_dwSize) return;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		pTemplate->cx =			pDlg->cx;
		pTemplate->cy =			pDlg->cy;
		pTemplate->x =			pDlg->x;
		pTemplate->y=			pDlg->y;
		pTemplate->exStyle =	pDlg->exStyle;
		pTemplate->helpID =		pDlg->helpID;
		pTemplate->style =		pDlg->style;
		pTemplate->items =		pDlg->cdit;
	}
	else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		pTemplate->cx =			pDlg->cx;
		pTemplate->cy =			pDlg->cy;
		pTemplate->x =			pDlg->x;
		pTemplate->y=			pDlg->y;
		pTemplate->exStyle =	pDlg->dwExtendedStyle;
		pTemplate->helpID =		0; // N/A
		pTemplate->style =		pDlg->style;
		pTemplate->items =		pDlg->cdit;
	}
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Item Accessors

BOOL CDialogHelper::GetItemRect(int nItem, CRect &rctItem)
{
	if(!m_dwSize) return FALSE;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return FALSE;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return FALSE;
	}

	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	DLGITEMTEMPLATE* pItem = (DLGITEMTEMPLATE*)pTmp;
	DLGITEMTEMPLATEEX* pItemEx = (DLGITEMTEMPLATEEX*)pTmp;

	if(m_bEx)	rctItem.SetRect(pItemEx->x,pItemEx->y,pItemEx->x + pItemEx->cx,pItemEx->y + pItemEx->cy);
	else		rctItem.SetRect(pItem->x,pItem->y,pItem->x + pItem->cx,pItem->y + pItem->cy);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

DWORD CDialogHelper::GetItemStyle(int nItem)
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	if(m_bEx) return ((DLGITEMTEMPLATEEX*)pTmp)->style;
	else return ((DLGITEMTEMPLATE*)pTmp)->style;
}

///////////////////////////////////////////////////////////////////////////////

DWORD CDialogHelper::GetItemExStyle(int nItem)
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}	
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	if(m_bEx) return ((DLGITEMTEMPLATEEX*)pTmp)->exStyle;
	else return ((DLGITEMTEMPLATE*)pTmp)->dwExtendedStyle;
}

///////////////////////////////////////////////////////////////////////////////

WORD CDialogHelper::GetItemID(int nItem)
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}	
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	if(m_bEx) return ((DLGITEMTEMPLATEEX*)pTmp)->id;
	else return ((DLGITEMTEMPLATE*)pTmp)->id;
}

///////////////////////////////////////////////////////////////////////////////

DWORD CDialogHelper::GetItemHelpID(int nItem)
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}	
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	if(m_bEx) return ((DLGITEMTEMPLATEEX*)pTmp)->helpID;
	else return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::GetItemClass(int nItem, WORD *pwOrd, CString &strClass)
{
	if(!m_dwSize) return;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return;
	}

	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	// Move to Class
	pTmp+= (m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);
	if(m_bEx)	pTmp = __Dword_Align(pTmp);
	else		pTmp = __Word_Align(pTmp);

	if(*(WORD*)pTmp == 0xFFFF){
		*pwOrd = *(WORD*)(pTmp+2);
	}else{
		*pwOrd = 0;
		char szTemp[MAX_PATH];
		int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pTmp,-1,szTemp,MAX_PATH,NULL,NULL);	
		szTemp[nCh] = 0;
		strClass = szTemp;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::GetItemTitle(int nItem, WORD *pwOrd, CString &strTitle)
{
	if(!m_dwSize) return;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return;
	}
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	// Move to Class
	pTmp+= (m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);
	if(m_bEx)	pTmp = __Dword_Align(pTmp);
	else		pTmp = __Word_Align(pTmp);

	if(*(WORD*)pTmp == 0xFFFF){
		pTmp+=4;
	}else{
		pTmp = __SkipString(pTmp);
	}

	// Title
	if(*(WORD*)pTmp == 0xFFFF){
		*pwOrd = *(WORD*)(pTmp+2);
	}else{
		*pwOrd = 0;
		char szTemp[MAX_PATH];
		int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pTmp,-1,szTemp,MAX_PATH,NULL,NULL);	
		szTemp[nCh] = 0;
		strTitle = szTemp;
	}
}

///////////////////////////////////////////////////////////////////////////////

WORD CDialogHelper::GetItemCreateDataSize(int nItem)
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return 0;
	}	
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	// Move past Class
	pTmp+= (m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);
	if(m_bEx)	pTmp = __Dword_Align(pTmp);
	else		pTmp = __Word_Align(pTmp);

	if(*(WORD*)pTmp == 0xFFFF){
		pTmp+=4;
	}else{
		pTmp = __SkipString(pTmp);
	}
	// Move past Title
	if(*(WORD*)pTmp == 0xFFFF){
		pTmp+=4;
	}else{
		pTmp = __SkipString(pTmp);
	}
	return *(WORD*)pTmp;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDialogHelper::GetItemCreateData(int nItem, BYTE *pBuff)
{
	if(!m_dwSize) return 0;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return FALSE;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return FALSE;
	}	
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);
	
	// Move past Class
	pTmp+= (m_bEx ? SIZE_OF_DLGITEMTEMPLATEEX : SIZE_OF_DLGITEMTEMPLATE);
	if(m_bEx)	pTmp = __Dword_Align(pTmp);
	else		pTmp = __Word_Align(pTmp);

	if(*(WORD*)pTmp == 0xFFFF){
		pTmp+=4;
	}else{
		pTmp = __SkipString(pTmp);
	}
	// Move past Title
	if(*(WORD*)pTmp == 0xFFFF){
		pTmp+=4;
	}else{
		pTmp = __SkipString(pTmp);
	}

	// Data
	if(*(WORD*)pTmp != 0x0000){// Data
		WORD wSize = *(WORD*)pTmp;
		CopyMemory(pBuff,pTmp+2,wSize);
	}
	else return FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CDialogHelper::GetItemTemplate(int nItem, DLGHELP_ITEM_TEMPLATE *pItemTemplate)
{
	if(!m_dwSize) return;
	if(m_bEx){
		DLGTEMPLATEEX* pDlg = (DLGTEMPLATEEX*)m_pTemplate;
		if(nItem > pDlg->cdit) return;
	}else{
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)m_pTemplate;
		if(nItem > pDlg->cdit) return;
	}
	BYTE* pTmp = __GetFirstItem();
	for(int x=0;x<nItem;x++) pTmp = __GetNextItem(pTmp);

	if(m_bEx){
		DLGITEMTEMPLATEEX* pDlg = (DLGITEMTEMPLATEEX*)pTmp;
		pItemTemplate->cx =		pDlg->cx;
		pItemTemplate->cy =		pDlg->cy;
		pItemTemplate->x =		pDlg->x;
		pItemTemplate->y =		pDlg->y;
		pItemTemplate->style=	pDlg->style;
		pItemTemplate->exStyle=	pDlg->exStyle;
		pItemTemplate->id=		pDlg->id;
		pItemTemplate->helpID=	pDlg->helpID;
	}
	else{
		DLGITEMTEMPLATE* pDlg = (DLGITEMTEMPLATE*)pTmp;
		pItemTemplate->cx =		pDlg->cx;
		pItemTemplate->cy =		pDlg->cy;
		pItemTemplate->x =		pDlg->x;
		pItemTemplate->y =		pDlg->y;
		pItemTemplate->style=	pDlg->style;
		pItemTemplate->exStyle=	0;
		pItemTemplate->id=		pDlg->id;
		pItemTemplate->helpID=	0;
	}
}

///////////////////////////////////////////////////////////////////////////////////