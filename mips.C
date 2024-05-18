//=================================================//
//   Mips Object Code Implementation    "mips.C"   //
//=================================================//
//                                  Tan Watanabe, UEC
#include "sub.h"
#include "arm.h"
#include "mips.h"

#define PARAM_areaSize 24 // (REG_param - REG_paramReg)*4  // Stacked parameter area size //## 010901
MachineReg mrg_reg;
Reg_symP 
  mrg_mips[32],  // Mips machine register pointer
  mrg_zero,   mrg_rval, mrg_param, mrg_temp, mrg_saved, 
  mrg_stackp, mrg_raddr;
String mipsOpCode[] = { // Mips operation code corresponding to Rcode.
  //opProg,   opPend,   opSubp,   opEnd, 
   "       ","       ","       ",".end   ",
  //opExport, opImport, opEntry,  opLabel, 
   ".globl ",".extern",".ent   ","       ",
  //opAdd,    opAdda,   opSub,    opMult,   opDiv,
   "add    ","addu   ","sub    ","mul    ","div    ",
  //opJump,   opCall,   opReturn, opLoad,   opLoada,  opStore, 
   "j      ","jal    ","j      ","lw     ","la     ","sw     ",
  //opStack,                                //## 010901
   "sw     ",                               //## 010901
  //opComp,   opBreq,   opBrne,   opBrgt,   opBrge,   opBrle,   opBrlt,
   "       ","beq    ","bne    ","bgt    ","bge    ","ble    ","blt    ",
  //opProlog, opEpilog, opSect,   opDword,  opDconst, opDparam
   "       ","       ","       ",".comm  ","       ","       " 
};

//##0206 BEGIN
int              // Stack frame layout parameters (see mips.h) 
  cgRegSaveBase, // Register save area base 
  cgTempBase,    // temporal area base 
  cgLocalBase,   // Local variable area base
  cgFrameSize,   // Stack frame size
  cgFrameOffset; // Offset of return address save area 
                 // from frame pointer (stack pointer of parent).
//##0206 END
  
//--------------------//
Reg_symP MachineReg::MrgInit(String pRegName, int pRegNumb)
{                            // Initiate the specified machine register.
  Reg_symP    lRegSym;
  MachineRegP lReg;
  
  lRegSym = sym_tbl.SearchOrAdd(pRegName, strlen(pRegName), tknMrg);
  lReg    = new MachineReg;
  lRegSym->MrgPSet(lReg);
  lReg->mrgNumb = pRegNumb;
  return lRegSym;
} //---- RegInit end ----//
  
//--------------------//
void MachineReg::Initiation()  // Initiate all machine registers.
{                              // Registers of the same kind make a ring.
  int li;
  Reg_symP lRegSym, lRegx, lRegy;
  
  mrg_mips[31] = MrgInit("$31", 31);
  for (li = 30; li >= 0; li--) {  // Generate machine registers.
    mrg_mips[li] = MrgInit(StrSyn("$",Itoaf(li)), li);
    mrg_mips[li]->MrgP()->mrgChain = mrg_mips[li+1];
  }
  mrg_zero   = mrg_mips[0];   // Zero reg.
  mrg_rval   = mrg_mips[2];   // Return value reg.
  mrg_param  = mrg_mips[4];   // Parameter reg list head.
  mrg_temp   = mrg_mips[8];   // Temporal reg list head.
  mrg_saved  = mrg_mips[16];  // Callee save reg list head.
  mrg_stackp = mrg_mips[29];  // Stack pointer.
  mrg_raddr  = mrg_mips[31];  // Return address reg.
  mrg_mips[ 7]->MrgP()->mrgChain = mrg_param;    // Ring of param reg.
  mrg_mips[15]->MrgP()->mrgChain = mrg_mips[24]; // Ring of temporal reg.
  mrg_mips[23]->MrgP()->mrgChain = mrg_saved;    // Ring of saved reg.
  mrg_mips[25]->MrgP()->mrgChain = mrg_temp;     // Ring of temporal reg.
} //---- Initiation end ----//

//--------------------//
Reg_symP MachineReg::MrgSearch( // Search machine reg to be allocated.
  Reg_symP pArmReg,    // Rcode register
  int      pLevel)     // 1: Search unused reg, 2: search noHolg reg.
{
  Reg_symP    lReg, lMrgReg;
  MachineRegP lMrgInf;
  int         lc;          // Search count
  
  if (pArmReg->RegP()->MrgAlloc() != NULL)  
    lReg = pArmReg->RegP()->MrgAlloc();    // Set search start point.
  else if (pArmReg->RegP()->RegKind() == regTi)
       lReg = mrg_temp;    // Search point for temporal register.
  else lReg = mrg_saved;   // Search point for other register.
  lMrgReg = NULL;
  for (lc = 1; (lMrgReg == NULL)&&(lc <= MRG_searchLim); lc++) {
    lMrgInf = lReg->MrgP();  
    if (pLevel == 1) {     // Use unused or already allocated reg.
      if ((lMrgInf->ArmReg() == NULL)||(lMrgInf->ArmReg() == pArmReg)||
          (lMrgInf->ArmReg()->RegP()->RegStat() == regUnused))
        lMrgReg = lReg;      // Unused register is found.
    }else if (pLevel == 2) { // Use noHold register.
      if (lMrgInf->ArmReg()->RegP()->RegStat() == regNoHold) {
        reg_tbl.RegFree(lMrgInf->ArmReg()); //##011210 Murasaki
        lMrgReg = lReg;  
      }
    }
    lReg = lMrgInf->mrgChain;  // Advance to the next one in the register ring.
  }
  return lMrgReg;
} //---- MrgSearch end ----//

//--------------------//
Reg_symP MipsReg(       // Allocate Mips register to Rcode register.
  Reg_symP  pArmReg,    //   Rcode register for allocation.
  Mrg_usage pUsage)     //   Use mode or set mode.
{
  Reg_symP    lReg, lMrgReg, lArmReg, lSaveReg;
  MachineReg* lMrgInf;
  int         lSearch;
  
  if (pArmReg == NULL)
    return NULL;
  if (pArmReg->Kind() == tknMrg) // It is already a machine register.
    return pArmReg;
  if (((pArmReg->RegP()->RegKind() == regAi)|| // Parameter register.//## 010901
       (pArmReg->RegP()->RegKind() == regFi))&&  //## 010901
      (pArmReg->RegP()->RegNumb() < REG_param))  //## 010901
    lMrgReg = mrg_mips[4+pArmReg->RegP()->RegNumb()];
  else if ((pArmReg->RegP()->MrgAlloc() == NULL)|| // Not yet allocated.
      (pArmReg->RegP()->MrgAlloc()->MrgP()->ArmReg() != pArmReg)) {
    lMrgReg = mrg_reg.MrgSearch(pArmReg, 1);    // Search free reg.
    if (lMrgReg == NULL) {  // Not found. Snatch some register.
      lMrgReg = mrg_reg.MrgSearch(pArmReg, 2);  // Search noHold reg.
      // A register that is still noHold is not used at present.
      //lSaveReg = lMrgReg->MrgP()->ArmReg();
      //MachineCode(acRs, opStore, lMrgReg, NULL, NULL, 
      //            lSaveReg->RegP()->MrgSave(), 0);
      //lSaveReg->RegP()->RegStatSet(regSaved);
    }
  }    // (pArmReg->RegP()->MrgAlloc()->MrgP()->ArmReg() != pArmReg) end
  else // Already allocated. Use it.
    lMrgReg = pArmReg->RegP()->MrgAlloc();
  lMrgReg->MrgP()->ArmRegSet(pArmReg);
  //if ((pUsage == mrgUse)&&(pArmReg->RegP()->RegStat() == regSaved)) 
  //  MachineCode(acRs, opLoad, lMrgReg, NULL, NULL, 
  //              pArmReg->RegP()->MrgSave(),0);
  pArmReg->RegP()->MrgAllocSet(lMrgReg);
  pArmReg->RegP()->RegStatSet(regHold);
  //fprintf(prtFile," ALLOC %s to %s ",lMrgReg->NameStr(),pArmReg->NameStr());
  return lMrgReg;
}  //---- MipsReg end ----//

//--------------------//
String MipsLab(    // Change Rcode label to Mips label.
  SymbolP pLab)    //   Rcode label.
{
  static char lName[TKN_lim];
  
  strcpy(lName, pLab->NameStr());
  if (lName[0] == '_')  // Change "_labxxx"
    lName[0] = '$';     //     to "$labxxx".
  return lName;
} //---- MipsLab end ----//

//--------------------//
void MachineReg::Print()   // Print machine reg information for debug.
{
  fprintf(prtFile, "rc %s ar %s mn %d ", mrgChain->NameStr(),
    ArmReg()->NameStr(), mrgNumb);
} //---- Print end ----//

//--------------------//
void MachineCode(Ac_form pForm, Sym_opCodeT pOpCode,Reg_symP pReg1, 
     Reg_symP pReg2, Reg_symP pReg3, SymbolP pSym, int pInt) 
{                 // Generate MIPS machine code.
  Reg_symP        lReg1, lReg2, lReg3;
  static Reg_symP lSym, lRegPrev1, lRegPrev2, lRegPrev3, lLabel = NULL;
  static String   lOpName, lOperand, lSymName;
  
  if ((lLabel != NULL)&&(pOpCode != opDword)&&(pOpCode != opDparam))
    fprintf(objFile, "\n%s:", MipsLab(lLabel));  // Previous label.
  if (pOpCode == opLabel) {  // Record the label in order to output it
    lLabel = pSym;           // in front of the next instruction.
    return;
  }
  lOpName = mipsOpCode[pOpCode];
  lReg1 = MipsReg(pReg1, (pOpCode==opStore?mrgUse:mrgSet));
  lReg2 = MipsReg(pReg2, mrgUse);
  lReg3 = MipsReg(pReg3, mrgUse);
  if (pReg2 != NULL)
    pReg2->RegP()->RegStatSet(regNoHold);
  if (pReg3 != NULL)
    pReg3->RegP()->RegStatSet(regNoHold);
  switch (pOpCode) {
  case opLoad:  
    if (pForm == acRc)
      lOpName = "li     ";
  case opLoada: 
    if ((pSym != NULL)&&(pSym->Kind() == tknParam)&&(pOpCode == opLoada))
      lOpName = mipsOpCode[opLoad];
  case opStore:
    fprintf(objFile, "\n         %s  %s", lOpName, lReg1->NameStr());
    if (pSym == NULL) {
      if (pReg2 == NULL) 
           fprintf(objFile, ",%d", pInt);
      else fprintf(objFile, ",%d(%s)", pInt, lReg2->NameStr());
    }else { // pSym != NULL
      lSymName = pSym->NameStr();
      if (pSym->Kind() == tknParam) 
        lSymName = StrSyn(Itoaf((pSym->VarP()->Index()-1)*4+cgFrameSize),"($sp)");  //##0206
        //##0206 lSymName = StrSyn(Itoaf((pSym->VarP()->Index()-1)*4+56+PARAM_areaSize),"($sp)");  //## 010901
      if (pReg2 == NULL) {
        if (pInt == 0) { /* If subscript is 0, omit subscript computation. */
          if (pSym->VarP()->IsLocal()) //##0206
            fprintf(objFile, ",%d($sp)", pSym->VarP()->Disp()+cgLocalBase); //##0206
          else  //##0206
            fprintf(objFile, ",%s", lSymName);
        }else {    // With constant subscript
          if (pSym->Kind() == tknVar) {
            if (pSym->VarP()->IsLocal()) //##0206
              fprintf(objFile, ",%d($sp)", pSym->VarP()->Disp()+cgLocalBase); //##0206
            else  //##0206
              fprintf(objFile, ",%s+%d", lSymName, pInt);
          }else {  // Parameter with constant subscript.
            fprintf(objFile, ",%s", lSymName);
            fprintf(objFile, "\n         addu     %s,%s,%d", 
                    lReg1->NameStr(),lReg1->NameStr(), pInt);
          }
        } // pInt != 0
      }  // pReg2 == NULL
      else { // With subscript register
        if (pSym->Kind() == tknVar) {
          if (pSym->VarP()->IsLocal()) //##0206
            fprintf(objFile, ",%d($sp+%s)", pSym->VarP()->Disp()+cgLocalBase, lReg2->NameStr()); //##0206
          }else  //##0206
            fprintf(objFile, ",%s(%s)", lSymName, lReg2->NameStr());
      }
    }
    pReg1->RegP()->RegStatSet((pOpCode==opStore ? regNoHold : regHold));
    break;
  case opStack:          //## 010901
    fprintf(objFile, "\n         %s  %s", lOpName, lReg1->NameStr()); //## 010901
    fprintf(objFile, ",%d($sp)", pInt);
    //##0206 fprintf(objFile, ",%d($sp)", pInt);
    break;
  case opAdda:
    fprintf(objFile, "\n         sll      %s,%s,2", 
            lReg3->NameStr(), lReg3->NameStr());
    pReg3->RegP()->RegContSet(regOther);
  case opAdd:
  case opSub:
  case opMult:
  case opDiv:
    fprintf(objFile, "\n         %s  %s,%s,%s", lOpName,
            lReg1->NameStr(), lReg2->NameStr(), lReg3->NameStr());
    pReg1->RegP()->RegStatSet(regHold);
    pReg1->RegP()->RegContSet(regOther);
    break;
  case opComp:
    lRegPrev2 = lReg2;
    lRegPrev3 = lReg3;
    pReg1->RegP()->RegStatSet(regHold);
    break;
  case opBreq:
  case opBrne:
  case opBrgt:
  case opBrge:
  case opBrle:
  case opBrlt:
    fprintf(objFile, "\n         %s  %s,%s,%s", lOpName,
      lRegPrev2->NameStr(), lRegPrev3->NameStr(), MipsLab(pSym));
    pReg1->RegP()->RegStatSet(regNoHold);
    break;
  case opJump:
    fprintf(objFile, "\n         %s  %s", lOpName, MipsLab(pSym));
    break;
  case opImport:
  case opExport:
  case opEntry:
  case opEnd:
  case opCall:
    fprintf(objFile, "\n         %s  %s", lOpName, pSym->NameStr());
    break;
  case opReturn:
    fprintf(objFile, "\n         j        $31");
    break;
  case opProlog:
    //##0206 BEGIN
    cgRegSaveBase = REG_param * 4;
    cgTempBase    = cgRegSaveBase + 36; 
    cgLocalBase   = cgTempBase + TEMP_lim * 4;
    cgFrameSize   = cgLocalBase + pSym->FuncP()->LocalDisp();
    cgFrameSize   = ((cgFrameSize + 7) / 8) * 8; // multiple of 8
    cgFrameOffset = cgFrameSize - (cgRegSaveBase + 32); //##020615
                    // Offset of return addr save area from frame pointer.
    fprintf(prtFile,
       "\n %s regSaveBase %d tempBase %d localBase %d frameSize %d localDisp %d\n",
       pSym->NameStr(), cgRegSaveBase, cgTempBase, 
       cgLocalBase, cgFrameSize, pSym->FuncP()->LocalDisp());
    fprintf(objFile, "\n         .ent     %s", pSym->NameStr());
    fprintf(objFile, "\n%s:", pSym->NameStr());
    fprintf(objFile, "\n         subu     $sp,%d", cgFrameSize); // Push stack
    fprintf(objFile, "\n         sw       $31,32+%d($sp)", cgRegSaveBase); // Return addr //##020615
    fprintf(objFile, "\n         sd       $22,24+%d($sp)", cgRegSaveBase); // Callee save
    fprintf(objFile, "\n         sd       $20,16+%d($sp)", cgRegSaveBase);
    fprintf(objFile, "\n         sd       $18,8+%d($sp)", cgRegSaveBase);
    fprintf(objFile, "\n         sd       $16,0+%d($sp)", cgRegSaveBase);
    fprintf(objFile, "\n         sd       $6,8+%d($sp)", cgFrameSize);
    fprintf(objFile, "\n         sd       $4,0+%d($sp)", cgFrameSize);
    fprintf(objFile, "\n         .frame   $sp,%d,$31", cgFrameSize);
    fprintf(objFile, "\n         .mask    0x80ff0000,-%d", cgFrameOffset);
    //##0206 END
    break;
  case opEpilog:
    if (pReg1 != NULL)           // Return value
      fprintf(objFile, "\n         addu     $2,%s,$0", lReg1->NameStr());
    //##0206 BEGIN
    fprintf(objFile, "\n         lw       $31,32+%d($sp)", cgRegSaveBase); // Return addr //##020615
    fprintf(objFile, "\n         ld       $22,24+%d($sp)", cgRegSaveBase); // Callee save
    fprintf(objFile, "\n         ld       $20,16+%d($sp)", cgRegSaveBase);
    fprintf(objFile, "\n         ld       $18,8+%d($sp)", cgRegSaveBase);
    fprintf(objFile, "\n         ld       $16,0+%d($sp)", cgRegSaveBase);
    fprintf(objFile, "\n         addu     $sp,%d", cgFrameSize); // Pop stack
    //##0206 END
    break;
  case opSect:
    if (pInt == acSectText)
         fprintf(objFile, "\n         .text");
    else fprintf(objFile, "\n         .sdata");
    break;
  case opDword:
    fprintf(objFile, "\n         .comm    %s,%d",
            lLabel->NameStr(), pInt);
    break;
  case opSubp:
  case opLabel: 
  case opDconst: case opDparam: case opProg: case opPend: 
  default:
    break;
  } //---- switch(pOpCode) end
  lLabel = NULL;
} //---- MachineCode end ----//

