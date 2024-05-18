
         .sdata
         .comm    x,4
         .globl   f
         .text
         .ent     f
f:
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
         lw       $8,96($sp)
         lw       $9,100($sp)
         add      $8,$8,$9
         lw       $10,104($sp)
         add      $8,$8,$10
         lw       $11,108($sp)
         add      $8,$8,$11
         lw       $12,112($sp)
         add      $8,$8,$12
         lw       $13,116($sp)
         add      $8,$8,$13
         sw       $8,x
         lw       $31,32+40($sp)
         ld       $22,24+40($sp)
         ld       $20,16+40($sp)
         ld       $18,8+40($sp)
         ld       $16,0+40($sp)
         addu     $sp,96
         j        $31
         .end     f
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
         li       $4,0
         li       $5,1
         li       $6,2
         li       $7,3
         li       $8,4
         sw       $8,16($sp)
         li       $9,5
         sw       $9,20($sp)
         jal      f
         lw       $31,32+40($sp)
         ld       $22,24+40($sp)
         ld       $20,16+40($sp)
         ld       $18,8+40($sp)
         ld       $16,0+40($sp)
         addu     $sp,96
         j        $31
         .end     main