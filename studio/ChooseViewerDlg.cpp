///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "ChooseViewerDlg.h"
#include ".\chooseviewerdlg.h"

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CChooseViewerDlg, CDialog)
CChooseViewerDlg::CChooseViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseViewerDlg::IDD, pParent)
{
}

///////////////////////////////////////////////////////////////////////////////

CChooseViewerDlg::~CChooseViewerDlg()
{
}

///////////////////////////////////////////////////////////////////////////////

void CChooseViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CChooseViewerDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LBVIEWERS, OnLbnSelchangeLbviewers)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CChooseViewerDlg message handlers

BOOL CChooseViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CListBox* pLB = (CListBox*)GetDlgItem(IDC_LBVIEWERS);
	ASSERT(pLB);

	CStudioDocTemplate* pCurDT;
	POSITION p = AfxGetApp()->GetFirstDocTemplatePosition();

	if(p == NULL)
	{
		ASSERT(FALSE); // Serious Problem if this happens
		return TRUE;
	}

	CString str;

	do
	{
		pCurDT = static_cast<CStudioDocTemplate*>(AfxGetApp()->GetNextDocTemplate(p));
		ASSERT(pCurDT);

		if(pCurDT->IsGenericViewer())
		{
			pCurDT->GetDocString(str,CDocTemplate::windowTitle);
			int nItem = pLB->AddString(str);

			LPSTUDIODOCTYPE ptype = pCurDT->GetStudioDocType();

			// HACK: FEATURE: For Moment Always Select Hexviewer
			if(StudioDocTypeHex == ptype->minor)
			{
				pLB->SetCurSel(nItem);
			}

			pLB->SetItemData(nItem,(DWORD_PTR)ptype);

		}
	}while(NULL != p);

	return TRUE;
}

void CChooseViewerDlg::OnBnClickedOk()
{
	CListBox* pLB = (CListBox*)GetDlgItem(IDC_LBVIEWERS);
	ASSERT(pLB);

	int nItem;
	if(LB_ERR != (nItem = pLB->GetCurSel()))
	{
		LPSTUDIODOCTYPE lpType = (LPSTUDIODOCTYPE)pLB->GetItemData(nItem);
		ASSERT(lpType);
		m_sdt.major = lpType->major;
		m_sdt.minor = lpType->minor;
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	OnOK();
}

void CChooseViewerDlg::OnLbnSelchangeLbviewers()
{
	CListBox* pLB = (CListBox*)GetDlgItem(IDC_LBVIEWERS);
	ASSERT(pLB);

	GetDlgItem(IDOK)->EnableWindow((LB_ERR != pLB->GetCurSel()));
}
