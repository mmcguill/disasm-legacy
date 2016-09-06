///////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winnt.h>
#include <delayimp.h>
#include <vector>
#include <map>
#include "resource.h"
#include "..\..\inc\debug.h"
#include <strsafe.h>

///////////////////////////////////////////////////////////////////////////////

#include <atlconv.h>

///////////////////////////////////////////////////////////////////////////////

// This is needed by Phoenix Disassembler in function LoadSymbols
// The disassembler will pass us the symbol table and we will fill it
// with exports and imports and entrypoints etc.

#include "..\..\inc\symbols.h"

///////////////////////////////////////////////////////////////////////////////

#include "..\..\inc\pedasm.h"
#include <shlwapi.h>

///////////////////////////////////////////////////////////////////////////////

static LPTSTR kszEntryPointSymbolName = TEXT("<EP>");

///////////////////////////////////////////////////////////////////////////////

CPEParser::CPEParser(void)
{
	m_pvecImportModules = NULL;
	m_pvecExports = NULL;
	m_pvecSections = NULL;
	m_pImageBase = NULL;
	m_ulImageSize = 0;
	m_lpszFile = NULL;
	m_fOpen = false;
}

///////////////////////////////////////////////////////////////////////////////

CPEParser::~CPEParser(void)
{
	Close();
}

///////////////////////////////////////////////////////////////////////////////

LPCTSTR CPEParser::GetFileName() const
{
	return m_lpszFile;
}

///////////////////////////////////////////////////////////////////////////////

bool CPEParser::Open(LPCTSTR pszFile)
{
	Close();

	Assert(pszFile);

	size_t cchFile = 0;
	if(	NULL == pszFile || 
		(!SUCCEEDED(StringCchLength(pszFile,MAX_PATH,&cchFile))))
	{
		PEDasmSetLastError(idsErrInvalidArg);
		return false;
	}

	m_lpszFile = new TCHAR[cchFile+1];
	Assert(pszFile);
	if(NULL == m_lpszFile)
	{
		PEDasmSetLastError(idsErrMemory);
		return false;
	}

	if(!SUCCEEDED(StringCchCopy(m_lpszFile,cchFile+1,pszFile)))
	{
		delete m_lpszFile;
		m_lpszFile = NULL;
		PEDasmSetLastError(idsErrInvalidArg);
		return false;
	}

	// Load

	PRELOADER_MAP_INFO pmi = {0};
	if(	!PreLoaderMapFile(&pmi) || !IsValidPEFile(&pmi) || !LoaderMapFile(&pmi)
		|| !LoadSections() || !LoadExports() || !LoadImports())
	{
		Close();
		return false;
	}

	m_fOpen = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

// Do not use for analysis purposes. This is used only for reverse lookup of 
// import names from ordinals for performance reasons.

bool CPEParser::OpenExpressExports(LPCTSTR pszFile)
{
	Close();

	Assert(pszFile);

	size_t cchFile = 0;
	if(	NULL == pszFile || 
		(!SUCCEEDED(StringCchLength(pszFile,MAX_PATH,&cchFile))))
	{
		PEDasmSetLastError(idsErrInvalidArg);
		return false;
	}

	m_lpszFile = new TCHAR[cchFile+1];
	Assert(pszFile);
	if(NULL == m_lpszFile)
	{
		PEDasmSetLastError(idsErrMemory);
		return false;
	}

	if(!SUCCEEDED(StringCchCopy(m_lpszFile,cchFile+1,pszFile)))
	{
		delete m_lpszFile;
		m_lpszFile = NULL;
		PEDasmSetLastError(idsErrInvalidArg);
		return false;
	}

	// Load

	PRELOADER_MAP_INFO pmi = {0};
	if(	!PreLoaderMapFile(&pmi) || !IsValidPEFile(&pmi) || !LoaderMapFile(&pmi)
		|| !LoadSections() || !LoadExports())
	{
		Close();
		return false;
	}

	m_fOpen = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Perform standard memory map of file for quick Loader mapping 
// and validity checking, discarded after Loader map

bool CPEParser::PreLoaderMapFile(PPRELOADER_MAP_INFO ppmi)
{
	Assert(m_lpszFile);
	
	// Open File

	ppmi->hFile = CreateFile(m_lpszFile,GENERIC_READ,FILE_SHARE_READ,NULL,
								OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,(HANDLE)0);

	Assert(INVALID_HANDLE_VALUE != ppmi->hFile);

	if (INVALID_HANDLE_VALUE == ppmi->hFile)
	{
		PEDasmSetLastError(idsErrOpenFile);
		return false;
	}

	
	// Size?

	LARGE_INTEGER liFileSize = {0};
	if(0 == GetFileSizeEx(ppmi->hFile,&liFileSize))
	{
		AssertSz(0,"GetFileSizeEx failed.");
		PEDasmSetLastError(idsErrOpenFile);
		return false;
	}
	
	ppmi->ulSize = liFileSize.LowPart;

	
	// Create a mapping

	Assert(0 != ppmi->ulSize  && 0xFFFFFFFF != ppmi->ulSize);

	if(0 == ppmi->ulSize  || 0xFFFFFFFF == ppmi->ulSize)
	{
		PEDasmSetLastError(idsErrInvalidFileSize);
		return false;
	}

	ppmi->hMap = CreateFileMapping(ppmi->hFile,NULL,PAGE_READONLY,0,0,NULL);
	
	Assert(NULL != ppmi->hMap);

	if (NULL == ppmi->hMap)
	{
		PEDasmSetLastError(idsErrOpenFile);
		CloseHandle(ppmi->hFile);
		return false;
	}
	

	// Map full view of file into memory

	ppmi->pBase = (LPBYTE)MapViewOfFile(ppmi->hMap,FILE_MAP_READ,0,0,0);

	Assert(NULL != ppmi->pBase);

	if (NULL == ppmi->pBase)
	{
		PEDasmSetLastError(idsErrOpenFile);
		CloseHandle(ppmi->hFile);
		CloseHandle(ppmi->hMap);
		return false;
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Ensure we have a valid PE

bool CPEParser::IsValidPEFile(PPRELOADER_MAP_INFO ppmi) const
{
	Assert(ppmi);
	Assert(ppmi->pBase);
	Assert(ppmi->ulSize);

	
	// Big enough to hold MSDOS header?

	if(sizeof(IMAGE_DOS_HEADER) > ppmi->ulSize)
	{
		PEDasmSetLastError(idsErrFileTooSmall);
		UnmapViewOfFile(ppmi->pBase);
		CloseHandle(ppmi->hMap);
		CloseHandle(ppmi->hFile);
		return false;
	}

	
	// big enough to hold all NT headers?

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ppmi->pBase;

	if(((unsigned long)pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS)) > ppmi->ulSize)
	{
		PEDasmSetLastError(idsErrFileTooSmall);
		UnmapViewOfFile(ppmi->pBase);
		CloseHandle(ppmi->hMap);
		CloseHandle(ppmi->hFile);
		return false;
	}


	// Check For MSDOS header signature => 'MZ'
	
	if(IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
	{
		PEDasmSetLastError(idsErrNoMSDOSSignature);
		//AssertSz(0,"No MSDOS signature found.");
		UnmapViewOfFile(ppmi->pBase);
		CloseHandle(ppmi->hMap);
		CloseHandle(ppmi->hFile);
		return false;
	}


	// Check for NT Signature 'PE00'

	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS) (ppmi->pBase + pDosHeader->e_lfanew);

	if(IMAGE_NT_SIGNATURE != pNTHeaders->Signature)
	{
		PEDasmSetLastError(idsErrNoNTSignature);
		//AssertSz(0,"No NT signature found.");
		UnmapViewOfFile(ppmi->pBase);
		CloseHandle(ppmi->hMap);
		CloseHandle(ppmi->hFile);
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Map PE File into memory as loader would

bool CPEParser::LoaderMapFile(PPRELOADER_MAP_INFO ppmi)
{
	Assert(ppmi);
	Assert(ppmi->pBase);
	Assert(ppmi->ulSize);
	
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ppmi->pBase;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS) 
		(ppmi->pBase + pDosHeader->e_lfanew);

	
	// Get Total size and reserve memory for ourselves

	m_pImageBase = (unsigned char*)VirtualAlloc(NULL,
		pNTHeaders->OptionalHeader.SizeOfImage,MEM_COMMIT,PAGE_READWRITE);

	
	// Change Image Size To Virtual Size Now.

	ppmi->ulSize = pNTHeaders->OptionalHeader.SizeOfImage;


	Assert(m_pImageBase);

	if(NULL == m_pImageBase)
	{
		PEDasmSetLastError(idsErrMemory);
		UnmapViewOfFile(ppmi->pBase);
		CloseHandle(ppmi->hMap);
		CloseHandle(ppmi->hFile);
		return false;
	}

	
	__try
	{
		// Map in headers

		CopyMemory(m_pImageBase,ppmi->pBase,SIZE_OF_ALL_HEADERS(pDosHeader, pNTHeaders));

		
		// Map in sections

		PIMAGE_SECTION_HEADER pish = 
			(PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION(pNTHeaders);

		for(int i=0;   i < pNTHeaders->FileHeader.NumberOfSections;   i++,pish++)
			{
			CopyMemory(m_pImageBase + pish->VirtualAddress,
				ppmi->pBase + pish->PointerToRawData, pish->SizeOfRawData);
			}
	    
	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		AssertSz(0,"Access Violation occured while mapping file.");
		AssertSz(0,(LPTSTR)m_lpszFile);
		PEDasmSetLastError(idsErrCorruptFile);
		UnmapViewOfFile(ppmi->pBase);
		CloseHandle(ppmi->hMap);
		CloseHandle(ppmi->hFile);
		return false;
	}

	// All ok initialize our private header members for ease of use

	CopyMemory(&m_hdDosHeader,pDosHeader,sizeof(IMAGE_DOS_HEADER));
	CopyMemory(&m_hdNTHeaders,pNTHeaders,sizeof(IMAGE_NT_HEADERS));

	
	// Close of Pre Loader map, no longer needed

	UnmapViewOfFile(ppmi->pBase);
	CloseHandle(ppmi->hMap);
	CloseHandle(ppmi->hFile);

	m_ulImageSize = ppmi->ulSize;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

PSECTIONS_VECTOR CPEParser::GetSectionsVector(void) const
{
	return m_pvecSections->GetVector();
}

///////////////////////////////////////////////////////////////////////////////

PEXPORTS_VECTOR CPEParser::GetExportsVector(void) const
{
	return m_pvecExports->GetVector();
}

///////////////////////////////////////////////////////////////////////////////

PIMPORT_MODULES_VECTOR CPEParser::GetImportModulesVector(void) const
{
	return m_pvecImportModules->GetVector();
}

///////////////////////////////////////////////////////////////////////////////

bool CPEParser::IsOpen(void) const
{
	return m_fOpen;
}

///////////////////////////////////////////////////////////////////////////////

bool CPEParser::Close(void)
{	
	if(m_pvecSections)
	{
		delete m_pvecSections;
		m_pvecSections = NULL;
	}


	if(m_pvecImportModules)
	{
		delete m_pvecImportModules;
		m_pvecImportModules = NULL;
	}

	
	if(m_pvecExports)
	{
		delete m_pvecExports;
		m_pvecExports = NULL;
	}

	
	if(m_pImageBase)
	{
		VirtualFree(m_pImageBase,0,MEM_RELEASE);
	}

	
	if(NULL != m_lpszFile)
	{
		delete m_lpszFile;
        m_lpszFile = NULL;
	}

	m_fOpen = false;
	m_ulImageSize = 0;

	ZeroMemory(&m_hdNTHeaders,sizeof(IMAGE_NT_HEADERS));
	ZeroMemory(&m_hdDosHeader,sizeof(IMAGE_DOS_HEADER));

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CPEParser::LoadSections(void)
{
	Assert(m_pImageBase);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);

	// Add all sections to m_pvecSections

	PIMAGE_SECTION_HEADER pish = (PIMAGE_SECTION_HEADER)
		IMAGE_FIRST_SECTION(m_pImageBase + m_hdDosHeader.e_lfanew);

	for(int i=0;i < m_hdNTHeaders.FileHeader.NumberOfSections;i++,pish++)
	{
		if(NULL == m_pvecSections)
		{
			m_pvecSections = new SECTIONS_AUTO_VECTOR;

			Assert(m_pvecSections);

			if(NULL == m_pvecSections)
			{
				PEDasmSetLastError(idsErrMemory);
				return false;
			}
		}

		PSECTION pSection = new SECTION(pish->VirtualAddress,
										pish->Misc.VirtualSize,
										pish->Characteristics,
										(LPCSTR)pish->Name);

		Assert(pSection);

		if(NULL == pSection)
		{
			PEDasmSetLastError(idsErrMemory);
			return false;
		}
		
		
		// Add

		m_pvecSections->GetVector()->push_back(pSection);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CPEParser::LoadExports(void)
{
	Assert(m_pImageBase);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);

	
	// Get Exports Directory

	if(0 == m_hdNTHeaders.OptionalHeader.
		DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
	{
		// No Exports
		return true;
	}
	

	PIMAGE_EXPORT_DIRECTORY ped = (PIMAGE_EXPORT_DIRECTORY)
		(m_pImageBase + 
		m_hdNTHeaders.OptionalHeader.
		DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		

	// Sanity Check Here

	if(0xFFFF < ped->NumberOfFunctions)
	{
        // Insane => Bail
		AssertSz(0,"Export Sanity Check Fail");
		return false;
	}

	// Add all exports to our std::vector
	

	for(DWORD i=0;i<ped->NumberOfFunctions;i++)
	{
		unsigned long ulOrdinal = ped->Base + i;
		unsigned long ulRVA = (*(unsigned long*)
			(m_pImageBase + ped->AddressOfFunctions + (i * sizeof(DWORD))));

		// Not a real export just a gap in the array, due to the way
		// ordinals are setup. e.g.

		//LIBRARY	dasmeng

		//EXPORTS
		//DisassembleEPs @1
		//GetCodeListings @7
		//CreateDisasm
		//CloseDisasm
		//TestShit  @5 NONAME

		// So this will work out like this:

		//Ordinal base:    00000001
		//# of functions:  00000007
		//# of Names:      00000004

		//Entry Pt  Ordn  Name
		//000020A0     1  DisassembleEPs
		//00002090     2  CloseDisasm
		//00002210     3  CreateDisasm
		//00002080     5
		//000020C0     7  GetCodeListings
		// 4 and 6 will be 0. That is, they are gaps.

		if(0 == ulRVA)
		{
			continue;
		}

		// Got a name?

		LPCSTR lpszName = NULL;
		for(DWORD j=0;j<ped->NumberOfNames;j++)
		{
			WORD dwCurNameOrdinal = *(WORD*)(m_pImageBase + 
				ped->AddressOfNameOrdinals + (j * sizeof(WORD)));
			
			if(dwCurNameOrdinal == i)
			{
				DWORD dwNameRVA = *(DWORD*)(m_pImageBase+ped->AddressOfNames+
					(j * sizeof(DWORD)));

				lpszName = (LPCSTR)(m_pImageBase + dwNameRVA);
				break;
			}
		}

		// Forwarder??

		DWORD dwSectBase = m_hdNTHeaders.OptionalHeader.
			DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

		DWORD dwSectEnd = dwSectBase + m_hdNTHeaders.OptionalHeader.
			DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		DWORD dwNameRVA = *(DWORD*)
			(m_pImageBase + ped->AddressOfFunctions + (i * sizeof(DWORD)));

		
		bool fForwarder = false;
		LPCSTR lpszForwarder = NULL;

		if( dwSectBase <= (dwNameRVA) &&
			dwSectEnd >= (dwNameRVA))
		{
			LPSTR pszForward = (LPSTR)(m_pImageBase + dwNameRVA);

			// check for at least: '%.%' => if this is found we accept this as 
			// a valid forwarder.

			LPSTR pszDot = strchr(pszForward,'.');
			if(NULL != pszDot &&			// Contains a dot
				pszForward != pszDot &&     // Dot Not as first character
				*(pszDot + 1) != 0)         // Dot Not as last character
			{
				fForwarder = true;
				lpszForwarder = pszForward;
			}
		}
	
        // Add it to our export std::vector

		PEXPORT_FUNCTION pef = new 
			EXPORT_FUNCTION(ulOrdinal,ulRVA,lpszName,fForwarder,lpszForwarder);

		Assert(pef);
		
		if(NULL == pef)
		{
			PEDasmSetLastError(idsErrMemory);
			return false;
		}

		if(NULL == m_pvecExports)
		{
			m_pvecExports = new EXPORTS_AUTO_VECTOR;
			Assert(m_pvecExports);

			if(NULL == m_pvecExports)
			{
				PEDasmSetLastError(idsErrMemory);
				delete pef;
				return false;
			}
		}

		m_pvecExports->GetVector()->push_back(pef);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CPEParser::LoadImports(void)
{
	Assert(m_pImageBase);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);

	
	// Get Imports Directory

	if(0 == m_hdNTHeaders.OptionalHeader.
		DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
	{
		// No Imports ???!!!
		return true;
	}

	PIMAGE_IMPORT_DESCRIPTOR piid = (PIMAGE_IMPORT_DESCRIPTOR) (m_pImageBase + 
				m_hdNTHeaders.OptionalHeader.
				DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	
	// Enumerate Each Module

	for(;piid->OriginalFirstThunk;piid++)
	{
		PIMPORT_MODULE pim = new IMPORT_MODULE((LPCSTR)(m_pImageBase + piid->Name),false);
		
		Assert(pim);
		
		if(NULL == pim)
		{
			PEDasmSetLastError(idsErrMemory);
			return false;
		}

		// Try Original First Thunk First if this fails try FirstThunk member
		// Old borland linkers do not emit an OriginalFirstThunk Table.
		// From here on in OriginalFirstThunk will be known as the INT (Import Names Table)
		// and FirstThunk as IAT (Import Addresses Table)

		PIMAGE_THUNK_DATA pitd = (PIMAGE_THUNK_DATA)(m_pImageBase + piid->OriginalFirstThunk);
		PIMAGE_THUNK_DATA pitdIAT = (PIMAGE_THUNK_DATA)(m_pImageBase + piid->FirstThunk);
		
		if(0 == pitd->u1.AddressOfData)
		{
			AssertSz(0,"OriginalFirstThunk (INT) first element was 0");
			pitd = pitdIAT;
		}

		// still zero, we have a problem

		if(0 == pitd->u1.AddressOfData)
		{
			AssertSz(0,"Both OriginalFirstThunk (INT) and FirstThunk (IAT) first elements were 0");
			PEDasmSetLastError(idsErrCorruptFile);
			delete pim;
			return false;
		}

		if(!LoadImportFunctionsFromFirstITDForModule(pim, pitd, pitdIAT, false, false))
		{
			AssertSz(0,"There was an error loading functions.");
			delete pim;
			return false;
		}

		
		// Add it to our import std::vector

		if(NULL == m_pvecImportModules)
		{
			m_pvecImportModules = new IMPORT_MODULES_AUTO_VECTOR;
			Assert(m_pvecImportModules);

			if(NULL == m_pvecImportModules)
			{
				PEDasmSetLastError(idsErrMemory);
				delete pim;
				return false;
			}
		}

		m_pvecImportModules->GetVector()->push_back(pim);
	}


	// Delay Loaded Imports

	// Get DelayLoad Imports Directory

	if(0 == m_hdNTHeaders.OptionalHeader.
		DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress)
	{
		// No Delay Loaded Imports
		return true;
	}

	ImgDelayDescr* pIDD = (ImgDelayDescr*) 
		(m_pImageBase + m_hdNTHeaders.OptionalHeader.
		DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress);

	
	for(;pIDD->rvaINT;pIDD++)
	{
		// pIDD->rvaINT: Depending on grAttrs bit 1 this is an RVA to 
		// the INT or a VA!! believe it or not 

		LPSTR lpszName = NULL;
		if((pIDD->grAttrs & dlattrRva) == dlattrRva)
		{
			lpszName = (LPSTR)(m_pImageBase + pIDD->rvaDLLName);
		}
		else
		{
			lpszName = (LPSTR)(m_pImageBase + 
				(pIDD->rvaDLLName - m_hdNTHeaders.OptionalHeader.ImageBase));
		}

		PIMPORT_MODULE pim = new IMPORT_MODULE(lpszName,true);
		
		Assert(pim);
		
		if(NULL == pim)
		{
			PEDasmSetLastError(idsErrMemory);
			return false;
		}

		// pIDD->rvaINT: Depending on grAttrs bit 1 this is an RVA to the
		// INT or a VA!! believe it or not 

		PIMAGE_THUNK_DATA pitd = NULL;
		PIMAGE_THUNK_DATA pitdIAT = NULL;

		bool fUseRVAs = ((pIDD->grAttrs & dlattrRva) == dlattrRva);
		if(fUseRVAs)
		{
			pitd = (PIMAGE_THUNK_DATA)(m_pImageBase + pIDD->rvaINT);
			pitdIAT = (PIMAGE_THUNK_DATA)(m_pImageBase + pIDD->rvaIAT);
		}
		else
		{
			pitd = (PIMAGE_THUNK_DATA)(m_pImageBase + 
				(pIDD->rvaINT - m_hdNTHeaders.OptionalHeader.ImageBase));
			pitdIAT = (PIMAGE_THUNK_DATA)(m_pImageBase + 
				(pIDD->rvaIAT- m_hdNTHeaders.OptionalHeader.ImageBase));
		}

		if(!LoadImportFunctionsFromFirstITDForModule(pim, pitd, pitdIAT,true, fUseRVAs))
		{
			AssertSz(0,"There was an error loading delay load functions.");
			delete pim;
			return false;
		}
		
		// Add it to our imports std::vector

		if(NULL == m_pvecImportModules)
		{
			m_pvecImportModules = new IMPORT_MODULES_AUTO_VECTOR;
			Assert(m_pvecImportModules);

			if(NULL == m_pvecImportModules)
			{
				PEDasmSetLastError(idsErrMemory);
				delete pim;
				return false;
			}
		}

		m_pvecImportModules->GetVector()->push_back(pim);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

// PathFindOnPath Function Pointer prototype
typedef BOOL (__stdcall *PFNPFOP)(LPTSTR pszFile, LPCTSTR *ppszOtherDirs);

BOOL PreloadReverseLookupModule(LPCTSTR lpszModName,CPEParser **ppMod, 
								PEXPORTS_VECTOR *ppev);

bool CPEParser::LoadImportFunctionsFromFirstITDForModule(
		PIMPORT_MODULE pim, 
		PIMAGE_THUNK_DATA pitd, 
		PIMAGE_THUNK_DATA pitdIAT,
		bool fDelayLoad, 
		bool fDelayLoadUseRVAs)
{
	BOOL fRevLookupModLoaded = FALSE;
	PEXPORTS_VECTOR pev = NULL;
	CPEParser *pExpModule = NULL;

	for(;pitd->u1.AddressOfData;pitd++,pitdIAT++)
	{
		// RVA to loc thru which calls are made
		// they are always made thru the IAT (FirstThunk member)

		unsigned long ulRVA = (unsigned long) (((unsigned char*)pitdIAT) + 
			FIELD_OFFSET(IMAGE_THUNK_DATA,u1.Function) - m_pImageBase);
		
		
		// Ordinal or Name??
	
		unsigned long ulOrdinal = 0;
		LPSTR lpszName = NULL;
		
		
		// For attempt at reverse ordinal lookup

		TCHAR szExpName[MAX_CCH_EXPORT_NAME];
#ifdef UNICODE
		char szMBStr[MAX_CCH_EXPORT_NAME];
#endif

		if(IMAGE_SNAP_BY_ORDINAL(pitd->u1.Ordinal))
		{
			ulOrdinal = IMAGE_ORDINAL(pitd->u1.Ordinal);

			// Attempt an export name lookup on the dll 
			// itself if we can find it.

			if(!fRevLookupModLoaded)
			{
				fRevLookupModLoaded = PreloadReverseLookupModule(pim->GetName(),&pExpModule,&pev);
			}

			if(fRevLookupModLoaded)
			{
				for(EXPORTS_VECTOR::iterator it = pev->begin();
					it != pev->end();it++)
				{
					PEXPORT_FUNCTION pef = *it;
					Assert(pef);
				
					if((ulOrdinal == pef->GetOrdinal()))
					{
						if(NULL == pef->GetName())
						{
							break;
						}

						StringCchCopy(szExpName,MAX_CCH_EXPORT_NAME-1,pef->GetName());

						#ifdef UNICODE 
							WideCharToMultiByte(CP_ACP,0,szExpName,-1,
												szMBStr,MAX_CCH_EXPORT_NAME,NULL,NULL);
							lpszName = szMBStr;
						#else
							lpszName = szExpName;
						#endif
					}
				}
			}
		}
		else
		{
			ulOrdinal = 0;
			PIMAGE_IMPORT_BY_NAME piibn = (PIMAGE_IMPORT_BY_NAME)(m_pImageBase + 
				(pitd->u1.AddressOfData - 
				((fDelayLoad && !fDelayLoadUseRVAs) 
				? m_hdNTHeaders.OptionalHeader.ImageBase : 0)) );

			lpszName = (LPSTR)piibn->Name;
		}

		// Add it to our import functions std::vector

		PIMPORT_FUNCTION pif = new 
			IMPORT_FUNCTION(lpszName,ulOrdinal,ulRVA);
			
		Assert(pif);
		
		if(NULL == pif)
		{
			PEDasmSetLastError(idsErrMemory);
			return false;
		}

		pim->GetImportFunctions()->push_back(pif);
	}

	if(fRevLookupModLoaded)
	{
		delete pExpModule;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////

BOOL PreloadReverseLookupModule(LPCTSTR lpszModName,CPEParser **ppMod, 
								PEXPORTS_VECTOR *ppev)
{
	Assert(lpszModName);
	Assert(ppMod);
	Assert(ppev);

	TCHAR szModName[MAX_PATH];
	StringCchCopy(szModName,MAX_PATH-1,lpszModName);

	if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(szModName))
	{
		HMODULE hMod = LoadLibrary(TEXT("SHLWAPI.DLL"));
		Assert(hMod);

		if(NULL == hMod)
		{
			return FALSE;
		}

		PFNPFOP pfnPathFindOnPath;

		#ifdef UNICODE 
		pfnPathFindOnPath = 
			(PFNPFOP)GetProcAddress(hMod,"PathFindOnPathW");
		#else
		pfnPathFindOnPath = 
			(PFNPFOP)GetProcAddress(hMod,"PathFindOnPathA");
		#endif

		Assert(pfnPathFindOnPath);

		if(NULL == pfnPathFindOnPath)
		{
			FreeLibrary(hMod);
			return FALSE;
		}
		
		if(!pfnPathFindOnPath(szModName,NULL))
		{
			FreeLibrary(hMod);
			return FALSE;
		}

		FreeLibrary(hMod);
	}

	*ppMod = new CPEParser();

	Assert(*ppMod);
	if(NULL == (*ppMod))
	{
		return FALSE;
	}


	if(!(*ppMod)->OpenExpressExports(szModName))
	{
		return FALSE;
	}

	
	*ppev = (*ppMod)->GetExportsVector();
	
	if(NULL == (*ppev))
	{
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////

bool CPEParser::LoadSymbols(PSYMBOL_TABLE pSymTab)
{
	Assert(m_pImageBase);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);

	// Symbols consist of Exports & Imports and any debug information we can
	// attain + Entrypoint if there is one.

	Assert(pSymTab);

	// start with entry point, there may not be one.

	unsigned long ulEntryPoint = m_hdNTHeaders.OptionalHeader.AddressOfEntryPoint;
	unsigned long ulImageBase = m_hdNTHeaders.OptionalHeader.ImageBase;

	if(ulEntryPoint)
	{
		ulEntryPoint += ulImageBase;

		USES_CONVERSION;

		PSYMBOL pSym = new SYMBOL(ulEntryPoint,symbol_entrypoint,
			T2A(kszEntryPointSymbolName));
		
		Assert(pSym);

		if(NULL == pSym)
		{
			PEDasmSetLastError(idsErrMemory);
			return false;
		}

		if(!pSymTab->InsertSymbol(pSym))
		{
			Assert(FALSE);
			delete pSym;
			return false;
		}
	}

	// Add Exports

	if(NULL != m_pvecExports)
	{
		std::vector<PEXPORT_FUNCTION>* pExpVec = m_pvecExports->GetVector();

		for(std::vector<PEXPORT_FUNCTION>::const_iterator theExp = pExpVec->begin();
			theExp != pExpVec->end();theExp++)
		{
			PEXPORT_FUNCTION pExp = *theExp;
		
			TCHAR szFormat[MAX_CCH_SYMBOL_NAME];
			if(pExp->GetName())
			{
				if(!SUCCEEDED(StringCchPrintf(szFormat,MAX_CCH_SYMBOL_NAME,TEXT("%s"),
					pExp->GetName())))
				{
					AssertSz(0,"Error Formatting");
					return false;
				}
			}
			else
			{
				if(!SUCCEEDED(StringCchPrintf(szFormat,MAX_CCH_SYMBOL_NAME,TEXT("Ordinal:0x%0.4X"),
					pExp->GetOrdinal())))
				{
					AssertSz(0,"Error Formatting");
					return false;
				}				
			}
			
			
			USES_CONVERSION;

			PSYMBOL pSym = new SYMBOL(pExp->GetRVA() + ulImageBase,
				symbol_export,T2A(szFormat));

			Assert(pSym);

			if(NULL == pSym)
			{
				PEDasmSetLastError(idsErrMemory);
				return false;
			}

			if(NULL == pSym)
			{
				PEDasmSetLastError(idsErrMemory);
				return false;
			}

			if(!pSymTab->InsertSymbol(pSym))
			{
				Assert(FALSE);
				delete pSym;
				return false;
			}
		}
	}

	// Add Imports

	if(NULL != m_pvecImportModules)
	{
		std::vector<PIMPORT_MODULE>* pImpModVec = m_pvecImportModules->GetVector();

		for(std::vector<PIMPORT_MODULE>::const_iterator theMod = pImpModVec->begin();
			theMod != pImpModVec->end(); theMod++)
		{

			PIMPORT_MODULE pim = (*theMod);
			
			// May not contain any functions!!

			PIMPORT_FUNCTIONS_VECTOR pImpFuncVec = pim->GetImportFunctions();
			if(0 == pImpFuncVec->size())
			{
				continue;
			}


			for(std::vector<PIMPORT_FUNCTION>::const_iterator theFunc = pImpFuncVec->begin();
				theFunc != pImpFuncVec->end(); theFunc++)
			{
				PIMPORT_FUNCTION pif = *theFunc;

				// Format Name like module!function

				TCHAR szFormat[MAX_CCH_SYMBOL_NAME];

				if(pif->GetName())
				{
					if(!SUCCEEDED(StringCchPrintf(szFormat,MAX_CCH_SYMBOL_NAME,TEXT("%s!%s"),
						pim->GetName(),pif->GetName())))
					{
						AssertSz(0,"Error Formatting");
						return false;
					}
				}
				else
				{
					if(!SUCCEEDED(StringCchPrintf(szFormat,MAX_CCH_SYMBOL_NAME,TEXT("%s!ord_%d"),
						pim->GetName(),pif->GetOrdinal())))
					{
						AssertSz(0,"Error Formatting");
						return false;
					}
				}


				USES_CONVERSION;

				PSYMBOL pSym = new SYMBOL(pif->GetRVA() + ulImageBase,
					(pim->IsDelayLoad()? symbol_delay_import : symbol_import),
					T2A(szFormat));

				Assert(pSym);

				if(NULL == pSym)
				{
					PEDasmSetLastError(idsErrMemory);
					return false;
				}

				if(NULL == pSym)
				{
					PEDasmSetLastError(idsErrMemory);
					return false;
				}

				if(!pSymTab->InsertSymbol(pSym))
				{
					Assert(FALSE);
					delete pSym;
					return false;
				}
			}
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////

// we use PathFindOnPath below from whlwapi, but we're late binding 
// so we don't have to feel the pain and so the dll will load on 
// older systems or linux!!

//typedef BOOL (__stdcall *PFNPFOP)(LPTSTR pszFile, LPCTSTR *ppszOtherDirs);
//
//bool CPEParser::AttemptOrdinalExpNameLookup(LPCTSTR lpszModName,
//											unsigned long ulOrdinal, 
//											LPTSTR lpszExpName)
//{
//	TCHAR szModName[MAX_PATH];
//	StringCchCopy(szModName,MAX_PATH-1,lpszModName);
//
//	if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(szModName))
//	{
//		HMODULE hMod = LoadLibrary(TEXT("SHLWAPI.DLL"));
//		Assert(hMod);
//
//		if(NULL == hMod)
//		{
//			return false;
//		}
//
//        PFNPFOP pfnPathFindOnPath;
//
//#ifdef UNICODE 
//		pfnPathFindOnPath = 
//			(PFNPFOP)GetProcAddress(hMod,"PathFindOnPathW");
//#else
//		pfnPathFindOnPath = 
//			(PFNPFOP)GetProcAddress(hMod,"PathFindOnPathA");
//#endif
//
//		Assert(pfnPathFindOnPath);
//
//		if(NULL == pfnPathFindOnPath)
//		{
//			return false;
//		}
//		
//		if(!pfnPathFindOnPath(szModName,NULL))
//		{
//			return false;
//		}
//	}
//
//	CPEParser tmp;
//	if(!tmp.Open(szModName))
//	{
//		return false;
//	}
//
//	PEXPORTS_VECTOR pev = tmp.GetExportsVector();
//    
//	if(NULL == pev)
//	{
//		return false;
//	}
//			
//	for(EXPORTS_VECTOR::iterator it = pev->begin();
//		it != pev->end();it++)
//	{
//		PEXPORT_FUNCTION pef = *it;
//		Assert(pef);
//	
//		if((ulOrdinal == pef->GetOrdinal()) &&
//			NULL != pef->GetName())
//		{
//			StringCchCopy(lpszExpName,MAX_CCH_EXPORT_NAME-1,pef->GetName());
//			return true;
//		}
//	}
//
//	return false;
//}

///////////////////////////////////////////////////////////////////////////////

unsigned char* CPEParser::GetImage(void) const
{
	Assert(m_fOpen);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);
	return m_pImageBase;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CPEParser::GetImageSize(void) const
{
	Assert(m_fOpen);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);
	return m_ulImageSize;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CPEParser::GetEntryPoint(void) const
{
	Assert(m_fOpen);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);
	return m_hdNTHeaders.OptionalHeader.AddressOfEntryPoint + 
		m_hdNTHeaders.OptionalHeader.ImageBase;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CPEParser::GetImageBase(void) const
{
	Assert(m_fOpen);
	Assert(m_hdNTHeaders.Signature == IMAGE_NT_SIGNATURE);
	return m_hdNTHeaders.OptionalHeader.ImageBase;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CSection::CSection() : m_ulRVA(0),m_ulSize(0),m_ulFlags(0),m_lpszName(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////

CSection::~CSection()
{
	if(m_lpszName)
	{
		delete m_lpszName;
	}
}

///////////////////////////////////////////////////////////////////////////////

CSection::CSection(	unsigned long ulRVA,unsigned long ulSize,
				   unsigned long ulFlags,LPCSTR lpszName)
{
	m_ulRVA=ulRVA;
	m_ulSize=ulSize;
	m_ulFlags=ulFlags;
	m_lpszName = NULL;

	// Name can be 8 characters long without term null or else shorter
	// with a term null

	Assert(lpszName);
	if(NULL == lpszName)
	{
		return;
	}

	int i=0;
	while(i<MAX_CB_SECTION_NAME && !(lpszName[i] == 0))
	{
		i++;
	}
	
	size_t cch = i; // Either way this is the length of the string(A)
    
	m_lpszName = new TCHAR[cch+1];
	if(NULL == m_lpszName)
	{
		AssertSz(0,"Out of Memory");
		return;
	}

	for(unsigned int i=0;i<cch;i++)
	{
		m_lpszName[i] = (TCHAR)lpszName[i]; // ANSI To TCHAR
	}
	m_lpszName[cch] = 0; // NULL Term
}

///////////////////////////////////////////////////////////////////////////////

LPCTSTR CSection::GetName(void) const			
{ 
	return m_lpszName; 
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CSection::GetRVA(void) const	
{ 
	return m_ulRVA; 
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CSection::GetSize(void) const	
{ 
	return m_ulSize; 
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CSection::GetFlags(void) const	
{ 
	return m_ulFlags;
}

///////////////////////////////////////////////////////////////////////////////

bool CSection::IsExecutable() const
{ 
	return ((m_ulFlags & IMAGE_SCN_MEM_EXECUTE)==IMAGE_SCN_MEM_EXECUTE)
		? true : false; 
} 

///////////////////////////////////////////////////////////////////////////////

bool CSection::ContainsRVA (unsigned long ulRVA) const
{ 
	return ((m_ulRVA <= ulRVA) && ((m_ulRVA + m_ulSize) >= ulRVA))
		? true : false; 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CExportFunction::CExportFunction() : m_lpszName(NULL),m_lpszForwarder(NULL) 
{
}

///////////////////////////////////////////////////////////////////////////////

CExportFunction::CExportFunction(unsigned long ulOrdinal,unsigned long ulRVA,
				LPCSTR lpszName,bool fForwarder,LPCSTR lpszForwarder)
{
	m_lpszName = NULL;
	m_lpszForwarder = NULL;

	m_ulOrdinal = ulOrdinal;
	m_ulRVA = ulRVA;
	m_fForwarder = fForwarder;

	if(lpszName)
	{
		size_t cch;
		if(!SUCCEEDED(StringCchLengthA(lpszName,MAX_CCH_EXPORT_NAME,&cch)))
		{
			return;
		}

		m_lpszName = new TCHAR[cch+1];
		Assert(m_lpszName);
		if(NULL == m_lpszName)
		{
			return;
		}

		for(unsigned int i=0;i<cch;i++)
		{
			m_lpszName[i] = (TCHAR)lpszName[i]; // ANSI TO T String
		}
		m_lpszName[cch] = 0; // NULL Term
	}

	if(lpszForwarder)
	{
		size_t cch;
		if(!SUCCEEDED(StringCchLengthA(lpszForwarder,MAX_CCH_FORWARDER_NAME,&cch)))
		{
			return;
		}

		m_lpszForwarder = new TCHAR[cch+1];
		Assert(m_lpszForwarder);
		if(NULL == m_lpszForwarder)
		{
			return;
		}

		for(unsigned int i=0;i<cch;i++)
		{
			m_lpszForwarder[i] = (TCHAR)lpszForwarder[i]; // ANSI TO T String
		}
		m_lpszForwarder[cch] = 0; // NULL Term
	}
}

///////////////////////////////////////////////////////////////////////////////

CExportFunction::~CExportFunction()
{
	if(m_lpszName){delete m_lpszName;}
	if(m_lpszForwarder){delete m_lpszForwarder;}
}

///////////////////////////////////////////////////////////////////////////////

LPCTSTR CExportFunction::GetName() const
{
	return m_lpszName;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CExportFunction::GetOrdinal() const
{
	return m_ulOrdinal;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CExportFunction::GetRVA() const
{
	return m_ulRVA;
}

///////////////////////////////////////////////////////////////////////////////

bool CExportFunction::IsForwarder() const
{
	return m_fForwarder;
}

///////////////////////////////////////////////////////////////////////////////

LPCTSTR CExportFunction::GetForwarder() const
{
	return m_lpszForwarder;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

CImportModule::CImportModule() : m_lpszName(NULL) , 
	m_pvecImportFunctions(NULL),m_fDelayLoad(false)
{
}

///////////////////////////////////////////////////////////////////////////////

CImportModule::CImportModule(LPCSTR lpszName, bool fDelayLoad)
{
	m_lpszName = NULL;
	m_pvecImportFunctions = NULL;
	m_fDelayLoad = fDelayLoad;

	Assert(lpszName);
	if(NULL == lpszName)
	{
		return;
	}

	size_t cch;
	if(!SUCCEEDED(StringCchLengthA(lpszName,MAX_CCH_IMPORT_MODULE,&cch)))
	{
		return;
	}

	m_lpszName = new TCHAR[cch+1];
	Assert(lpszName);
	if(NULL == lpszName)
	{
		return;
	}

	for(unsigned int i=0;i<cch;i++)
	{
		m_lpszName[i] = (TCHAR)lpszName[i]; // ANSI TO T String
	}
	m_lpszName[cch] = 0; // NULL Term		
}

///////////////////////////////////////////////////////////////////////////////

CImportModule::~CImportModule()
{
	if(m_lpszName){delete m_lpszName;}
	if(m_pvecImportFunctions){delete m_pvecImportFunctions;}
}

///////////////////////////////////////////////////////////////////////////////

LPCTSTR CImportModule::GetName() const
{
	return m_lpszName;
}

///////////////////////////////////////////////////////////////////////////////

bool CImportModule::IsDelayLoad() const
{
	return m_fDelayLoad;
}

///////////////////////////////////////////////////////////////////////////////

PIMPORT_FUNCTIONS_VECTOR CImportModule::GetImportFunctions()
{
	// Created on demand

	if(NULL == m_pvecImportFunctions)
	{
		m_pvecImportFunctions = new IMPORT_FUNCTIONS_AUTO_VECTOR;
		Assert(m_pvecImportFunctions);
		if(NULL == m_pvecImportFunctions)
		{
			PEDasmSetLastError(idsErrMemory);
			return NULL;
		}
	}

	return m_pvecImportFunctions->GetVector();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

CImportFunction::CImportFunction() : m_lpszName(NULL),m_ulOrdinal(0),
	m_ulRVA(0)
{
}

///////////////////////////////////////////////////////////////////////////////

CImportFunction::CImportFunction(LPSTR lpszName,unsigned long ulOrdinal,
				unsigned long ulRVA)
{
	m_lpszName = NULL;
	m_ulOrdinal = ulOrdinal;
	m_ulRVA = ulRVA;

	if(NULL != lpszName)
	{
		size_t cch;
		if(!SUCCEEDED(StringCchLengthA(lpszName,MAX_CCH_IMPORT_FUNCTION,&cch)))
		{
			return;
		}

		m_lpszName = new TCHAR[cch+1];
		Assert(lpszName);
		if(NULL == lpszName)
		{
			return;
		}

		for(unsigned int i=0;i<cch;i++)
		{
			m_lpszName[i] = (TCHAR)lpszName[i]; // ANSI TO T String
		}
		m_lpszName[cch] = 0; // NULL Term		
	}
}

///////////////////////////////////////////////////////////////////////////////

CImportFunction::~CImportFunction()
{
	if(m_lpszName)
	{
		delete m_lpszName;
	}
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CImportFunction::GetOrdinal() const
{
	return m_ulOrdinal;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CImportFunction::GetRVA() const
{
	return m_ulRVA;
}

///////////////////////////////////////////////////////////////////////////////

LPCTSTR CImportFunction::GetName() const
{
	return m_lpszName;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long glLastError = 0;

///////////////////////////////////////////////////////////////////////

unsigned long PEDasmGetLastError(void)
	{
	return glLastError;
	}

///////////////////////////////////////////////////////////////////////

void PEDasmSetLastError(unsigned long lLastError)
	{
	glLastError = lLastError;
	}

///////////////////////////////////////////////////////////////////////

void PEDasmGetErrorString(unsigned long err, LPTSTR lpszErr, size_t cchErr)
{
	// LoadString(ghInst,err,lpszErr,(int)cchErr);
	// Assert(FALSE);
	// TODO:
}

///////////////////////////////////////////////////////////////////////