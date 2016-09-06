///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

#include "DasmDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CWSNavTree window

class CWSNavTree : public CTreeCtrl
{
	DECLARE_DYNCREATE(CWSNavTree)
// Construction
public:
	CWSNavTree();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWSNavTree)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual BOOL Create(CWnd* pParent, UINT nID);
	virtual ~CWSNavTree();

	// Generated message map functions
protected:
	CImageList m_il;
	HTREEITEM m_hRoot;
	//{{AFX_MSG(CWSNavTree)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnUpdate(WPARAM, LPARAM);

	void Cleanup(void);
	BOOL FillEntryPoints(void);
	void CreateViewOfCurrentItem();
	vector<CStudioWorkspaceDasmItemInfo*> m_vecItems;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	void CleanupWSItems();
	void EmptyTree();
	CStudioDoc* m_pDoc;
};

/////////////////////////////////////////////////////////////////////////////