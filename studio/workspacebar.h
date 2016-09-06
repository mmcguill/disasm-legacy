///////////////////////////////////////////////////////////////////////////////

#ifndef _WORKSPACEBAR_H_
#define _WORKSPACEBAR_H_

///////////////////////////////////////////////////////////////////////////////

#include "sizecbar.h"
#include "wsrestree.h"
#include "wsnavtree.h"
#include "WSImpExpTree.h"

///////////////////////////////////////////////////////////////////////////////

#define NUM_TABS 3

///////////////////////////////////////////////////////////////////////////////

class CWorkSpaceBar : public CSizingControlBar
{
	DECLARE_DYNAMIC(CWorkSpaceBar)
	// Constructors
public:
	CWorkSpaceBar();
	BOOL Create(CWnd* pParent, UINT nID);
private:
	struct TAB_ITEM{
		CWnd* pWnd;
		UINT nID;
	};
	// Attributes
protected:
	BOOL m_bDataAvailable;
	CTabCtrl m_tab;
	CImageList m_il;
	TAB_ITEM m_tabPages[NUM_TABS];
	CWSNavTree m_wndNav;
	CWSResTree m_wndRes;
	CWSImpExpTree m_wndImpExp;

	// Operations

	// Overrides

	// Implementation
public:
	void SetResourceViewRaw(BOOL bRaw = TRUE);
	BOOL GetResourceViewRaw();
	void Update(PUPDATE_WORKSPACE_PARAM pUwp);
	virtual ~CWorkSpaceBar();
	void DocumentWantsData();
	// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	afx_msg int OnCreate( LPCREATESTRUCT lpcs);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#endif //_WORKSPACEBAR_H_

///////////////////////////////////////////////////////////////////////////////