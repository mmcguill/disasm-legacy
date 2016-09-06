///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "MainFrm.h"
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_MESSAGE(WM_UPDATEWORKSPACE,OnUpdateWorkSpace)
	ON_COMMAND(ID_VIEW_WORKSPACE, OnViewWorkspace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORKSPACE, OnUpdateViewWorkspace)
	ON_COMMAND(ID_VIEW_RAWRESOURCES, OnViewRawresources)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RAWRESOURCES, OnUpdateViewRawresources)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

///////////////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

///////////////////////////////////////////////////////////////////////////////

CMainFrame::~CMainFrame()
{
}

///////////////////////////////////////////////////////////////////////////////

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	
	// Toolbars & Such

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	if(!m_wndWorkSpaceBar.Create(this, IDW_WORKSPACE))
	{
		TRACE0("Failed to create Workspace bar");
	}
     
	// Dock

	EnableDocking(CBRS_ALIGN_ANY);
	m_wndWorkSpaceBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndWorkSpaceBar);

	// Tool TIps
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | 
								CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CMainFrame::RecalcLayout(BOOL bNotify) 
{
	CMDIFrameWnd::RecalcLayout(bNotify);
	CMDIFrameWnd::RecalcLayout(bNotify); //necessary for workspacebar
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnUpdateWorkSpace(WPARAM wParam, LPARAM lParam)
{
	PUPDATE_WORKSPACE_PARAM puwp = (PUPDATE_WORKSPACE_PARAM)wParam;
	ASSERT(puwp);

	CString strTitle;

	if(uwp_event_opening == puwp->event) // Update Window Title with Filename
	{
		CString strFullPath = puwp->pDocument->GetFilePath();
		int index = strFullPath.ReverseFind(_T('\\'));
        strTitle = strFullPath.Mid(index+1);
	}
	else
	{
		strTitle.LoadString(IDS_MAINFRAME_TITLE_CLOSED);
		
	}

	SetTitle(strTitle);

	m_wndWorkSpaceBar.Update(puwp);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnViewWorkspace()
{
	ShowControlBar(&m_wndWorkSpaceBar,
		!m_wndWorkSpaceBar.IsWindowVisible(),0);
}

///////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateViewWorkspace(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndWorkSpaceBar.IsWindowVisible());
}

///////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnViewRawresources()
{
	m_wndWorkSpaceBar.SetResourceViewRaw(!m_wndWorkSpaceBar.GetResourceViewRaw());
}

///////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateViewRawresources(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndWorkSpaceBar.GetResourceViewRaw());
}

///////////////////////////////////////////////////////////////////////////////