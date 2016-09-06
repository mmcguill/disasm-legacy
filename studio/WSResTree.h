///////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////

#include "ResourcesDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CWSResTree window

class CWSResTree : public CTreeCtrl
{
DECLARE_DYNCREATE(CWSResTree)

// TODO: Is This Enum Necessary?
typedef enum _ResourceViewAsType
{
	ResourceViewAsTypeDefault,
	ResourceViewAsTypeHex,
	ResourceViewAsTypeUserChoose,
}ResourceViewAsType;

// Construction
public:
	CWSResTree();
	static StudioMinorDocType GetStudioDocTypeFromResourceType(LPTSTR lpwType);

	// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWSResTree)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetViewRaw(BOOL bRaw = TRUE);
	BOOL GetViewRaw();
	void CreateViewOfCurrentItem(ResourceViewAsType rvat);
	virtual BOOL Create(CWnd* pParent, UINT nID);
	virtual ~CWSResTree();
	virtual void Cleanup();

	// Generated message map functions
protected:
	BOOL m_bViewRaw;
	CImageList m_il;
	HTREEITEM m_hRoot;
	
	// Used to keep track of resource item info & allow quick deletion
	vector<CStudioWorkspaceResourceItemInfo*> m_vecItems;

	//{{AFX_MSG(CWSResTree)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg void OnRclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUpdate(WPARAM, LPARAM);
	afx_msg void OnViewHex();
	afx_msg void OnViewStd();

	DECLARE_MESSAGE_MAP()
private:
	BOOL FillStringSubTree(DWORD dwRoot,DWORD dwIndex);
	//BOOL FillIconCursorSubTree(DWORD dwRoot,DWORD dwGroupCursorObj, BOOL bCursor);
	BOOL FillStdSubTree(DWORD dwRoot, LPTSTR lpszType, BOOL fTypeIsString,DWORD dwIndex);
	BOOL FillTree();
public:
	afx_msg void OnCtxtViewwith();

protected:
	CStudioDoc* m_pDoc;
};

/////////////////////////////////////////////////////////////////////////////