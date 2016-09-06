/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////

#include <afxcview.h>
#include "TextView.h"
#include "ResourcesDoc.h"
#include "DlgPropDlg.h"

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVersionView view

class CVersionView : public CListView
{
protected:
	CVersionView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CVersionView)

// Attributes
public:

// Operations
public:
	CVersionDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVersionView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CVersionView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CVersionView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStringTableView view

class CStringTableView : public CListView
{
protected:
	CStringTableView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CStringTableView)

// Attributes
public:

// Operations
public:
	CStringTableDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringTableView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CStringTableView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CStringTableView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMenuView view

typedef struct _TOPLEVEL_MENU_ITEM{
	CRect rct;
	CString strText;
	MENUITEMTEMPLATE* pMenuChild;
	_TOPLEVEL_MENU_ITEM* pNext;
}TOPLEVEL_MENU_ITEM,*LPTOPLEVEL_MENU_ITEM;

///////////////////////////////////////////////////////////////////////////////

class CMenuView : public CScrollView
{
protected:
	void StdLoadTopMenuItems();
	void ExLoadTopMenuItems();
	HMENU StdMyCreatePopupMenu(MENUITEMTEMPLATE* pMit,MENUITEMTEMPLATE** ppNextMenuItem);
	CMenuView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMenuView)

// Attributes
public:
// Operations
public:
	CMenuDoc* GetDocument();
	BOOL m_bEx;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMenuView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DeleteAllMenuItems(TOPLEVEL_MENU_ITEM* pItem);
	TOPLEVEL_MENU_ITEM* m_pMenuFirst;
	int m_cxMenu;
	virtual ~CMenuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMenuView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CIconView view

class CIconView : public CScrollView
{
protected:
	CIconView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CIconView)

// Attributes
public:

// Operations
public:
	CIconDoc* GetDocument();
	void SetIconIndex(int nIcon);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CIconView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CIconView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CHexView view

class CHexView : public CTextView
{
	enum HEXVIEWMODE{VIEW_BYTE,VIEW_WORD,VIEW_DWORD};
protected:
	CHexView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHexView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHexView)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	CHexDoc* GetDocument();
//	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );
	virtual ~CHexView();
	HEXVIEWMODE m_hvmViewMode;
	int m_nBytesPerLine;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CHexView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	afx_msg void OnContextMenu( CWnd* pWnd, CPoint pos );
	afx_msg void OnViewByte();
	afx_msg void OnViewWord();
	afx_msg void OnViewDword();
	DECLARE_MESSAGE_MAP()
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgView view
class CDlgView : public CScrollView
{
protected:
	CDlgView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDlgView)

// Attributes
public:

// Operations
public:
	CDlgDoc* GetDocument();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void BuildPropertyString(CString& strProp,DWORD *pdwStyle, DWORD *pdwExStyle, WINDOW_TYPE* ptype);
	int m_nCurControl;
	BOOL GetDlgItemFromPoint(CPoint &pt, int* pnItem);
	virtual ~CDlgView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CRectTracker m_trck;
	CDialog m_dlg;

	// Generated message map functions
	//{{AFX_MSG(CDlgView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnControlProperties();
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBmpView view

class CBmpView : public CScrollView
{
protected:
	CBmpView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBmpView)

// Attributes
public:

// Operations
public:
	CBmpDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBmpView)
	public:
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void RecalcScrolls();
	virtual ~CBmpView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CBmpView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAccelView view
 
class CAccelView : public CListView
{
protected:
	CAccelView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAccelView)

// Attributes
public:

// Operations
public:
	CAccelDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccelView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAccelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CAccelView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////