///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgPropDlg dialog
enum WINDOW_TYPE{DIALOG,STATIC,BUTTON,COMBOBOX,LISTBOX,EDIT,DLG_PROP_DLG_UNKNOWN};

class CDlgPropDlg : public CDialog
{
// Construction
public:
	WINDOW_TYPE m_type;
	DWORD m_dwExStyle;
	DWORD m_dwStyle;

	CDlgPropDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgPropDlg)
	enum { IDD = IDD_DLGPROPERTIES };
	CString	m_strData;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DoStyles();

	// Generated message map functions
	//{{AFX_MSG(CDlgPropDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
