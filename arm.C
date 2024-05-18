//=============================================//
//  Rcode Class Implementation    "arm.C"    //
//=============================================//
//                             Tan Watanabe, UEC
#include "sub.h"
#include "arm.h"

//============== RegInf Class Implementation =========//

  RegInf reg_tbl;  
  Reg_symP 
    reg_sp,    // Stack pointer register
    reg_ri,    // Return integer value register
    reg_gi0,   // First callee save register
    reg_ti0,   // First caller save register
    reg_fi0,   // First formal parameter integer register
    reg_ai0,   // First actual parameter integer register
    reg_tail[6]  = { NULL, NULL, NULL, NULL, NULL, NULL },
    reg_linkTail = NULL;
  Reg_symP              //## 010901
    reg_ai[REG_param],  //## 010901
    reg_fi[REG_param];  //## 010901
  int    reg_index[12];  // Reg index used for each register kind.
  static String reg_statName[] = {"unused","hold","noHold","saved"};
  
//--------------------//
void RegInf::Initiation()
{
  int i;    //## 010901
  reg_sp  = reg_tail[regSp] = RegGet(regSp); // Head of register chain.
  reg_ri  = reg_tail[regRi] = RegGet(regRi);
  reg_gi0 = reg_tail[regGi] = RegGet(regGi);
  reg_ti0 = reg_tail[regTi] = RegGet(regTi);
  reg_fi0 = reg_tail[regFi] = RegGet(regFi);
  reg_ai0 = reg_tail[regAi] = RegGet(regAi);
  reg_ai[0] = reg_ai0;                         //## 010901
  reg_fi[0] = reg_fi0;                         //## 010901
  for (i = 1; i < REG_param; i++) {            //## 010901
    reg_ai[i] = RegGet(regAi);                 //## 010901
    reg_ai[i-1]->RegP()->regChain = reg_ai[i]; //## 010901
    reg_fi[i] = RegGet(regFi);                 //## 010901
    reg_fi[i-1]->RegP()->regChain = reg_fi[i]; //## 010901
  }                                            //## 010901
} //---- Initiation end ----//

//--------------------//
Reg_symP RegInf::RegGet(  // Get Rcode register of pKind as 
  Reg_kind pKind)         //   a tail register of that kind.
{
  static String lKindName[] = {
    "%gi", "%ti", "%ri", "%fi", "%ai", "%sp" };
  Reg_symP lRegSym, lTail, lTemp;
  RegInfP  lReg;
  int      lNumb;   // Register number relative in the given kind.
  char     lRegName[8];
  
  lTail = reg_tail[pKind];
  lNumb = ((lTail == NULL) ? 0 : lTail->RegP()->regNumb + 1);
  strcat(strcpy(lRegName, lKindName[pKind]),Itoaf(lNumb)); // Make reg name.
  lRegSym = sym_tbl.SearchOrAdd(lRegName, strlen(lRegName), tknReg);
  // lTemp = sym_tbl.GeneratedVar();
  lReg = new RegInf;
  lRegSym->RegPSet(lReg);
  lReg->regKind = pKind;
  lReg->regNumb = lNumb;
  // lReg->MrgSaveSet(lTemp);
  if (lTail != NULL)          // Make chain from previous register.
    lTail->RegP()->regChain = lRegSym;
  if (reg_linkTail != NULL)
    reg_linkTail->RegP()->regLink = lRegSym;  // Link to the tail.
  reg_tail[pKind] = lRegSym;
  reg_linkTail = lRegSym;
  return lRegSym;
} //---- RegGet end ----//

//--------------------//
Reg_symP RegInf::RegAlloc(    // Allocate Rcode register.
         Reg_symP pReg, SymbolP pVar, int pConst, Reg_contents pCont)
{
  Reg_symP lReg, lAlloc;
  int      i;
  
  if (pReg->RegP()->RegStat() == regReserved) // If resereved register,
       lAlloc = pReg;                         //   allocate it.
  else lAlloc = NULL;
  //##030309 if (pVar != NULL) { //## 021219
    for (i = 0, lReg = pReg; // Allocate reg having the same contents if there is.
         (lReg != NULL)&&(lAlloc == NULL)&&(i < REG_numMax); i++) {
      lReg->RegP()->reusable = false;
      if (((pVar != NULL)&&  //##030309
           (lReg->RegP()->BoundVar() == pVar)&&
           (lReg->RegP()->RegCont() != regOther)&& //## 011003
           ((lReg->RegP()->RegCont() == pCont)||
            ((lReg->RegP()->RegValue() == regConstVal)&&
             (pCont == regSymVal)&& 
             (lReg->RegP()->RegStat() != regHold)&& //##030309
             (lReg->RegP()->RegStat() != regUnused))))|| //##030309
          ((pVar == NULL)&&
           (lReg->RegP()->RegCont() == regConstVal)&&
           (lReg->RegP()->RegValue() == pConst)&&
           (lReg->RegP()->RegStat() != regHold)&& //##030309
           (lReg->RegP()->RegStat() != regUnused))) { //##030309
        lAlloc = lReg;       // Allocate lReg having the same contents.
        lAlloc->RegP()->reusable = true;
      }else
        lReg = lReg->RegP()->regChain;     // Examine the next.
    }
  //##030309 } //## 021219
  if (lAlloc == NULL) {
    for (i = 0, lReg = pReg; 
         (lReg != NULL)&&(lAlloc == NULL)&&(i < REG_numMax); i++) { //## 021219
      if ((lReg->RegP()->BoundVar() == NULL)&&  // Find unbounded &
              (lReg->RegP()->RegStat() != regHold)) // no-hold one.
        lAlloc = lReg;
      else 
        lReg = lReg->RegP()->regChain;     // Examine the next.
    } //## 021219
  }
  if (lAlloc == NULL) {                  // Not yet allocated.
    lAlloc = RegGet(pReg->RegP()->RegKind());  // Get a new register.
    lAlloc->RegP()->reusable = false;
  }
  lAlloc->RegP()->RegContSet(pCont);     // Record the contents inf.
  if (pVar != NULL) {
    lAlloc->RegP()->BoundVarSet(pVar);   // Bind a variable
  }else 
    lAlloc->RegP()->RegValueSet(pConst); // or record a value.
  lAlloc->RegP()->RegStatSet(regHold);   // Hold the register.
  return lAlloc;                         // Return the allocated register.
} //---- RegAlloc end ----//

//--------------------//
void RegInf::RegFree(Reg_symP pReg)      // Free pReg.
{
  SymbolP lSym;
  
  lSym = pReg->RegP()->BoundVar();
  if (lSym != NULL)
    lSym->VarP()->AllocRegSet(NULL);
  pReg->RegP()->BoundVarSet(NULL);
  pReg->RegP()->RegContSet(regEmpty);
  pReg->RegP()->RegStatSet(regUnused);
  pReg->RegP()->reusable = false;
} //---- RegFree end ----//

//--------------------//
void RegInf::RegFreeAll()               // Free all registers.
{
  int l;
  Reg_symP lReg;
  
  for (lReg = reg_sp; lReg != NULL; lReg = lReg->RegP()->regLink)
    RegFree(lReg);
} //---- RegFreeAll end ----//

//--------------------//
void RegInf::Print()   // Print symbol information for debug.
{
  fprintf(prtFile, "rc %s ma %s bv %s ms %s ", regChain->NameStr(),
    MrgAlloc()->NameStr(), BoundVar()->NameStr(), MrgSave()->NameStr());
} //---- Print end ----//

//========== Rcode Implementations ==========//

//--------------------//
void GenCode(      // Generate Rcode and corresponding machine code.
     Ac_form  pForm, Sym_opCodeT pOpCode,
     Reg_symP pReg1, Reg_symP    pReg2, Reg_symP pReg3,
     SymbolP  pSym,  int pInt) 
{
  int lComma;

  // Generate machine code to objFile.
  MachineCode(pForm, pOpCode, pReg1, pReg2, pReg3, pSym, pInt);
  // Print Rcode to prtFile.
  if (pOpCode == opLabel) {    
    fprintf(prtFile, "\n %s: ", pSym->NameStr());
    return;
  }
  lComma = false;
  fprintf(prtFile, "\n         %s  ", sym_opCodeName[pOpCode]);
  if (pReg1 != NULL) {
    fprintf(prtFile, "%s", pReg1->NameStr());
    lComma = true;
  }
  if (pReg2 != NULL) {
    if (lComma)
      fprintf(prtFile, ",");
    fprintf(prtFile, "%s", pReg2->NameStr());
    lComma = true;
  }
  if (pReg3 != NULL) {
    if (lComma)
      fprintf(prtFile, ",");
    fprintf(prtFile, "%s", pReg3->NameStr());
    lComma = true;
  }
  if (pSym != NULL) {
    if (lComma)
      fprintf(prtFile, ",");
    fprintf(prtFile, "%s", pSym->NameStr());
    lComma = true;
  }
  switch (pForm) {
  case acRc:
  case acSc:
  case acRrc:
  case acRsc:
    fprintf(prtFile, ",%d", pInt);
    break;
  case acC:
    fprintf(prtFile, "%d", pInt);
    break;
  default:
    break;
  }
  fprintf(prtFile, ";  %c", TKN_tab);
} //---- GenCode end ----//

//--------------------//
void Rcode::Initiation()
{
  ac_labelGenIndex = 0;  // Reset the index of generated labels.
  ac_curr = &ac_head;
} //---- Initiation end ----//

//--------------------//
SymbolP Rcode::LabelGen()    // Generate a label of a form "_labxxx"
{                              //   where, xxx is 001, 002, 003, ...
  char    lLabelName[12];
  SymbolP lSym;
  
  ac_labelGenIndex++;
  strcat(strcpy(lLabelName, "_lab"), Itoaf(ac_labelGenIndex));
  lSym = lSym->SearchOrAdd(lLabelName, strlen(lLabelName), tknLabel);
  return lSym;
} //---- LabelGen end ----//

//--------------------//
Reg_symP Rcode::Load( // Load value or address of variable/parameter.
  SymbolP  pVar,        //   Variable or parameter symbol.
  Reg_symP pCand,       //   Candidate register.
  Reg_symP pIndexReg,   //   Index regiser (unscaled). NULL if none.
  int      pValue,      //   Displacement.
  Ac_lrT   pType)       //   lValue or rValue.
{
  Reg_symP     lReg, lSymAddr, lVarAddr, lCand;
  Reg_contents lCont;
  Sym_opCodeT  lOpCode;
  
  if (pIndexReg == NULL) { // Const, simple var or const subscripted var.
    if (pVar == NULL) {    // Constant.
      lReg = reg_tbl.RegAlloc(pCand, NULL, pValue, regConstVal);
      if (lReg->RegP()->reusable == false)  // Omit if reusable.
        GenCode(acRc, opLoad, lReg, NULL, NULL, NULL, pValue);
    }else {    // Simple variable without subscript or constant-subscripted var.//##011003
      if (pVar->VarP()->ElemCount() <= 1) { // Scalar var/param. //## 011003
        if (pType == lValue) {  // Left value. Load address.
          lCont   = (pValue == 0 ? regSymAddr : regOther);
          lOpCode = (pVar->Kind() == tknParam ? opLoad : opLoada);
          lSymAddr = reg_tbl.RegAlloc(pCand, pVar, -1, lCont); //##030309
          if (lSymAddr->RegP()->reusable == false) 
            GenCode(acRsc, lOpCode, lSymAddr, NULL, NULL, pVar, pValue);
          return lSymAddr;
        } // lValue end
        lCont = (pValue == 0 ? regSymVal : regOther);
        lReg = reg_tbl.RegAlloc(pCand, pVar, pValue, lCont);
        if (lReg->RegP()->reusable == false) // Omit if reusable. //## 011003
          GenCode(acRsc, opLoad, lReg, NULL, NULL, pVar, pValue);
      }else { // Array element  //## 011003
        if (pType == lValue) {  // Left value. Load address. //## 011003
          lReg = reg_tbl.RegAlloc(pCand, NULL, -1, regOther);  //##030309 011003
          GenCode(acRsc, opLoada, lReg, NULL, NULL, pVar, pValue); //## 011003
        }else { //## 011003
          lVarAddr = reg_tbl.RegAlloc(reg_gi0, NULL, -1, regOther); //##030309 011003
          GenCode(acRsc, opLoada, lVarAddr, NULL, NULL, pVar, pValue); //## 011003
          lReg = reg_tbl.RegAlloc(pCand, NULL, -1, regOther);  //##030309 011003
          GenCode(acRr, opLoad, lReg, lVarAddr, NULL, NULL, 0); //## 011003
        }
      }
    } // end of simple var or with constant subscriot.
  }else {   // Subscripted var with non-constant index.
    lCont = (pValue == 0 ? regSymAddr : regOther);
    lSymAddr = reg_tbl.RegAlloc(reg_gi0, pVar, pValue, lCont);
    if (lSymAddr->RegP()->reusable == false) 
      GenCode(acRsc, opLoada, lSymAddr, NULL, NULL, pVar, pValue);
    lCand = (pType == lValue ? pCand : reg_gi0);
    lVarAddr = reg_tbl.RegAlloc(lCand, NULL, -1, regOther); //##030309
    GenCode(acRrr, opAdda, lVarAddr, lSymAddr, pIndexReg, NULL, 0);
    if (pType == lValue)
      return lVarAddr;
    lReg = reg_tbl.RegAlloc(pCand, NULL, -1, regOther); //##030309
    GenCode(acRr, opLoad, lReg, lVarAddr, NULL, NULL, 0);
  }
  return lReg;  // Return the register containing value/address of variable.
} //---- Load end ----//

//--------------------//
void Rcode::Store(    // Store pRightPart to pLeftPart.
  SymbolP  pLeftPart,   //   Address register or simple variable.
  Reg_symP pRightPart)  //   Register containing right part exp.
{
  if (pLeftPart->Kind() == tknReg)
    GenCode(acRr, opStore, pRightPart, pLeftPart, NULL, NULL, 0);
  else { // Left part is a simple variable
    GenCode(acRs, opStore, pRightPart, NULL, NULL, pLeftPart, 0);
    //## 021219 BEGIN
    if ((pLeftPart->Kind() == tknVar)||(pLeftPart->Kind() == tknParam)) {
      Reg_symP lReg; // Unbound the variable from other registers.
      for (lReg = reg_sp; lReg != NULL; lReg = lReg->RegP()->regLink) {
        if ((lReg->RegP()->BoundVar() == pLeftPart)&&
            (pRightPart != lReg))
          lReg->RegP()->BoundVarSet(NULL); 
      }
    }
    //## 021219 END
    pRightPart->RegP()->BoundVarSet(pLeftPart);
    pRightPart->RegP()->RegContSet(regSymVal);
  }
} //---- Store end ----//

//====== Global symbols ======//
  Rcode  ac_head;
  RcodeP ac_curr;
  int      ac_labelGenIndex; // Generated label index.


