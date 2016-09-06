///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

typedef struct _SELECTION_INFO
{
	int lineBegin;
	int lineEnd;
	int hdrCol;
	int startChar;
	int endChar;
	BOOL fSelected;
}SELECTION_INFO, *LPSELECTIONINFO;

///////////////////////////////////////////////////////////////////////////////

typedef struct _ORIG_CARET_COLUMN_INFO 
{
	int header;
	int col;
}ORIG_CARET_COLUMN_INFO, *LPORIG_CARET_COLUMN_INFO;

///////////////////////////////////////////////////////////////////////////////

typedef struct tagLOG_CARET_POS
{
	LONG hdrCol;
	LONG col;
	LONG row;
}LOG_CARET_POS, *PLOG_CARET_POS;

///////////////////////////////////////////////////////////////////////////////

class CTextViewHeaderCtrl : public CHeaderCtrl
{
	DECLARE_DYNAMIC(CTextViewHeaderCtrl)

public:
	CTextViewHeaderCtrl(){}
	virtual ~CTextViewHeaderCtrl(){}

	int GetItemMaxWidth(int col)
	{
		HDITEM hdi;
		hdi.mask = HDI_LPARAM;

		if(!GetItem(col,&hdi))
		{
			ASSERT(FALSE);
			return 0;
		}

		return hdi.lParam;
	}

	int GetItemLeft(int col)
	{
		CRect rc;
		if(!GetItemRect(col,&rc))
		{
			ASSERT(FALSE);
			return 0;
		}

		// Offset this rect if this header ctrl is
		// negatively placed back in the parent window
		// first column should always have left of zero!

		CWnd *pParent = GetParent();
		ASSERT(pParent);
		if(NULL == pParent)
		{
			return 0;
		}

		CRect rcWnd;
		GetWindowRect(rcWnd);
		CRect rcParent;
		pParent->GetWindowRect(rcParent);
		static int cxEdge = GetSystemMetrics(SM_CXEDGE);
		int offset = rcParent.left - rcWnd.left + cxEdge;

		rc.left += offset;
		rc.right+= offset;

		// Return in terms of parent

		ClientToScreen(rc);
		pParent->ScreenToClient(rc);
		return rc.left;
	}

	int GetItemWidth(int col)
	{
		HDITEM hdi;
		hdi.mask = HDI_WIDTH;

		if(!GetItem(col,&hdi))
		{
			ASSERT(FALSE);
			return 0;
		}

		return hdi.cxy;
	}

protected:
	DECLARE_MESSAGE_MAP()
public:
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CTextViewLine 
{
public:
	CTextViewLine()
	{
		int m_cColumns = 0;
		m_fInit = FALSE;
	}

	~CTextViewLine()
	{
		Cleanup();
	}

protected:
	void Cleanup()
	{
		for(vector<LPCTSTR>::iterator theText = m_vecColumnText.begin();
			theText!= m_vecColumnText.end();theText++)
		{
			if((*theText))
			{
				delete(*theText);
			}
		}

		m_vecColumnText.clear();
	}


public:
	BOOL Init(int cColumns)
	{
		m_cColumns = cColumns;
		for(int i=0;i<m_cColumns;i++)
		{
			// Set Empty
			m_vecColumnText.push_back(NULL);
			m_vecColumnColor.push_back(RGB(0,0,0));
		}

		m_fInit = TRUE;
		return TRUE;
	}

	BOOL SetColumnText(int nColumn, COLORREF color,LPCTSTR lpszText, int cch)
	{
		ASSERT(m_fInit);
		ASSERT(nColumn < m_cColumns && nColumn >= 0);
		if(!(nColumn < m_cColumns && nColumn >= 0))
		{
			return FALSE;
		}

		LPTSTR lpszTmp = new TCHAR[cch + 1];
		ASSERT(lpszTmp);

		if(NULL == lpszTmp)
		{
			return FALSE;
		}

		_tcscpy(lpszTmp,lpszText);
		m_vecColumnText[nColumn] = lpszTmp;
		m_vecColumnColor[nColumn] = color;
		return TRUE;
	}

	COLORREF GetColumnColor(int nColumn)
	{
		ASSERT(nColumn < m_cColumns && nColumn >= 0);
		if(!(nColumn < m_cColumns && nColumn >= 0))
		{
			return NULL;
		}

		return m_vecColumnColor[nColumn];
	}

	LPCTSTR GetColumnText(int nColumn)
	{
		ASSERT(nColumn < m_cColumns && nColumn >= 0);
		if(!(nColumn < m_cColumns && nColumn >= 0))
		{
			return NULL;
		}

		return m_vecColumnText[nColumn];
	}

	int GetColumnTextLength(int nColumn)
	{
		ASSERT(nColumn < m_cColumns && nColumn >= 0);
		if(!(nColumn < m_cColumns && nColumn >= 0))
		{
			return NULL;
		}

		return _tcslen(m_vecColumnText[nColumn]);
	}

	int GetColumnCount() {return m_cColumns;}

protected:
	int m_cColumns;
	BOOL m_fInit;
	vector<COLORREF> m_vecColumnColor;
	vector<LPCTSTR> m_vecColumnText;
};

///////////////////////////////////////////////////////////////////////////////

class CTextView : public CCtrlView
{
	DECLARE_DYNCREATE(CTextView)

public:
	typedef enum _ViewMode
	{
		viewModeViewOnly,
		viewModeCaret,
		viewModeLine,
	}ViewMode;

private:
	typedef enum _SelectionStatus
	{
		SelectionNone,
		SelectionStart,
		SelectionInProgress,
	}SelectionStatus;

	typedef enum _CaretMoveType
	{
		CaretMoveNone,
		CaretMoveTabLeft,
		CaretMoveTabRight,
		CaretMoveCharLeft,
		CaretMoveCharRight,
		CaretMoveLineDown,
		CaretMoveLineUp,
		CaretMoveLineBegin,
		CaretMoveLineEnd,
		CaretMovePageDown,
		CaretMovePageUp,
		CaretMoveDocEnd,
		CaretMoveDocBegin,
	}CaretMoveType;

protected:
	CTextView();           // protected constructor used by dynamic creation
	virtual ~CTextView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	
	BOOL		m_fInit; // Are we initialized?
	CFont*		m_pFont; // The Font
	CBrush*		m_pBrshHilightLine;
	CSize		m_szChar;// Size of a single character
	CSize		m_szDoc; // Total Size of this document
	CSize		m_szPage;
	BOOL		m_fShowHeader;
	int			m_cyHdr;
	CSize		*m_pszClip;

	UINT		m_uScrollWheelLines;

	// behaviour

	ViewMode	m_ViewMode;
	BOOL		m_fSelectionPossible;
	
	// Line Addition

	BOOL		m_fDeferAddLine;

	// Keeps Track of keyboard selection

	BOOL		m_fKeyBdSelectionHasStarted;

	// Used to avoid this maximize bug:
	// INFO: Needed as when this window is maximized with a selection
	// it recieves wm_mousemove with flag of lbuttondown, causing a phantom 
	// capture, which removes users current selection

	BOOL		m_fAllowMouseSelectionStart;	
	BOOL		m_fInCapture; // We're capturing mouse input

	// Save Caret Postion on lose focus & restore on gain focus

	//CPoint			m_ptCaret;
	LOG_CARET_POS	m_posLogCaret; // Logical (row, col) based caret pos

	// Carries all selection info for drawing or continuation
	
	int	m_nSelectionCaretColumn; // used to limit selection to a column
	SELECTION_INFO m_selInfo;
	SELECTION_INFO m_prevSI;

	// Columns & Lines

	//vector<CTextViewLine*> m_vecLines;

	CTextViewHeaderCtrl m_hdr;
	LPBYTE m_lpbData;
	int m_cbData;
	int m_cbDataUsed;
	int m_cchMaxLine;
	int m_cLines;
	int m_cColumns;

	LPBYTE* m_lpbLineMap;
	int m_cbLineData;

	//std::map<int,LPBYTE> m_mapLine2Mem;

protected:
	LPVOID		GetLineParam(int line) const;
	COLORREF	GetColumnColour(int line,int col) const;
	LPTSTR		GetColumnText(int line,int col) const;

	void RecalcScrolls(BOOL fRecalcDocSize, BOOL fSetColumnsSizeToFitText);
	void LayoutHeader();
	void Cleanup();
	
	virtual void DrawSelection(CDC* pDC);
	virtual void DrawColumnLine(CDC* pDC, CRect& rcClip, int col, int line);
	virtual void HilightLine(CDC* pDC, CRect& rcClip, int line, LPCOLORREF pColor = NULL);

	int	 GetHeaderColumn(int x, LONG *pColumn  = NULL );
	void MoveCaretToLogPos(PLOG_CARET_POS plcp,SelectionStatus status,
						CaretMoveType cmt = CaretMoveNone);

	void InvalidateCurrentLine();
	void InvalidateLine(int line, int col = -1);
    void InvalidateSelectionInProgress();
	void ScrollCaretIntoView(CaretMoveType cmt);
	void PerformKeyboardCaretMovement(SelectionStatus status,
										CaretMoveType  cmt );


	void ClientToDocument(CPoint& pt);
	void SnapToCaretGrid(CPoint& pt, PLOG_CARET_POS plcp);

	void GetTrueClientRect(CRect& rcWnd);
	// Scrolling

	CPoint GetScrollPosition();// const;
	void GetScrollBarSizes(CSize& sizeSb);
	void ScrollToPosition(POINT pt);    // set upper left position
	int GetHeaderHeight();

	CPoint GetCaretPixelPos();
	void SetRealCaretPosFromLog();

public:
	BOOL ClearLines();

    BOOL Init(	CTextView::ViewMode viewMode= CTextView::viewModeCaret,
				BOOL fSelectionPossible = FALSE,
				LPCTSTR lpszFontFace =NULL, 
				int nFontSize=0, 
				int cColumns = 1,
				LPCTSTR lpszColumns[] = NULL, 
				BOOL fAutoCalcColumnSize =TRUE);

	BOOL BeginDeferAddLine();
	BOOL AddLine(LPCTSTR pText[], COLORREF colors[], int cColumns, 
											LPVOID lpvParam = NULL);
	BOOL EndDeferAddLine();

	int GetCurrentLine() {return m_posLogCaret.row;}
	BOOL SetCurrentLine(int line);

protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEndTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnHdrChangeRecalcScrolls(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	void OnScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar, BOOL fHorz);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnFileSaveAs();
};

///////////////////////////////////////////////////////////////////////////////