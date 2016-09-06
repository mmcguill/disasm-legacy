///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "TextView.h"

///////////////////////////////////////////////////////////////////////////////

#define DEFAULT_FONTFACE	_T("Courier New")
#define DEFAULT_FONTSIZE	10
#define COL_CURLINE			(RGB(200,200,255))
#define COL_SEL_BG_FOCUS	(RGB(49,106,197))
#define COL_SEL_BG_NO_FOCUS (RGB(122,150,223))
#define SELECTED_TEXT_COLOR (RGB(255,255,255))

// Display purposes allow some room between text columns

#define AUTOCALC_COLUMN_EXTRA_WIDTH (3 * m_szChar.cx)
#define WC_MCGUILL_TEXTVIEW			_T("McGuill_TextView")

#define WS_MCGUILL_TEXTVIEW			(	AFX_WS_DEFAULT_VIEW |	\
										WS_VSCROLL |			\
										WS_HSCROLL |			\
										WS_CLIPCHILDREN)

#define WM_HDRCHANGE_RECALCSCROLLS	WM_USER + 1

///////////////////////////////////////////////////////////////////////////////

#define DATA_ALLOC_GRANULARITY (4 * 1024)	// 4MB For Moment?

///////////////////////////////////////////////////////////////////////////////

// TODO: Place this somewhere maintainable

#define IDC_HEADER 8798

IMPLEMENT_DYNAMIC(CTextViewHeaderCtrl, CHeaderCtrl)
BEGIN_MESSAGE_MAP(CTextViewHeaderCtrl, CHeaderCtrl)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CTextView

IMPLEMENT_DYNCREATE(CTextView, CCtrlView)

///////////////////////////////////////////////////////////////////////////////

CTextView::CTextView() : CCtrlView(WC_MCGUILL_TEXTVIEW, WS_MCGUILL_TEXTVIEW)
{
	m_fInit = FALSE;
	m_ViewMode = viewModeViewOnly;
	m_fSelectionPossible = FALSE;
	m_fDeferAddLine = FALSE;
	m_fShowHeader = TRUE;

	m_szDoc.SetSize(0,0);
	m_szPage.SetSize(0,0);
	m_szChar.SetSize(0,0);

	m_fInCapture = FALSE;
	m_fAllowMouseSelectionStart = FALSE;
	m_fKeyBdSelectionHasStarted = FALSE;
	m_nSelectionCaretColumn = 0;

	m_posLogCaret.col = 0;
	m_posLogCaret.row = 0;
	m_posLogCaret.hdrCol = 0;

	ZeroMemory(&m_selInfo,sizeof(SELECTION_INFO));
	ZeroMemory(&m_prevSI,sizeof(SELECTION_INFO));
	m_pFont = NULL;
	m_pBrshHilightLine = NULL;

	m_cyHdr = 0;
	m_uScrollWheelLines = 0;
	m_pszClip = NULL;

	m_lpbData = NULL;
	m_cbData = 0;
	m_cbDataUsed = 0;
	m_cchMaxLine = 128;
	m_cLines = 0;
	m_cColumns = 0;

	m_lpbLineMap = NULL;
	m_cbLineData = 0;
}

///////////////////////////////////////////////////////////////////////////////

CTextView::~CTextView()
{
	Cleanup();
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::Cleanup()
{
	if(m_pFont)
	{
		delete m_pFont;
	}

	if(m_pBrshHilightLine)
	{
		delete m_pBrshHilightLine;
	}

	if(m_pszClip)
	{
		delete []m_pszClip;
	}

	//for(vector<CTextViewLine*>::iterator theLine = m_vecLines.begin();
	//	theLine!= m_vecLines.end();theLine++)
	//{
	//	if((*theLine))
	//	{
	//		delete(*theLine);
	//	}
	//}
	//m_vecLines.clear();

	if(m_lpbData)
	{
		::VirtualFree(m_lpbData,m_cbData,MEM_RELEASE);
		m_lpbData = NULL;
		m_cbData = 0;
		m_cbDataUsed = 0;
		m_cLines = 0;
		//m_mapLine2Mem.clear();
	}

	if(m_lpbLineMap)
	{
		::VirtualFree(m_lpbLineMap,m_cbLineData,MEM_RELEASE);
		m_lpbLineMap = 0;
		m_cbLineData = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTextView, CCtrlView)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_NOTIFY(HDN_ENDTRACK, IDC_HEADER, OnEndTrack)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_HDRCHANGE_RECALCSCROLLS,OnHdrChangeRecalcScrolls)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// CTextView diagnostics

#ifdef _DEBUG
void CTextView::AssertValid() const
{
	CCtrlView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::Dump(CDumpContext& dc) const
{
	CCtrlView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

int CTextView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCtrlView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc;
	rc.SetRectEmpty();

	if(!m_hdr.Create(CCS_TOP | WS_CHILD | HDS_HORZ,rc,this,IDC_HEADER))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	
	// Set Font

	::SendMessage(m_hdr.m_hWnd,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELONG(TRUE,0));

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Register Class

	WNDCLASS wndClass = {0};
	if(!GetClassInfo(AfxGetInstanceHandle(),WC_MCGUILL_TEXTVIEW,&wndClass))
	{
		wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndClass.hCursor = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);
		wndClass.hInstance = AfxGetInstanceHandle();
		wndClass.lpfnWndProc = ::DefWindowProc;
		wndClass.lpszClassName = WC_MCGUILL_TEXTVIEW;
		wndClass.style = CS_DBLCLKS;

		if(!AfxRegisterClass(&wndClass))
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}
	
	return CCtrlView::PreCreateWindow(cs);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnPaint()
{
    CView::OnPaint();
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnSize(UINT nType, int cx, int cy)
{
	CCtrlView::OnSize(nType, cx, cy);

	// Resize Header
	
	LayoutHeader();

	RecalcScrolls(FALSE,FALSE);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnEndTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	PostMessage(WM_HDRCHANGE_RECALCSCROLLS,0,0);	
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CTextView::OnHdrChangeRecalcScrolls(WPARAM wParam, LPARAM lParam)
{
	RecalcScrolls(TRUE,FALSE);
	Invalidate();
	UpdateWindow();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int CTextView::GetHeaderHeight()
{
	CRect rc;
	m_hdr.GetClientRect(rc);
	
	return rc.bottom;
}

///////////////////////////////////////////////////////////////////////////////

CPoint CTextView::GetCaretPixelPos()
{
	CPoint pt;
    pt.x = m_hdr.GetItemLeft(m_posLogCaret.hdrCol) + 
				(m_posLogCaret.col * m_szChar.cx);

	pt.y = m_cyHdr + 
				(m_posLogCaret.row * m_szChar.cy);

	return pt;
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::SetRealCaretPosFromLog()
{
	CPoint ptCaret = GetCaretPixelPos();
	CPoint ptBase = GetScrollPosition();
	
	ptCaret-=ptBase;

	SetCaretPos(ptCaret);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnSetFocus(CWnd* pOldWnd)
{
	CCtrlView::OnSetFocus(pOldWnd);

	/*if(m_vecLines.size())*/
	if(m_cLines)
	{
        ::CreateCaret(this->m_hWnd,NULL,0,m_szChar.cy);

		SetRealCaretPosFromLog();

		if(viewModeCaret == m_ViewMode)
		{
			ShowCaret();
		}
		else
		{
			HideCaret();
		}
	
		// colors will change => redraw 

		Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnKillFocus(CWnd* pNewWnd)
{
	CCtrlView::OnKillFocus(pNewWnd);

	/*if(m_vecLines.size())*/
	if(m_cLines)
	{
		//m_ptCaret = GetCaretPos();

		DestroyCaret();

		// colors will change => redraw 

		Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::ClearLines()
{
	//for(vector<CTextViewLine*>::iterator theLine = m_vecLines.begin();
	//	theLine!= m_vecLines.end();theLine++)
	//{
	//	if((*theLine))
	//	{
	//		delete(*theLine);
	//	}
	//}
	//m_vecLines.clear();

	/*******************/

	m_cbDataUsed = 0;
	m_cLines = 0;
	//m_mapLine2Mem.clear();

	/*******************/

	RecalcScrolls(TRUE,FALSE);

	m_posLogCaret.col = 0;
	m_posLogCaret.hdrCol = 0;
	m_posLogCaret.row = 0;

	SetRealCaretPosFromLog();

	Invalidate();
	UpdateWindow();

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::Init(	CTextView::ViewMode viewMode/*= CTextView::viewModeCaret*/,
						BOOL fSelectionPossible /*= FALSE*/,
						LPCTSTR lpszFontFace /*=NULL*/, 
						int nFontSize/*=0*/, 
						int cColumns /*= 1*/,
						LPCTSTR pColumns[]/*=NULL*/, 
						BOOL fAutoCalcColumnSize /*=TRUE*/)
{
	ASSERT(cColumns > 0);

	// TODO: Reinitialization? = > Not Allowed

	if(m_fInit)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Font

	LOGFONT lf = {0};
	//lf.lfWeight = FW_BOLD;
	lf.lfHeight = -MulDiv(
		((nFontSize == 0) ? DEFAULT_FONTSIZE : nFontSize), 
		GetDC()->GetDeviceCaps(LOGPIXELSY),	72); 

	lstrcpy(lf.lfFaceName,
		((lpszFontFace == NULL) ? DEFAULT_FONTFACE : lpszFontFace)); 

	m_pFont = new CFont;
	ASSERT(m_pFont);
	if(NULL == m_pFont || !m_pFont->CreateFontIndirect(&lf))
	{
		Cleanup();
		return FALSE;
	}

	// Line Hilighting Brush

	m_pBrshHilightLine = new CBrush(COL_CURLINE);
	ASSERT(m_pFont);
	if(NULL == m_pBrshHilightLine)
	{
		Cleanup();
		return FALSE;
	}

	// Get size of one character of this font
	// Wrap here to release dc ASAP
	{
		CClientDC dc(this);
		
		dc.SelectObject(m_pFont);
		TEXTMETRIC tm = {0};
		dc.GetTextMetrics(&tm);
		
		m_szChar.cy = tm.tmHeight + tm.tmExternalLeading;
		m_szChar.cx = tm.tmAveCharWidth;

	}
	
	ASSERT(m_szChar.cy);
	ASSERT(m_szChar.cx);
	if(!m_szChar.cy || !m_szChar.cx)
	{
		Cleanup();
		return FALSE;
	}

	// Selection Possible

	m_fSelectionPossible = fSelectionPossible;

	// view mode

	m_ViewMode = viewMode;
	if(m_ViewMode != viewModeCaret)
	{
		m_fSelectionPossible = FALSE;
		ZeroMemory(&m_selInfo,sizeof(SELECTION_INFO));
		ZeroMemory(&m_prevSI,sizeof(SELECTION_INFO));
	}

	// Columns 

	for(int i=0;i<cColumns;i++)
	{
		HDITEM hdi = {0};
		if(pColumns)
		{
			hdi.mask = HDI_TEXT;
			hdi.pszText = (LPTSTR)pColumns[i];
		}

		if(-1 == m_hdr.InsertItem(i,&hdi))
		{
			Cleanup();
			return FALSE;
		}
	}
	m_cColumns = cColumns;

	// Header Tab stops 

	m_pszClip = new CSize[cColumns];
	Assert(m_pszClip);
	if(NULL == m_pszClip)
	{
		Cleanup();
		return FALSE;
	}

	for(int i=0;i<cColumns;i++)
	{
		m_pszClip[i].cx = m_hdr.GetItemLeft(i);
		m_pszClip[i].cy = m_pszClip[i].cx + m_hdr.GetItemWidth(i);
	}


	// Scroll Wheel Num of lines to move

	SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&m_uScrollWheelLines,0);

	m_fInit = TRUE;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::BeginDeferAddLine()
{
	m_fDeferAddLine = TRUE;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::EndDeferAddLine()
{
	m_fDeferAddLine = FALSE;
	RecalcScrolls(TRUE,TRUE);
	Invalidate();
	UpdateWindow();
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::AddLine(LPCTSTR pText[], COLORREF colors[], int cColumns, 
						LPVOID lpvParam /* = NULL */)
{
    ASSERT(m_fInit);
	ASSERT(cColumns == m_hdr.GetItemCount());
	if(!m_fInit || cColumns != m_hdr.GetItemCount())
	{
		return FALSE;
	}


	// Do we need to Allocate or Re-Allocate memory?

	// Line 2 Data Map

	if(m_cLines * sizeof(LPBYTE) >= m_cbLineData)
	{
		if(0 == m_cbLineData)
		{
			m_lpbLineMap = (LPBYTE*)VirtualAlloc(NULL,DATA_ALLOC_GRANULARITY,MEM_COMMIT,PAGE_READWRITE);
			m_cbLineData = DATA_ALLOC_GRANULARITY;
		}
		else // Realloc
		{
			LPBYTE *tmp = (LPBYTE*)VirtualAlloc(NULL,m_cbLineData * 2,MEM_COMMIT,PAGE_READWRITE);
			if(NULL == tmp)
			{
				return FALSE;
			}

			CopyMemory(tmp,m_lpbLineMap,m_cbLineData);
			VirtualFree(m_lpbLineMap,m_cbLineData,MEM_RELEASE);
			
			m_lpbLineMap = tmp;
			m_cbLineData *= 2;
		}

		if(NULL == m_lpbLineMap)
		{
			// TODO: Set Last Error!!
			return FALSE;
		}
	}

	
	// Actual Lines

	if((m_cbDataUsed + 
			((m_cchMaxLine * sizeof(TCHAR) + sizeof(COLORREF)) * m_cColumns) +
			(sizeof(LPVOID))) 
			>= m_cbData)
	{
		if(0 == m_cbData)
		{
			m_lpbData = (LPBYTE)VirtualAlloc(NULL,DATA_ALLOC_GRANULARITY,MEM_COMMIT,PAGE_READWRITE);
			m_cbData = DATA_ALLOC_GRANULARITY;
		}
		else
		{
			// Is there a better way to do this?

			TCHAR szDbg[256];
			_sntprintf(szDbg,255,_T("Mem Before: %d\n"),m_cbData);
			OutputDebugString(szDbg);

			LPBYTE tmp = (LPBYTE)VirtualAlloc(NULL,m_cbData * 2,MEM_COMMIT,PAGE_READWRITE);
			if(NULL == tmp)
			{
				return FALSE;
			}

			CopyMemory(tmp,m_lpbData,m_cbData);
			VirtualFree(m_lpbData,m_cbData,MEM_RELEASE);

			m_lpbData = tmp;
			m_cbData *= 2;
		}

		if(NULL == m_lpbData)
		{
			// TODO: Set Last Error!!
			return FALSE;
		}
	}


	// Add The Line

	LPBYTE lpBase = m_lpbData + m_cbDataUsed;

	int cbUsed = 0;


	// Data

	*((LPVOID*)lpBase) = lpvParam;
	cbUsed += sizeof(LPVOID);

	// Colours

	for(int i=0;i<cColumns;i++)
	{
		LPCOLORREF lpCol = (LPCOLORREF)(lpBase + cbUsed);
		*lpCol = colors[i];
		cbUsed += sizeof(COLORREF);
	}

	
	// Text

	for(int i=0;i<cColumns;i++)
	{
		LPTSTR lpszText = (LPTSTR)(lpBase + cbUsed);

		_tcsncpy(lpszText,pText[i],m_cchMaxLine);
		lpszText[m_cchMaxLine-1] = 0;
		cbUsed += ((_tcslen(lpszText) + 1) * sizeof(TCHAR));
	}

	m_cbDataUsed += cbUsed;




	//TCHAR szDbg[256];
	//_sntprintf(szDbg,255,_T("cLines: %d\n"),m_cLines);
	//OutputDebugString(szDbg);

	//m_mapLine2Mem.insert(std::map<int,LPBYTE>::value_type(m_cLines,lpBase));

	m_lpbLineMap[m_cLines] = lpBase;
	m_cLines++;


	// if this is our first line && we have focus => create our caret

	if((m_cLines == 1) && (GetFocus() == this))
	{
		ASSERT(m_posLogCaret.row == 0 && m_posLogCaret.col == 0 && m_posLogCaret.hdrCol == 0);

		::CreateCaret(this->m_hWnd,NULL,0,m_szChar.cy);
		
		SetRealCaretPosFromLog();

		if(viewModeCaret == m_ViewMode)
		{
			ShowCaret();
		}
		else
		{
			HideCaret();
		}
	}



	if(!m_fDeferAddLine)
	{
		RecalcScrolls(TRUE,TRUE);
		Invalidate();
		UpdateWindow();
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

LPVOID CTextView::GetLineParam(int line) const
{
	Assert(m_cLines);

	if(line < 0 || line >= m_cLines)
	{
		Assert(FALSE);
		return NULL;
	}

	//std::map<int,LPBYTE>::const_iterator theData = m_mapLine2Mem.find(line);

	//if(theData == m_mapLine2Mem.end())
	//{
	//	Assert(FALSE);
	//	return NULL;
	//}

	LPVOID *plpv = (LPVOID*)m_lpbLineMap[line]; //(theData->second);

	return *plpv;
}

///////////////////////////////////////////////////////////////////////////////

COLORREF CTextView::GetColumnColour(int line,int col) const
{
	Assert(m_cLines);
	Assert(col < m_cColumns);

	if(line < 0 || line >= m_cLines)
	{
		Assert(FALSE);
		return NULL;
	}

	LPBYTE lpBase = (LPBYTE)m_lpbLineMap[line];

	// Skip Data

	lpBase += sizeof(LPVOID);

	LPCOLORREF lpCol = (LPCOLORREF)(lpBase + (sizeof(COLORREF) * col));

	return *lpCol;
}

///////////////////////////////////////////////////////////////////////////////

LPTSTR CTextView::GetColumnText(int line,int col) const
{
	Assert(m_cLines);

	if(line < 0 || line >= m_cLines)
	{
		Assert(FALSE);
		return NULL;
	}

	LPBYTE lpBase = (LPBYTE)m_lpbLineMap[line];
	
	// Skip Data

	lpBase += sizeof(LPVOID);
	
	// Skip Colors

	LPTSTR lpszLine = (LPTSTR)(lpBase + (sizeof(COLORREF) * m_cColumns));

	
	// skip col strings

	int j=0;
	for(int i=0;i<col;i++)
	{
		while(lpszLine[j] != 0)
		{
			j++;
		}

		j++; // Next String
	}

	return &lpszLine[j];
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::RecalcScrolls(BOOL fRecalcDocSize, BOOL fSetColumnsSizeToFitText)
{
	if(!m_fInit)
	{
		m_szDoc.cx = m_szDoc.cy = 0;
		
		SCROLLINFO si = {0};
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.cbSize = sizeof(SCROLLINFO);
		SetScrollInfo(SB_VERT,&si);
		SetScrollInfo(SB_HORZ,&si);
		return;
	}

	if(fRecalcDocSize)
	{
		/* TODO: move this to addline and calc on each new line? */

		for(int i=0;i<m_cColumns;i++)
		{

			int maxcch = 0;
			int cch = 0;
	        
			for(int j=0;j<m_cLines;j++)
			{
				LPTSTR lpszText = GetColumnText(j,i);
				cch = _tcslen(lpszText);
				maxcch = (cch > maxcch) ? cch : maxcch;
			}
		
			HD_ITEM hdi = {0};
			hdi.mask = HDI_LPARAM | (fSetColumnsSizeToFitText ? HDI_WIDTH : 0);
			hdi.lParam = (maxcch * m_szChar.cx) +  
				((i==m_cColumns-1) ? 0 : (AUTOCALC_COLUMN_EXTRA_WIDTH));

			hdi.cxy = hdi.lParam;

			m_hdr.SetItem(i,&hdi);
		}

		// Size = max width of last column + left

		int nMaxWidth = m_hdr.GetItemMaxWidth(m_cColumns - 1);
		int nWidth = m_hdr.GetItemWidth(m_cColumns - 1);
		int nLeft = m_hdr.GetItemLeft(m_cColumns - 1);

		m_szDoc.cx = nWidth + nLeft;
		m_szDoc.cy = m_cLines * m_szChar.cy;
	}

	// Resize Header ? if asked

	if(fSetColumnsSizeToFitText)
	{
		LayoutHeader();
	}

	CRect rc;
	GetTrueClientRect(rc);
	m_szPage.cx = rc.Width();
	m_szPage.cy = rc.Height();

	SCROLLINFO si = {0};
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.cbSize = sizeof(SCROLLINFO);
	si.nMax = m_szDoc.cx;
	si.nPage = m_szPage.cx;
	SetScrollInfo(SB_HORZ,&si);

	si.nMax = m_szDoc.cy;
	si.nPage = m_szPage.cy;
	SetScrollInfo(SB_VERT,&si);

	
	m_cyHdr = m_fShowHeader ? GetHeaderHeight() : 0;

	int cHdrs = m_hdr.GetItemCount();
	for(int i=0;i<cHdrs;i++)
	{
		m_pszClip[i].cx = m_hdr.GetItemLeft(i);
		m_pszClip[i].cy = m_pszClip[i].cx + m_hdr.GetItemWidth(i);
	}

	// Caret may need to be adjusted when scrolls are recalced
	// EG maximize will remove scrolls but caret will stay at 
	// same window location

	SetRealCaretPosFromLog();
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	if(!pDC->IsPrinting())
	{
		CPoint pt = -GetScrollPosition();
		pDC->SetViewportOrg(pt);
	}
	else
	{
		pDC->SetViewportOrg(CPoint(0,0));
	}

	CCtrlView::OnPrepareDC(pDC, pInfo);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnDraw(CDC* pDC)
{
    if(!m_fInit || !m_cLines)
	{
		return;
	}

	pDC->SelectObject(m_pFont);


	CRect rct;
	pDC->GetClipBox(&rct);

	// Set Up Background
	
	pDC->SetBkMode(TRANSPARENT);

	// Draw Selection BG

	if(m_fSelectionPossible && m_selInfo.fSelected)
	{
		DrawSelection(pDC);
	}

	// If view mode = line draw line

	if(m_ViewMode == viewModeLine)
	{
		HilightLine(pDC,rct,m_posLogCaret.row);
	}

	// Draw Text

	//TCHAR szDbg[256];
	//_sntprintf(szDbg,255,_T("OnDraw => GetClipBox() -> [%d,%d,%d, %d]\n"),rct.left,rct.top,rct.right,rct.bottom);
	//OutputDebugString(szDbg);

	int nLineBegin = rct.top / m_szChar.cy;
	int nLineEnd =  rct.bottom / m_szChar.cy;

	// Allow a one line margin outside visible bounds for partial line vis

	nLineBegin = (0 == nLineBegin) ? 0 : (nLineBegin - 1);
	nLineEnd = nLineEnd+1;

	// Ensure begin line & end line are not out of range

	nLineBegin = (nLineBegin > (int)(m_cLines - 1)) ? 
				(m_cLines - 1) : nLineBegin;

	nLineEnd = (nLineEnd > (int)(m_cLines - 1)) ? 
				(m_cLines - 1) : nLineEnd;


	// Setup Clipping Rects for each column

	int cHdrs = m_hdr.GetItemCount();

	CRect rcClip;

	for(int i=0;i<cHdrs;i++)
	{
		int top = m_szChar.cy * nLineBegin + m_cyHdr;
		int bottom = top + m_szChar.cy;

		for(int x=nLineBegin;x<=nLineEnd;x++)
		{
			rcClip.SetRect(m_pszClip[i].cx,top,m_pszClip[i].cy,bottom);

			DrawColumnLine(pDC,rcClip,i,x);

			top += m_szChar.cy;
			bottom += m_szChar.cy;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::DrawColumnLine(CDC* pDC, CRect& rcClip, int col, int line)
{
	ASSERT(pDC);
	ASSERT(m_fInit);

	// Clipping Rect 

	LPCTSTR lpszText = GetColumnText(line,col);
	int len = _tcslen(lpszText);

	COLORREF colorDef = GetColumnColour(line,col);

	// is there a selection or is the selection in this column? 
	// is the line inside selection
	// no => Draw and bail

	int top = min(m_selInfo.lineBegin,m_selInfo.lineEnd);
	int bottom = max(m_selInfo.lineBegin,m_selInfo.lineEnd);

	COLORREF colorLine = 
		(!m_selInfo.fSelected || col != m_selInfo.hdrCol || 
		(line < top || line > bottom)) ?	colorDef : 
											SELECTED_TEXT_COLOR;

	if(!(m_selInfo.fSelected && 
		col == m_selInfo.hdrCol && 
		(line == top || line == bottom)))
	{
		pDC->SetTextColor(colorLine);
		pDC->ExtTextOut(rcClip.left,rcClip.top,ETO_CLIPPED,rcClip,
							lpszText,len,NULL);	

		return;
	}

	//  First or last line of selection? 

	BOOL fMultiLine = (top != bottom);
	BOOL fForward = (fMultiLine) ? 
						(m_selInfo.lineEnd > m_selInfo.lineBegin) : 
						(m_selInfo.startChar <= m_selInfo.endChar);

	
	int beginch = 0;
	int endch = 0;
	
	if(line == m_selInfo.lineBegin)
	{
		// Break Line Up into selected & unselected

		if(fForward)
		{
			beginch = m_selInfo.startChar;
			endch = (fMultiLine) ? len : m_selInfo.endChar;
		}
		else
		{
			beginch = (fMultiLine) ? 0 : m_selInfo.endChar;
			endch = m_selInfo.startChar;
		}
	}
	else if(fMultiLine && line == m_selInfo.lineEnd)
	{
		// Break Line Up into selected & unselected

		beginch = (fForward) ? 0 : m_selInfo.endChar;
		endch = (fForward) ? m_selInfo.endChar : len;
	}
	else
	{
		ASSERT(FALSE); // WTF?
		return;
	}

	// beginning unselected first

	pDC->SetTextColor(colorDef);
	ASSERT(beginch >=0);
	if(beginch >=0)
	{
		pDC->ExtTextOut(rcClip.left,rcClip.top,
			ETO_CLIPPED,rcClip,	lpszText,beginch,NULL);	
	}

	
	// selected

	pDC->SetTextColor(SELECTED_TEXT_COLOR);
	ASSERT(endch >= beginch);
	if(endch >= beginch)
	{
		pDC->ExtTextOut(
			rcClip.left + (beginch * m_szChar.cx),
			rcClip.top,ETO_CLIPPED,	rcClip,	&lpszText[beginch],
			endch - beginch, NULL);
	}

	
	// ending unselected

	pDC->SetTextColor(colorDef);
	ASSERT(len >= endch);
	if(len >= endch)
	{
		pDC->ExtTextOut(
			rcClip.left + (endch * m_szChar.cx),
			rcClip.top,ETO_CLIPPED,rcClip,&lpszText[endch],
			len-endch,NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::HilightLine(CDC* pDC, CRect& rcClip, int line, 
							LPCOLORREF pColor /* = NULL */)
{
	CRect rc;
	
	rc.left = rcClip.left;
	rc.right = rcClip.right;
	rc.top = (line * m_szChar.cy) + m_cyHdr;
	rc.bottom = rc.top + m_szChar.cy;

	if(NULL == pColor)
	{
		pDC->FillRect(rc,m_pBrshHilightLine);
	}
	else
	{
		CBrush* pBrsh = new CBrush(*pColor);
		ASSERT(pBrsh);
	
		if(NULL == pBrsh)
		{
			return;
		}

		pDC->FillRect(rc,pBrsh);

		delete pBrsh;
	}
}

/////////////////////////////////////////////////////////////////////

void CTextView::DrawSelection(CDC* pDC)
{
	ASSERT(pDC);
	ASSERT(m_fInit);
	ASSERT(m_selInfo.fSelected);

	if(!m_selInfo.fSelected)
	{
		return;
	}

	CBrush* pBrsh = new CBrush((GetFocus() == this) ? 
		COL_SEL_BG_FOCUS : COL_SEL_BG_NO_FOCUS);

	ASSERT(pBrsh);
	if(NULL == pBrsh)
	{
		return;
	}

	// Draw Start & End Line Selections then lines in between

	BOOL fForward;
	BOOL fMultiLine;

	// backward or forward?

	if(m_selInfo.lineEnd != m_selInfo.lineBegin) // multiline
	{
		fForward = (m_selInfo.lineEnd > m_selInfo.lineBegin);
		fMultiLine = TRUE;
	}
	else
	{
		fForward = (m_selInfo.startChar <= m_selInfo.endChar);
		fMultiLine = FALSE;
	}

	// start line sel

	// Clip To Header Width

	int maxright = m_hdr.GetItemLeft(m_selInfo.hdrCol) + 
		m_hdr.GetItemWidth(m_selInfo.hdrCol);

	CRect rc;
	rc.top = m_selInfo.lineBegin * m_szChar.cy + m_cyHdr;
	rc.bottom = rc.top + m_szChar.cy;

	rc.left = m_hdr.GetItemLeft(m_selInfo.hdrCol) + 
		((fForward) ? 
			(m_szChar.cx * m_selInfo.startChar) : 
			((fMultiLine) ? 0 : m_selInfo.endChar * m_szChar.cx));


	rc.right = m_hdr.GetItemLeft(m_selInfo.hdrCol) + 
		((fForward) 
			? 
			((fMultiLine) ? 
			(/*m_vecLines[m_selInfo.lineBegin]->GetColumnTextLength(m_selInfo.hdrCol)*/
				_tcslen(GetColumnText(m_selInfo.lineBegin,m_selInfo.hdrCol))
				* m_szChar.cx)
				: 
			(m_szChar.cx * m_selInfo.endChar))

			:
			
			(m_szChar.cx * m_selInfo.startChar));

	
	rc.right = (rc.right > maxright) ? maxright : rc.right;
	pDC->FillRect(rc,pBrsh);

	// only draw end if its multiline

	if(fMultiLine)
	{
		rc.top = m_selInfo.lineEnd * m_szChar.cy + m_cyHdr;
		rc.bottom = rc.top + m_szChar.cy;

		rc.left = (fForward) ?
			m_hdr.GetItemLeft(m_selInfo.hdrCol) :
			m_hdr.GetItemLeft(m_selInfo.hdrCol) + (m_szChar.cx * m_selInfo.endChar);

		rc.right = m_hdr.GetItemLeft(m_selInfo.hdrCol) + 
			((fForward) ?
			(m_szChar.cx * m_selInfo.endChar) :
		    (/*m_vecLines[m_selInfo.lineEnd]->GetColumnTextLength(m_selInfo.hdrCol)*/
				_tcslen(GetColumnText(m_selInfo.lineEnd,m_selInfo.hdrCol))
				* m_szChar.cx));

		rc.right = (rc.right > maxright) ? maxright : rc.right;
		pDC->FillRect(rc,pBrsh);
	}
	
	// Contained lines

	for(int i=min(m_selInfo.lineBegin, m_selInfo.lineEnd)+1;
		i<max(m_selInfo.lineBegin, m_selInfo.lineEnd);	i++)
	{
		rc.top = i*m_szChar.cy + m_cyHdr;
		rc.bottom = rc.top + m_szChar.cy;
		rc.left = m_hdr.GetItemLeft(m_selInfo.hdrCol);
		int cch = _tcslen(GetColumnText(i,m_selInfo.hdrCol));
		rc.right = rc.left + (cch*m_szChar.cx);
		rc.right = (rc.right > maxright) ? maxright : rc.right;
		pDC->FillRect(rc,pBrsh);	
	}
			
	delete pBrsh;
}

/////////////////////////////////////////////////////////////////////

int CTextView::GetHeaderColumn(int x, LONG *pColumn /* = NULL */)
{
	ASSERT(m_fInit);
	if(!m_fInit)
	{
		return 0;
	}

    for(int i=m_hdr.GetItemCount()-1;i>=0;i--)
	{
		if(x >= m_hdr.GetItemLeft(i))
		{
			if(pColumn)
			{
				x -= m_hdr.GetItemLeft(i);

				// Nice Rounding up or down
				*pColumn = (x / m_szChar.cx);
				*pColumn += ((x % m_szChar.cx) > (m_szChar.cx / 2)) ? 1 : 0;
			}

			return i;
		}
	}

	// Not found

	if(pColumn)
	{
		*pColumn = 0;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTextView::SetCurrentLine(int line)
{
	// Set And Scroll into view

	LOG_CARET_POS lcp;
	lcp.col		= m_posLogCaret.col;
	lcp.hdrCol	= m_posLogCaret.hdrCol;
	lcp.row		= line;

	MoveCaretToLogPos(&lcp,SelectionNone);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CTextView::MoveCaretToLogPos(PLOG_CARET_POS plcp,SelectionStatus status,
							CaretMoveType cmt/* = CaretMoveNone*/)
{
	ASSERT(m_ViewMode != viewModeViewOnly);
	ASSERT(m_fInit);
	ASSERT(plcp);
	ASSERT(m_cLines);

    // Limit Caret Row

	//TCHAR szDbg[256];
	//_sntprintf(szDbg,255,_T("MoveCaret => plcp->row: %d\n"),plcp->row);
	//OutputDebugString(szDbg);	
	
	plcp->row = ((plcp->row) < 0) ? 0 : plcp->row; //min
	plcp->row = (plcp->row > (int)m_cLines - 1) ? 
					(m_cLines - 1) : plcp->row; // max


	// Limit Column Header

	int maxColHdr = m_hdr.GetItemCount() - 1;
	plcp->hdrCol = (plcp->hdrCol > maxColHdr) ? maxColHdr : plcp->hdrCol;
	plcp->hdrCol = (plcp->hdrCol < 0) ? 0 : plcp->hdrCol;
	//
	//_sntprintf(szDbg,255,_T("MoveCaret => plcp->row[3]: %d\n"),plcp->row);
	//OutputDebugString(szDbg);

	// Limit Caret Column 
	
	if(m_fSelectionPossible && SelectionInProgress == status)
	{
		if(plcp->hdrCol < m_nSelectionCaretColumn)
		{
			plcp->col = 0;
			
		}
		else if(plcp->hdrCol > m_nSelectionCaretColumn)
		{
			int len = _tcslen(GetColumnText(plcp->row,m_nSelectionCaretColumn));
			plcp->col = len;

			//TCHAR szDbg[256];
			//_sntprintf(szDbg,255,_T("MoveCaret => len: %d\n"),len);
			//OutputDebugString(szDbg);
		}
		else // Edge Case Here but it does happen
		{
			plcp->col = (plcp->col < 0) ? 0 : plcp->col; // min
			int len = _tcslen(GetColumnText(plcp->row,m_nSelectionCaretColumn));
			plcp->col = (plcp->col > len) ? len : plcp->col; // max
		}

		plcp->hdrCol = m_nSelectionCaretColumn;
	}
	else
	{
		plcp->col = (plcp->col < 0) ? 0 : plcp->col; // min
		int len = _tcslen(GetColumnText(plcp->row,plcp->hdrCol));
		plcp->col = (plcp->col > len) ? len : plcp->col; // max
	}


	// If we're entering or leaving selection store
		
	if(m_fSelectionPossible)
	{
		if(SelectionStart == status)
		{
			m_selInfo.fSelected = FALSE;
			m_selInfo.lineBegin = plcp->row;
			m_selInfo.lineEnd = plcp->row;
			m_selInfo.startChar = plcp->col;
			m_selInfo.endChar = plcp->col;
			m_selInfo.hdrCol = plcp->hdrCol;
		}
		else if(SelectionInProgress == status)
		{
			m_selInfo.lineEnd = plcp->row;
			m_selInfo.endChar = plcp->col;

			// Is there a selection?
			if(m_selInfo.lineBegin != m_selInfo.lineEnd || 
				(m_selInfo.endChar != m_selInfo.startChar &&
				m_selInfo.lineBegin == m_selInfo.lineEnd))
			{
				m_selInfo.fSelected = TRUE;
			}
			else
			{
				m_selInfo.fSelected = FALSE;
			}

			// Need to repaint
		
			InvalidateSelectionInProgress();
		}
		else if(SelectionNone == status)
		{
			if(m_selInfo.fSelected)
			{
				m_selInfo.fSelected = FALSE;
				Invalidate();
			}
		}
	}

	
	// Clear Old And New Line

	if(m_ViewMode == viewModeLine)
	{
		InvalidateCurrentLine();
		m_posLogCaret.row = plcp->row;
		InvalidateCurrentLine();
	}

	
	// Final Caret Position

	m_posLogCaret.col =		plcp->col;
	m_posLogCaret.hdrCol =	plcp->hdrCol;
	m_posLogCaret.row  =	plcp->row;

	//_sntprintf(szDbg,255,_T("MoveCaret => m_posLogCaret.row: %d\n\n"),m_posLogCaret.row);
	//OutputDebugString(szDbg);	

	SetRealCaretPosFromLog();
	
	// Scroll caret into view

	ScrollCaretIntoView(cmt);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::ScrollCaretIntoView(CaretMoveType cmt)
{
	ASSERT(m_ViewMode != viewModeViewOnly);
	ASSERT(m_fInit);

	CRect rcWnd;
	GetTrueClientRect(rcWnd);
	
	CPoint ptBase = GetScrollPosition();

	// Vertical

	CPoint ptScrollTo = ptBase;

	int nTopMostVisLine = ptBase.y / m_szChar.cy;
	int cLinesVisible = rcWnd.Height() / m_szChar.cy;

	TCHAR szDbg[256];

	//_sntprintf(szDbg,255,
	//	_T("ScrollCaretIntoView => nTopMostVisLine: %d rcWnd.Height(): %d / m_szChar.cy: %d cLinesVisible: %d\n"),
	//	nTopMostVisLine,rcWnd.Height(),m_szChar.cy,cLinesVisible);
	//OutputDebugString(szDbg);

	BOOL fScrollNeeded = FALSE;

	if ((m_posLogCaret.row <= nTopMostVisLine) || 
		(m_posLogCaret.row >= (nTopMostVisLine + cLinesVisible)))
	{
		ptScrollTo.y = m_posLogCaret.row;

		//_sntprintf(szDbg,255,_T("ScrollCaretIntoView => ptScrollTo.y %d\n"),ptScrollTo.y);
		//OutputDebugString(szDbg);

		ptScrollTo.y *= m_szChar.cy;

		// For Example m_posLogCaretRow == 0 and nTopMostVisLine == 0
		// we get needless scrolling and invalidation...

		if(ptScrollTo.y != ptBase.y)
		{
			fScrollNeeded = TRUE;
		}
	}
	
	// Horizontal
	
	int nRightMostVisCol = 0;

	int cx = rcWnd.Width() - m_szChar.cx;
	int nRightMostVisColHdr = GetHeaderColumn(cx,(LONG*)&nRightMostVisCol);

	//_sntprintf(szDbg,255,_T("ScrollCaretIntoView => nRightMostVisColHdr: %0.4d  nRightMostVisCol: %0.4d\n"),nRightMostVisColHdr,nRightMostVisCol);
	//OutputDebugString(szDbg);

	if(m_posLogCaret.hdrCol == nRightMostVisColHdr)
	{
		if(m_posLogCaret.col > nRightMostVisCol)
		{
			ptScrollTo.x = m_posLogCaret.col;

			ptScrollTo.x *= m_szChar.cx;

			if(ptScrollTo.x != ptBase.x)
			{
				fScrollNeeded = TRUE;
			}
		}
	}

    if(0 == m_posLogCaret.hdrCol)
	{
		int nLeftMostVisCol = ptBase.x / m_szChar.cx;

		//_sntprintf(szDbg,255,_T("ScrollCaretIntoView => nLeftMostVisCol: %d\n"),nLeftMostVisCol);
		//OutputDebugString(szDbg);

		if(m_posLogCaret.col < nLeftMostVisCol)
		{
			ptScrollTo.x = m_posLogCaret.col;
			
			ptScrollTo.x *= m_szChar.cx;

			if(ptScrollTo.x != ptBase.x)
			{
				fScrollNeeded = TRUE;
			}
		}
	}

	if(fScrollNeeded)
	{
		ScrollToPosition(ptScrollTo);
	}
}

/////////////////////////////////////////////////////////////////////

void CTextView::InvalidateCurrentLine()
{
	InvalidateLine(m_posLogCaret.row);
}

/////////////////////////////////////////////////////////////////////

void CTextView::InvalidateLine(int line, int col /* = -1 */)
{
	CRect rc;
	GetTrueClientRect(rc);

	CPoint ptBase = GetScrollPosition();
	ptBase.y -= m_cyHdr;
	rc.top = line * m_szChar.cy;
	rc.bottom = rc.top + m_szChar.cy;
	rc.OffsetRect(-ptBase);

	if(-1 != col)
	{
		rc.left =		m_hdr.GetItemLeft(col) - ptBase.x;
		rc.right =		rc.left + m_hdr.GetItemWidth(col);
		rc.right -= 	ptBase.x;
	}

	InvalidateRect(rc);
}

/////////////////////////////////////////////////////////////////////

void CTextView::InvalidateSelectionInProgress()		
{
    ASSERT(m_fSelectionPossible);
	ASSERT(m_fInit);
	
	/*TCHAR szDbg[256];
	_sntprintf(szDbg,255,_T("InalidateSelInProg => prevSI: [%d , %d] curSI: [%d , %d]\n"),m_prevSI.lineBegin,m_prevSI.lineEnd,m_selInfo.lineBegin,m_selInfo.lineEnd);
	OutputDebugString(szDbg);*/

	// Starting And Ending Lines First

	if(m_prevSI.lineEnd != m_selInfo.lineEnd)
	{
		int top = min(m_prevSI.lineEnd,m_selInfo.lineEnd);
		int bottom = max(m_prevSI.lineEnd,m_selInfo.lineEnd);

		for(int i=top;i<bottom + 1;i++)
		{
			InvalidateLine(i,m_selInfo.hdrCol);
		}
	}
	else if(m_prevSI.endChar != m_selInfo.endChar)
	{
		// You Could Def make this more efficient
		
		//_sntprintf(szDbg,255,_T("InalidateSelInProg => prevSI (Char): [%d , %d] curSI (Char): [%d , %d]\n"),m_prevSI.startChar,m_prevSI.endChar,m_selInfo.startChar,m_selInfo.endChar);
		//OutputDebugString(szDbg);

		InvalidateLine(m_selInfo.lineEnd,m_selInfo.hdrCol);
	}


	if(m_prevSI.lineBegin != m_selInfo.lineBegin)
	{
		int top = min(m_prevSI.lineEnd,m_selInfo.lineEnd);
		int bottom = max(m_prevSI.lineEnd,m_selInfo.lineEnd);

		for(int i=top;i<bottom + 1;i++)
		{
			InvalidateLine(i,m_selInfo.hdrCol);
		}
	}
	else if(m_prevSI.startChar != m_selInfo.startChar)
	{
		// You Could Def make this more efficient

		InvalidateLine(m_selInfo.lineBegin,m_selInfo.hdrCol);
	}

	CopyMemory(&m_prevSI,&m_selInfo,sizeof(SELECTION_INFO));
}

/////////////////////////////////////////////////////////////////////

// Client Co-ords in
void CTextView::ClientToDocument(CPoint& pt)
{
    CPoint ptConv = pt;
	CPoint ptBase = GetScrollPosition();
	pt += ptBase;
	pt.y -= m_cyHdr;

	//TCHAR szDbg[256];
	//_sntprintf(szDbg,255,_T("ClientToDocument => [%d,%d] => [%d,%d]\n"),
	//			ptConv.x,ptConv.y,pt.x,pt.y);

	//OutputDebugString(szDbg);
}

/////////////////////////////////////////////////////////////////////

// Document Co-ords in

void CTextView::SnapToCaretGrid(CPoint& pt, PLOG_CARET_POS plcp)
{
	ASSERT(plcp);

	plcp->hdrCol = GetHeaderColumn(pt.x,&plcp->col);

	plcp->row = (pt.y / m_szChar.cy);

	//TCHAR szDbg[256];
	//_sntprintf(szDbg,255,_T("SnapToCaretGrid => [%d,%d,%d]\n"),
	//			plcp->col,plcp->row,plcp->hdrCol);

	//OutputDebugString(szDbg);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CCtrlView::OnLButtonDown(nFlags, point);

	if(!m_fInit || 0 == m_cLines || m_ViewMode == viewModeViewOnly)
	{
		return;
	}

	LOG_CARET_POS lcp;
	ClientToDocument(point);
    SnapToCaretGrid(point,&lcp);

	MoveCaretToLogPos(&lcp,SelectionNone); // Empty Current Selection

	// Reset in OnMouseMove with lbutton or lbuttonup
	m_fAllowMouseSelectionStart = (m_ViewMode == viewModeCaret); 
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnMouseMove(UINT nFlags, CPoint point)
{
	CCtrlView::OnMouseMove(nFlags, point);

	if(m_ViewMode == viewModeViewOnly)
	{
		return;
	}

	if(m_fInCapture)
	{
		LOG_CARET_POS lcp;
		ClientToDocument(point);
		SnapToCaretGrid(point,&lcp);

		MoveCaretToLogPos(&lcp,SelectionInProgress);
	}
	else if(((MK_LBUTTON  & nFlags) == MK_LBUTTON) && 
			m_fAllowMouseSelectionStart)
	{
		// This is the start of a selection if we have
		// just recieved an lbutton down event also

		m_nSelectionCaretColumn = m_posLogCaret.hdrCol;
		m_fInCapture = TRUE;
		SetCapture();

		LOG_CARET_POS lcp;
		ClientToDocument(point);
		SnapToCaretGrid(point,&lcp);
		
		MoveCaretToLogPos(&lcp,SelectionStart);

		m_fAllowMouseSelectionStart = FALSE; // Reset
	}
	else
	{
        m_fAllowMouseSelectionStart = FALSE; // Reset
	}
	
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CCtrlView::OnLButtonUp(nFlags, point);

	if(m_ViewMode != viewModeCaret)
	{
		return;
	}

    m_fAllowMouseSelectionStart = FALSE; // Reset

	if(m_fInCapture)
	{
		LOG_CARET_POS lcp;
		ClientToDocument(point);
		SnapToCaretGrid(point,&lcp);

		MoveCaretToLogPos(&lcp,SelectionInProgress);
		ReleaseCapture();
		m_fInCapture = FALSE;
	}

	// Possibly bring back selectionend enum?
	
	if(m_selInfo.fSelected) // Allow full repaint
	{	
		Invalidate();
		UpdateWindow();
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!m_fInit)
	{
		return;
	}

	SelectionStatus sel = m_fKeyBdSelectionHasStarted ? 
							SelectionInProgress : SelectionNone;

    switch(nChar)
	{
	case VK_SHIFT:
		m_fKeyBdSelectionHasStarted = TRUE;
		m_nSelectionCaretColumn = m_posLogCaret.hdrCol;

		// perhaps theres already a selection, continue it instead of starting

		if(!m_selInfo.fSelected)
		{
			PerformKeyboardCaretMovement(SelectionStart,CaretMoveNone);
		}
		break;
	case VK_UP:
		PerformKeyboardCaretMovement(sel,CaretMoveLineUp);
		break;
	case VK_DOWN:
		PerformKeyboardCaretMovement(sel,CaretMoveLineDown);
		break;
	case VK_NEXT:
		PerformKeyboardCaretMovement(sel,CaretMovePageDown);
		break;
	case VK_PRIOR:
    	PerformKeyboardCaretMovement(sel,CaretMovePageUp);
		break;
	case VK_END:
		if(GetAsyncKeyState(VK_CONTROL)) // Vertical End
		{
            PerformKeyboardCaretMovement(sel,CaretMoveDocEnd);
		}
		else
		{
			PerformKeyboardCaretMovement(sel,CaretMoveLineEnd);
		}
		break;
	case VK_HOME:
		if(GetAsyncKeyState(VK_CONTROL))
		{
			PerformKeyboardCaretMovement(sel,CaretMoveDocBegin);
		}
		else
		{
			PerformKeyboardCaretMovement(sel,CaretMoveLineBegin);
		}
		break;
	case VK_LEFT:
		PerformKeyboardCaretMovement(sel,CaretMoveCharLeft);
		break;
	case VK_RIGHT:
		PerformKeyboardCaretMovement(sel,CaretMoveCharRight);
		break;
	case VK_TAB:
		// BUGGY: BUG: Select something then tab it skips a column
		// and cancels any current selection

		if(GetAsyncKeyState(VK_SHIFT))
		{
		//	//m_fKeyBdSelectionHasStarted = FALSE;
			PerformKeyboardCaretMovement(sel,CaretMoveTabLeft);
		}
		else
		{
			PerformKeyboardCaretMovement(sel,CaretMoveTabRight);
		}
		break;
	}

	CCtrlView::OnKeyDown(nChar, nRepCnt, nFlags);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!m_fInit)
	{
		return;
	}

	switch(nChar)
	{
	case VK_SHIFT:
		m_fKeyBdSelectionHasStarted = FALSE;
		break;
	}

	CCtrlView::OnKeyUp(nChar, nRepCnt, nFlags);
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::PerformKeyboardCaretMovement(SelectionStatus status,
											 CaretMoveType  cmt )
{
	if(m_ViewMode == viewModeViewOnly) // No Visible Caret or Line
	{
		return;
	}

	ASSERT(m_fInit);

	// Find Current Selection info

	BOOL fForward;
	BOOL fMultiLine;
	if(m_selInfo.lineEnd != m_selInfo.lineBegin) // multiline
	{
		fForward = (m_selInfo.lineEnd > m_selInfo.lineBegin);
		fMultiLine = TRUE;
	}
	else
	{
		fForward = (m_selInfo.startChar <= m_selInfo.endChar);
		fMultiLine = FALSE;
	}

	// Are we canceling a selection?
	// if so different behaviour as to next character

	BOOL fCancelingSelection = (m_selInfo.fSelected && SelectionNone == status);

	LOG_CARET_POS lcp;
	lcp.col =		m_posLogCaret.col;
	lcp.hdrCol =	m_posLogCaret.hdrCol;
	lcp.row		=	m_posLogCaret.row;

	// Perform The Motion

	switch(cmt)
	{
	case CaretMoveLineDown:
		if(fCancelingSelection && !fForward)
		{
			// Reverse Selection Cancel => drop one below sel start line
			lcp.row = m_selInfo.lineBegin;
		}

		lcp.row++;
		break;
	case CaretMoveLineUp:
		if(fCancelingSelection && fForward)
		{
			//Forward Selection Cancel => up one from sel start line
			lcp.row = m_selInfo.lineBegin;
		}

		lcp.row--;
		break;
	case CaretMovePageDown:
		if(fCancelingSelection && !fForward)
		{
			// Reverse Selection Cancel => drop one below sel start line
			lcp.row = m_selInfo.lineBegin;
		}

		lcp.row += (m_szPage.cy / m_szChar.cy);
		break;
	case CaretMovePageUp:
		if(fCancelingSelection && fForward)
		{
			lcp.row = m_selInfo.lineBegin;
		}

		lcp.row -= (m_szPage.cy / m_szChar.cy);
		break;
	case CaretMoveDocEnd:
		lcp.row = m_cLines - 1;
		lcp.col = _tcslen(GetColumnText(lcp.row,lcp.hdrCol));
		break;
	case CaretMoveDocBegin:
		lcp.row = 0;
		lcp.col = 0;
		break;
	case CaretMoveLineEnd:
		if(m_ViewMode == viewModeCaret)
		{
			if(fCancelingSelection)
			{
				lcp.row =  (fForward) ? m_selInfo.lineEnd : m_selInfo.lineBegin;
			}
			lcp.col = _tcslen(GetColumnText(lcp.row,lcp.hdrCol));
		}
		else if(m_ViewMode == viewModeLine)			 
		{
			lcp.row = m_cLines - 1;
			lcp.col = _tcslen(GetColumnText(lcp.row,lcp.hdrCol));
		}
		break;
	case CaretMoveLineBegin:
		if(m_ViewMode == viewModeCaret)
		{
			if(fCancelingSelection)
			{
				lcp.row = (fForward) ? m_selInfo.lineBegin : m_selInfo.lineEnd;
			}
			lcp.col = 0;
		}
		else if(m_ViewMode == viewModeLine)			 
		{
			lcp.row = 0;
			lcp.col = 0;
		}
		break;
	case CaretMoveCharLeft:
		if(m_ViewMode == viewModeCaret)
		{
			if(fCancelingSelection)
			{
				lcp.row = (fForward) ? m_selInfo.lineBegin : m_selInfo.lineEnd;
				lcp.col = (fForward) ? m_selInfo.startChar : m_selInfo.endChar;
			}
			else
			{
				lcp.col--;
			}
		}
		break;
	case CaretMoveCharRight:
		if(m_ViewMode == viewModeCaret)
		{
			if(fCancelingSelection)
			{
				lcp.row = (fForward) ? m_selInfo.lineEnd : m_selInfo.lineBegin;
				lcp.col = (fForward) ? m_selInfo.endChar : m_selInfo.startChar;
			}
			else
			{
				lcp.col++;
			}
		}
		break;
	case CaretMoveTabRight:
		if(m_ViewMode == viewModeCaret)
		{
			lcp.hdrCol = (lcp.hdrCol >= (int)m_cColumns-1) ? 0 : lcp.hdrCol + 1;
			lcp.col = 0;
		}
		break;
	case CaretMoveTabLeft:
		if(m_ViewMode == viewModeCaret)
		{
			lcp.hdrCol = (lcp.hdrCol == 0) ? (m_cColumns-1) : lcp.hdrCol - 1;
			lcp.col = 0;
		}
		break;
	}

	// Recalc Caret Point Now

	MoveCaretToLogPos(&lcp,status,cmt);
}

///////////////////////////////////////////////////////////////////////////////
// Scrolling

void CTextView::GetScrollBarSizes(CSize& sizeSb)
{
	sizeSb.cx = GetSystemMetrics(SM_CXVSCROLL);
	sizeSb.cy = GetSystemMetrics(SM_CYHSCROLL);
}

///////////////////////////////////////////////////////////////////////////////

CPoint CTextView::GetScrollPosition()
{
	CPoint pt(0,0);
	SCROLLINFO si = {0};
	si.cbSize = sizeof(SCROLLINFO);

	GetScrollInfo(SB_VERT,&si,SIF_ALL);
	pt.y = si.nPos;

	si.cbSize = sizeof(SCROLLINFO);
	GetScrollInfo(SB_HORZ,&si,SIF_ALL);
	pt.x = si.nPos;

	return pt;
}

///////////////////////////////////////////////////////////////////////////////

void CTextView::ScrollToPosition(POINT pt)    // logical coordinates
{
	// Limit co-ords;

	int limX = GetScrollLimit(SB_HORZ);
	int limY = GetScrollLimit(SB_VERT);

	pt.x = (pt.x > limX) ? limX : pt.x;
	pt.y = (pt.y > limY) ? limY : pt.y;
	pt.x = (pt.x < 0) ? 0 : pt.x;
	pt.y = (pt.y < 0) ? 0 : pt.y;

	CPoint ptBefore = GetScrollPosition();

	
	SCROLLINFO si = {0};

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;

	si.nPos = pt.y;
	SetScrollInfo(SB_VERT,&si);

	si.nPos = pt.x;
	SetScrollInfo(SB_HORZ,&si);

	
	CPoint ptAfter = GetScrollPosition();

	
	// Adjust Header

	m_hdr.SetWindowPos(NULL,-ptAfter.x,0,0,0,SWP_NOSIZE | SWP_NOZORDER);

	// Real Caret Needs to be repositioned

	SetRealCaretPosFromLog();

	//// Only Invalidate where needed

	//CRect rcWnd;
	//GetClientRect(rcWnd);

	//// Vertical

	//int top = max(ptAfter.y,ptBefore.y);
	//int bottom = min(ptAfter.y,ptBefore.y);

	//InvalidateRect(CRect(ptAfter.x,top,ptAfter.x + rcWnd.Width(),bottom));

	//// Horizontal

	//int left = min(ptAfter.x,ptBefore.x);
	//int right = max(ptAfter.x,ptBefore.x);

	//CRect rc(ptAfter.x,ptAfter.y,ptAfter.x+rcWnd.Width(),ptAfter.y + rcWnd.Height());
	//rc.InflateRect(m_szChar.cx * 2,m_szChar.cy * 2,m_szChar.cx * 2,m_szChar.cy * 2);

	////InvalidateRect(rc);

	Invalidate();
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	OnScroll(nSBCode, nPos, pScrollBar, FALSE);
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	OnScroll(nSBCode, nPos, pScrollBar, TRUE);
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar, BOOL fHorz)
{
	SCROLLINFO si = {0};
	int x, y;

	GetScrollInfo(SB_HORZ,&si,SIF_ALL);
	x = si.nPos;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);
	y = si.nPos;

	if(fHorz)
	{
		switch(nSBCode)
		{
		case SB_TOP:
			x = 0;
			break;
		case SB_BOTTOM:
			x = INT_MAX;
			break;
		case SB_LINEUP:
			x -= m_szChar.cx;
			break;
		case SB_LINEDOWN:
			x += m_szChar.cx;
			break;
		case SB_PAGEUP:
			x -= m_szPage.cx;
			break;
		case SB_PAGEDOWN:
			x += m_szPage.cx;
			break;
		case SB_THUMBTRACK:
			GetScrollInfo(SB_HORZ,&si,SIF_ALL);
			x = si.nTrackPos;
			break;
		default:
			return;
			break;
		}
	}
	else
	{
		switch(nSBCode)
		{
		case SB_TOP:
			y = 0;
			break;
		case SB_BOTTOM:
			y = INT_MAX;
			break;
		case SB_LINEUP:
			y -= m_szChar.cy;
			break;
		case SB_LINEDOWN:
			y += m_szChar.cy;
			break;
		case SB_PAGEUP:
			y -= m_szPage.cy;
			break;
		case SB_PAGEDOWN:
			y += m_szPage.cy;
			break;
		case SB_THUMBTRACK:
			GetScrollInfo(SB_VERT,&si,SIF_ALL);
			y = si.nTrackPos;
			break;
		default:
			return;
			break;
		}
	}


	//if(!fHorz)
	//{
	//	TCHAR szOut[256];

	//	switch(nSBCode)
	//	{
	//	case SB_TOP:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_TOP        => y = [%0.4d]\n"), y);
	//		break;
	//	case SB_BOTTOM:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_BOTTOM     => y = [%0.4d]\n"), y);
	//		break;
	//	case SB_LINEUP:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_LINEUP     => y = [%0.4d]\n"), y);
	//		break;
	//	case SB_LINEDOWN:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_LINEDOWN   => y = [%0.4d]\n"), y);
	//		break;
	//	case SB_PAGEUP:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_PAGEUP     => y = [%0.4d]\n"), y);
	//		break;
	//	case SB_PAGEDOWN:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_PAGEDOWN   => y = [%0.4d]\n"), y);
	//		break;
	//	case SB_THUMBTRACK:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: SB_THUMBTRACK => y = [%0.4d]\n"), y, nPos);
	//		break;
	//	default:
	//		_sntprintf(szOut,255, _T("ScrollBar Vert Tracking: OTHER         => y = [%0.4d] code = %d\n"), y,nSBCode);
	//		break;
	//	}

	//	OutputDebugString(szOut);
	//}

	ScrollToPosition(CPoint(x,y));
}

/////////////////////////////////////////////////////////////////////

void CTextView::GetTrueClientRect(CRect& rcWnd)
{
	GetClientRect(rcWnd);
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);
	rcWnd.top += m_cyHdr;
}

/////////////////////////////////////////////////////////////////////

void CTextView::LayoutHeader()
{
	CRect rc;
	WINDOWPOS wpos;
	GetClientRect(&rc);
	rc.left = -GetScrollPosition().x;
	rc.right = max(m_szDoc.cx,rc.right);

	HDLAYOUT layout = {0};
	layout.prc = &rc;
	layout.pwpos = &wpos;
	m_hdr.Layout(&layout);

	m_hdr.SetWindowPos(
		CWnd::FromHandle(wpos.hwndInsertAfter),
		wpos.x,
		wpos.y,
		wpos.cx,
		wpos.cy,
		wpos.flags | SWP_SHOWWINDOW);
}

/////////////////////////////////////////////////////////////////////

BOOL CTextView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int nLines = abs(zDelta/WHEEL_DELTA) * m_uScrollWheelLines;

	SCROLLINFO si = {0};
	int x, y;

	GetScrollInfo(SB_HORZ,&si,SIF_ALL);
	x = si.nPos;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);
	y = si.nPos;

	int dist = m_szChar.cy * nLines;

	y += (zDelta > 0) ? -dist : dist;

	ScrollToPosition(CPoint(x,y));

	return CCtrlView::OnMouseWheel(nFlags, zDelta, pt);
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnEditCopy()
{
	if(!m_selInfo.fSelected)
	{
		return;
	}

	// We copy in text only format
	// crlf's for line breaks

	// Normalize selection

	int nStartLine = min(m_selInfo.lineBegin,m_selInfo.lineEnd);
	int nEndLine = max(m_selInfo.lineBegin,m_selInfo.lineEnd);

	BOOL fReverse = (m_selInfo.lineBegin > m_selInfo.lineEnd);

	int nStartChar = fReverse ? m_selInfo.endChar : m_selInfo.startChar;
	int nEndChar = fReverse ? m_selInfo.startChar : m_selInfo.endChar;

	TCHAR szDbg[256];
	_sntprintf(szDbg,255,_T("OnEditCopy() => %d -> %d  ==  %d -> %d\n"),nStartLine,nEndLine,nStartChar,nEndChar);
	OutputDebugString(szDbg);

	// How much mem do we need to alloc for this selection?

	int cch = 0;
	for(int i=nStartLine;i<nEndLine+1;i++)
	{
		cch += _tcslen(GetColumnText(i,m_selInfo.hdrCol));
		cch += 2; // for CRLF's
	}
	
	int cb = (cch + 1) * sizeof(TCHAR);
	HGLOBAL hBuffer = GlobalAlloc(GMEM_DDESHARE,cb);

	ASSERT(hBuffer);
	if(hBuffer == NULL)
	{
		return;
	}

	LPTSTR lpszCB = (LPTSTR)GlobalLock(hBuffer);

	ASSERT(lpszCB);
	if(NULL == lpszCB)
	{
		return;
	}

	
	// Copy
	
	lpszCB[0] = 0;
	int cbMax = cb - sizeof(TCHAR);

	for(int i=nStartLine;i<nEndLine+1;i++)
	{
		LPCTSTR lpszCur = GetColumnText(i,m_selInfo.hdrCol);

		if(i == nStartLine)
		{
			_tcsncat(lpszCB,&lpszCur[nStartChar],cbMax);
		}
		else if(i == nEndLine)
		{
			_tcsncat(lpszCB,lpszCur,nEndChar);
		}
		else
		{
			_tcsncat(lpszCB,lpszCur,cbMax);
		}

		// Add a CRLF?

		if(i != nEndLine)
		{
			_tcsncat(lpszCB,_T("\r\n"),cbMax);
		}
	}

	if(!GlobalUnlock(hBuffer))
	{
		ASSERT(FALSE);
		return;
	}


	// Store in Clipboard...
	
	if(!OpenClipboard() || !EmptyClipboard())
	{
		Assert(FALSE);
		AfxMessageBox(_T("Error opening or emptying clipboard."));
		return;
	}

	HANDLE hCB = NULL;

#ifdef _UNICODE
	hCB = SetClipboardData(CF_UNICODETEXT,hBuffer);
#else
	hCB = SetClipboardData(CF_TEXT,hBuffer);
#endif

	ASSERT(hCB);
	if(NULL == hCB)
	{
		return;
	}

	CloseClipboard();
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	// Can only copy if there is a selection

	pCmdUI->Enable(m_selInfo.fSelected);
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_cLines);
}

/////////////////////////////////////////////////////////////////////

void CTextView::OnFileSaveAs()
{
	CFileDialog dlg(FALSE);
	if(IDOK != dlg.DoModal())
	{
		return;
	}
       
	CString str = dlg.GetPathName();
    	
	HANDLE hFile = CreateFile((LPCTSTR)str,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);

	if(INVALID_HANDLE_VALUE == hFile)
	{
		ASSERT(FALSE);
		AfxMessageBox(_T("Error Saving File: CreateFile()"));
		return;
	}

    
	// Text Only Format / All Columns (Tab Separated)

	int cColumns = m_cColumns;

	LPCTSTR kszCRLF = _T("\r\n");

	// For each column find max width

	vector<int> vecMaxCol;

	for(int j=0;j<cColumns;j++)
	{
		int cchMax = 0;
		for(int i=0;i<m_cLines;i++)
		{
			LPCTSTR lpszCur = GetColumnText(i,j);	
			int len = _tcslen(lpszCur);

			if(len > cchMax)
			{
				cchMax = len;
				if(j > 0)
				{
					vecMaxCol.push_back(len + vecMaxCol[j-1]);
				}
				else
				{
					vecMaxCol.push_back(len);
				}
			}
		}
	}
	

	// Format Line

	const int max_out_len = 253; // + 2 for crlf + 1 for null term

	TCHAR szOutLine[max_out_len + 2 + 1];

	for(int i=0;i<m_cLines;i++)
	{
		szOutLine[0]=0;
		int curlen = 0;

		for(int j=0;j<cColumns;j++)
		{
			LPCTSTR lpszText =GetColumnText(i,j);	
			int textlen = _tcslen(lpszText);

			int cchMax = vecMaxCol[j];
			int nSpaces = cchMax - textlen + 3; // 3 space gap min 

			_tcsncat(szOutLine,lpszText,max_out_len - textlen);
			curlen = _tcslen(szOutLine);

			for(int k=0;k<nSpaces && curlen < max_out_len;k++)
			{
				szOutLine[curlen++] = _T(' ');
			}
			szOutLine[curlen++] = 0;
		}
		_tcscat(szOutLine,kszCRLF);


		// Write to file...

		DWORD cbWrite = _tcslen(szOutLine) * sizeof(TCHAR);
		DWORD cbWritten = 0;

		if(!WriteFile(hFile,szOutLine,cbWrite,&cbWritten,NULL) ||
			cbWritten < cbWrite)
		{
			ASSERT(FALSE);
			AfxMessageBox(_T("Error Saving File: CreateFile()"));
			CloseHandle(hFile);
			return;
		}
	}

	CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////