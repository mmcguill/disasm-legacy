///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "ResourcesView.h"
#include "ResourcesFrm.h"
#include ".\resourcesview.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CStringTableView

IMPLEMENT_DYNCREATE(CStringTableView, CListView)

///////////////////////////////////////////////////////////////////////////////

CStringTableView::CStringTableView()
{
}

///////////////////////////////////////////////////////////////////////////////

CStringTableView::~CStringTableView()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStringTableView, CListView)
	//{{AFX_MSG_MAP(CStringTableView)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStringTableView drawing

void CStringTableView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CStringTableView diagnostics

#ifdef _DEBUG
void CStringTableView::AssertValid() const
{
	CListView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CStringTableView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStringTableView message handlers

void CStringTableView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();

	CListCtrl& lc = GetListCtrl();
	lc.ModifyStyle(0,LVS_REPORT);
	lc.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	CString str;
	str.LoadString(IDS_STRVIEW_HEAD_ID);
	lc.InsertColumn(0,str,LVCFMT_LEFT,50,0);
	str.LoadString(IDS_STRVIEW_HEAD_STRING);
	lc.InsertColumn(1,str,LVCFMT_LEFT,500,1);
}

///////////////////////////////////////////////////////////////////////////////

void CStringTableView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CStringTableDoc* pDoc = GetDocument();
	int numStr = pDoc->GetNumStrings();
	if(numStr){
		CListCtrl& lc = GetListCtrl();
		lc.DeleteAllItems();
		CString strID, str;
		WORD wID;
		for(int x=0;x<numStr;x++){
			pDoc->GetString(x,str,&wID);
			strID.Format(_T("%d"),wID);
			lc.InsertItem(x,strID);
			str.Replace(_T("\n"),_T("\\n"));
			str.Replace(_T("\r"),_T("\\r"));
			str.Replace(_T("\t"),_T("\\t"));
			lc.SetItem(x,1,LVIF_TEXT,str,0,0,0,0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CStringTableView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// Removed, dont allow hex view without raw resources

	//CMenu mnu;
	//if(!mnu.LoadMenu(IDM_CTXT_RESVIEW))
	//{
	//	ASSERT(FALSE);
	//	return;
	//}

	//CMenu *pCtxt = mnu.GetSubMenu(0);
	//pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

CStringTableDoc* CStringTableView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CStringTableDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionView

IMPLEMENT_DYNCREATE(CVersionView, CListView)

///////////////////////////////////////////////////////////////////////////////

CVersionView::CVersionView()
{
}

///////////////////////////////////////////////////////////////////////////////

CVersionView::~CVersionView()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CVersionView, CListView)
	//{{AFX_MSG_MAP(CVersionView)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionView diagnostics

#ifdef _DEBUG
void CVersionView::AssertValid() const
{
	CListView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CVersionView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionView message handlers

void CVersionView::OnInitialUpdate() 
{
	CListCtrl& lc = GetListCtrl();
	lc.ModifyStyle(0,LVS_REPORT);
	lc.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	CString str;
	str.LoadString(IDS_VERVIEW_HEAD_FIELD);
	lc.InsertColumn(0,str,LVCFMT_LEFT,200,0);
	str.LoadString(IDS_VERVIEW_HEAD_VALUE);
	lc.InsertColumn(1,str,LVCFMT_LEFT,200,1);
	CListView::OnInitialUpdate();
}

///////////////////////////////////////////////////////////////////////////////

void CVersionView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CVersionDoc* pDoc = GetDocument();
	if(!pDoc->GetRawData()) return;

	CListCtrl& lc = GetListCtrl();
	lc.DeleteAllItems();
	
	VS_VERSIONINFO_1* pVer1 = (VS_VERSIONINFO_1*)pDoc->GetRawData();

	TCHAR szKey[64];

	WORD* pRaw = (WORD*)((BYTE*)pVer1 + sizeof(VS_VERSIONINFO_1));

#ifdef UNICODE
	lstrcpyn(szKey,(LPCWSTR)pRaw,64);
	
#else
	WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pRaw,-1,szKey,64,NULL,NULL);	
#endif
	
	int nCh = (int)_tcslen(szKey) + 1;

    if(lstrcmp(szKey,_T("VS_VERSION_INFO")))
	{
		return;
	}

	pRaw += nCh;
	DWORD nAdd = (DWORD)(DWORD_PTR)pRaw;
	if(nAdd % 4) nAdd += (4 - (nAdd % 4));
	pRaw = (WORD*)(DWORD_PTR)nAdd;

	VS_FIXEDFILEINFO* pvffi = (VS_FIXEDFILEINFO*)pRaw;

	CString strInfo;
	CString str;
	lc.InsertItem(0,_T("Signature"));
	strInfo.Format(_T("%X"),pvffi->dwSignature);
	lc.SetItem(0,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(1,_T("Struct Version"));
	strInfo.Format(_T("%d"),pvffi->dwStrucVersion);
	lc.SetItem(1,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(2,_T("File Version MS"));
    strInfo.Format(_T("%d"),pvffi->dwFileVersionMS); 
	lc.SetItem(2,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(3,_T("File Version LS"));
    strInfo.Format(_T("%d"),pvffi->dwFileVersionLS);
	lc.SetItem(3,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(4,_T("Product Version MS"));
    strInfo.Format(_T("%d"),pvffi->dwProductVersionMS);
	lc.SetItem(4,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(5,_T("Product Version LS"));
    strInfo.Format(_T("%d"),pvffi->dwProductVersionLS);
	lc.SetItem(5,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(6,_T("File Flags Mask"));
    strInfo.Format(_T("%d"),pvffi->dwFileFlagsMask);
	lc.SetItem(6,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(7,_T("File Flags"));
    strInfo.Format(_T("%d"),pvffi->dwFileFlags);
	lc.SetItem(7,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(8,_T("File OS"));
    strInfo.Format(_T("%d"),pvffi->dwFileOS);
	lc.SetItem(8,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(9,_T("File Type"));
    strInfo.Format(_T("%d"),pvffi->dwFileType); 
	lc.SetItem(9,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(10,_T("File Sub Type"));
    strInfo.Format(_T("%d"),pvffi->dwFileSubtype);
	lc.SetItem(10,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(11,_T("File Date MS"));
    strInfo.Format(_T("%d"),pvffi->dwFileDateMS);
	lc.SetItem(11,1,LVIF_TEXT,strInfo,0,0,0,0);

	lc.InsertItem(12,_T("File Date LS"));
    strInfo.Format(_T("%d"),pvffi->dwFileDateLS);
	lc.SetItem(12,1,LVIF_TEXT,strInfo,0,0,0,0);

	//move past padding2

	/*pRaw = (WORD*)((BYTE*)pRaw + (sizeof(VS_FIXEDFILEINFO)));
	nAdd = (DWORD)pRaw;
	if(nAdd % 4) nAdd += (4 - (nAdd % 4));
	pRaw = (WORD*)nAdd;*/
}

///////////////////////////////////////////////////////////////////////////////

void CVersionView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_RESVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

CVersionDoc* CVersionView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CVersionDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuView

#define MENUVIEW_TOP_MARGIN 5
#define MENUVIEW_LEFT_MARGIN 5
#define MENUVIEW_COLSPACE 10

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMenuView, CScrollView)

///////////////////////////////////////////////////////////////////////////////

CMenuView::CMenuView()
{
	m_bEx = FALSE;
	m_cxMenu = 0;
	m_pMenuFirst = NULL;
}

///////////////////////////////////////////////////////////////////////////////

CMenuView::~CMenuView()
{
	if(m_pMenuFirst) DeleteAllMenuItems(m_pMenuFirst);
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::DeleteAllMenuItems(TOPLEVEL_MENU_ITEM* pItem)
{
	if(pItem->pNext) DeleteAllMenuItems(pItem->pNext);
	delete pItem;
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMenuView, CScrollView)
	//{{AFX_MSG_MAP(CMenuView)
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuView diagnostics

#ifdef _DEBUG
void CMenuView::AssertValid() const
{
	CScrollView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuView message handlers

void CMenuView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CMenuDoc* pDoc = GetDocument();
	if(pDoc->GetRawData()){
		if(m_pMenuFirst){
			DeleteAllMenuItems(m_pMenuFirst);
			m_pMenuFirst = NULL;
		}

		m_bEx = (*(DWORD*)pDoc->GetRawData() == 0x00040001);

		if(m_bEx)
			ExLoadTopMenuItems();
		else
			StdLoadTopMenuItems();
	}
	SetScrollSizes(MM_TEXT,CSize(m_cxMenu,200));
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::ExLoadTopMenuItems()
{
/*	ASSERT(m_bEx);
	
	CMenuDoc* pDoc = GetDocument();
	EX_MENUHEADER* pmh = (EX_MENUHEADER*)pDoc->m_pData;
	EX_MENUITEM* pmi = (EX_MENUITEM*)((BYTE*)pmh + (sizeof(EX_MENUHEADER)) + pmh->wOffset);

	int nNextX = LEFT_MARGIN + COLSPACE;
	CSize sizeText;
	CString strText;
	int nPopLevel = 0;
	BYTE* pEndMenuRes = (pDoc->m_pData + pDoc->GetRawDataSize());
	while((BYTE*)pmi < pEndMenuRes){
		WORD* pStr = (&pmi->wResInfo) + 1;
		if(pmi->wResInfo & 0x0001) pStr--;

		// Get Text
		char szText[MAX_PATH];
		int nCh = WideCharToMultiByte(CP_ACP,0,	pStr,-1,szText,MAX_PATH,NULL,NULL);	
		szText[nCh] = 0;
		strText = szText;

		// Add Top Level
		if(nPopLevel == 0){
			TOPLEVEL_MENU_ITEM* pTmp = m_pMenuFirst;
			if(pTmp){
				while(pTmp->pNext){
					pTmp = pTmp->pNext;
				}
				pTmp->pNext =  new TOPLEVEL_MENU_ITEM;
				pTmp->pNext->strText = strText;
				pTmp->pNext->pNext = NULL;
				// Check for separator
				
				if(pmi->dwType == 0 && strText.IsEmpty()){
					//vertical sep
					pTmp->pNext->pMenuChild = NULL;
				}
				else{
					if((DWORD)(pStr + nCh) < (DWORD)pEndMenuRes)
						pTmp->pNext->pMenuChild = (MENUITEMTEMPLATE*)(pStr + nCh);
					else
						pTmp->pNext->pMenuChild = NULL;
				}
				
			}
			else{
				m_pMenuFirst = new TOPLEVEL_MENU_ITEM;
				m_pMenuFirst->strText = strText;
				m_pMenuFirst->pNext = NULL;
				
				if(pmi->dwType == 0 && strText.IsEmpty()){
					//vertical sep
					m_pMenuFirst->pMenuChild = NULL;
				}
				else{
					if((DWORD)(pStr + nCh) < (DWORD)pEndMenuRes)
						m_pMenuFirst->pMenuChild = (MENUITEMTEMPLATE*)(pStr + nCh);
					else
						m_pMenuFirst->pMenuChild  = NULL;
				}
			}
			sizeText = GetDC()->GetTextExtent(strText);
			nNextX+= sizeText.cx + COLSPACE;
		}
		// Advance
		pStr += nCh;
		if(pmi->wResInfo == 0x0001){
			nPopLevel++;
			pStr+=2; //Help ID
		}
		// Adjust Popup Level
		if(pmi->wResInfo == 0x0080) nPopLevel--;

		if((DWORD)pStr % 4) pStr++; //Dword Align
		pmi = (EX_MENUITEM*)pStr;
	}*/
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::StdLoadTopMenuItems()
{
	ASSERT(!m_bEx);
	
	CMenuDoc* pDoc = GetDocument();
	MENUITEMTEMPLATEHEADER* pmith = (MENUITEMTEMPLATEHEADER*)pDoc->GetRawData();
	MENUITEMTEMPLATE* pmit = (MENUITEMTEMPLATE*)((BYTE*)pmith + (sizeof(MENUITEMTEMPLATEHEADER)) + pmith->offset);
		
	int nNextX = MENUVIEW_LEFT_MARGIN + MENUVIEW_COLSPACE;
	CSize sizeText;
	CString strText;
	int nPopLevel = 0;
	BYTE* pEndMenuRes = (pDoc->GetRawData() + pDoc->GetRawDataSize());
	while((BYTE*)pmit < pEndMenuRes){
		WORD* pStr = (WORD*)&pmit->mtString[0];
		if(pmit->mtOption & MF_POPUP) pStr--;
		
		char szText[MAX_PATH];
		int nCh = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)pStr,-1,szText,MAX_PATH,NULL,NULL);	
		szText[nCh] = 0;
		strText = szText;

		if(nPopLevel == 0){
			TOPLEVEL_MENU_ITEM* pTmp = m_pMenuFirst;
			if(pTmp){
				while(pTmp->pNext){
					pTmp = pTmp->pNext;
				}
				pTmp->pNext =  new TOPLEVEL_MENU_ITEM;
				pTmp->pNext->strText = strText;
				pTmp->pNext->pNext = NULL;
				// Check for separator
				if(pmit->mtID == 0 && strText.IsEmpty()){
					//vertical sep
					pTmp->pNext->pMenuChild = NULL;
				}
				else{
					if((DWORD_PTR)(pStr + nCh) < (DWORD_PTR)pEndMenuRes)
						pTmp->pNext->pMenuChild = (MENUITEMTEMPLATE*)(pStr + nCh);
					else
						pTmp->pNext->pMenuChild = NULL;
				}
				
			}
			else{
				m_pMenuFirst = new TOPLEVEL_MENU_ITEM;
				m_pMenuFirst->strText = strText;
				m_pMenuFirst->pNext = NULL;
					if(pmit->mtID == 0 && strText.IsEmpty()){
					//vertical sep
					m_pMenuFirst->pMenuChild = NULL;
				}
				else{
					if((DWORD_PTR)(pStr + nCh) < (DWORD_PTR)pEndMenuRes)
						m_pMenuFirst->pMenuChild = (MENUITEMTEMPLATE*)(pStr + nCh);
					else
						m_pMenuFirst->pMenuChild  = NULL;
				}
			}
			sizeText = GetDC()->GetTextExtent(strText);
			nNextX+= sizeText.cx + MENUVIEW_COLSPACE;
		}
		// Adjust Popup Level
		if(pmit->mtOption & MF_POPUP) nPopLevel++;
		else if(pmit->mtOption & MF_CHANGE) nPopLevel--;
		pmit = (MENUITEMTEMPLATE*)(pStr+=nCh);
	}
	m_cxMenu = nNextX;
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::OnDraw(CDC* pDC)
{
	CMenuDoc* pDoc = GetDocument();
	if(!pDoc->GetRawData()) return;

	CFont* pFontOld = (CFont*)pDC->SelectStockObject(DEFAULT_GUI_FONT);
	if(m_bEx){
		CString str;
		str.LoadString(IDS_ERROR_MENU_EXTENDED);
		CRect rct;
		GetClientRect(rct);
		pDC->DrawText(str,rct,DT_CENTER);
		return;
	}

	CBrush brshBack(::GetSysColor(COLOR_BTNFACE));
	int cyBack = ::GetSystemMetrics(SM_CYMENU) + 2;
	CRect rctBack(MENUVIEW_LEFT_MARGIN,MENUVIEW_TOP_MARGIN,m_cxMenu,cyBack);
	pDC->FillRect(rctBack,&brshBack);
	
	pDC->SetBkMode(TRANSPARENT);
	TOPLEVEL_MENU_ITEM* pTmp = m_pMenuFirst;
	CSize sizeText;
	CRect rctText = rctBack;
	int nNextX = MENUVIEW_LEFT_MARGIN + MENUVIEW_COLSPACE;
	while(pTmp){
		sizeText = pDC->GetTextExtent(pTmp->strText);
		rctText.left = nNextX;
		rctText.right = nNextX + sizeText.cx + MENUVIEW_COLSPACE;
		pTmp->rct = rctText;
		
		if(pTmp->pMenuChild == NULL && pTmp->strText.IsEmpty()) // Vert Separator
			pDC->DrawEdge(rctText,EDGE_ETCHED,BF_LEFT);
		else
			pDC->DrawText(pTmp->strText,rctText,DT_SINGLELINE|DT_EXTERNALLEADING|DT_VCENTER);

		nNextX += sizeText.cx + MENUVIEW_COLSPACE;
		pTmp = pTmp->pNext;
	}

	pDC->SelectObject(pFontOld);
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CMenuDoc* pDoc = GetDocument();
	if(!pDoc->GetRawData()){
		CScrollView::OnLButtonDown(nFlags, point);
		return;
	}

	if(!m_bEx){
		TOPLEVEL_MENU_ITEM* pTmp = m_pMenuFirst;
		BOOL bFound = FALSE;
		CPoint ptTst = point;
		CPoint ptOff = GetScrollPosition();
		ptTst.Offset(ptOff);
		while(pTmp && !bFound){
			if(pTmp->rct.PtInRect(ptTst)) bFound = TRUE;
			if(!bFound) pTmp = pTmp->pNext;
		}
		
		if(bFound){
			CPoint ptTrack(pTmp->rct.left,pTmp->rct.bottom);
			ptTrack.Offset(-ptOff);
			if(!(ptTrack.x < 0 || ptTrack.y < 0)){
				ClientToScreen(&ptTrack);
				MENUITEMTEMPLATE* pMit = pTmp->pMenuChild;
				HMENU hMenu = StdMyCreatePopupMenu(pMit, NULL);
				if(hMenu) ::TrackPopupMenu(hMenu,TPM_LEFTALIGN,ptTrack.x,ptTrack.y,0,this->m_hWnd,0);
				if(hMenu) ::DestroyMenu(hMenu);
			}
		}
	}
	CScrollView::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////

HMENU CMenuView::StdMyCreatePopupMenu(MENUITEMTEMPLATE* pMit,MENUITEMTEMPLATE** ppNextMenuItem)
{
	ASSERT(!m_bEx);

	if(pMit == NULL) return NULL;

	HMENU hCur = ::CreatePopupMenu();
	BOOL bEnd = FALSE;
	BOOL bAdvance = TRUE;
	CString strItemText;
	do{
		bAdvance = TRUE;
		// Text
		WORD* pStr = (WORD*)&pMit->mtString[0];
		if(pMit->mtOption & MF_POPUP) pStr--;
		char szText[MAX_PATH];
		int nCh = WideCharToMultiByte(CP_ACP,0,	(LPCWSTR)pStr,-1,szText,MAX_PATH,NULL,NULL);	
		szText[nCh] = 0;
		strItemText = szText;

		// Style
		DWORD dwStyle = MF_ENABLED;
		if((pMit->mtID == 0) && strItemText.IsEmpty()) 	dwStyle |= MF_SEPARATOR;
		
		// Add To Menu
		if(pMit->mtOption & MF_POPUP){
			dwStyle |= MF_POPUP;
			MENUITEMTEMPLATE* pChildMit = (MENUITEMTEMPLATE*)(pStr+nCh);
			MENUITEMTEMPLATE* pNextMenuItem;
			HMENU hTemp = StdMyCreatePopupMenu(pChildMit,&pNextMenuItem);
			pMit = pNextMenuItem;
			::AppendMenu(hCur,dwStyle,(UINT_PTR)hTemp,strItemText);
			bAdvance = FALSE;
		}
		else{
			::AppendMenu(hCur,dwStyle,0x0000,strItemText);
			if(pMit->mtOption & MF_CHANGE){
				bEnd = TRUE;
			}
		}

		// Advance
		if(bAdvance) pMit = (MENUITEMTEMPLATE*)(pStr+nCh);
		if(ppNextMenuItem) *ppNextMenuItem = pMit;
	}while(!bEnd);
	return hCur;
}

///////////////////////////////////////////////////////////////////////////////

void CMenuView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_RESVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

CMenuDoc* CMenuView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CMenuDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconView

IMPLEMENT_DYNCREATE(CIconView, CScrollView)

///////////////////////////////////////////////////////////////////////////////

CIconView::CIconView()
{
}

///////////////////////////////////////////////////////////////////////////////

CIconView::~CIconView()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CIconView, CScrollView)
	//{{AFX_MSG_MAP(CIconView)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconView drawing

void CIconView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 0;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

///////////////////////////////////////////////////////////////////////////////

void CIconView::OnDraw(CDC* pDC)
{
	CIconDoc* pDoc = GetDocument();
	HICON hIcon = pDoc->GetIcon();
	CSize sz = pDoc->GetSize();
	if(!hIcon) return;
	DrawIconEx(pDC->m_hDC,2,2,hIcon,sz.cx,sz.cy,0,NULL,DI_NORMAL);	
}

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconView diagnostics

#ifdef _DEBUG
void CIconView::AssertValid() const
{
	CScrollView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CIconView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconView message handlers

void CIconView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CIconDoc* pDoc = GetDocument();
	CIconFrame* pFrame = (CIconFrame*)GetParentFrame();

	int nCount = pDoc->GetIconCount();
	int cx,cy,numColors,nbase;
	CString strDevImg;
	for(int x = 0;x<nCount;x++){
		pDoc->GetIconInfo(x,&cx,&cy,&nbase,&numColors);
		strDevImg.Format(IDS_ICONVIEW_FORMAT_DEVICE_IMAGE,cx,cx,numColors);
		pFrame->AddDevImage(strDevImg);
	}
	
	if(!pDoc->IsIcon())
	{
		pFrame->SetHotSpot(TRUE,pDoc->GetHotSpot());
	}
	else
	{
		pFrame->SetHotSpot(FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////

void CIconView::SetIconIndex(int nIcon)
{
	CIconDoc* pDoc = GetDocument();
	pDoc->SetIconIndex(nIcon);
	
	CSize sz = pDoc->GetSize();
	SetScrollSizes(MM_TEXT,CSize(sz.cx,sz.cy));

	CIconFrame* pFrame = (CIconFrame*)GetParentFrame();
	if(!pDoc->IsIcon()){
		pFrame->SetHotSpot(TRUE,pDoc->GetHotSpot());
	}
	else{
		pFrame->SetHotSpot(FALSE);
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

void CIconView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_RESVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

CIconDoc* CIconView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CIconDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CHexView

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CHexView, CTextView)

/////////////////////////////////////////////////////////////////////////////

CHexView::CHexView()
{
	m_hvmViewMode = VIEW_BYTE;
	m_nBytesPerLine = 16;
}

/////////////////////////////////////////////////////////////////////////////

CHexView::~CHexView()
{
}

/////////////////////////////////////////////////////////////////////////////

#define HEXVIEW_COLOR_ADDRESS RGB(0,0,200)
#define HEXVIEW_COLOR_HEX RGB(0,0,0)
#define HEXVIEW_COLOR_CHARDATA RGB(0,180,0)

//static const TEXTVIEW_COLUMN rgColumns[] = 
//{
//	{_T("Address"),100,0},
//	{_T("Hex"),400,100},
//	{_T("Text"),300,500},
//};

static LPCTSTR rgColumns[] = 
{
	_T("Address"),
	_T("Hex"),
	_T("Text"),
};

#define MAX_COLS (sizeof(rgColumns) / sizeof(LPCTSTR))

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CHexView, CTextView)
	//{{AFX_MSG_MAP(CHexView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CTXT_HEXVIEW_VIEWBYTE,OnViewByte)
	ON_COMMAND(ID_CTXT_HEXVIEW_VIEWWORD,OnViewWord)
	ON_COMMAND(ID_CTXT_HEXVIEW_VIEWDWORD,OnViewDword)
//	ON_WM_CREATE()
ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CHexDoc* CHexView::GetDocument()
{
	return (CHexDoc*)CTextView::GetDocument();
}

/////////////////////////////////////////////////////////////////////////////

int CHexView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTextView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	if(!Init(CTextView::viewModeCaret,TRUE,NULL,0,MAX_COLS,rgColumns,TRUE))
	{
		ASSERT(FALSE);
		return -1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CHexView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CHexDoc* pDoc = GetDocument();
	ASSERT(pDoc);

	DWORD dwSize = pDoc->GetRawDataSize();
	
	int lines = (dwSize / m_nBytesPerLine) + 
		((dwSize % m_nBytesPerLine) ? 1 : 0);

	if(0 == lines)
	{
		return;
	}

	BYTE* pData = pDoc->GetRawData();
	ASSERT(pData);
	if(NULL == pData)
	{
		return;
	}

	COLORREF colors[] = {
		HEXVIEW_COLOR_ADDRESS,
		HEXVIEW_COLOR_HEX,
		HEXVIEW_COLOR_CHARDATA,
		};

	BeginDeferAddLine();
    	
	for(int i=0;i<lines;i++)
	{
		// Address
		TCHAR szAdd[16];
		wsprintf(szAdd,_T("%0.8X"),i * m_nBytesPerLine);

		
		// Numeric Hex

		int bytes =  ((i == (lines-1)) ? 
				(dwSize % m_nBytesPerLine) : m_nBytesPerLine);

		if((i == (lines-1)) && (!(dwSize % m_nBytesPerLine)))// Even
		{
			bytes = m_nBytesPerLine;
		}

		TCHAR szNumeric[128] = _T("\0");
		TCHAR szTmp[16];

		for(int j=0;j<bytes;j++)
		{
			wsprintf(szTmp,_T("%0.2X "),pData[i*m_nBytesPerLine + j]);
			_tcscat(szNumeric,szTmp);
		}
		
		int len = _tcslen(szNumeric);
		if(len)
		{
			szNumeric[len-1] = 0; // trim extra space
		}

		// Text

		TCHAR szText[32];
		for(int j=0;j<bytes;j++)
		{
			int z = pData[i*m_nBytesPerLine + j];
			int ch = (isprint(z) && z != _T(' ')) ? z : int(_T('.'));
			szText[j] = (TCHAR)ch;
		}
		szText[j] = 0;

		LPCTSTR	rgszText[] = {szAdd,szNumeric,szText};

		if(!AddLine(rgszText,colors,MAX_COLS))
		{
			EndDeferAddLine();	
			return;
		}
	}

	EndDeferAddLine();
}

/////////////////////////////////////////////////////////////////////////////

//void CHexView::DrawLine(CDC* pDC, CHexDoc* pDoc, DWORD dwLine)
//{
	//CString strTemp,strTemp2;
	//CString strHex;
	//
	//for(DWORD x = 0;x<HEXVIEW_BYTES_PER_LINE;x++){
	//	if((dwLine*HEXVIEW_BYTES_PER_LINE)+ x < pDoc->GetRawDataSize())
	//	{
	//		strTemp.Empty();
	//		BYTE* pData  = &pDoc->GetRawData()[(dwLine*HEXVIEW_BYTES_PER_LINE)+ x];
	//		switch(m_hvmViewMode){
	//		case VIEW_BYTE:
	//			strTemp.Format(_T("%0.2X "),*(BYTE*)pData);
	//			break;
	//		case VIEW_WORD:
	//			if(!(x%2)) strTemp.Format(_T("%0.4X "),*(WORD*)pData);
	//			break;
	//		case VIEW_DWORD:
	//			if(!(x%4)) strTemp.Format(_T("%0.8X "),*(DWORD*)pData);
	//			break;
	//		}
	//		strHex += strTemp;
	//	}
	//}

	//CString strChar;
	//for(DWORD x = 0;x<HEXVIEW_BYTES_PER_LINE;x++)
	//{
	//	if((dwLine*HEXVIEW_BYTES_PER_LINE)+ x < pDoc->GetRawDataSize())
	//	{
	//		int z = pDoc->GetRawData()[(dwLine*HEXVIEW_BYTES_PER_LINE)+ x];
	//		strTemp2.Format(_T("%c"),((isprint(z)) ? (TCHAR)z : _T('.')));
	//		strChar += strTemp2;
	//	}
	//}

	//CString strAddr;
	//strAddr.Format(_T("%0.8X"),dwLine*HEXVIEW_BYTES_PER_LINE);

	//pDC->SetTextColor(HEXVIEW_COLOR_ADDRESS);
	//pDC->TextOut(0,dwLine*m_nCharHeight,strAddr);

	//pDC->SetTextColor(HEXVIEW_COLOR_HEX);
	//pDC->TextOut(HEXVIEW_ADDRESS_WIDTH,dwLine*m_nCharHeight,strHex);

	//pDC->SetTextColor(HEXVIEW_COLOR_CHARDATA);
	//pDC->TextOut(HEXVIEW_ADDRESS_WIDTH + m_nHexWidth,dwLine*m_nCharHeight,strChar);
//}

/////////////////////////////////////////////////////////////////////////////
// CHexView diagnostics

#ifdef _DEBUG
void CHexView::AssertValid() const
{
	CTextView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CHexView::Dump(CDumpContext& dc) const
{
	CTextView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHexView message handlers

void CHexView::OnContextMenu( CWnd* pWnd, CPoint pos )
{
	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_HEXVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,pos.x,pos.y,this);
}

/////////////////////////////////////////////////////////////////////////////

void CHexView::OnViewByte()
{
	m_hvmViewMode = VIEW_BYTE;
	//RecalcScrolls();
	//Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void CHexView::OnViewWord()
{
	m_hvmViewMode = VIEW_WORD;
	//RecalcScrolls();
	//Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void CHexView::OnViewDword()
{
	m_hvmViewMode = VIEW_DWORD;
	//RecalcScrolls();
	//Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgView

#define DLGVIEW_LEFT_MARGIN 8
#define DLGVIEW_TOP_MARGIN  8
#define DLGVIEW_TRACK_RECT_SIZE 6

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDlgView, CScrollView)

///////////////////////////////////////////////////////////////////////////////

CDlgView::CDlgView()
{
	m_nCurControl = -1; // -1 is dialog itself
	m_trck.m_nStyle = CRectTracker::hatchedBorder | CRectTracker::resizeOutside;
	m_trck.m_nHandleSize = DLGVIEW_TRACK_RECT_SIZE;
}

///////////////////////////////////////////////////////////////////////////////

CDlgView::~CDlgView()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDlgView, CScrollView)
	//{{AFX_MSG_MAP(CDlgView)
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_CTXT_DLGVIEW_PROPERTIES,OnControlProperties)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

CDlgDoc* CDlgView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CDlgDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CDlgDoc* pDoc = GetDocument();
	SetScrollSizes(MM_TEXT,CSize(0,0));
	if(pDoc->IsValid() && !m_dlg.GetSafeHwnd())
	{
		if(!pDoc->m_dlgHelp.CreateChildDialogCopy(&m_dlg,this)) 
		{
			return;
		}

		m_dlg.SetWindowPos(NULL,DLGVIEW_LEFT_MARGIN,DLGVIEW_TOP_MARGIN,0,0,SWP_NOSIZE | SWP_NOZORDER);
		m_dlg.ShowWindow(SW_SHOW);
		m_dlg.EnableWindow(FALSE);
		
		CRect rct;
		m_dlg.GetWindowRect(rct);
		ScreenToClient(rct);
		m_trck.m_rect = rct;
		m_nCurControl = -1;
		rct.InflateRect(0,0,DLGVIEW_LEFT_MARGIN*2,DLGVIEW_TOP_MARGIN*2);
		SetScrollSizes(MM_TEXT,rct.Size());
	}
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(m_dlg.GetSafeHwnd()){
		int nItem;
		if(GetDlgItemFromPoint(point, &nItem)){
			m_nCurControl = nItem;
			CDlgDoc* pDoc = GetDocument();
			CRect rct;
			pDoc->m_dlgHelp.GetItemRect(nItem,rct);
			m_dlg.MapDialogRect(rct);
			m_dlg.ClientToScreen(&rct);
			ScreenToClient(&rct);
 
			// Set New
			m_trck.m_rect = rct;
			Invalidate();
		}
		else{
			m_nCurControl = -1;
			CRect rct;
			m_dlg.GetWindowRect(rct);
			ScreenToClient(rct);

			// Set New
			m_trck.m_rect = rct;
			Invalidate();
		}
	}
	CScrollView::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(m_dlg.GetSafeHwnd()){
		int nItem;
		if(GetDlgItemFromPoint(point, &nItem)){
			m_nCurControl = nItem;
			CDlgDoc* pDoc = GetDocument();
			CRect rct;
			pDoc->m_dlgHelp.GetItemRect(nItem,rct);
			m_dlg.MapDialogRect(rct);
			m_dlg.ClientToScreen(&rct);
			ScreenToClient(&rct);
 
			// Set New
			m_trck.m_rect = rct;
			Invalidate();
		}
		else{
			m_nCurControl = -1;
			CRect rct;
			m_dlg.GetWindowRect(rct);
			ScreenToClient(rct);

			// Set New
			m_trck.m_rect = rct;
			Invalidate();
		}
	}
	CScrollView::OnRButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if(!m_dlg.GetSafeHwnd()) return;

	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_DLGVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);
	if(!m_dlg.GetSafeHwnd())
	{
		CFont* pOldFont = (CFont*)pDC->SelectStockObject(DEFAULT_GUI_FONT);
		CString str;
		str.LoadString(IDS_ERROR_DLG_FAIL);
		CRect rct;
		GetClientRect(rct);
		pDC->DrawText(str,rct,DT_CENTER);
		pDC->SelectObject(pOldFont);
		return;
	}
	m_dlg.RedrawWindow(); // Make sure dialog is drawn first
	m_trck.Draw(pDC);
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnControlProperties()
{
	if(!m_dlg.GetSafeHwnd()) return;

	CDlgPropDlg dlg;
	CString strCont;
	
	DWORD dwStyle, dwExStyle;
	WINDOW_TYPE type;
	BuildPropertyString(strCont,&dwStyle,&dwExStyle,&type);
	dlg.m_dwStyle = dwStyle;
	dlg.m_dwExStyle = dwExStyle;
	dlg.m_type = type;
	dlg.m_strData=strCont;
	dlg.DoModal();
}

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgView diagnostics

#ifdef _DEBUG
void CDlgView::AssertValid() const
{
	CScrollView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

BOOL CDlgView::GetDlgItemFromPoint(CPoint &pt, int* pnItem) //Recievies Client Co- ords
{
	CDlgDoc* pDoc = GetDocument();
	if(!(pDoc->IsValid() && m_dlg.GetSafeHwnd())) return FALSE;
	
	ClientToScreen(&pt);
	
	int nCont = pDoc->m_dlgHelp.GetNumControls();
	CRect rctItem, rctTemp;
	rctTemp.SetRectEmpty();
	BOOL bFound = FALSE;
	for(int x=0;x<nCont;x++){
		pDoc->m_dlgHelp.GetItemRect(x,rctItem);
		m_dlg.MapDialogRect(rctItem);
		m_dlg.ClientToScreen(&rctItem);
		if(rctItem.PtInRect(pt)){
			if(bFound){
				if((rctItem.Size().cx * rctItem.Size().cy) < (rctTemp.Size().cx * rctTemp.Size().cy)){
					*pnItem = x;
					rctTemp = rctItem;
				}
			}
			else{
				*pnItem = x;
				rctTemp = rctItem;
			}
			bFound = TRUE;
		}
	}
	
	ScreenToClient(&pt); //No Damage
	return bFound;
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::BuildPropertyString(CString &strProp, DWORD *pdwStyle, DWORD* pdwExStyle, WINDOW_TYPE* ptype)
{
	CDlgDoc* pDoc = GetDocument();
	if(!(pDoc->IsValid() && m_dlg.GetSafeHwnd())) return;

	if(m_nCurControl == -1){ //Dialog
		DLGHELP_TEMPLATE dt;
		WORD wClass,wPoint,wWeight,wItalic,wMenu;
		CString strClass,strFace,strMenu,strTitle;

		pDoc->m_dlgHelp.GetTemplate(&dt);
		pDoc->m_dlgHelp.GetClass(&wClass, strClass);
		pDoc->m_dlgHelp.GetFont(&wPoint,&wWeight,&wItalic,strFace);
		pDoc->m_dlgHelp.GetMenu(&wMenu, strMenu);
		pDoc->m_dlgHelp.GetTitle(strTitle);

		strProp.Format(IDS_DLGVIEW_DLGPROP_STR, dt.style,dt.exStyle,dt.x,dt.y,dt.x + dt.cx,dt.y + dt.cy,dt.cx,dt.cy,strTitle,wClass,strClass,wMenu,strMenu,strFace,wPoint,wWeight,wItalic);

		*pdwStyle = dt.style;
		*pdwExStyle = dt.exStyle;
		*ptype = DIALOG;
	}
	else{ // Control
		DLGHELP_ITEM_TEMPLATE dit;
		WORD wClass, wTitle;
		CString strClass, strTitle;

		pDoc->m_dlgHelp.GetItemTemplate(m_nCurControl,&dit);
		pDoc->m_dlgHelp.GetItemClass(m_nCurControl,&wClass,strClass);
		pDoc->m_dlgHelp.GetItemTitle(m_nCurControl,&wTitle,strTitle);


		*ptype = DLG_PROP_DLG_UNKNOWN;
		switch(wClass){
		case 0x0080:
			strClass.LoadString(IDS_DLGVIEW_PREDEF_CLASS_BUTTON);
			*ptype = BUTTON;
			break;
		case 0x0081:
			strClass.LoadString(IDS_DLGVIEW_PREDEF_CLASS_EDIT);
			*ptype = EDIT;
			break;
		case 0x0082:
			strClass.LoadString(IDS_DLGVIEW_PREDEF_CLASS_STATIC);
			*ptype = STATIC;
			break;
		case 0x0083:
			strClass.LoadString(IDS_DLGVIEW_PREDEF_CLASS_LISTBOX);
			*ptype = LISTBOX;
			break;
		case 0x0084:
			strClass.LoadString(IDS_DLGVIEW_PREDEF_CLASS_SCROLLBAR);
			break;
		case 0x0085:
			strClass.LoadString(IDS_DLGVIEW_PREDEF_CLASS_COMBOBOX);
			*ptype = COMBOBOX;
			break;
		}

		strProp.Format(IDS_DLGVIEW_CTRLPROP_STR,dit.style,dit.exStyle,dit.x,dit.y,dit.x + dit.cx,dit.y + dit.cy,dit.cx,dit.cy,wTitle,strTitle,wClass,strClass,dit.id,dit.helpID);

		*pdwStyle = dit.style;
		*pdwExStyle = dit.exStyle;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CDlgView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(!m_dlg.GetSafeHwnd()) return;

	CDlgPropDlg dlg;
	CString strCont;
	
	DWORD dwStyle, dwExStyle;
	WINDOW_TYPE type;
	BuildPropertyString(strCont,&dwStyle,&dwExStyle,&type);
	dlg.m_dwStyle = dwStyle;
	dlg.m_dwExStyle = dwExStyle;
	dlg.m_strData=strCont;
	dlg.m_type = type;
	dlg.DoModal();
	CScrollView::OnLButtonDblClk(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpView

IMPLEMENT_DYNCREATE(CBmpView, CScrollView)

///////////////////////////////////////////////////////////////////////////////

CBmpView::CBmpView()
{
}

///////////////////////////////////////////////////////////////////////////////

CBmpView::~CBmpView()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CBmpView, CScrollView)
	//{{AFX_MSG_MAP(CBmpView)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpView drawing

void CBmpView::OnDraw(CDC* pDC)
{
	CFont* pFontOld = (CFont*)pDC->SelectStockObject(DEFAULT_GUI_FONT);
	HBITMAP hBmp = (HBITMAP)(GetDocument()->GetBitmap());
	CSize* pSize = GetDocument()->GetBitmapSize();

	ASSERT(hBmp);	
	ASSERT(pSize);

	if(!hBmp || !pSize)
	{
		CString str;
		str.LoadString(IDS_ERROR_BITMAP_LOAD);
		CRect rct;
		GetClientRect(rct);
		pDC->DrawText(str,rct,DT_CENTER);
		return;
	}

	CDC dc2;
	dc2.CreateCompatibleDC(pDC);
	dc2.SelectObject(hBmp);
	pDC->BitBlt(2,2,pSize->cx,pSize->cy,&dc2,0,0,SRCCOPY);
}

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpView diagnostics

#ifdef _DEBUG
void CBmpView::AssertValid() const
{
	CScrollView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CBmpView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpView message handlers

void CBmpView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	RecalcScrolls();
}

///////////////////////////////////////////////////////////////////////////////

void CBmpView::RecalcScrolls()
{
	HBITMAP hBmp = (HBITMAP)(GetDocument()->GetBitmap());
	if(hBmp)
	{
		CSize* pSize = GetDocument()->GetBitmapSize();
		ASSERT(pSize);

		SetScrollSizes(MM_TEXT,*pSize);
	}
	else
	{
		SetScrollSizes(MM_TEXT,CSize(0,0));
	}
}

///////////////////////////////////////////////////////////////////////////////

void CBmpView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_RESVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

CBmpDoc* CBmpView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CBmpDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelView

IMPLEMENT_DYNCREATE(CAccelView, CListView)

///////////////////////////////////////////////////////////////////////////////

CAccelView::CAccelView()
{
}

///////////////////////////////////////////////////////////////////////////////

CAccelView::~CAccelView()
{
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CAccelView, CListView)
	//{{AFX_MSG_MAP(CAccelView)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelView drawing

void CAccelView::OnDraw(CDC* pDC)
{
}

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelView diagnostics

#ifdef _DEBUG
void CAccelView::AssertValid() const
{
	CListView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CAccelView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelView message handlers

void CAccelView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CAccelDoc* pDoc = GetDocument();
	if(pDoc->GetRawData()){
		CListCtrl& lc = GetListCtrl();
		lc.DeleteAllItems();
		int numItems = pDoc->GetRawDataSize() / (sizeof(ACCELTABLEENTRY));
		ACCELTABLEENTRY* pAccel = (ACCELTABLEENTRY*)pDoc->GetRawData();
		for(int x = 0;x<numItems;x++){
			CString strID, strKey, strFlags, strTemp;
			strID.Format(_T("0x%0.4X (%d)"),pAccel->wId,pAccel->wId);
			lc.InsertItem(x,strID);

			if(pAccel->fFlags & FCONTROL) strKey.LoadString(IDS_ACCEL_CTRL_PLUS);
			if(pAccel->fFlags & FSHIFT) strKey.LoadString(IDS_ACCEL_SHIFT_PLUS);
			if(pAccel->fFlags & FALT) strKey.LoadString(IDS_ACCEL_ALT_PLUS);
								
			if(pAccel->fFlags & FVIRTKEY){
				pDoc->GetVirtKeyString(pAccel->wAnsi,strTemp);
			}
			else{
				if(pAccel->wAnsi < 0x32)
					strTemp.Format(IDS_ACCEL_ASCII_CTRL,(pAccel->wAnsi + 0x40));
				else
					strTemp.Format(_T("%c"),pAccel->wAnsi);
			}

			strKey += strTemp;
			lc.SetItem(x,1,LVIF_TEXT,strKey,0,0,0,0);
			pAccel++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CAccelView::OnInitialUpdate() 
{
	CListCtrl& lc = GetListCtrl();
	lc.ModifyStyle(0,LVS_REPORT);
	lc.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	CString str;
	str.LoadString(IDS_ACCEL_HEAD_ID);
	lc.InsertColumn(0,str,LVCFMT_LEFT,100,0);
	str.LoadString(IDS_ACCEL_HEAD_KEY);
	lc.InsertColumn(1,str,LVCFMT_LEFT,200,1);
	CListView::OnInitialUpdate();
}

///////////////////////////////////////////////////////////////////////////////

void CAccelView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu mnu;
	if(!mnu.LoadMenu(IDM_CTXT_RESVIEW))
	{
		ASSERT(FALSE);
		return;
	}

	CMenu *pCtxt = mnu.GetSubMenu(0);
	pCtxt->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

///////////////////////////////////////////////////////////////////////////////

CAccelDoc* CAccelView::GetDocument()
{
	CDocument* pDoc = CView::GetDocument();
	ASSERT(pDoc);
	return (CAccelDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
