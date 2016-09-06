/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "ResourcesDoc.h"
#include ".\resourcesdoc.h"

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CResBaseDoc, CWorkspaceItemBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CResBaseDoc::CResBaseDoc()
{
	m_pData = NULL;
	m_dwSize = 0;
	m_pItemInfo = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CResBaseDoc::~CResBaseDoc()
{
	ASSERT(m_pItemInfo); // Shouldnt have doc without this!

	if(m_pItemInfo)
	{
		delete m_pItemInfo;
	}

	if(m_pData) 
	{
		delete[] m_pData;
	}
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CResBaseDoc, CWorkspaceItemBaseDoc)
	//{{AFX_MSG_MAP(CResBaseDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_CTXT_VIEWHEX, OnCtxtViewhex)
	ON_COMMAND(ID_CTXT_VIEWWITH, OnCtxtViewwith)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CResBaseDoc::OnCtxtViewhex()
{
	STUDIODOCTYPE sdt;
	sdt.major = StudioMajorTypeResource;
	sdt.minor = StudioDocTypeHex;

	((CStudioApp*)AfxGetApp())->
		CreateOrActivateViewOfWorkspaceItem(m_pItemInfo,&sdt);
}

///////////////////////////////////////////////////////////////////////////////

void CResBaseDoc::OnCtxtViewwith()
{
	STUDIODOCTYPE sdt;
	sdt.major = StudioMajorTypeResource;
	sdt.minor = StudioDocTypeUnknown; // This will invoke view chooser

	((CStudioApp*)AfxGetApp())->
		CreateOrActivateViewOfWorkspaceItem(m_pItemInfo,&sdt);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CResBaseDoc::SetWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo)
{
	CStudioWorkspaceResourceItemInfo* pInfo = 
		(CStudioWorkspaceResourceItemInfo*)pItemInfo;

	// Make Local Copy of Item Info

	m_pItemInfo = new CStudioWorkspaceResourceItemInfo(pInfo);
	ASSERT(m_pItemInfo);
		
	// Initialize using this data 

	return InitDoc();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CResBaseDoc::InitDoc()
{
	ASSERT(m_pItemInfo);

	if(!m_pItemInfo)
	{
		return FALSE;
	}

	// TODO: Clear previous - shouldn't happen but be safe

	if(m_pData){
		delete[] m_pData;
		m_pData = NULL;
		m_dwSize = 0;
	}

	CPERes *pPE = m_pItemInfo->GetWorkspaceDoc()->GetResourceParser();
	ASSERT(pPE);

	DWORD dwRoot = pPE->ResGetRootDir();
	if(!dwRoot)
	{
		return FALSE;
	}

	DWORD dwTypeDir;
	pPE->ResGetDirObj(dwRoot,m_pItemInfo->GetPEIndex(),&dwTypeDir);

	//Now Find obj with specified ID or Name
	DWORD dwNumIDrName, dwID;
	CString strName;
	BOOL bFound = FALSE;

	pPE->ResGetNumObj(dwTypeDir,&dwNumIDrName);
	for(DWORD x=0;x<dwNumIDrName && !bFound;x++)
	{
		if(m_pItemInfo->GetID())
		{
			pPE->ResGetObjID(dwTypeDir,x,&dwID);
			if(m_pItemInfo->GetID() == dwID) bFound = TRUE;
		}
		else
		{ //Named
			if(NULL == m_pItemInfo->GetResourceName())
			{
				ASSERT(FALSE);
				return FALSE;
			}

			TCHAR szTmp[MAX_PATH] = {0};
			pPE->ResGetObjName(dwTypeDir,x,szTmp);
			strName = szTmp;

			if((*(m_pItemInfo->GetResourceName())) == strName)
			{
				bFound = TRUE;
			}
		}
	}

	if(!bFound) 
	{
		return FALSE;
	}

	// decrement x
	x--;

	// Now Find Language
	bFound = FALSE;
	DWORD dwIDDir, dwLang, dwNumLangs;
	pPE->ResGetDirObj(dwTypeDir,x,&dwIDDir);
	pPE->ResGetNumObj(dwIDDir,&dwNumLangs);
	
	for(DWORD y=0;y<dwNumLangs && !bFound;y++)
	{
		pPE->ResGetObjID(dwIDDir,y,&dwLang);
		if(m_pItemInfo->GetLanguage() == dwLang) bFound = TRUE;
	}
	
	if(!bFound)
	{
		return FALSE;
	}

	y--;

	pPE->ResGetObjDataSize(dwIDDir,y,&m_dwSize);
	m_pData = new unsigned char[m_dwSize];
	pPE->ResGetObjData(dwIDDir,y,m_pData);



	// Give Me A 'Nice' Title

	if(!SetNiceTitle())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CStudioWorkspaceItemInfo* CResBaseDoc::GetWorkspaceItemInfo() const
{
	return (CStudioWorkspaceItemInfo*)m_pItemInfo;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CResBaseDoc::SetNiceTitle()
{
	// Find The Default Doc Template For this item and get string for type

	BOOL bFound = FALSE;
	
	Assert(m_pItemInfo);

	CPERes *pPE = m_pItemInfo->GetWorkspaceDoc()->GetResourceParser();
	ASSERT(pPE);
	
	CStudioDocTemplate* pCurDT;
	POSITION p = AfxGetApp()->GetFirstDocTemplatePosition();

	if(p == NULL)
	{
		ASSERT(FALSE); // Serious Problem if this happens
		return FALSE;
	}

	do
	{
		pCurDT = static_cast<CStudioDocTemplate*>(AfxGetApp()->GetNextDocTemplate(p));
		ASSERT(pCurDT);

		if(m_pItemInfo->GetDefaultDocType()->minor == pCurDT->GetStudioDocType()->minor)
		{
			bFound = TRUE;
			break; // Got Template, Bail Out
		}
	}while(NULL != p);

	if(!bFound && m_pItemInfo->GetDefaultDocType()->minor != StudioDocTypeUnknown)
	{
		ASSERT(FALSE); // Couldnt find a template for this item
		return FALSE;
	}


	//CString strType;
	//
	//if(m_pItemInfo->GetDefaultDocType()->minor != StudioDocTypeUnknown)
	//{
	//	pCurDT->GetDocString(strType,CDocTemplate::windowTitle);
	//}
	//else
	//{
	//	strType.LoadString(IDS_RESOURCE_TYPE_UNKNOWN);
	//}

	CString strID;
	TCHAR szLang[256];
	pPE->ResGetLangStringFromID(m_pItemInfo->GetLanguage(),szLang);
	
	if(0 == m_pItemInfo->GetID())
	{
		ASSERT(m_pItemInfo->GetResourceName());
		if(NULL == m_pItemInfo->GetResourceName())
		{
			return FALSE;
		}

		strID = *m_pItemInfo->GetResourceName();
	}
	else
	{
		strID.Format(_T("0x%X (%d)"),m_pItemInfo->GetID(),m_pItemInfo->GetID());
	}

	CString strTitle;

	ASSERT(m_pItemInfo->GetResourceTypeString());

	strTitle.Format(IDS_FORMAT_RESDOCTITLE,
		*m_pItemInfo->GetResourceTypeString(),
		strID,
		szLang);

	SetTitle(strTitle);	

	return TRUE;	
}

/////////////////////////////////////////////////////////////////////////////

void CResBaseDoc::OnCloseDocument() 
{
	if(m_pData)
	{
		delete[] m_pData;
		m_pData = NULL;
	}
	m_dwSize = 0;
	
	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStringTableDoc

IMPLEMENT_DYNCREATE(CStringTableDoc,CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CStringTableDoc::CStringTableDoc()
{
	m_pFirst = NULL;
	m_dwStrCount = 0;
}

/////////////////////////////////////////////////////////////////////////////

CStringTableDoc::~CStringTableDoc()
{
	if(m_pFirst) DeleteAllStrings(m_pFirst);
}

/////////////////////////////////////////////////////////////////////////////

void CStringTableDoc::DeleteAllStrings(STRING_TABLE_ITEM* pEntry)
{
	if(pEntry->pNext) DeleteAllStrings(pEntry->pNext);
	delete pEntry;
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStringTableDoc,CResBaseDoc)
	//{{AFX_MSG_MAP(CStringTableDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStringTableDoc diagnostics

#ifdef _DEBUG
void CStringTableDoc::AssertValid() const
{
	CResBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CStringTableDoc::Dump(CDumpContext& dc) const
{
	CResBaseDoc::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////

BOOL CStringTableDoc::SetNiceTitle()
{
	ASSERT(m_pItemInfo);
	CPERes *pPE = m_pItemInfo->GetWorkspaceDoc()->GetResourceParser();

	ASSERT(pPE);

	// TODO: Get a proper string length for this

	TCHAR szLang[256];
	if(!pPE->ResGetLangStringFromID(m_pItemInfo->GetLanguage(),szLang))
	{
		ASSERT(FALSE);	
		return FALSE;
	}

	CString strTitle;
	strTitle.Format(IDS_FORMAT_RESTREE_STRINGTABLE,szLang);
	SetTitle(strTitle);	

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////

BOOL CStringTableDoc::InitDoc()
{
	ASSERT(m_pItemInfo);
	CPERes *pPE = m_pItemInfo->GetWorkspaceDoc()->GetResourceParser();

	ASSERT(pPE);

	DWORD dwRoot = pPE->ResGetRootDir();

	DWORD dwStringDir;
	pPE->ResGetDirObj(dwRoot,m_pItemInfo->GetPEIndex(),&dwStringDir);
	
	DWORD dwNumObj;
	pPE->ResGetNumObj(dwStringDir,&dwNumObj);
	DWORD dwID;
	for(DWORD x=0;x<dwNumObj;x++){
		pPE->ResGetObjID(dwStringDir,x,&dwID);

		DWORD dwLangDir;
		pPE->ResGetDirObj(dwStringDir,x,&dwLangDir);

		//Find Correct language
		DWORD dwNumLang;
		pPE->ResGetNumObj(dwLangDir,&dwNumLang);
		DWORD dwLang;
		for(DWORD y=0;y<dwNumLang;y++){
			pPE->ResGetObjID(dwLangDir,y,&dwLang);
			if(dwLang == m_pItemInfo->GetLanguage())
			{
				DWORD dwSize;
				if(!pPE->ResGetObjDataSize(dwLangDir,y,&dwSize)) return FALSE;
				BYTE* pData = new unsigned char[dwSize];
				if(!pData) return FALSE;
				if(!pPE->ResGetObjData(dwLangDir,y,pData)) return FALSE;

				ProcessStringBlock((WORD*)pData,(WORD)dwID);
				delete[] pData;
				break;
			}
		}
	}

	// TODO: Whys this here?
	UpdateAllViews(NULL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CStringTableDoc::GetString(DWORD str, CString &strString, WORD* pwID)
{
	if(str > m_dwStrCount) return;
	STRING_TABLE_ITEM* pTmp = m_pFirst;
	for(DWORD x=0;x<str;x++){
		pTmp = pTmp->pNext;
	}
	if(pTmp){
		strString = pTmp->strText;
		*pwID = pTmp->wID;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CStringTableDoc::ProcessStringBlock(WORD *pStr, WORD wID)
{
	for(int x=0;x<16;x++){
		if(*pStr != 0x0000){
			WORD wLen = *pStr;
			pStr++;
			char* pszText = new char[wLen+1];
			int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pStr,wLen,pszText,wLen,NULL,NULL);	
			pszText[nCh] = 0;

			// Add String
			STRING_TABLE_ITEM* pTemp = m_pFirst;
			if(pTemp){
				while(pTemp->pNext){
					pTemp = pTemp->pNext;
				}
				pTemp->pNext = new STRING_TABLE_ITEM;
				pTemp->pNext->pNext = NULL;
				pTemp->pNext->strText = pszText;
				pTemp->pNext->wID = ((wID-1) << 4) + x;
				m_dwStrCount++;
			}
			else{
				m_pFirst = new STRING_TABLE_ITEM;
				m_pFirst->pNext = NULL;
				m_pFirst->strText = pszText;
				m_pFirst->wID = ((wID-1) << 4) + x;
				m_dwStrCount++;
			}
			pStr += nCh;
			if(pszText) delete[] pszText;
		}
		else pStr++;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelDoc

IMPLEMENT_DYNCREATE(CAccelDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CAccelDoc::CAccelDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

CAccelDoc::~CAccelDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CAccelDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CAccelDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelDoc diagnostics

#ifdef _DEBUG
void CAccelDoc::AssertValid() const
{
	CResBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CAccelDoc::Dump(CDumpContext& dc) const
{
	CResBaseDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelDoc commands

BOOL CAccelDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	// TODO: Whys This Here?

	UpdateAllViews(NULL);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CAccelDoc::GetVirtKeyString(WORD wKey, CString &str)
{
	if(wKey >= _T('0') && wKey <= _T('9')){
		str.Format(_T("%c"),wKey);
		return;
	}
	if(wKey >= _T('A') && wKey <= _T('Z')){
		str.Format(_T("%c"),wKey);
		return;
	}

	switch(wKey){
		case VK_ACCEPT:
			str = _T("VK_ACCEPT");
		break;
		case VK_ADD:
			str = _T("VK_ADD");
		break;
		case VK_BACK:
			str = _T("VK_BACK");
		break;
		case VK_CANCEL:
			str = _T("VK_CANCEL");
		break;
		case VK_CLEAR:
			str = _T("VK_CLEAR");
		break;
		case VK_CONVERT:
			str = _T("VK_CONVERT");
		break;
		case VK_DECIMAL:
			str = _T("VK_DECIMAL");
		break;
		case VK_DELETE:
			str = _T("VK_DELETE");
		break;
		case VK_DIVIDE:
			str = _T("VK_DIVIDE");
		break;
		case VK_DOWN:
			str = _T("VK_DOWN");
		break;
		case VK_END:
			str = _T("VK_END");
		break;
		case VK_ESCAPE:
			str = _T("VK_ESCAPE");
		break;
		case VK_F1:
			str = _T("VK_F1");
		break;
		case VK_F10:
			str = _T("VK_F10");
		break;
		case VK_F11:
			str = _T("VK_F11");
		break;
		case VK_F12:
			str = _T("VK_F12");
		break;
		case VK_F2:
			str = _T("VK_F2");
		break;
		case VK_F3:
			str = _T("VK_F3");
		break;
		case VK_F4:
			str = _T("VK_F4");
		break;
		case VK_F5:
			str = _T("VK_F5");
		break;
		case VK_F6:
			str = _T("VK_F6");
		break;
		case VK_F7:
			str = _T("VK_F7");
		break;
		case VK_F8:
			str = _T("VK_F8");
		break;
		case VK_F9:
			str = _T("VK_F9");
		break;
		case VK_FINAL:
			str = _T("VK_FINAL");
		break;
		case VK_HELP:
			str = _T("VK_HELP");
		break;
		case VK_HOME:
			str = _T("VK_HOME");
		break;
		case VK_INSERT:
			str = _T("VK_INSERT");
		break;
		case VK_KANA:
			str = _T("VK_KANA");
		break;
		case VK_KANJI:
			str = _T("VK_KANJI");
		break;
		case VK_LEFT:
			str = _T("VK_LEFT");
		break;
		case VK_MODECHANGE:
			str = _T("VK_MODECHANGE");
		break;
		case VK_MULTIPLY:
			str = _T("VK_MULTIPLY");
		break;
		case VK_NONCONVERT:
			str = _T("VK_NONCONVERT");
		break;
		case VK_NUMPAD0:
			str = _T("VK_NUMPAD0");
		break;
		case VK_NUMPAD1:
			str = _T("VK_NUMPAD1");
		break;
		case VK_NUMPAD2:
			str = _T("VK_NUMPAD2");
		break;
		case VK_NUMPAD3:
			str = _T("VK_NUMPAD3");
		break;
		case VK_NUMPAD4:
			str = _T("VK_NUMPAD4");
		break;
		case VK_NUMPAD5:
			str = _T("VK_NUMPAD5");
		break;
		case VK_NUMPAD6:
			str = _T("VK_NUMPAD6");
		break;
		case VK_NUMPAD7:
			str = _T("VK_NUMPAD7");
		break;
		case VK_NUMPAD8:
			str = _T("VK_NUMPAD8");
		break;
		case VK_NUMPAD9:
			str = _T("VK_NUMPAD9");
		break;
		case VK_PAUSE:
			str = _T("VK_PAUSE");
		break;
		case VK_RETURN:
			str = _T("VK_RETURN");
		break;
		case VK_RIGHT:
			str = _T("VK_RIGHT");
		break;
		case VK_SPACE:
			str = _T("VK_SPACE");
		break;
		case VK_SUBTRACT:
			str = _T("VK_SUBTRACT");
		break;
		case VK_TAB:
			str = _T("VK_TAB");
		break;
		case VK_UP:
			str = _T("VK_UP");
		break;
		case VK_LBUTTON:
			str = _T("VK_LBUTTON");
			break;
		case VK_RBUTTON:
			str = _T("VK_RBUTTON");
			break;
		case VK_MBUTTON:
			str = _T("VK_MBUTTON");
			break;
		case VK_SHIFT:
			str = _T("VK_SHIFT");
			break;
		case VK_CONTROL:
			str = _T("VK_CONTROL");
			break;
		case VK_MENU:
			str = _T("VK_MENU");
			break;
		case VK_CAPITAL:
			str = _T("VK_CAPITAL");
			break;
		case VK_JUNJA:
			str = _T("VK_JUNJA");
			break;
		case VK_PRIOR:
			str = _T("VK_PRIOR");
			break;
		case VK_NEXT:
			str = _T("VK_NEXT");
			break;
		case VK_SELECT:
			str = _T("VK_SELECT");
			break;
		case VK_PRINT:
			str = _T("VK_PRINT");
			break;
		case VK_EXECUTE:
			str = _T("VK_EXECUTE");
			break;
		case VK_SNAPSHOT:
			str = _T("VK_SNAPSHOT");
			break;
		case VK_LWIN:
			str = _T("VK_LWIN");
			break;
		case VK_RWIN:
			str = _T("VK_RWIN");
			break;
		case VK_APPS:
			str = _T("VK_APPS");
			break;
		case VK_SEPARATOR:
			str = _T("VK_SEPARATOR");
			break;
		case VK_F13:
			str = _T("VK_F13");
			break;
		case VK_F14:
			str = _T("VK_F14");
			break;
		case VK_F15:
			str = _T("VK_F15");
			break;
		case VK_F16:
			str = _T("VK_F16");
			break;
		case VK_F17:
			str = _T("VK_F17");
			break;
		case VK_F18:
			str = _T("VK_F18");
			break;
		case VK_F19:
			str = _T("VK_F19");
			break;
		case VK_F20:
			str = _T("VK_F20");
			break;
		case VK_F21:
			str = _T("VK_F21");
			break;
		case VK_F22:
			str = _T("VK_F22");
			break;
		case VK_F23:
			str = _T("VK_F23");
			break;
		case VK_F24:
			str = _T("VK_F24");
			break;
		case VK_NUMLOCK:
			str = _T("VK_NUMLOCK");
			break;
		case VK_SCROLL:
			str = _T("VK_SCROLL");
			break;
		case VK_LSHIFT:
			str = _T("VK_LSHIFT");
			break;
		case VK_RSHIFT:
			str = _T("VK_RSHIFT");
			break;
		case VK_LCONTROL:
			str = _T("VK_LCONTROL");
			break;
		case VK_RCONTROL:
			str = _T("VK_RCONTROL");
			break;
		case VK_LMENU:
			str = _T("VK_LMENU");
			break;
		case VK_RMENU:
			str = _T("VK_RMENU");
			break;
		case VK_PROCESSKEY:
			str = _T("VK_PROCESSKEY");
			break;
		case VK_ATTN:
			str = _T("VK_ATTN");
			break;
		case VK_CRSEL:
			str = _T("VK_CRSEL");
			break;
		case VK_EXSEL:
			str = _T("VK_EXSEL");
			break;
		case VK_EREOF:
			str = _T("VK_EREOF");
			break;
		case VK_PLAY:
			str = _T("VK_PLAY");
			break;
		case VK_ZOOM:
			str = _T("VK_ZOOM");
			break;
		case VK_NONAME:
			str = _T("VK_NONAME");
			break;
		case VK_PA1:
			str = _T("VK_PA1");
			break;
		case VK_OEM_CLEAR:
			str = _T("VK_OEM_CLEAR");
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpDoc

IMPLEMENT_DYNCREATE(CBmpDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CBmpDoc::CBmpDoc()
{
	m_hBmp = NULL;
	m_szBmp.SetSize(0,0);
}

/////////////////////////////////////////////////////////////////////////////

CBmpDoc::~CBmpDoc()
{
	if(m_hBmp)
	{
		DeleteObject(m_hBmp);
	}
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CBmpDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CBmpDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpDoc diagnostics

#ifdef _DEBUG
void CBmpDoc::AssertValid() const
{
	CResBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CBmpDoc::Dump(CDumpContext& dc) const
{
	CResBaseDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpDoc commands

BOOL CBmpDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	m_hBmp = NULL;
	m_szBmp.SetSize(0,0);

	ASSERT(m_pItemInfo);
	CString& strPath = m_pItemInfo->GetWorkspaceDoc()->GetFilePath();

	HINSTANCE hInst = (HINSTANCE)LoadLibraryEx(strPath,NULL,LOAD_LIBRARY_AS_DATAFILE);

	ASSERT(hInst);
	if(NULL == hInst)
	{
		return FALSE;
	}

	m_hBmp = LoadImage(hInst,
				(m_pItemInfo->GetID() ? 
				MAKEINTRESOURCE(m_pItemInfo->GetID()) : 
				*m_pItemInfo->GetResourceName()),
			IMAGE_BITMAP,
			0,0,LR_DEFAULTCOLOR | LR_DEFAULTSIZE);

	FreeLibrary(hInst);

	ASSERT(m_hBmp);
	if(!m_hBmp)
	{
		return FALSE;
	}	

	// Get Size

	PBITMAPINFO pbinfo = (PBITMAPINFO)m_pData;
	m_szBmp.cx = pbinfo->bmiHeader.biWidth;
	m_szBmp.cy = pbinfo->bmiHeader.biHeight;

	// TODO: Whys This Here

	UpdateAllViews(NULL);	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgDoc

IMPLEMENT_DYNCREATE(CDlgDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CDlgDoc::CDlgDoc()
{
	m_bValid = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

CDlgDoc::~CDlgDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDlgDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CDlgDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgDoc diagnostics

#ifdef _DEBUG
void CDlgDoc::AssertValid() const
{
	CResBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CDlgDoc::Dump(CDumpContext& dc) const
{
	CResBaseDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgDoc commands

BOOL CDlgDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	// Initialise helper

	if(!m_dlgHelp.Init(m_pData,m_dwSize))
	{
		return FALSE;
	}

	m_bValid = TRUE;

	// TODO: Whys THis Here?

	UpdateAllViews(NULL);	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDlgDoc::IsValid()
{
	return m_bValid;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CHexDoc

IMPLEMENT_DYNCREATE(CHexDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CHexDoc::CHexDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

CHexDoc::~CHexDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CHexDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CHexDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CHexDoc diagnostics

#ifdef _DEBUG
void CHexDoc::AssertValid() const
{
	CDocument::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CHexDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CHexDoc commands

BOOL CHexDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	// TODO: Whys This Here?
	UpdateAllViews(NULL);	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconDoc

IMPLEMENT_DYNCREATE(CIconDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CIconDoc::CIconDoc()
{
	m_nIconCount = 0;
	m_cx = m_cy = 0;
	m_hIcon = NULL;
	m_bIcon = FALSE;
	m_ptHot.x = m_ptHot.y = 0;
}

/////////////////////////////////////////////////////////////////////////////

CIconDoc::~CIconDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CIconDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CIconDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconDoc diagnostics

#ifdef _DEBUG
void CIconDoc::AssertValid() const
{
	CDocument::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CIconDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconDoc commands

BOOL CIconDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	// Parse data
	GROUP_ICON_HEADER* pgih;
	pgih = (GROUP_ICON_HEADER*)m_pData;

	m_bIcon = (pgih->wType == 1); // Icon or Cursor?
	m_nIconCount = pgih->wCount;
	SetIconIndex(0); //Set To First

	// TODO: Whys This Here?
	UpdateAllViews(NULL);	

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CIconDoc::GetIconInfo(int nIcon, int *pcx, int *pcy,int* pnBase, int *pNumColors /*= NULL*/)
{
	GROUP_ICON_RESOURCE* pgrh;
	GROUP_CURSOR_RESOURCE* pgcr;

	pgrh = (GROUP_ICON_RESOURCE*)(m_pData + sizeof(GROUP_ICON_HEADER));
	pgcr = (GROUP_CURSOR_RESOURCE*)(m_pData + sizeof(GROUP_ICON_HEADER));
	// Move to icon
	if(m_bIcon){
		pgrh+=nIcon;
		*pnBase = pgrh->wNameOrdinal;
		*pcx = pgrh->bWidth;
		*pcy = pgrh->bHeight;
		if(pNumColors) *pNumColors = (1 << pgrh->wBitCount);
	}
	else{
		pgcr+=nIcon;
		*pnBase = pgcr->wNameOrdinal;
		*pcx = pgcr->bWidth;
		*pcy = pgcr->bHeight / 2;
		if(pNumColors) *pNumColors = (1 << pgcr->wBitCount);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CIconDoc::SetIconIndex(int nIcon)
{
	Assert(m_pItemInfo);
	CPERes *pPE = m_pItemInfo->GetWorkspaceDoc()->GetResourceParser();

	ASSERT(pPE);

	if(m_hIcon) DeleteObject(m_hIcon);

	DWORD dwRoot = pPE->ResGetRootDir();
	DWORD dwNumObj, dwID;
	pPE->ResGetNumObj(dwRoot,&dwNumObj);

	BOOL bFound = FALSE;
	for(DWORD x=0;x<dwNumObj && !bFound;x++){
		if(pPE->ResGetObjID(dwRoot,x,&dwID)){
			if(dwID == (m_bIcon ? (DWORD_PTR)RT_ICON : (DWORD_PTR)RT_CURSOR)) bFound = TRUE;
		}
	}
	if(!bFound) return FALSE;
	x--;

	DWORD dwIconDir;
	pPE->ResGetDirObj(dwRoot,x,&dwIconDir);
	
	DWORD nBase;
	GetIconInfo(nIcon,&m_cx,&m_cy,(int*)&nBase);

	pPE->ResGetNumObj(dwIconDir,&dwNumObj);
	bFound = FALSE;
	for(x=0;x<dwNumObj && !bFound;x++){
		if(pPE->ResGetObjID(dwIconDir,x,&dwID)){
			if(dwID == nBase) bFound = TRUE;
		}
	}
	if(!bFound) return FALSE;
	x--;

	DWORD dwLangDir;
	pPE->ResGetDirObj(dwIconDir,x,&dwLangDir);

	DWORD nSize;
	if(!pPE->ResGetObjDataSize(dwLangDir,0,&nSize)) return FALSE;

	BYTE* pData = new unsigned char[nSize];
	if(!pData) return NULL;
	if(!pPE->ResGetObjData(dwLangDir,0,pData)){
		delete[] pData;
		return FALSE;
	}
	
	// Set HotSpot
	if(!m_bIcon){
		m_ptHot.x = *(WORD*)(pData);
		m_ptHot.y = *(WORD*)(pData+2);
	}
	// Create Icon Or Cursor
	m_hIcon = CreateIconFromResourceEx(pData,nSize,m_bIcon,0x00030000,m_cx,m_cy,LR_DEFAULTCOLOR);
	delete[] pData;
	pData = NULL;
	if(m_hIcon) return TRUE;
	else return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuDoc

IMPLEMENT_DYNCREATE(CMenuDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CMenuDoc::CMenuDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

CMenuDoc::~CMenuDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMenuDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CMenuDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuDoc diagnostics

#ifdef _DEBUG
void CMenuDoc::AssertValid() const
{
	CResBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CMenuDoc::Dump(CDumpContext& dc) const
{
	CResBaseDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuDoc commands
BOOL CMenuDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	// TODO: Whys This Here?
	UpdateAllViews(NULL);	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionDoc

IMPLEMENT_DYNCREATE(CVersionDoc, CResBaseDoc)

/////////////////////////////////////////////////////////////////////////////

CVersionDoc::CVersionDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

CVersionDoc::~CVersionDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CVersionDoc, CResBaseDoc)
	//{{AFX_MSG_MAP(CVersionDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionDoc diagnostics

#ifdef _DEBUG
void CVersionDoc::AssertValid() const
{
	CResBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CVersionDoc::Dump(CDumpContext& dc) const
{
	CResBaseDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionDoc commands

BOOL CVersionDoc::InitDoc()
{
	if(!__super::InitDoc())
	{
		return FALSE;
	}

	// TODO: Whys This Here?
	UpdateAllViews(NULL);	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
