///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

class CResBaseFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CResBaseFrame)
protected:
	CResBaseFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconFrame)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CResBaseFrame();

	// Generated message map functions
	//{{AFX_MSG(CIconFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconFrame frame

class CIconFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CIconFrame)
protected:
	CIconFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	void SetHotSpot(BOOL bShow, CPoint ptHot = CPoint(0,0));
	void AddDevImage(CString& strImg);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconFrame)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:
	CDialogBar m_wndHeadBar;
	virtual ~CIconFrame();

	// Generated message map functions
	//{{AFX_MSG(CIconFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg void OnCmbChange();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////