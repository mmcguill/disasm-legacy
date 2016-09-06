// DlgPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Studio.h"
#include "DlgPropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPropDlg dialog


CDlgPropDlg::CDlgPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPropDlg)
	m_strData = _T("");
	//}}AFX_DATA_INIT
}


void CDlgPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPropDlg)
	DDX_Text(pDX, IDC_EDCONTROL, m_strData);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPropDlg, CDialog)
	//{{AFX_MSG_MAP(CDlgPropDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPropDlg message handlers

BOOL CDlgPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	DoStyles();
	
	return TRUE;
}

void CDlgPropDlg::DoStyles()
{
	CListBox* pLB = (CListBox*)GetDlgItem(IDC_LBSTYLES);
	
	// Top level only
	if(m_type == DIALOG){
		if((m_dwStyle & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW){
			pLB->AddString(_T("WS_OVERLAPPEDWINDOW"));
			m_dwStyle &= (~WS_OVERLAPPEDWINDOW);
		}
		if((m_dwStyle & WS_POPUPWINDOW) == WS_POPUPWINDOW){
			pLB->AddString(_T("WS_POPUPWINDOW"));
			m_dwStyle &= (~WS_POPUPWINDOW);
		}
		if((m_dwStyle & WS_CAPTION) == WS_CAPTION){
			pLB->AddString(_T("WS_CAPTION"));
			m_dwStyle &= (~WS_CAPTION);
		}
		if((m_dwStyle & WS_MINIMIZE) == WS_MINIMIZE){
			pLB->AddString(_T("WS_MINIMIZE"));
			m_dwStyle &= (~WS_MINIMIZE);
		}
		if((m_dwStyle & WS_MAXIMIZE)== WS_MAXIMIZE){
			pLB->AddString(_T("WS_MAXIMIZE"));
			m_dwStyle &= (~WS_MAXIMIZE);
		}
		// ordinary single styles
		if((m_dwStyle & WS_OVERLAPPED)==WS_OVERLAPPED) {
			pLB->AddString(_T("WS_OVERLAPPED"));
			m_dwStyle &= (~WS_OVERLAPPED);
		}
		if((m_dwStyle & WS_SYSMENU)==WS_SYSMENU){
			pLB->AddString(_T("WS_SYSMENU"));
			m_dwStyle &= (~WS_SYSMENU);
		}
		if((m_dwStyle & WS_THICKFRAME)==WS_THICKFRAME){
			pLB->AddString(_T("WS_THICKFRAME"));
			m_dwStyle &= (~WS_THICKFRAME);
		}
		if((m_dwStyle & WS_POPUP)==WS_POPUP){
			pLB->AddString(_T("WS_POPUP"));
			m_dwStyle &= (~WS_POPUP);
		}
		if((m_dwStyle & DS_ABSALIGN) == DS_ABSALIGN){
			pLB->AddString(_T("DS_ABSALIGN"));
			m_dwStyle &= (~DS_ABSALIGN);
		}
		if((m_dwStyle & DS_SYSMODAL) == DS_SYSMODAL){
			pLB->AddString(_T("DS_SYSMODAL"));
			m_dwStyle &= (~DS_SYSMODAL);
		}
		if((m_dwStyle & DS_LOCALEDIT) == DS_LOCALEDIT){
			pLB->AddString(_T("DS_LOCALEDIT"));
			m_dwStyle &= (~DS_LOCALEDIT);
		}
		if((m_dwStyle & DS_SETFONT) == DS_SETFONT){
			pLB->AddString(_T("DS_SETFONT"));
			m_dwStyle &= (~DS_SETFONT);
		}
		if((m_dwStyle & DS_MODALFRAME) == DS_MODALFRAME){
			pLB->AddString(_T("DS_MODALFRAME"));
			m_dwStyle &= (~DS_MODALFRAME);
		}
		if((m_dwStyle & DS_NOIDLEMSG) == DS_NOIDLEMSG){
			pLB->AddString(_T("DS_NOIDLEMSG"));
			m_dwStyle &= (~DS_NOIDLEMSG);
		}
		if((m_dwStyle & DS_SETFOREGROUND) == DS_SETFOREGROUND){
			pLB->AddString(_T("DS_SETFOREGROUND"));
			m_dwStyle &= (~DS_SETFOREGROUND);
		}
		if((m_dwStyle & DS_3DLOOK) == DS_3DLOOK){
			pLB->AddString(_T("DS_3DLOOK"));
			m_dwStyle &= (~DS_3DLOOK);
		}
		if((m_dwStyle & DS_FIXEDSYS) == DS_FIXEDSYS){
			pLB->AddString(_T("DS_FIXEDSYS"));
			m_dwStyle &= (~DS_FIXEDSYS);
		}
		if((m_dwStyle & DS_NOFAILCREATE) == DS_NOFAILCREATE){
			pLB->AddString(_T("DS_NOFAILCREATE"));
			m_dwStyle &= (~DS_NOFAILCREATE);
		}
		if((m_dwStyle & DS_CONTROL) == DS_CONTROL){
			pLB->AddString(_T("DS_CONTROL"));
			m_dwStyle &= (~DS_CONTROL);
		}
		if((m_dwStyle & DS_CENTER) == DS_CENTER){
			pLB->AddString(_T("DS_CENTER"));
			m_dwStyle &= (~DS_CENTER);
		}
		if((m_dwStyle & DS_CENTERMOUSE) == DS_CENTERMOUSE){
			pLB->AddString(_T("DS_CENTERMOUSE"));
			m_dwStyle &= (~DS_CENTERMOUSE);
		}
		if((m_dwStyle & DS_CONTEXTHELP) == DS_CONTEXTHELP){
			pLB->AddString(_T("DS_CONTEXTHELP"));
			m_dwStyle &= (~DS_CONTEXTHELP);
		}
	}

	// The following styles apply to any window
	if((m_dwStyle & WS_CHILD)==WS_CHILD){
		pLB->AddString(_T("WS_CHILD"));
		m_dwStyle &= (~WS_CHILD);
	}
	if((m_dwStyle & WS_BORDER)==WS_BORDER){
		pLB->AddString(_T("WS_BORDER"));
		m_dwStyle &= (~WS_BORDER);
	}
	if((m_dwStyle & WS_VISIBLE)==WS_VISIBLE){
		pLB->AddString(_T("WS_VISIBLE"));
		m_dwStyle &= (~WS_VISIBLE);
	}
	if((m_dwStyle & WS_DISABLED)==WS_DISABLED){
		pLB->AddString(_T("WS_DISABLED"));
		m_dwStyle &= (~WS_DISABLED);
	}
	if((m_dwStyle & WS_CLIPSIBLINGS)==WS_CLIPSIBLINGS){
		pLB->AddString(_T("WS_CLIPSIBLINGS"));
		m_dwStyle &= (~WS_CLIPSIBLINGS);
	}
	if((m_dwStyle & WS_CLIPCHILDREN)==WS_CLIPCHILDREN){
		pLB->AddString(_T("WS_CLIPCHILDREN"));
		m_dwStyle &= (~WS_CLIPCHILDREN);
	}
	if((m_dwStyle & WS_DLGFRAME)==WS_DLGFRAME){
		pLB->AddString(_T("WS_DLGFRAME"));
		m_dwStyle &= (~WS_DLGFRAME);
	}
	if((m_dwStyle & WS_VSCROLL)==WS_VSCROLL){
		pLB->AddString(_T("WS_VSCROLL"));
		m_dwStyle &= (~WS_VSCROLL);
	}
	if((m_dwStyle & WS_HSCROLL)==WS_HSCROLL){
		pLB->AddString(_T("WS_HSCROLL"));
		m_dwStyle &= (~WS_HSCROLL);
	}
	if((m_dwStyle & WS_GROUP)==WS_GROUP){
		pLB->AddString(_T("WS_GROUP"));
		m_dwStyle &= (~WS_GROUP);
	}
	if((m_dwStyle & WS_TABSTOP)==WS_TABSTOP){
		pLB->AddString(_T("WS_TABSTOP"));
		m_dwStyle &= (~WS_TABSTOP);
	}

	//BUTTON
	if(m_type == BUTTON && m_dwStyle != 0){ // Chekc combination styles first
		if((m_dwStyle & BS_PUSHLIKE) == BS_PUSHLIKE){
			pLB->AddString(_T("BS_PUSHLIKE"));
			m_dwStyle &= (~BS_PUSHLIKE);
		}
		if((m_dwStyle & BS_MULTILINE) == BS_MULTILINE){
			pLB->AddString(_T("BS_MULTILINE"));
			m_dwStyle &= (~BS_MULTILINE);
		}
		if((m_dwStyle & BS_NOTIFY) == BS_NOTIFY){
			pLB->AddString(_T("BS_NOTIFY"));
			m_dwStyle &= (~BS_NOTIFY);
		}
		if((m_dwStyle & BS_FLAT) == BS_FLAT){
			pLB->AddString(_T("BS_FLAT"));
			m_dwStyle &= (~BS_FLAT);
		}

		if((m_dwStyle & BS_VCENTER) == BS_VCENTER){
			pLB->AddString(_T("BS_VCENTER"));
			m_dwStyle &= (~BS_VCENTER);
		}
		if((m_dwStyle & BS_TOP) == BS_TOP){
			pLB->AddString(_T("BS_TOP"));
			m_dwStyle &= (~BS_TOP);
		}
		if((m_dwStyle & BS_BOTTOM) == BS_BOTTOM){
			pLB->AddString(_T("BS_BOTTOM"));
			m_dwStyle &= (~BS_BOTTOM);
		}
		if((m_dwStyle & BS_CENTER) == BS_CENTER){
			pLB->AddString(_T("BS_CENTER"));
			m_dwStyle &= (~BS_CENTER);
		}
		if((m_dwStyle & BS_LEFT) == BS_LEFT){
			pLB->AddString(_T("BS_LEFT"));
			m_dwStyle &= (~BS_LEFT);
		}
		if((m_dwStyle & BS_RIGHT) == BS_RIGHT){
			pLB->AddString(_T("BS_RIGHT"));
			m_dwStyle &= (~BS_RIGHT);
		}

		if((m_dwStyle & BS_ICON) == BS_ICON){
			pLB->AddString(_T("BS_ICON"));
			m_dwStyle &= (~BS_ICON);
		}
		if((m_dwStyle & BS_BITMAP) == BS_BITMAP){
			pLB->AddString(_T("BS_BITMAP"));
			m_dwStyle &= (~BS_BITMAP);
		}
		if((m_dwStyle & BS_LEFTTEXT) == BS_LEFTTEXT){
			pLB->AddString(_T("BS_LEFTTEXT"));
			m_dwStyle &= (~BS_LEFTTEXT);
		}

		if((m_dwStyle & BS_OWNERDRAW) == BS_OWNERDRAW){
			pLB->AddString(_T("BS_OWNERDRAW"));
			m_dwStyle &= (~BS_OWNERDRAW);
		}
		if((m_dwStyle & BS_AUTORADIOBUTTON) == BS_AUTORADIOBUTTON){
			pLB->AddString(_T("BS_AUTORADIOBUTTON"));
			m_dwStyle &= (~BS_AUTORADIOBUTTON);
		}
		if((m_dwStyle & BS_USERBUTTON) == BS_USERBUTTON){
			pLB->AddString(_T("BS_USERBUTTON"));
			m_dwStyle &= (~BS_USERBUTTON);
		}
		if((m_dwStyle & BS_GROUPBOX) == BS_GROUPBOX){
			pLB->AddString(_T("BS_GROUPBOX"));
			m_dwStyle &= (~BS_GROUPBOX);
		}
		if((m_dwStyle & BS_AUTO3STATE) == BS_AUTO3STATE){
			pLB->AddString(_T("BS_AUTO3STATE"));
			m_dwStyle &= (~BS_AUTO3STATE);
		}
		if((m_dwStyle & BS_3STATE) == BS_3STATE){
			pLB->AddString(_T("BS_3STATE"));
			m_dwStyle &= (~BS_3STATE);
		}
		if((m_dwStyle & BS_RADIOBUTTON) == BS_RADIOBUTTON){
			pLB->AddString(_T("BS_RADIOBUTTON"));
			m_dwStyle &= (~BS_RADIOBUTTON);
		}
		if((m_dwStyle & BS_AUTOCHECKBOX) == BS_AUTOCHECKBOX){
			pLB->AddString(_T("BS_AUTOCHECKBOX"));
			m_dwStyle &= (~BS_AUTOCHECKBOX);
		}
		if((m_dwStyle & BS_CHECKBOX) == BS_CHECKBOX){
			pLB->AddString(_T("BS_CHECKBOX"));
			m_dwStyle &= (~BS_CHECKBOX);
		}
		if((m_dwStyle & BS_DEFPUSHBUTTON) == BS_DEFPUSHBUTTON){
			pLB->AddString(_T("BS_DEFPUSHBUTTON"));
			m_dwStyle &= (~BS_DEFPUSHBUTTON);
		}
	}
	//EDIT
	if(m_type == EDIT){
		if((m_dwStyle & ES_CENTER) == ES_CENTER){
			pLB->AddString(_T("ES_CENTER"));
			m_dwStyle &= (~ES_CENTER);
		}
		if((m_dwStyle & ES_RIGHT) == ES_RIGHT){
			pLB->AddString(_T("ES_RIGHT"));
			m_dwStyle &= (~ES_RIGHT);
		}
		if((m_dwStyle & ES_MULTILINE) == ES_MULTILINE){
			pLB->AddString(_T("ES_MULTILINE"));
			m_dwStyle &= (~ES_MULTILINE);
		}
		if((m_dwStyle & ES_UPPERCASE) == ES_UPPERCASE){
			pLB->AddString(_T("ES_UPPERCASE"));
			m_dwStyle &= (~ES_UPPERCASE);
		}
		if((m_dwStyle & ES_LOWERCASE) == ES_LOWERCASE){
			pLB->AddString(_T("ES_LOWERCASE"));
			m_dwStyle &= (~ES_LOWERCASE);
		}
		if((m_dwStyle & ES_PASSWORD) == ES_PASSWORD){
			pLB->AddString(_T("ES_PASSWORD"));
			m_dwStyle &= (~ES_PASSWORD);
		}
		if((m_dwStyle & ES_AUTOVSCROLL) == ES_AUTOVSCROLL){
			pLB->AddString(_T("ES_AUTOVSCROLL"));
			m_dwStyle &= (~ES_AUTOVSCROLL);
		}
		if((m_dwStyle & ES_AUTOHSCROLL) == ES_AUTOHSCROLL){
			pLB->AddString(_T("ES_AUTOHSCROLL"));
			m_dwStyle &= (~ES_AUTOHSCROLL);
		}
		if((m_dwStyle & ES_NOHIDESEL) == ES_NOHIDESEL){
			pLB->AddString(_T("ES_NOHIDESEL"));
			m_dwStyle &= (~ES_NOHIDESEL);
		}
		if((m_dwStyle & ES_OEMCONVERT) == ES_OEMCONVERT){
			pLB->AddString(_T("ES_OEMCONVERT"));
			m_dwStyle &= (~ES_OEMCONVERT);
		}
		if((m_dwStyle & ES_READONLY) == ES_READONLY){
			pLB->AddString(_T("ES_READONLY"));
			m_dwStyle &= (~ES_READONLY);
		}
		if((m_dwStyle & ES_WANTRETURN) == ES_WANTRETURN){
			pLB->AddString(_T("ES_WANTRETURN"));
			m_dwStyle &= (~ES_WANTRETURN);
		}
		if((m_dwStyle & ES_NUMBER) == ES_NUMBER){
			pLB->AddString(_T("ES_NUMBER"));
			m_dwStyle &= (~ES_NUMBER);
		}
	}
	//STATIC
	if(m_type == STATIC){
		if((m_dwStyle & SS_NOPREFIX) == SS_NOPREFIX){
			pLB->AddString(_T("SS_NOPREFIX"));
			m_dwStyle &= (~SS_NOPREFIX);
		}
		if((m_dwStyle & SS_ETCHEDFRAME) == SS_ETCHEDFRAME){
			pLB->AddString(_T("SS_ETCHEDFRAME"));
			m_dwStyle &= (~SS_ETCHEDFRAME);
		}
		if((m_dwStyle & SS_ETCHEDVERT) == SS_ETCHEDVERT){
			pLB->AddString(_T("SS_ETCHEDVERT"));
			m_dwStyle &= (~SS_ETCHEDVERT);
		}
		if((m_dwStyle & SS_ETCHEDHORZ) == SS_ETCHEDHORZ){
			pLB->AddString(_T("SS_ETCHEDHORZ"));
			m_dwStyle &= (~SS_ETCHEDHORZ);
		}
		if((m_dwStyle & SS_ENHMETAFILE) == SS_ENHMETAFILE){
			pLB->AddString(_T("SS_ENHMETAFILE"));
			m_dwStyle &= (~SS_ENHMETAFILE);
		}
		if((m_dwStyle & SS_BITMAP) == SS_BITMAP){
			pLB->AddString(_T("SS_BITMAP"));
			m_dwStyle &= (~SS_BITMAP);
		}
		if((m_dwStyle & SS_OWNERDRAW) == SS_OWNERDRAW){
			pLB->AddString(_T("SS_OWNERDRAW"));
			m_dwStyle &= (~SS_OWNERDRAW);
		}
		if((m_dwStyle & SS_LEFTNOWORDWRAP) == SS_LEFTNOWORDWRAP){
			pLB->AddString(_T("SS_LEFTNOWORDWRAP"));
			m_dwStyle &= (~SS_LEFTNOWORDWRAP);
		}
		if((m_dwStyle & SS_SIMPLE) == SS_SIMPLE){
			pLB->AddString(_T("SS_SIMPLE"));
			m_dwStyle &= (~SS_SIMPLE);
		}
		if((m_dwStyle & SS_USERITEM) == SS_USERITEM){
			pLB->AddString(_T("SS_USERITEM"));
			m_dwStyle &= (~SS_USERITEM);
		}
		if((m_dwStyle & SS_WHITEFRAME) == SS_WHITEFRAME){
			pLB->AddString(_T("SS_WHITEFRAME"));
			m_dwStyle &= (~SS_WHITEFRAME);
		}
		if((m_dwStyle & SS_GRAYFRAME) == SS_GRAYFRAME){
			pLB->AddString(_T("SS_GRAYFRAME"));
			m_dwStyle &= (~SS_GRAYFRAME);
		}
		if((m_dwStyle & SS_BLACKFRAME) == SS_BLACKFRAME){
			pLB->AddString(_T("SS_BLACKFRAME"));
			m_dwStyle &= (~SS_BLACKFRAME);
		}
		if((m_dwStyle & SS_WHITERECT) == SS_WHITERECT){
			pLB->AddString(_T("SS_WHITERECT"));
			m_dwStyle &= (~SS_WHITERECT);
		}
		if((m_dwStyle & SS_GRAYRECT) == SS_GRAYRECT){
			pLB->AddString(_T("SS_GRAYRECT"));
			m_dwStyle &= (~SS_GRAYRECT);
		}
		if((m_dwStyle & SS_BLACKRECT) == SS_BLACKRECT){
			pLB->AddString(_T("SS_BLACKRECT"));
			m_dwStyle &= (~SS_BLACKRECT);
		}
		if((m_dwStyle & SS_ICON) == SS_ICON){
			pLB->AddString(_T("SS_ICON"));
			m_dwStyle &= (~SS_ICON);
		}
		if((m_dwStyle & SS_RIGHT) == SS_RIGHT){
			pLB->AddString(_T("SS_RIGHT"));
			m_dwStyle &= (~SS_RIGHT);
		}
		if((m_dwStyle & SS_CENTER) == SS_CENTER){
			pLB->AddString(_T("SS_CENTER"));
			m_dwStyle &= (~SS_CENTER);
		}
		if((m_dwStyle & SS_NOTIFY) == SS_NOTIFY){
			pLB->AddString(_T("SS_NOTIFY"));
			m_dwStyle &= (~SS_NOTIFY);
		}
		if((m_dwStyle & SS_CENTERIMAGE) == SS_CENTERIMAGE){
			pLB->AddString(_T("SS_CENTERIMAGE"));
			m_dwStyle &= (~SS_CENTERIMAGE);
		}
		if((m_dwStyle & SS_RIGHTJUST) == SS_RIGHTJUST){
			pLB->AddString(_T("SS_RIGHTJUST"));
			m_dwStyle &= (~SS_RIGHTJUST);
		}
		if((m_dwStyle & SS_REALSIZEIMAGE) == SS_REALSIZEIMAGE){
			pLB->AddString(_T("SS_REALSIZEIMAGE"));
			m_dwStyle &= (~SS_REALSIZEIMAGE);
		}
		if((m_dwStyle & SS_SUNKEN) == SS_SUNKEN){
			pLB->AddString(_T("SS_SUNKEN"));
			m_dwStyle &= (~SS_SUNKEN);
		}
		if((m_dwStyle & SS_WORDELLIPSIS) == SS_WORDELLIPSIS){
			pLB->AddString(_T("SS_WORDELLIPSIS"));
			m_dwStyle &= (~SS_WORDELLIPSIS);
		}
		if((m_dwStyle & SS_ENDELLIPSIS) == SS_ENDELLIPSIS){
			pLB->AddString(_T("SS_ENDELLIPSIS"));
			m_dwStyle &= (~SS_ENDELLIPSIS);
		}
		if((m_dwStyle & SS_PATHELLIPSIS) == SS_PATHELLIPSIS){
			pLB->AddString(_T("SS_PATHELLIPSIS"));
			m_dwStyle &= (~SS_PATHELLIPSIS);
		}
	}
	//LISTBOX
	if(m_type == LISTBOX){
		if((m_dwStyle & LBS_NOTIFY) == LBS_NOTIFY){
			pLB->AddString(_T("LBS_NOTIFY"));
			m_dwStyle &= (~LBS_NOTIFY);
		}
		if((m_dwStyle & LBS_SORT) == LBS_SORT){
			pLB->AddString(_T("LBS_SORT"));
			m_dwStyle &= (~LBS_SORT);
		}
		if((m_dwStyle & LBS_NOREDRAW) == LBS_NOREDRAW){
			pLB->AddString(_T("LBS_NOREDRAW"));
			m_dwStyle &= (~LBS_NOREDRAW);
		}
		if((m_dwStyle & LBS_MULTIPLESEL) == LBS_MULTIPLESEL){
			pLB->AddString(_T("LBS_MULTIPLESEL"));
			m_dwStyle &= (~LBS_MULTIPLESEL);
		}
		if((m_dwStyle & LBS_OWNERDRAWFIXED) == LBS_OWNERDRAWFIXED){
			pLB->AddString(_T("LBS_OWNERDRAWFIXED"));
			m_dwStyle &= (~LBS_OWNERDRAWFIXED);
		}
		if((m_dwStyle & LBS_OWNERDRAWVARIABLE) == LBS_OWNERDRAWVARIABLE){
			pLB->AddString(_T("LBS_OWNERDRAWVARIABLE"));
			m_dwStyle &= (~LBS_OWNERDRAWVARIABLE);
		}
		if((m_dwStyle & LBS_HASSTRINGS) == LBS_HASSTRINGS){
			pLB->AddString(_T("LBS_HASSTRINGS"));
			m_dwStyle &= (~LBS_HASSTRINGS);
		}
		if((m_dwStyle & LBS_USETABSTOPS) == LBS_USETABSTOPS){
			pLB->AddString(_T("LBS_USETABSTOPS"));
			m_dwStyle &= (~LBS_USETABSTOPS);
		}
		if((m_dwStyle & LBS_NOINTEGRALHEIGHT) == LBS_NOINTEGRALHEIGHT){
			pLB->AddString(_T("LBS_NOINTEGRALHEIGHT"));
			m_dwStyle &= (~LBS_NOINTEGRALHEIGHT);
		}
		if((m_dwStyle & LBS_MULTICOLUMN) == LBS_MULTICOLUMN){
			pLB->AddString(_T("LBS_MULTICOLUMN"));
			m_dwStyle &= (~LBS_MULTICOLUMN);
		}
		if((m_dwStyle & LBS_WANTKEYBOARDINPUT) == LBS_WANTKEYBOARDINPUT){
			pLB->AddString(_T("LBS_WANTKEYBOARDINPUT"));
			m_dwStyle &= (~LBS_WANTKEYBOARDINPUT);
		}
		if((m_dwStyle & LBS_EXTENDEDSEL) == LBS_EXTENDEDSEL){
			pLB->AddString(_T("LBS_EXTENDEDSEL"));
			m_dwStyle &= (~LBS_EXTENDEDSEL);
		}
		if((m_dwStyle & LBS_DISABLENOSCROLL) == LBS_DISABLENOSCROLL){
			pLB->AddString(_T("LBS_DISABLENOSCROLL"));
			m_dwStyle &= (~LBS_DISABLENOSCROLL);
		}
		if((m_dwStyle & LBS_NODATA) == LBS_NODATA){
			pLB->AddString(_T("LBS_NODATA"));
			m_dwStyle &= (~LBS_NODATA);
		}
		if((m_dwStyle & LBS_NOSEL) == LBS_NOSEL){
			pLB->AddString(_T("LBS_NOSEL"));
			m_dwStyle &= (~LBS_NOSEL);
		}
	}
	//COMBOBOX
	if(m_type == COMBOBOX){
		if((m_dwStyle & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST){
			pLB->AddString(_T("CBS_DROPDOWNLIST"));
			m_dwStyle &= (~CBS_DROPDOWNLIST);
		}
		if((m_dwStyle & CBS_SIMPLE) == CBS_SIMPLE){
			pLB->AddString(_T("CBS_SIMPLE"));
			m_dwStyle &= (~CBS_SIMPLE);
		}
		if((m_dwStyle & CBS_DROPDOWN) == CBS_DROPDOWN){
			pLB->AddString(_T("CBS_DROPDOWN"));
			m_dwStyle &= (~CBS_DROPDOWN);
		}
		if((m_dwStyle & CBS_OWNERDRAWFIXED) == CBS_OWNERDRAWFIXED){
			pLB->AddString(_T("CBS_OWNERDRAWFIXED"));
			m_dwStyle &= (~CBS_OWNERDRAWFIXED);
		}
		if((m_dwStyle & CBS_OWNERDRAWVARIABLE) == CBS_OWNERDRAWVARIABLE){
			pLB->AddString(_T("CBS_OWNERDRAWVARIABLE"));
			m_dwStyle &= (~CBS_OWNERDRAWVARIABLE);
		}
		if((m_dwStyle & CBS_AUTOHSCROLL) == CBS_AUTOHSCROLL){
			pLB->AddString(_T("CBS_AUTOHSCROLL"));
			m_dwStyle &= (~CBS_AUTOHSCROLL);
		}
		if((m_dwStyle & CBS_OEMCONVERT) == CBS_OEMCONVERT){
			pLB->AddString(_T("CBS_OEMCONVERT"));
			m_dwStyle &= (~CBS_OEMCONVERT);
		}
		if((m_dwStyle & CBS_SORT) == CBS_SORT){
			pLB->AddString(_T("CBS_SORT"));
			m_dwStyle &= (~CBS_SORT);
		}
		if((m_dwStyle & CBS_HASSTRINGS) == CBS_HASSTRINGS){
			pLB->AddString(_T("CBS_HASSTRINGS"));
			m_dwStyle &= (~CBS_HASSTRINGS);
		}
		if((m_dwStyle & CBS_NOINTEGRALHEIGHT) == CBS_NOINTEGRALHEIGHT){
			pLB->AddString(_T("CBS_NOINTEGRALHEIGHT"));
			m_dwStyle &= (~CBS_NOINTEGRALHEIGHT);
		}
		if((m_dwStyle & CBS_DISABLENOSCROLL) == CBS_DISABLENOSCROLL){
			pLB->AddString(_T("CBS_DISABLENOSCROLL"));
			m_dwStyle &= (~CBS_DISABLENOSCROLL);
		}
		if((m_dwStyle & CBS_UPPERCASE) == CBS_UPPERCASE){
			pLB->AddString(_T("CBS_UPPERCASE"));
			m_dwStyle &= (~CBS_UPPERCASE);
		}
		if((m_dwStyle & CBS_LOWERCASE) == CBS_LOWERCASE){
			pLB->AddString(_T("CBS_LOWERCASE"));
			m_dwStyle &= (~CBS_LOWERCASE);
		}
	}
	//////////////////////////////////////////////
	//Extended Window Styles
	CListBox* pLBx = (CListBox*)GetDlgItem(IDC_LBEXSTYLES);
	if((m_dwExStyle & WS_EX_OVERLAPPEDWINDOW) == WS_EX_OVERLAPPEDWINDOW){
		pLBx->AddString(_T("WS_EX_OVERLAPPEDWINDOW"));
		m_dwExStyle &= (~WS_EX_OVERLAPPEDWINDOW);
	}
	if((m_dwExStyle & WS_EX_PALETTEWINDOW) == WS_EX_PALETTEWINDOW){
		pLBx->AddString(_T("WS_EX_PALETTEWINDOW"));
		m_dwExStyle &= (~WS_EX_PALETTEWINDOW);
	}

	// Single styles
	if((m_dwExStyle & WS_EX_WINDOWEDGE)==WS_EX_WINDOWEDGE){
		pLBx->AddString(_T("WS_EX_WINDOWEDGE"));
		m_dwExStyle &= (~WS_EX_WINDOWEDGE);
	}
	if((m_dwExStyle & WS_EX_CLIENTEDGE)== WS_EX_CLIENTEDGE){
		pLBx->AddString(_T("WS_EX_CLIENTEDGE"));
		m_dwExStyle &= (~WS_EX_CLIENTEDGE);
	}
	if((m_dwExStyle & WS_EX_TOOLWINDOW)== WS_EX_TOOLWINDOW){
		pLBx->AddString(_T("WS_EX_TOOLWINDOW"));
		m_dwExStyle &= (~WS_EX_TOOLWINDOW);
	}
	if((m_dwExStyle & WS_EX_TOPMOST)== WS_EX_TOPMOST){
		pLBx->AddString(_T("WS_EX_TOPMOST"));
		m_dwExStyle &= (~WS_EX_TOPMOST);
	}
	if((m_dwExStyle & WS_EX_DLGMODALFRAME)== WS_EX_DLGMODALFRAME){
		pLBx->AddString(_T("WS_EX_DLGMODALFRAME"));
		m_dwExStyle &= (~WS_EX_DLGMODALFRAME);
	}
	if((m_dwExStyle & WS_EX_NOPARENTNOTIFY)== WS_EX_NOPARENTNOTIFY){
		pLBx->AddString(_T("WS_EX_NOPARENTNOTIFY"));
		m_dwExStyle &= (~WS_EX_NOPARENTNOTIFY);
	}
	if((m_dwExStyle & WS_EX_ACCEPTFILES)== WS_EX_ACCEPTFILES){
		pLBx->AddString(_T("WS_EX_ACCEPTFILES"));
		m_dwExStyle &= (~WS_EX_ACCEPTFILES);
	}
	if((m_dwExStyle & WS_EX_TRANSPARENT)== WS_EX_TRANSPARENT){
		pLBx->AddString(_T("WS_EX_TRANSPARENT"));
		m_dwExStyle &= (~WS_EX_TRANSPARENT);
	}
	if((m_dwExStyle & WS_EX_MDICHILD)== WS_EX_MDICHILD){
		pLBx->AddString(_T("WS_EX_MDICHILD"));
		m_dwExStyle &= (~WS_EX_MDICHILD);
	}
	if((m_dwExStyle & WS_EX_CONTEXTHELP)== WS_EX_CONTEXTHELP){
		pLBx->AddString(_T("WS_EX_CONTEXTHELP"));
		m_dwExStyle &= (~WS_EX_CONTEXTHELP);
	}
	if((m_dwExStyle & WS_EX_RIGHT)== WS_EX_RIGHT){
		pLBx->AddString(_T("WS_EX_RIGHT"));
		m_dwExStyle &= (~WS_EX_RIGHT);
	}
	if((m_dwExStyle & WS_EX_RTLREADING)== WS_EX_RTLREADING){
		pLBx->AddString(_T("WS_EX_RTLREADING"));
		m_dwExStyle &= (~WS_EX_RTLREADING);
	}
	if((m_dwExStyle & WS_EX_LEFTSCROLLBAR)== WS_EX_LEFTSCROLLBAR){
		pLBx->AddString(_T("WS_EX_LEFTSCROLLBAR"));
		m_dwExStyle &= (~WS_EX_LEFTSCROLLBAR);
	}
	if((m_dwExStyle & WS_EX_CONTROLPARENT)== WS_EX_CONTROLPARENT){
		pLBx->AddString(_T("WS_EX_CONTROLPARENT"));
		m_dwExStyle &= (~WS_EX_CONTROLPARENT);
	}
	if((m_dwExStyle & WS_EX_STATICEDGE)== WS_EX_STATICEDGE){
		pLBx->AddString(_T("WS_EX_STATICEDGE"));
		m_dwExStyle &= (~WS_EX_STATICEDGE);
	}
	if((m_dwExStyle & WS_EX_APPWINDOW)== WS_EX_APPWINDOW){
		pLBx->AddString(_T("WS_EX_APPWINDOW"));
		m_dwExStyle &= (~WS_EX_APPWINDOW);
	}

	// Leftovers
	if(m_dwExStyle != 0){
		CString strExtra;
		strExtra.Format(_T("0x%0.8X"),m_dwExStyle);
		pLBx->AddString(strExtra);
	}
	if(m_dwStyle != 0){
		CString strExtra;
		strExtra.Format(_T("0x%0.8X"),m_dwStyle);
		pLB->AddString(strExtra);
	}
}



