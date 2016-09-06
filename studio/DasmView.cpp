///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "DasmView.h"
#include "DasmFrame.h"
#include ".\dasmview.h"

///////////////////////////////////////////////////////////////////////////////

#define COL_DATA	  RGB(50,50,50)
#define COL_SYMBOL	  RGB(0,0,128)
#define COL_ADDRESS   RGB(10,120,10)
#define COL_INSBYTES  RGB(178,178,0)
#define COL_ASM       RGB(0,0,255)
#define COL_JUMP      RGB(255,0,255)
#define COL_APICALL   RGB(255,0,0)
#define COL_LINE      RGB(200,255,200)
#define COL_JUMPLINE  RGB(200,200,255)
#define COL_DESTLINE  RGB(200,200,200)

///////////////////////////////////////////////////////////////////////////////

const int MAX_REPEAT_BYTE_COUNT = 3;	// More Than 3 and we use short hand
										// DUP() Notation for data bytes

///////////////////////////////////////////////////////////////////////////////

static LPCTSTR rgColumns[] = 
{
	_T("Address"),
	_T("Instruction Bytes"),
	_T("Level 1 Asm"),
};

#define MAX_COLS (sizeof(rgColumns) / sizeof(LPCTSTR))

///////////////////////////////////////////////////////////////////////////////
// CDasmView

IMPLEMENT_DYNCREATE(CDasmView, CTextView)

///////////////////////////////////////////////////////////////////////////////

CDasmView::CDasmView()
{
	m_pBmpUp = NULL;
	m_pBmpDown = NULL;

	m_fDrawJump = FALSE;
	m_nDestLine = 0;
	m_fDestSouth = FALSE;
	m_pmapVA2LineNo = NULL;
	m_pwndGlobView = NULL;
}

///////////////////////////////////////////////////////////////////////////////

CDasmView::~CDasmView()
{
	if(m_pBmpDown)
	{
		delete m_pBmpDown;
	}

	if(m_pBmpUp)
	{
		delete m_pBmpUp;
	}

	SAFE_DELETE(m_pmapVA2LineNo);
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDasmView, CTextView)
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_DASM_GOTO, OnDasmGoto)
	ON_MESSAGE(WM_DASM_ENGINE_UPDATE,OnEngineUpdate)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

int CDasmView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTextView::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	
	// Initialize This View

	if(!Init(CTextView::viewModeLine,FALSE,NULL,0,MAX_COLS,rgColumns,TRUE))
	//if(!Init(CTextView::viewModeCaret,TRUE,NULL,0,MAX_COLS,rgColumns,TRUE))
	{
		return -1;
	}


	// Setup Directional Bitmaps

	m_pBmpUp = new CBitmap();
	m_pBmpDown = new CBitmap();
	m_pBmpUp->LoadBitmap(IDB_UP);
	m_pBmpDown->LoadBitmap(IDB_DOWN);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CDasmView::OnEngineUpdate(WPARAM, LPARAM)
{
	DASM_VIEW_UPDATE_PARAM dvp;
	dvp.dwAddress = 0;
	dvp.updType = dvutFullNoMoveAddr;

	OnUpdate(NULL,(LPARAM)&dvp,NULL);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CDasmView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	PDASM_VIEW_UPDATE_PARAM  pdu = (PDASM_VIEW_UPDATE_PARAM)lHint;
	if(NULL == pdu)
	{
		return;
	}

	if(dvutFull == pdu->updType)
	{
		CDasmBaseDoc* pDasmDoc = GetDocument();
		ASSERT(pDasmDoc);
		if(NULL == pDasmDoc)
		{
			return;
		}

		PLISTINGS_MANAGER pLstasm = pDasmDoc->GetListingsManager();

		if(NULL == pLstasm)
		{
			return;
		}
		
		
		ClearLines();

		FillFromDasmList(pLstasm);
		MoveToAddress(pdu->dwAddress);

		// Global View Toolbar

		//Assert(m_pwndGlobView);
		//m_pwndGlobView->SetDocument(pDasmDoc);
	}
	else if(dvutFullNoMoveAddr == pdu->updType)
	{
		CDasmBaseDoc* pDasmDoc = GetDocument();
		ASSERT(pDasmDoc);
		if(NULL == pDasmDoc)
		{
			return;
		}

		PLISTINGS_MANAGER pLstasm = pDasmDoc->GetListingsManager();

		if(NULL == pLstasm)
		{
			return;
		}
		
		
		ClearLines();

		FillFromDasmList(pLstasm);
	}
	else if(dvutCurAddressOnly == pdu->updType)
	{
		MoveToAddress(pdu->dwAddress);

		
		// Global View Toolbar

		//Assert(m_pwndGlobView);
		//m_pwndGlobView->SetCurrentAddress(pdu->dwAddress);
	}
	else
	{
		ASSERT(FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// CDasmView diagnostics

#ifdef _DEBUG
void CDasmView::AssertValid() const
{
	CTextView::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CDasmView::Dump(CDumpContext& dc) const
{
	CTextView::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

CDasmBaseDoc* CDasmView::GetDocument() const
{
	CDocument* pDoc = CTextView::GetDocument();
	ASSERT(pDoc);
	return (CDasmBaseDoc*)pDoc;
}

///////////////////////////////////////////////////////////////////////////////

// TODO: This is a copy of CIA32...::IsJumpInstruction()
// use this function instead. Basically need to shift this strucure to assembly
// conversion to dasmeng.dll

BOOL IsJumpInstruction(MnemonicEnum mne)
{
	if (MneJo == mne || MneJno == mne || MneJb == mne || 
		MneJnb == mne || MneJz == mne || MneJnz == mne || 
		MneJbe == mne || MneJa == mne || MneJs == mne || 
		MneJns == mne || MneJp == mne || MneJnp == mne || 
		MneJl == mne || MneJge == mne || MneJle == mne || 
		MneJg == mne || MneCall == mne || MneCallf == mne || 
		MneJmpf == mne || MneJmp == mne)
	{
		return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CDasmView::FillFromDasmList(PLISTINGS_MANAGER pLstasm)
{
	PLISTINGS pll = pLstasm->AcquireListings();
	if(NULL == pll)
	{
		pLstasm->ReleaseListings();
		return TRUE;
	}

	CStudioDoc* pStudioDoc = GetDocument()->
			GetWorkspaceItemInfo()->GetWorkspaceDoc();

	CPEParser *pPEParser = pStudioDoc->GetExecutableParser();
    
	PSYMBOLS_MAP pSymMap = pStudioDoc->GetSymbolTable()->GetSymbolsMap();

	Assert(pSymMap);

	PBYTE pBuf = pPEParser->GetImage();
	size_t len = pPEParser->GetImageSize();
	ULONG imageBase = pPEParser->GetImageBase();
	ULONG start = pPEParser->GetEntryPoint();

	// Create our Virtual Address to Line Number Map

	SAFE_DELETE(m_pmapVA2LineNo);
	
	m_pmapVA2LineNo = new std::map<DWORD, DWORD>;
	ASSERT(m_pmapVA2LineNo);

	int nLine = 0;

	// Add The Lines

	BeginDeferAddLine();

	for(LISTINGS::iterator lst = pll->begin();
		lst != pll->end();lst++)
	{
		PLISTING pl = (*lst);

		if(ListingData == pl->GetListingType())
		{
			TCHAR szAddress[256] = _T("\0");
			TCHAR szData[32] = _T("\0");
			TCHAR szOut[256] = _T("\0");

			ULONG end = pl->GetEndAddress();
			ULONG base = pPEParser->GetImageBase();
			PBYTE image = pPEParser->GetImage();

			for(ULONG ulAddr = pl->GetStartAddress();ulAddr < end + 1;ulAddr++)
			{
				ULONG index = ulAddr - base;
				BYTE b = image[index];

				_sntprintf(szAddress,256,_T("%0.8X"), ulAddr);

				// Do a quick scan ahead and see if this byte is repeated
				// more than x times, if so do short hand rather than line
				// by line 'DB x DUP (Byte)'

				BOOL fDiff = FALSE;
				ULONG t1 = ulAddr + 1;
				while(!fDiff && t1 < (end + 1))
				{
					BYTE b2 = image[t1 - base];	

					fDiff = (b != b2);
					t1++;
				}

				int len = (t1 - ulAddr);

				
				// because we've overrun one

				if(t1 != (end + 1))
				{
					len--;
				}

				
				if(len > MAX_REPEAT_BYTE_COUNT)
				{
					_sntprintf(szData,32,_T("DB %d DUP (%0.2Xh)"),len,b);
					ulAddr += len - 1;
				}
				else
				{
					if(isprint(b))
					{
						_sntprintf(szData,32,_T("DB %0.2X ; '%c'"),b,b);
					}
					else
					{
						_sntprintf(szData,32,_T("DB %0.2X"),b);
					}
				}

				LPCTSTR rgszText[MAX_COLS] = {szAddress,_T(""),szData};
				COLORREF rgColors[MAX_COLS] = {COL_ADDRESS,COL_INSBYTES,COL_DATA};

				if(!AddLine(rgszText,rgColors,MAX_COLS))
				{
					ASSERT(FALSE);
					EndDeferAddLine();
					pLstasm->ReleaseListings();
					return FALSE;
				}
				nLine++;
			}
		}
		else if(ListingCode == pl->GetListingType())
		{
			PCODE_LISTING pcl = static_cast<PCODE_LISTING>(pl);
			PINSTRUCTION_MAP pim = pcl->GetInstructionMap();

			if( NULL == pim || 
				0 == pim->size())
			{
				continue;
			}
			
			for(std::map<ULONG, PINSTRUCTION>::iterator theIns = pim->begin();
				theIns != pim->end();theIns++)
			{
				std::pair<ULONG, PINSTRUCTION> ins = *theIns;
				SYMBOLS_MAP::iterator symbol;
				BOOL fSymbolFound = FALSE;
				PINSTRUCTION pInst = ins.second;

				if(NULL != pSymMap)
				{
					// Convert Addresses to Symbols where possible
					
					if (IsJumpInstruction(pInst->m_Mnemonic))
					{
						ULONG ulJumpEA = pInst->m_ulDirectJumpLoc;

						// Check for indirect jump...

						if(0 == ulJumpEA)
						{
							// This is probably an indirect jump
							// or it could actually be a jump to Zero!
							// so check for indirect jump indicated in
							// this case simply by checking for '[' char. 
							// TODO: Find a better way!
							
							Assert(pInst->m_rgszOperand[0]);

							LPSTR lpszIndirect = NULL;
							if (NULL != 
								(lpszIndirect = strchr(pInst->m_rgszOperand[0],'[')))
							{
								// See if we can find a simple address
								// nothing complicated at this stage or 
								// on this parse

								ULONG ulPtr = strtoul(lpszIndirect+1,NULL,16);
								
								
								// Check For Indirect Symbol here

								symbol = pSymMap->find(ulPtr);

								if (symbol != pSymMap->end()/* &&
									(symbol_indirect == symbol->second->GetType())*/)
								{
									fSymbolFound = TRUE;
								}
								else if(	
									ulPtr >= imageBase &&
									ulPtr <= ((	imageBase + len)-4))
								{
									ulJumpEA = 
										*(ULONG*)&(pBuf[ulPtr - imageBase]);
								}
							}
						}

						
						// Look for direct symbol if not found an indirect one

						if(!fSymbolFound)
						{
							symbol = pSymMap->find(ulJumpEA);
							if	(symbol != pSymMap->end()/* &&
								(symbol_indirect != symbol->second->GetType())*/)
							{
								fSymbolFound = TRUE;
							}
						}
					}
				}
			
				/*  address  */
				
				TCHAR szAddress[256] = _T("\0");
				TCHAR szIns[256] = _T("\0");
				TCHAR szAsm[1024] = _T("\0");
				TCHAR szOut[1024] = _T("\0");

				_sntprintf(szAddress,255,_T("%0.8X"),ins.first);	

				/*  instruction bytes  */

				PINSTRUCTION pIns = ins.second;

				INT cbInsBytes = pIns->m_cbInstruction;
				const PBYTE pInsb = ins.second->m_ucInstruction;
				
				TCHAR szTmp[256];

				for(INT i=0;i<cbInsBytes;i++)
				{
					_sntprintf(szTmp,255,_T("%0.2X"),pInsb[i]);
					_tcsncat(szIns,szTmp,255);
				}

				/* assembly */

#ifdef _UNICODE
				WCHAR szMne[MAX_SZ_MNEMONIC];
				MultiByteToWideChar(CP_ACP,0,pIns->m_szMnemonic,-1,szMne,MAX_SZ_MNEMONIC-1);
				szMne[MAX_SZ_MNEMONIC - 1] = 0;
				wcscpy(szAsm,szMne);
#else
				strncpy(szAsm,pIns->m_szMnemonic,MAX_SZ_MNEMONIC - 1);
				szAsm[MAX_SZ_MNEMONIC - 1] = 0;
#endif
				
					
				if(0 != pIns->m_cOperands)
				{
					_tcsncat(szAsm,_T(" "),255);

					BOOL fFirst = TRUE;
					
					for(ULONG i= 0;i<pIns->m_cOperands;i++)
					{
						if(!fFirst)
						{
							_tcsncat(szAsm,_T(", "),255);
						}
						else
						{
							fFirst = FALSE;
						}

						if(fSymbolFound)
						{
#ifdef _UNICODE
							WCHAR szSymName[MAX_CCH_SYMBOL_NAME];
							MultiByteToWideChar(CP_ACP,0,symbol->second->GetName(),-1,szSymName,MAX_CCH_SYMBOL_NAME-1);
							szSymName[MAX_CCH_SYMBOL_NAME - 1] = 0;
							wcscat(szAsm,szSymName);
#else
							strncat(szAsm, symbol->second->GetName(),MAX_CCH_SYMBOL_NAME-1);
#endif
						}
						else
						{
#ifdef _UNICODE
							WCHAR szOperand[MAX_SZ_OPERAND];
							MultiByteToWideChar(CP_ACP,0,pIns->m_rgszOperand[i],-1,szOperand,MAX_SZ_OPERAND-1);
							szOperand[MAX_SZ_OPERAND - 1] = 0;
							wcscat(szAsm,szOperand);
#else
							strncat(szAsm, pIns->m_rgszOperand[i],MAX_SZ_OPERAND-1);
#endif
						}
					}
				}
					
				// Color and Add

				COLORREF colorL1 = COL_ASM;
				BOOL bJump = IsJumpInstruction(pIns->m_Mnemonic);
				
				if(bJump)
				{
					colorL1 = COL_JUMP;
				}

				LPCTSTR rgszText[MAX_COLS] = {szAddress,szIns,szAsm};
				COLORREF rgColors[MAX_COLS] = {COL_ADDRESS,COL_INSBYTES,colorL1};

				SYMBOLS_MAP::iterator symCurrent = pSymMap->find(ins.first);

				if(symCurrent != pSymMap->end())
				{
					TCHAR szSym[256];

#ifdef _UNICODE
					WCHAR szSymName[MAX_CCH_SYMBOL_NAME];
					MultiByteToWideChar(CP_ACP,0,symCurrent->second->GetName(),-1,szSymName,MAX_CCH_SYMBOL_NAME-1);
					szSymName[MAX_CCH_SYMBOL_NAME - 1] = 0;

					_sntprintf(szSym,255,_T("%s:"),szSymName);
#else
					_sntprintf(szSym,255,_T("%s:"),symCurrent->second->GetName());
#endif

					LPCTSTR rgszSym[MAX_COLS] = {szAddress,szSym,_T("")};

					COLORREF rgColSym[MAX_COLS] = {COL_ADDRESS,COL_SYMBOL,COL_SYMBOL};

					if(!AddLine(rgszSym,rgColSym,MAX_COLS))
					{
						ASSERT(FALSE);
						pLstasm->ReleaseListings();
						EndDeferAddLine();
						return FALSE;
					}

					std::pair<std::map<DWORD, DWORD>::iterator,bool> ret = 
						m_pmapVA2LineNo->
							insert(std::map<DWORD, DWORD>::value_type(ins.first,nLine++));

					ASSERT(ret.second);

					if(!ret.second)
					{
						EndDeferAddLine();
						pLstasm->ReleaseListings();
						return FALSE;
					}

					if(!AddLine(rgszText,rgColors,MAX_COLS,pInst))
					{
						ASSERT(FALSE);
						EndDeferAddLine();
						pLstasm->ReleaseListings();
						return FALSE;
					}
					nLine++;
				}
				else
				{
					if(!AddLine(rgszText,rgColors,MAX_COLS,pInst))
					{
						ASSERT(FALSE);
						EndDeferAddLine();
						pLstasm->ReleaseListings();
						return FALSE;
					}

					std::pair<std::map<DWORD, DWORD>::iterator,bool> ret = 
						m_pmapVA2LineNo->
							insert(std::map<DWORD, DWORD>::value_type(ins.first,nLine++));

					ASSERT(ret.second);

					if(!ret.second)
					{
						EndDeferAddLine();
						pLstasm->ReleaseListings();
						return FALSE;
					}
				}
			}
		}

		// Separator Line
		
		LPCTSTR rgszText[MAX_COLS] = {_T("========"),_T("=================="),_T("========================================")};
		COLORREF rgColors[MAX_COLS] = {COL_ADDRESS,COL_ADDRESS,COL_ADDRESS};

		if(!AddLine(rgszText,rgColors,MAX_COLS))
		{
			ASSERT(FALSE);
			EndDeferAddLine();
			pLstasm->ReleaseListings();
			return FALSE;
		}
		nLine++;
	}

	EndDeferAddLine();
	pLstasm->ReleaseListings();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////

BOOL CDasmView::MoveToAddress(DWORD dwAddress)
{
	if(NULL == m_pmapVA2LineNo)
	{
		return FALSE;
	}

	std::map<DWORD, DWORD>::iterator entry = m_pmapVA2LineNo->find(dwAddress);

	if(m_pmapVA2LineNo->end() == entry)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return SetCurrentLine(entry->second);
}

///////////////////////////////////////////////////////////////////////////////

void CDasmView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_fDrawJump = FALSE;
	CTextView::OnKeyDown(nChar, nRepCnt, nFlags);

	// if curline is local jump display up down arrow and highlight destination

	if(VK_RETURN == nChar)
	{
		// See if we can jump in here

		CDasmBaseDoc* pDasmDoc = GetDocument();
		ASSERT(pDasmDoc);
		if(NULL == pDasmDoc)
		{
			return;
		}

		PINSTRUCTION pInst = (PINSTRUCTION)GetLineParam(GetCurrentLine());
		if(NULL == pInst)
		{
			return;
		}

		if(pInst->m_ulDirectJumpLoc)
		{
			m_stkCall.push(pInst->m_ulAddress);
			MoveToAddress(pInst->m_ulDirectJumpLoc);
		}
	}

	if(VK_BACK == nChar || VK_ESCAPE == nChar)
	{
		if(!m_stkCall.size())
		{
			return;
		}

		MoveToAddress(m_stkCall.top());

		m_stkCall.pop();
	}

	CheckCurrentLineJump();

	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

void CDasmView::CheckCurrentLineJump()
{
	int line = GetCurrentLine();

	// See if we can jump in here
	CDasmBaseDoc* pDasmDoc = GetDocument();
	ASSERT(pDasmDoc);
	if(NULL == pDasmDoc)
	{
		return;
	}

	//PCODE_LISTINGS_MANAGER pLstasm = pDasmDoc->GetDasmList();
	//ASSERT(pLstasm);
	
	//if(!pLstasm || pLstasm->MoveToLine(line) != ERR_SUCCESS)
	//{
	//	return;
	//}

	//CAsmLine* pLine = pLstasm->GetCurrentItem();

	//if(!pLine || !(pLine->IsJumpInst() || pLine->IsCallInst()))
	//{
	//	return;
	//}

	//DWORD dwJump = pLine->GetJumpLoc();

	//// Check if its an internal jump

	//if(pLstasm->MoveToAddress(dwJump) != ERR_SUCCESS)
	//{
	//	return;
	//}

	//// Local jump draw up down arrows and destination line

	//m_fDrawJump = TRUE;
	//m_nDestLine = pLstasm->GetCurrentVisLineNumber();
	//m_fDestSouth = line < m_nDestLine;
}

///////////////////////////////////////////////////////////////////////////////

void CDasmView::OnDraw(CDC* pDC)
{
	CTextView::OnDraw(pDC);

	//if(m_fDrawJump)
	//{
	//	CRect rcWnd;
	//	GetTrueClientRect(rcWnd);

	//	COLORREF colDest = COL_DESTLINE;

	//	HilightLine(pDC,rcWnd,m_nDestLine,&colDest);

 //       CFont* pfntold = (CFont*)pDC->SelectObject(m_pFont);
	//	//DrawLine(pDC,m_nDestLine);
	//	pDC->SelectObject(pfntold);

	//	
	//	int line = GetCurrentLine();



	//	//insert bitmap

	//	CDC dc2;
	//	dc2.CreateCompatibleDC(pDC);
	//	dc2.SelectObject(m_fDestSouth ? m_pBmpDown : m_pBmpUp);

	//	pDC->BitBlt(rcWnd.left,line * m_szChar.cy + rcWnd.top,15,17,&dc2,0,0,
	//		SRCCOPY);
	//	pDC->BitBlt(rcWnd.right - 15,line * m_szChar.cy + rcWnd.top,15,17,&
	//		dc2,0,0,SRCCOPY);
	//}
}

///////////////////////////////////////////////////////////////////////////////

void CDasmView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_fDrawJump = FALSE;

	CTextView::OnLButtonUp(nFlags, point);

	CheckCurrentLineJump();
	
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

void CDasmView::OnDasmGoto()
{
	CDasmGotoDialog dlg;

	if(IDOK == dlg.DoModal())
	{
		TCHAR szAddress[256];

		dlg.GetAddress(szAddress,255);

		DWORD dwAddress = _tcstoul(szAddress,NULL,16);

		if(!MoveToAddress(dwAddress))
		{
			AfxMessageBox(_T("Could not move to Address..."));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// CDasmGotoDialog dialog

IMPLEMENT_DYNAMIC(CDasmGotoDialog, CDialog)
CDasmGotoDialog::CDasmGotoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDasmGotoDialog::IDD, pParent)
{
}

CDasmGotoDialog::~CDasmGotoDialog()
{
}

BEGIN_MESSAGE_MAP(CDasmGotoDialog, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////
// CDasmGotoDialog message handlers

void CDasmGotoDialog::OnBnClickedOk()
{
	CComboBox* pCmb = (CComboBox*)GetDlgItem(IDC_CMBADDRESS);
	Assert(pCmb);

	pCmb->GetWindowText(m_szAddress,DASM_GOTO_CCHMAX_ADDRESS - 1);

	OnOK();
}

/////////////////////////////////////////////////////////////////////

void CDasmGotoDialog::GetAddress(LPTSTR lpszAddress, int nMaxCount)
{
	_tcsncpy(lpszAddress,m_szAddress,
		((nMaxCount > DASM_GOTO_CCHMAX_ADDRESS - 1) ? 
		DASM_GOTO_CCHMAX_ADDRESS-1 : nMaxCount));
}

/////////////////////////////////////////////////////////////////////// 

IMPLEMENT_DYNAMIC(CGlobalViewBar, CDialogBar)

/////////////////////////////////////////////////////////////////////// 

BEGIN_MESSAGE_MAP(CGlobalViewBar, CDialogBar)
	ON_WM_PAINT()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////

void CGlobalViewBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogBar::OnPaint() for painting messages

	CRect rc;
	GetClientRect(rc);

	rc.DeflateRect(2,2,2,2);

	static CBrush brsh(RGB(0,0,0));

	dc.FillRect(rc,&brsh);
}
