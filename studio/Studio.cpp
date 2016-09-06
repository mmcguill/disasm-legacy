///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Studio.h"
#include "MainFrm.h"

#include "ResourcesDoc.h"
#include "ResourcesFrm.h"
#include "ResourcesView.h"

#include "DasmDoc.h"
#include "DasmFrame.h"
#include "DasmView.h"

#include "ChooseViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CStudioDocTemplate,CMultiDocTemplate)

///////////////////////////////////////////////////////////////////////////////

typedef struct _DOCTEMPLATE_HELPER
{
	UINT uID;
	STUDIODOCTYPE stDocType;
	BOOL fGenericViewerCapable; // Can be usedt view any kind of data not just specially formatted
	CRuntimeClass* rtcView;
	CRuntimeClass* rtcDoc;
	CRuntimeClass* rtcFrame;
}DOCTEMPLATE_HELPER,*LPDOCTEMPLATE_HELPER;

///////////////////////////////////////////////////////////////////////////////

#define DOCTEMP_ENTRY(a,b,c,d,e,f,g) {a,{StudioMajorType##b,StudioDocType##c},d,RUNTIME_CLASS(e),RUNTIME_CLASS(f),RUNTIME_CLASS(g)},

static DOCTEMPLATE_HELPER rgDocTemplate[] = 
{
	DOCTEMP_ENTRY(IDR_MAINFRAME,	Workspace,	Workspace,	FALSE,	CStudioView,	CStudioDoc,	CStudioFrame)
	DOCTEMP_ENTRY(IDR_ICONTYPE,		Resource,	Icon,		FALSE,	CIconView,		CIconDoc,	CIconFrame)
	DOCTEMP_ENTRY(IDR_HEXTYPE,		Resource,	Hex,		TRUE,	CHexView,		CHexDoc,	CResBaseFrame)
	DOCTEMP_ENTRY(IDR_ACCELTYPE,	Resource,	Accelerator,FALSE,	CAccelView,		CAccelDoc,	CResBaseFrame)
	DOCTEMP_ENTRY(IDR_BITMAPTYPE,	Resource,	Bitmap,		FALSE,	CBmpView,		CBmpDoc,	CResBaseFrame)
	DOCTEMP_ENTRY(IDR_DIALOGTYPE,	Resource,	Dialog,		FALSE,	CDlgView,		CDlgDoc,	CResBaseFrame)
	DOCTEMP_ENTRY(IDR_MENUTYPE,		Resource,	Menu,		FALSE,	CMenuView,		CMenuDoc,	CResBaseFrame)
	DOCTEMP_ENTRY(IDR_STRINGTABLETYPE,Resource,	StringTable,FALSE,CStringTableView,	CStringTableDoc,CResBaseFrame)
	DOCTEMP_ENTRY(IDR_VERSIONTYPE,	Resource,	Version,	FALSE,	CVersionView,	CVersionDoc,CResBaseFrame)
	DOCTEMP_ENTRY(IDR_DASMTYPE,		Dasm,		Dasm,		FALSE,	CDasmView,		CDasmBaseDoc,	CDasmBaseFrame)
};


#define NUMBER_OF_DOCTEMPLATES (sizeof(rgDocTemplate) / sizeof(DOCTEMPLATE_HELPER))

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStudioApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_OPEN_WORKSPACE, OnFileOpenWorkspace)
	ON_COMMAND(ID_FILE_CLOSE_WORKSPACE, OnFileCloseWorkspace)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE_WORKSPACE, OnUpdateFileCloseWorkspace)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	ON_COMMAND(ID_FILE_CLOSEALLWINDOWS, OnFileCloseallwindows)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEALLWINDOWS, OnUpdateFileCloseallwindows)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

CStudioApp theApp;

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	
	// Standard initialization
	
	SetRegistryKey(_T("McGuill"));
	LoadStdProfileSettings(4); // Load standard INI file options (including MRU)
	

	// Register the application's document templates.  Document templates
	// serve as the connection between documents, frame windows and views
	
	for(int i=0;i<NUMBER_OF_DOCTEMPLATES;i++)
	{
		CStudioDocTemplate* pDocTemplate;

		pDocTemplate = new CStudioDocTemplate(
				rgDocTemplate[i].uID,
				&rgDocTemplate[i].stDocType,
				rgDocTemplate[i].fGenericViewerCapable,
				rgDocTemplate[i].rtcDoc,
				rgDocTemplate[i].rtcFrame,
				rgDocTemplate[i].rtcView);

		ASSERT(pDocTemplate);

		if (!pDocTemplate)
		{
			return FALSE;
		}

		AddDocTemplate(pDocTemplate);
	}


	// create main MDI Frame window
	
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		return FALSE;
	}

	m_pMainWnd = pMainFrame;
	
	
	// call DragAcceptFiles only if there's a suffix
	// In an MDI app, this should occur immediately after setting m_pMainWnd
	// Parse command line for standard shell commands, DDE, file open
	
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	
	
	// Dont Open a new Doc on launch

	cmdInfo.m_nShellCommand = cmdInfo.FileNothing;

	if (!ProcessShellCommand(cmdInfo))
	{
		return FALSE;
	}

	
	// The main window has been initialized, so show and update it

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioApp::CreateOrActivateViewOfWorkspaceItem(
		CStudioWorkspaceItemInfo* pItemInfo, 
		LPSTUDIODOCTYPE psdtViewAsDocType/* = NULL*/)
{
	ASSERT(pItemInfo);
	if(NULL == pItemInfo)
	{
		return FALSE;
	}

	// Use Default Doc if none specified

	if(NULL == psdtViewAsDocType)
	{
		psdtViewAsDocType = pItemInfo->GetDefaultDocType();
	}

	// Find The Required Doc Template For this item

	CStudioDocTemplate* pDT = GetDocTemplateFromStudioDocType(psdtViewAsDocType);

	if(NULL == pDT)
	{
        return FALSE;
	}

	
	// Check if open already, if so activate it instead

	BOOL fAlreadyOpen = FALSE;
	if(!ActivateIfAlreadyOpen(pDT,pItemInfo,&fAlreadyOpen))
	{
		return FALSE;
	}

	if(!fAlreadyOpen && !CreateNew(pDT,pItemInfo))
    {
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioApp::ActivateIfAlreadyOpen(CStudioDocTemplate* pDT,
									   CStudioWorkspaceItemInfo* pItemInfo, 
									   BOOL *pfAlreadyOpen)
{
	ASSERT(pDT);
	ASSERT(pItemInfo);
	ASSERT(pfAlreadyOpen);

	*pfAlreadyOpen = FALSE;
	BOOL bFound = FALSE;

    POSITION p = pDT->GetFirstDocPosition();
	if(NULL == p)
	{
		return TRUE; // No error here
	}

	while (p != NULL && !bFound)
	{
		CWorkspaceItemBaseDoc* pDoc = 
			static_cast<CWorkspaceItemBaseDoc*>(pDT->GetNextDoc(p));

		// Use This Items Default Doc Type and nID as a Unique Identifier
		// If this is equal to our current items Unique ID Then we have
		// a match

		// Exception if default doc type is hex!

		if( (pDoc->GetWorkspaceItemInfo()->GetDefaultDocType()->minor == 
			pItemInfo->GetDefaultDocType()->minor) && 
			(pDoc->GetWorkspaceItemInfo()->GetUniqueItemID() == 
			pItemInfo->GetUniqueItemID()))
		{
			bFound = TRUE;

			// Allow Update from this Item

			if(!pDoc->UpdateWorkspaceItemInfo(pItemInfo))
			{
				pDoc->OnCloseDocument();
				ASSERT(FALSE);
				return FALSE;
			}

			
			// Activate The Owner Frame...

			POSITION p2 = pDoc->GetFirstViewPosition();
			if(NULL == p2)
			{
				ASSERT(FALSE);
				return FALSE;
			}
				
			CView* pVw = pDoc->GetNextView(p2);

			if(NULL == pVw)
			{
				ASSERT(FALSE);
				return FALSE;
			}
			
			CFrameWnd* pFrm = pVw->GetParentFrame();
			if(NULL == pFrm)
			{
				ASSERT(FALSE);
				return FALSE;
			}
			
			pFrm->ActivateFrame();
		}
	}

	*pfAlreadyOpen = bFound;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioApp::CreateNew(CStudioDocTemplate* pDT,CStudioWorkspaceItemInfo* pItemInfo)
{
	ASSERT(pDT);
	ASSERT(pItemInfo);

	CWorkspaceItemBaseDoc* pDoc = (CResBaseDoc*)pDT->OpenDocumentFile(NULL,TRUE);
	ASSERT(pDoc);
	if(!pDoc)
	{
		return FALSE;
	}

	if(!pDoc->SetWorkspaceItemInfo(pItemInfo))
	{
		pDoc->OnCloseDocument();
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

CStudioDocTemplate* CStudioApp::GetDocTemplateFromStudioDocType(
	LPSTUDIODOCTYPE psdtViewAsDocType)
{
	ASSERT(psdtViewAsDocType);

	// If This is a resource & tyype unknown offer user choice of viewers

	if( StudioMajorTypeResource == psdtViewAsDocType->major && 
		StudioDocTypeUnknown == psdtViewAsDocType->minor)
	{
		CChooseViewerDlg dlg;
		if(IDOK == dlg.DoModal())
		{
			psdtViewAsDocType->major = dlg.GetStudioDocType()->major;
			psdtViewAsDocType->minor = dlg.GetStudioDocType()->minor;
		}
		else
		{
			return NULL;
		}
	}

	BOOL bFound = FALSE;
    CStudioDocTemplate* pCurDT;
	POSITION p = AfxGetApp()->GetFirstDocTemplatePosition();

	if(p == NULL)
	{
		ASSERT(FALSE); // Serious Problem if this happens
		return NULL;
	}

	do
	{
		pCurDT = static_cast<CStudioDocTemplate*>(AfxGetApp()->GetNextDocTemplate(p));
		ASSERT(pCurDT);

		if(	psdtViewAsDocType->minor == pCurDT->GetStudioDocType()->minor &&
			psdtViewAsDocType->major == pCurDT->GetStudioDocType()->major)
		{
			bFound = TRUE;
			break; // Got Template, Bail Out
		}
	}while(NULL != p);

	if(!bFound)
	{
		ASSERT(FALSE); // Couldnt find a template for this item
		return NULL;
	}

	return pCurDT;
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnFileOpen()
{
	ASSERT(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnFileNew()
{
	ASSERT(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioApp::OnOpenRecentFile(UINT nID)
{
	CloseAllDocuments(FALSE);

	return CWinApp::OnOpenRecentFile(nID);
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnFileOpenWorkspace()
{
	CString strFile;
	if(DoPromptFileName(strFile,IDS_FILE_OPEN_WORKSPACE_TITLE,OFN_EXPLORER,TRUE,NULL))
	{
		CloseAllDocuments(FALSE);	
		OpenDocumentFile(strFile);
	}
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnFileCloseWorkspace()
{
	CloseAllDocuments(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnUpdateFileCloseWorkspace(CCmdUI *pCmdUI)
{

	POSITION pos = GetFirstDocTemplatePosition();	
	BOOL bFound = FALSE;

	while(pos && !bFound)
	{
		CStudioDocTemplate* pTemplate = (CStudioDocTemplate*)GetNextDocTemplate(pos);
		
		if(pTemplate->GetFirstDocPosition() != NULL)
		{
			bFound = TRUE;
			break;
		}
	}
	
	pCmdUI->Enable(bFound);
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnFileCloseallwindows()
{
	POSITION pos = GetFirstDocTemplatePosition();	

	while(pos)
	{
		CStudioDocTemplate* pTemplate = (CStudioDocTemplate*)GetNextDocTemplate(pos);
				
		if(StudioDocTypeWorkspace != pTemplate->GetStudioDocType()->minor)
		{
			pTemplate->CloseAllDocuments(FALSE);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CStudioApp::OnUpdateFileCloseallwindows(CCmdUI *pCmdUI)
{
	// As long as we find an open document (excluding the workspace)
	// we should enable close 'all' windows button item

	POSITION pos = GetFirstDocTemplatePosition();	
	BOOL bFound = FALSE;

	while(pos && !bFound)
	{
		CStudioDocTemplate* pTemplate = (CStudioDocTemplate*)GetNextDocTemplate(pos);
		

		if(StudioDocTypeWorkspace != pTemplate->GetStudioDocType()->minor)
		{
			if(pTemplate->GetFirstDocPosition() != NULL)
			{
				bFound = TRUE;
				break;
			}
		}
	}
	
	pCmdUI->Enable(bFound);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CStudioDoc, CDocument)

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStudioDoc, CDocument)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

CStudioDoc::CStudioDoc()
{
	m_pResParser = NULL;
	m_pExeParser = NULL;
	m_hDisasm = NULL;
	m_pSymTab = NULL;
}

///////////////////////////////////////////////////////////////////////////////

CStudioDoc::~CStudioDoc()
{
	Cleanup();
}

///////////////////////////////////////////////////////////////////////////////

void CStudioDoc::Cleanup()
{
	if(NULL != m_pResParser)
	{
		delete m_pResParser;
		m_pResParser = NULL;
	}

	if(NULL != m_pExeParser)
	{
		delete m_pExeParser;
        m_pExeParser = NULL;
	}

	if(NULL != m_hDisasm)
	{
		CloseDisasm(m_hDisasm);
		m_hDisasm = NULL;
	}

	m_strFile.Empty();
}

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CStudioDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CStudioDoc::AssertValid() const
{
	CDocument::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////

void CStudioDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

BOOL CStudioDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// Stage 1 - Resources

	m_pResParser = new CPERes();

	ASSERT(m_pResParser);
	if(NULL == m_pResParser)
	{
		return FALSE;
	}

	UINT nError;
	if(!m_pResParser->FileLoad((const LPTSTR)lpszPathName,&nError))
	{
		// TODO: Put in something useful here!
		AfxMessageBox(_T("Error Opening File."),MB_ICONINFORMATION);
		return FALSE;
	}
	
	
	// Stage 2 - The Core Disassembly Class

	m_pExeParser = new CPEParser();

	ASSERT(m_pExeParser);
	if(NULL == m_pExeParser)
	{
		Cleanup();
		return FALSE;
	}

	if(!m_pExeParser->Open(lpszPathName))
	{
		// TODO: Put in something useful here!
		DWORD dwErr = PEDasmGetLastError();

		TCHAR szErr[256];
		PEDasmGetErrorString(dwErr,szErr,256);

		AfxMessageBox(szErr,MB_ICONINFORMATION);
		return FALSE;
	}

	// Stage 3 - Allow File Parser to Fill Symbol Table with
	//			 Any known points in the file. e.g entry points
	//			 exports, imports, etc...
	//			 We also create the disassembly engine here...
	//			 NOTE : No Disassemly is initiated, just symbolic
	//			 stuff at this stage...

	PBYTE pucImage= m_pExeParser->GetImage();
	ULONG ulBase =	m_pExeParser->GetImageBase();
	ULONG ulSize =	m_pExeParser->GetImageSize();

	m_hDisasm = CreateDisasm(TRUE,pucImage,ulSize,ulBase,TRUE);
	ASSERT(m_hDisasm);

	if(NULL == m_hDisasm)
	{
		Cleanup();
		return FALSE;
	}

	m_pSymTab = ::GetSymTable(m_hDisasm);
	Assert(m_pSymTab);
	
	if(NULL == m_pSymTab)
	{
		Cleanup();
		return FALSE;
	}
	
	// Ask File parser to load symbol table with goodies!

	m_pExeParser->LoadSymbols(m_pSymTab);
	
	m_strFile = lpszPathName;

	// Stage 4 - Loaded => Update Workspace

	UPDATE_WORKSPACE_PARAM uwp;
	uwp.event = uwp_event_opening;
	uwp.pDocument = this;

	AfxGetMainWnd()->SendMessage(WM_UPDATEWORKSPACE,(WPARAM)&uwp,0);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void CStudioDoc::OnCloseDocument()
{
	// Close Parsers

	Cleanup();

	// Update Workspace

	UPDATE_WORKSPACE_PARAM uwp;
	uwp.event = uwp_event_closing;
	uwp.pDocument = this;

	AfxGetMainWnd()->SendMessage(WM_UPDATEWORKSPACE,(WPARAM)&uwp,0);
	
	// HACK:	Need to set focus else where as if there is an active 
	//			MDI window an assertion is caused in MFC. Sigh...

	AfxGetMainWnd()->SetFocus(); 

    CDocument::OnCloseDocument();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CStudioFrame, CMDIChildWnd)

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStudioFrame, CMDIChildWnd)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CStudioView, CView)

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStudioView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWorkspaceItemBaseDoc,CDocument)

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CWorkspaceItemBaseDoc, CDocument)
	//{{AFX_MSG_MAP(CResBaseDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////