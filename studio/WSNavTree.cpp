///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "studio.h"
#include "WSNavTree.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWSNavTree

IMPLEMENT_DYNCREATE(CWSNavTree, CTreeCtrl)

///////////////////////////////////////////////////////////////////////////////

CWSNavTree::CWSNavTree()
{
	m_hRoot = NULL;
	m_pDoc = NULL;
}

///////////////////////////////////////////////////////////////////////////////

CWSNavTree::~CWSNavTree()
{
	Cleanup();
}

///////////////////////////////////////////////////////////////////////////////

void CWSNavTree::Cleanup()
{
	CleanupWSItems();
	m_pDoc = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void CWSNavTree::CleanupWSItems()
{
	for(vector<CStudioWorkspaceDasmItemInfo*>::iterator theItem = m_vecItems.begin();
		theItem != m_vecItems.end();theItem++)
	{
		if(NULL != (*theItem))
		{
			delete (*theItem);
		}
	}

	m_vecItems.clear();
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CWSNavTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CWSNavTree)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATE_DOCKVIEW,OnUpdate)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CWSNavTree message handlers

BOOL CWSNavTree::Create(CWnd *pParent, UINT nID)
{
	return CTreeCtrl::Create(/*TVS_TRACKSELECT|*/WS_BORDER|WS_TABSTOP|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS,CRect(0,0,0,0),pParent,nID);
}

///////////////////////////////////////////////////////////////////////////////

int CWSNavTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyleEx(0,WS_EX_CLIENTEDGE);

	m_il.Create(16,16,TRUE,3,0);
	m_il.Add(AfxGetApp()->LoadIcon(IDI_FOLDERC));
	m_il.Add(AfxGetApp()->LoadIcon(IDI_FOLDERO));
	m_il.Add(AfxGetApp()->LoadIcon(IDI_DOC));
	SetImageList(&m_il,TVSIL_NORMAL);
	CString str;
	str.LoadString(IDR_WORKSPACE_DASM);
	m_hRoot = InsertItem(str,0,1);
	SetItemData(m_hRoot,NULL);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CWSNavTree::OnUpdate(WPARAM wParam, LPARAM)
{
	PUPDATE_WORKSPACE_PARAM pUwp = (PUPDATE_WORKSPACE_PARAM)wParam;
	ASSERT(pUwp);

	if(uwp_event_opening == pUwp->event)
	{
		ASSERT(pUwp->pDocument);
		m_pDoc = pUwp->pDocument;

		if(!FillEntryPoints())
		{
			// TODO: DoSomething
		}
	}
	else
	{
		EmptyTree();
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CWSNavTree::EmptyTree()
{
	DeleteAllItems();
	CleanupWSItems();
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSNavTree::FillEntryPoints(void)
{
	EmptyTree();

	CString str;
	str.LoadString(IDR_WORKSPACE_DASM);
	m_hRoot = InsertItem(str,0,1);
	SetItemData(m_hRoot,NULL);

	ASSERT(m_pDoc);
    CPEParser* pPE = m_pDoc->GetExecutableParser();
	//ASSERT(pPE);
	
	
	// TODO! Add exports and entry point...

	PSYMBOLS_MAP pSymMap = m_pDoc->GetSymbolTable()->GetSymbolsMap();
	ASSERT(pSymMap);

	for(SYMBOLS_MAP::iterator sym = pSymMap->begin();
		sym!=pSymMap->end();sym++)
	{
		DWORD dwEntry = sym->first;
		PSYMBOL pSym = sym->second;

		if (0 == dwEntry)
		{
			ASSERT(FALSE);
            continue;
		}
		

		if (symbol_import == pSym->GetType() ||
			symbol_indirect == pSym->GetType())
		{
            continue;
		}

		// Add Entry Point

		TCHAR szSym[MAX_CCH_SYMBOL_NAME];

#ifdef _UNICODE
		WCHAR szSymName[MAX_CCH_SYMBOL_NAME];
		MultiByteToWideChar(CP_ACP,0,pSym->GetName(),-1,szSymName,MAX_CCH_SYMBOL_NAME-1);
		szSymName[MAX_CCH_SYMBOL_NAME - 1] = 0;

		wcscpy(szSym,szSymName);
#else
		strcpy(szSym,pSym->GetName());
#endif
					
		CStudioWorkspaceDasmItemInfo* lpData = 
					new CStudioWorkspaceDasmItemInfo(m_pDoc,
						StudioMajorTypeDasm,StudioDocTypeDasm,
						0,dwEntry,0,szSym);

		ASSERT(lpData);
		if(NULL == lpData)
		{
			// TODO: What do we do with out Of Mem errors?
			return FALSE;
		}

		m_vecItems.push_back(lpData);
		
		CString strEP;
		strEP.Format(_T("%s [%0.8X]"),
			(LPCTSTR)*lpData->GetFriendlySymbolName(),
			lpData->GetStartAddress());

		HTREEITEM hItem = InsertItem(strEP,2,2,m_hRoot);

		SetItemData(hItem,(DWORD_PTR)lpData);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CWSNavTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	CreateViewOfCurrentItem();
	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////

void CWSNavTree::CreateViewOfCurrentItem()
{
	HTREEITEM hItem;
	CStudioWorkspaceItemInfo* pItemInfo;

	if ((NULL == (hItem = GetSelectedItem())) ||  
		(NULL == (pItemInfo = 
		(CStudioWorkspaceItemInfo*)GetItemData(hItem))))
	{
		return;
	}

	((CStudioApp*)AfxGetApp())->CreateOrActivateViewOfWorkspaceItem(pItemInfo,pItemInfo->GetDefaultDocType());
}

///////////////////////////////////////////////////////////////////////////////