
         .sdata
         .comm    i,4
         .comm    j,4
         .comm    b0,4
         .comm    b1,4
         .globl   main
         .text
         .ent     main
main:
         subu     $sp,96
         sw       $31,32+40($sp)
         sd       $22,24+40($sp)
         sd       $20,16+40($sp)
         sd       $18,8+40($sp)
         sd       $16,0+40($sp)
         sd       $6,8+96($sp)
         sd       $4,0+96($sp)
         .frame   $sp,96,$31
         .mask    0x80ff0000,-24
         li       $8,0
         sw       $8,i
         li       $9,0
         sw       $9,j
         add      $8,$8,$9
         sw       $8,b0
         li       $10,0
         sw       $10,i
         li       $11,1
         sw       $11,j
         add      $10,$10,$11
         sw       $10,b1
         li       $16,0
         addu     $2,$16,$0
         lw       $31,32+40($sp)
         ld       $22,24+40($sp)
         ld       $20,16+40($sp)
         ld       $18,8+40($sp)
         ld       $16,0+40($sp)
         addu     $sp,96
         j        $31
         .end     main