///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

#include "afxcmn.h"

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CWSImpExpTree

class CWSImpExpTree : public CTreeCtrl
{
	DECLARE_DYNAMIC(CWSImpExpTree)

public:
	CWSImpExpTree();
	virtual ~CWSImpExpTree();
	virtual BOOL Create(CWnd* pParent, UINT nID);
		
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnUpdate(WPARAM, LPARAM);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

protected:
   // HTREEITEM m_hRoot;
	CImageList m_il;
	BOOL FillTree(void);
	BOOL FillImports(CPEParser* pExe);
	BOOL FillExports(CPEParser* pExe);
	BOOL FillTLB(CPEParser* pExe);

public:
	void GetDetailsOfCurrentItem(void);

protected:
	CStudioDoc* m_pDoc;
	void EmptyTree();
};

///////////////////////////////////////////////////////////////////////////////