///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

#include "WorkSpaceBar.h"

///////////////////////////////////////////////////////////////////////////////

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CReBar			m_wndReBar;
	CWorkSpaceBar	m_wndWorkSpaceBar;

// Generated message map functions
protected:
	afx_msg LRESULT OnUpdateWorkSpace(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewWorkspace();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateViewWorkspace(CCmdUI *pCmdUI);
	afx_msg void OnViewRawresources();
	afx_msg void OnUpdateViewRawresources(CCmdUI *pCmdUI);
};

///////////////////////////////////////////////////////////////////////////////
