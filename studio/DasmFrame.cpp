///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "DasmFrame.h"

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDasmBaseFrame

IMPLEMENT_DYNCREATE(CDasmBaseFrame, CMDIChildWnd)

///////////////////////////////////////////////////////////////////////////////

CDasmBaseFrame::CDasmBaseFrame()
{
}

///////////////////////////////////////////////////////////////////////////////

CDasmBaseFrame::~CDasmBaseFrame()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDasmBaseFrame, CMDIChildWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CDasmBaseFrame message handlers

int CDasmBaseFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	EnableDocking(CBRS_ALIGN_TOP);

	if(!m_wndGlobalView.Create(this,IDD_GLOBAL_VIEW_PANE,
		WS_CHILD | CBRS_TOP | /*CBRS_GRIPPER |*/ 
		CBRS_TOOLTIPS | CBRS_FLYBY,AFX_IDW_TOOLBAR))
	{
		return -1;
	}


	CString str;
	str.LoadString(IDS_GLOBAL_VIEW_BAR_CAPTION);
	m_wndGlobalView.SetWindowText((LPCTSTR)str);
	m_wndGlobalView.EnableDocking(CBRS_ALIGN_TOP);

	// TODO: Should I enumerate all views ?
	
	//CDasmView* pView = (CDasmView*)GetActiveView();
	//Assert(pView);
	//pView->SetGlobalViewBarWnd(&m_wndGlobalView);

	//FloatControlBar(&m_wndGlobalView,CPoint(20,30));
	DockControlBar(&m_wndGlobalView);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CDasmBaseFrame::Push(LPCTSTR lpszAddress)
{
	//CListBox* pLB = (CListBox*)m_wndCallStack.GetDlgItem(IDC_CALLSTACK);
	//pLB->InsertString(0,lpszAddress);
}

///////////////////////////////////////////////////////////////////////////////

void CDasmBaseFrame::Pop()
{
	/*CListBox* pLB = (CListBox*)m_wndCallStack.GetDlgItem(IDC_CALLSTACK);
	pLB->DeleteString(0);*/
}

///////////////////////////////////////////////////////////////////////////////
