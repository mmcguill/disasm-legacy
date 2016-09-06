/////////////////////////////////////////////////////////////////////

#include <map>
#include <string.h>
//#include <vector>
//#include <set>
#include "..\..\inc\types.h"
#include "..\..\inc\debug.h"

#include "..\..\inc\symbols.h"

///////////////////////////////////////////////////////////////////////////////

CSymbol::CSymbol() : m_lpszName(NULL), m_ulAddress(0), m_Type(symbol_unknown) 
{
}

///////////////////////////////////////////////////////////////////////////////

CSymbol::CSymbol(ULONG ulAddress,SYMBOL_TYPE Type, LPCSTR lpszName)
{
	m_lpszName=NULL;
	m_ulAddress=0;
	m_Type=symbol_unknown;

	Assert(lpszName);
	if(NULL == lpszName)
	{
		AssertSz(0,"CSymbol() => Error: lpszName == NULL");
		return;
	}

	size_t cch = strlen(lpszName);

	m_lpszName = new CHAR[cch+1];
	Assert(m_lpszName);
	if(NULL == m_lpszName)
	{
		return;
	}
	
	strncpy(m_lpszName,lpszName,cch);
	m_lpszName[cch] = 0;

	m_Type = Type;
	m_ulAddress = ulAddress;
}

///////////////////////////////////////////////////////////////////////////////

CSymbol::~CSymbol()
{
	if(m_lpszName)
	{
		delete m_lpszName;
	}
}

///////////////////////////////////////////////////////////////////////////////

LPCSTR CSymbol::GetName() const
{
	return m_lpszName;
}

///////////////////////////////////////////////////////////////////////////////

SYMBOL_TYPE CSymbol::GetType() const
{
	return m_Type;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CSymbol::GetAddress() const
{
	return m_ulAddress;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CSymbolTable::CSymbolTable()
{
	m_pmapSymbols = new SYMBOLS_MAP;
		
	Assert(m_pmapSymbols);

	if(NULL == m_pmapSymbols)
	{
		return;
	}
}

/////////////////////////////////////////////////////////////////////

CSymbolTable::~CSymbolTable()
{
	for (SYMBOLS_MAP::iterator theSym = m_pmapSymbols->begin();
		theSym != m_pmapSymbols->end(); theSym++)
	{
		Assert(theSym->second);
		delete theSym->second;
	}

	m_pmapSymbols->clear();
	delete m_pmapSymbols;
	m_pmapSymbols = NULL;
}

/////////////////////////////////////////////////////////////////////

BOOL CSymbolTable::InsertSymbol(PSYMBOL pSym)
{
	std::pair<SYMBOLS_MAP::iterator, BOOL> pr = 
		m_pmapSymbols->insert(SYMBOLS_MAP::value_type(pSym->GetAddress(),pSym));

	if((pr.second) == FALSE)
	{
		//AssertSz(0,"Duplicate Map Entry!");
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////

PSYMBOLS_MAP CSymbolTable::GetSymbolsMap()
{
	Assert(m_pmapSymbols);
	return m_pmapSymbols;
}

/////////////////////////////////////////////////////////////////////