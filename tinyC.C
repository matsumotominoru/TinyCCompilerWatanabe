//=========================================//
//  Tiny C Implementation   "tinyC.C"      //
//=========================================//
//                          Tan Watanabe, UEC (tan@cs.uec.ac.jp)
#include "sub.h"
#include "arm.h"
#include "mips.h"

//-- Global variables --//
  FILE    *inFile, *prtFile, *objFile;
  SymbolP subCurrent;      // Current subprogram name
  int     returnStmt;      // 1 if return stmt, 0 if other executable stmt.
//-- Forward declarations --//
  void     DeclStatement(SymbolP pTypeName);
  void     VarDecl      (SymbolP pTypeName, SymbolP pSymName, Boolean pLocal); //##0206
  void     FuncDecl     (SymbolP pTypeName, SymbolP lSymName); 
  int      ExecStatement(SymbolP pSymp);
  void     Block        ();
  Reg_symP Variable     (SymbolP pSymp, Reg_symP pRegCand, Ac_lrT pType);
  Reg_symP Factor       (Reg_symP pRegCand); 
  Reg_symP Term         (Reg_symP pRegCand);
  Reg_symP Expression   (Reg_symP pRegCand);
  void     Condition    (SymbolP pFalseLab);
  int      IfStatement  ();
  Reg_symP SubCall      (SymbolP pSymp);
  void     WhileStatement();
  void     Expect       (SymbolP pSymp);

//-------- Main program of the Tiny C compiler --------//
int main( int  pCount,    // Command parameter count.
          char *pArg[])   // Command parameter (source program file name).
{
  SymbolP lSymbolP, lSymName,  lTypeName;
  char    lObjFile[TKN_lim];        // Object file name.
  int     i;
  if (pCount > 1) {                 // Make object file name by replacing the
    strcpy(&lObjFile[0], pArg[1]);  // extension name (.c) of source to ".s"
    for (i = 0; (lObjFile[i] != '\0')&&(lObjFile[i] != '.'); i++);
    lObjFile[i+1] = 's';
    lObjFile[i+2] = '\0';
    printf("Compile %s (object file %s)\n", pArg[1], lObjFile);
  }
  inFile  = fopen(pArg[1],   "r"); 
  if (inFile == NULL) {
    printf("Source file %s can not be opened.\n", pArg[1]);
    return 1;
  }
  prtFile = stdout;
  objFile = fopen(lObjFile,   "w");
  sym_string.Initiation(); // String table class             StringT.
  tkn_curr.Initiation();   // Token class                    Token.
  sym_tbl.Initiation();    // Symbol table class             SymbolT.
  reg_tbl.Initiation();    // Register information class     RegInf.
  ac_head.Initiation();    // Abstract register machine code Rcode.
  mrg_reg.Initiation();    // Machine register class         MachineReg.
  sym_curr = NULL;
  sym_tempVar = 0;   // Reset the number of generated temporal variables.
  GenCode(acS, opProg, NULL, NULL, NULL, NULL, 0);          // Heading part.
  GenCode(acC, opSect, NULL, NULL, NULL, NULL, acSectData);
  while (sym_curr != sym_eof) {         // Loop until EOF.
    sym_curr = sym_tbl.Get('?');        // Get the next symbol.
    if ((sym_curr->Kind() == tknType))  // Should begin with type key word.
      DeclStatement(sym_curr);          // Process the declaration.
  }
  GenCode(acN, opPend, NULL, NULL, NULL, NULL, 0);   // Trailing part.
  sym_tbl.PrintAll();                   // Print the symbol table.
  fclose(inFile);
  fclose(objFile);
  return 0;
} //---- main end ----//

//--------------------//
void DeclStatement(     // Process a declaration statement.
  SymbolP pTypeName)    //   Type name.
          // At entry, type name or type key word or Id is read.
          // At exit, ';' or '{' is read.
{
  SymbolP lSymName, lTypeName, lTypeKey;
  int     lSymNameProcessed;          // true: name part is already processed.

  sym_curr = sym_tbl.Get('?');        // Get the next symbol.
  if (sym_curr != sym_scolon) {
    lSymName = sym_curr;              // Variable or function name.
    sym_curr = sym_tbl.Get('?');      // Get the next symbol.
  }
  if (sym_curr == sym_lpar) {         // '('. Function decl. 
    FuncDecl(pTypeName, lSymName);
  }else if ((sym_curr == sym_comma)||(sym_curr == sym_lbracket)||
            (sym_curr == sym_scolon)) {  
    lSymNameProcessed = false;
    if (sym_curr != sym_scolon)
      do {         // VariableDecl::= TypeName VarSpec {"," VarSpec} ...";"
        VarDecl(pTypeName, lSymName, false); // Variable declaration. //##0206
        lSymNameProcessed = true;
        if (sym_curr == sym_comma) {
          lSymName = sym_tbl.Get('a');
          sym_curr = sym_tbl.Get('?');
          lSymNameProcessed = false;
        }
      } while ((sym_curr == sym_comma)||(sym_curr == sym_lbracket));
    if (sym_curr == sym_scolon) {
      if ((lSymNameProcessed == false)&&(lSymName != NULL)&&
          (lSymName->Kind() == tknId)) 
        VarDecl(pTypeName, lSymName, false); //##0206
    }
    Expect(sym_scolon);            // Semicolon is expected.
  }     // else-end (variable decl)
} //---- DeclStatement end ----//

//--------------------//  // Process a variable declaration.
void VarDecl(         // VarSpec::= VarName {"[" ArraySize "]"}
  SymbolP pTypeName,  //   Type name symbol pointer.
  SymbolP pSymName,   //   Variable name.
  Boolean pLocal)     // true if local decl, false otherwise. //##0206
          // At entry, '[' or ',' or ';' next to variable name is read.
          // At exit , ',' or ';' is read.
{
  SymbolP lVarName, lArraySizep;
  VarInfP lVar;
  
  lVarName = pSymName;
  if (lVarName->IsIdentifier()) {  // Process Variable name part.
    lVarName->KindSet(tknVar);
    lVarName->VTypeSet(pTypeName);
    lVarName->SizeSet(pTypeName->Size());
    lVar = new VarInf;
    lVarName->VarPSet(lVar);
    lVar->ElemCountSet(1);
  }  // end-if (tknId)
  else
    lVarName->Error(100, "Id expected \n");
  if (sym_curr == sym_lbracket) {  // Array variable with '['.
    lArraySizep = sym_tbl.Get('0');
    lVar->ElemCountSet(lArraySizep->IValue());
    lVarName->SizeSet((lVarName->Size())*(lVar->ElemCount()));
    sym_curr = sym_tbl.Get(']');
    sym_curr = sym_tbl.Get('?');    // Read comma or semicolon.
  }  // end-if (array var)
  if (! pLocal) { //##0206
    GenCode(acS, opLabel, NULL, NULL, NULL, lVarName, 0);
    GenCode(acC, opDword, NULL, NULL, NULL, NULL, lVarName->Size());
  } //##0206
} //---- VarDecl end ----//

//--------------------//
void FuncDecl(           // Process a function declaration.
  SymbolP pTypeName,     //   Type name pointer.
  SymbolP pFuncName)     //   Function name pointer.
          // Return the parent node under which opSubp is attached.
          // At entry, '(' is read. At exit, '}' or ';' is read.
          // FunctionDef::= TypeName FuncName "(" { TypeName ParamSpec //## 010830
          //                {{"," TypeName ParamSpec} ... } } ")"  [Block|";"]
          // ParamSpec  ::= ParamName {"[" ArraySize "]"}
{
  SymbolP  lParamList, lParamType, lParamName, lDim;
  SymbolP  lTypeName, lSymName;  //##0206
  FuncInfP lFunc;
  VarInfP  lParamInf;
  int      lParamC;
  Boolean  lSymNameProcessed;
  
  lFunc = new FuncInf;
  subCurrent = pFuncName;
  subCurrent->KindSet(tknFunc);
  subCurrent->VTypeSet(pTypeName);
  subCurrent->FuncPSet(lFunc);
  GenCode(acS, opSubp, NULL, NULL, NULL, subCurrent, 0);
  lParamC = 0;
  lParamList = subCurrent;
  sym_curr = sym_tbl.Get('?');           // Read parameter type or ')'.
  while (sym_curr->Kind() == tknType) {  // If parameter type.
    lParamType = sym_curr;               // Get paramter name.
    sym_curr->Get('a');
    if (sym_curr->Kind() == tknId) {     // sym_curr is a parameter name.
      lParamC++;
      lParamName = sym_curr;
      if (lParamC == 1)
           lParamList->FuncP()->ParamChainSet(lParamName);
      else lParamList->VarP()->ParamChainSet(lParamName);
      lParamName->KindSet(tknParam);
      lParamName->VTypeSet(lParamType);
      lParamName->SizeSet(lParamType->Size());
      lParamInf = new VarInf;
      lParamName->VarPSet(lParamInf);
      lParamInf->IndexSet(lParamC);
      lParamInf->ElemCountSet(1);        // Set default.
      sym_curr = sym_tbl.Get('?');       // Read the next.
      if (sym_curr == sym_lbracket) {    // Array parameter.
        lDim = sym_tbl.Get('0');         // Array size.
        lParamInf->ElemCountSet(lDim->IValue());
        lParamName->SizeSet((lParamName->Size())*(lDim->IValue()));
        sym_curr = sym_tbl.Get(']');
        sym_curr = sym_tbl.Get('?');     // Read comma or semicolon.
      }  //---- end-if (array var)
      GenCode(acS, opLabel, NULL, NULL, NULL, lParamName, 0);
      GenCode(acC, opDparam, NULL, NULL, NULL, NULL, lParamName->Size());
      lParamList = lParamName;
    } //---- Param name end
    if (sym_curr == sym_comma)
      sym_curr = sym_tbl.Get('a');     // Read the next.
  };       
  subCurrent->FuncP()->ParamCountSet(lParamC);
  Expect(sym_rpar);                    // ')' is expected.
  sym_curr = sym_tbl.Get('?');         // Read '{'.
  if (sym_curr == sym_lbrace) {        // Function body is given.
    reg_tbl.RegFreeAll();
    GenCode(acS, opExport, NULL, NULL, NULL, subCurrent, 0);

    //##0206 BEGIN  Add local variable declaration
    sym_curr = sym_tbl.Get('?');         // Read the next token after '{'.
    while (sym_curr->Kind() == tknType) { // Local variable decl follows.
      lTypeName = sym_curr;
      sym_curr = sym_tbl.Get('?');     // Read variable name 
      lSymName = sym_curr;
      lSymNameProcessed = false;
      sym_curr = sym_tbl.Get('?');     // Read next sym after var name 
      if (sym_curr != sym_scolon)
        do {         // VariableDecl::= TypeName VarSpec {"," VarSpec} ...";"
          VarDecl(lTypeName, lSymName, true); // Variable declaration.
          lSymName->VarP()->LocalFlagSet();
          lSymName->VarP()->DispSet(subCurrent->FuncP()->LocalDisp());
          subCurrent->FuncP()->LocalDispAdd(lSymName->Size());
          lSymNameProcessed = true;
          if (sym_curr == sym_comma) {
            lSymName = sym_tbl.Get('a');
            sym_curr = sym_tbl.Get('?');
            lSymNameProcessed = false;
          }
        } while ((sym_curr == sym_comma)||(sym_curr == sym_lbracket));
      if (sym_curr == sym_scolon) {
        if ((lSymNameProcessed == false)&&(lSymName != NULL)&&
            (lSymName->Kind() == tknId)) 
          VarDecl(lTypeName, lSymName, true);
          lSymName->VarP()->LocalFlagSet();
          lSymName->VarP()->DispSet(subCurrent->FuncP()->LocalDisp());
          subCurrent->FuncP()->LocalDispAdd(lSymName->Size());
      }
      Expect(sym_scolon);            // Semicolon is expected.
      sym_curr = sym_tbl.Get('?');   // Read the next token
    }     // else-end (variable decl)
    //##0206 END

    GenCode(acC, opSect,   NULL, NULL, NULL, NULL, acSectText);
    GenCode(acS, opProlog, NULL, NULL, NULL, subCurrent, 0);
    returnStmt = 0;        // Reset return flag.
    Block();               // Process the function body.
    if (returnStmt == 0) { // If previous statement is not "return"
      GenCode(acS,  opEpilog, NULL, NULL, NULL, subCurrent, 0);
      GenCode(acRs, opReturn, NULL, NULL, NULL, subCurrent, 0);
    }
    GenCode(acS,  opEnd,    NULL, NULL, NULL, subCurrent, 0);
    Expect(sym_rbrace);    // '}' is expected.
  }
} //---- FuncDecl end ----//

//--------------------//
void Expect(            // Expect pSym as current or next input token.
  SymbolP pSym)         //   Symbol pointer of the expected symbol.
          // If unexpected symbol appears, skip symbols until pSym appears.
{
  if (sym_curr == pSym)
    return;
  sym_curr = sym_tbl.Get('?');
  if (sym_curr != pSym) {
    sym_curr->Error(110, " encountered. ");
    fprintf(prtFile, " %s expected.\n", pSym->NameStr());
    while ((sym_curr != pSym)&&(sym_curr != sym_eof))
      sym_curr = sym_tbl.Get('?');
  }
} //---- Expect end ----//

//--------------------//  // Process a block.
void Block()              // Block::= "{" Statement ... "}"
      // At entry, '{' is read or the token next to '{'is read.//##0206
      // At exit, '}' is read.
{
  Reg_symP lRegCond;
  int      lNextRead;
  
  if (sym_curr == sym_lbrace) //##0206
    sym_curr = sym_tbl.Get('?'); // First token of the first statement.
  while ((sym_curr->Kind() != tknEof)&&(sym_curr != sym_rbrace)) {
    lNextRead = ExecStatement(sym_curr);
    if (lNextRead == 0)
      sym_curr = sym_tbl.Get('?');
  }
  Expect(sym_rbrace);
} //---- Block end ----//

//--------------------//
int ExecStatement(    // Process an executable statement.
  SymbolP pSymp)      //   First symbol of the statement. 
      // At entry, 1st symbol is read. 
      // At exit, ';' or '}' is read if return value is 0.
      // At exit, a token next to ';' or '}' is read if return value is 1.
{
  Reg_symP lLeftPart, lExpReg;
  SymbolP  lSym;
  int      lNextRead = 0;              // 1: The next symbol is already read.
  
  returnStmt = 0;                      // Reset return statement flag.
  if ((pSymp->Kind() == tknVar)||
      (pSymp->Kind() == tknParam)) {   // Variable "=" Expression ";"
    lLeftPart = Variable(pSymp, reg_ti0, lValue);
    if (sym_curr == sym_assign) {      // It is "="
      sym_curr = sym_tbl.Get('?');     // Get 1st symbol of the expression,
      lExpReg = Expression(reg_ti0);   // evaluate it, (reg_ti0: default reg.)
      ac_curr->Store(lLeftPart, lExpReg);  // and store the result.
    }
  }else if (pSymp->Kind() == tknFunc) { // Function call.
    lExpReg = SubCall(pSymp);
  }else if (pSymp == sym_if) {         // "if"
    lNextRead = IfStatement();         // 0 if with else, 1 if without else.
  }else if (pSymp == sym_while)        // "while" 
    WhileStatement();
  else if (pSymp == sym_lbrace)        // "{"
    Block();
  else if (pSymp == sym_return) {      // Statement::= "return" ";"
    sym_curr = sym_tbl.Get('?');
    if (sym_curr != sym_scolon)        // Not ";", with return value.
      lExpReg = Expression(reg_ri);    // Use the return value register reg_ri.
    else
      lExpReg = NULL;
    GenCode(acRs, opEpilog, lExpReg, NULL, NULL, subCurrent, 0);
    GenCode(acS,  opReturn, NULL,    NULL, NULL, subCurrent, 0);
    returnStmt = 1;                   // Set return flag.
  }
  if ((sym_curr != sym_rbrace)&&(lNextRead == 0)) 
    Expect(sym_scolon);               // ";" is expected.
  return lNextRead;                   // 1 if the next symbol is read.
} //---- ExecStatement end ----//

//--------------------// // Process a variable.
Reg_symP Variable(       // Variable::= Name {"["Expression"]"}
  SymbolP  pSymp,        //   Header name of the variable.
  Reg_symP pRegCand,     //   Register candidate
  Ac_lrT   pType)        //   lValue/rValue.
           // Return resultant register 
           // (or variable if lValue and the variable is simple name).
           // At entry, variable name is read.
           // At exit, the next token following the variable is read.
{
  Reg_symP lRegIndex, lRegVar;
  Ac_lrT   lType;
  
  sym_curr = sym_tbl.Get('?');         // Get the next symbol.
  if (sym_curr == sym_lbracket) {      // With subscript exp.
    sym_curr = sym_tbl.Get('?');
    if (sym_curr->Kind() == tknIntC)   // Constant subscript.
       // && next token is sym_bracket !! REFINE  
      lRegVar = ac_curr->Load(pSymp, pRegCand, NULL, 
                              (sym_curr->IValue())*COM_sizeInt, pType);
    else {                             // Non-constant subscript.
      lRegIndex = Expression(reg_ti0); // Process the subscript.
      lRegVar = ac_curr->Load(pSymp, pRegCand, lRegIndex, 0, pType);
    }
    Expect(sym_rbracket);              // "]" is expected.
    sym_curr = sym_tbl.Get('?');
  }else {                  // Simple variable.
    if (pType == lValue)   //   If lValue, return the variable itself
      return pSymp;        //   for using it as an operand of store.
    lType = (pSymp->VarP()->ElemCount() > 1 ? lValue : pType);
    lRegVar = ac_curr->Load(pSymp, pRegCand, NULL, 0, lType);
  }
  return lRegVar;          // Return a register corresponding to the variable.
} //---- Variable end ----// 

//--------------------// // Process a factor.
Reg_symP Factor(         // Factor::= IntConst|Variable|"("Expression")"
  Reg_symP pRegCand)     //   Candidate register.
            // At entry, first token is read.
            // At exit, the next token following the factor is read.
{
  Reg_symP lReg;
  
  if ((sym_curr->Kind() == tknVar)||(sym_curr->Kind() == tknParam)) 
    lReg = Variable(sym_curr, pRegCand, rValue);
  // else if (sym_curr->Kind() == tknSubp) { } // Not Yet Implemented. 
  else {
    if (sym_curr->Kind() == tknIntC) {   // Integer constant.
      lReg = reg_tbl.RegAlloc(pRegCand, NULL, sym_curr->IValue(),
              regConstVal);
      if (lReg->RegP()->reusable == false) //##030309
        GenCode(acRc, opLoad, lReg, NULL, NULL, NULL, sym_curr->IValue());
    }else if (sym_curr == sym_lpar) {   // "(" Expression ")"
      sym_curr = sym_tbl.Get('?');      // 1st symbol of the expression.
      lReg = Expression(pRegCand);
      Expect(sym_rpar);                 // ")" is expected.
    }else {                                      //## 010828
      sym_curr->Error(90, "Unexpected symbol");  //## 010828
      lReg = reg_tbl.RegAlloc(reg_ti0, NULL, 1, regConstVal);  //## 010828
    }
    sym_curr = sym_tbl.Get('?');        // Get the next symbol.
  }
  return lReg;              // Return a register containing the factor value.
} //---- Factor end ----//

//--------------------// // Process a term.
Reg_symP Term(           // Term::= Factor {["*"|"/"] Factor} ...
  Reg_symP pRegCand)     //   Candidate register.
            // At entry, the first token is read.
            // At exit, a token next to the term is read.
{
  Reg_symP lReg, lReg2;
  Sym_opCodeT lOpCode;
  
  lReg = Factor(pRegCand);
  while ((sym_curr == sym_mult)||(sym_curr == sym_div)) {
    lOpCode  = sym_curr->OpCode();
    sym_curr = sym_tbl.Get('?');
    lReg2    = Factor(reg_ti0);   // Use the default register reg_ti0.
    GenCode(acRrr, lOpCode, lReg, lReg, lReg2, NULL, 0);
  }
  return lReg;    // Return a register containing the value of the term.
} //---- Term end ----//

//--------------------// // Process an expression.
Reg_symP Expression(     // Expression::= {"+"|"-"}Term {["+"|"-"] Term} ...
  Reg_symP pRegCand)     //   Candidate register.
            // At entry, first symbol of the expression is read.
            // At exit, the token next to the expression is read.
{
  Reg_symP    lReg, lReg1, lReg2;
  Sym_opCodeT lOpCode;
  
  lReg1 = NULL;
  if (sym_curr == sym_sub)   {    // Unary "-" 
    lReg1 = reg_tbl.RegAlloc(reg_ti0, NULL, -1, regConstVal); //##030309
    if (lReg1->RegP()->reusable == false) //##030309
      GenCode(acRc, opLoad, lReg1, NULL, NULL, NULL, 0);
    sym_curr = sym_tbl.Get('?');
  }else if (sym_curr == sym_add)  // Unary "+"
    sym_curr = sym_tbl.Get('?');
  lReg = Term(pRegCand); 
  if (lReg1 != NULL) 
    GenCode(acRrr, opSub, lReg, lReg1, lReg, NULL, 0);
  while ((sym_curr == sym_add)||(sym_curr == sym_sub)) {
    lOpCode = sym_curr->OpCode();
    sym_curr = sym_tbl.Get('?');
    lReg2 = Term(reg_ti0);       // Use the default register.
    GenCode(acRrr, lOpCode, lReg, lReg, lReg2, NULL, 0);
  }
  return lReg;   // Return a register containing the expression value.
} //---- Expression end ----//

//--------------------// // Process a conditional expression.
void Condition(          // ConditionExp::= Expression CompOper Expression
  SymbolP pFalseLab)     //   Branch label for false jump.
          // At entry, 1st symbol or '(' of conditional exp is read.
          // At exit, the token next to conditional exp is read.
{
  Reg_symP lReg, lReg2;
  Sym_opCodeT lOpCode;
  
  sym_curr = sym_tbl.Get('?');
  lReg = Expression(reg_ti0); 
  if (sym_curr->Kind() == tknOper) {
    lOpCode = sym_curr->OpCode();   // Get the operation code for the operator.
    sym_curr = sym_tbl.Get('?');
    lReg2 = Expression(reg_ti0);
    GenCode(acRrr, opComp, lReg, lReg, lReg2, NULL, 0);     // Compare.
    GenCode(acRs, lOpCode, lReg, NULL, NULL, pFalseLab, 0); // jump false.
  }
  if (sym_curr == sym_rpar)
    sym_curr = sym_tbl.Get('?');   // Read a symbol next to ")".
} // Condition end ----//

//--------------------// // "if" "(" ConditionExp ")" Statement
int IfStatement()        // { "else" Statement }
      // At entry, "if" is read. 
      // At exit, a token after ';' is read if without "else" (return 1).
      // At exit, ';' is read if without "else" (return 0).
{
  Reg_symP lRegCond;
  SymbolP  lFalseLab, lIfEndLab;
  int      lNextRead;
  
  lFalseLab = ac_curr->LabelGen(); // Get labels.
  lIfEndLab = ac_curr->LabelGen();
  sym_curr = sym_tbl.Get('(');     // Read '(' enclosing ConfitionExp.
  Condition(lFalseLab);            // Generate instructions for it.
  lNextRead = ExecStatement(sym_curr);  // "then" part. Token after ';' is read.
  GenCode(acS, opJump,  NULL, NULL, NULL, lIfEndLab, 0);
  GenCode(acS, opLabel, NULL, NULL, NULL, lFalseLab, 0);
  reg_tbl.RegFreeAll();
  if (lNextRead == 0) {            // Read the next token.
    sym_curr = sym_tbl.Get('?');   // Token after ';' or '}' is read.
    lNextRead = 1;                 // Indicate that the next token is read.
  }
  if (sym_curr == sym_else) {      // "else" Statement
    sym_curr = sym_tbl.Get('?');
    lNextRead = ExecStatement(sym_curr);   //  ';' or '}' is read.
  }
  GenCode(acS, opLabel, NULL, NULL, NULL, lIfEndLab, 0);
  reg_tbl.RegFreeAll();
  return lNextRead;
} //---- IfStamenent end ----//

//--------------------// // Process a subprogram call.
Reg_symP SubCall(        // FuncName "(" Expression {"," Expression} ...")"
  SymbolP pSym)          //   Symbol pointer of the subprogram.
          // At entry, '(' is read. At exit, ')' is read.
{
  SymbolP lParam, lParamType;  // Parameter name & type.
  Reg_symP  lParamReg;         // Parameter register.

  sym_curr = sym_tbl.Get('?');
  lParam   = pSym->FuncP()->ParamChain();  // The first formal parameter.
  // lParamType = lParam->VType();         // lParam may be NULL. //## 010830
  lParamReg  = reg_ai0;        // reg_ai0: first parameter register.
  while ((sym_curr != sym_rpar)&&(sym_curr != sym_scolon)&&
         (lParam != NULL)&&(lParamReg != NULL)) {          //## 010901
    if ((sym_curr == sym_comma)||(sym_curr == sym_lpar))
      sym_curr = sym_tbl.Get('?');  // Get the first token of the expression.
    //## if (lParamReg == NULL)  // Allocate a parameter register.   //## 010901
    //##   lParamReg = reg_tbl.RegAlloc(reg_ai0, NULL, 0, regEmpty); //## 010901
    lParamReg->RegP()->RegStatSet(regReserved); // Reserve the param reg.
    lParamReg = Expression(lParamReg);          // Evaluate the parameter.
    if (lParamReg->RegP()->RegNumb() >= REG_paramReg) {   //## 010901
      // Put the parameter in the stack.
      GenCode(acRc, opStack, lParamReg, NULL, NULL, NULL, (lParamReg->RegP()->RegNumb())*COM_sizeInt);  //##0206
      //##0206 GenCode(acRc, opStack, lParamReg, NULL, NULL, NULL, (lParamReg->RegP()->RegNumb()-REG_paramReg)*COM_sizeInt);  //## 010901
    }                                                    //## 010901
    lParam = lParam->VarP()->ParamChain();      // The next formal param.
    lParamReg = lParamReg->RegP()->RegChain();  // Get the next param reg.
  }  
  Expect(sym_rpar);                             // ")" is expected.
  GenCode(acS, opCall, NULL, NULL, NULL, pSym, 0);
  reg_tbl.RegFreeAll();
  return reg_ri;     // reg_ri: return value register.
} //---- SubCall end ----//

//--------------------//
void WhileStatement() // "while" "(" ConditionExp ")" Statement
                      // At entry, 'while' is read.
                      // At exit, ';' or '}' is read.
{
  SymbolP  lLoopStartLab, lLoopEndLab;
  Reg_symP lReg;

  lLoopStartLab = ac_curr->LabelGen();  // Generate loop-start label.
  lLoopEndLab   = ac_curr->LabelGen();  // Generate loop-end   label.
  GenCode(acS, opLabel, NULL, NULL, NULL, lLoopStartLab, 0);
  reg_tbl.RegFreeAll();                 // Free registers.
  sym_curr = sym_tbl.Get('(');
  Condition(lLoopEndLab);  // At exit, the next token is read.
  ExecStatement(sym_curr);              // Loop body.
  GenCode(acS, opJump,  NULL, NULL, NULL, lLoopStartLab, 0);
  GenCode(acS, opLabel, NULL, NULL, NULL, lLoopEndLab, 0);
  reg_tbl.RegFreeAll();
} //---- WhileStatement end ----//

