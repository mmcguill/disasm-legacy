#include "StdAfx.h"
#include "Studio.h"
#include "dasmdoc.h"

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDasmBaseDoc, CWorkspaceItemBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CDasmBaseDoc::CDasmBaseDoc(void)
{
	m_pItemInfo = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CDasmBaseDoc::~CDasmBaseDoc(void)
{
	Cleanup();
}

/////////////////////////////////////////////////////////////////////////////

void CDasmBaseDoc::Cleanup()
{
	ASSERT(m_pItemInfo); // Shouldnt have document without this!
	if(m_pItemInfo)
	{
		delete m_pItemInfo;
		m_pItemInfo = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDasmBaseDoc, CWorkspaceItemBaseDoc)
	//{{AFX_MSG_MAP(CDasmBaseDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CDasmBaseDoc::SetWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo)
{
	CStudioWorkspaceDasmItemInfo* pInfo = (CStudioWorkspaceDasmItemInfo*)pItemInfo;

	m_pItemInfo = new CStudioWorkspaceDasmItemInfo(pInfo);
    ASSERT(m_pItemInfo);

	if(!SetNiceTitle())
	{
		Cleanup();
		return FALSE;
	}

	if(!InitDoc())
	{
		Cleanup();
		return FALSE;
	}

	DASM_VIEW_UPDATE_PARAM dvp;
	dvp.dwAddress = m_pItemInfo->GetStartAddress();
	dvp.updType = dvutFull;

	UpdateAllViews(NULL,(LPARAM)&dvp,NULL);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDasmBaseDoc::UpdateWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo)
{
	CStudioWorkspaceDasmItemInfo* pInfo = (CStudioWorkspaceDasmItemInfo*)pItemInfo;
	ASSERT(pInfo);

	DASM_VIEW_UPDATE_PARAM dvp;
	dvp.dwAddress = pInfo->GetStartAddress();
	dvp.updType = dvutCurAddressOnly;

	UpdateAllViews(NULL,(LPARAM)&dvp,NULL);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

DWORD WINAPI DasmThreadProc(LPVOID lpv)
{
	PDASM_THREAD_PARAM pdtp = (PDASM_THREAD_PARAM)lpv;
	return WIN_DisassembleEPs(pdtp->hDasm,pdtp->hWndUpdate);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDasmBaseDoc::InitDoc()
{
	ASSERT(m_pItemInfo);

	CPEParser *pPEParser = m_pItemInfo->GetWorkspaceDoc()->GetExecutableParser();
	ASSERT(pPEParser);


	// TODO: Delegate to another thread and display progress...
    
	HDISASM hDasm = m_pItemInfo->GetWorkspaceDoc()->GetDisasmHandle();

	DWORD dwId = 0;

	m_dtp.hDasm = hDasm;
	m_dtp.hWndUpdate = AfxGetMainWnd()->GetSafeHwnd();

	HANDLE m_htdDasm = CreateThread(NULL,0,DasmThreadProc,&m_dtp,0,&dwId);

	WaitForSingleObject(m_htdDasm,INFINITE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CStudioWorkspaceItemInfo* CDasmBaseDoc::GetWorkspaceItemInfo() const
{
	return (CStudioWorkspaceItemInfo*)m_pItemInfo;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDasmBaseDoc::SetNiceTitle()
{
	CString str;
	m_pDocTemplate->GetDocString(str,CDocTemplate::windowTitle);

	CString strFormat;

	if(m_pItemInfo->GetFriendlySymbolName())
	{
	strFormat.Format(
		IDS_FORMAT_DASM_NICE_TITLE,
		str,
		*m_pItemInfo->GetFriendlySymbolName(),
		m_pItemInfo->GetStartAddress());
	}
	else
	{
	strFormat.Format(
		IDS_FORMAT_DASM_NICE_TITLE,
		str,
		_T("<NO SYMBOL>"),
		m_pItemInfo->GetStartAddress());
	}

	SetTitle(strFormat);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

PLISTINGS_MANAGER CDasmBaseDoc::GetListingsManager()
{
	ASSERT(m_pItemInfo);

	HDISASM hDasm = m_pItemInfo->GetWorkspaceDoc()->GetDisasmHandle();
    if(NULL == hDasm)
	{
		// Not Ready Yet, havent been initialized	
		return NULL;
	}

	PLISTINGS_MANAGER pMgr = GetListings_Manager(hDasm);

	ASSERT(pMgr);
	return pMgr;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDasmBaseDoc::DisassembleAddress(DWORD dwAddress)
{
	ASSERT(m_pItemInfo);

	HDISASM hDasm = m_pItemInfo->GetWorkspaceDoc()->GetDisasmHandle();
    if(NULL == hDasm)
	{
		// Not Ready Yet, havent been initialized	
		return FALSE;
	}

	return FALSE; //(ERR_SUCCESS == m_pDisEng->Disassemble(dwAddress));
}

/////////////////////////////////////////////////////////////////////////////