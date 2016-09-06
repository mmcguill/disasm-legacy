#pragma once

#include "DasmView.h"

// CDasmBaseFrame frame

class CDasmBaseFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CDasmBaseFrame)
protected:
	CDasmBaseFrame();           // protected constructor used by dynamic creation
	virtual ~CDasmBaseFrame();

protected:
	DECLARE_MESSAGE_MAP()
public:
	void Push(LPCTSTR lpszAddress);
	void Pop();
protected:
	CGlobalViewBar m_wndGlobalView;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


