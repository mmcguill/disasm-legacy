/*******************************************************************/
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

#include <windows.h>

#define WM_DASM_ENGINE_UPDATE	(WM_USER + 0xDA5)

/////////////////////////////////////////////////////////////////////

typedef void *HDISASM;

/*******************************************************************/

HDISASM CreateDisasm(BOOL fUse32, const PBYTE pucImage,
					const ULONG ulSize,const ULONG ulImageBaseAddress,
					BOOL fIntelligentDasm);

/*******************************************************************/

void CloseDisasm(HDISASM hDasm);

/*******************************************************************/

BOOL WIN_DisassembleEPs(HDISASM hDasm, HWND hWndUpdate);

/*******************************************************************/

BOOL DisassembleEPs(HDISASM hDasm);

/*******************************************************************/

PLISTINGS_MANAGER GetListings_Manager(HDISASM hDasm);

/*******************************************************************/

PSYMBOL_TABLE GetSymTable(HDISASM hDasm);

/*******************************************************************/