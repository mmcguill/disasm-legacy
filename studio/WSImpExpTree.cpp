///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "studio.h"
#include "WSImpExpTree.h"
#include <dbghelp.h>

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWSImpExpTree, CTreeCtrl)

///////////////////////////////////////////////////////////////////////////////

CWSImpExpTree::CWSImpExpTree()
{
	m_pDoc = NULL;
}

///////////////////////////////////////////////////////////////////////////////

CWSImpExpTree::~CWSImpExpTree()
{
	m_pDoc = NULL;
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CWSImpExpTree, CTreeCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WM_UPDATE_DOCKVIEW,OnUpdate)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

BOOL CWSImpExpTree::Create(CWnd *pParent, UINT nID)
{
	return CTreeCtrl::Create(WS_BORDER | WS_TABSTOP | TVS_SHOWSELALWAYS | 
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
		CRect(0,0,0,0),pParent,nID);
}

///////////////////////////////////////////////////////////////////////////////

int CWSImpExpTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ModifyStyleEx(0,WS_EX_CLIENTEDGE);
	
	m_il.Create(16,16,TRUE,4,0);
	m_il.Add(AfxGetApp()->LoadIcon(IDI_FOLDERC));
	m_il.Add(AfxGetApp()->LoadIcon(IDI_FOLDERO));
	m_il.Add(AfxGetApp()->LoadIcon(IDI_DOC));
	m_il.Add(AfxGetApp()->LoadIcon(IDI_ITEMERROR));
	SetImageList(&m_il,TVSIL_NORMAL);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CWSImpExpTree::OnUpdate(WPARAM wParam, LPARAM lParam)
{
	PUPDATE_WORKSPACE_PARAM pUwp = (PUPDATE_WORKSPACE_PARAM)wParam;
	ASSERT(pUwp);

	if(uwp_event_opening == pUwp->event)
	{
		ASSERT(pUwp->pDocument);
		m_pDoc = pUwp->pDocument;

		if(!FillTree())
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

void CWSImpExpTree::EmptyTree()
{
	DeleteAllItems();
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSImpExpTree::FillTree(void)
{
	ASSERT(m_pDoc);
	CPEParser* pExe = m_pDoc->GetExecutableParser();
	Assert(pExe);

	EmptyTree();

	if(!FillImports(pExe) || !FillExports(pExe) || !FillTLB(pExe))
	{
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSImpExpTree::FillImports(CPEParser* pExe)
{
	Assert(pExe);

	CString strFmt;

	PIMPORT_MODULES_VECTOR pImpModules = pExe->GetImportModulesVector();
	Assert(pImpModules);
	if(NULL == pImpModules)
	{
		return TRUE;
	}

	strFmt.Format(IDS_IMPORTROOTNAME,pImpModules->size());
	HTREEITEM hImpRoot = InsertItem(strFmt,0,1,TVI_ROOT);

	int nDelayLoad = 0;
	HTREEITEM hDelayLoadRoot = NULL;

	for(IMPORT_MODULES_VECTOR::iterator theModule = pImpModules->begin();
		theModule != pImpModules->end();theModule++)
	{
		PIMPORT_MODULE pMod = *theModule;
		Assert(pMod);

		PIMPORT_FUNCTIONS_VECTOR pImpFuncs = pMod->GetImportFunctions();
		Assert(pImpFuncs);

		if(!pImpFuncs)
		{
			continue;
		}
		
		// Like: msvcrt (512 imported funcs)

		strFmt.Format(IDS_IMPEXP_FMT_MODNAME,pMod->GetName(),pImpFuncs->size());

		if(pMod->IsDelayLoad())
		{
			if(0 == nDelayLoad)
			{
				strFmt.Format(IDS_DELAYLOADROOTNAME,nDelayLoad);
				hDelayLoadRoot = InsertItem(strFmt,0,1,TVI_ROOT);
			}

			InsertItem(pMod->GetName(),2,2,hDelayLoadRoot); // Add to Delay Load
			nDelayLoad++;

			// Update

			strFmt.Format(IDS_DELAYLOADROOTNAME,nDelayLoad);
			SetItem(hDelayLoadRoot,TVIF_TEXT,strFmt,0,1,0,0,0);
		}

		HTREEITEM hCurModRoot = InsertItem(strFmt,0,1,hImpRoot);

		strFmt.LoadString(IDS_IMPEXP_IMPBYNAME_ROOT);
		HTREEITEM hByName = InsertItem(strFmt,0,1,hCurModRoot);
		strFmt.LoadString(IDS_IMPEXP_IMPBYORD_ROOT);
		HTREEITEM hByOrd = InsertItem(strFmt,0,1,hCurModRoot);			

		// Add them in

		for(IMPORT_FUNCTIONS_VECTOR::iterator theFunc = pImpFuncs->begin();
			theFunc!= pImpFuncs->end();theFunc++)
		{
			PIMPORT_FUNCTION pFunc = *theFunc;
			Assert(pFunc);

			LPCTSTR lpszName = pFunc->GetName();
	
			// Give undecorated name aswell

			char szANSIName[512] = {0};
			TCHAR szName[512] = {0};
			if(NULL != lpszName)
			{
				LPCSTR lpszANSIName;
				#ifdef _UNICODE
					USES_CONVERSION;
					lpszANSIName = W2CA(lpszName);
				#else
					lpszANSIName = lpszName;
				#endif

				if(!UnDecorateSymbolName(lpszANSIName,szANSIName,512,UNDNAME_COMPLETE))
				{
					StringCchCopy(szName,512,lpszName);
				}
				else
				{
					#ifdef _UNICODE
						USES_CONVERSION;
						LPCTSTR tmp = A2CW(szANSIName);
						if(_tcsncmp(lpszName,tmp,512))
						{
							_sntprintf(szName,512,_T("%s => [%s]"),lpszName,tmp);
						}
						else
						{
							StringCchCopy(szName,512,lpszName);
						}
					#else
						if(_tcsncmp(lpszName,szANSIName,512))
						{
							_sntprintf(szName,512,_T("%s => [%s]"),lpszName,szANSIName);
						}
						else
						{
							StringCchCopy(szName,512,lpszName);
						}
					#endif
				}

				InsertItem(szName,2,2,hByName);
			}
			else
			{
				if(pFunc->GetOrdinal())
				{
					strFmt.Format(IDS_IMPEXP_FMT_ORDINALIMP_ONLY,pFunc->GetOrdinal());
					InsertItem(strFmt,2,2,hByOrd);
				}
				else
				{
					AssertSz(0,"WTF?");

					strFmt.LoadString(IDS_IMPEXP_IMPORT_READERROR);
					InsertItem(strFmt,3,3,hCurModRoot);
				}
			}
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSImpExpTree::FillExports(CPEParser* pExe)
{
	Assert(pExe);

	PEXPORTS_VECTOR pExports = pExe->GetExportsVector();
	if(NULL == pExports)
	{
		return TRUE;
	}

	CString strFmt;
	strFmt.Format(IDS_EXPORTROOTNAME,pExports->size());
	HTREEITEM hExpRoot = InsertItem(strFmt,0,1,TVI_ROOT);
	strFmt.LoadString(IDS_IMPEXP_EXPBYNAME_ROOT);
	HTREEITEM hExpByNameRoot = InsertItem(strFmt,0,1,hExpRoot);
	strFmt.LoadString(IDS_IMPEXP_EXPBYORD_ROOT);
	HTREEITEM hExpByOrdRoot = InsertItem(strFmt,0,1,hExpRoot);

	for(EXPORTS_VECTOR::iterator theExp = pExports->begin();
		theExp!=pExports->end();theExp++)
	{
		PEXPORT_FUNCTION pFunc = *theExp;
		Assert(pFunc);

		LPCTSTR lpszName = pFunc->GetName();
		HTREEITEM hInsert = 
			(NULL == lpszName) ? hExpByOrdRoot : hExpByNameRoot;

		// Give undecorated name aswell

		char szANSIName[512] = {0};
		TCHAR szName[512] = {0};
		
		if(NULL != lpszName)
		{
			LPCSTR lpszANSIName;
			#ifdef _UNICODE
				USES_CONVERSION;
				lpszANSIName = W2CA(lpszName);
			#else
				lpszANSIName = lpszName;
			#endif

			if(!UnDecorateSymbolName(lpszANSIName,szANSIName,512,UNDNAME_COMPLETE))
			{
				StringCchCopy(szName,512,lpszName);
			}
			else
			{
				#ifdef _UNICODE
					USES_CONVERSION;
					LPCTSTR tmp = A2CW(szANSIName);
					if(_tcsncmp(lpszName,tmp,512))
					{
						_sntprintf(szName,512,_T("%s => [%s]"),lpszName,A2CW(szANSIName));
					}
					else
					{
						StringCchCopy(szName,512,lpszName);
					}
				#else
					if(_tcsncmp(lpszName,szANSIName,512))
					{
						_sntprintf(szName,512,_T("%s => [%s]"),lpszName,szANSIName);
					}
					else
					{
						StringCchCopy(szName,512,lpszName);
					}
				#endif
			}
		}

		if(pFunc->IsForwarder())
		{
			strFmt.Format(IDS_FORMAT_IMPEXP_EXP_FORWARDER,szName,
				pFunc->GetOrdinal(),pFunc->GetForwarder());
		}
		else
		{
			strFmt.Format(IDS_FORMAT_IMPEXP_EXP_NORMAL,szName,
				pFunc->GetOrdinal());
		}

		InsertItem(strFmt,2,2,hInsert);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CWSImpExpTree::FillTLB(CPEParser* pExe)
{
	Assert(pExe);

	LPWSTR lpszFile = NULL;

#ifdef _UNICODE
	lpszFile = (LPWSTR)pExe->GetFileName();
#else
	size_t cch = 0;
	if(FAILED(StringCchLengthA(pExe->GetFileName(),MAX_PATH,&cch)))
	{
		return FALSE;
	}

	lpszFile = new WCHAR[cch+1];
	Assert(lpszFile);
	if(NULL == lpszFile)
	{
		// Go Mad
		return FALSE;
	}

	if(!MultiByteToWideChar(CP_ACP,0,pExe->GetFileName(),cch+1,lpszFile,cch+1))
	{
#ifndef _UNICODE
    delete lpszFile;
#endif

		return FALSE;
	}

#endif

	// TODO: Multiple Type Library resources in a file??

	ITypeLib* pitl;
	if(FAILED(LoadTypeLibEx(lpszFile,REGKIND_NONE,&pitl)))
	{
#ifndef _UNICODE
    delete lpszFile;
#endif
		return FALSE;
	}
	
#ifndef _UNICODE
    delete lpszFile;
#endif

	Assert(pitl);

	CString strFmt;
	strFmt.LoadString(IDS_IMPEXP_TLB_ROOT);
	HTREEITEM hTlbRoot = InsertItem(strFmt,0,1,TVI_ROOT);

	TLIBATTR* pTLibAttr;
	if(FAILED(pitl->GetLibAttr(&pTLibAttr)))
	{
		return FALSE;
	}

	WCHAR szGuid[64];
	StringFromGUID2(pTLibAttr->guid,szGuid,64);
	HTREEITEM hGuidRoot = NULL;
	
	{
		USES_CONVERSION;
		hGuidRoot = InsertItem(OLE2CT(szGuid),0,1,hTlbRoot);
	}
	
	//pTLibAttr->lcid;
	//pTLibAttr->syskind;
	//pTLibAttr->wLibFlags;
	//pTLibAttr->wMajorVerNum;
	//pTLibAttr->wMinorVerNum;

	pitl->ReleaseTLibAttr(pTLibAttr);

   	UINT cTypes = pitl->GetTypeInfoCount();
	for(UINT i=0;i<cTypes;i++)
	{
		ITypeInfo* pTInfo;
		if(FAILED(pitl->GetTypeInfo(i,&pTInfo)))
		{
			AssertSz(0,"Error Getting Type Info");
			continue;
		}

		Assert(pTInfo);

		TYPEATTR* pTypeAttr;
		if(FAILED(pTInfo->GetTypeAttr(&pTypeAttr)))
		{
			AssertSz(0,"Error Getting Type Info");
			pTInfo->Release();
			continue;
		}

		StringFromGUID2(pTypeAttr->guid,szGuid,64);

		//

		LPCTSTR lpszTypeType = NULL;
		switch(pTypeAttr->typekind)
		{
		case TKIND_ENUM:
			lpszTypeType = TEXT("Enum");
			break;
		case TKIND_RECORD:
			lpszTypeType = TEXT("Struct");
			break;
		case TKIND_MODULE:
			lpszTypeType = TEXT("Module");
			break;
		case TKIND_INTERFACE:
			lpszTypeType = TEXT("Interface");
            break;
		case TKIND_DISPATCH:
			lpszTypeType = TEXT("Interface (Dispatch)");
			break;
		case TKIND_COCLASS:
			lpszTypeType = TEXT("CoClass");
			break;
		case TKIND_ALIAS:
			lpszTypeType = TEXT("Alias");
			break;
		case TKIND_UNION:
			lpszTypeType = TEXT("Union");
			break;
		case TKIND_MAX:
			lpszTypeType = TEXT("End Of Enum");
			break;
		default:
			lpszTypeType = TEXT("<Unknown>");
		}

		HTREEITEM hTypeRoot;
		{
			USES_CONVERSION;	
			strFmt.Format(TEXT("%s: %s"),lpszTypeType,OLE2CT(szGuid));
		}

		hTypeRoot = InsertItem(strFmt,0,1,hGuidRoot);

		if(TKIND_COCLASS == pTypeAttr->typekind)
		{
			BSTR bstrNames[5];
			UINT cNames = 0;
			pTInfo->GetNames(i,bstrNames,5,&cNames);
			pTInfo->Release();
			continue;
		}

		
		//// ImplType

		//UINT cImplTypes = pTypeAttr->cImplTypes;
		//HTREEITEM hImplTypeRoot = InsertItem(TEXT("Impl Types"),0,1,hTypeRoot);

		//for(UINT j=0;j<cImplTypes;j++)
		//{
		//	pTInfo->GetImplTypeFlags(
		//}
		
		// Variable Descriptions

		UINT cVars = pTypeAttr->cVars;
		HTREEITEM hVarRoot = InsertItem(TEXT("Variables"),0,1,hTypeRoot);

		for(UINT j=0;j<cVars;j++)
		{
			VARDESC* pVarDesc;
			if(FAILED(pTInfo->GetVarDesc(j,&pVarDesc)))
			{
                continue;
			}

			InsertItem(TEXT("< Placeholder >"),2,2,hVarRoot);

			pTInfo->ReleaseVarDesc(pVarDesc);
		}

		// Function Descriptions

		UINT cFuncs = pTypeAttr->cFuncs;
		HTREEITEM hFuncRoot = InsertItem(TEXT("Functions"),0,1,hTypeRoot);

		for(UINT j=0;j<cFuncs;j++)
		{
			FUNCDESC* pFuncDesc;
			if(FAILED(pTInfo->GetFuncDesc(j,&pFuncDesc)))
			{
                continue;
			}

			// Try Get Function Name
			
			UINT cNames = pFuncDesc->cParams + 1;
			BSTR* pBstrNames = new BSTR[cNames];
			Assert(pBstrNames);
			if(NULL == pBstrNames)
			{
				pTInfo->ReleaseFuncDesc(pFuncDesc);
				continue;
			}


			UINT cNamesRetr = 0;
			if(FAILED(pTInfo->GetNames(pFuncDesc->memid,pBstrNames,cNames,&cNamesRetr)))
			{
				delete pBstrNames;
				pTInfo->ReleaseFuncDesc(pFuncDesc);
				continue;
			}

			for(UINT k=0;k<cNamesRetr;k++)
			{
				USES_CONVERSION;
				if(0==k)
				{
					strFmt = OLE2CT((LPCWSTR)pBstrNames[k]);
					strFmt += TEXT("(");
				}
				else
				{
					if(k != 1)
					{
						strFmt += TEXT(", ");
					}
					
					strFmt += OLE2CT((LPCWSTR)pBstrNames[k]);
				}
				SysFreeString(pBstrNames[k]);
			}

			strFmt += TEXT(")");

			delete pBstrNames;

			pTInfo->ReleaseFuncDesc(pFuncDesc);

			InsertItem(strFmt,2,2,hFuncRoot);
		}

		pTInfo->ReleaseTypeAttr(pTypeAttr);
		pTInfo->Release();
	}

	pitl->Release();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CWSImpExpTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	GetDetailsOfCurrentItem();
	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////

void CWSImpExpTree::GetDetailsOfCurrentItem(void)
{
	/* ******************************************************
	Item Data is used to track the corresponding functions so
	when its double clicked we can quickly locate that particular
	function in the file. This is the format

	1. Item must not have any children, ie be a leaf node
	2. If its an export the Low word is 0xFFFF and HI word is function index
	3. if its an import the low word is module and hi is function
	4. if error all Item Data = 0xFFFFFFFF
	*********************************************************/
	HTREEITEM hSel = GetSelectedItem();
	if(!hSel) return; // Nothing Selected!
	if(ItemHasChildren(hSel)) return; // Not Leaf Node

	DWORD dwData = (DWORD)GetItemData(hSel);
	if(dwData == 0xFFFFFFFF) return; //Error Item


	// Import or Export?
	//CDialog dlg(IDD_FUNCTION_PROPS,NULL);
	if(LOWORD(dwData) == 0xFFFF){ //Export
	//	dlg.SetDlgItemText(IDC_EDPROPERTIES,"Export");
	}
	else{ // Import
	//	dlg.SetDlgItemText(IDC_EDPROPERTIES,"Import");
	}
	//dlg.DoModal();
}

///////////////////////////////////////////////////////////////////////////////