///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

//*********************************************************//
// dialoghelper.h                                          //
//                                                         //
// CDialogHelper: Class to aid dialog template disassembly //
// Author: Mark McGuill                                    //
// Date Of Birth: 11/04/2002                               //
// Used by: CDlgView And Doc and CRawstructdoc             //
//                                                         //
//                                                         //
//*********************************************************//

#ifndef _DIALOGHELPER_H_
#define _DIALOGHELPER_H_

/////////////////////////////////////////////////////////////////////////////

typedef struct _DLGHELP_TEMPLATE{
	DWORD style;
	DWORD exStyle;
	DWORD helpID;
	WORD items;
	WORD x;
	WORD y;
	WORD cx;
	WORD cy;
}DLGHELP_TEMPLATE, *LPDLGHELP_TEMPLATE;

/////////////////////////////////////////////////////////////////////////////

typedef struct _DLGHELP_ITEM_TEMPLATE{
	DWORD style;
	DWORD exStyle;
	DWORD helpID;
	WORD x;
	WORD y;
	WORD cx;
	WORD cy;
	WORD id;
}DLGHELP_ITEM_TEMPLATE,*LPDLGHELP_ITEM_TEMPLATE;

/////////////////////////////////////////////////////////////////////////////

typedef struct _DLGTEMPLATEEX{
    WORD  dlgVer; 
    WORD  signature; 
    DWORD  helpID; 
    DWORD  exStyle; 
    DWORD  style; 
	WORD   cdit; 
    short  x; 
    short  y; 
    short  cx; 
    short  cy; 
}DLGTEMPLATEEX,*LPDLGTEMPLATEEX;
#define SIZE_OF_DLGTEMPLATEEX 26
#define SIZE_OF_DLGTEMPLATE 18

/////////////////////////////////////////////////////////////////////////////

typedef struct _DLGITEMTEMPLATEEX{
    DWORD  helpID; 
    DWORD  exStyle; 
    DWORD  style; 
    short  x; 
    short  y; 
    short  cx; 
    short  cy; 
    WORD   id; 
}DLGITEMTEMPLATEEX,*LPDLGITEMTEMPLATEEX;
#define SIZE_OF_DLGITEMTEMPLATEEX 22
#define SIZE_OF_DLGITEMTEMPLATE 18

/////////////////////////////////////////////////////////////////////////////

class CDialogHelper
{
// Construction
public:
	CDialogHelper();
	BOOL  CreateChildDialogCopy(CDialog* pAttach, CWnd* pParent);
	BOOL  Init(BYTE* pData, DWORD dwSize);
	virtual ~CDialogHelper();

// Op's
public:
	BOOL  IsExtended(){return m_bEx;}

	// Dialog Accessors
	void GetTemplate(DLGHELP_TEMPLATE* pTemplate);
	WORD GetNumControls();
	void GetRect(CRect rctDlg);
	BOOL GetMenu(WORD* pwMenuOrd, CString& strMenu);
	BOOL GetFont(WORD* pwPoint,WORD* pwWeight,WORD* pwItalic,CString& strFace);
	BOOL GetClass(WORD* pwOrd, CString& strClass);
	BOOL GetTitle(CString& strTitle);

	// Dialog Item Accessors	
	void  GetItemTemplate(int nItem, DLGHELP_ITEM_TEMPLATE* pItemTemplate);
	BOOL  GetItemCreateData(int nItem, BYTE* pBuff);
	WORD  GetItemCreateDataSize(int nItem);
	void  GetItemTitle(int nItem, WORD* pwOrd, CString& strTitle);
	void  GetItemClass(int nItem, WORD* pwOrd, CString& strClass);
	DWORD GetItemHelpID(int nItem);
	WORD  GetItemID(int nItem);
	DWORD GetItemExStyle(int nItem);
	DWORD GetItemStyle(int nItem);
	BOOL  GetItemRect(int nItem, CRect& rctItem);
	

protected:
	// Internal Help
	BYTE* __GetFirstItem();
	BYTE* __GetNextItem(BYTE* pCur);
	void  __ModifyChildControlsCopy(BYTE* pNewTemplate, BYTE* pNewCurControl, BYTE* pOrigCurControl);
	void  __ModifyControlClassCopy(BYTE** ppCur, BYTE** ppOrig, DWORD* pdwItemStyle);
	BYTE* __SkipString(BYTE *pStr);
	BYTE* __Dword_Align(BYTE* pAl);
	BYTE* __Word_Align(BYTE* pAl);

	// Attributes
	BOOL m_bEx;
	BYTE* m_pTemplate;
	DWORD m_dwSize;
};

#endif //_DIALOGHELPER_H_