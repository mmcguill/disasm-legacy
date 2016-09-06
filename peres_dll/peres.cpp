///////////////////////////////////////////////////////////////////////////////
// CPERes written and designed by Mark McGuill 30/6/99 11:52 PM         //
// Edited Again: 14/03/2002 15:33											 //
//                                                                           //
// Revision History:                                                         //
//														                     //
// 27/06/04 =>	Converting to Dll and calling CPERes because its not my //
//				Greatest work! Im building a new one now and im leaving this //
//				in a dll because it requires a string table that i dont		 // 
//				wanna dirty my new apps with							     //
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tchar.h>
#include <winnls.h>
#include <stdlib.h>
#include <malloc.h>
#include "resource.h"
#include "..\..\inc\peres.h"

///////////////////////////////////////////////////////////////////////////////

HINSTANCE ghInstance = NULL;

///////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(	HANDLE hModule, DWORD  ul_reason_for_call, 
						LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ghInstance = (HINSTANCE)hModule;
		DisableThreadLibraryCalls((HMODULE)hModule);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

////////////////////////////////////////////////////////////////
// CPERes 

BOOL CPERes::FileLoad(const LPTSTR lpszFilename, UINT* pnIDError)
{
	// Just in Casey
	FileClose(); 

	// Map File
	m_hFile = CreateFile(	lpszFilename,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							(HANDLE)0);

	if (m_hFile == INVALID_HANDLE_VALUE){
		*pnIDError = IDS_PEFILE_ERROR_CREATEFILE;
		return FALSE;
	}

	m_hFileMapping = CreateFileMapping(m_hFile,NULL,PAGE_READONLY,0,0,NULL);
	if (m_hFileMapping == 0){
		*pnIDError = IDS_PEFILE_ERROR_CREATEFILE;
		return FALSE;
	}
	
	m_pFileData = (LPBYTE)MapViewOfFile(m_hFileMapping,FILE_MAP_READ,0,0,0);
	if (m_pFileData == 0){
		*pnIDError = IDS_PEFILE_ERROR_CREATEFILE;
		return FALSE;
	}

	// Load Only PE
	if(*(WORD*)m_pFileData != AUTH_DOS_SIG){
		*pnIDError = IDS_PEFILE_ERROR_NODOSSIG;
		return FALSE; 
	}

	if(IsBadReadPtr(NTSIGNATURE(m_pFileData),SIZE_OF_NT_SIGNATURE)){
		*pnIDError = IDS_PEFILE_ERROR_NOPESIG;
		return FALSE;
	}

	if(*(DWORD*)NTSIGNATURE(m_pFileData) != AUTH_PE_SIG) {
		*pnIDError = IDS_PEFILE_ERROR_NOPESIG;
		return FALSE; 
	}
	
	// Init Member Vars
//	_tcsncpy(m_szFilename,lpszFilename,MAX_PATH);

	//if(!__InitMembers()){
	//	*pnIDError = IDS_PEFILE_ERROR_INITMEMBERS;
	//	return FALSE;
	//}
	//m_bValidFile = TRUE;

	if(!InfGetOptHeader(&m_ioh)) return FALSE;

	// IFH
	if(!InfGetHeader(&m_ifh)) return FALSE;

	// No. of Sections

	m_dwNumSect = InfGetNumSections();

	if(m_dwNumSect == 0)
	{
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

//BOOL CPERes::__InitMembers()
//{
//	// File Size
//	//m_dwFileSize = GetFileSize(m_hFile,NULL);
//	//if(m_dwFileSize == 0xFFFFFFFF) return FALSE;
//
//	// IOH
//	//
//
//	//// IFH
//	//if(!InfGetHeader(&m_ifh)) return FALSE;
//
//	//// No. of Sections
//	//m_dwNumSect = InfGetNumSections();
//	//if(m_dwNumSect == 0)
//	//{
//	//	return FALSE;
//	//}
//
//	//if(!LoadImports())
//	//{
//	//	return FALSE;
//	//}
//
//	return TRUE;
//}

///////////////////////////////////////////////////////////////////////////////

//BOOL CPERes::LoadImports()
//{
//	m_pImpTab = new CImportTable;
//	ASSERT(m_pImpTab);
//
//	int numMods = ImpGetNumModules();
//
//	TCHAR szModule[MAX_MODULE_NAME] = _T("\0");
//	TCHAR szFunction[MAX_FUNCTION_NAME] = _T("\0");
//	DWORD dwAddr = 0;
//	
//
//	for(int x=0;x<numMods;x++)
//	{
//		ImpGetModuleName(x, szModule);
//		
//		int numFuncs = 0;
//		if(!ImpGetNumFuncsMod(x,(DWORD*)&numFuncs))
//		{
//			ASSERT(FALSE);
//			return FALSE;
//		}
//
//		for(int y=0;y<numFuncs;y++)
//		{
//			LPTSTR lpszFunction = szFunction;
//			BOOL fFnIsOrdinal = FALSE;
//
//			if(!ImpGetFuncName(x,y,szFunction))
//			{
//				DWORD dwOrd;
//				if(ImpGetFuncOrd(x,y,&dwOrd))
//				{
//					lpszFunction = (LPTSTR)(DWORD_PTR)dwOrd;
//					fFnIsOrdinal = TRUE;
//				}
//				else
//				{
//					return FALSE;
//				}
//			}
//
//			dwAddr = ImpGetFuncIAT(x,y);
//			m_pImpTab->Add(szModule,lpszFunction,fFnIsOrdinal,dwAddr);
//		}
//	}
//
//	return TRUE;
//}

///////////////////////////////////////////////////////////////////////////////

void CPERes::FileClose()
{
	if(m_pFileData)						UnmapViewOfFile(m_pFileData);
	if(m_hFileMapping)					CloseHandle(m_hFileMapping);
	if(m_hFile != INVALID_HANDLE_VALUE)	CloseHandle(m_hFile);
	//if(m_pImpTab) delete m_pImpTab;

	//
	m_pFileData =	NULL;
	m_hFileMapping=	NULL;
	m_hFile =		NULL;
	//m_pImpTab =     NULL;

	//m_szFilename[0] = _T('\0');
	//m_dwFileSize = 0;
	//m_dwNumSect = 0;

	//memchr(&m_ioh,0,sizeof(IMAGE_OPTIONAL_HEADER));
	//memchr(&m_ifh,0,sizeof(IMAGE_FILE_HEADER));
	//m_bValidFile = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// Header Funcs
//BOOL CPERes::InfGetMSDOSHeader(PIMAGE_DOS_HEADER pIDH) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	PIMAGE_DOS_HEADER pActIDH= (PIMAGE_DOS_HEADER)m_pFileData;
//	if(IsBadReadPtr(pActIDH,sizeof(IMAGE_DOS_HEADER))) return FALSE;
//	CopyMemory(pIDH,pActIDH,sizeof(IMAGE_DOS_HEADER));
//	return TRUE;
//}
//

BOOL CPERes::InfGetHeader(PIMAGE_FILE_HEADER pIFH) const
{
	if(m_pFileData == NULL) return FALSE;

	PIMAGE_FILE_HEADER pActIFH = (PIMAGE_FILE_HEADER) PEFHDROFFSET(m_pFileData);
	if(IsBadReadPtr(pActIFH,sizeof(IMAGE_FILE_HEADER))) return FALSE;
	CopyMemory(pIFH,pActIFH,sizeof(IMAGE_FILE_HEADER));
	return TRUE;
}


BOOL CPERes::InfGetOptHeader(PIMAGE_OPTIONAL_HEADER pIOH) const
{
	if(m_pFileData == NULL) return FALSE;

	PIMAGE_OPTIONAL_HEADER pActIOH= (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(m_pFileData);
	if(IsBadReadPtr(pActIOH,sizeof(IMAGE_OPTIONAL_HEADER))) return FALSE;
	CopyMemory(pIOH,pActIOH,sizeof(IMAGE_OPTIONAL_HEADER));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
//// Section Funcs
BOOL CPERes::InfGetSectionHeader(PIMAGE_SECTION_HEADER pISH,const DWORD dwSect) const
{
	if(m_pFileData == NULL) return FALSE;
	if(dwSect >= m_dwNumSect) return FALSE;

	PIMAGE_SECTION_HEADER pActISH = (PIMAGE_SECTION_HEADER)SECHDROFFSET(m_pFileData);
	if(pActISH == NULL) return FALSE;

	pActISH+=dwSect;
	if(IsBadReadPtr(pActISH,sizeof(IMAGE_SECTION_HEADER))) return FALSE;

	CopyMemory(pISH,pActISH,sizeof(IMAGE_SECTION_HEADER));
	return TRUE;
}
//
//BOOL CPERes::InfGetSectionName(const DWORD dwSect,LPTSTR lpszName)
//{
//	if(m_pFileData == NULL) return FALSE;
//	if(dwSect < 0 || dwSect >= m_dwNumSect) return FALSE;
//
//	IMAGE_SECTION_HEADER ISH;
//	if(!InfGetSectionHeader(&ISH,dwSect)) return FALSE;
//	
//	// Copy only up to 8 chars
//
//	TCHAR szFormat[MAX_PATH];
//	if(!LoadString(ghInstance,IDS_FORMAT_SECTION_NAME,szFormat,MAX_PATH))
//	{
//		return FALSE;
//	}
//
//	// ASCII To T
//	
//	int cch = (int)strlen((char*)ISH.Name);
//	LPTSTR lpTmp = NULL;
//
//#ifdef UNICODE
//	lpTmp = (LPTSTR)malloc((cch+1) * sizeof(TCHAR));
//	MultiByteToWideChar(CP_ACP,0,(LPCSTR)ISH.Name,cch,lpTmp,cch+1);
//#else
//	lpTmp = (char*)ISH.Name;
//#endif
//
//	wsprintf(lpszName,szFormat,lpTmp); 
//
//#ifdef UNICODE
//	free(lpTmp);
//#endif
//
//	return TRUE;
//}
//
//DWORD CPERes::InfGetSectionSize(const DWORD dwSect) const
//{
//	if(m_pFileData == NULL) return FALSE;
//	
//	IMAGE_SECTION_HEADER ish;
//	if(!InfGetSectionHeader(&ish,dwSect)) return 0;
//	return __MMCG_MIN(ish.SizeOfRawData,ish.Misc.VirtualSize);
//}
//
//BOOL CPERes::InfGetCodeSection(DWORD* pdwSect) const
//{
//	if(m_pFileData == NULL) return FALSE;
//	DWORD dwCodeBase = InfGetCodeBase();
//	if(dwCodeBase == 0) return FALSE;
//
//	BOOL bFound = FALSE;
//	IMAGE_SECTION_HEADER ish;
//
//	// BUG Here: look attring to open test-resources app
//
//	for(DWORD i=0;i<m_dwNumSect && !bFound;i++){
//		if(InfGetSectionHeader(&ish,i) && ish.VirtualAddress == dwCodeBase)	bFound = TRUE;                              
//	}
//
//	if(!bFound) return FALSE;
//	*pdwSect = --i;
//	return TRUE;
//}
//
//BOOL CPERes::InfGetSectionByName(const LPTSTR lpszName, DWORD* pdwSect)
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	TCHAR szCurSect[32];
//	BOOL bFound = FALSE;
//	for (DWORD i=0;i<m_dwNumSect && !bFound;i++){
//		if(!InfGetSectionName(i,szCurSect)) return FALSE;
//		bFound = (_tcscmp(szCurSect,lpszName) == 0);
//	}
//
//	if(!bFound) return FALSE;
//	*pdwSect = --i;
//	return TRUE;
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////
//// Data Functions
//BOOL CPERes::DataGetSection(BYTE* pData,const DWORD dwSect,const DWORD dwSize) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	IMAGE_SECTION_HEADER ish;
//	if(!InfGetSectionHeader(&ish,dwSect)) return FALSE;
//	return DataGetRawBytes(pData,ish.PointerToRawData,dwSize);
//}
//
//BOOL CPERes::DataGetRawBytes(BYTE* pData,const DWORD dwOffset,const DWORD dwSize) const
//{
//	if(m_pFileData == NULL) return FALSE;
//	if(IsBadReadPtr(m_pFileData + dwOffset,dwSize)) return FALSE;
//	
//	CopyMemory(pData,m_pFileData + dwOffset,dwSize);
//	return TRUE;
//}
//
////////////////////////////////////////////////////
//// Import Modules/Function Functions
//
//DWORD CPERes::ImpGetNumModules() const
//{
//	if(m_pFileData == NULL) return FALSE;
//	
//	//Get First Mod
//	PIMAGE_IMPORT_DESCRIPTOR pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//
//	DWORD dwCount = 0;
//	do{ // this will = 0 at after last module name
//		if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return dwCount;
//		pid++;
//		dwCount++;
//	}while(pid->Name);
//	return dwCount;
//}
//
//BOOL CPERes::ImpGetModuleName(const DWORD dwMod, LPTSTR lpszName) const
//{
//	if(m_pFileData == NULL) return FALSE;
//	
//	// Get First Image Imp Desc 
//	PIMAGE_IMPORT_DESCRIPTOR pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//
//	// Move To wanted Module
//	pid+=dwMod; 
//	if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//	if(!pid->Name) return FALSE;
//	
//	char* pMod;
//	if(!__InfGetFilePtrFromRva(pid->Name,0,(BYTE**)&pMod,TRUE)) return FALSE;
//	if(IsBadStringPtrA(pMod,MAX_MODULE_NAME)) return FALSE;
//
//	TCHAR szMod[MAX_MODULE_NAME];
//#ifdef UNICODE
//		MultiByteToWideChar(CP_ACP,0,pMod,-1,szMod,MAX_MODULE_NAME);
//#else
//		_tcsncpy(szMod,pMod,MAX_MODULE_NAME);
//#endif
//
//	lstrcpyn(lpszName,szMod,MAX_MODULE_NAME + 1);
//
//	return TRUE;
//}
//
//BOOL CPERes::ImpGetModuleByName(const LPTSTR lpszModName, DWORD* pdwMod) const
//{
//	if(m_pFileData == NULL) return FALSE;
//	
//	// Get Relative Offset with in import section body
//	PIMAGE_IMPORT_DESCRIPTOR pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//	
//	// Check
//	if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//	if(!pid->Name) return FALSE;
//	
//	// Search
//	DWORD i = 0;
//	BOOL bFound = FALSE;
//	do{
//		char* pMod;
//		if(!__InfGetFilePtrFromRva(pid->Name,0,(BYTE**)&pMod,TRUE)) return FALSE;
//		if(IsBadStringPtrA(pMod,MAX_MODULE_NAME))
//		{
//			return FALSE;
//		}
//
//		TCHAR szTmp[MAX_MODULE_NAME];
//
//#ifdef UNICODE
//		MultiByteToWideChar(CP_ACP,0,pMod,-1,szTmp,MAX_MODULE_NAME);
//#else
//		_tcsncpy(szTmp,pMod,MAX_MODULE_NAME);
//#endif
//
//		bFound = (_tcscmp(lpszModName,szTmp) == 0);
//		
//		// Update
//		i++;pid++;
//		if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//	}while(!bFound && pid->Name);
//	if(!bFound) return FALSE;
//
//	*pdwMod = --i;
//	return TRUE;
//}
//
//BOOL CPERes::ImpGetNumFuncsMod(const DWORD dwMod, DWORD* pdwNumFuncs) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	// Get Relative Offset with in import section body
//	PIMAGE_IMPORT_DESCRIPTOR pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//	pid+=dwMod; 
//	if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//
//	// Get ITD DWORD
//	PIMAGE_THUNK_DATA pITD;
//	if(!__InfGetFilePtrFromRva((pid->OriginalFirstThunk) ? (pid->OriginalFirstThunk) : (pid->FirstThunk),0,(BYTE**)&pITD)) return FALSE;
//		
//	// Count Funcs
//	DWORD dwCount = 0;
//	if(IsBadReadPtr(pITD,sizeof(IMAGE_THUNK_DATA))) return FALSE;
//	while((DWORD)pITD->u1.AddressOfData){
//		dwCount++;
//		pITD++;
//		if(IsBadReadPtr(pITD,sizeof(IMAGE_THUNK_DATA))) return FALSE;
//	}
//		
//	*pdwNumFuncs = dwCount;
//	return TRUE;
//}
//
//BOOL CPERes::ImpGetFuncName(const DWORD dwMod, const DWORD dwFunc, LPTSTR lpszName)
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	// Get First Module
//	PIMAGE_IMPORT_DESCRIPTOR		pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//	
//	// Move to this Module
//	pid+=dwMod;
//	if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//
//	PIMAGE_THUNK_DATA pITD;
//	if(!__InfGetFilePtrFromRva((pid->OriginalFirstThunk) ? (pid->OriginalFirstThunk) : (pid->FirstThunk),0,(BYTE**)&pITD)) return FALSE;
//	
//	// Move to this function
//	pITD+=dwFunc;
//	if(IsBadReadPtr(pITD,sizeof(IMAGE_THUNK_DATA))) return FALSE;
//
//	//Check if its ordinal not name
//	if((DWORD)(pITD->u1.Ordinal & IMAGE_ORDINAL_FLAG32)){
//			lpszName[0] = _T('\0');
//		return FALSE;
//	}
//
//	// Copy String
//	PIMAGE_IMPORT_BY_NAME pIIBN;
//	if(!__InfGetFilePtrFromRva((DWORD)pITD->u1.AddressOfData,0,(BYTE**)&pIIBN,TRUE)) return FALSE;
//	if(IsBadStringPtrA((char*)pIIBN->Name,MAX_FUNCTION_NAME)) return FALSE;
//
//	TCHAR szFunc[MAX_FUNCTION_NAME];
//
//#ifdef UNICODE
//		MultiByteToWideChar(CP_ACP,0,(char*)pIIBN->Name,-1,szFunc,MAX_FUNCTION_NAME);
//#else
//		_tcsncpy(szFunc,(char*)pIIBN->Name,MAX_FUNCTION_NAME);
//#endif
//
//	lstrcpyn(lpszName,szFunc,MAX_FUNCTION_NAME + 1);
//	return TRUE;
//}
//
//BOOL CPERes::ImpGetFuncOrd(const DWORD dwMod,const DWORD dwFunc,DWORD* pdwOrd) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	PIMAGE_IMPORT_DESCRIPTOR		pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//		
//	// Move to this module
//	pid+=dwMod;
//	if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//
//	PIMAGE_THUNK_DATA pITD;
//	if(!__InfGetFilePtrFromRva((pid->OriginalFirstThunk) ? (pid->OriginalFirstThunk) : (pid->FirstThunk),0,(BYTE**)&pITD)) return FALSE;
//	
//	// Move to this function
//	pITD+=dwFunc; //Move To wanted Function
//	if(IsBadReadPtr(pITD,sizeof(IMAGE_THUNK_DATA))) return FALSE;
//	
//	if(pITD->u1.Ordinal & IMAGE_ORDINAL_FLAG32){
//		*pdwOrd = IMAGE_ORDINAL(pITD->u1.Ordinal);
//		return TRUE;
//	}
//	return FALSE; 
//}
//
//DWORD CPERes::ImpGetFuncIAT(const DWORD dwMod, const DWORD dwFunc) const
//{
//	if(m_pFileData == NULL) return FALSE;
//	
//	PIMAGE_IMPORT_DESCRIPTOR		pid;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_IMPORT,(BYTE**)&pid)) return FALSE;
//
//	//Move To This module
//	pid+=dwMod;
//	if(IsBadReadPtr(pid,sizeof(IMAGE_IMPORT_DESCRIPTOR))) return FALSE;
//
//	DWORD dwIAT = (DWORD)(DWORD_PTR)((PIMAGE_THUNK_DATA*)(DWORD_PTR)(pid->FirstThunk + dwFunc));
//	if((InfGetImageBase() + dwIAT) > InfGetTotalImageSize()){//Microsoft Optimisation - Binding
//		return dwIAT;
//	}
//	else{//Non MS optimised
//		return InfGetImageBase() + dwIAT;
//	}
////	return 0; 
//}
//
////////////////////////////////////////////////////
//// Export Modules/Function Functions
//DWORD CPERes::ExpGetNumFuncs() const
//{
//	if(m_pFileData == NULL) return 0;
//
//	PIMAGE_EXPORT_DIRECTORY		ped;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_EXPORT,(BYTE**)&ped)) return 0;
//	if(IsBadReadPtr(ped,sizeof(IMAGE_EXPORT_DIRECTORY))) return 0;
//
//	return ped->NumberOfFunctions;
//}
//
//BOOL CPERes::ExpGetFuncOrdinal(const DWORD dwFunc, DWORD* pdwOrd) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	PIMAGE_EXPORT_DIRECTORY		ped;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_EXPORT,(BYTE**)&ped)) return FALSE;
//	if(IsBadReadPtr(ped,sizeof(IMAGE_EXPORT_DIRECTORY))) return FALSE;
//	if(dwFunc > ped->NumberOfFunctions) return FALSE;
//	
//	*pdwOrd = dwFunc + ped->Base;
//	return TRUE;
//}
//
//BOOL CPERes::ExpGetFuncName(const DWORD dwFunc, LPTSTR lpszName) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	// Get Export Directory
//	PIMAGE_EXPORT_DIRECTORY		ped;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_EXPORT,(BYTE**)&ped)) return FALSE;
//	if(IsBadReadPtr(ped,sizeof(IMAGE_EXPORT_DIRECTORY))) return FALSE;
//	if(dwFunc > ped->NumberOfFunctions) return FALSE;
//	
//	// Check if its in NameOrdinals table
//	WORD* pNameOrds;
//	if(!__InfGetFilePtrFromRva(ped->AddressOfNameOrdinals,0,(BYTE**)&pNameOrds)) return FALSE;
//	if(IsBadReadPtr( pNameOrds,(sizeof(WORD) * ped->NumberOfNames) ) ) return FALSE;
//	
//	// Search
//	BOOL bFound = FALSE;
//	for(DWORD x=0;x<ped->NumberOfNames && !bFound;x++){
//		if(dwFunc == *(pNameOrds + x)) bFound = TRUE;
//	}
//	if(!bFound) return FALSE; // Nope
//	x--; // Yep
//
//	// Get Name Table		
//	DWORD* pdwNameRvas;
//	if(!__InfGetFilePtrFromRva(ped->AddressOfNames,0,(BYTE**)&pdwNameRvas)) return FALSE;
//	//Move to current func with in name table index we found earlier
//	pdwNameRvas += x; 
//	if(IsBadReadPtr(pdwNameRvas,sizeof(DWORD))) return FALSE;
//
//	// Get RVA and use to get name
//	char* pszFName;
//	if(!__InfGetFilePtrFromRva(*pdwNameRvas,0,(BYTE**)&pszFName)) return FALSE;
//	if(IsBadStringPtrA(pszFName,MAX_FUNCTION_NAME)) return FALSE;
//
//	TCHAR szFunc[MAX_FUNCTION_NAME];
//
//#ifdef UNICODE
//		MultiByteToWideChar(CP_ACP,0,pszFName,-1,szFunc,MAX_FUNCTION_NAME);
//#else
//		_tcsncpy(szFunc,pszFName,MAX_FUNCTION_NAME);
//#endif
//
//	lstrcpyn(lpszName,szFunc,MAX_FUNCTION_NAME + 1);
//	return TRUE;
//}
//
//BOOL CPERes::ExpGetFuncEntryPoint(const DWORD dwFunc, DWORD* pdwEntry) const
//{
//	if(m_pFileData == NULL) return FALSE;
//
//	PIMAGE_EXPORT_DIRECTORY		ped;
//	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_EXPORT,(BYTE**)&ped)) return FALSE;
//	if(IsBadReadPtr(ped,sizeof(IMAGE_EXPORT_DIRECTORY))) return FALSE;
//	if(dwFunc > ped->NumberOfFunctions) return FALSE;
//
//	// Get Ordinal
//	DWORD dwOrd;
//	if(!ExpGetFuncOrdinal(dwFunc, &dwOrd)) return FALSE;
//
//	// Subtract base ordinal
//	dwOrd -= ped->Base;
//		
//	// Get Pointer to Address of Functions
//	DWORD* pFunc;
//	if(!__InfGetFilePtrFromRva(ped->AddressOfFunctions,0,(BYTE**)&pFunc)) return FALSE;
//	// Check OK on one we want
//	pFunc+=dwOrd;
//	if(IsBadReadPtr(pFunc,sizeof(DWORD))) return FALSE;
//	
//	// Grand
//	*pdwEntry = *pFunc;
//	return TRUE;
//}
//
/////////////////////////////////////////////////////
// Resource Functions

DWORD CPERes::ResGetRootDir() const
{
	if(m_pFileData == NULL) return 0;

	PIMAGE_RESOURCE_DIRECTORY		prd;
	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_RESOURCE,(BYTE**)&prd)) return 0;
	return (DWORD)(DWORD_PTR)prd;
}

BOOL CPERes::ResGetNumObj(const DWORD dwDir, DWORD* pdwNumObj) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;

	PIMAGE_RESOURCE_DIRECTORY prd = (PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir;
	if(IsBadReadPtr(prd,sizeof(IMAGE_RESOURCE_DIRECTORY))) return FALSE;

	*pdwNumObj = prd->NumberOfNamedEntries + prd->NumberOfIdEntries;
	return TRUE;
}

BOOL CPERes::ResGetObjName(const DWORD dwDir, const DWORD dwObj,LPTSTR lpszName) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pirde= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir+1); //the 1 moves it to the directory entries
	pirde+=dwObj;
	if(IsBadReadPtr(pirde,sizeof(PIMAGE_RESOURCE_DIRECTORY_ENTRY))) return FALSE;

	// Get Name
	char szName[MAX_RESOURCE_NAME];
	if(pirde->Name & IMAGE_RESOURCE_NAME_IS_STRING)
	{
		//Need To Work Base Of Resources
		DWORD dwBase;
		if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_RESOURCE,(BYTE**)&dwBase)) return FALSE;
				
		PIMAGE_RESOURCE_DIR_STRING_U pirdsu = (PIMAGE_RESOURCE_DIR_STRING_U)(DWORD_PTR)((int)pirde->Id + (int)dwBase);
		int nCh = WideCharToMultiByte(CP_ACP,0,pirdsu->NameString,pirdsu->Length,szName,MAX_PATH,NULL,NULL);
		szName[nCh]=0;

#ifdef UNICODE
		MultiByteToWideChar(CP_ACP,0,szName,-1,lpszName,MAX_PATH);
#else
		_tcsncpy(lpszName,szName,MAX_PATH);
#endif

		return TRUE;
	}
	
	return FALSE;
}

BOOL CPERes::ResGetObjID(const DWORD dwDir, const DWORD dwObj, DWORD* pdwID) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pirde= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir+1); //the 1 moves it to the directory entries
	pirde+=dwObj;
	if(IsBadReadPtr(pirde,sizeof(PIMAGE_RESOURCE_DIRECTORY_ENTRY))) return FALSE;
	
	// Get ID
	if(pirde->Name & IMAGE_RESOURCE_NAME_IS_STRING) return FALSE;

	*pdwID = pirde->Id;
	return TRUE;
}

BOOL CPERes::ResIsObjDir(const DWORD dwDir, const DWORD dwObj) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pirde= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir+1); //the 1 moves it to the directory entries
	pirde+=dwObj;
	if(IsBadReadPtr(pirde,sizeof(PIMAGE_RESOURCE_DIRECTORY_ENTRY))) return FALSE;

	return (pirde->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY);
}

BOOL CPERes::ResGetDirObj(const DWORD dwDir, const DWORD dwObj, DWORD* pdwDirObj) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pirde= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir+1); //the 1 moves it to the directory entries
	
	pirde+=dwObj;
	if(IsBadReadPtr(pirde,sizeof(PIMAGE_RESOURCE_DIRECTORY_ENTRY))) return FALSE;
	
	//Got Entry, now get directory location and return it
	DWORD dwBase;
	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_RESOURCE,(BYTE**)&dwBase)) return FALSE;
	PIMAGE_RESOURCE_DIRECTORY prd = (PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)((int)pirde->OffsetToDirectory + (int)dwBase);
	if(IsBadReadPtr(prd,sizeof(PIMAGE_RESOURCE_DIRECTORY))) return FALSE;

	*pdwDirObj = (DWORD)(DWORD_PTR)prd;
	return TRUE;
}

BOOL CPERes::ResGetObjDataSize(const DWORD dwDir,const DWORD dwObj, DWORD* pdwSize) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pirde= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir+1); //the 1 moves it to the directory entries
	pirde+=dwObj;
	if(IsBadReadPtr(pirde,sizeof(PIMAGE_RESOURCE_DIRECTORY_ENTRY))) return FALSE;

	// Make sure its not a directory
	if(pirde->DataIsDirectory) return FALSE;
	//Got Entry, now get data entry
	DWORD dwBase;
	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_RESOURCE,(BYTE**)&dwBase)) return FALSE;
	PIMAGE_RESOURCE_DATA_ENTRY pirDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(DWORD_PTR)((int)pirde->OffsetToData + (int)dwBase);
	if(IsBadReadPtr(pirDataEntry,sizeof(PIMAGE_RESOURCE_DATA_ENTRY))) return FALSE;

	*pdwSize = pirDataEntry->Size;
	return TRUE;
}

BOOL CPERes::ResGetObjData(const DWORD dwDir,const DWORD dwObj,BYTE* pData) const
{
	if(m_pFileData == NULL) return FALSE;
	if(!dwDir) return FALSE;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pirde= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PIMAGE_RESOURCE_DIRECTORY)(DWORD_PTR)dwDir+1); //the 1 moves it to the directory entries
	pirde+=dwObj;
	if(IsBadReadPtr(pirde,sizeof(PIMAGE_RESOURCE_DIRECTORY_ENTRY))) return FALSE;
	
	//make sure it aint a dir
	if(pirde->DataIsDirectory) return FALSE;

	// Got Entry, now get data entry location
	DWORD dwBase;
	if(!__InfGetFilePtrFromRva(0,IMAGE_DIRECTORY_ENTRY_RESOURCE,(BYTE**)&dwBase)) return FALSE;
	
	PIMAGE_RESOURCE_DATA_ENTRY pirDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(DWORD_PTR)((int)pirde->OffsetToData + (int)dwBase);
	if(IsBadReadPtr(pirDataEntry,sizeof(PIMAGE_RESOURCE_DATA_ENTRY))) return FALSE;

	// Copy Data
	BYTE* pAddData;
	if(!__InfGetFilePtrFromRva(pirDataEntry->OffsetToData,0,(BYTE**)&pAddData)) return FALSE;

	if(IsBadReadPtr(pAddData,pirDataEntry->Size)) return FALSE;
	CopyMemory(pData,pAddData,pirDataEntry->Size);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////
// Private Internals

// 16/03/2002 - Weird Borland Delta
// Special check for weird rare borland linked (possibly TLINK32)
// Exe and dlls. First of all size of raw data and virtual size 
// fields are reversed. Then if there is a discardable section 
// previous to this import section we have to recalculate the 
// pointer to raw data!! Believe it or not. 
// Also this only affects pointers which access actual data,
// IMAGE_THUNK_DATA, IMAGE_IMPORT_DESCRIPTOR and other header parts
// remain unaffected.This may affect disassembly i'm not fully clear on 
// what effect this ridiculous pseudo-optimisation has on other sections. 
// Mark McGuill. 

// The Main Man
BOOL CPERes::__InfGetFilePtrFromRva(DWORD dwRVA,const DWORD dwDataDirectory,BYTE** ppbyteOffset,const BOOL bCheckWeirdBorlandDelta /*=FALSE*/) const
{
	//Check Whether user wants us to use dwRVA or DataDirectory
	if(dwRVA == 0 && dwDataDirectory < m_ioh.NumberOfRvaAndSizes) 
		dwRVA = m_ioh.DataDirectory[dwDataDirectory].VirtualAddress;

	//Locate Section
	BOOL bFound = FALSE;
	IMAGE_SECTION_HEADER ISH = {0};
	for(DWORD i=0;i<m_dwNumSect && !bFound;i++){
		if(!InfGetSectionHeader(&ISH,i)) return FALSE;
		if((ISH.VirtualAddress <= dwRVA) && (ISH.VirtualAddress + __MMCG_MAX(ISH.SizeOfRawData,ISH.Misc.VirtualSize) > dwRVA)){
			bFound = TRUE;
		}
	}
	if (!bFound) return FALSE;

	// Found Section, now calculate Borland delta and file offset
	// Weird Borland Delta - See top of import functions section in this file
	DWORD dwWeirdBorlandDelta = 0;
	if(bCheckWeirdBorlandDelta){
		DWORD dwSect = --i;
		IMAGE_SECTION_HEADER ishPrev;
		if(IS_BORLAND_LINK_STYLE(ISH)){
			if(InfGetSectionHeader(&ishPrev,dwSect-1) && (ishPrev.Characteristics & IMAGE_SCN_MEM_DISCARDABLE)){
				dwWeirdBorlandDelta = (ISH.VirtualAddress - ISH.PointerToRawData) - (ishPrev.VirtualAddress - ishPrev.PointerToRawData);
			}
		}
	}

	// Return Happy as larry
	*ppbyteOffset = m_pFileData + ISH.PointerToRawData + dwRVA - ISH.VirtualAddress + dwWeirdBorlandDelta;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Language and Resource Strings

BOOL CPERes::ResGetLangStringFromID(const DWORD dwID,LPTSTR lpszLang)
{
	TCHAR szFormat[MAX_PATH];

	switch(PRIMARYLANGID(dwID))
	{
	case LANG_NEUTRAL:
		LoadString(ghInstance,IDS_RES_LANG_NEUTRAL,lpszLang,MAX_PATH);
		break;
	case LANG_ARABIC:
		LoadString(ghInstance,IDS_RES_LANG_ARABIC,lpszLang,MAX_PATH);
		break;
	case LANG_AFRIKAANS:
		LoadString(ghInstance,IDS_RES_LANG_AFRIKAANS,lpszLang,MAX_PATH);
		break;
	case LANG_ALBANIAN:
		LoadString(ghInstance,IDS_RES_LANG_ALBANIAN,lpszLang,MAX_PATH);
		break;
	case LANG_BASQUE:
		LoadString(ghInstance,IDS_RES_LANG_BASQUE,lpszLang,MAX_PATH);
		break;
	case LANG_BULGARIAN:
		LoadString(ghInstance,IDS_RES_LANG_BULGARIAN,lpszLang,MAX_PATH);
		break;
	case LANG_CATALAN:
		LoadString(ghInstance,IDS_RES_LANG_CATALAN,lpszLang,MAX_PATH);
		break;
	case LANG_CHINESE:
		LoadString(ghInstance,IDS_RES_LANG_CHINESE,lpszLang,MAX_PATH);
		break;
	case LANG_CROATIAN:
		LoadString(ghInstance,IDS_RES_LANG_CROATIAN,lpszLang,MAX_PATH);
		break;
	case LANG_CZECH:
		LoadString(ghInstance,IDS_RES_LANG_CZECH,lpszLang,MAX_PATH);
		break;
	case LANG_DANISH:
		LoadString(ghInstance,IDS_RES_LANG_DANISH,lpszLang,MAX_PATH);
		break;
	case LANG_DUTCH:
		LoadString(ghInstance,IDS_RES_LANG_DUTCH,lpszLang,MAX_PATH);
		break;
	case LANG_ENGLISH:
		switch(SUBLANGID(dwID))
		{
		case SUBLANG_ENGLISH_US:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_US,lpszLang,MAX_PATH);
			break;
		case SUBLANG_ENGLISH_UK:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_UK,lpszLang,MAX_PATH);
			break;
		case SUBLANG_ENGLISH_AUS:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_AUS,lpszLang,MAX_PATH);
			break;
		case SUBLANG_ENGLISH_CAN:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_CAN,lpszLang,MAX_PATH);
			break;
		case SUBLANG_ENGLISH_NZ :
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_NZ,lpszLang,MAX_PATH);
			break;
		case SUBLANG_ENGLISH_EIRE:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_EIRE,lpszLang,MAX_PATH);
			break;
		case SUBLANG_ENGLISH_JAMAICA:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH_JAMAICA,lpszLang,MAX_PATH);
			break;
		default:
			LoadString(ghInstance,IDS_RES_LANG_ENGLISH,lpszLang,MAX_PATH);
			break;
		}
		break;
	case LANG_ESTONIAN:
		LoadString(ghInstance,IDS_RES_LANG_ESTONIAN,lpszLang,MAX_PATH);
		break;
	case LANG_FAEROESE:
		LoadString(ghInstance,IDS_RES_LANG_FAEROESE,lpszLang,MAX_PATH);
		break;
	case LANG_FARSI:
		LoadString(ghInstance,IDS_RES_LANG_FARSI,lpszLang,MAX_PATH);
		break;
	case LANG_FINNISH:
		LoadString(ghInstance,IDS_RES_LANG_FINNISH,lpszLang,MAX_PATH);
		break;
	case LANG_FRENCH:
		LoadString(ghInstance,IDS_RES_LANG_FRENCH,lpszLang,MAX_PATH);
		break;
	case LANG_GERMAN:
		LoadString(ghInstance,IDS_RES_LANG_GERMAN,lpszLang,MAX_PATH);
		break;
	case LANG_GREEK:
		LoadString(ghInstance,IDS_RES_LANG_GREEK,lpszLang,MAX_PATH);
		break;
	case LANG_HEBREW:
		LoadString(ghInstance,IDS_RES_LANG_HEBREW,lpszLang,MAX_PATH);
		break;
	case LANG_HUNGARIAN:
		LoadString(ghInstance,IDS_RES_LANG_HUNGARIAN,lpszLang,MAX_PATH);
		break;
	case LANG_ICELANDIC:
		LoadString(ghInstance,IDS_RES_LANG_ICELANDIC,lpszLang,MAX_PATH);
		break;
	case LANG_INDONESIAN:
		LoadString(ghInstance,IDS_RES_LANG_INDONESIAN,lpszLang,MAX_PATH);
		break;
	case LANG_ITALIAN:
		LoadString(ghInstance,IDS_RES_LANG_ITALIAN,lpszLang,MAX_PATH);
		break;
	case LANG_JAPANESE:
		LoadString(ghInstance,IDS_RES_LANG_JAPANESE,lpszLang,MAX_PATH);
		break;
	case LANG_KOREAN:
		LoadString(ghInstance,IDS_RES_LANG_KOREAN,lpszLang,MAX_PATH);
		break;
	case LANG_LATVIAN:
		LoadString(ghInstance,IDS_RES_LANG_LATVIAN,lpszLang,MAX_PATH);
		break;
	case LANG_LITHUANIAN:
		LoadString(ghInstance,IDS_RES_LANG_LITHUANIAN,lpszLang,MAX_PATH);
		break;
	case LANG_NORWEGIAN:
		LoadString(ghInstance,IDS_RES_LANG_NORWEGIAN,lpszLang,MAX_PATH);
		break;
	case LANG_POLISH:
		LoadString(ghInstance,IDS_RES_LANG_POLISH,lpszLang,MAX_PATH);
		break;
	case LANG_PORTUGUESE:
		LoadString(ghInstance,IDS_RES_LANG_PORTUGUESE,lpszLang,MAX_PATH);
		break;
	case LANG_ROMANIAN:
		LoadString(ghInstance,IDS_RES_LANG_ROMANIAN,lpszLang,MAX_PATH);
		break;
	case LANG_RUSSIAN:
		LoadString(ghInstance,IDS_RES_LANG_RUSSIAN,lpszLang,MAX_PATH);
		break;
	case LANG_SLOVAK:
		LoadString(ghInstance,IDS_RES_LANG_SLOVAK,lpszLang,MAX_PATH);
		break;
	case LANG_SLOVENIAN:
		LoadString(ghInstance,IDS_RES_LANG_SLOVENIAN,lpszLang,MAX_PATH);
		break;
	case LANG_SPANISH:
		LoadString(ghInstance,IDS_RES_LANG_SPANISH,lpszLang,MAX_PATH);
		break;
	case LANG_SWEDISH:
		LoadString(ghInstance,IDS_RES_LANG_SWEDISH,lpszLang,MAX_PATH);
		break;
	case LANG_THAI:
		LoadString(ghInstance,IDS_RES_LANG_THAI,lpszLang,MAX_PATH);
		break;
	case LANG_TURKISH:
		LoadString(ghInstance,IDS_RES_LANG_TURKISH,lpszLang,MAX_PATH);
		break;
	case LANG_UKRAINIAN:
		LoadString(ghInstance,IDS_RES_LANG_UKRAINIAN,lpszLang,MAX_PATH);
		break;
	default:
		LoadString(ghInstance,IDS_RES_LANG_FORMAT_UNKNOWN,szFormat,MAX_PATH);
		wsprintf(lpszLang,szFormat,dwID);
		return FALSE;
		break;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CPERes::ResGetTypeStringFromID(const DWORD dwID,LPTSTR lpszType)
{
	TCHAR szFormat[MAX_PATH];

	switch(dwID)
	{
	case RT_CURSOR:
		LoadString(ghInstance,IDS_RES_TYPE_CURSOR,lpszType,MAX_PATH);
		break;
	case RT_BITMAP:      
		LoadString(ghInstance,IDS_RES_TYPE_BITMAP,lpszType,MAX_PATH);
		break;
	case RT_ICON:
		LoadString(ghInstance,IDS_RES_TYPE_ICON,lpszType,MAX_PATH);
		break;
	case RT_MENU:        
		LoadString(ghInstance,IDS_RES_TYPE_MENU,lpszType,MAX_PATH);
		break;
	case RT_DIALOG:           
		LoadString(ghInstance,IDS_RES_TYPE_DIALOG,lpszType,MAX_PATH);
		break;
	case RT_STRING:         
		LoadString(ghInstance,IDS_RES_TYPE_STRING,lpszType,MAX_PATH);
		break;
	case RT_FONTDIR:
		LoadString(ghInstance,IDS_RES_TYPE_FONTDIR,lpszType,MAX_PATH);
		break;
	case RT_FONT:  
		LoadString(ghInstance,IDS_RES_TYPE_FONT,lpszType,MAX_PATH);
		break;
	case RT_ACCELERATOR:
		LoadString(ghInstance,IDS_RES_TYPE_ACCELERATOR,lpszType,MAX_PATH);
		break;
	case RT_RCDATA:
		LoadString(ghInstance,IDS_RES_TYPE_RCDATA,lpszType,MAX_PATH);
		break;
	case RT_MESSAGETABLE:
		LoadString(ghInstance,IDS_RES_TYPE_MESSAGETABLE,lpszType,MAX_PATH);
		break;
	case RT_GROUP_CURSOR:
		LoadString(ghInstance,IDS_RES_TYPE_GROUPCURSOR,lpszType,MAX_PATH);
		break;
	case RT_GROUP_ICON:
		LoadString(ghInstance,IDS_RES_TYPE_GROUPICON,lpszType,MAX_PATH);
		break;
	case RT_VERSION:
		LoadString(ghInstance,IDS_RES_TYPE_VERSION,lpszType,MAX_PATH);
		break;
	case RT_DLGINCLUDE:
		LoadString(ghInstance,IDS_RES_TYPE_DLGINCLUDE,lpszType,MAX_PATH);
		break;
	case RT_PLUGPLAY:
		LoadString(ghInstance,IDS_RES_TYPE_PLUGPLAY,lpszType,MAX_PATH);
		break;
	case RT_VXD:
		LoadString(ghInstance,IDS_RES_TYPE_VXD,lpszType,MAX_PATH);
		break;
	case RT_ANICURSOR:
		LoadString(ghInstance,IDS_RES_TYPE_ANICURSOR,lpszType,MAX_PATH);
		break;
	case RT_ANIICON:
		LoadString(ghInstance,IDS_RES_TYPE_ANIICON,lpszType,MAX_PATH);
		break;
	case RT_HTML:
		LoadString(ghInstance,IDS_RES_TYPE_HTML,lpszType,MAX_PATH);
		break;
	case RT_MANIFEST:
		LoadString(ghInstance,IDS_RES_TYPE_MANIFEST,lpszType,MAX_PATH);
		break;
	default:
		LoadString(ghInstance,IDS_RES_TYPE_FORMAT_UNKNOWN,szFormat,MAX_PATH);
		wsprintf(lpszType,szFormat,dwID);
		return FALSE;
		break;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

//BOOL CPERes::IsValidObj()
//{
//	return m_bValidFile;
//}

///////////////////////////////////////////////////////////////////////////////

//BOOL CPERes::ImportFuncFromAddress(DWORD dwIndAdd, PIMPORT_TABLE_ENTRY pite)
//{
//	ASSERT(pite);
//
//	if(m_pFileData == NULL)
//	{
//		return FALSE;
//	}
//
//	IMAGE_SECTION_HEADER ISH;
//	PIMAGE_IMPORT_DESCRIPTOR pid;
//
//	//Get Relative Offset with in import section body
//
//	DWORD dwSect;
//	pid=(PIMAGE_IMPORT_DESCRIPTOR)InfGetImageDataDirectoryOffset(IMAGE_DIRECTORY_ENTRY_IMPORT,&dwSect);
//	
//	if(pid == NULL)
//	{
//		return FALSE;
//	}
//	
//	if(!InfGetSectionHeader(&ISH,dwSect))
//	{
//		return FALSE;
//	}
//
//	DWORD dwBase = InfGetImageBase();
//	DWORD nFirstThunkRva = dwIndAdd - (dwBase + ISH.VirtualAddress);
//	DWORD *pdwLookup = (DWORD*)((BYTE*)(pid) + nFirstThunkRva); //import by name rva
//
//	if(!IsBadReadPtr(pdwLookup,sizeof(DWORD)))
//	{
//		DWORD dwLookup = *pdwLookup;
//		if(!m_pImpTab->GetImportEntryFromAddr(dwLookup, pite))
//		{
//			pdwLookup = (DWORD*)(m_pFileData + (dwIndAdd - dwBase));
//			if(!IsBadReadPtr(pdwLookup,sizeof(DWORD)))
//			{ //try see if its direct
//				dwLookup = *pdwLookup;
//			
//				if(!m_pImpTab->GetImportEntryFromAddr(dwLookup, pite))
//				{
//					return FALSE;
//				}
//			}
//		}
//	}
//
//	return TRUE;
//}

///////////////////////////////////////////////////////////////////////////////

// TODO: see if this function can be replaced by __InfGetFilePtrFromRva?

//LPVOID CPERes::InfGetImageDataDirectoryOffset(DWORD dwDir, PDWORD pdwSect)
//{
//    if(m_pFileData == NULL) 
//	{
//		return FALSE;
//	}
//
//	PIMAGE_OPTIONAL_HEADER   poh;
//	PIMAGE_SECTION_HEADER    psh;
//
//	poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(m_pFileData);
//
//	if(IsBadReadPtr(poh,sizeof(IMAGE_OPTIONAL_HEADER)))
//	{
//		return NULL;
//	}
//
//	if (dwDir >= poh->NumberOfRvaAndSizes)
//	{
//		return NULL;
//	}
//
//	LPVOID VAImageDir = (LPVOID)(DWORD_PTR)poh->DataDirectory[dwDir].VirtualAddress;
//
//    // locate section containing image directory
//	BOOL bFound = FALSE;
//	int nSections = InfGetNumSections();
//	psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET(m_pFileData);
//	
//	for(int i=0;i<nSections && !bFound;i++,psh++)
//	{
//		if(IsBadReadPtr(psh,sizeof(IMAGE_SECTION_HEADER))) 
//		{
//			return NULL;
//		}
//		
//		if(psh->VirtualAddress <= (DWORD_PTR)VAImageDir && psh->VirtualAddress + psh->SizeOfRawData > (DWORD_PTR)VAImageDir)
//		{
//			bFound = TRUE;
//		}
//	}
//
//	if (bFound)
//	{
//		i--;
//		psh--;
//		
//		if(pdwSect != NULL)
//		{
//			*pdwSect=i;
//		}
//
//		unsigned char* pData = (unsigned char*)(LPVOID)((DWORD_PTR)m_pFileData + (int)psh->PointerToRawData + (((DWORD_PTR)VAImageDir) - (int)psh->VirtualAddress));
//
//		if(IsBadReadPtr(pData,sizeof(DWORD)))
//		{
//			return NULL;
//		}
//
//		return pData;
//	}
//
//	return NULL;
//}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CImportTable
//
//CImportTable::CImportTable()
//{
//	first = NULL;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//CImportTable::~CImportTable()
//{
//	DeleteRec(first);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CImportTable::DeleteRec(PIMPORT_TABLE_ENTRY pte)
//{
//	if(pte){
//		if(pte->next != NULL) DeleteRec(pte->next);
//		delete pte;
//	}
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CImportTable::Add(LPTSTR  pszModule, LPTSTR pszFuncName, BOOL fFnIsOrdinal, DWORD dwAddress)
//{
//	if(first == NULL)
//	{
//		first = new IMPORT_TABLE_ENTRY;
//
//		lstrcpy(first->szModule, pszModule);
//
//		if(!fFnIsOrdinal)
//		{
//            lstrcpy(first->szFuncName, pszFuncName);
//			first->fOrdOnly = FALSE;
//			first->dwOrd = 0;
//		}
//		else
//		{
//			first->dwOrd = (DWORD)(DWORD_PTR)pszFuncName;
//			first->fOrdOnly = TRUE;
//			first->szFuncName[0] = 0;
//		}
//
//		first->dwAddr = dwAddress;
//		first->next = NULL;
//	}
//	else
//	{
//		PIMPORT_TABLE_ENTRY ptemp = first;
//		
//		while(ptemp->next) ptemp = ptemp->next;
//
//		ptemp->next = new IMPORT_TABLE_ENTRY;
//		
//		lstrcpy(ptemp->next->szModule, pszModule);
//
//		if(!fFnIsOrdinal)
//		{
//            lstrcpy(ptemp->next->szFuncName, pszFuncName);
//			ptemp->next->fOrdOnly = FALSE;
//			ptemp->next->dwOrd = 0;
//		}
//		else
//		{
//			ptemp->next->dwOrd = (DWORD)(DWORD_PTR)pszFuncName;
//			ptemp->next->fOrdOnly = TRUE;
//			ptemp->next->szFuncName[0] = 0;
//		}
//		
//		ptemp->next->dwAddr = dwAddress;
//		ptemp->next->next = NULL;
//	}
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//BOOL CImportTable::GetImportEntryFromAddr(DWORD dwAddr, PIMPORT_TABLE_ENTRY pite)
//{
//	ASSERT(pite);
//	PIMPORT_TABLE_ENTRY ptemp = first;
//
//	while(ptemp)
//	{
//		if(ptemp->dwAddr == dwAddr)
//		{
//			pite->dwAddr = ptemp->dwAddr;
//			pite->next = ptemp->next;
//			pite->fOrdOnly = ptemp->fOrdOnly;
//			pite->dwOrd = ptemp->dwOrd;
//
//			lstrcpyn(pite->szFuncName,ptemp->szFuncName,MAX_FUNCTION_NAME);
//			lstrcpyn(pite->szModule,ptemp->szModule,MAX_MODULE_NAME);
//
//			return TRUE;
//		}
//		ptemp = ptemp->next;
//	}
//	return FALSE;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//PIMPORT_TABLE_ENTRY CImportTable::GetFirst()
//{
//	return first;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//PIMPORT_TABLE_ENTRY CImportTable::GetNext(PIMPORT_TABLE_ENTRY ptemp)
//{
//	ASSERT(ptemp);
//
//	if(ptemp)
//	{
//		ptemp = ptemp->next;
//		return ptemp;
//	}
//
//	return NULL;
//}
//
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
