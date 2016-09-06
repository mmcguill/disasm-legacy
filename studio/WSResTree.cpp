///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "studio.h"
#include "WSResTree.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CWSResTree

IMPLEMENT_DYNCREATE(CWSResTree,CTreeCtrl)

///////////////////////////////////////////////////////////////////////////////

#define WORKSPACE_RESOURCES_UNKNOWN_TYPE_UNIQUE_ID_FLAG 0x1000

///////////////////////////////////////////////////////////////////////////////

typedef struct _ResourceTypeToStudioDocTypeEntry
{
	LPTSTR ResourceType;
	StudioMinorDocType type;
}ResourceTypeToStudioDocTypeEntry;

static ResourceTypeToStudioDocTypeEntry rgResourceToStudioMap[] = 
{
	{0,					StudioDocTypeHex}, // Unknown
	{RT_CURSOR,			StudioDocTypeIcon},
	{RT_BITMAP,			StudioDocTypeBitmap},
	{RT_ICON,			StudioDocTypeIcon},
	{RT_MENU,			StudioDocTypeMenu},
	{RT_DIALOG,			StudioDocTypeDialog},
	{RT_STRING,			StudioDocTypeStringTable},
	{RT_FONTDIR,		StudioDocTypeHex},
	{RT_FONT,			StudioDocTypeHex},
	{RT_ACCELERATOR,	StudioDocTypeAccelerator},
	{RT_RCDATA,			StudioDocTypeHex},
	{RT_MESSAGETABLE,	StudioDocTypeHex},
	{RT_GROUP_CURSOR,	StudioDocTypeHex},
	{RT_GROUP_ICON,		StudioDocTypeHex},
	{RT_VERSION,		StudioDocTypeVersion},
	{RT_DLGINCLUDE,		StudioDocTypeHex},
	{RT_PLUGPLAY,		StudioDocTypeHex},
	{RT_VXD,			StudioDocTypeHex},
	{RT_ANICURSOR,		StudioDocTypeHex},
	{RT_ANIICON,		StudioDocTypeHex},
	{RT_HTML,			StudioDocTypeHex},
	{RT_MANIFEST,		StudioDocTypeHex},
};

#define NUMBER_OF_RESOURCE_TO_STUDIO_MAP_ENTRIES \
	(sizeof(rgResourceToStudioMap) / sizeof(ResourceTypeToStudioDocTypeEntry))

///////////////////////////////////////////////////////////////////////////////

/*static*/StudioMinorDocType CWSResTree::GetStudioDocTypeFromResourceType(LPTSTR lpwType)
{
	if(IS_INTRESOURCE(lpwType)) // Ignore non integer resources (custom resources)
	{
		for(int i=0;i<NUMBER_OF_RESOURCE_TO_STUDIO_MAP_ENTRIES;i++)
		{
			if(rgResourceToStudioMap[i].ResourceType  == lpwType)
			{
				return rgResourceToStudioMap[i].type;
			}
		}

		// Missing a map entry?

		TRACE(traceAppMsg,0,
			"WARNING: Missing Resource Map Entry for Res ID: %d\n",lpwType);
	}

	return StudioDocTypeUnknown; // Fail Safe
}

///////////////////////////////////////////////////////////////////////////////

CWSResTree::CWSResTree()
{
	m_hRoot = TVI_ROOT;
	m_bViewRaw = FALSE;
	m_pDoc = NULL;
}

///////////////////////////////////////////////////////////////////////////////

CWSResTree::~CWSResTree()
{
	Cleanup();
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::Cleanup()
{
	for(vector<CStudioWorkspaceResourceItemInfo*>::iterator theItem = m_vecItems.begin();
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

BEGIN_MESSAGE_MAP(CWSResTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CWSResTree)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATE_DOCKVIEW,OnUpdate)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclk)
	ON_COMMAND(ID_CTXT_VIEW,OnViewStd)
	ON_COMMAND(ID_CTXT_VIEWHEX,OnViewHex)
	ON_COMMAND(ID_CTXT_VIEWWITH, OnCtxtViewwith)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

BOOL CWSResTree::Create(CWnd *pParent, UINT nID)
{
	return CTreeCtrl::Create(WS_BORDER|WS_TABSTOP|TVS_SHOWSELALWAYS|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS,CRect(0,0,0,0),pParent,nID);
}

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CWSResTree message handlers
int CWSResTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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
	str.LoadString(IDR_WORKSPACE_RESOURCES);
	m_hRoot = InsertItem(str,0,1);
	SetItemData(m_hRoot,0);
	return 0;
}

/////////////////////////////////////////////////////////////////////

LRESULT CWSResTree::OnUpdate(WPARAM wParam, LPARAM lParam)
{
	PUPDATE_WORKSPACE_PARAM pUwp = (PUPDATE_WORKSPACE_PARAM)wParam;

	if(uwp_event_opening == pUwp->event)
	{
		m_pDoc = pUwp->pDocument;

		ASSERT(m_pDoc);
		if(NULL == m_pDoc)
		{
			return 0;
		}

		if(!FillTree())
		{
			DeleteAllItems();
			Cleanup();
			m_pDoc = NULL;

			// TODO: Raise some fuss here please
			ASSERT(FALSE);
		}
	}
	else
	{
		DeleteAllItems();
		Cleanup();
		m_pDoc = NULL;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////

BOOL CWSResTree::FillTree()
{
	DeleteAllItems();
	Cleanup();	// Deletes all item data structures associated 
				// with previous tree

	CString str;
	str.LoadString(IDR_WORKSPACE_RESOURCES);
	m_hRoot = InsertItem(str,0,1);
	SetItemData(m_hRoot,0);

	//CPERes* pPE = __AppGetPEFile();
	
	ASSERT(m_pDoc);
	CPERes* pPE = m_pDoc->GetResourceParser();
	ASSERT(pPE);
	if(NULL == pPE)
	{
		return FALSE;
	}

	//if(!pPE->IsValidObj())
	//{
 //       return 0;
	//}

	DWORD dwRoot = pPE->ResGetRootDir();
	if(!dwRoot)
	{
		// No Resources
		return TRUE;
	}

	DWORD dwNumObj, dwCurObj;
	if(!pPE->ResGetNumObj(dwRoot,&dwNumObj))
	{
		// TODO: Should this asser tbe here, what if no res's?
		ASSERT(FALSE);
		return FALSE;
	}

	for(DWORD x = 0;x<dwNumObj;x++)
	{

		if(!pPE->ResGetDirObj(dwRoot,x,&dwCurObj))
		{
			// TODO: Should this asser tbe here, what if no res's?
			ASSERT(FALSE);
			return FALSE;
		}

		if(!pPE->ResIsObjDir(dwRoot,x))
		{
			// TODO: Should this asser tbe here, what if no res's?
			ASSERT(FALSE);
			return FALSE;
		}
		
		BOOL fTypeIsString = FALSE;
		LPTSTR lpszType = NULL;
		if(!pPE->ResGetObjID(dwRoot,x,(DWORD*)&lpszType))
		{
			// Type Maybe A String
			
			lpszType = new TCHAR[MAX_PATH];
			ASSERT(lpszType);

			if(NULL == lpszType)
			{
				return FALSE;
			}

			if(!pPE->ResGetObjName(dwRoot,x,lpszType))
			{
				delete lpszType;
				ASSERT(FALSE);
				return FALSE;
			}

			fTypeIsString = TRUE;
		}

		if(m_bViewRaw)
		{
			FillStdSubTree(dwRoot, lpszType, fTypeIsString, x); //Just Fill
		}
		else if(lpszType != RT_ICON && lpszType != RT_CURSOR)
		{
			if(lpszType == RT_GROUP_ICON) 
			{
				lpszType = RT_ICON;
			}
			if(lpszType == RT_GROUP_CURSOR) 
			{
				lpszType = RT_CURSOR;
			}
			
			if(lpszType == RT_STRING)
			{
				FillStringSubTree(dwRoot, x);
			}
			else 
			{
				if(!FillStdSubTree(dwRoot, lpszType, fTypeIsString ,x))
				{
					return FALSE;
				}
			}
		}

		if(fTypeIsString)
		{
			ASSERT(lpszType);
			delete[] lpszType;
			lpszType = NULL;
		}
	}

	// Sort
	SortChildren(m_hRoot);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSResTree::FillStdSubTree(DWORD dwRoot, LPTSTR lpszType, BOOL fTypeIsString,DWORD dwIndex)
{
	ASSERT(dwRoot);
	ASSERT(lpszType);

	if(0 == dwRoot || NULL == lpszType)
	{
		return FALSE;
	}

    //CPERes* pPE = __AppGetPEFile();
	//ASSERT(pPE);
	//if(!pPE->IsValidObj())
	//{
	//	return FALSE;
	//}
	
	ASSERT(m_pDoc);
	CPERes* pPE = m_pDoc->GetResourceParser();
	ASSERT(pPE);
	if(NULL == pPE)
	{
		return FALSE;
	}


	// Resource Type First
	
	TCHAR strType[MAX_PATH];
	
	if(!fTypeIsString)
	{
		// TODO: Remove these casts when pefile is cleaned up
		if(pPE->ResGetTypeStringFromID((DWORD)(DWORD_PTR)lpszType,strType))
		{
			// TODO: Figure out what to do here, probably just fall through?
			//ASSERT(FALSE);
        }
	}
	else
	{
		CString	str;
		str.Format(IDS_FORMAT_RESTREE_RESIDSTRING,lpszType);
		lstrcpyn(strType,str,MAX_PATH);
	}

	HTREEITEM hObjRoot = InsertItem(strType,0,1,m_hRoot);
	SetItemData(hObjRoot,0);
	
	// Get Items

	DWORD dwDir, dwNumObjs;
	if(!pPE->ResGetDirObj(dwRoot,dwIndex,&dwDir))
	{
		// TODO: Should this fail everything?
		ASSERT(FALSE);
		return FALSE;
	}

	if(!pPE->ResGetNumObj(dwDir,&dwNumObjs))
	{
		// TODO: Should this fail everything?
		ASSERT(FALSE);
		return FALSE;
	}

	DWORD dwID = 0;
	CString strRes, strLang;

	UINT nUniqueId = 0;
	for(DWORD x=0;x<dwNumObjs;x++)
	{
		TCHAR strResName[MAX_PATH];

		if( !pPE->ResGetObjID(dwDir,x,&dwID) &&
            !pPE->ResGetObjName(dwDir,x,strResName))
		{
			// TODO: Should this fail everything?
			ASSERT(FALSE);
			return FALSE;
		}
		
		DWORD dwLangDir, dwNumLang;
		if( !pPE->ResGetDirObj(dwDir,x,&dwLangDir) ||
			!pPE->ResGetNumObj(dwLangDir,&dwNumLang))
		{
			// TODO: Should this fail everything?
			ASSERT(FALSE);
			return FALSE;
		}

		// Add Item With its Language to tree

		for(DWORD y=0;y<dwNumLang;y++)
		{
			// Get Language String

			TCHAR strLang[MAX_PATH];
			DWORD dwLang;

			if( !pPE->ResGetObjID(dwLangDir,y,&dwLang) || 
                !pPE->ResGetLangStringFromID(dwLang,strLang))
			{
				// TODO: Should this fail everything?
				ASSERT(FALSE);
				return FALSE;
			}
			
			
			// Named or ID'd?

			if(dwID) 
			{
				strRes.Format(IDS_FORMAT_RESTREE_RESOBJ,dwID,strLang);
			}
			else
			{
				strRes.Format(IDS_FORMAT_RESTREE_RESNAMEDOBJ,strResName,strLang);
			}

			HTREEITEM hRes = InsertItem(strRes,2,2,hObjRoot);
			nUniqueId++;

            // Store Data for this item & Set Tree Item Data

			CStudioWorkspaceResourceItemInfo* lpData = new 
				CStudioWorkspaceResourceItemInfo(m_pDoc,
					StudioMajorTypeResource,
					GetStudioDocTypeFromResourceType(lpszType),
					MAKELONG(dwIndex,nUniqueId),
					dwIndex,
					dwID,
					dwLang,
					((dwID == 0) ? (LPCTSTR)strResName : NULL),
					strType);

			ASSERT(lpData);
			if(NULL == lpData)
			{
				// TODO: What do we do with out Of Mem errors?
				return FALSE;
			}

			m_vecItems.push_back(lpData);
			SetItemData(hRes,(DWORD_PTR)lpData);
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

// This inserts a pseudo string entry, document handles things from when its
// opened. looks Good.

BOOL CWSResTree::FillStringSubTree(DWORD dwRoot,DWORD dwIndex)
{
	//CPERes* pPE = __AppGetPEFile();
	
	//if(!pPE->IsValidObj())
	//{
	//	return FALSE;
	//}
	
	ASSERT(m_pDoc);
	CPERes* pPE = m_pDoc->GetResourceParser();
	ASSERT(pPE);
	if(NULL == pPE)
	{
		return FALSE;
	}

	// Type

	TCHAR strType[MAX_PATH];
	if(!pPE->ResGetTypeStringFromID((DWORD_PTR)RT_STRING,strType))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	HTREEITEM hStringRoot = InsertItem(strType,0,1,m_hRoot);
	ASSERT(hStringRoot);

    SetItemData(hStringRoot,0);
	
	// String Block
	
	DWORD dwIDDir, dwNumID, dwID;
	if( !pPE->ResGetDirObj(dwRoot,dwIndex,&dwIDDir) || 
		!pPE->ResGetNumObj(dwIDDir,&dwNumID))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	UINT nUniqueId = 0;
	vector<DWORD> vecLanguagesUsed;
	for(DWORD x=0;x<dwNumID;x++)
	{
		// Get Base ID
		
		pPE->ResGetObjID(dwIDDir,x,&dwID);
		
		// Get Language & only insert one psuedo entry per language
		// if we already have a pseudo entry for this language, skip
		
		DWORD dwLangDir, dwNumLang;
		pPE->ResGetDirObj(dwIDDir,x,&dwLangDir);
		pPE->ResGetNumObj(dwLangDir,&dwNumLang);

		for(DWORD y=0;y<dwNumLang;y++)
		{
			DWORD dwLang;
			pPE->ResGetObjID(dwLangDir,y,&dwLang);

			// Ensure its a unique language

			BOOL fLanguageAlreadyUsed = FALSE;
			for(vector<DWORD>::iterator theLang = vecLanguagesUsed.begin();
				theLang != vecLanguagesUsed.end();theLang++)
			{
				if((*theLang) == dwLang)
				{
					fLanguageAlreadyUsed = TRUE;
					break;
				}
			}
			if(fLanguageAlreadyUsed)
			{
				continue; // Next Language Please
			}

			
			// Get String For this language

			TCHAR strLang[MAX_PATH];
			CString strString;
			pPE->ResGetLangStringFromID(dwLang,strLang);
			strString.Format(IDS_FORMAT_RESTREE_STRINGTABLE,strLang);

			HTREEITEM hRes = InsertItem(strString,2,2,hStringRoot);
			nUniqueId++;
			ASSERT(hRes);

			// Store Data for this item & Set Tree Item Data

			CStudioWorkspaceResourceItemInfo* lpData = new 
				CStudioWorkspaceResourceItemInfo(m_pDoc,
					StudioMajorTypeResource,
					GetStudioDocTypeFromResourceType(RT_STRING),
					MAKELONG(dwIndex,nUniqueId),
					dwIndex,
					dwID,
					dwLang,
					NULL,
					strType);

			ASSERT(lpData);
			if(NULL == lpData)
			{
				// TODO: What do we do with out Of Mem errors?
                return FALSE;
			}

			vecLanguagesUsed.push_back(dwLang);
			m_vecItems.push_back(lpData);
			SetItemData(hRes,(DWORD_PTR)lpData);
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CreateViewOfCurrentItem(CWSResTree::ResourceViewAsTypeDefault);
	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::CreateViewOfCurrentItem(CWSResTree::ResourceViewAsType rvat)
{
	HTREEITEM hItem;
	CStudioWorkspaceResourceItemInfo* pItemInfo;

	if ((NULL == (hItem = GetSelectedItem())) ||  
		(NULL == (pItemInfo = 
		(CStudioWorkspaceResourceItemInfo*)GetItemData(hItem))))
	{
		return;
	}

	// EXCEPTION: For String Table with raw resources off:
	// Do not allow any view except default string table one
	// as this is a pseudo entry and view will not be complete
	// or consistent with rest of app
	// FUTURE: Show all string blocks when this happens

	if(!m_bViewRaw && 
		StudioDocTypeStringTable == pItemInfo->GetDefaultDocType()->minor &&
		rvat != CWSResTree::ResourceViewAsTypeDefault)
	{
		// TODO: Nice Message Here Instead
		AfxMessageBox(IDS_RESOURCE_NO_NONDEFAULT_STRINGTABLE_VIEW);
		return;
	}

    // If Raw Resources On always View With HexViewer

	if(m_bViewRaw)
	{
		rvat = CWSResTree::ResourceViewAsTypeHex;
	}

	// Work Out What Kind of Document we need

	STUDIODOCTYPE sdtTargetDoc;
	sdtTargetDoc.major = StudioMajorTypeResource;
	
	switch(rvat)
	{
	case CWSResTree::ResourceViewAsTypeHex:
		sdtTargetDoc.minor = StudioDocTypeHex;
		break;
	case CWSResTree::ResourceViewAsTypeDefault:
		sdtTargetDoc.minor = pItemInfo->GetDefaultDocType()->minor;
		break;
	case CWSResTree::ResourceViewAsTypeUserChoose:
		sdtTargetDoc.minor = StudioDocTypeUnknown;
		break;
	}

	((CStudioApp*)AfxGetApp())->CreateOrActivateViewOfWorkspaceItem(pItemInfo,&sdtTargetDoc);
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::SetViewRaw(BOOL bRaw)
{
	m_bViewRaw = bRaw;
    FillTree();
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSResTree::GetViewRaw()
{
	return m_bViewRaw;
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::OnViewHex()
{
	CreateViewOfCurrentItem(ResourceViewAsTypeHex);
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::OnViewStd()
{
	CreateViewOfCurrentItem(ResourceViewAsTypeDefault);
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::OnCtxtViewwith()
{
	CreateViewOfCurrentItem(ResourceViewAsTypeUserChoose);
}

///////////////////////////////////////////////////////////////////////////////

void CWSResTree::OnRclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	DWORD dwP = GetMessagePos();
	CPoint pt(LOWORD(dwP),HIWORD(dwP));
	ScreenToClient(&pt);
	UINT flags;
	HTREEITEM hSel = HitTest(pt,&flags);

	CStudioWorkspaceResourceItemInfo* pItemInfo;

	if(NULL == (pItemInfo = (CStudioWorkspaceResourceItemInfo*)GetItemData(hSel)))
	{
		return;
	}

	if(hSel)
	{
		SelectItem(hSel);
		CMenu mnu;
		if(!mnu.LoadMenu(IDM_CTXT_RESTREEVIEW))
		{
			return;
		}

		CMenu* pCtxt = mnu.GetSubMenu(0);
		ASSERT(pCtxt);
		
		ClientToScreen(&pt);
		pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,LOWORD(dwP),HIWORD(dwP),this);
	}

	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////
