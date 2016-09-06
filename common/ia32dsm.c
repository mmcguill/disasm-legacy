/*******************************************************************/
/* (c) Copyright 2004 - - Mark McGuill. All Rights Reserved.       */
/*                                                                 */
/* File: ia32dsm.c                                                 */
/* Owner: Mark McGuill                                             */
/* Create Date: 09/Sep/2004										   */
/*                                                                 */
/* Purpose: Implementation of Single Instruction Disassembler      */
/*			for Intel 80x86 compatible machine code                */
/*                                                                 */
/* Comments: File is fully ANSI C compliant, should compile under  */
/*           Windows, Linux, BSD etc, tested with VC, GCC, CC      */
/*                                                                 */
/*                                                                 */
/* Modified:                                                       */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "..\..\inc\types.h"
#include "..\..\inc\ia32dsm.h"
#include "instructions.h"
#include "..\..\inc\debug.h"

/*******************************************************************/

#define SIZEOF_TABLE(x) (sizeof(x) / sizeof(x[0]))

/*******************************************************************/

#define MAX_PREFIXES 16				/*  Maximum space for prefixes */
#define MAX_SZ_DISPLACEMENT 48
#define MAX_SZ_IMMEDIATE 48
#define MAX_SZ_EA 48
#define MAX_SZ_SIB 48

/*******************************************************************/

static LPCSTR kszEmpty =			"";
static LPCSTR kszBytePtrPrefix =	"byte ptr ";
static LPCSTR kszWordPtrPrefix =	"word ptr ";
static LPCSTR kszDwordPtrPrefix  =	"dword ptr ";
static LPCSTR kszFwordPtrPrefix  =	"fword ptr ";
static LPCSTR kszTwordPtrPrefix  =	"tword ptr ";
static LPCSTR kszQwordPtrPrefix  =	"qword ptr ";
static LPCSTR kszDqwordPtrPrefix =  "oword ptr ";

static LPCSTR kszBytePrefix =		"byte ";
static LPCSTR kszWordPrefix =		"word ";
static LPCSTR kszDwordPrefix  =		"dword ";

static LPCSTR kszRepPrefix = "rep ";
static LPCSTR kszRepePrefix = "repe ";
static LPCSTR kszRepnePrefix = "repne ";
static LPCSTR kszLockPrefix = "lock ";

/*******************************************************************/

typedef enum tagRegSet
{
	rsNone,
	rsGeneral,
	rsMMX,
	rsXMM,
	rsDebug,
	rsControl,
	rsSegment,
}RegSet;

/*******************************************************************/

typedef struct tagSEGOFF1632
{
	ULONG off;
	USHORT seg;
}SEGOFF1632, *PSEGOFF1632;

/*******************************************************************/

typedef struct tagSEGOFF1616
{
	USHORT off;
	USHORT seg;
}SEGOFF1616, *PSEGOFF1616;

/*******************************************************************/

typedef struct tagAM_DECODE_INFO
{
	PULONG	pcbEaten;
	PBYTE	pOperStream;
	PTABLE_INSTRUCTION pTabIns;
	INT		nOperand;
	LPSTR	lpszOperand;
	ULONG	ulInsAddress;
	ULONG	ulDirectJumpLoc;
}AM_DECODE_INFO,*PAM_DECODE_INFO;

/*******************************************************************/

typedef BOOL (*PFNADDRESSINGMODE_DECODER)(PAM_DECODE_INFO pdi);

/*******************************************************************/

typedef struct tagAM_DECODER
{
	AddressingMethodEnum am;
	PFNADDRESSINGMODE_DECODER pfnDecode;
}AM_DECODER, *PAM_DECODER;

/*******************************************************************/

#define TWO_BYTE_OPCODE_MASK 0x00000F00

/*******************************************************************/

BOOL BuildPrefixes(void);
BOOL BuildInstruction(PTABLE_INSTRUCTION pTabInst);
BOOL BuildOperands(PTABLE_INSTRUCTION pTabInst,ULONG ulAddress, 
				   PINSTRUCTION pInstRet);

/*******************************************************************/

BOOL IsExtensionGroupOpcode(ULONG ulOp, PINT pnGroup);
ULONG GetOpcode(void);
PBYTE GetModRMByte(void);
void MergeTableAndExtensionOpcodes(PTABLE_INSTRUCTION pTi, 
								   PTABLE_INSTRUCTION pTiExt);

LPCSTR GetHardCodedRegString(OperandTypeEnum otReg, INT nOperSize);
BOOL FormatJumpDisplacement(ULONG ulAddress,PBYTE pStream, 
							LPSTR lpszDisp,ULONG nBitSize, 
							PULONG pulDirectJumpLoc);

BOOL FormatDisplacement(PBYTE pStream, LPSTR lpszDisp,
						ULONG nBitSize);
BOOL FormatImmediate(PBYTE pStream, LPSTR lpszDisp,ULONG nBitSize);
BOOL LookupOperSizeDepIns(PTABLE_INSTRUCTION pIns,INT nOperSize);
BOOL LookupAddrSizeDepIns(PTABLE_INSTRUCTION pIns,INT nAddrSize);
BOOL LookupPrefixDepIns(PTABLE_INSTRUCTION pIns);
BOOL LookupModIs3DepIns(PTABLE_INSTRUCTION pIns,INT nMod);
BOOL GetEffectiveAddressString(INT nAddrSize, LPSTR lpszEA, 
							   RegSet regSet,PAM_DECODE_INFO pdi);


/* -------------------------- ModRM Functions -------------------- */

ULONG ModRM_GetExtOpcode(const BYTE modrm);
BOOL ModRM_HasSIB(INT nAddrSize, const BYTE modrm);
ULONG ModRM_GetModValue(const BYTE modrm);
ULONG ModRM_GetRegValue(const BYTE modrm);
ULONG ModRM_GetRMValue(const BYTE modrm);
BOOL ModRM_GetDisplacementString(INT nAddrSize,BYTE *pModrm, 
								 LPSTR lpszDisp,ULONG ulDispSize);
LPCSTR ModRM_GetEffectiveAddressOrRegString(INT nAddrSize, 
											const BYTE modrm, 
											INT nRegSize,  
											RegSet regSet);
LPCSTR ModRM_GetRegString(INT nRegSize, const BYTE modrm,RegSet rs);
LPCSTR ModRM_GetRegisterStringFromIndex(INT nRegSize, 
										RegSet regSet, INT index);


/* -------------------------- SIB Functions ---------------------- */

ULONG SIB_GetScale(const BYTE sib);
ULONG SIB_GetIndex(const BYTE sib);
ULONG SIB_GetBase(const BYTE sib);
LPCSTR SIB_GetIndexString(const BYTE sib, LPSTR lpszIndex);
LPCSTR SIB_GetSIBString(const BYTE sib, LPSTR lpszSIB);
LPCSTR SIB_GetBaseString(const BYTE sib);

/*******************************************************************/

BOOL AMDecode_A(PAM_DECODE_INFO pdi);
BOOL AMDecode_C(PAM_DECODE_INFO pdi);
BOOL AMDecode_D(PAM_DECODE_INFO pdi);
BOOL AMDecode_E(PAM_DECODE_INFO pdi);
BOOL AMDecode_G(PAM_DECODE_INFO pdi);
BOOL AMDecode_I(PAM_DECODE_INFO pdi);
BOOL AMDecode_J(PAM_DECODE_INFO pdi);
BOOL AMDecode_M(PAM_DECODE_INFO pdi);
BOOL AMDecode_O(PAM_DECODE_INFO pdi);
BOOL AMDecode_P(PAM_DECODE_INFO pdi);
BOOL AMDecode_Q(PAM_DECODE_INFO pdi);
BOOL AMDecode_R(PAM_DECODE_INFO pdi);
BOOL AMDecode_S(PAM_DECODE_INFO pdi);
BOOL AMDecode_T(PAM_DECODE_INFO pdi);
BOOL AMDecode_V(PAM_DECODE_INFO pdi);
BOOL AMDecode_W(PAM_DECODE_INFO pdi);
BOOL AMDecode_X(PAM_DECODE_INFO pdi);
BOOL AMDecode_Y(PAM_DECODE_INFO pdi);
BOOL AMDecode_FloatingPoint(PAM_DECODE_INFO pdi);
BOOL AMDecode_HardCodeFPStackReg(PAM_DECODE_INFO pdi);
BOOL AMDecode_HardCodeReg(PAM_DECODE_INFO pdi);
BOOL AMDecode_HardCodeNum(PAM_DECODE_INFO pdi);

/*******************************************************************/

/*static */ const BYTE krgPrefixes[] = 
	{
	0xF0,
	0xF2,
	0xF3,
	0x2E,
	0x36,
	0x3E,
	0x26,
	0x64,
	0x65,
	0x66,
	0x67,
	};

#define SIZEOF_PREFIXES SIZEOF_TABLE(krgPrefixes)

/*******************************************************************/

/*static*/ const AM_DECODER krgAMDecoders[] =
{
	{amA, AMDecode_A},
	{amC, AMDecode_C},
	{amD, AMDecode_D},
	{amE, AMDecode_E},
	{amG, AMDecode_G},
	{amI, AMDecode_I},
	{amJ, AMDecode_J},
	{amM, AMDecode_M},
	{amO, AMDecode_O},
	{amP, AMDecode_P},
	{amQ, AMDecode_Q},
	{amR, AMDecode_R},
	{amS, AMDecode_S},
	{amT, AMDecode_T},
	{amV, AMDecode_V},
	{amW, AMDecode_W},
	{amX, AMDecode_X},
	{amY, AMDecode_Y},
	{amFloatingPoint, AMDecode_FloatingPoint},
	{amHardCodeFPStackReg, AMDecode_HardCodeFPStackReg},
	{amHardCodeReg, AMDecode_HardCodeReg},
	{amHardCodeNum, AMDecode_HardCodeNum},
};

#define SIZEOF_AMDECODERS SIZEOF_TABLE(krgAMDecoders)

/*******************************************************************/

PBYTE	gpStream = NULL;
ULONG	gcbStream = 0;
BYTE	gPrefix[MAX_PREFIXES];
ULONG	gcPrefixes;
INT		gnSegPrefix = 0;
INT		gnRepPrefix = 0;
INT		gnLockPrefix = 0;
INT		gnAddrSize = 0;
INT		gnOperSize = 0;
INT		gcbEaten = 0;
BOOL	gfModRM = FALSE;
BOOL	gfUse32 = 0;
ULONG	gLastError = IA32DSM_ERROR_NONE;

/*******************************************************************/

ULONG IA32Dsm_GetLastError()
{
	return gLastError;
}

/*******************************************************************/

BOOL DisassembleSingleInstruction(PBYTE pStream, 
								  ULONG cbStream,
								  ULONG ulAddress, 
								  BOOL fUse32,
								  PINSTRUCTION pInstRet)
{
	TABLE_INSTRUCTION tabInst;
	CHAR szTmp[MAX_SZ_MNEMONIC];
	

	Assert(pStream);
	Assert(pInstRet);
	Assert(cbStream > 0);

	if(	NULL == pStream || NULL == pInstRet || 0 == cbStream)
	{
		gLastError = IA32DSM_ERROR_BAD_PARAM;
		return FALSE;
	}


	/*  Zero the return instruction out  */
	/*  TODO: (Maybe make this a debug thing?) */

	pInstRet->m_cbInstruction = 0;
	pInstRet->m_ulAddress = 0;
	pInstRet->m_cOperands = 0;
	pInstRet->m_Mnemonic = MneNone;
	pInstRet->m_szMnemonic[0] = 0;

    	
	/* init globals */ 

	gpStream = pStream;
	gcbStream = cbStream;
	gnSegPrefix =	0;
	gnRepPrefix =	0;
	gnLockPrefix =	0;
	gnAddrSize =	0;
	gnOperSize =	0;
	gcPrefixes =	0;
	gcbEaten =		0;
	gfModRM =		FALSE;
	gfUse32 =		fUse32;
	gLastError = IA32DSM_ERROR_NONE;

    /*  This function sets address & operand size, repeat flags, */
	/*  Segment override, Lock flags */

	if(!BuildPrefixes())
	{
		return FALSE;
	}

	
	/*  Build the Instruction from table and built-in rules */
	/*  This also builds the instruction string...			*/
	/*  Also sets gfModRM flag								*/

	if(!BuildInstruction(&tabInst))
	{
		return FALSE;
	}	

	
	/*  LOCK etc here... */

	if(gnLockPrefix == 0xF0)
	{
		strncat(pInstRet->m_szMnemonic,kszLockPrefix,
				MAX_SZ_MNEMONIC);
	}

	
	/*  REP/REPE/REPNE */

	if(gnRepPrefix)
	{
		if (MneIns == tabInst.Mnemonic ||
			MneOuts == tabInst.Mnemonic ||
			MneMovs == tabInst.Mnemonic ||
			MneLods == tabInst.Mnemonic ||
			MneStos == tabInst.Mnemonic)
		{
			if(gnRepPrefix == 0xF3)
			{
				strncat(pInstRet->m_szMnemonic,kszRepPrefix,
						MAX_SZ_MNEMONIC);
			}
		}
		else if(MneCmps == tabInst.Mnemonic ||
				MneScas == tabInst.Mnemonic)
		{
			if(gnRepPrefix == 0xF3)
			{
				strncat(pInstRet->m_szMnemonic,kszRepePrefix,
						MAX_SZ_MNEMONIC);
			}
			else if(gnRepPrefix == 0xF2)
			{
				strncat(pInstRet->m_szMnemonic,kszRepnePrefix,
						MAX_SZ_MNEMONIC);
			}
		}
	}


	/*  Operands */

    if (tabInst.cOperands != 0 && 
	   (!BuildOperands(&tabInst, ulAddress, pInstRet)))
	{
		return FALSE;
	}
	

	/*  Post Disassembly Rules Check */
	/*  TODO: simple rules like no loading cs with mov or pop */
	
	

	
	/*  Set Info for return... */

	strncpy(szTmp,tabInst.lpszMne,MAX_SZ_MNEMONIC);
	szTmp[0] = (CHAR)tolower(szTmp[0]);
	strncat(pInstRet->m_szMnemonic,szTmp,MAX_SZ_MNEMONIC);
	pInstRet->m_szMnemonic[MAX_SZ_MNEMONIC-1] = 0;

	pInstRet->m_Mnemonic  = tabInst.Mnemonic;

	Assert(gcbEaten <= MAX_INSTRUCTION_BYTES);
	if(gcbEaten > MAX_INSTRUCTION_BYTES)
	{
		gLastError = IA32DSM_ERROR_INSTRUCTION_TOO_LONG;
		return FALSE;
	}

	memcpy(pInstRet->m_ucInstruction,gpStream,gcbEaten);
	pInstRet->m_cbInstruction = gcbEaten;

	pInstRet->m_ulAddress = ulAddress;

	return TRUE;
}


/*******************************************************************/


BOOL BuildPrefixes()
{
	/*  Determine Default & Non Default Addressing & Operand Size */

	INT nNonDefAddrOperSize = gfUse32 ? 16 : 32;
	INT nDefAddrOperSize = gfUse32 ? 32 : 16;
	PBYTE pTmp = gpStream;
	BOOL fFound = FALSE;
	BYTE cur;
	INT i;

	Assert(gpStream);

	/*  Default the flags */

	gnSegPrefix = 0;
	gnRepPrefix = 0;
	gnLockPrefix = 0;
	gnAddrSize = nDefAddrOperSize;
	gnOperSize = nDefAddrOperSize;

	
	/*  Loop Collecting the prefixes and examining */
	/*  Change flags if necessary */

	gcPrefixes = 0;


	/* TODO: Do mmx/sse etc instructions affect these settings or  */
	/* should they?											       */
	/* prefixes used to determine the opcode that is, should they  */
	/* affect rep, seg, addr, oper prefixes?					   */
	/* Still don't know, doesnt seem to occur in instruction set   */

	do
	{
		fFound = FALSE;
		for(i=0;i<SIZEOF_PREFIXES;i++)
		{
			if(*pTmp == krgPrefixes[i])
			{
				fFound = TRUE;
                
				if(gcPrefixes > MAX_PREFIXES)
				{
					gLastError = IA32DSM_ERROR_TOO_MANY_PREFIXES;					
					return FALSE;
				}

				cur = *pTmp;

				gPrefix[(gcPrefixes)] = cur;
				(gcPrefixes)++;
				
				
				/*  Check if need to set some flags */

				/*  Segment Override? */

				if(	cur == 0x2E || cur == 0x36 || cur == 0x3E ||
					cur == 0x26 || cur == 0x64 || cur == 0x65)
				{
					gnSegPrefix = cur;
				}
			

				/*  Rep/n/e? */

				gnRepPrefix = (cur == 0xF2 || cur == 0xF3) ? cur : gnRepPrefix;


				/*  Lock, Address, and Operand Size */
				
				gnLockPrefix = (cur == 0xF0) ? cur : gnLockPrefix;

				gnAddrSize =	(cur == 0x67) ? 
						nNonDefAddrOperSize : gnAddrSize;

				gnOperSize =	(cur == 0x66) ? 
						nNonDefAddrOperSize : gnOperSize;

				pTmp++;
				gcbEaten++;

                break;
			}
		}
	}while(fFound);
		
	return TRUE;
}


/*******************************************************************/

/*  Build the Instruction from table and built-in rules */
/*  This also builds the instruction string...			*/
/*  Also sets gfModRM flag								*/

BOOL BuildInstruction(PTABLE_INSTRUCTION pTabInst)
{
	ULONG ulOp;
	INT nGroup;
	TABLE_INSTRUCTION ti;
	TABLE_INSTRUCTION tiExt;
	BYTE *pucModrm;
	BYTE modrm;
	INT row;
	INT col;
	ULONG i;


	Assert(pTabInst);
	Assert(gpStream);

	gfModRM = FALSE;

	ulOp = GetOpcode();
		
	/*  Do Opcode Lookup */
	
	if(TWO_BYTE_OPCODE_MASK == (TWO_BYTE_OPCODE_MASK & ulOp)) 
	{
		ti = gtab2ByteInstructions	[(ulOp & 0x000000FF) / 0x10]
									[(ulOp & 0x000000FF) % 0x10];
		gcbEaten += 2;
	}
	else
	{
		ti = gtab1ByteInstructions[ulOp / 0x10][ulOp % 0x10];
		gcbEaten++;
	}


	/*  Instruction Could Be Floating Point */

	if(MneFloatEsc == ti.Mnemonic)
	{
		pucModrm = GetModRMByte();

		/*  Stack or Mem instruction ? */

		if(*pucModrm > 0xBF)
		{
			row = ((*pucModrm) >> 4) - 0xC;
			col = ((*pucModrm) & 0x0F);
			ti = gtabFPStackReg[ulOp-0xD8][row][col];
		}
		else
		{
			ti = gtabFPMem[ulOp-0xD8][((*pucModrm) & 0x38) >> 3];
		}

		gfModRM = TRUE;
	}
	else if(IsExtensionGroupOpcode(ulOp,&nGroup))
	{
		pucModrm = GetModRMByte();

		modrm = *pucModrm;
		tiExt = gtabExtensionInstructions[nGroup - 1]
										[ModRM_GetExtOpcode(modrm)];

		
		/*  Merge instructions */

		MergeTableAndExtensionOpcodes(&ti,&tiExt);

		gfModRM = TRUE;
	}


	/*  Instruction may now need extra lookup... */
	
	if(ifdOperSizeLookupNeeded == ti.ifd)
	{
		if(!LookupOperSizeDepIns(&ti,gnOperSize))
		{
			gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
			return FALSE;
		}
	}
	else if(ifdAddrSizeLookupNeeded == ti.ifd)
	{
		if(!LookupAddrSizeDepIns(&ti,gnAddrSize))
		{
			gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
			return FALSE;
		}
	}
	else if(ifdPrefixLookupNeeded == ti.ifd)
	{
		if(!LookupPrefixDepIns(&ti))
		{
			gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
			return FALSE;
		}
	}
	
	
	/*  Now a mod equal to 3 lookup maybe needed! */

	if(ifdModEqual3LookupNeeded == ti.ifd)
	{
		pucModrm = GetModRMByte();

		modrm = *pucModrm;
		if(!LookupModIs3DepIns(&ti,ModRM_GetModValue(modrm)))
		{
			gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
			return FALSE;
		}
	}
	

	/*  Check if instruction implicitly uses modrm through its */
	/*  operands if we don't already know */

	if(!(gfModRM))
	{
		for(i=0;i<ti.cOperands;i++)
		{
			AddressingMethodEnum ame = ti.Operand[i].AddressingMethod;

			if (ame == amC || ame == amD || ame == amE || 
				ame == amG || ame == amM || ame == amP || 
				ame == amQ || ame == amR || ame == amS || 
				ame == amT || ame == amV || ame == amW ||
				amFloatingPoint == ame || 
				amHardCodeFPStackReg == ame)
			{
				gfModRM = TRUE;
				break;
			}
		}
	}

	/*  If its still not resolved then it must be invalid */

	if(MneNone == ti.Mnemonic)
	{
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	gcbEaten += ((gfModRM) ? 1:0);
	*pTabInst = ti;
	return TRUE;
}


/*******************************************************************/


BOOL BuildOperands(	PTABLE_INSTRUCTION pTabInst,
					ULONG ulAddress,
					PINSTRUCTION pInstRet)
{
	ULONG cbMaxEaten;
	ULONG i;
	INT j;
	CHAR szOperand[MAX_SZ_OPERAND];
	ULONG cbThisEat;
	AM_DECODE_INFO adi = {0};


	Assert(pTabInst);
	Assert(gpStream);
	Assert(MneNone != pTabInst->Mnemonic);

	cbMaxEaten = gcbEaten;

	for(i =0;i<pTabInst->cOperands;i++)
	{
		BOOL fDecoded = FALSE;
		for(j=0;j<SIZEOF_AMDECODERS;j++)
		{
			if(pTabInst->Operand[i].AddressingMethod == 
				krgAMDecoders[j].am)
			{
				szOperand[0] = 0;
				cbThisEat = gcbEaten;

				adi.pcbEaten = &cbThisEat;

				/* points to just after opcode */
				adi.pOperStream = gpStream + (gcbEaten) - (gfModRM ? 1:0); 

				adi.pTabIns = pTabInst;
				adi.nOperand = i;
				adi.lpszOperand = szOperand;
				adi.ulInsAddress = ulAddress;
				adi.ulDirectJumpLoc = 0;

				if(!krgAMDecoders[j].pfnDecode(&adi))
				{
					/* Decoders Set Their own last error */
					return FALSE;
				}
				
				if(cbThisEat > cbMaxEaten)
				{
					cbMaxEaten = cbThisEat;
				}

				strncpy(pInstRet->m_rgszOperand[pInstRet->m_cOperands],
					szOperand,MAX_SZ_OPERAND);

				pInstRet->m_rgszOperand[pInstRet->m_cOperands]
										[MAX_SZ_OPERAND-1] = 0;
				
				pInstRet->m_cOperands++;
				
				pInstRet->m_ulDirectJumpLoc = adi.ulDirectJumpLoc;

				fDecoded = TRUE;
				break;
			}
		}

		/*  TODO: this is debug only code */

		if(!fDecoded)
		{
			Assert(FALSE);
		}
	}

	(gcbEaten) = cbMaxEaten;
	return TRUE;
}

/*******************************************************************/
/*******************************************************************/




/*******************************************************************/
/*******************************************************************/

BOOL AMDecode_A(PAM_DECODE_INFO pdi)
{
	/* A: Direct Address. The instruction has no MODRM byte; the   */
	/* address of the operand is encoded  in the instruction; and  */
	/* no base register, index register or scaling factor can be   */
	/* applied e.g. JMP (EA). Format is XXXX:XXXXXXXX or XXXX:XXXX */
	/* depending on operand size								   */

	PTABLE_OPERAND pto;
	CHAR szTmp[MAX_SZ_OPERAND];
	CHAR szTmp1[MAX_SZ_OPERAND];
	CHAR szTmp2[MAX_SZ_OPERAND];


	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];
	if( pto->Type != otp)
	{
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	if(16 == gnOperSize)	/*  16 bit offset */
	{
		PSEGOFF1616 psegoff = (PSEGOFF1616)pdi->pOperStream;
		
		sprintf(szTmp1,"%X",psegoff->seg);
		if(psegoff->seg > 9)
		{
			strncat(szTmp1,"h",MAX_SZ_OPERAND);
		}

		/*  if first CHAR is alphabetic prefix 0 */

		if( szTmp1[0] == 'A' || szTmp1[0] == 'B' || 
			szTmp1[0] == 'C' || szTmp1[0] == 'D' || 
			szTmp1[0] == 'E' || szTmp1[0] == 'F')
		{
			sprintf(szTmp,"0%s",szTmp1);
			strncpy(szTmp1,szTmp,MAX_SZ_OPERAND);
		}


		sprintf(szTmp2,"%X",psegoff->off);
		if(psegoff->off > 9)
		{
			strncat(szTmp2,"h",MAX_SZ_OPERAND);
		}

		/*  if first CHAR is alphabetic prefix 0 */

		if( szTmp2[0] == 'A' || szTmp2[0] == 'B' || 
			szTmp2[0] == 'C' || szTmp2[0] == 'D' || 
			szTmp2[0] == 'E' || szTmp2[0] == 'F')
		{
			sprintf(szTmp,"0%s",szTmp2);
			strncpy(szTmp2,szTmp,MAX_SZ_OPERAND);
		}


		sprintf(szTmp,"%s:%s",szTmp1,szTmp2);

		strncpy(pdi->lpszOperand, szTmp, MAX_SZ_OPERAND);
		pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

		(*(pdi->pcbEaten)) += 4;
	}
	else if(32 == gnOperSize)			/*  32 bit offset */
	{
		PSEGOFF1632 psegoff = (PSEGOFF1632)pdi->pOperStream;
		sprintf(szTmp1,"%X",psegoff->seg);
		if(psegoff->seg > 9)
		{
			strncat(szTmp1,"h",MAX_SZ_OPERAND);
		}

		/*  if first CHAR is alphabetic prefix 0 */

		if( szTmp1[0] == 'A' || szTmp1[0] == 'B' || 
			szTmp1[0] == 'C' || szTmp1[0] == 'D' || 
			szTmp1[0] == 'E' || szTmp1[0] == 'F')
		{
			sprintf(szTmp,"0%s",szTmp1);
			strncpy(szTmp1,szTmp,MAX_SZ_OPERAND);
		}


		sprintf(szTmp2,"%X",psegoff->off);
		if(psegoff->seg > 9)
		{
			strncat(szTmp2,"h",MAX_SZ_OPERAND);
		}

		/*  if first CHAR is alphabetic prefix 0 */

		if( szTmp2[0] == 'A' || szTmp2[0] == 'B' || 
			szTmp2[0] == 'C' || szTmp2[0] == 'D' || 
			szTmp2[0] == 'E' || szTmp2[0] == 'F')
		{
			sprintf(szTmp,"0%s",szTmp2);
			strncpy(szTmp2,szTmp,MAX_SZ_OPERAND);
		}


		sprintf(szTmp,"%s:%s",szTmp1,szTmp2);

		strncpy(pdi->lpszOperand, szTmp, MAX_SZ_OPERAND);
		pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

		(*(pdi->pcbEaten)) += 6;
	}
	else
	{
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_C(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE modrm;
	LPCSTR lpszReg;


	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otd)
	{
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	modrm = *pdi->pOperStream;
	lpszReg = ModRM_GetRegString(32,modrm,rsControl);

	if(lpszReg)
	{
		strncpy(pdi->lpszOperand, lpszReg, MAX_SZ_OPERAND);
		pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;
		return TRUE;
	}

	gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
	return FALSE;
}

/*******************************************************************/

BOOL AMDecode_D(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE modrm;
	LPCSTR lpszReg;

	
	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otd)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	modrm = *pdi->pOperStream;
	lpszReg = ModRM_GetRegString(32,modrm,rsDebug);

	if(lpszReg)
	{
		strncpy(pdi->lpszOperand, lpszReg, MAX_SZ_OPERAND);
		pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;
		return TRUE;
	}

	gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
	return FALSE;
}

/*******************************************************************/

BOOL AMDecode_E(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	CHAR szEffectiveAddress[MAX_SZ_EA];

	
	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otv && pto->Type != otb && 
	    pto->Type != otw && pto->Type != otd &&
		pto->Type != otq && pto->Type != otp)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}
	
	if(!GetEffectiveAddressString(	gnAddrSize,szEffectiveAddress,
									rsGeneral,pdi))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand, szEffectiveAddress, MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/
BOOL AMDecode_G(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	INT nRegSize;
	BYTE modrm;

	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	/*  So Far only op type v working (16/32 bit) */
	/*  and b and w */

	if (pto->Type != otv && 
		pto->Type != otb && 
		pto->Type != otw &&
		pto->Type != otd)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	nRegSize = (pto->Type == otb) ? 8 : gnOperSize;
	nRegSize = (pto->Type == otw) ? 16 : nRegSize;
	nRegSize = (pto->Type == otd) ? 32 : nRegSize;

	modrm = *pdi->pOperStream;

	strncpy(pdi->lpszOperand, 
		ModRM_GetRegString(nRegSize,modrm,rsGeneral),
		MAX_SZ_OPERAND);

	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_I(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto, pro;
	INT nImmedRead = 0;
	INT read,i,nSize,Mod;
	PBYTE pImmed;
	ULONG cbDisp;
	BYTE modrm;
	BYTE sib;
	CHAR szImmed[MAX_SZ_IMMEDIATE];

	Assert(pdi);

	pdi->lpszOperand[0] = 0;
	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	/*  So Far only op type v working (16/32 bit) */
	/*  and b and w; */

	if( pto->Type != otv && pto->Type != otb &&	pto->Type != otw)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	/*  Because their may be multiple immediate operands */
	/*  eg Enter (C8) Iw, Ib we must keep track of how many */
	/*  immediate bytes we've read already this instruction */

	if(0 != pdi->nOperand)
	{
		/*  Check Previous Operands */

		read = 0;
		for(i=0;i<pdi->nOperand;i++)
		{
			if(pdi->pTabIns->Operand[i].AddressingMethod == amI)
			{
				pro = &pdi->pTabIns->Operand[i];
				read = (pro->Type == otb) ? 8 : gnOperSize;
				read = (pro->Type == otw) ? 16 : read;
				nImmedRead += (read >> 3);
			}
		}
	}

	/*  b or w operand type force hard size */

	nSize = (pto->Type == otb) ? 8 : gnOperSize;
	nSize = (pto->Type == otw) ? 16 : nSize;

	
	/*  Decoration Required? */

	if(opfdSizePrefix == pto->dec)
	{
		if(8 == nSize)
		{
			strncpy(pdi->lpszOperand,kszBytePrefix,MAX_SZ_OPERAND);
		}
		else if(16 == nSize)
		{
			strncpy(pdi->lpszOperand,kszWordPrefix,MAX_SZ_OPERAND);
		}
		else if(32 == nSize)
		{
			strncpy(pdi->lpszOperand,kszDwordPrefix,MAX_SZ_OPERAND);
		}
	}

	/*  immediate data follows displacement/sib/modrm */
    /*  need to find out if modrm exists */

	pImmed = pdi->pOperStream;
	cbDisp = 0;

	if(gfModRM)
	{
		modrm = *pImmed;
		sib = *(pImmed+1);

		Mod = ModRM_GetModValue(modrm);
		
		pImmed += (ModRM_HasSIB(gnAddrSize, modrm) ? 2 : 1);

		
		/*  SIB? increase Eaten count */

		(*(pdi->pcbEaten)) += (ModRM_HasSIB(gnAddrSize,modrm) ? 1 : 0);

		
		/*  At Displacement, is there one and what size is it? */

		if(16 == gnAddrSize) /* 16 bit Effective Address ModRM */
							 /* encoded */
		{
			if(1 == Mod)
			{
				cbDisp = 1;
			}
			else if(2 == Mod  || 
					(0 == Mod && 6 == ModRM_GetRMValue(modrm)))
			{
				cbDisp = 2;
			}
		}
		else if(32 == gnAddrSize)	/* 32 bit Effective Address */
									/* ModRM encoded */
		{
			if(1 == Mod)
			{	
				cbDisp = 1;
			}
			else if
				(2 == Mod || 
				(0 == Mod && 5 == ModRM_GetRMValue(modrm)) ||
				(0 == Mod && 4 == ModRM_GetRMValue(modrm) && 
				 5 == SIB_GetBase(sib)))
			{
				cbDisp = 4;
			}
		}
	}
	
	/*  Skip Displacement */

	pImmed += cbDisp;

	/*  Skip Prev Read Immediate Bytes */

	pImmed += nImmedRead;

	if(!FormatImmediate(pImmed,szImmed,nSize))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	nImmedRead += (nSize >> 3);
	(*(pdi->pcbEaten)) += (cbDisp + nImmedRead);

	strncat(pdi->lpszOperand,szImmed,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_J(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	INT nSize, nInsSize;
	CHAR szDisp[MAX_SZ_DISPLACEMENT];
	ULONG ulDirectJumpLoc;

	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if(otv != pto->Type && otb != pto->Type)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	nSize = (pto->Type == otb) ? 8 : gnOperSize;
	nInsSize = (*(pdi->pcbEaten)) + (nSize >> 3);

	FormatJumpDisplacement(pdi->ulInsAddress + nInsSize,
							pdi->pOperStream,szDisp,nSize,&ulDirectJumpLoc);

	 /*  used no extra bytes from instruction stream... */
	(*(pdi->pcbEaten)) += (nSize >> 3);

	strncpy(pdi->lpszOperand,szDisp,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;
	pdi->ulDirectJumpLoc = ulDirectJumpLoc;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_M(PAM_DECODE_INFO pdi)
{
	BYTE modrm;
	CHAR szEffectiveAddress[MAX_SZ_EA];


	Assert(pdi);

	modrm = *pdi->pOperStream;

	if(ModRM_GetModValue(modrm) == 3)
	{
		AssertSz(0,"Access must be to memory only");
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	/*  Get Effective address... */

	if(!GetEffectiveAddressString(	gnAddrSize,szEffectiveAddress,
									rsGeneral,pdi))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,szEffectiveAddress,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_O(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	CHAR szOffset[MAX_SZ_DISPLACEMENT];
	CHAR szSegPrefix[32] = "\0";

	Assert(pdi);
	
	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if(otv != pto->Type && otb != pto->Type)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

    FormatDisplacement(pdi->pOperStream,szOffset,gnAddrSize);

	/*  Segment Override Prefix? */

	if(gnSegPrefix)
	{
		switch(gnSegPrefix)
		{
		case 0x2E:
			strncpy(szSegPrefix,"cs:",32);
			break;
		case 0x36:
			strncpy(szSegPrefix,"ss:",32);
			break;
		case 0x3E:
			strncpy(szSegPrefix,"ds:",32);
			break;
		case 0x26:
			strncpy(szSegPrefix,"es:",32);
			break;
		case 0x64:
			strncpy(szSegPrefix,"fs:",32);
			break;
		case 0x65:
			strncpy(szSegPrefix,"gs:",32);
			break;
		}
	}
		
	sprintf(pdi->lpszOperand,"[%s%s]",szSegPrefix,szOffset);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	/*  used no extra bytes from instruction stream... */

	(*(pdi->pcbEaten)) += (gnAddrSize >> 3); 
	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_P(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE modrm;
	
	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otq &&
		pto->Type != otd &&
		pto->Type != otdq &&
		pto->Type != otpi )
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	modrm = *pdi->pOperStream;

	strncpy(pdi->lpszOperand,ModRM_GetRegString(64,modrm,rsMMX),
							MAX_SZ_OPERAND);

	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;
	
	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_Q(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE modrm;
	CHAR szEffectiveAddress[MAX_SZ_EA];


	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otq &&
		pto->Type != otdq &&
		pto->Type != otd &&
		pto->Type != otpi)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	if(otpi == pto->Type)
	{
		modrm = *pdi->pOperStream;
		if(3 != ModRM_GetModValue(modrm))
		{
			AssertSz(0,"Can only specify a direct register");
			gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
			return FALSE;
		}
	}

	if(!GetEffectiveAddressString(gnAddrSize,szEffectiveAddress,
									rsMMX,pdi))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,szEffectiveAddress,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_R(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE modrm;
	CHAR szEffectiveAddress[MAX_SZ_EA];


	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	/*  So Far only op type v working (16/32 bit) */
	/*  and b and w */

	if (pto->Type != otd)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	modrm = *pdi->pOperStream;
	if(3 != ModRM_GetModValue(modrm))
	{
		AssertSz(0,"Can only specify a direct general register");
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	/*  Get Effective address... which will be a register not and ea */

	if(!GetEffectiveAddressString(gnAddrSize,szEffectiveAddress,
									rsGeneral,pdi))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,szEffectiveAddress,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_S(PAM_DECODE_INFO pdi)
	{

	/*  TODO: Mov Sw, Ew, opcode 8E, cannot be used to load */
	/*  cs. so mov cs, [eax] is an invalid instruction. Should we */
	/*  check for this here? currently no... */

	PTABLE_OPERAND pto;
	BYTE modrm;
	LPCSTR lpszSeg;

	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	/*  So Far only op type v working (16/32 bit) */
	/*  and b and w */

	if (pto->Type != otw)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	modrm = *pdi->pOperStream;

	lpszSeg = ModRM_GetRegString(32,modrm,rsSegment);

	if(NULL == lpszSeg)
	{
		AssertSz(0,"NULL == lpszSeg");
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,lpszSeg,MAX_SZ_OPERAND);

	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
	}

/*******************************************************************/

BOOL AMDecode_T(PAM_DECODE_INFO pdi)
{
	pdi; /*  Unref */
	AssertSz(0,"Addressing Mode Decoder Not Implemented");
	gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
	return FALSE;
}

/*******************************************************************/

BOOL AMDecode_V(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE modrm;

	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	/*  So Far only op type v working (16/32 bit) */
	/*  and b and w */

	if (pto->Type != otdq &&
		pto->Type != otps &&
		pto->Type != otss &&
		pto->Type != otpd &&
		pto->Type != otsd && 
		pto->Type != otq )
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	modrm = *pdi->pOperStream;
	strncpy(pdi->lpszOperand,ModRM_GetRegString(64,modrm,rsXMM),
			MAX_SZ_OPERAND);

	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_W(PAM_DECODE_INFO pdi)
{
	CHAR szEffectiveAddress[MAX_SZ_EA];
	PTABLE_OPERAND pto;
	
	
	Assert(pdi);

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otdq &&
		pto->Type != otps &&
		pto->Type != otss &&
		pto->Type != otpd &&
		pto->Type != otsd &&
		pto->Type != otq &&
		pto->Type != ots &&
		pto->Type != otd )
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	if(!GetEffectiveAddressString(gnAddrSize,szEffectiveAddress,
		rsXMM,pdi))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,szEffectiveAddress,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_X(PAM_DECODE_INFO pdi)
{
	pdi; /*  Unref */
	AssertSz(0,"Addressing Mode Decoder Not Implemented");
	gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
	return FALSE;
}

/*******************************************************************/

BOOL AMDecode_Y(PAM_DECODE_INFO pdi)
{
	pdi; /*  Unref */
	AssertSz(0,"Addressing Mode Decoder Not Implemented");
	gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
	return FALSE;
}

/*******************************************************************/

BOOL AMDecode_HardCodeReg(PAM_DECODE_INFO pdi)
{	
	PTABLE_OPERAND pto;
	LPCSTR lpszReg;

	Assert(pdi);
	
	pto = &pdi->pTabIns->Operand[pdi->nOperand];
	
	lpszReg = GetHardCodedRegString(pto->Type,gnOperSize);
    if(NULL == lpszReg)
	{
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,lpszReg,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;
    
	return TRUE;
}

/*******************************************************************/

BOOL AMDecode_HardCodeNum(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;


	Assert(pdi);
	
	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if(pto->Type != otHard_1)
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	pdi->lpszOperand[0] = '1';
	pdi->lpszOperand[1] = 0;

	return TRUE;
}

/*******************************************************************/

/*	Note Floating point instructions which access memory as these */
/*	ones do are automatically prefixed with ptr size, no need for */
/*	decoration spec in instruction */

BOOL AMDecode_FloatingPoint(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	CHAR szEffectiveAddress[MAX_SZ_EA];

#ifdef _DEBUG
	BYTE modrm;
#endif

	Assert(pdi);
	
	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	if (pto->Type != otFP_DoubleReal && 
		pto->Type != otFP_SingleReal &&
		pto->Type != otFP_1428B &&
		pto->Type != otFP_2B &&
		pto->Type != otFP_DwordInteger &&
		pto->Type != otFP_ExtendedReal &&
		pto->Type != otFP_98108B &&
		pto->Type != otFP_WordInteger &&
		pto->Type != otFP_PackedBCD && 
		pto->Type != otFP_QwordInteger)

	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	
	/*  Debug Check to make sure mod is not 3 */
#ifdef _DEBUG
	modrm = *(pdi->pOperStream);
	if(ModRM_GetModValue(modrm) == 3)
	{
		/*  if mod is 3 it should be going through the hard coded */
		/*  reg floating point decoder...! Panic if you hit this! */
		Assert(FALSE);
	}
#endif


	/*  Get Effective address... */

	if(!GetEffectiveAddressString(gnAddrSize,szEffectiveAddress,
									rsGeneral,pdi))
	{
		Assert(FALSE);
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	/*  Form the operand string... */
	
	pdi->lpszOperand[0] = 0;
	switch(pto->Type)
	{
	case otFP_DoubleReal:
	case otFP_QwordInteger:
		strncpy(pdi->lpszOperand,kszQwordPtrPrefix,MAX_SZ_OPERAND);
		break;
	case otFP_SingleReal:
	case otFP_DwordInteger:
		strncpy(pdi->lpszOperand,kszDwordPtrPrefix,MAX_SZ_OPERAND);
		break;
	case otFP_1428B:
	case otFP_98108B:
		strncpy(pdi->lpszOperand,kszBytePtrPrefix,MAX_SZ_OPERAND);
		break;
	case otFP_2B:
	case otFP_WordInteger:
		strncpy(pdi->lpszOperand,kszWordPtrPrefix,MAX_SZ_OPERAND);
		break;
	case otFP_ExtendedReal:
	case otFP_PackedBCD:
		strncpy(pdi->lpszOperand,kszTwordPtrPrefix,MAX_SZ_OPERAND);
		break;		
	}
	
	strncat(pdi->lpszOperand,szEffectiveAddress,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/

/*  TODO: this is a carbon copy of the HardCodeReg decoder, needed */
/*  at the moment because we use HardCodeFPStackReg to determine   */
/*  presence of modrm byte in InstructioHasModrmByte(). Please     */
/*  remove the need for this Addressing mode...					   */

BOOL AMDecode_HardCodeFPStackReg(PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	LPCSTR lpszReg;

	Assert(pdi);
	
	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	lpszReg = GetHardCodedRegString(pto->Type,gnOperSize);
    if(NULL == lpszReg)
	{
		gLastError = IA32DSM_ERROR_COULD_NOT_DECODE;
		return FALSE;
	}

	strncpy(pdi->lpszOperand,lpszReg,MAX_SZ_OPERAND);
	pdi->lpszOperand[MAX_SZ_OPERAND-1] = 0;

	return TRUE;
}

/*******************************************************************/
/*******************************************************************/

static LPCSTR krgsz16BitEA[] = 
{
	"bx+si",
	"bx+di",
	"bp+si",
	"bp+di",
	"si",
	"di",
	"bp",
	"bx",
};

static LPCSTR krgsz32BitEA[] = 
{
	"eax",
	"ecx",
	"edx",
	"ebx",
	"",
	"ebp",
	"esi",
	"edi",
};

static LPCSTR krgszMMXReg[] = 
{
	"mm0",
	"mm1",
	"mm2",
	"mm3",
	"mm4",
	"mm5",
	"mm6",
	"mm7",
};

static LPCSTR krgszXMMReg[] = 
{
	"xmm0",
	"xmm1",
	"xmm2",
	"xmm3",
	"xmm4",
	"xmm5",
	"xmm6",
	"xmm7",
};

static LPCSTR krgsz8BitReg[] = 
{
	"al",
	"cl",
	"dl",
	"bl",
	"ah",
	"ch",
	"dh",
	"bh",
};

static LPCSTR krgsz16BitReg[] = 
{
	"ax",
	"cx",
	"dx",
	"bx",
	"sp",
	"bp",
	"si",
	"di",
};

static LPCSTR krgsz32BitReg[] = 
{
	"eax",
	"ecx",
	"edx",
	"ebx",
	"esp",
	"ebp",
	"esi",
	"edi",
};

static LPCSTR krgszControlReg[] = 
{
	"cr0",
	NULL,
	"cr2",
	"cr3",
	"cr4",
	NULL,
	NULL,
	NULL,
};

static LPCSTR krgszDebugReg[] = 
{
	"dr0",
	"dr1",
	"dr2",
	"dr3",
	"dr4",
	"dr5",
	"dr6",
	"dr7",
};

static LPCSTR krgszSegs[] = 
{
	"es",
	"cs",
	"ss",
	"ds",
	"fs",
	"gs",
	NULL,
	NULL,
};

/*******************************************************************/

BOOL ModRM_GetDisplacementString(INT nAddrSize, BYTE *pModrm, 
								LPSTR lpszDisp,	ULONG ulDispSize)
{
	PBYTE pDisp;


	Assert(ulDispSize == 8 || ulDispSize == 16 || ulDispSize == 32);

    /*  Determine Whether SIB byte exists if so move on one */

	if(ModRM_HasSIB(nAddrSize, (*pModrm)))
	{
		pDisp = pModrm + 2;
	}
	else
	{
		pDisp = pModrm + 1;
	}

	/*  Take however many bytes we need and format correctly */
	
	return FormatDisplacement(pDisp, lpszDisp, ulDispSize);
}

/*******************************************************************/

/*  Used by E,W and Q */

LPCSTR ModRM_GetEffectiveAddressOrRegString(INT nAddrSize, 
											const BYTE modrm, 
											INT nRegSize, 
											RegSet rs)
{
	if(32 == nAddrSize)
	{
		if(3 == ModRM_GetModValue(modrm))
		{
			return ModRM_GetRegisterStringFromIndex(nRegSize, rs,
										ModRM_GetRMValue(modrm));
		}
		else if(0 == ModRM_GetModValue(modrm) && 
				5 == ModRM_GetRMValue(modrm))
		{
			return kszEmpty;
		}
		else
		{
			return krgsz32BitEA[ModRM_GetRMValue(modrm)];
		}
	}
	else if (16 == nAddrSize)
	{
		if (0 == ModRM_GetModValue(modrm) && 
			6 == ModRM_GetRMValue(modrm))
		{
			return kszEmpty;
		}
		else if(3 == ModRM_GetModValue(modrm))
		{
			return ModRM_GetRegisterStringFromIndex(nRegSize, 
									rs,ModRM_GetRMValue(modrm));
		}
		else
		{
			return krgsz16BitEA[ModRM_GetRMValue(modrm)];
		}
	}

	Assert(FALSE);
	return kszEmpty;
}

/*******************************************************************/

LPCSTR ModRM_GetRegisterStringFromIndex(INT nRegSize, 
										RegSet regSet, 
										INT index)
{
	if(rsGeneral == regSet)
	{
		if(32 == nRegSize)
		{
			return krgsz32BitReg[index];
		}
		else if (16 == nRegSize)
		{
			return krgsz16BitReg[index];
		}
		else if (8 == nRegSize)
		{
			return krgsz8BitReg[index];
		}
	}
	else if(rsMMX == regSet)
	{
		return krgszMMXReg[index];
	}
	else if(rsXMM == regSet)
	{
		return krgszXMMReg[index];
	}
	else if(rsControl == regSet)
	{
		return krgszControlReg[index];
	}
	else if(rsDebug == regSet)
	{
		return krgszDebugReg[index];
	}
	else if(rsSegment == regSet)
	{
		return krgszSegs[index];
	}

	Assert(FALSE);
	return kszEmpty;
}

/*******************************************************************/

LPCSTR ModRM_GetRegString(INT nRegSize,const BYTE modrm,RegSet rs)
{
	return ModRM_GetRegisterStringFromIndex(nRegSize, rs, 
										ModRM_GetRegValue(modrm));
}

/*******************************************************************/

BOOL ModRM_HasSIB(INT nAddrSize, const BYTE modrm)
{
	return	(nAddrSize == 32 && (3 != ModRM_GetModValue(modrm)) && 
			(4 == ModRM_GetRMValue(modrm)));
}

/*******************************************************************/

ULONG ModRM_GetExtOpcode(const BYTE modrm)
{
	return (modrm & 0x38) >> 3;
}

/*******************************************************************/

ULONG ModRM_GetModValue(const BYTE modrm)
{
	return (modrm & 0xC0) >> 6;
}

/*******************************************************************/

ULONG ModRM_GetRegValue(const BYTE modrm)
{
	return (modrm & 0x38) >> 3;
}

/*******************************************************************/

ULONG ModRM_GetRMValue(const BYTE modrm)
{
	return (modrm & 0x07);
}

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

static LPCSTR krgszSIB_Base[] = 
{
	"eax",
	"ecx",
	"edx",
	"ebx",
	"esp",
	"ebp",
	"esi",
	"edi",
};

static LPCSTR krgszSIB_Index[] = 
{
	"eax",
	"ecx",
	"edx",
	"ebx",
	"",
	"ebp",
	"esi",
	"edi",
};

/*******************************************************************/

ULONG SIB_GetScale(const BYTE sib)
{
	return (1 << ((sib & 0xC0) >> 6));
}

/*******************************************************************/

ULONG SIB_GetIndex(const BYTE sib)
{
	return (sib & 0x38) >> 3;
}

/*******************************************************************/

ULONG SIB_GetBase(const BYTE sib)
{
	return (sib & 0x07);
}

/*******************************************************************/

LPCSTR SIB_GetIndexString(const BYTE sib, LPSTR lpszIndex)
{
	if(4 == SIB_GetIndex(sib))
	{
		lpszIndex[0] = 0;	
	}
    else if(SIB_GetScale(sib) > 1)
	{
		sprintf(lpszIndex,"%s*%ul",krgszSIB_Index[SIB_GetIndex(sib)],
												SIB_GetScale(sib));
	}
	else
	{
		sprintf(lpszIndex,"%s",krgszSIB_Index[SIB_GetIndex(sib)]);
	}

	return lpszIndex;
}

/*******************************************************************/

LPCSTR SIB_GetBaseString(const BYTE sib)
{
	return krgszSIB_Base[SIB_GetBase(sib)];
}

/*******************************************************************/

LPCSTR SIB_GetSIBString(const BYTE sib, LPSTR lpszSIB)
{
	CHAR szSI[MAX_SZ_SIB];
	SIB_GetIndexString(sib, szSI);

	lpszSIB[0] = 0;
	strncpy(lpszSIB, SIB_GetBaseString(sib),MAX_SZ_SIB);
	
	if(szSI[0])
	{
		strncat(lpszSIB, "+",MAX_SZ_SIB);
		strncat(lpszSIB,szSI,MAX_SZ_SIB);
	}

	lpszSIB[MAX_SZ_SIB-1] = 0;
	return lpszSIB;
}

/*******************************************************************/

/*  NOTE: ulAddress is address of next instruction not current one */

BOOL FormatJumpDisplacement(ULONG ulAddress,PBYTE pStream, 
							LPSTR lpszDisp,ULONG nBitSize, 
							PULONG pulDirectJumpLoc)
{
	ULONG x;
	CHAR szTmp[MAX_SZ_DISPLACEMENT];

	switch(nBitSize)
	{
	case 8: /*  Always Sign Extended */
		{
		x = (*(PCHAR)pStream);
		x += ulAddress;
		break;
		}
	case 16:
		{
		x = (INT)(*(short*)pStream);
		x += ulAddress;
		break;
		}
	case 32:
		{
		x = (INT)(*(long*)pStream);
		x += ulAddress;
		break;
		}
	default:
		{
		Assert(FALSE);
		return FALSE;
		}
	}

	sprintf(lpszDisp,"%X",x);
	if(x > 9)
	{
		strncat(lpszDisp,"h",MAX_SZ_DISPLACEMENT);
	}

	*pulDirectJumpLoc = x;

	/*  if first CHAR is alphabetic prefix 0 */

	if( lpszDisp[0] == 'A' || lpszDisp[0] == 'B' || 
		lpszDisp[0] == 'C' || lpszDisp[0] == 'D' || 
		lpszDisp[0] == 'E' || lpszDisp[0] == 'F')
	{
		sprintf(szTmp,"0%s",lpszDisp);
		strncpy(lpszDisp,szTmp,MAX_SZ_DISPLACEMENT);
		szTmp[MAX_SZ_DISPLACEMENT -1] = 0;
	}

	return TRUE;
}

/*******************************************************************/

BOOL FormatDisplacement(PBYTE pStream, LPSTR lpszDisp,ULONG nBitSize)
{
	BOOL fNeg = FALSE;
	ULONG x;
	CHAR szTmp[MAX_SZ_DISPLACEMENT];


	switch(nBitSize)
	{
	case 8: /*  Always Sign Extended */
		x = (*(PCHAR)pStream);
		if(0x80000000 & x) // Negative
		{
			x = (~x)+1;
			sprintf(lpszDisp,"%X",x);
			fNeg = TRUE;
		}
		else
		{
			sprintf(lpszDisp,"%X",x);
		}
		break;
	case 16:
		x = (ULONG)(*(PUSHORT)pStream);
		sprintf(lpszDisp,"%X",x);
		break;
	case 32:
		x = (ULONG)(*(PULONG)pStream);
		sprintf(lpszDisp,"%X",x);
		break;
	default:
		Assert(FALSE);
		return FALSE;
		break;
	}


	if(x > 9)
	{
		strncat(lpszDisp,"h",MAX_SZ_DISPLACEMENT);
	}

	/*  if first CHAR is alphabetic prefix 0 */

	if( lpszDisp[0] == 'A' || lpszDisp[0] == 'B' || 
		lpszDisp[0] == 'C' || lpszDisp[0] == 'D' || 
		lpszDisp[0] == 'E' || lpszDisp[0] == 'F')
	{
		sprintf(szTmp,"0%s",lpszDisp);
		strncpy(lpszDisp,szTmp,MAX_SZ_DISPLACEMENT);
	}

	if(fNeg)
	{
		sprintf(szTmp,"-%s",lpszDisp);
		strncpy(lpszDisp,szTmp,MAX_SZ_DISPLACEMENT);
		szTmp[MAX_SZ_DISPLACEMENT - 1] = 0;
	}

	return TRUE;
}

/*******************************************************************/

BOOL FormatImmediate(PBYTE pStream, LPSTR lpszDisp,ULONG nBitSize)
{
	CHAR szTmp[MAX_SZ_IMMEDIATE];
	ULONG x;

	switch(nBitSize)
	{
	case 8: /*  Always Sign Extended?? Nope? */
		x = *(PBYTE)pStream;
		sprintf(lpszDisp,"%X",x);
		break;
	case 16:
		x = *(USHORT*)pStream;
		sprintf(lpszDisp,"%X",x);
		break;
	case 32:
		x = *(PULONG)pStream;
		sprintf(lpszDisp,"%X",x);
		break;
	default:
		Assert(FALSE);
		return FALSE;
		break;
	}


	if(x > 9)
	{
		strncat(lpszDisp,"h",MAX_SZ_IMMEDIATE);
	}

	/*  if first CHAR is alphabetic prefix 0 */

	if( lpszDisp[0] == 'A' || lpszDisp[0] == 'B' || 
		lpszDisp[0] == 'C' || lpszDisp[0] == 'D' || 
		lpszDisp[0] == 'E' || lpszDisp[0] == 'F')
	{
		sprintf(szTmp,"0%s",lpszDisp);
		strncpy(lpszDisp,szTmp,MAX_SZ_IMMEDIATE);
		szTmp[MAX_SZ_IMMEDIATE-1]=0;
	}

	return TRUE;
}

/*******************************************************************/

ULONG GetOpcode()
{
	PBYTE pOp;

	Assert(gpStream);

	pOp = gpStream + gcPrefixes;

	if(*pOp == 0x0F)
	{
		return ((*pOp) << 8) | (*(pOp+1));
    }
	else
	{
		return *(PBYTE)pOp;
    }
}

/*******************************************************************/

PBYTE GetModRMByte(void)
{
	PBYTE pTmp;

	Assert(gpStream);

	pTmp = gpStream + gcPrefixes; /* Skip Prefixes */

	return ((*pTmp == 0x0F) ? (pTmp + 2) : (pTmp + 1));
}

/*******************************************************************/

/*******************************************************************/
/*  Often Extension Opcodes depend on the opcode it was extended   */
/*  for, for oepreand types etc This function takes original and   */
/*  extension opcodes and merges then according to a set of rules  */
/*  which are:													   */
/*																   */
/*  1.	Old Mnemonic must be changed to extension mnemonic         */
/*  2.	Operand Count must be set to Extension Operand Count       */
/*  3.	If any of the extension operand types have Addressing Mode */
/* 		of amNone or Operand Type of otNone then they use the      */
/* 		original Opcodes AddressingMode and Operand Type		   */
/*******************************************************************/

void MergeTableAndExtensionOpcodes(PTABLE_INSTRUCTION pTi, 
								   PTABLE_INSTRUCTION pTiExt)
{
	ULONG i;


	/*  Merge this ext opcode into our previous 1 byte or 2 byte	   */
	/*	original opcode Extension overrides how many operands are used */

	pTi->Mnemonic = pTiExt->Mnemonic;
	pTi->lpszMne = pTiExt->lpszMne;
	pTi->cOperands = pTiExt->cOperands;
	pTi->ifd = pTiExt->ifd;

	/*  Operand may depend on previous opcode */

	for(i=0;i<pTi->cOperands;i++)
	{
		if(pTiExt->Operand[i].AddressingMethod != amNone)
		{
			pTi->Operand[i].AddressingMethod = 
				pTiExt->Operand[i].AddressingMethod;
		}

		if(pTiExt->Operand[i].Type != otNone)
		{
			pTi->Operand[i].Type = pTiExt->Operand[i].Type;
		}

		if(pTiExt->Operand[i].dec != opfdNone)
		{
			pTi->Operand[i].dec = pTiExt->Operand[i].dec;
		}
	}
}

/*******************************************************************/

BOOL IsExtensionGroupOpcode(ULONG ulOp, PINT pnGroup)
{
	if(ulOp >= 0x0080 && ulOp <=0x0083) /*  Group 1 */
	{
		*pnGroup = 1;
		return TRUE;
	}
	else if(0x00C0 == ulOp ||				/*  Group 2 */
			0x00C1 == ulOp || 
			0x00D0 == ulOp || 
			0x00D1 == ulOp || 
			0x00D2 == ulOp || 
			0x00D3 == ulOp)
	{
		*pnGroup = 2;
		return TRUE;
	}
	else if(0x00F6 == ulOp ||				/*  Group 3 */
			0x00F7 == ulOp)
	{
		*pnGroup = 3;
		return TRUE;
	}
	else if(0x00FE == ulOp)					/*  Group 4 */
	{
		*pnGroup = 4;
		return TRUE;
	}
	else if(0x00FF == ulOp)					/*  Group 5 */
	{
		*pnGroup = 5;
		return TRUE;
	}
	else if(0x0F00 == ulOp)					/*  Group 6 */
	{
		*pnGroup = 6;
		return TRUE;
	}
	else if(0x0F01 == ulOp)					/*  Group 7 */
	{
		*pnGroup = 7;
		return TRUE;
	}
	else if(0x0FBA == ulOp)					/*  Group 8 */
	{
		*pnGroup = 8;
		return TRUE;
	}
	else if(0x0FC7 == ulOp)					/*  Group 9 */
	{
		*pnGroup = 9;
		return TRUE;
	}
	else if(0x0FB9 == ulOp)					/*  Group 10 */
	{
		*pnGroup = 10;
		return TRUE;
    }
	else if(0x00C6 == ulOp ||				/*  Group 11 */
			0x00C7 == ulOp)					
	{
		*pnGroup = 11;
		return TRUE;
	}
	else if(0x0F71 == ulOp)					/*  Group 12 */
	{
		*pnGroup = 12;
		return TRUE;
	}
	else if(0x0F72 == ulOp)					/*  Group 13 */
	{
		*pnGroup = 13;
		return TRUE;
	}
	else if(0x0F73 == ulOp)					/*  Group 14 */
	{
		*pnGroup = 14;
		return TRUE;
	}
	else if(0x0FAE == ulOp)					/*  Group 15 */
	{
		*pnGroup = 15;
		return TRUE;
	}
	else if(0x0F18 == ulOp)					/*  Group 16 */
	{
		*pnGroup = 16;
		return TRUE;
	}

	return FALSE;
}

/*******************************************************************/

BOOL LookupOperSizeDepIns(PTABLE_INSTRUCTION pIns,INT nOperSize)
{
	INT i;

	for(i=0;i<SIZEOF_OPER_SIZE_DEP_INS_MAP;i++)
	{
		if(pIns->Mnemonic == oper_size_dep_ins_map[i].mne)
		{
			if(16 == nOperSize)
			{
				*pIns = oper_size_dep_ins_map[i].ti16bit;
				return TRUE;
			}
			else if(32 == nOperSize)
			{
				*pIns = oper_size_dep_ins_map[i].ti32bit;
				return TRUE;
			}
		}
	}

	AssertSz(FALSE,"No mapped Operand Size Dependant "
					"Instruction for this mne");
	return FALSE;
}

/*******************************************************************/

BOOL LookupAddrSizeDepIns(PTABLE_INSTRUCTION pIns,INT nAddrSize)
{
	INT i;


	for(i=0;i<SIZEOF_ADDR_SIZE_DEP_INS_MAP;i++)
	{
		if(pIns->Mnemonic  == addr_size_dep_ins_map[i].mne)
		{
			if(16 == nAddrSize)
			{
				*pIns = addr_size_dep_ins_map[i].ti16bit;
				return TRUE;
			}
			else if(32 == nAddrSize)
			{
				*pIns = addr_size_dep_ins_map[i].ti32bit;
				return TRUE;
			}
		}
	}

	Assert(FALSE);
	return FALSE;
}

/*******************************************************************/

/*  BUGBUG: TODO: Note only first prefix is used for determining */
/*	instruction, is this correct? */

BOOL LookupPrefixDepIns(PTABLE_INSTRUCTION pIns)
{
	ULONG i,j;

	for(i=0;i<SIZEOF_PREFIX_DEP_INS_MAP;i++)
	{
		if(pIns->Mnemonic  == prefix_dep_ins_map[i].mne)
		{
			/* First Match Only */

			for(j = 0;j<gcPrefixes;j++)
			{
				if(0x66 == gPrefix[j])
				{
					*pIns = prefix_dep_ins_map[i].ti66;
					return TRUE;
				}
				else if(0xF2 == gPrefix[j])
				{
					*pIns = prefix_dep_ins_map[i].tiF2;
					return TRUE;
				}
				else if(0xF3 == gPrefix[j])
				{
					*pIns = prefix_dep_ins_map[i].tiF3;
					return TRUE;
				}
			}

			/* No Match? No Prefix So => */

			*pIns = prefix_dep_ins_map[i].tiNoPrefix;
			return TRUE;
		}
	}

	Assert(FALSE);
	return FALSE;
}

/*******************************************************************/

BOOL LookupModIs3DepIns(PTABLE_INSTRUCTION pIns,INT nMod)
{
	INT i;


	for(i=0;i<SIZEOF_MOD_IS_3_DEP_MAP;i++)
	{
		if(pIns->Mnemonic  == mod_is_3_dep_ins_map[i].mne)
		{
			if(3 == nMod)
			{
				*pIns = mod_is_3_dep_ins_map[i].tiModIs3;
				return TRUE;
			}
			else
			{
				*pIns = mod_is_3_dep_ins_map[i].tiModIsNot3;
				return TRUE;
			}
		}
	}

	Assert(FALSE);
	return FALSE;
}

/*******************************************************************/

BOOL GetEffectiveAddressString(	INT nAddrSize, LPSTR lpszEA, 
								RegSet regSet, PAM_DECODE_INFO pdi)
{
	PTABLE_OPERAND pto;
	BYTE sib;
	INT nRegSize;
	BYTE modrm;
	INT Mod;
	LPCSTR lpszEAorReg;
	INT cbDisp;
	CHAR szDisp[MAX_SZ_DISPLACEMENT] = "\0";
	CHAR szSIB[MAX_SZ_SIB] = {0};


	Assert(lpszEA);

	lpszEA[0] = 0;

	pto = &pdi->pTabIns->Operand[pdi->nOperand];

	/*  We need Mod (2 bits) and R/M (3 bits) conbined to give us  */
	/*	an Effective Address row from table in Intel Manual Vol II */
	/*	table 2.1 and 2.2 (16 and 32 bit resp.) so get these now.  */

	sib = *(pdi->pOperStream+1);

	/*  Standard otv... */
	
	nRegSize = gnOperSize;

	
	/*  Special otp... */

	if(pto->Type == otp) 
	{
		nRegSize = (gnOperSize == 16) ? 32 : 48;
	}
	else
	{
		switch(pto->Type)
		{
		case otb:
			nRegSize = 8;
			break;
		case otw:
			nRegSize = 16;
			break;
		case otd:
		case otss:
			nRegSize = 32;
			break;
		case otq:
		case otsd:
		case ots:
			nRegSize = 64;
			break;
		case otdq:
		case otps:
		case otpd:
			nRegSize = 128;
			break;
		}
	}


	/*  Decoration Required?  */
	/*  if its a register ie (mod == 3), then its not a ptr so no */
	/*  decoration... */

	modrm = *pdi->pOperStream;
	Mod = ModRM_GetModValue(modrm);
	if(opfdPtrSizePrefix == pto->dec && Mod != 3)
	{
		if(8 == nRegSize)
		{
			strncpy(lpszEA,kszBytePtrPrefix,MAX_SZ_EA);
		}
		else if(16 == nRegSize)
		{
			strncpy(lpszEA,kszWordPtrPrefix,MAX_SZ_EA);
		}
		else if(32 == nRegSize)
		{
			strncpy(lpszEA,kszDwordPtrPrefix,MAX_SZ_EA);
		}
		else if(48 == nRegSize)
		{
			strncpy(lpszEA,kszFwordPtrPrefix,MAX_SZ_EA);
		}
		else if(64 == nRegSize)
		{
			strncpy(lpszEA,kszQwordPtrPrefix,MAX_SZ_EA);
		}
		else if(128 == nRegSize)
		{
			strncpy(lpszEA,kszDqwordPtrPrefix,MAX_SZ_EA);
		}
		else
		{
			/*  WTF? */
			Assert(FALSE);
			return FALSE;
		}
	}
	
	/*  So Get The Magic String... */
	
	lpszEAorReg = ModRM_GetEffectiveAddressOrRegString(nAddrSize, 
										modrm, nRegSize,regSet);

	
	/*  Could be a Displacement... */

	cbDisp = 0;
	if(16 == gnAddrSize) /* 16 bit EA ModRM encoded */
	{
		if(1 == Mod)
		{
			cbDisp = 1;
		}
		else if	(2 == Mod  || 
				(0 == Mod && 6 == ModRM_GetRMValue(modrm)))
		{
			cbDisp = 2;
		}
	}
	else if(32 == gnAddrSize)	/* 32 bit EA ModRM encoded */
	{
		if(1 == Mod)
		{
			cbDisp = 1;
		}
		else if
			(2 == Mod || 
			(0 == Mod && 5 == ModRM_GetRMValue(modrm)) ||
			(0 == Mod && 4 == ModRM_GetRMValue(modrm) && 
			 5 == SIB_GetBase(sib)))
		{
			cbDisp = 4;
		}
	}


	/*  if so get it... */
	
	if(cbDisp)
	{
		if(!ModRM_GetDisplacementString(gnAddrSize,pdi->pOperStream,
										szDisp,cbDisp << 3))
		{
			/*  TODO: */
			AssertSz(0,"");
			return FALSE;
		}

		(*(pdi->pcbEaten))+=cbDisp; /*  + displacement */
	}
	
	
	/*  Could be an SIB... */

	if(32 == gnAddrSize && (3 != Mod && 4 == ModRM_GetRMValue(modrm)))
	{
		if (0 == Mod && 5 == SIB_GetBase(sib)) /*  v.special case! */
		{
			SIB_GetIndexString(sib, szSIB);
		}
		else
		{
			SIB_GetSIBString(sib, szSIB);
		}
		
		(*(pdi->pcbEaten))++;
	}

	
	/*  Form the complete operand... */
		
	if(3 != Mod)
	{
		strncat(lpszEA,"[",MAX_SZ_EA);

		/*  Segment Override Prefix? */

		if(gnSegPrefix)
		{
			switch(gnSegPrefix)
			{
			case 0x2E:
				strncat(lpszEA,"cs:",MAX_SZ_EA);
				break;
			case 0x36:
				strncat(lpszEA,"ss:",MAX_SZ_EA);
				break;
			case 0x3E:
				strncat(lpszEA,"ds:",MAX_SZ_EA);
				break;
			case 0x26:
				strncat(lpszEA,"es:",MAX_SZ_EA);
				break;
			case 0x64:
				strncat(lpszEA,"fs:",MAX_SZ_EA);
				break;
			case 0x65:
				strncat(lpszEA,"gs:",MAX_SZ_EA);
				break;
			}
		}
	}
		
	strncat(lpszEA,lpszEAorReg,MAX_SZ_EA);
	strncat(lpszEA,szSIB,MAX_SZ_EA);
	
	if(szDisp[0])
	{
		if(szDisp[0] != '-' && (lpszEAorReg[0] || szSIB[0]))
		{
			strncat(lpszEA,"+",MAX_SZ_EA);
		}
		strncat(lpszEA,szDisp,MAX_SZ_EA);
	}

	if(3 != Mod)
	{
		strncat(lpszEA,"]",MAX_SZ_EA);
	}
		
	lpszEA[MAX_SZ_EA-1] = 0;

	return TRUE;
}

/*******************************************************************/

/* NOTE: Depends OperandTypeEnum ... */
/* static */ LPCSTR krgszHardCodeRegOperand[] = 
{
	/*otRegAL,	*/ "al",
	/*otRegCL,	*/ "cl",
	/*otRegDL,	*/ "dl",
	/*otRegBL,	*/ "bl",
	/*otRegAH,	*/ "ah",
	/*otRegCH,	*/ "ch",
	/*otRegDH,	*/ "dh",
	/*otRegBH,	*/ "bh",
	/*otRegAX,	*/ "ax",
	/*otRegDX,	*/ "dx",
	/*otRegFS,	*/ "fs",
	/*otRegGS,  */ "gs",
	/*otRegES,	*/ "es",
	/*otRegCS,	*/ "cs",
	/*otRegSS,	*/ "ss",
	/*otRegDS,	*/ "ds",
	/*otRegeAX	*/ "eax",
	/*otRegeDX	*/ "edx",
	/*otRegeSP	*/ "esp",
	/*otRegeCX	*/ "ecx",
	/*otRegeBX	*/ "ebx",
	/*otRegeBP	*/ "ebp",
	/*otRegeSI	*/ "esi",
	/*otRegeDI	*/ "edi",
	/*otRegEAX,	*/ "eax",
	/*otRegEDX,	*/ "edx",
	/*otRegESP,	*/ "esp",
	/*otRegECX,	*/ "ecx",
	/*otRegEBX,	*/ "ebx",
	/*otRegEBP,	*/ "ebp",
	/*otRegESI,	*/ "esi",
	/*otRegEDI,	*/ "edi",
	/*otFP_ST0,	*/ "st",
	/*otFP_ST1,	*/ "st(1)",
	/*otFP_ST2,	*/ "st(2)",
	/*otFP_ST3,	*/ "st(3)",
	/*otFP_ST4,	*/ "st(4)",
	/*otFP_ST5,	*/ "st(5)",
	/*otFP_ST6,	*/ "st(6)",
	/*otFP_ST7,	*/ "st(7)",
};

#define SIZEOF_HARDCODED_REGS \
	sizeof(krgszHardCodeRegOperand) \
	/ sizeof(krgszHardCodeRegOperand[0])

/*******************************************************************/

LPCSTR GetHardCodedRegString(OperandTypeEnum otReg, INT nOperSize)
{
	LPCSTR lpszReg;

	if(otReg > SIZEOF_HARDCODED_REGS || otReg < 0)
	{
		AssertSz(0,"Reg Operand Out of range...");
		return NULL;
	}

	lpszReg = krgszHardCodeRegOperand[otReg];

	/*  16 bit? -> skip the E */

	if (16 == nOperSize && 
		(otReg == otRegeAX || otReg == otRegeDX ||
		otReg == otRegeSP || otReg == otRegeCX ||
		otReg == otRegeBX || otReg == otRegeBP ||
		otReg == otRegeSI || otReg == otRegeDI))
	{
		lpszReg++;
	}

	return lpszReg;
}

/*******************************************************************/