/*******************************************************************/

#include <stdio.h>
#include <string>
#include <set>
#include <map>
#include <vector>
#include "..\..\inc\types.h"
#include "..\..\inc\ia32dsm.h"
#include "..\common\lists.h"
#include "..\..\inc\debug.h"
#include "..\..\inc\util.h"
#include "..\..\inc\symbols.h"
#include "..\common\dasm_driver.h"

// Needed for PEDasm!

#include <windows.h>
#include "..\..\inc\pedasm.h"

/*******************************************************************/

static BOOL gfHideAddress = FALSE;
static BOOL gfHideInsBytes = FALSE;
static BOOL gfUse32 = FALSE;
static BOOL gfDumbDriver = FALSE;
static BOOL gfPEDasm = FALSE;
static BOOL gfNoDisAsm = FALSE;
static BOOL gfPrintExp = FALSE;
static BOOL gfPrintImp = FALSE;

/*******************************************************************/

struct cmd_switch
{
	const CHAR ch;
	PBOOL pf;
	const CHAR *desc;
};

/*******************************************************************/

cmd_switch switch_map[] = 
{
	{'a',&gfHideAddress,"Hide Addresses"},
	{'b',&gfHideInsBytes,"Hide Instruction Bytes"},
	{'u',&gfUse32,"32 bit disassembly (USE32)"},
	{'d',&gfDumbDriver,"Use Dumb Disassembly Driver "
						"(Don't Follow Jumps etc.)"},
	{'p',&gfPEDasm,	"Disassemble this file as a Portable "
							"Executable (PE)"},
	{'n',&gfNoDisAsm,"Don't Disassemble (Info only)"},
	/*{'o=<Offset (Hex (0x) or Decimal>',
		&gfOffset,
		"Offset within file to start disassembly."},*/
	{'e',&gfPrintExp,"Print Exports (PE File only)"},
	{'i',&gfPrintImp,"Print Imports (PE File only)"},
};

#define NO_OF_SWITCHES sizeof(switch_map) / sizeof(switch_map[0])

/*******************************************************************/

const int MAX_REPEAT_BYTE_COUNT = 3;

/*******************************************************************/

INT main(INT argc, CHAR** argv)
{
	// Do Command Line Processing...

	if(argc < 2)
	{
		printf(	"McGuill Command Line Disassembler\n"
			"usage: %s [options] input_file\nOptions:\n", argv[0]);

		for(INT i=0;i<NO_OF_SWITCHES;i++)
		{
			printf("-%c   %s\n",
				switch_map[i].ch,switch_map[i].desc);
		}

		return 1;
	}

	INT cSwitches = 0;
	while(argv[cSwitches+1][0] == '-') /*  switches */
	{
		cSwitches++;

		INT len = (INT)strlen(argv[cSwitches]);

		BOOL fFoundSwitch = FALSE;
		for(INT i=1;i<len;i++)
		{
			for(INT j=0;j<NO_OF_SWITCHES;j++)
			{
				if(argv[cSwitches][i] == switch_map[j].ch)
				{
					fFoundSwitch = TRUE;

					/*  TURN ON SWITCH */					
					(*switch_map[j].pf) = TRUE;

					break;
				}
			}

			if(!fFoundSwitch)
			{
				fprintf(stderr,"Error: unknown switch: -%c", argv[1][i]);
				return 1;
			}
		}
	}
	
	
	// Load File...

	INT file_arg_index = cSwitches+1;
	BOOL fIsPE = FALSE;

	PBYTE pBuf = NULL;
	size_t len = 0;
	ULONG imageBase = 0;
	ULONG start = 0;

	CPEParser pe;
	
	// If imports or exports request set PE Flag

	if(gfPrintExp || gfPrintImp)
	{
		gfPEDasm = TRUE;
	}

	if(!gfPEDasm)
	{
		LONG filelen = 0;
		FILE *fp = fopen(argv[file_arg_index],"rb");
		if (NULL == fp)
		{
			fprintf(stderr,"Error opening file...\n");
			return 0;
		}

		fseek(fp,0,SEEK_END);
		filelen = ftell(fp);
		fseek(fp,0,SEEK_SET);

		if(-1 == filelen)
		{
			fprintf(stderr,"Error seeking file...\n");
			return 0;
		}
		if(0 == filelen)
		{
			fprintf(stderr,"Zero Byte File...\n");
			return 0;
		}

		pBuf = new BYTE[filelen];
		if(NULL == pBuf)
		{
			fprintf(stderr,"Out of Memory...\n");
			fclose(fp);
			return 0;
		}

		len = fread(pBuf,1,filelen,fp);
		fclose(fp);
	}
	else
	{
		if(!pe.Open(argv[file_arg_index]))
		{
			fprintf(stderr,"Could not load this file as a PE file...\n");
			return 0;
		}

		// Auto Switch disassembler to 32 bit mode, as file is PE
		// and PE contains 32 bit code.
		
		fIsPE = TRUE;
		gfUse32 = TRUE;
		
		pBuf = pe.GetImage();
		len = pe.GetImageSize();
		imageBase = pe.GetImageBase();
		start = pe.GetEntryPoint();
	}

	
	// Do Imports and Exports

	if(gfPrintImp)
	{
		printf("IMPORTS:\n");
		PIMPORT_MODULES_VECTOR pimv = pe.GetImportModulesVector();
	
		if(NULL != pimv)
		{
			for(IMPORT_MODULES_VECTOR::iterator mod = pimv->begin();
				mod != pimv->end();mod++)
			{
				if((*mod)->IsDelayLoad())
				{
					printf( "[%s (Delay Loaded)]\n"
							"IAT LOC    Import\n"
							"-------    ------\n",
							(*mod)->GetName());
				}
				else
				{
					printf(	"[%s]\nIAT LOC    Import\n"
							"-------    ------\n",
							(*mod)->GetName());
				}

				PIMPORT_FUNCTIONS_VECTOR pifv = (*mod)->GetImportFunctions();
				if(NULL != pifv)
				{
					char szFuncOrd[256];
					for(IMPORT_FUNCTIONS_VECTOR::iterator func = pifv->begin();
						func != pifv->end();func++)
					{
						LPCTSTR lpszFN = (*func)->GetName();
						if(NULL == lpszFN)
						{
							sprintf(szFuncOrd,"[Ord: %.4Xh]",(*func)->GetOrdinal());
							lpszFN = szFuncOrd;
						}

						printf("%.8X   %s\n",(*func)->GetRVA()+imageBase,lpszFN);
					}
				}

				printf("\n\n");
			}
			printf("\n\n");
		}
	}


	// Exports

	if(gfPrintExp)
	{
		printf("EXPORTS:\n");
		PEXPORTS_VECTOR pev = pe.GetExportsVector();
		
		if(NULL != pev)
		{
			for(EXPORTS_VECTOR::iterator exp = pev->begin();
				exp != pev->end();exp++)
			{
				LPCTSTR lpszFwd = (*exp)->GetForwarder();
				LPCTSTR lpszName = (*exp)->GetName();

				printf("%.8X   [Ord: %.4Xh]",	(*exp)->GetRVA()+imageBase,
												(*exp)->GetOrdinal());

				if(lpszName)
				{
					printf("   %s",lpszName);
				}
				if(lpszFwd)
				{
					printf(" --> %s",lpszFwd);
				}

				printf("\n");
			}
		}

		printf("\n\n");
	}


	// May not even have to disasm...

	if(gfNoDisAsm)
	{
		return 1;
	}

	// Start Disassembly...

	CIA32Dasm disEngine;

	// Load File and Prepare for dasm///

	if(!disEngine.Load(gfUse32,pBuf,(ULONG)len,imageBase,!gfDumbDriver))
	{
		fprintf(stderr,"Could not Disassemble this file.");
		return 0;
	}

	// Start Dasm

	if(gfPEDasm)
	{
		// PE => so use preloaded symbols, exports and entrypoints!

		if (!pe.LoadSymbols(disEngine.GetSymbolTable()) ||
            !disEngine.DasmFromEPs())
		{
			fprintf(stderr,"Could not Disassemble this file.");
			return 0;
		}
	}
	else
	{
		if(!disEngine.Dasm(start))
		{
			fprintf(stderr,"Could not Disassemble this file.");
			return 0;
		}
	}
	

	// Get Listings and display

	PLISTINGS pll = disEngine.GetListingsManager()->AcquireListings();
	
	printf("Disassembly Successful...\n");

	
	// Analyse and Add Symbols where possible

	PSYMBOLS_MAP pSymMap = NULL;
	if(gfPEDasm)
	{
		PSYMBOL_TABLE pSymTab = disEngine.GetSymbolTable();
		pSymMap = pSymTab->GetSymbolsMap();
		Assert(pSymMap);
	}


	if(pSymMap)
	{
		printf("SYMBOLS:\n");

		for(SYMBOLS_MAP::iterator sym = pSymMap->begin();
			sym != pSymMap->end();sym++)
		{
			printf("%.8X   %s",	sym->second->GetAddress(),
											sym->second->GetName());

			printf("\n");
		}

		printf("\n\n");
	}


	// Print Listings

	std::string strTemp;
	
	for(LISTINGS::iterator theListing = pll->begin();
		theListing != pll->end();theListing++)
	{
		PLISTING pl = (*theListing);

		if(ListingData == pl->GetListingType())
		{
			// Make Sure this is a pe file!

			Assert(fIsPE);
			if(!fIsPE)
			{
				continue;
			}

			printf("***********************************************************\n");
			//printf("DATA BLOCK: [%0.8X] -> [%0.8X]\n",
			//	pl->GetStartAddress(),pl->GetEndAddress());

			ULONG end = pl->GetEndAddress();
			ULONG base = pe.GetImageBase();
			PBYTE image = pe.GetImage();

			for(ULONG ulAddr = pl->GetStartAddress();ulAddr < end + 1;ulAddr++)
			{
				ULONG index = ulAddr - base;
				BYTE b = image[index];

				if(!gfHideAddress)
				{
					printf("%0.8X  ", ulAddr);
				}

				// Do a quick scan ahead and see if this byte is repeated
				// more than x times, if so do short hand rather than line
				// by line 'DB x DUP (Byte)'

				BOOL fDiff = FALSE;
				ULONG t1 = ulAddr + 1;
				while(!fDiff && t1 < (end + 1))
				{
					BYTE b2 = image[t1 - base];	

					fDiff = (b != b2);
					t1++;
				}

				int len = (t1 - ulAddr);

				
				// because we've overrun one

				if(t1 != (end + 1))
				{
					len--;
				}

				
				if(len > MAX_REPEAT_BYTE_COUNT)
				{
					printf("DB %d DUP (%0.2Xh)\n",len,b);
					ulAddr += len - 1;
				}
				else
				{
					if(isprint(b))
					{
						printf("DB %0.2X ; '%c'\n",b,b);
					}
					else
					{
						printf("DB %0.2X \n",b);
					}
				}
			}
		}
		else if(ListingCode == pl->GetListingType())
		{
			printf("***********************************************************\n");

			PCODE_LISTING pcl = (PCODE_LISTING)pl;
			PINSTRUCTION_MAP pim = pcl->GetInstructionMap();
			
			if(NULL == pim)
			{
				continue;
			}

			for(std::map<ULONG, PINSTRUCTION>::iterator theIns = pim->begin();
				theIns != pim->end();theIns++)
			{
				std::pair<ULONG, PINSTRUCTION> ins = *theIns;
				SYMBOLS_MAP::iterator symbol;
				BOOL fSymbolFound = FALSE;
				PINSTRUCTION pInst = ins.second;

				if(NULL != pSymMap)
				{
					// Convert Addresses to Symbols where possible
					
					if (CIA32ProcessorDriver::IsJumpInstruction(pInst->m_Mnemonic))
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
								
								
								// Check For Indirect Symbol here

								symbol = pSymMap->find(ulPtr);

								if (symbol != pSymMap->end()/* &&
									(symbol_indirect == symbol->second->GetType())*/)
								{
									fSymbolFound = TRUE;
								}
								else if(	
									ulPtr >= imageBase &&
									ulPtr <= ((	imageBase + len)-4))
								{
									ulJumpEA = 
										*(ULONG*)&(pBuf[ulPtr - imageBase]);
								}
							}
						}

						
						// Look for direct symbol if not found an indirect one

						if(!fSymbolFound)
						{
							symbol = pSymMap->find(ulJumpEA);
							if	(symbol != pSymMap->end()/* &&
								(symbol_indirect != symbol->second->GetType())*/)
							{
								fSymbolFound = TRUE;
							}
						}
					}
				}
			


				/*  address  */
				
				if(!gfHideAddress)
				{
					printf("%0.8X  ", ins.first);
				}


				/*  bytes  */

				PINSTRUCTION pIns = ins.second;

				INT cbInsBytes = pIns->m_cbInstruction;
				const PBYTE pInsb = ins.second->m_ucInstruction;
				
				CHAR szBuf[256] = "\0";
				CHAR szTmp[256];

				if(!gfHideInsBytes)
				{
					for(INT i=0;i<cbInsBytes;i++)
					{
						sprintf(szTmp,"%0.2X",pInsb[i]);
						strcat(szBuf,szTmp);
					}

					printf("%s",szBuf);

					INT len = (INT)strlen(szBuf);
					for(INT i=0;i<18 - len;i++)
					{
						printf(" ");
					}
				}
				

				strTemp = pIns->m_szMnemonic;
					
				if(0 != pIns->m_cOperands)
				{
					strTemp += " ";
					BOOL fFirst = TRUE;
					
					for(ULONG i= 0;i<pIns->m_cOperands;i++)
					{
						if(!fFirst)
						{
							strTemp += ", ";
						}
						else
						{
							fFirst = FALSE;
						}

						if(fSymbolFound)
						{
							strTemp += symbol->second->GetName();
						}
						else
						{
							strTemp += pIns->m_rgszOperand[i];
						}
					}
				}
					
				printf("%s\n", strTemp.c_str());
			}
		}
		else
		{
			// WTF?
			Assert(0);
		}
	}

	if(!fIsPE)
	{
		delete[] pBuf;
	}

	disEngine.GetListingsManager()->ReleaseListings();
	return 1;
}

/*******************************************************************/