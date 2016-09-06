/////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

//**************************************************************************************//
// Mark McGuill - 11/04/2002                                                            //
//		                                                                                //
// Private Structures used by Dasm Studio to Parse different objects in                 //
// the PE File. A lot of em are from resfmt.txt or the sdk help files, some             //
// had o be created from scratch. Define sizeof after each truct and do not use the     //
// c++ 'sizeof' operator as compiler will pack structures and alter their actual	    //
// byte size ==> see /Zp option - MSVC 6. so use size defines.                          //
//**************************************************************************************//

typedef struct _ACCELTABLEENTRY{ 
    WORD fFlags; 
    WORD wAnsi; 
    WORD wId; 
    WORD padding; 
}ACCELTABLEENTRY,*PACCELTABLEENTRY;
#define SIZE_OF_ACCELTABLEENTRY 8

/////////////////////////////////////////////////////////////////////////////

typedef struct _EX_MENUHEADER{
     WORD  wVersion; 
     WORD  wOffset; 
     DWORD dwHelpId; 
}EX_MENUHEADER,*PEX_MENUHEADER; 
#define SIZE_OF_EX_MENUHEADER 8

/////////////////////////////////////////////////////////////////////////////

typedef struct _EX_MENUITEM{
     DWORD  dwType; 
     DWORD  dwState; 
     UINT   uId; 
     WORD   wResInfo; 
}EX_MENUITEM,*PEX_MENUITEM; 
#define SIZE_OF_EX_MENUITEM 14

/////////////////////////////////////////////////////////////////////////////

typedef struct _GROUP_CURSOR_RESOURCE{
	WORD bWidth;
	WORD bHeight;
	WORD wPlanes;
	WORD wBitCount;
	WORD wBytesInResLow; // Use Words here instead of one dword
	WORD wBytesInResHigh;// look at /Zp compiler option to explain!!
	WORD wNameOrdinal;
}GROUP_CURSOR_RESOURCE,*PGROUP_CURSOR_RESOURCE;
#define SIZE_OF_GROUP_CURSOR_RESOURCE 14

/////////////////////////////////////////////////////////////////////////////

typedef struct _GROUP_ICON_RESOURCE{
	BYTE bWidth;
	BYTE bHeight;
	BYTE bColorCount;
	BYTE bReserved;
	WORD wPlanes;
	WORD wBitCount;
	WORD wBytesInResLow; // Use Words here instead of one dword
	WORD wBytesInResHigh;// look at /Zp compiler option to explain!!
	WORD wNameOrdinal;
}GROUP_ICON_RESOURCE,*PGROUP_ICON_RESOURCE;
#define SIZE_OF_GROUP_ICON_RESOURCE 14

/////////////////////////////////////////////////////////////////////////////

typedef struct _GROUP_ICON_HEADER{
	WORD wReserved;
	WORD wType;
	WORD wCount;
}GROUP_ICON_HEADER,*PGROUP_ICON_HEADER;
#define SIZE_OF_GROUP_ICON_HEADER 6

/////////////////////////////////////////////////////////////////////////////

typedef struct _VS_VERSIONINFO_1{
	WORD  wLength; 
    WORD  wValueLength; 
    WORD  wType; 
}VS_VERSIONINFO_1,*PVS_VERISONINFO_1;
#define SIZE_OF_VS_VERSIONINFO_1 6

/////////////////////////////////////////////////////////////////////////////

class CStudioWorkspaceResourceItemInfo : public CStudioWorkspaceItemInfo
{
public:
	CStudioWorkspaceResourceItemInfo()
	{
		m_pstrName = NULL;
		m_pstrType = NULL; // Resource Type
		m_fValid = FALSE;
	}

	~CStudioWorkspaceResourceItemInfo()
	{
		if(m_pstrName)
		{
			delete m_pstrName;
		}
		if(m_pstrType)
		{
			delete m_pstrType;
		}
	}

	CStudioWorkspaceResourceItemInfo(CStudioWorkspaceResourceItemInfo* pCopy)
	{
		ASSERT(pCopy);
		m_sdtDefault.major = pCopy->GetDefaultDocType()->major;
		m_sdtDefault.minor = pCopy->GetDefaultDocType()->minor;
		m_nUniqueID = pCopy->GetUniqueItemID();
		m_pWSDoc = pCopy->m_pWSDoc;

		m_fValid = pCopy->IsValid();		
		
		m_dwPEIndex = pCopy->GetPEIndex();
		m_dwID = pCopy->GetID();
		m_dwLang = pCopy->GetLanguage();
		if(pCopy->GetResourceName())
		{
			m_pstrName = new CString(*pCopy->GetResourceName());
			ASSERT(m_pstrName);
		}
		else
		{
			m_pstrName = NULL;
		}

		if(pCopy->GetResourceTypeString())
		{
			m_pstrType = new CString(*pCopy->GetResourceTypeString());
			ASSERT(m_pstrType);
		}
		else
		{
			m_pstrType = NULL;
		}
	}

	CStudioWorkspaceResourceItemInfo(CStudioDoc* pWSDoc,		
		StudioMajorDocType major,StudioMinorDocType minor,
		UINT nUniqueID,	DWORD dwPEIndex,DWORD dwID,DWORD dwLang,
		LPCTSTR lpszName,LPCTSTR lpszTypeString) : 
	CStudioWorkspaceItemInfo(pWSDoc, major,minor,nUniqueID)
	{
		m_dwPEIndex=dwPEIndex;
		m_dwID =	dwID;
		m_dwLang =	dwLang;
		if(lpszName)
		{
			m_pstrName = new CString(lpszName);
			ASSERT(m_pstrName);
		}
		else
		{
			m_pstrName = NULL;
		}

		if(lpszTypeString)
		{
			m_pstrType = new CString(lpszTypeString);
			ASSERT(m_pstrType);
		}
		else
		{
			m_pstrType = NULL;
		}

		m_fValid = TRUE;
	}

	CString* GetResourceName()  const		{ASSERT(m_fValid);return m_pstrName;}
	CString* GetResourceTypeString()  const	{ASSERT(m_fValid);return m_pstrType;}

	DWORD GetPEIndex() const	{ASSERT(m_fValid);return m_dwPEIndex;}
	DWORD GetID() const			{ASSERT(m_fValid);return m_dwID;}
	DWORD GetLanguage() const	{ASSERT(m_fValid);return m_dwLang;}
	
private:
	DWORD m_dwPEIndex;	// Index into type in this pefile => Allows Quick Lookup
	DWORD m_dwID;			// Resource ID
	DWORD m_dwLang;			// Language ID
	CString* m_pstrName;		// Maybe Named Resource
	CString* m_pstrType;
};

/////////////////////////////////////////////////////////////////////////////

class CResBaseDoc : public CWorkspaceItemBaseDoc
{
protected:
	CResBaseDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CResBaseDoc)

// Attributes
public:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResBaseDoc)
	public:
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

	BYTE* GetRawData()
	{
		return m_pData;
	}

	DWORD GetRawDataSize()
	{
		return m_dwSize;
	}

protected:
	DWORD m_dwSize;
	BYTE* m_pData;

// Implementation
public:
	virtual ~CResBaseDoc();
	virtual BOOL SetWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo);
	virtual BOOL InitDoc();
	virtual CStudioWorkspaceItemInfo* GetWorkspaceItemInfo() const;
	virtual BOOL SetNiceTitle();

	// Generated message map functions
protected:
	//{{AFX_MSG(CResBaseDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCtxtViewhex();
	afx_msg void OnCtxtViewwith();

protected:
	CStudioWorkspaceResourceItemInfo* m_pItemInfo;
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStringTableDoc document

typedef struct _STRING_TABLE_ITEM
{
	WORD wID;
	CString strText;
	_STRING_TABLE_ITEM* pNext;
}STRING_TABLE_ITEM,*LPSTRING_TABLE_ITEM;

/////////////////////////////////////////////////////////////////////////////

class CStringTableDoc : public CResBaseDoc
{
protected:
	CStringTableDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CStringTableDoc)
	
// Attributes
protected:
	STRING_TABLE_ITEM* m_pFirst;
	DWORD m_dwStrCount;
// Operations
public:
	DWORD GetNumStrings(){return m_dwStrCount;}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringTableDoc)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	void GetString(DWORD str, CString& strString, WORD* pwID);
	virtual BOOL InitDoc();
	virtual BOOL SetNiceTitle();
	virtual ~CStringTableDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	void ProcessStringBlock(WORD* pStr, WORD wID);
	void DeleteAllStrings(STRING_TABLE_ITEM* pEntry);
	//{{AFX_MSG(CStringTableDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CHexDoc

class CHexDoc : public CResBaseDoc
{
protected:
	CHexDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHexDoc)

// Attributes
public:
// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHexDoc)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	void _SetRawData(BYTE* pData, DWORD dwSize,CString& strTitle);
	virtual ~CHexDoc();
	virtual BOOL InitDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// Generated message map functions
protected:
	//{{AFX_MSG(CHexDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelDoc document

class CAccelDoc : public CResBaseDoc
{
protected:
	CAccelDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAccelDoc)

// Attributes
public:

// Operations
public:
	void GetVirtKeyString(WORD wKey, CString &str);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccelDoc)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual BOOL InitDoc();
	virtual ~CAccelDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CAccelDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpDoc document

class CBmpDoc : public CResBaseDoc
{
protected:
	CBmpDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBmpDoc)

// Attributes
public:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBmpDoc)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBmpDoc();
	virtual BOOL InitDoc();
	HANDLE GetBitmap()
	{
		return m_hBmp;
	}

	CSize* GetBitmapSize()
	{
		return &m_szBmp;
	}

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	HANDLE m_hBmp;
	CSize m_szBmp;
//	CBitmap m_bmp;
	//{{AFX_MSG(CBmpDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgDoc document

#include "dialoghelper.h"

class CDlgDoc : public CResBaseDoc
{
protected:
	CDlgDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDlgDoc)

// Attributes
public:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDoc)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL IsValid();
	CDialogHelper m_dlgHelp;
	virtual ~CDlgDoc();
	virtual BOOL InitDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	BOOL m_bValid;
	//{{AFX_MSG(CDlgDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconDoc document

class CIconDoc : public CResBaseDoc
{
protected:
	CIconDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CIconDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconDoc)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	// inlines
	int		GetIconCount(){return m_nIconCount;}
	HICON	GetIcon(){return m_hIcon;}
	CSize	GetSize(){return CSize(m_cx,m_cy);}
	CPoint	GetHotSpot(){return m_ptHot;}
	BOOL	IsIcon(){return m_bIcon;}

	BOOL SetIconIndex(int nIcon);
	BOOL GetIconInfo(int nIcon,int* pcx,int* pcy,int* pnBase, int* pNumColors =NULL);
	virtual ~CIconDoc();
	virtual BOOL InitDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	CPoint	m_ptHot;
	BOOL	m_bIcon;
	int		m_nIconCount;
	int		m_cy;
	int		m_cx;
	HICON	m_hIcon;

	//{{AFX_MSG(CIconDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// CMenuDoc document

class CMenuDoc : public CResBaseDoc
{
protected:
	CMenuDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMenuDoc)

// Attributes
public:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMenuDoc)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMenuDoc();
	virtual BOOL InitDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMenuDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionDoc document

class CVersionDoc : public CResBaseDoc
{
protected:
	CVersionDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CVersionDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVersionDoc)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVersionDoc();
	virtual BOOL InitDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CVersionDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
