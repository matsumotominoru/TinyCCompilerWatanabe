//=============================================//
//   Mips Object Code Interface       "Mips.h" //
//=============================================//
//                              Tan Watanabe, UEC
#pragma SC once
#ifndef MIPS
#define MIPS
#include "sub.h"
#include "arm.h"

//========== MachineReg Class ==========//
                          //---- Mips machine register inf
enum Mrg_usage {
  mrgUse,            // Use the value of a register
  mrgSet             // Set a   value to a register
};
class MachineReg {
public:
  Reg_symP armReg;   // Bounded Rcode register (NULL if none)
  Reg_symP mrgChain; // Allocation chain
  int      mrgNumb;  // Register number
  //----- Member functions --------//
           MachineReg(void) { armReg = NULL; mrgChain = NULL; }
  Reg_symP ArmReg    (void) { return armReg; }
  int      MrgNumb   (void) { return mrgNumb; }
  void     ArmRegSet (Reg_symP p) { armReg = p; }  
  Reg_symP MrgInit (  // Initiate a machine register.
                    String pRegName,  // Register name 
                    int pRegNumb);    // Register number
  Reg_symP MrgSearch( // Search machine reg to be allocated.
                     Reg_symP pArmReg, // Rcode register
                     int      pLevel); // 1: Search unused reg, 
                                       // 2: search noHolg reg.
  void     Print     ();  // Print machine register information.
  void     Initiation();  // Initiate machine registers.
}; //---- MachineReg class end ----//

//-------- Global symbols -------//
typedef  MachineReg* MachineRegP;  
Reg_symP MipsReg( // Allocate Mips register to Rcode register.
                 Reg_symP  pArmReg, // Rcode reg for allocation.
                 Mrg_usage pUsage); // Use mode or set mode.
void MipsCode(   // Generate Mips code corresponding to Rcode.
              Ac_form pForm, Sym_opCodeT pOpCode, Reg_symP pReg1,
              Reg_symP pReg2, Reg_symP pReg3, SymbolP pSym,
              int pInt);
extern MachineReg mrg_reg;  // Machine register instance.
extern Reg_symP 
  mrg_mips[32],  // Mips machine register pointers.
  mrg_zero, mrg_rval, mrg_param, mrg_temp, mrg_saved, 
  mrg_stackp, mrg_raddr;
#define MRG_searchLim 10

//##0206 BEGIN
// Stack frame for each subprogram:
//   upper address             
//        | GFP 2n           | GFP: int/ptr formal param. (up to 10)
//        | GFP 2n-1         | 
//        |   ...            | 
//        | GFP 7            | Set only when actual param is given 
//        | GFP 6            | 
//        | GFP 5            | GFP5 <- sp+cgFrameSize +16 formal param 5
//        | $7               | $7   <- sp+cgFrameSize +12 formal param 4
//        | $6               | $6   <- sp+cgFrameSize +8  formal param 3
//        | $5               | $5   <- sp+cgFrameSize +4  formal param 2
// $fp -> | $4               | $4   <- sp+cgFrameSize     formal param 1
//        |                  | (virtual frame ptr $fp = old $sp)
//        |                  | ($fp = $sp+cgFrameSize)
//        | local var k      |
//        |   ...            |
//        | local var 2      |
//        | local var 1      | <- sp+cgLocalBase+disp_within_local_area
//                             <- $sp+cgLocalBase 
//                                (cgLocalBase=cgTempBase+TEMP_lim*4)
//        | Temp var __temp[]| <- $sp+cgTempBase (size is TEMP_lim*4)
//        |                  |      (cgTempBase=cgRegSaveBase+36)
//        | $31 (return addr)| <- $sp+cgRegSaveBase+32 ($fp-cgFrameOffset)
//        | Reg save area    | 
//        |   for $23        | <- $sp+cgRegSaveBase+28 ($fp-cgFrameOffset-4)
//        |       $22        | <- $sp+cgRegSaveBase+24 ($fp-cgFrameOffset-8)
//        |       ...        |                     ...
//        |       $17        | <- $sp+cgRegSaveBase+4
//        |       $16        | <- $sp+(param area size)
//        |                  |    = $sp + cgRegSaveBase 
//        | GAP 2n           | GAP: int/ptr actual param
//        | GAP 2n-1         | (actual param area is prepared for 10 params)
//        |   ...            |    ...
//        | GAP 6            | <- $sp+20  actual param 6
//        | GAP 5            | <- $sp+16  actual param 5
//        | GAP 4            | $7  1-4 general param on $4-$7.
//        | GAP 3            | $6  
//        | GAP 2            | $5  (param regs are saved here by callee)
// $sp -> | GAP 1            | $4  
//     lower address
//
// cgRegSaveBase = REG_param * 4 
// cgTempBase    = cgRegSaveBase + 36 
// cgLocalBase   = cgTempBase + TEMP_lim*4
// cgFrameSize   = cgLocalBase + (local area size = LocalDisp()) + GAP
// cgFrameOffset = Offset of return address save area from
//                 frame pointer (stack pointer of parent)
// cgFrameSize and cgRegSaveBase is set to be multiple of 8 
//   in order to use ld/sd. GAP is adjustment for it.
//##0206 END

#endif
  
