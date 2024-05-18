//=========================================//
//  Tiny C Compiler Header        "sub.h"  //
//=========================================//
//                     Tan Watanabe, UEC (tan@cs.uec.ac.jp)
//                     Copy right (c), 1997, 2002 all rights reserved.
//                     (Copy is permitted for educational purpose only)
// Revision (June, 2002) //##0206
//   Up to 10 formal parameters are permitted. (REG_param) //##0109
//   Layout of stack frame is changed to permit local variables. //##0206
//   Local variable declarations are permitted. //##0206
//   Class fields are changed from public to protected. //##0206
#pragma SC once
#ifndef SUB
#define SUB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
// Structure of the Tiny C Compiler:
//   source   ==>  Parser  ==>  Rcode   ==> MIPS machine code
//   program              (Abstract Register 
//                           Machine Code)
//   Parser translates source to Rcode represented as a parameter sequence:
//     GenCode(instructionForm, operationCode, resultRegister,
//             operandRegster1, operandRegister2, operandSymbol,
//             operandConstant).
//   Machine dependent part is confined to mips.h & mips.C which convert the
//   Rcode represented as the parameter sequence to MIPS machine code.
//
// Naming rules:
//  	Xxx	     Reserved prefix for a class or group of functions. It is 
//  	         up to 4 characters. First 2~4 characters of class or
//  	         group name are used as its reserved prefix.
//  	xxx.h    Header file containing declarations for the class/group Xxx.
//  	         Several classes may be integrated into one file.
//  	xxx.C    Implementation file corresponding to xxx.h (g++).
//  	xxx.cpp  Implementation file corresponding to xxx.h (Mac c++).
//    XxxYyy   Type name declared in xxx.h and used globally.
//  	XXX_yyy  Macro name declared for the class/group Xxx.
//  	xxx_yyy  Global variable declared for the class/group Xxx.
//  	xxxZzz   Global symbol such as enumeration constant for Xxx.
//  	pYyy     Formal parameter
//  	lZzz     Local variable within a function
//  	i..n     Local variable within a function (lower case single 
//  	         character identifiers i, j, k, l, m, n).
//    Aaaa     Member function name. (No restriction except its initial 
//             character is upper case letter.)
//    XxxZzz   Non-member global function name in the class/gruop Xxx.
//
//========== Common Header ===========//
// Storage size (byte) of basic types
#define COM_sizeInt   4
#define COM_sizePtr   4
extern FILE
    *inFile,      // Source program file
    *prtFile,     // Print out file
    *objFile;     // Object file
typedef char* String;   // Sequence of characters.
// enum Boolean { false, true };  // Mac
typedef bool Boolean;   // For g++ (Change if other compiler is to be used.)
  
//========== String Class ===========//
                                // String table block size.
#define STR_blkSize  496
// All symbol names are stored in String table which increase its size
// according to the amount of names used. Unit of increase is STR_blkSize.
                                //---- String class 
class StringT {
  StringT*  nextBlk;            // Link string blocks.
  String    top;                // Top  of string within the block. 
  String    tail;               // Tail of string within the block.
  char      text[STR_blkSize];  // Strings are stored here.
  static    StringT* first;     // First   StringT instance.
  static    StringT* current;   // Current StringT instance.
public: 
         StringT() {top = &text[0]; tail = &text[STR_blkSize-2]; }
  void   Initiation();          // Initiate string block list.
  String Append(String pStr);   // Append pStr to current block.
                                // If no room, allocate the next block.
};
//-------- Global symbols ----//
  typedef StringT* StringP;     // String instance pointer.
  extern  String   str_null;    // Null string "".

//========== Token Class ===========//
                           // Maximum character length of one token.
#define TKN_lim  64
                           // End line / tab / end file (==-1) code
#define TKN_eol  '\n'
#define TKN_tab  '\t'
#define TKN_eof  EOF
extern  String   tkn_kindName[];  // Convert token kind to token name.
                           //---- Tkn_kindT definitions:
enum Tkn_kindT {  // Token kind 
  tknBlank,       //   Blank
  tknSpec,        //   Special character
  tknCompound,    //   Initial character of compound operator
                  // Above ones are used at token-read time.
  tknEol,         //   End of line
  tknEof,         //   End of file
  tknIntC,        //   Integer constant
  tknKey,         //   Key word such as if, else, while, etc. (reserved)
  tknId,          //   Identifier (Not yet classified)
                  //   (tknId through tknLabel are at first an identifier)
                  // Following ones are used after symbol definition. 
  tknVar,         //   Variable name
  tknParam,       //   Formal parameter
  tknFunc,        //   Function name
  tknType,        //   Basic type name
  tknLabel,       //   Label name
  tknOper,        //   Operator name
  tknReg,         //   Abstract register name
  tknMrg          //   Target machine register name
};
                           //---- Token class
class Token {     
protected:    //   Tokens are usually replaced with new one after //##0206
              //   it is moved to the symbol table.
  char name[TKN_lim];  // Character string of current token.
                       // (Moved to StringT and replaced with new one.)
  int         leng;    // Character length of current token.
  long        value;   // Value if the token is a constant.
  Tkn_kindT   kind;    // Token kind.
  static char tknC;    // Current character 
public: //##0206
            Token   () { tknC = TKN_eol; }
  String    Name    () { return name; }
  int       Leng    () { return leng; }
  long      Value   () { return value; }
  Tkn_kindT Kind    () { return kind; }
  void      Initiation();    // Initiate token class.
  void      Get();           // Get the next token from inFile.
  inline char NextC() {      // Get the next character.
                    tknC = getc(inFile);
                    if (tknC != TKN_eof)
                      putc(tknC, prtFile);
                    return tknC;} 
};                             //---- End of Token class 
//-------- Global symbols ----//
extern Tkn_kindT tkn_chKindT[384]; // Token kind guessed by 1st char.
extern Token     tkn_curr;         // Current token

//========== Symbol Table Class ===========//
                         // Hash table size (Should be a prime number).
#define SYM_hashLim  499
                         //---- Type name definitions:
enum Sym_opCodeT {       // Operation codes of abstract register machine.
  opProg,  opPend,  opSubp,  opEnd, 
  opExport,opImport,opEntry, opLabel, 
  opAdd,   opAdda,  opSub,   opMult,  opDiv,
  opJump,  opCall,  opReturn,opLoad,  opLoada, opStore, 
  opStack,                                               //## 010901
  opComp,  opBreq,  opBrne,  opBrgt,  opBrge,  opBrle,  opBrlt,
  opProlog,opEpilog,opSect,  opDword, opDconst,opDparam
};
class VarInf;          // Forward class declarations.
class FuncInf;
class RegInf;
class Rcode;
class MachineReg;
                         //---- SymbolT class
class SymbolT {         
protected:                 //##0206
  String      name;        // Name string pointer
  SymbolT*    next;        // This links all symbols.
  SymbolT*    hashChain;   // Previous symbol with the same hash key.
                           // Last symbol added is pointed from hash[].
  SymbolT*    vType;       // Value type of the symbol
  int         size;        // Size (bytes) 
  Tkn_kindT   kind;        // Symbol kind
  union {                  // More information
    VarInf*     var;       //    Variable information  (tknVar)
    FuncInf*    func;      //    Function information  (tknFunc)
    RegInf*     reg;       //    Abstract register inf (tknReg)
    MachineReg* mrg;       //    Machine  register inf (tknMrg)
    long        iValue;    //    Integer constant      (tknIntC)
    Sym_opCodeT opCode;    //    Opearation code       (tknOpCode)
  } a1;
//----- Member functions --------//
public: //##0206
              SymbolT  (void) { next = NULL; hashChain = NULL;
                                size = 0;    a1.var    = NULL; } 
  String      Name     (void) { return name; }
  Tkn_kindT   Kind     (void) { return kind; }
  SymbolT*    Next     (void) { return next; } 
  SymbolT*    VType    (void) { return vType; }
  int         Size     (void) { return size; }
  int         IValue   (void) { return a1.iValue; }
  VarInf*     VarP     (void) { if ((kind==tknVar)||(kind==tknParam))
                                     return a1.var; 
                                else return NULL; }
  FuncInf*    FuncP    (void) { return (kind==tknFunc? a1.func:NULL);}
  RegInf*     RegP     (void) { return (kind==tknReg ? a1.reg: NULL);}
  MachineReg* MrgP     (void) { return (kind==tknMrg ? a1.mrg: NULL);}
  Sym_opCodeT OpCode   (void) { return a1.opCode; }
  String      NameStr  (void) { if (this==NULL) return "NULL";
                                else return Name(); }
  String KindName     (void)  { return tkn_kindName[Kind()]; }
  String VTypeName    (void)  { return VType()->NameStr(); }
  void   KindSet      (Tkn_kindT p)   { kind  = p; }
  void   VTypeSet     (SymbolT*  p)   { vType = p; }
  void   SizeSet      (int       p)   { size  = p; }
  void   VarPSet      (VarInf*   p)   { if((kind==tknVar)||
                                       (kind==tknParam)) a1.var=p;}
  void   FuncPSet     (FuncInf*  p)   { if(kind==tknFunc) a1.func=p;}
  void   RegPSet      (RegInf*   p)   { if(kind==tknReg) a1.reg=p;}
  void   MrgPSet      (MachineReg* p) { if(kind==tknMrg) a1.mrg=p;}
  void   IValueSet    (int         p) { a1.iValue = p; }
  void   OpCodeSet    (Sym_opCodeT p) { a1.opCode = p; }
  int    HashKey(String pString, int pLeng) {
           return abs(((((((pLeng<<7) + (unsigned int)pString[0])<<7) + 
             (unsigned int)pString[pLeng>>1])<<7) +
             (unsigned int)pString[pLeng-1]) % SYM_hashLim); }
  int    IsIdentifier() { if ((Kind() >= tknId)&&(Kind() <= tknLabel))
                               return 1;
                          else return 0; } 
  void     Initiation ();
  SymbolT* SearchOrAdd(  // Get symbol table position of pString.
                       String    pString,// Name string of symbol.
                       int       pLeng,  // Length of the name.
                       Tkn_kindT pKind); // Symbol kind of pString.
  SymbolT* Get( // Read a token and return its symbol table ptr.
               char pExpected);// Token kind expected (a: Id, 0: int,
                   // ?: any symbol, other special char: that char.)
  SymbolT* GeneratedVar();     // Get generated variable symbol.
  void     Print       ();     // Print information of a symbol.
  void     PrintAll    ();     // Print all symbols.
  void     Error       (int pErrorCode, String pMessage);

protected: //##0206
//-------- Static variables --------//
  static SymbolT* hash[SYM_hashLim];  // hash[i] points a symbol added 
                               // last and having hash key i.
  static SymbolT* head;        // Head of the symbol table.
  static SymbolT* tail;        // Tail of the symbol table.
public: //##0206
  SymbolT* Set  (String pString, Tkn_kindT pKind);
  SymbolT* SetOp(String pString, Tkn_kindT pKind, Sym_opCodeT pCode);
};  //-- End of SymbolT class --//
//-------- Global symbols --------//
typedef SymbolT* SymbolP;      // Call symbol pointer as SymbolP.
  extern StringT sym_string;   // Hold name string of input symbol.
  extern SymbolT sym_tbl;      // Pre-declared symbol table.
  extern SymbolP sym_curr;     // Current symbol.
  extern SymbolP               // Predefined symbols.
    sym_eof,    sym_lpar,   sym_rpar,  sym_lbracket,sym_rbracket,
    sym_lbrace, sym_rbrace, sym_comma, sym_scolon,  sym_period,
    sym_arrow,  sym_int,    sym_ptr,   sym_void,    sym_extern,  
    sym_if ,    sym_else,   sym_while, sym_return,  sym_add ,
    sym_sub,    sym_mult,   sym_div,   sym_assign,  sym_equal,
    sym_ne,     sym_gt,     sym_ge,    sym_lt,      sym_le,
    sym_main;
  extern String sym_opCodeName[]; // Operaton code name string.
  extern int    sym_tempVar;      // Generated temporal index.

//======== VarInf Class ========//
                           // Variable information
class VarInf {          
protected:           //##0206
    SymbolT* paramChain;   // Next parameter of the same subprogram
    SymbolT* allocReg;     // Registers allocated to a variable
    int      elemCount;    // Array element count (0 for scalar)
    int      disp;         // Displacement within local area //##0206
    int      index;        // Index (param, generated temporal var) 1,2,3,..
    Boolean  localFlag;    // true if local, false if global //##0206
//-------- Member functions --------//
public: //##0206
           VarInf    (void) { paramChain = NULL; allocReg = NULL; 
                              elemCount = 0; disp = 0; index = 0; //##0206
                              localFlag = false; }  //##0206
  int      ElemCount (void) { return elemCount; }
  SymbolT* AllocReg  (void) { return allocReg; }  // for variable
  int      Disp      (void) { return disp; }      // for variable //##0206
  SymbolT* ParamChain(void) { return paramChain; }// for parameter
  int      Index     (void) { return index; }     // for param & temp
  void     ElemCountSet (int p)      { elemCount = p; } // variable
  void     AllocRegSet  (SymbolT* p) { allocReg = p; }  // variable
  void     DispSet      (int p)      { disp = p; } // for local var //##0206
  void     IndexSet     (int p)      { index = p; }     // param & temp
  void     ParamChainSet(SymbolT* p) { paramChain = p; }// parameter 
  Boolean  IsLocal      ()           { return localFlag; } //##0206
  void     LocalFlagSet ()           { localFlag = true; } //##0206
};  //---- VarInf class end
typedef VarInf* VarInfP;
#define TEMP_lim  4  // Number of temporal variables per function //##0206
                     // Should be even number //##0206

//========== Function Class ==========//
                           //---- Function information
class FuncInf { 
protected:           //##0206
    SymbolT* paramChain;   // First parameter of the function
    Rcode*   bodyP;        // Function body ptr. NULL if external.
    int      paramCount;   // Formal parameter count
    int      localDisp;    // Cumulative local variable displacement //##0206
    int      tempCount;    // Number of temporal variables. //##0206

public: //##0206
//-------- Member functions --------//
           FuncInf      (void)      { paramChain=NULL; bodyP=NULL;
                                  paramCount = 0; localDisp = 0;} //##0206
  SymbolT* ParamChain   (void)      { return paramChain; } 
  Rcode*   BodyP        (void)      { return bodyP; }  
  int      ParamCount   (void)      { return paramCount; }  
  void     ParamChainSet(SymbolT* p){ paramChain = p; } 
  void     BodyPSet     (Rcode*   p){ bodyP = p; }
  void     ParamCountSet(int p)     { paramCount = p; }
  int      LocalDisp    ()          { return localDisp; } //##0206
  void     LocalDispAdd (int pSize) { localDisp = localDisp + pSize; } //##0206
  int      TempCount    ()          { return tempCount; }  //##0206
  void     TempCountSet (int p)     { tempCount = p; }     //##0206
};  //---- Function class end
typedef FuncInf* FuncInfP;

//========== Utility Interface ===========//
char *StrSyn(char *pStr1, char *pStr2);
  // Synthesize pStr1 and pStr2 in a work area.
char *StrAppend(char *pStr, char pChar);
  // Append pChar to pStr in a work area.
void StrReverse(char *pStr);     // Reverse the string pStr.
char *Itoaf(int pi);    // Change binary int pi to character string.

#endif

