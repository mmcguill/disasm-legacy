/*******************************************************************/
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

#include <map>
#include <vector>
#include <set>
#include "..\..\inc\types.h"
#include "..\..\inc\ia32dsm.h"
#include "..\..\inc\debug.h"
#include "..\..\inc\util.h"
#include "..\..\inc\symbols.h"
#include "..\common\lists.h"

#include "dasmeng.h"

#include "..\common\dasm_driver.h"

/*******************************************************************/

HDISASM CreateDisasm(BOOL fUse32, const PBYTE pucImage,
					const ULONG ulSize,const ULONG ulImageBaseAddress,
					BOOL fIntelligentDasm)
{
	CIA32Dasm* pDasm = new CIA32Dasm;
	Assert(pDasm);

	if(!pDasm->Load(fUse32,pucImage, ulSize,ulImageBaseAddress,fIntelligentDasm))
	{
		delete pDasm;
		return NULL;
	}

	return (HDISASM)pDasm;
}

/*******************************************************************/

void CloseDisasm(HDISASM hDasm)
{
	//OutputDebugString("CloseDisasm()\n");
	Assert(hDasm);

	CIA32Dasm *pDsm = (CIA32Dasm*)hDasm;

	if(NULL != pDsm)
	{
		delete pDsm;
	}
}

/*******************************************************************/

BOOL WIN_DisassembleEPs(HDISASM hDasm, HWND hWndUpdate)
{
	Assert(hDasm);
	if(NULL == hDasm)
	{
		return FALSE;
	}

	CIA32Dasm* pDasm = (CIA32Dasm*)hDasm;

	return pDasm->DasmFromEPsWithWndUpdate(hWndUpdate);
}

/*******************************************************************/

BOOL DisassembleEPs(HDISASM hDasm)
{
	Assert(hDasm);
	if(NULL == hDasm)
	{
		return FALSE;
	}

	CIA32Dasm* pDasm = (CIA32Dasm*)hDasm;

	return pDasm->DasmFromEPs();
}

/*******************************************************************/

PLISTINGS_MANAGER GetListings_Manager(HDISASM hDasm)
{
	Assert(hDasm);
    if(NULL == hDasm)
	{
		return FALSE;
	}

	CIA32Dasm* pDasm = (CIA32Dasm*)hDasm;

	return pDasm->GetListingsManager();
}

/*******************************************************************/

PSYMBOL_TABLE GetSymTable(HDISASM hDasm)
{
	Assert(hDasm);
    if(NULL == hDasm)
	{
		return FALSE;
	}

	CIA32Dasm* pDasm = (CIA32Dasm*)hDasm;

	return pDasm->GetSymbolTable();
}

/*******************************************************************/