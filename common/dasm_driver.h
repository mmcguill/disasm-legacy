/////////////////////////////////////////////////////////////////////

#define WM_DASM_ENGINE_UPDATE	(WM_USER + 0xDA5)

/////////////////////////////////////////////////////////////////////

typedef struct tagIMAGE_INFO
{
	BYTE* pucImage;
	ULONG ulSize;
	ULONG ulImageBaseAddress;
}IMAGE_INFO;

typedef IMAGE_INFO* PIMAGE_INFO;
typedef const PIMAGE_INFO PCIMAGE_INFO;

/////////////////////////////////////////////////////////////////////

class CIA32Processor
{
public:
	CIA32Processor(void) {Reset();}

	virtual ~CIA32Processor(void) {Reset();}

	BOOL SetImage(PIMAGE_INFO pImageInfo)
	{
		m_pImageInfo = pImageInfo;
		return TRUE;
	}

	BOOL Reset(void) {m_pImageInfo = NULL; m_ulIP = 0; return TRUE;}

	BOOL SetIP(ULONG ulVA) {m_ulIP = ulVA; return TRUE;}

	ULONG GetIP(void)	{return m_ulIP;}

	PBYTE GetCurrentByte() 
	{
		Assert(m_pImageInfo);
		return m_pImageInfo->pucImage + 
			(m_ulIP - m_pImageInfo->ulImageBaseAddress);
	}

	// A Clock Tick = Reads instruction at current IP returns this
	// as an Instruction and updates IP. TRUE if it could step 
	// or FALSE if not

	BOOL Tick(BOOL fUse32, PINSTRUCTION* ppInst);

protected:
	PIMAGE_INFO m_pImageInfo;
	ULONG m_ulIP;
};

/////////////////////////////////////////////////////////////////////

// Gather as many code listings as we can each time go is called 
// and return them back

class CIA32ProcessorDriver
{
public:
	CIA32ProcessorDriver()
	{
		Reset();
	}

	~CIA32ProcessorDriver()
	{
		Reset();
	}

	BOOL SetImage(PIMAGE_INFO pImageInfo)
	{
		Assert(pImageInfo);
		m_pImageInfo = pImageInfo;
		m_proc.SetImage(m_pImageInfo);
		return TRUE;
	}

	BOOL Reset()
	{
		m_pImageInfo = NULL;
		m_proc.Reset();
		return TRUE;
	}


	BOOL GoSimple(	BOOL fUse32, 
					const ULONG ulStart,
					PLISTINGS_MANAGER pManager);


	BOOL GoIntelligent( const ULONG ulStart,
						BOOL fUse32,
						PLISTINGS_MANAGER pManager,
						HWND hWndUpdate);

	BOOL CIA32ProcessorDriver::GoIntelligent(
						std::vector<ULONG>& vecDasm,
						BOOL fUse32,
						PLISTINGS_MANAGER pManager,
						HWND hWndUpdate);

	static BOOL IsEndBlockInstruction(MnemonicEnum mne);
	static BOOL IsJumpInstruction(MnemonicEnum mne);

protected:
	BOOL DisassembleBranches(std::vector<ULONG> *pvecDasm,
							BOOL fUse32,
							PLISTINGS_MANAGER pManager,
							HWND hWndUpdate);
	
protected:
	CIA32Processor m_proc;
	PIMAGE_INFO m_pImageInfo;
private:
};

/////////////////////////////////////////////////////////////////////

class CIA32Dasm
{
public:
	CIA32Dasm(void)
	{
		m_pListsMgr = NULL;
		m_pSymTab = NULL;
		m_fIntelligentDasm = FALSE;
		m_fUse32 = FALSE;
		Reset();
	}

	virtual ~CIA32Dasm(void)
	{
		Reset();
	}

	BOOL Reset()
	{
		SAFE_DELETE(m_pListsMgr);
		SAFE_DELETE(m_pSymTab);

		m_procdriver.Reset();
		return TRUE;
	}


	PLISTINGS_MANAGER GetListingsManager() const
	{
		return m_pListsMgr;
	}

	PSYMBOL_TABLE GetSymbolTable(void)
	{
		// Create on demand...

		if(NULL == m_pSymTab)
		{
			m_pSymTab = new SYMBOL_TABLE();
			Assert(m_pSymTab);
			if(NULL == m_pSymTab)
			{
				return NULL;
			}
		}

		return m_pSymTab;
	}

	BOOL Load ( BOOL fUse32, const PBYTE pucImage,
				const ULONG ulSize,
				const ULONG ulImageBaseAddress,
				BOOL fIntelligentDasm);

	BOOL DasmFromEPs();
	BOOL DasmFromEPsWithWndUpdate(HWND hWndUpdate);

	BOOL Dasm(const ULONG ulStart);

protected:
	PLISTINGS_MANAGER	m_pListsMgr;
	PSYMBOL_TABLE		m_pSymTab;

	CIA32ProcessorDriver m_procdriver;
	BOOL m_fIntelligentDasm;
	BOOL m_fUse32;
	IMAGE_INFO m_ii;
};






/////////////////////////////////////////////////////////////////////
