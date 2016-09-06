/////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <functional>
#include <set>
#include <map>
#include <vector>
#include "..\..\inc\types.h"
#include "..\..\inc\util.h"
#include "..\..\inc\debug.h"
#include "..\..\inc\ia32dsm.h"
#include "..\..\inc\symbols.h"

#include "..\common\lists.h"

#include "dasm_driver.h"

/////////////////////////////////////////////////////////////////////

BOOL CIA32Dasm::Load(BOOL fUse32, const PBYTE pucImage,
					 const ULONG ulSize,
					 const ULONG ulImageBaseAddress,
					 BOOL fIntelligentDasm)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);

    Assert(pucImage && ulSize);

	Reset();
	
	// Here we organise all the listings we can get back from the processor driver
	// and arrange high level stuff. we will also soon!! be able to access all symbols
	// because an ILoader interface will be passed in here.

	// Also need to look at UI interaction for status of the disassembly job here.

	m_ii.pucImage = (PBYTE)pucImage;
	m_ii.ulImageBaseAddress = ulImageBaseAddress;
	m_ii.ulSize = ulSize;

	m_procdriver.SetImage(&m_ii);

	
	// Create Our Listings Manager

	if(NULL == m_pListsMgr)
	{
		m_pListsMgr = new LISTINGS_MANAGER;
		Assert(m_pListsMgr);

		if(NULL == m_pListsMgr)
		{
			return FALSE;
		}
	}

	m_fIntelligentDasm = fIntelligentDasm;
	m_fUse32 = fUse32;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// TODO: Not Reentrant ->  Can only be called once I think, not designed
// to be called more than once.
/////////////////////////////////////////////////////////////////////

BOOL CIA32Dasm::DasmFromEPs()
{
	return DasmFromEPsWithWndUpdate(NULL);
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32Dasm::DasmFromEPsWithWndUpdate(HWND hWndUpdate)
{
	Assert(m_pSymTab);	// Need our symbols -> Should have been created
						// by a call to GetSymbolTable() for
						// earlier File Parser Load

	Assert(m_pListsMgr); // Should be available otherwise 
							 // Load() hasn't been called
	Assert(m_fIntelligentDasm); // Only available on intelligent dasms

	if (NULL == m_pSymTab || NULL == m_pListsMgr || 
		NULL == m_fIntelligentDasm)
	{
		return FALSE;
	}
	
	// Setup set of addresses to disassemble
	
	std::vector<ULONG> vecDasm;
	
	PSYMBOLS_MAP pSymMap = m_pSymTab->GetSymbolsMap();
	Assert(pSymMap);
	if(NULL == pSymMap)
	{
		return TRUE;
	}

	for(SYMBOLS_MAP::iterator sym = pSymMap->begin();
		sym!=pSymMap->end();sym++)
	{
		ULONG dwEntry = sym->first;
		PSYMBOL pSym = sym->second;

		if (0 == dwEntry)
		{
			Assert(FALSE);
            continue;
		}
		

		if (symbol_import == pSym->GetType() ||
			symbol_indirect == pSym->GetType())
		{
            continue;
		}

		// Add Entry Point

		vecDasm.push_back(pSym->GetAddress());
	}

	// Start the disassembly...

	if(!m_procdriver.GoIntelligent(vecDasm,m_fUse32,m_pListsMgr,hWndUpdate))
	{
		return FALSE;
	}

	
	// Consolidate Code Lists 
	
	if(!m_pListsMgr->ConsolidateCodeListings())
	{
		return FALSE;
	}


	// 1a. Scan for Simple Thunks, ie. calls to jumps and add
	//     symbols like j_xxx

	PLISTINGS pll = m_pListsMgr->AcquireListings();
	
	Assert(pll);
	if(NULL == pll)
	{
		m_pListsMgr->ReleaseListings();
		return FALSE;
	}
	
	for(LISTINGS::iterator lst = pll->begin();
		lst != pll->end();lst++)
	{
		if(ListingCode != (*lst)->GetListingType())
		{
			continue;
		}

		PCODE_LISTING pcl = (PCODE_LISTING)(*lst);
		PINSTRUCTION_MAP pim = pcl->GetInstructionMap();

		if( NULL == pim || 0 == pim->size())
		{
			continue;
		}
			
		for(std::map<ULONG, PINSTRUCTION>::iterator theIns = pim->begin();
			theIns != pim->end();theIns++)
		{
			std::pair<ULONG, PINSTRUCTION> ins = *theIns;
			BOOL fSymbolFound = FALSE;
			PINSTRUCTION pInst = ins.second;

			if(MneCall != pInst->m_Mnemonic)
			{
				continue;
			}
			
			PINSTRUCTION pDest = 
				m_pListsMgr->GetInstructionAtAddress(pInst->m_ulDirectJumpLoc);

			if(pDest == NULL)
			{
				continue;
			}

			if(MneJmp != pDest->m_Mnemonic)
			{
				continue;
			}

			// Direct or Indirect?

			if(pDest->m_ulDirectJumpLoc)
			{
				// Nothing With it for moment!
			}
			else
			{
				Assert(pDest->m_rgszOperand[0]);

				LPSTR lpszIndirect = NULL;
				if (NULL != 
					(lpszIndirect = strchr(pDest->m_rgszOperand[0],'[')))
				{
					// See if we can find a simple address
					// nothing complicated at this stage or 
					// on this parse

					ULONG ulPtr = strtoul(lpszIndirect+1,NULL,16);
					SYMBOLS_MAP::iterator symbol = 
						m_pSymTab->GetSymbolsMap()->find(ulPtr);

					if (symbol != m_pSymTab->GetSymbolsMap()->end())
					{
						// Create A New Symbol Based on this one!

						char szNew[MAX_CCH_SYMBOL_NAME];

						_snprintf(szNew,MAX_CCH_SYMBOL_NAME - 1,"j_%s",symbol->second->GetName());
						szNew[MAX_CCH_SYMBOL_NAME - 1] = 0;
						PSYMBOL pNew = new CSymbol(pDest->m_ulAddress,symbol_std,szNew);

						if(!m_pSymTab->InsertSymbol(pNew))
						{
							// This is prob a duplicate...
							delete pNew;
						}
					}
				}
			}
		}
	}
	
	
	// Stage2:  create listings of data which was not found as
	//			code, so we can see full view

	ULONG base = m_ii.ulImageBaseAddress;
	ULONG end = m_ii.ulImageBaseAddress + m_ii.ulSize - 1;

	//printf("Size: %d\n",pll->size());

	for(LISTINGS::iterator lst = pll->begin();
		lst != pll->end();lst++)
	{
		PLISTING pl = (*lst);
		ULONG first = pl->GetStartAddress();
		ULONG last = pl->GetEndAddress();

		if(first < base)
		{
			AssertSz(0,"Something funny going on here...");
			m_pListsMgr->ReleaseListings();
			return FALSE;
		}

		
		// See if we need to add a data listing

		if(first > base)
		{
			// new data list here
			
			PDATA_LISTING pData = new DATA_LISTING(base,first-1);

			// add

			m_pListsMgr->AddListing(pData);

			// Because we have added one we need to bump up the iterator

			lst++;
		}

		base = last + 1;

		// Debug

		/*char szDbg[256];
		sprintf(szDbg,"%0.8X -> %0.8X\n",first,last);
		printf(szDbg);*/
	}

	
	// Could be one last list from end of last code listing 
	// to image end... fill that in now...

	if(end > base)
	{
		// new data list here
		
		PDATA_LISTING pData = new DATA_LISTING(base, end);

		// add

		m_pListsMgr->AddListing(pData);
	}


	// TODO: Second Phase Parsing & Analysis Here
	// TODO: Following register cache'd calls etc
	
	m_pListsMgr->ReleaseListings();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32Dasm::Dasm(const ULONG ulStart)
{
	Assert(m_pListsMgr);
	if(NULL == m_pListsMgr)
	{
		return FALSE;
	}

	// Stage 1: Simple disassembly, definite branch following

	if(m_fIntelligentDasm)
	{
		if(!m_procdriver.GoIntelligent(m_fUse32, ulStart, m_pListsMgr,NULL))
		{
			return FALSE;
		}
	}
	else
	{
		if(!m_procdriver.GoSimple(m_fUse32, ulStart, m_pListsMgr))
		{
			return FALSE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32ProcessorDriver::GoIntelligent(
						std::vector<ULONG>& vecDasm,
						BOOL fUse32,
						PLISTINGS_MANAGER pManager,
						HWND hWndUpdate)
{
	Assert(m_pImageInfo);

	BOOL fRet = DisassembleBranches(&vecDasm,fUse32, pManager, hWndUpdate);

	//printf("Finished Disassembly.\n\n",ulStart);
	//printf("Branches Taken: %d\n",snBranchesTaken);
	
	return fRet;
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32ProcessorDriver::GoIntelligent(const ULONG ulStart,
										 BOOL fUse32,
										 PLISTINGS_MANAGER pManager,
										 HWND hWndUpdate)
{
	Assert(m_pImageInfo);

	std::vector<ULONG> vecDasm;
	vecDasm.push_back(ulStart);

	//printf("Starting Disassembly From: %X\n",ulStart);

	BOOL fRet = DisassembleBranches(&vecDasm,fUse32, pManager, hWndUpdate);

	//printf("Finished Disassembly.\n\n",ulStart);
	//printf("Branches Taken: %d\n",snBranchesTaken);
	
	return fRet;
}

/////////////////////////////////////////////////////////////////////

//class vs 
//{
//public:
//	ULONG addr;
//	bool visited;
//};
//
//typedef vs *pvs;
//
//template<class _Arg1,
//	class _Arg2,
//	class _Result>
//	struct binary_function
//	{	// base class for binary functions
//	typedef _Arg1 first_argument_type;
//	typedef _Arg2 second_argument_type;
//	typedef _Result result_type;
//	};
//
//template<class _Ty>
//	struct my_less
//		: public binary_function<_Ty, _Ty, bool>
//	{	// functor for operator<
//	bool operator()(const _Ty& _Left, const _Ty& _Right) const
//		{	// apply operator< to operands
//		return (_Left->addr < _Right->addr);
//		}
//	};

BOOL CIA32ProcessorDriver::DisassembleBranches(	std::vector<ULONG> *pvecDasm,
												BOOL fUse32,
												PLISTINGS_MANAGER pManager,
												HWND hWndUpdate)
{
	Assert(pvecDasm);
	Assert(pManager);

	std::vector<ULONG> vecDasm;
	std::set<ULONG> setBranches;
	//std::set<pvs, my_less<pvs> > setVisited;
	int nBranchDone = 0;

	// Copy 

	for(std::vector<ULONG>::iterator entry = pvecDasm->begin();
		entry != pvecDasm->end();entry++)
	{
		vecDasm.push_back(*entry);

		//pvs a = new vs;
		//a->addr = (*entry);
		//a->visited = false;

		//setVisited.insert(a);
	}


	// Start Large scale BFS Disassembly

	while(vecDasm.size())
	{
		if(hWndUpdate)
		{
			//PostMessage(hWndUpdate,WM_DASM_ENGINE_UPDATE,0,0);
		}


		// Breadth first search...

		for(std::vector<ULONG>::iterator entry = vecDasm.begin();
			entry != vecDasm.end();entry++)
		{
			ULONG ulAdd = (*entry);
			
			if(pManager->IsAddressDisassembled(ulAdd))
			{
				//printf("[%0.8X]\n",ulAdd);
				continue;
			}


			//printf("Starting: %0.8X\n",ulAdd);

			PCODE_LISTING pCodeListing = new CODE_LISTING;
			Assert(pCodeListing);

			if(NULL == pCodeListing)
			{
				// TODO: Set out of memory here
				return FALSE;
			}

			m_proc.SetIP(ulAdd);
			PINSTRUCTION pInst = NULL;

			while(m_proc.Tick(fUse32, &pInst))
			{
				pCodeListing->AddInstruction(pInst);

				
				// Build up branches

				// We are excluding certain types of jumps
				// so get rid of them here (the check for 0xFF)
				// TODO: Find out how to deal with them. 
					
				if (IsJumpInstruction(pInst->m_Mnemonic)/* &&
					pInst->m_ucInstruction[0] != 0xFF*/)
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
							
							if(	ulPtr >= m_pImageInfo->ulImageBaseAddress &&
								ulPtr <= 
								((	m_pImageInfo->ulImageBaseAddress + 
								m_pImageInfo->ulSize)-4))
							{
								ulJumpEA = 
									*(ULONG*)&(m_pImageInfo->
									pucImage[ulPtr - m_pImageInfo->ulImageBaseAddress]);
							}
						}
					}

					// Sanity Check Branch here, should be within imageBase
					// and imageBase + imageSize

					if(	ulJumpEA >= m_pImageInfo->ulImageBaseAddress &&
						ulJumpEA <= 
						((	m_pImageInfo->ulImageBaseAddress + 
							m_pImageInfo->ulSize)-1))
					{
						// Since this is a set if we try to insert a duplicate,
						// insert will fail, however this is the behaviour we
						// want and so no check for it here...

						if(!pManager->IsAddressDisassembled(ulJumpEA))
						{
							//pvs z = new vs;
							//z->addr = ulJumpEA;
							//z->visited = false;

							//std::pair<std::set<pvs,  my_less<pvs> >::iterator, bool> ret = 
							//setVisited.insert(z);

							//if(ret.second)
							//{
							setBranches.insert(ulJumpEA);
							//}
						}
					}
				}

				
				// End of Block or code we've already disassembled... bail

				if (IsEndBlockInstruction(pInst->m_Mnemonic) || 
					pManager->IsAddressDisassembled(m_proc.GetIP()))
				{
					break;
				}
			}

			
			// Listing done add to manager...

			pManager->AddListing(pCodeListing);

			
			
			// Progress - Not Working
		
			//printf("******************************************\n");

			//printf("Code Listing [%0.8X] -> [%0.8X]\n",
			//	pCodeListing->GetStartAddress(),
			//	pCodeListing->GetEndAddress());

			//for(std::set<pvs, my_less<pvs> >::iterator b = setVisited.begin();
			//	b != setVisited.end();b++)
			//{
			//	pvs z = (*b);
			////	printf("Checking Address %0.8X...",z->addr);

			//	if(AddressIsInstruction == pCodeListing->IsAddressWithinListing(z->addr))
			//	{
			////		printf("Y\n");
			//		z->visited = true;
			//		nBranchDone++;
			//	}
			//	else
			//	{
			////		printf("N\n");
			//	}
			//}
			

			//printf("******************************************\n");
			//printf("Progress: %d / %d\n", nBranchDone, setVisited.size());
			//printf("******************************************\n\n");
		}
	
		vecDasm.clear();
		
		for(std::set<ULONG>::iterator entry = setBranches.begin();
			entry != setBranches.end();entry++)
		{
			vecDasm.push_back(*entry);
		}

		setBranches.clear();
	}


	//printf("******************************************\n");

	//for(std::set<pvs, my_less<pvs> >::iterator b = setVisited.begin();
	//	b != setVisited.end();b++)
	//{
	//	printf("UB: %0.8X: %s\n",(*b)->addr,(*b)->visited ? "Y" : "N");
	//}

	//printf("******************************************\n");

	//printf("Unique Branches Visited: %d\n", setVisited.size());

	//printf("******************************************\n");

	return TRUE;
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32ProcessorDriver::IsEndBlockInstruction(MnemonicEnum mne)
{
	if(MneJmp == mne || MneRet == mne)
	{
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32ProcessorDriver::IsJumpInstruction(MnemonicEnum mne)
{
	// TODO: Performance here. Since all conditional jumps
	// have 7 in their high nibble i think, can we use this?

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

/////////////////////////////////////////////////////////////////////

BOOL CIA32ProcessorDriver::GoSimple(BOOL fUse32, const ULONG ulStart,
									PLISTINGS_MANAGER pManager)
{
	Assert(m_pImageInfo);

	m_proc.SetIP(ulStart);

	PINSTRUCTION pInst = NULL;
	
	PCODE_LISTING pCodeListing = new CODE_LISTING;
	Assert(pCodeListing);

	if(NULL == pCodeListing)
	{
		// TODO: Set out of memory here
		return FALSE;
	}

	while(1)
	{
		if (!m_proc.Tick(fUse32, &pInst))
		{
			if(IA32DSM_ERROR_COULD_NOT_DECODE == IA32Dsm_GetLastError())
			{
				pInst = new INSTRUCTION;
				Assert(pInst);
				if(NULL == pInst)
				{
					delete pCodeListing;
					return FALSE;
				}

				// skip this byte and start again + 1
				// db 0x?? ->

				PBYTE tmp = m_proc.GetCurrentByte();
				Assert(tmp);
				if(NULL == tmp)
				{
					delete pInst;
					delete pCodeListing;
					return FALSE;
				}

				BYTE skip = *tmp;
				char szTmp[MAX_SZ_MNEMONIC];

				if(skip > 0x9F)
				{
					sprintf(szTmp,"db 0%X%c",skip,(skip > 9) ? 'h' : 0);
				}
				else
				{
					sprintf(szTmp,"db %X%c",skip,(skip > 9) ? 'h' : 0);
				}

				pInst->m_ucInstruction[0] = skip;
				pInst->m_cbInstruction = 1;
				pInst->m_cOperands = 0;
				pInst->m_Mnemonic = MneNone;
				strncpy(pInst->m_szMnemonic,szTmp,MAX_SZ_MNEMONIC);
				pInst->m_ulAddress = m_proc.GetIP();

				m_proc.SetIP(m_proc.GetIP() + pInst->m_cbInstruction);
			}
			else
			{
				break;
			}
		}

		pCodeListing->AddInstruction(pInst);
	}
		
	pManager->AddListing(pCodeListing);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////

BOOL CIA32Processor::Tick(BOOL fUse32, PINSTRUCTION* ppInstRet)
{
	Assert(m_pImageInfo);

	if((m_ulIP - m_pImageInfo->ulImageBaseAddress) >= m_pImageInfo->ulSize)
	{
		return FALSE;
	}

	PBYTE pucOffset = m_pImageInfo->pucImage + (m_ulIP - m_pImageInfo->ulImageBaseAddress);
	ULONG ulLength = m_pImageInfo->ulSize - (m_ulIP - m_pImageInfo->ulImageBaseAddress);

	PINSTRUCTION pInst = new INSTRUCTION;
	Assert(pInst);
	if(NULL == pInst)
	{
		*ppInstRet = NULL;
		return FALSE;
	}

	if(DisassembleSingleInstruction(pucOffset,ulLength,m_ulIP, fUse32, pInst))
	{
		m_ulIP += pInst->m_cbInstruction;
		*ppInstRet = pInst;		
		return TRUE;
	}

	delete pInst;
	pInst = NULL;
	return FALSE;
}

