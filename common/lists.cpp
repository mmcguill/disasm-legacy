/////////////////////////////////////////////////////////////////////

#include "..\..\inc\util.h"
#include "..\..\inc\debug.h"
#include "..\..\inc\types.h"
#include "..\..\inc\ia32dsm.h"
#include <map>
#include <vector>
#include "lists.h"

/////////////////////////////////////////////////////////////////////

CCodeListing::CCodeListing(void) : CListing(ListingCode)
{
	m_pmapInstructions = NULL;
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
}

/////////////////////////////////////////////////////////////////////

CCodeListing::~CCodeListing(void)
{
	if(NULL != m_pmapInstructions)
	{
		for(INSTRUCTION_MAP::iterator theIns = 
			m_pmapInstructions->begin();
			theIns != m_pmapInstructions->end();theIns++)
		{
			SAFE_DELETE(theIns->second);
		}
		SAFE_DELETE(m_pmapInstructions);
	}
}

/////////////////////////////////////////////////////////////////////

BOOL CCodeListing::AddInstruction(PINSTRUCTION pInst)
{
	Assert(pInst);

	if(NULL == m_pmapInstructions)
	{
		m_pmapInstructions = new INSTRUCTION_MAP;
		Assert(m_pmapInstructions);

		if(NULL == m_pmapInstructions)
		{
			// TODO: Set out of memory
			return FALSE;
		}
	}

	std::pair<INSTRUCTION_MAP::iterator, BOOL> prRes = 
		m_pmapInstructions->insert(
			INSTRUCTION_MAP::value_type(pInst->m_ulAddress,pInst));

	Assert(prRes.second);

	return (prRes.second);
}

/////////////////////////////////////////////////////////////////////

AddressContainedType CCodeListing::IsAddressWithinListing(ULONG ulAddress)
{
//	Assert(m_pmapInstructions);

	if (NULL == m_pmapInstructions ||
		0 == m_pmapInstructions->size())
	{
		return AddressNotContained;
	}

	INSTRUCTION_MAP::const_iterator itEnd = m_pmapInstructions->end();
	INSTRUCTION_MAP::const_iterator itBegin = m_pmapInstructions->begin();
    itEnd--;
	
	ULONG ulNextAfterLast = itEnd->first + itEnd->second->m_cbInstruction;
	ULONG ulFirst = itBegin->first;

	if (ulAddress < ulFirst ||
		ulAddress >= ulNextAfterLast)
	{
		return AddressNotContained;
	}


	INSTRUCTION_MAP::const_iterator theIns = 
		m_pmapInstructions->find(ulAddress);

	if(theIns != m_pmapInstructions->end())
	{
		return AddressIsInstruction;
	}
	else 
	{
		AssertSz(0,"Integrity Problem here.");
		return AddressContainedButNotIntegrally;
	}
}

/////////////////////////////////////////////////////////////////////

ULONG CCodeListing::GetFirst()
{
//	Assert(m_pmapInstructions);

	if (NULL == m_pmapInstructions ||
		0 == m_pmapInstructions->size())
	{
		return 0;
	}

	INSTRUCTION_MAP::const_iterator itBegin = 
		m_pmapInstructions->begin();

	return itBegin->first;
}

/////////////////////////////////////////////////////////////////////

ULONG CCodeListing::GetNextInsLocAfterLast()
{
//	Assert(m_pmapInstructions);

	if (NULL == m_pmapInstructions ||
		0 == m_pmapInstructions->size())
	{
		return 0;
	}

	INSTRUCTION_MAP::const_iterator itEnd =
		m_pmapInstructions->end();
    itEnd--;
	
	return itEnd->first + itEnd->second->m_cbInstruction;
}

/////////////////////////////////////////////////////////////////////

ULONG CCodeListing::GetStartAddress()
{
	return GetFirst();
}

/////////////////////////////////////////////////////////////////////

ULONG CCodeListing::GetEndAddress()
{
	return GetNextInsLocAfterLast() - 1;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CListingsManager::CListingsManager(void) 
{ 
	m_pListings = NULL;
	InitializeCriticalSection(&m_cs);
}

/////////////////////////////////////////////////////////////////////

CListingsManager::~CListingsManager(void)
{
	if(!TryEnterCriticalSection(&m_cs))
	{
		AssertSz(0,"Could not enter critical section in destructor");
	}

	if(NULL != m_pListings)
	{
		for(LISTINGS::iterator theListing = 
			m_pListings->begin();
			theListing != m_pListings->end();theListing++)
		{
			Assert(*theListing);
			delete (*theListing);
		}

		delete m_pListings;
	}
	
	LeaveCriticalSection(&m_cs);
	DeleteCriticalSection(&m_cs);
}

/////////////////////////////////////////////////////////////////////

PLISTINGS CListingsManager::AcquireListings()
{
	//printf("Trying Critical Section...\n");
	//OutputDebugString("Trying Critical Section...\n");

	EnterCriticalSection(&m_cs);
	
	//printf("Entered Critical Section...\n");
	//OutputDebugString("Entered Critical Section...\n");

    return m_pListings;
}

/////////////////////////////////////////////////////////////////////

void CListingsManager::ReleaseListings()
{
	//printf("Leaving Critical Section\n");
	//OutputDebugString("Leaving Critical Section\n");

	LeaveCriticalSection(&m_cs);
}

/////////////////////////////////////////////////////////////////////

BOOL CListingsManager::AddListing(PLISTING pList)
{
	Assert(pList);
	if(NULL == pList)
	{
		return FALSE;
	}

	// Acquire Lock

	EnterCriticalSection(&m_cs);
	
	
	// Create if necessary 
	
	if(NULL == m_pListings)
	{
		m_pListings = new LISTINGS;
		Assert(m_pListings);

		if(NULL == m_pListings)
		{
			LeaveCriticalSection(&m_cs);
			// TODO: Out of Mem
			return FALSE;
		}
	}

	
	// Scan our listings and find best place for this list
	// order it...

	for(LISTINGS::iterator listCmp = m_pListings->begin();
		listCmp!=m_pListings->end();listCmp++)
	{
		CListing* pCL = *listCmp;
		ULONG ulCmpFirst = pCL->GetStartAddress();
		if(ulCmpFirst > pList->GetStartAddress())
		{
			break;
		}
	}

	m_pListings->insert(listCmp,pList);
	
	
	// Release Lock

	LeaveCriticalSection(&m_cs);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////

PINSTRUCTION CListingsManager::GetInstructionAtAddress(ULONG ulAddress)
{
	PLISTINGS pListings = AcquireListings();

	if(NULL == pListings)
	{
		ReleaseListings();
		return NULL;
	}

	for(LISTINGS::iterator theListing = pListings->begin();
		theListing != pListings->end();theListing++)
	{
		if(ListingData == (*theListing)->GetListingType())
		{
			continue;
		}

		// be safe 
		Assert(ListingCode == (*theListing)->GetListingType());
		if(ListingCode != (*theListing)->GetListingType())
		{
			ReleaseListings();
			return NULL;
		}


		PCODE_LISTING pcl = static_cast<PCODE_LISTING>(*theListing);

		AddressContainedType ret = 
			pcl->IsAddressWithinListing(ulAddress);

		if(AddressNotContained == ret)
		{
			continue;
		}
		else if(AddressIsInstruction == ret)
		{
			PINSTRUCTION_MAP pim = pcl->GetInstructionMap();
			INSTRUCTION_MAP::const_iterator ins = pim->find(ulAddress);
			if(ins == pim->end())
			{
				continue;
			}

			PINSTRUCTION pins = ins->second;
			ReleaseListings();
			return pins;
		}
		else
		{
			AssertSz(0,"Address is contained within listing but not "
						"on an instruction boundary...");
			ReleaseListings();
			return  NULL;
		}
	}

	ReleaseListings();
	return NULL;
}

/////////////////////////////////////////////////////////////////////

BOOL CListingsManager::IsAddressDisassembled(ULONG ulAddress)
{
	PLISTINGS pListings = AcquireListings();

	if(NULL == pListings)
	{
		ReleaseListings();
		return FALSE;
	}

	for(LISTINGS::iterator theListing = 
		pListings->begin();
		theListing != pListings->end();theListing++)
	{
		if(ListingData == (*theListing)->GetListingType())
		{
			continue;
		}

		// be safe 
		Assert(ListingCode == (*theListing)->GetListingType());
		if(ListingCode != (*theListing)->GetListingType())
		{
			ReleaseListings();
			return FALSE;
		}


		PCODE_LISTING pcl = static_cast<PCODE_LISTING>(*theListing);

		AddressContainedType ret = 
			pcl->IsAddressWithinListing(ulAddress);

		if(AddressNotContained == ret)
		{
			continue;
		}
		else if(AddressIsInstruction == ret)
		{
			ReleaseListings();
			return TRUE;
		}
		else
		{
			AssertSz(0,"Address is contained within listing but not "
						"on an instruction boundary...");
			ReleaseListings();
			return TRUE;
		}
	}

	ReleaseListings();
	return FALSE;
}

/////////////////////////////////////////////////////////////////////

BOOL CListingsManager::ConsolidateCodeListings(void)
{
	PLISTINGS pListings = AcquireListings();

	Assert(pListings);
	if(NULL == pListings)
	{
		ReleaseListings();
		return TRUE;
	}

	// Lists are ordered

	LISTINGS::iterator list = pListings->begin();
	LISTINGS::iterator nextList = list + 1;

	while(nextList != pListings->end())
	{
		if (ListingCode != (*list)->GetListingType() ||
			ListingCode != (*nextList)->GetListingType())
		{
			list++;
			nextList = list+1;
			continue;
		}

		PCODE_LISTING p1 = static_cast<PCODE_LISTING>(*list);
		PCODE_LISTING p2 = static_cast<PCODE_LISTING>(*nextList);

		ULONG last = p1->GetNextInsLocAfterLast();
		ULONG first = p2->GetFirst();
			
		if(last == first)
		{
			// Merge

			PINSTRUCTION_MAP pInsMap = p2->GetInstructionMap();
			//Assert(pInsMap);

			if(NULL != pInsMap)
			{
				for(INSTRUCTION_MAP::iterator ins = pInsMap->begin();
					ins != pInsMap->end();ins++)
				{
					p1->AddInstruction(ins->second);
				}

				// Remove old listing

				p2->EmptyMapWithNoDelete();
				delete p2;
				pListings->erase(nextList);
				continue; // Skip update
			}
		}

		list++;
		nextList = list+1;
	}

	ReleaseListings();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////