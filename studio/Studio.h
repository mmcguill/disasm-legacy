///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "..\..\inc\ia32dsm.h"
#include "..\common\lists.h"
#include "..\..\inc\dasmeng.h"

#include "..\..\inc\peres.h"

///////////////////////////////////////////////////////////////////////////////

// TODO: Get Rid Of These Defines to somewhere maintainable

#define WM_UPDATEWORKSPACE			WM_USER+1
#define WM_UPDATE_DOCKVIEW			WM_USER+2
#define WM_DOCWANTDATA				WM_USER+3
#define WM_DOCKVIEW_DOCWANTDATA		WM_USER+4

#define ID_VIEW_RESOURCE_STANDARD       107
#define ID_VIEW_RESOURCE_HEX            109
#define ID_WS_TAB                       101
#define IDW_WORKSPACE                   102

///////////////////////////////////////////////////////////////////////////////

//CPERes* __AppGetPEFile();

///////////////////////////////////////////////////////////////////////////////

typedef enum _StudioMajorDocType
{
	StudioMajorTypeWorkspace,
	StudioMajorTypeDasm,
	StudioMajorTypeResource,
}StudioMajorDocType;

///////////////////////////////////////////////////////////////////////////////

typedef enum _StudioMinorDocType 
{
	StudioDocTypeUnknown, // eg. Unknown resource type => use hexviewer
	StudioDocTypeDasm,
	StudioDocTypeWorkspace,
	StudioDocTypeAccelerator,
	StudioDocTypeBitmap,
	StudioDocTypeDialog,
	StudioDocTypeHex,
	StudioDocTypeIcon,
	StudioDocTypeMenu,
	StudioDocTypeStringTable,
	StudioDocTypeVersion,
}StudioMinorDocType;

///////////////////////////////////////////////////////////////////////////////

typedef struct _STUDIODOCTYPE
{
	StudioMajorDocType major;
	StudioMinorDocType minor;
}STUDIODOCTYPE, *LPSTUDIODOCTYPE;

typedef const STUDIODOCTYPE *LPCSTUDIODOCTYPE;

///////////////////////////////////////////////////////////////////////////////

class CStudioDocTemplate : public CMultiDocTemplate
{
	DECLARE_DYNAMIC(CStudioDocTemplate)
public:
	CStudioDocTemplate(UINT nIDResource, LPSTUDIODOCTYPE psdtDocType, BOOL fGenViewCap, 
		CRuntimeClass* pDocClass,CRuntimeClass* pFrameClass, 
		CRuntimeClass* pViewClass) :
		CMultiDocTemplate(nIDResource, pDocClass,pFrameClass, pViewClass)
	{
		m_sdt.major = psdtDocType->major;
		m_sdt.minor = psdtDocType->minor;
		m_fGenericViewerCapable = fGenViewCap;
	}

// Implementation
public:
	LPSTUDIODOCTYPE GetStudioDocType()
	{
		return &m_sdt;
	}

	BOOL IsGenericViewer()
	{
		return m_fGenericViewerCapable;
	}

protected:
	STUDIODOCTYPE m_sdt;
	BOOL m_fGenericViewerCapable; // Capable of handling any format of data
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CStudioDoc : public CDocument
{
protected: // create from serialization only
	CStudioDoc();
	DECLARE_DYNCREATE(CStudioDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CStudioDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();

protected:
	void Cleanup();
	
public:
	CPERes*		GetResourceParser() { return m_pResParser; }
	CPEParser*	GetExecutableParser() { return m_pExeParser; }
	HDISASM		GetDisasmHandle() {	return m_hDisasm;}
	PSYMBOL_TABLE GetSymbolTable() {return m_pSymTab;}
	CString&	GetFilePath() {return m_strFile;}
protected:
	CPERes*		m_pResParser;
	CPEParser*	m_pExeParser;
	HDISASM		m_hDisasm;
	PSYMBOL_TABLE m_pSymTab;
	CString		m_strFile;
};

///////////////////////////////////////////////////////////////////////////////

typedef enum tagUWP_EVENT 
{
	uwp_event_opening, 
	uwp_event_closing
}UWP_EVENT;

typedef struct tagUPDATE_WORKSPACE_PARAM
{
	UWP_EVENT event;
	CStudioDoc* pDocument;
}UPDATE_WORKSPACE_PARAM,*PUPDATE_WORKSPACE_PARAM;

///////////////////////////////////////////////////////////////////////////////

typedef enum tagDASM_VIEW_UPDATE_TYPE
{
	dvutCurAddressOnly,
	dvutFull,
	dvutFullNoMoveAddr,
}DASM_VIEW_UPDATE_TYPE;

///////////////////////////////////////////////////////////////////////////////

typedef struct tagDASM_VIEW_UPDATE_PARAM
{
	DASM_VIEW_UPDATE_TYPE updType;
    DWORD dwAddress;
}DASM_VIEW_UPDATE_PARAM, *PDASM_VIEW_UPDATE_PARAM;

///////////////////////////////////////////////////////////////////////////////

class CStudioWorkspaceItemInfo 
{
public:

	CStudioWorkspaceItemInfo()
	{
		m_fValid = FALSE;
		m_pWSDoc = NULL;
	}

	CStudioWorkspaceItemInfo(CStudioDoc* pWSDoc, StudioMajorDocType major,
		StudioMinorDocType minor,UINT nUniqueID)
	{
		ASSERT(pWSDoc);
		m_pWSDoc = pWSDoc;

		m_sdtDefault.major = major;
		m_sdtDefault.minor = minor;
		m_nUniqueID = nUniqueID;

		m_fValid = FALSE; //TODO: Why False
	}

	LPSTUDIODOCTYPE GetDefaultDocType() {ASSERT(m_fValid); return &m_sdtDefault;}
	UINT GetUniqueItemID()	{ASSERT(m_fValid); return m_nUniqueID;}
    CStudioDoc* GetWorkspaceDoc() {ASSERT(m_fValid && m_pWSDoc); return m_pWSDoc;}

	BOOL IsValid() { return m_fValid; }
protected:
	STUDIODOCTYPE m_sdtDefault;
	UINT m_nUniqueID;	// Used with Default Doc Type to provide unique identifier 
						// for this Document

	CStudioDoc* m_pWSDoc;
	BOOL m_fValid;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CStudioFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CStudioFrame)
public:
	virtual void ActivateFrame(int nCmdShow = -1)
	{
		 // Hide Frame as soon as it becomes visible
		nCmdShow = SW_HIDE;
		CMDIChildWnd::ActivateFrame(nCmdShow);
	}

protected:
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CStudioView : public CView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CStudioView)

// Attributes
public:

#ifdef _DEBUG
	CStudioDoc* GetDocument() const
	{
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CStudioDoc)));
		return (CStudioDoc*)m_pDocument;
	}
#else
	CStudioDoc* GetDocument() const
	{ 
		return reinterpret_cast<CStudioDoc*>(m_pDocument); 
	}

#endif

// Operations
public:

// Overrides
	public:
		virtual void OnDraw(CDC* pDC) {}; // Needed, its abstract in base class
protected:

// Implementation
public:

#ifdef _DEBUG
	virtual void AssertValid() const
	{
		CView::AssertValid();
    }

	virtual void Dump(CDumpContext& dc) const
	{
		CView::Dump(dc);
	}

#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialog
{
public:
	CAboutDlg() : CDialog(CAboutDlg::IDD) {}

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CWorkspaceItemBaseDoc : public CDocument
{
	DECLARE_DYNAMIC(CWorkspaceItemBaseDoc)
protected:

// Attributes
public:

// Operations
public:

// Overrides

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResBaseDoc)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual CStudioWorkspaceItemInfo* GetWorkspaceItemInfo() const = 0;

	// Make a local copy of this info, call initdoc & SetNiceTitle
	virtual BOOL SetWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo) = 0;
	virtual BOOL UpdateWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo) { return TRUE;}

protected:
	virtual BOOL SetNiceTitle() = 0;
	virtual BOOL InitDoc() = 0;

#ifdef _DEBUG
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CResBaseDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CStudioApp : public CWinApp
{
public:

// Overrides
public:
	virtual BOOL InitInstance();

	BOOL CreateNew(CStudioDocTemplate* pDT,CStudioWorkspaceItemInfo* pItemInfo);

	BOOL ActivateIfAlreadyOpen(CStudioDocTemplate* pDT,
							   CStudioWorkspaceItemInfo* pItemInfo, 
							   BOOL *pfAlreadyOpen);

	BOOL CreateOrActivateViewOfWorkspaceItem(
			CStudioWorkspaceItemInfo* pItemInfo, 
			LPSTUDIODOCTYPE psdtViewAsDocType = NULL);


	// Implementation

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpenWorkspace();
	afx_msg void OnFileCloseWorkspace();
	afx_msg void OnUpdateFileCloseWorkspace(CCmdUI *pCmdUI);
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnFileCloseallwindows();
	afx_msg void OnUpdateFileCloseallwindows(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()

protected:
	CStudioDocTemplate* GetDocTemplateFromStudioDocType(LPSTUDIODOCTYPE psdtViewAsDocType);
};

///////////////////////////////////////////////////////////////////////////////

extern CStudioApp theApp;

///////////////////////////////////////////////////////////////////////////////