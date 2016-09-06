/*******************************************************************/
/* (c) Copyright 2004 - - Mark McGuill. All Rights Reserved.       */
/*                                                                 */
/* File: lists.h                                                   */
/* Owner: Mark McGuill                                             */
/* Create Date: 09/Sep/2004										   */
/*                                                                 */
/* Purpose: All the custom lists for disassembly & analysis        */
/*                                                                 */
/*                                                                 */
/* Comments: File is fully ANSI C compliant, should compile under  */
/*           Windows, Linux, BSD etc, tested with VC, GCC, CC      */
/*                                                                 */
/*                                                                 */
/* Modified:                                                       */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

// needed for Critical Section!

#define WIN32_LEAN_AND_MEAN
#define WINVER			0x0400		
#define _WIN32_WINNT	0x0400
#define _WIN32_WINDOWS	0x0410
#define _WIN32_IE		0x0400

#include <windows.h>

/*******************************************************************/

typedef std::map<ULONG, PINSTRUCTION>	INSTRUCTION_MAP, 
										*PINSTRUCTION_MAP;

typedef const PINSTRUCTION_MAP	PCINSTRUCTION_MAP;

/*******************************************************************/

enum AddressContainedType
{
	AddressIsInstruction, // Address is start of Valid instruction
	
	AddressNotContained,  // Address is not within this listing

	AddressContainedButNotIntegrally,	// Address is within this 
										// listing but in middle of 
										// another instruction 
										// (integrity problem??)
};

/*******************************************************************/

enum ListingType
{
	ListingCode,
	ListingData,
};

/*******************************************************************/

class CListing
{
public:
	CListing(ListingType type) : m_type(type)
	{
	}

	virtual ~CListing() {}

	// This an inclusive addresses
	virtual ULONG GetStartAddress() = 0;

	// This is an inclusive addresses
	virtual ULONG GetEndAddress() = 0;

	// This is the listing type (code / data)
	ListingType GetListingType() {return m_type;}

protected:
	ListingType m_type;
};

/*******************************************************************/

typedef CListing LISTING, *PLISTING;

/*******************************************************************/

class CDataListing : public CListing
{
public:
	CDataListing(ULONG start, ULONG end) : CListing(ListingData) 
	{
		m_ulStart = start;
		m_ulEnd = end;
	}

	virtual ULONG GetStartAddress() { return m_ulStart;}
	virtual ULONG GetEndAddress()   { return m_ulEnd;}

private:
    ULONG m_ulStart;
	ULONG m_ulEnd;
};

/*******************************************************************/

typedef CDataListing DATA_LISTING, *PDATA_LISTING;

/*******************************************************************/

class CCodeListing : public CListing
{
public:
	CCodeListing(void);
	virtual ~CCodeListing(void);
	
	BOOL AddInstruction(PINSTRUCTION pInst);
	AddressContainedType 
		IsAddressWithinListing(ULONG ulAddress);

	PINSTRUCTION_MAP GetInstructionMap() 
						{return m_pmapInstructions;}

	// overrides

	virtual ULONG GetStartAddress();
	virtual ULONG GetEndAddress();

	
	// Useful Funcs 

	ULONG GetFirst();
	ULONG GetNextInsLocAfterLast();


	// HACKHACK: Used by CCodeListingsManager when merging listings
	// Remove ASAP

	BOOL EmptyMapWithNoDelete()
		{m_pmapInstructions->clear();return TRUE;}

protected:
	PINSTRUCTION_MAP m_pmapInstructions;
};

/*******************************************************************/

typedef CCodeListing CODE_LISTING, *PCODE_LISTING;

/*******************************************************************/
/*******************************************************************/

typedef std::vector<PLISTING>	LISTINGS, 
								*PLISTINGS;

typedef const PLISTINGS			PCLISTINGS;

/*******************************************************************/

class CListingsManager
{
public:
	CListingsManager(void);
	~CListingsManager(void);

	// Synchronisation is up to caller...

	PLISTINGS AcquireListings();
	void ReleaseListings();

	
	// These Handle synchronisation themselves

	BOOL AddListing(PLISTING pList);

	// Change this to AddressIsType(); -> return type
	
	BOOL IsAddressDisassembled(ULONG ulAddress);
	
	
	PINSTRUCTION GetInstructionAtAddress(ULONG ulAddress);

	BOOL ConsolidateCodeListings(void);

protected:
	PLISTINGS m_pListings;
	CRITICAL_SECTION m_cs;
private:
};

/*******************************************************************/

typedef CListingsManager		LISTINGS_MANAGER,	
								*PLISTINGS_MANAGER;

typedef const PLISTINGS_MANAGER	PCLISTINGS_MANAGER;

/*******************************************************************/