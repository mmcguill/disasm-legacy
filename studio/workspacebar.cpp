/////////////////////////////////////////////////////////////////////////////
// File: workspacebar.cpp
// Author: Mark McGuill
// Start Date: 13/03/2002
// Purpose: Creates Main Workspace Bar for managing disassembly
// Last Updated: 

#include "stdafx.h"
#include "studio.h" //for resource ids
#include "workspacebar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CXBORDER 8
/////////////////////////////////////////////////////////////////////////////
// CWorkSpaceBar

IMPLEMENT_DYNAMIC(CWorkSpaceBar, CSizingControlBar)

BEGIN_MESSAGE_MAP(CWorkSpaceBar, CSizingControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, ID_WS_TAB, OnSelchangeTab)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// Constructors
CWorkSpaceBar::CWorkSpaceBar()
{
	m_bDataAvailable = FALSE;
}

CWorkSpaceBar::~CWorkSpaceBar()
{
}

BOOL CWorkSpaceBar::Create(CWnd* pParent, UINT nID)
{
	return CSizingControlBar::Create(pParent,CSize(300,300),nID,CBRS_SIZE_DYNAMIC | CBRS_LEFT);
}

/////////////////////////////////////////////////////////////////////////
// Message Handlers
int CWorkSpaceBar::OnCreate( LPCREATESTRUCT lpCS)
{
	if(CSizingControlBar::OnCreate(lpCS) == -1) return -1;

	// Create Views - Initialise Tabs
	m_tabPages[0].nID = IDR_WORKSPACE_DASM;
	m_tabPages[0].pWnd = NULL;
	m_tabPages[1].nID = IDR_WORKSPACE_RESOURCES;
	m_tabPages[1].pWnd = NULL;
	m_tabPages[2].nID = IDR_WORKSPACE_IMPEXP;
	m_tabPages[2].pWnd = NULL;

	// Create Views - Actual Create
	m_wndNav.Create(this,m_tabPages[0].nID);
	m_wndRes.Create(this,m_tabPages[1].nID);
	m_wndImpExp.Create(this,m_tabPages[2].nID);
	m_tabPages[0].pWnd=&m_wndNav;
	m_tabPages[1].pWnd=&m_wndRes;
	m_tabPages[2].pWnd=&m_wndImpExp;

	//Check Their OK
	for(int x=0;x<NUM_TABS;x++){
		if(m_tabPages[x].pWnd == NULL)
		{
			CString str;
			str.LoadString(m_tabPages[x].nID);
			TRACE1("Error Creating tab Page: %s",str);
			ASSERT(FALSE); //error Creating Tab Window - see debug output.
			return -1; //Cannot Continue
		}
	}

	// Image List
	m_il.Create(16,16,TRUE,NUM_TABS,0);
	for(x=0;x<NUM_TABS;x++){
		m_il.Add(AfxGetApp()->LoadIcon(m_tabPages[x].nID));
	}

	//Tab Control
	m_tab.Create(TCS_FOCUSNEVER|TCS_BOTTOM|TCS_TOOLTIPS|TCS_TABS|TCS_SINGLELINE|WS_VISIBLE,CRect(0,0,0,0),this,ID_WS_TAB);
	CFont *font = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	m_tab.SetFont(font);
	m_tab.SetImageList(&m_il);

	//Make The Tabs
	CString str;
	for(x=0;x<NUM_TABS;x++){
		str.LoadString(m_tabPages[x].nID);
		m_tab.InsertItem(x,str,x);
	}
	return 0;
}

void CWorkSpaceBar::OnSize(UINT nType, int cx, int cy)
{
	CSizingControlBar::OnSize(nType, cx, cy);
	m_tab.MoveWindow(CXBORDER,CXBORDER, cx - (CXBORDER * 2), cy-(CXBORDER * 2));
	
	// Resize Views
	CRect rc;
	m_tab.GetWindowRect(rc);
	m_tab.AdjustRect(FALSE,rc);
	ScreenToClient(rc);
	for(int x=0;x<NUM_TABS;x++){
		m_tabPages[x].pWnd->MoveWindow(rc);
	}
}

/////////////////////////////////////////////////////////////////////

void CWorkSpaceBar::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	if(m_bDataAvailable){
		int nCur = m_tab.GetCurFocus();
		for(int x=0;x<NUM_TABS;x++){
			m_tabPages[x].pWnd->ShowWindow((x==nCur) ? SW_SHOW : SW_HIDE);
		}
	}
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////

void CWorkSpaceBar::Update(PUPDATE_WORKSPACE_PARAM pUwp)
{
	m_bDataAvailable = (pUwp->event == uwp_event_opening);

	int nCur = m_tab.GetCurFocus();

	for(int x=0;x<NUM_TABS;x++)
	{
		m_tabPages[x].pWnd->SendMessage(WM_UPDATE_DOCKVIEW,(WPARAM)pUwp,0);
		m_tabPages[x].pWnd->ShowWindow((x==nCur && m_bDataAvailable) ? SW_SHOW : SW_HIDE);
	}
}

/////////////////////////////////////////////////////////////////////

BOOL CWorkSpaceBar::GetResourceViewRaw()
{
	return m_wndRes.GetViewRaw();
}

/////////////////////////////////////////////////////////////////////

void CWorkSpaceBar::SetResourceViewRaw(BOOL bRaw)
{
	m_wndRes.SetViewRaw(bRaw);
}

/////////////////////////////////////////////////////////////////////