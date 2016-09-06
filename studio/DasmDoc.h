///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

class CStudioWorkspaceDasmItemInfo : public CStudioWorkspaceItemInfo
{
public:

	CStudioWorkspaceDasmItemInfo()
	{
		m_pstrFriendlySymbolName = NULL;
		m_fValid = FALSE;
	}

	~CStudioWorkspaceDasmItemInfo()
	{
		if(m_pstrFriendlySymbolName)
		{
			delete m_pstrFriendlySymbolName;
		}
	}

	CStudioWorkspaceDasmItemInfo(CStudioWorkspaceDasmItemInfo* pCopy)
	{
		ASSERT(pCopy);
		m_sdtDefault.major = pCopy->GetDefaultDocType()->major;
		m_sdtDefault.minor = pCopy->GetDefaultDocType()->minor;
		m_nUniqueID = pCopy->GetUniqueItemID();
		m_fValid = pCopy->IsValid();		
		m_pWSDoc = pCopy->GetWorkspaceDoc();


		m_dwStartAddress = pCopy->GetStartAddress();
		m_dwEndAddress = pCopy->GetEndAddress();
		
		if(pCopy->GetFriendlySymbolName())
		{
			m_pstrFriendlySymbolName = new CString(*pCopy->GetFriendlySymbolName());
			ASSERT(m_pstrFriendlySymbolName);
		}
		else
		{
			m_pstrFriendlySymbolName = NULL;
		}
	}

	CStudioWorkspaceDasmItemInfo(CStudioDoc* pWSDoc,
		StudioMajorDocType major,StudioMinorDocType minor,
		UINT nUniqueID,	DWORD dwStartAddress,DWORD dwEndAddress,
		LPCTSTR lpszFriendlySymbolName) : 
	CStudioWorkspaceItemInfo(pWSDoc, major,minor,nUniqueID)
	{
		m_dwStartAddress = dwStartAddress;
		m_dwEndAddress = dwEndAddress;
		
		if(lpszFriendlySymbolName)
		{
			m_pstrFriendlySymbolName = new CString(lpszFriendlySymbolName);
			ASSERT(m_pstrFriendlySymbolName);
		}
		else
		{
			m_pstrFriendlySymbolName = NULL;
		}
		m_fValid = TRUE;
	}

	CStudioWorkspaceDasmItemInfo(CStudioDoc* pWSDoc,
		StudioMajorDocType major,
		StudioMinorDocType minor,
		UINT nUniqueID,
		DWORD dwStartAddress,
		DWORD dwEndAddress,
		UINT idString) : CStudioWorkspaceItemInfo(pWSDoc,major,minor,nUniqueID)
	{
		m_dwStartAddress = dwStartAddress;
		m_dwEndAddress = dwEndAddress;
		
		ASSERT(idString);
		
		m_pstrFriendlySymbolName = new CString;
		ASSERT(m_pstrFriendlySymbolName);

		m_pstrFriendlySymbolName->LoadString(idString);

		m_fValid = TRUE;
	}

	DWORD GetStartAddress()				{ASSERT(m_fValid);return m_dwStartAddress;}
	DWORD GetEndAddress()				{ASSERT(m_fValid);return m_dwEndAddress;}
	CString* GetFriendlySymbolName()	{ASSERT(m_fValid);return m_pstrFriendlySymbolName;}

private:
    DWORD m_dwStartAddress;
	DWORD m_dwEndAddress;
	CString* m_pstrFriendlySymbolName;
};
											
///////////////////////////////////////////////////////////////////////////////

typedef struct _DASM_THREAD_PARAM
{
	HDISASM hDasm;
	HWND hWndUpdate;
}DASM_THREAD_PARAM, *PDASM_THREAD_PARAM;

/////////////////////////////////////////////////////////////////////////////

class CDasmBaseDoc : public CWorkspaceItemBaseDoc
{
public:
	CDasmBaseDoc(void);
	DECLARE_DYNCREATE(CDasmBaseDoc)

public:
	virtual BOOL SetWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo);
	virtual BOOL UpdateWorkspaceItemInfo(CStudioWorkspaceItemInfo* pItemInfo);

	virtual CStudioWorkspaceItemInfo* GetWorkspaceItemInfo() const;
	virtual ~CDasmBaseDoc(void);

	PLISTINGS_MANAGER GetListingsManager();
	BOOL DisassembleAddress(DWORD dwAddress);

protected:
	virtual BOOL InitDoc();
	virtual BOOL SetNiceTitle();
	
protected:
	void Cleanup();

protected:
	DECLARE_MESSAGE_MAP()

	CStudioWorkspaceDasmItemInfo* m_pItemInfo;
	DASM_THREAD_PARAM m_dtp;
private:

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////