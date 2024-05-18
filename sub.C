//=====================================================//
//   StringT, Token, SymbolT Implementation  "sub.C"   //
//=====================================================//
#include "sub.h"
#include "arm.h"
#include "mips.h"
//                                               Tan Watanabe, UEC
//========== StringT Class Implementation ==========//

//---- Globals and statics ----//
  String  str_null;            // Null string ("").
  StringP StringT::first;      // First   StringT instance.
  StringP StringT::current;    // Current StringT instance.

//--------------------//
void StringT::Initiation()     // Initiate the string block list.
{
  first    = this;
  current  = first;
  str_null = Append(""); 
} //---- Initiation end ----//

//--------------------//
String StringT:: Append(String pStr) // Append pStr to the current block.
{                                    // If no room, allocate the next block.
  StringP lNext;
  String  lStr;
  int     lLeng;
  
  lLeng = strlen(pStr);
  if ((current->top+lLeng) >= current->tail) { // There is no room.
    lNext = new StringT;                       // Allocate the next block
    current->nextBlk = lNext;
    current = lNext;
    if ((current->top+lLeng) >= current->tail) // pStr is too long.
      return (NULL);             // Error
  }
  strcpy(current->top, pStr);    // Append the string to the tail
  lStr = current->top;           //   of the current block.
  current->top += (lLeng+1);     // Add length + 1(for '\0')
  return lStr;
} //---- Append end ====//

//========== Token Class Implementation ==========//

//---- Global variables ----//
Tkn_kindT tkn_chKindT[384];  // Token kind guessed by 1st char.
Token     tkn_curr;          // Current token
String    tkn_kindName[] =
    {"blank  ","spec   ","compoun","eol    ","eof    ",
     "intC   ","key    ","id     ","var    ","param  ",
     "func   ","type   ","label  ","oper   ","reg    ","mrg    " };

//--------------------//
void Token::Initiation()     // Initiate the token class.
{
  int i;
  
  tknC   = TKN_eol;  
  leng   = 0;
  value  = 0;
  for (i = 0; i < TKN_lim; i++) 
    name[i] = ' ';
  for (i = 0; i < 384; i++)           // Initiate by special char kind.
    tkn_chKindT[i] = tknSpec;         // (tkn_chKindT[0..126] is unused)
  for (i = '0'; i <= '9'; i++)        // Numeric constant header.
    tkn_chKindT[i+128] = tknIntC;     // char is -128 ~ 127
  for (i = 'A'; i <= 'Z'; i++)        // Identifier initial.
    tkn_chKindT[i+128] = tknId; 
  for (i = 'a'; i <= 'z'; i++)        // Identifier initial.
    tkn_chKindT[i+128] = tknId; 
  tkn_chKindT[' '+128] = tknBlank;    // Blank
  tkn_chKindT['='+128] = tknCompound; // Compound operator initial.
  tkn_chKindT['>'+128] = tknCompound;
  tkn_chKindT['<'+128] = tknCompound;
  tkn_chKindT['!'+128] = tknCompound;
  tkn_chKindT[TKN_eol+128] = tknEol;  // End of line.
  tkn_chKindT[TKN_eof+128] = tknEof;  // End of file (EOF == -1).
} //---- Initiation end ----//

//--------------------//
void Token::Get()             // Get the next token from inFile.
{
  while ((tknC != TKN_eof)&&
         ((tknC == ' ')||(tknC == TKN_tab)||(tknC == TKN_eol)))
    tknC = NextC();              // Skip blanks, tabs, and EOL.
  leng = 0;
  kind = tkn_chKindT[tknC+128];  // Guess the token kind by 1st char.
  if (kind == tknId) {           // Identifier.
    do {
      if (leng < TKN_lim)        // Cut out the identifier.
        name[leng] = tknC;
      leng++;
      tknC = NextC();
    } while ((tkn_chKindT[tknC+128] == tknId)||
             (tkn_chKindT[tknC+128] == tknIntC));
    if (leng > TKN_lim)          // Truncate if too long.
      leng = TKN_lim;
  }else if (kind == tknIntC) {   // Numeric constant.
    value = 0;
    do {                         // Cut out the integer constant.
      value = value*10 + ((int)tknC - (int)'0');
      if (leng < TKN_lim)
        name[leng] = tknC;
      leng++;
      tknC = NextC();
    } while (tkn_chKindT[tknC+128] == tknIntC);
    if (leng > TKN_lim)          // Truncate if too long.
      leng = TKN_lim;
  }else {            // Special character or compound operator.
    name[0] = tknC;
    leng = 1;
    tknC = NextC();
    if (kind == tknCompound) {  // May be compound operator.
      if (((name[0] == '=')&&((tknC == '=')||(tknC == '>'))) ||
          ((name[0] == '!')&&(tknC == '=')) ||
          ((name[0] == '<')&&(tknC == '=')) ||
          ((name[0] == '>')&&(tknC == '=')) ||
          ((name[0] == '-')&&(tknC == '>')) ) { // Compound operator.
        name[1] = tknC;
        leng = 2;
        tknC = NextC();
      }
    }else if ((name[0] == '/')&&(tknC == '*')) {  // Comment start
      tknC = NextC();
      while ((tknC != TKN_eof)&&
             ((tknC != '*')||((tknC = NextC()) != '/'))) 
        tknC = NextC();
      tknC = NextC();
      Get();
    }
  }
  name[leng] = '\0';            // Attach end_of_string indication.
} //---- Get end ----//

//-------- Static Variables --------//
char Token::tknC;           // Current token character.

//========== SymbolTable Class Implementation ==========//

//-------- Global Variables --------//
  StringT sym_string;       // String table containing symbol names.
  SymbolP                   // Predefined symbols.
    sym_eof,    sym_lpar,   sym_rpar,  sym_lbracket,sym_rbracket,
    sym_lbrace, sym_rbrace, sym_comma, sym_scolon,  sym_period,
    sym_arrow,  sym_int,    sym_ptr,   sym_void,    sym_extern,  
    sym_if ,    sym_else,   sym_while, sym_return,  sym_add ,
    sym_sub,    sym_mult,   sym_div,   sym_assign,  sym_equal,
    sym_ne,     sym_gt,     sym_ge,    sym_lt,      sym_le,
    sym_main;
  SymbolT sym_tbl;      // Pre-declared symbol table.
  SymbolP sym_curr;     // Current symbol.
  int     sym_tempVar;  // Generated temporal index.

//--------------------//
void SymbolT::Initiation()    
     // Initiate symbol table and string table.
     // (initiate hash table, set predefined symbols, and so on.) 
{
  char lEof[2];
  
  for (int i = 0; i < SYM_hashLim; i++)
    hash[i] = NULL;                     // Nullify the hash table.
  sym_string.Initiation();              // Initiate the string table.
  head         = this;                  // Set pointers.
  tail         = head;
  name         = str_null;
  lEof[0]      = TKN_eof;               // End file code
  lEof[1]      = '\0';                  // followed by end of string.
  sym_eof      = Set(lEof,  tknEof);    // Set availabel special characters
  sym_lpar     = Set("(",   tknSpec);   // in the symbol table.
  sym_rpar     = Set(")",   tknSpec);
  sym_lbracket = Set("[",   tknSpec);
  sym_rbracket = Set("]",   tknSpec);
  sym_lbrace   = Set("{",   tknSpec);
  sym_rbrace   = Set("}",   tknSpec);
  sym_comma    = Set(",",   tknSpec);
  sym_scolon   = Set(";",   tknSpec);
  sym_period   = Set(".",   tknSpec);
  sym_int      = Set("int",    tknType);  // Set key words in the symbol table.
  sym_void     = Set("void",   tknType);
  sym_extern   = Set("extern", tknType);
  sym_if       = Set("if",     tknKey);
  sym_else     = Set("else",   tknKey);
  sym_while    = Set("while",  tknKey);
  sym_return   = Set("return", tknKey);
  sym_add      = SetOp("+" ,tknOper, opAdd ); // Set operators in the 
  sym_sub      = SetOp("-" ,tknOper, opSub ); //   symbol table.
  sym_mult     = SetOp("*" ,tknOper, opMult);
  sym_div      = SetOp("/" ,tknOper, opDiv );
  sym_assign   = SetOp("=" ,tknOper, opStore);
  sym_equal    = SetOp("==",tknOper, opBrne); 
  sym_ne       = SetOp("!=",tknOper, opBreq);
  sym_gt       = SetOp(">" ,tknOper, opBrle);
  sym_ge       = SetOp(">=",tknOper, opBrlt);
  sym_lt       = SetOp("<" ,tknOper, opBrge);
  sym_le       = SetOp("<=",tknOper, opBrgt);
  sym_main     = Set("main",tknFunc);
  sym_int->SizeSet(4);
} //---- Initiation end ----//

//--------------------//
SymbolP SymbolT::SearchOrAdd( // Search given symbol in the symbol table
                          // and add it if not found.
  String    pString,      //   Name string of the symbol to be searched.
  int       pLeng,        //   Character length of pString.
  Tkn_kindT pKind)        //   Token kind of the symbol. For identifier,
                          //   give tknId if it is not yet declared.
                          // Return symbol table ptr of the symbol.
{
  int     lHash;
  SymbolP lSymp, lPrev, lNew;
  String  lName;
  
  lHash = HashKey(pString, pLeng);
  lSymp = hash[lHash];
  lPrev = NULL;
  while ((lSymp != NULL)&&strcmp(pString, lSymp->name)) { 
    lPrev = lSymp;                      // Not equal.
    lSymp = lSymp->hashChain;           // Advance to the next symbol.
  }
  if (lSymp == NULL) {                  // New symbol.
    lName = sym_string.Append(pString); // Record the name. 
    lNew = new SymbolT;                 // Get new instance.
    lNew->name = lName;
    lNew->kind = pKind;
    tail->next = lNew;
    tail = lNew;
    lNew->hashChain = hash[lHash];      // Insert this at top of chain.
    hash[lHash] = lNew;
    lSymp = lNew;
  }
  return lSymp;
} //---- SearchOrAdd end ----//

//--------------------//
SymbolP SymbolT::Get( // Read a token and return its symbol table ptr.
  char   pExpected)   //   Token kind expected (a: Id, 0: integer,
                      //   ?: any symbol, other special char: that char.)
{
  Tkn_kindT lTknKind;
  SymbolP   lSymp;
  Boolean   lError;
  
  tkn_curr.Get();
  if (tkn_curr.Kind() != tknEof) {
    lSymp = SearchOrAdd(tkn_curr.Name(), tkn_curr.Leng(),
                         tkn_curr.Kind());
    lError = false;
    if (tkn_curr.Kind() == tknId) {
      if ((pExpected != 'a')&&(pExpected != '?'))
        lError = true;
    }else if (tkn_curr.Kind() == tknIntC) {
      lSymp->IValueSet(tkn_curr.Value());
      if ((pExpected != '0')&&(pExpected != '?'))
        lError = true;
    }else if ((*(lSymp->name) != pExpected)&&(pExpected != '?'))
      lError = true;
    if (lError)
      lSymp->Error(10, StrAppend(" should be class ", pExpected));
    sym_curr = lSymp;
    return lSymp;
  }else {
    sym_curr = sym_eof;
    return sym_eof;
  }
} //---- Get end ----//

//--------------------//
SymbolP SymbolT::GeneratedVar() // Generate a temporal variable.
{                               // NOT USED IN THE CURRENT VERSION.
  char lGeneratedName[16];
  SymbolP lName;
  VarInfP lVarInf;
  
  sym_tempVar++;
  strcat(strcpy(lGeneratedName, "_temp"), Itoaf(sym_tempVar));
  lName = SearchOrAdd(lGeneratedName, strlen(lGeneratedName), tknVar);
  lVarInf = new VarInf;
  lName->VarPSet(lVarInf);
  lVarInf->ElemCountSet(1);
  lVarInf->IndexSet(sym_tempVar);
  return lName;
} //---- GeneratedName end ----//

//--------------------//
void SymbolT::Print()   // Print symbol informations for debug.
{
  SymbolP lSym;

    if (this == sym_eof)
      fprintf(prtFile, "\n ");
    else 
      fprintf(prtFile, "\n %s ", Name()); 
    fprintf(prtFile, "%s sz %d h %s ",  
      KindName(),  Size(), hashChain->NameStr());
    switch (Kind()) {
    case tknIntC:
      fprintf(prtFile, "%d ", IValue());
      break;
    case tknOper:
      fprintf(prtFile, "%s ", sym_opCodeName[OpCode()]);
      break;
    case tknVar:
    case tknParam:
      fprintf(prtFile, "vt %s ec %d ix %d pn %s ", VTypeName(), 
        VarP()->ElemCount(), VarP()->Index(), VarP()->ParamChain()->NameStr());
      break;
    case tknFunc:
      fprintf(prtFile, "pn %s pc %d ", FuncP()->ParamChain()->NameStr(), 
        FuncP()->ParamCount());
      break;
    case tknReg:
      RegP()->Print();
      break;
    case tknMrg:
      MrgP()->Print();
      break;
    default:
      break;
    }
} //---- Print end ----//

//--------------------//
void SymbolT::PrintAll() // Print all symbols in the symbol table.
{
  SymbolP lSymp;
  
  for (lSymp = head; lSymp != NULL; lSymp = lSymp->next)
    lSymp->Print();
  fprintf(prtFile, "\n");
} //---- PrintAll end ----//

//--------------------//
void SymbolT::Error(    // Print error message with symbol name.
  int    pErrorCode,    //   Error code.
  String pMessage)      //   Error message.
{
  fprintf(prtFile, "\n ** ERROR %d %s %s \n",    //## 010828
          pErrorCode, name, pMessage);
// Error( 10, StrAppend(" should be class ", pExpected)) in symTbl.Get
// Error( 20, "Parameter name is expected \n") in FuncDecl
// Error( 90, "Unexpected symbol") in Factor      //## 010828
// Error(100, "Id expected \n"); in VarDecl
// Error(110, " encountered. ") in Expect
// Error(130, "without preceeding if.") in Block
} //---- Error end ----//

//--------------------//
SymbolP SymbolT::Set(   // Set to the symbol table as pKind symbol.
  String    pString,    //   Name string of the symbol.
  Tkn_kindT pKind)      //   Token kind to be set for the symbol.
{
  SymbolP lSymp;
  
  lSymp = SearchOrAdd(pString, strlen(pString), pKind);
  lSymp->KindSet(pKind);
  return lSymp;
} //---- Set end ----//

//--------------------//
SymbolP SymbolT::SetOp( // Set operator to the symbol table.
  String      pString,  //   Name string of the operator.
  Tkn_kindT   pKind,    //   Token kind.
  Sym_opCodeT pCode)    //   Abstract operation code of the operator.
{
  SymbolP lSymp;
  
  lSymp = SearchOrAdd(pString, strlen(pString), pKind);
  lSymp->KindSet(pKind);
  lSymp->OpCodeSet(pCode);
  return lSymp;
} //---- SetOp end ----//

//-------- Static Variables --------//
  SymbolP SymbolT::hash[SYM_hashLim]; // Hash table. 
  SymbolP SymbolT::head;              // Head of symbol list.
  SymbolP SymbolT::tail;              // Tail of symbol list.

//------ Global Variable ------//
  String sym_opCodeName[] = {
    "prog   ","pend   ","subp   ","end    ",
    "export ","import ","entry  ","label  ",
    "add    ","adda   ","sub    ","mult   ","div    ",
    "jump   ","call   ","return ","load   ","loada  ","store  ",
    "stack  ",                                           //## 010901
    "comp   ","breq   ","brne   ","brgt   ","brge   ","brle   ","brlt   ",  
    "prolog ","epilog ","sect   ","dword  ","dconst ","dparam "
  };

//========== Utility Implementation ==========//

//--------------------//
char *StrSyn(char *pStr1, char *pStr2)  // Synthesize pStr1 and pStr2
{                                       //   in a work area.
  static char lStr[256];  // Work area holding the result.
  
  strcpy(lStr, pStr1);
  strcat(lStr, pStr2);
  return lStr;
}  //---- StrSyn end ----//
//--------------------//
char *StrAppend(char *pStr, char pChar) // Append pChar to pStr
{                                       //   in a work area.
  static char lStr[256];  // Work area holding the result.
  int li;
  
  strcpy(lStr, pStr);
  li = strlen(lStr);
  lStr[li] = pChar;
  lStr[li+1] = '\0';
  return lStr;
}  // StrAppend end ----//
//--------------------//
void StrReverse(char *pStr)    // Reverse given character string pStr.
{
  int  li, lj;
  char lc;
  
  for (li = 0, lj = strlen(pStr)-1; li < lj; li++, lj--) {
    lc = pStr[li];
    pStr[li] = pStr[lj];
    pStr[lj] = lc;
  }
}  //---- StrReverse end ----//
//--------------------//
char *Itoaf(int pi) /* Change binary integer pi to character string */
{
  static char lStr[16];
  int    li, lVal, lSign;
  
  lVal  = pi;
  lSign = pi;
  if (lVal < 0) 
    lVal = -lVal;
  li = 0;
  do {
    lStr[li++] = lVal%10 + '0';
  } while ((lVal /= 10) > 0);
  if (lSign < 0)
    lStr[li++] = '-';
  lStr[li] = '\0';
  StrReverse(lStr);
  return lStr;
}  //---- Itoaf ----//

