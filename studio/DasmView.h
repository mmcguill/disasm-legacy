/////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////

//typedef struct _CODE_LOC
//{
//	DWORD dwStartDis;
//	DWORD dwLine;
//}CODE_LOC,*LPCODE_LOC;

/////////////////////////////////////////////////////////////////////

#include "DasmDoc.h"
#include "TextView.h"

/////////////////////////////////////////////////////////////////////

class CGlobalViewBar : public CDialogBar
{
	DECLARE_DYNAMIC(CGlobalViewBar)

public:
	CGlobalViewBar() {m_pDoc = NULL;m_dwAddress=0;}

	void SetDocument(CDasmBaseDoc* pDasmDoc);
	void SetCurrentAddress(DWORD dwAddress);
protected:
	DECLARE_MESSAGE_MAP()

	CDasmBaseDoc*	m_pDoc;
	DWORD		m_dwAddress;
public:
	afx_msg void OnPaint();
};

/////////////////////////////////////////////////////////////////////
// CDasmView view

class CDasmView : public CTextView
{
	DECLARE_DYNCREATE(CDasmView)

protected:
	CDasmView();           // protected constructor used by dynamic creation
	virtual ~CDasmView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
public:
	CDasmBaseDoc* GetDocument() const;

	void SetGlobalViewBarWnd(CGlobalViewBar* pGVBar)
	{
		Assert(pGVBar);
		m_pwndGlobView = pGVBar;
	}

	DECLARE_MESSAGE_MAP()



protected:
	BOOL FillFromDasmList(PLISTINGS_MANAGER pLstasm);
	void CheckCurrentLineJump();
	
	BOOL MoveToAddress(DWORD dwAddress);


	CBitmap		*m_pBmpUp,*m_pBmpDown;
	BOOL m_fDrawJump;
	int m_nDestLine;
	BOOL m_fDestSouth;
	std::map<DWORD, DWORD> *m_pmapVA2LineNo;
	CGlobalViewBar* m_pwndGlobView;

protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg LRESULT OnEngineUpdate(WPARAM,LPARAM);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	
	stack<DWORD> m_stkCall;
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDasmGoto();
};

/////////////////////////////////////////////////////////////////////

#define DASM_GOTO_CCHMAX_ADDRESS 256

class CDasmGotoDialog : public CDialog
{
	DECLARE_DYNAMIC(CDasmGotoDialog)

public:
	CDasmGotoDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDasmGotoDialog();

// Dialog Data
	enum { IDD = IDD_DASMGOTODIALOG };

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	void GetAddress(LPTSTR lpszAddress, int nMaxCount);

protected:
	TCHAR m_szAddress[DASM_GOTO_CCHMAX_ADDRESS];
};

/////////////////////////////////////////////////////////////////////