//=========================================================//
//   Abstract Register Machine Code (Rcode) Class "arm.h"  //
//=========================================================//
//                              Tan Watanabe, UEC
#pragma SC once
#ifndef ARM
#define ARM
#include "sub.h"

//========== RegInf Class ==========//
                                  //---- Register information
#define REG_numMax   15
#define REG_param    10 // Allowable number of parameters //## 010901
                        // Should be even number          //##0206
#define REG_paramReg  4 // Number of parameter registers  //## 010901
#define TEMP_index    4 // Number of generated temporals  //## 0206
enum Reg_kind {    // Abstract register kind
  regGi,           //   callee save      integer
  regTi,           //   caller save      integer
  regRi,           //   return value     integer
  regFi,           //   formal parameter integer
  regAi,           //   actual parameter integer
  regSp            //   special register
}; 
enum Reg_statT {   // Register status type
  regUnused,       //   Not yet used (availabel). -->Hold, -->Reserved
  regReserved,     //   Reserved for a var/param. -->Hold
  regHold,         //   Holded for some use.      -->NoHold
  regNoHold,       //   Used later.               -->Unused, -->Saved
  regSaved         //   Saved in a temporal.      -->Hold
};
enum Reg_contents {// Register contents
  regEmpty,        //   Empty
  regSymVal,       //   Symbol (variable, etc.) value
  regSymAddr,      //   Symbol (variable, etc.) address
  regConstVal,     //   Constant value
  regOther         //   Others (computed value, etc.)
};
                            // Register class definition
class RegInf {
public:
  SymbolP      boundVar;    // Variable bounded to this register
  SymbolP      regChain;    // Next register for allocation search
  SymbolP      mrgAlloc;    // Allocated machine register
  SymbolP      mrgSave;     // Temporal variable to save this reg
  SymbolP      regLink;     // Pointer to link all registers.
  int          regValue;    // Constant value contained in the register.
  int          regNumb;     // Sequence number for each reg kind
  Reg_kind     regKind;     // Register kind
  Reg_statT    regStat;     // Register status
  Reg_contents regCont;     // Register contents
  Boolean      reusable;    // True if requested value is contained.
  //----- Member functions --------//
               RegInf(void) { boundVar = NULL; regChain = NULL;
                 mrgAlloc = NULL; mrgSave = NULL; regLink = NULL;
                 regStat = regUnused; regCont = regOther; 
                 reusable = false; } 
  SymbolP      BoundVar(void) { return boundVar; }
  SymbolP      RegChain(void) { return regChain; }
  SymbolP      MrgAlloc(void) { return mrgAlloc; }
  SymbolP      MrgSave (void) { return mrgSave; }
  int          RegNumb (void) { return regNumb; }
  Reg_kind     RegKind (void) { return regKind; }
  Reg_statT    RegStat (void) { return regStat; }
  Reg_contents RegCont (void) { return regCont; }
  int          RegValue(void) { return regValue; }
  void BoundVarSet(SymbolP p)      { boundVar = p; }
  void MrgAllocSet(SymbolP p)      { mrgAlloc = p; }
  void MrgSaveSet (SymbolP p)      { mrgSave  = p; }
  void RegStatSet (Reg_statT p)    { regStat  = p; }
  void RegContSet (Reg_contents p) { regCont  = p; }
  void RegValueSet(int          p) { regValue = p; }
  void     Initiation(void);
  SymbolP  RegGet    (Reg_kind pKind); // Get Rcode register.
  SymbolP  RegAlloc  (            // Allocate Rcode register.
                      SymbolP pReg,    // Candidate register.
                      SymbolP pVar,    // Variable bound to the reg.
                      int     pConst,  // Constant value to be loaded (nonnegative).
                                       // -1 if no constant value is given. //##030309
                      Reg_contents pCont); // Contents kind.
  void     RegFree   (SymbolP pReg);   // Free the register pReg.
  void     RegFreeAll();               // Free all registers.
  void     Print     ();               // Print information for debug.
};             // Register class end
//-------- Global symbols -------//
typedef SymbolP Reg_symP;              // Register symbol pointer
typedef RegInf* RegInfP;               // Register information pointer
extern  RegInf  reg_tbl;               // Register information table
extern  Reg_symP  // 1st register for each register kind.
  reg_sp,         // Stack pointer register
  reg_ri,         // Return value register for integer 
  reg_gi0,        // 1st callee save integer register
  reg_ti0,        // 1st caller save integer register
  reg_fi0,        // 1st formal parameter integer register
  reg_ai0,        // 1st actual parameter integer register
  reg_tail[6],    // Tail register of each kind.
  reg_linkTail;   // Tail of the all register link.
extern  Reg_symP      //## 010901
  reg_ai[REG_param],  //## 010901
  reg_fi[REG_param];  //## 010901

//============== Rcode Class ===========//

enum Ac_form {  // Rcode operation format
  acN,      // no operand
  acR,      // reg1                %dest or %src
  acS,      // sym1                sym
  acC,      // const1              const
  acRr,     // reg1,reg2           %dest,%src
  acRs,     // reg1,sym2           %dest,sym or %src,sym  
  acRc,     // reg1,const2         %dest,const
  acSc,     // sym1,const2         sym+const
  acRrr,    // reg1,reg2,reg3      %dest,%src1,%src2
  acRrs,    // reg1,reg2,sym2      %dest,sym+%index or %src,sym+%index
  acRrc,    // reg1,reg2,const2    %dest,%src,const
  acRsc     // reg1,sdisp1,sym2    %dest,sym+sdisp or %src,sym+sdisp
            // where,
            //   %dest    destination register
            //   %src     source register
            //   %base    base register
            //   %index   index register
            //   sym      symbol (variable, subprogram, etc.) or label
            //   sdisp    short displacement 
            //   const    constant value or long displacement
};          // Ac_form end
enum Ac_lrT {   // Value kind
  lValue,       //   Left  hand value (address)
  rValue        //   Right hand value (value)
};              // Ac_lrT end
enum Ac_section {
  acSectText,           //   Text (code) section
  acSectData            //   Data section
};

class Rcode {           // Rcode class
public:
  Sym_opCodeT opCode;   // Operation code
  Ac_form     form;     //   Instruction format
  Reg_symP    reg1;     //   Register 1
  Reg_symP    reg2;     //   Register 2
  Reg_symP    reg3;     //   Register 3
  SymbolP     sym;      //   Symbol pointer
  int         iConst;   //   Integer value
//----- Member Functions -----//
  Sym_opCodeT OpCode (void) { return opCode; }
  Ac_form   Form   (void) { return form; }
  Reg_symP  Reg1   (void) { return reg1; }
  Reg_symP  Reg2   (void) { return reg2; }
  Reg_symP  Reg3   (void) { return reg3; }
  SymbolP   Sym    (void) { return sym; }
  int       IConst (void) { return iConst; }
  String    OpCodeName()  { return sym_opCodeName[OpCode()]; }
  String    FormName  (void);
  String    RegName   (Reg_symP    p);  
  void      OpCodeSet (Sym_opCodeT p) { opCode = p; }
  void      FormSet   (Ac_form     p) { form = p; }
  void      Reg1Set   (Reg_symP    p) { reg1 = p; }
  void      Reg2Set   (Reg_symP    p) { reg2 = p; }
  void      Reg3Set   (Reg_symP    p) { reg3 = p; }
  void      SymSet    (SymbolP     p) { sym  = p; }
  void      IConstSet (int         p) { iConst = p; }
  void      Initiation();
  SymbolP   LabelGen  ();   // Generate a label.
  Reg_symP  Load( // Load value or address of variable/parameter.
                 SymbolP  pVar,      // Variable/parameter symbol.
                 Reg_symP pCand,     // Candidate register.
                 Reg_symP pIndexReg, // Index register (unscaled). 
                 int      pValue,    // Displacement.
                 Ac_lrT   pType);    // lValue or rValue.
  void      Store(          // Store pRightpart to pLeftPart.
                  SymbolP  pLeftPart,  // Addr reg or simple var.
                  Reg_symP pRightPart);// Register containing exp.
};  //---- Rcode class end ----//

//====== Global symbols ======//
  typedef Rcode*   RcodeP;
  extern  Rcode    ac_head;          // Head    node of Rcode
  extern  RcodeP   ac_curr;          // Current node of Rcode.
  extern  int      ac_labelGenIndex; // Generated label index.
  void GenCode(                      // Generate an Rcode instruction.
               Ac_form pForm,        // Instruction format.
               Sym_opCodeT pOpCode,  // Operation code
               Reg_symP pReg1, Reg_symP pReg2, Reg_symP pReg3, //operand
               SymbolP pSym,         // operand symbol.
               int pInt);            // Operand integer value.
  void MachineCode(              // Generate a machine code instruction.
               Ac_form pForm, Sym_opCodeT pOpCode, Reg_symP pReg1,
               Reg_symP pReg2, Reg_symP pReg3, SymbolP pSym, int pInt);

#endif
