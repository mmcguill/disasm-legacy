#pragma once


// CChooseViewerDlg dialog

class CChooseViewerDlg : public CDialog
{
	DECLARE_DYNAMIC(CChooseViewerDlg)

public:
	CChooseViewerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseViewerDlg();

// Dialog Data
	enum { IDD = IDD_CHOOSEVIEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	LPSTUDIODOCTYPE GetStudioDocType(void)
	{
		return &m_sdt;
	}

protected:
	STUDIODOCTYPE m_sdt;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnLbnSelchangeLbviewers();
};
