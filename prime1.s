
         .sdata
         .comm    candidat,4
         .comm    quotient,4
         .comm    remaindr,4
         .comm    index,4
         .comm    nth,4
         .comm    primenum,4
         .comm    loopend,4
         .comm    primeNumbers,400
         .globl   getPrime
         .text
         .ent     getPrime
getPrime:
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
         li       $9,2
         sw       $9,0($8)
         li       $8,1
         sw       $8,nth
         li       $9,3
         sw       $9,candidat
$lab1:
         lw       $8,nth
         lw       $9,100($sp)
         bge      $8,$9,$lab2
         li       $10,1
         sw       $10,remaindr
         li       $11,0
         sw       $11,index
         li       $12,0
         sw       $12,loopend
$lab3:
         lw       $8,loopend
         li       $9,0
         bne      $8,$9,$lab4
         lw       $9,index
         lw       $16,96($sp)
         sll      $9,$9,2
         addu     $17,$16,$9
         lw       $10,0($17)
         sw       $10,primenum
         lw       $11,candidat
         div      $11,$11,$10
         sw       $11,quotient
         lw       $12,candidat
         mul      $11,$11,$10
         sub      $12,$12,$11
         sw       $12,remaindr
         li       $13,0
         bne      $12,$13,$lab5
         li       $13,1
         sw       $13,loopend
         j        $lab6
$lab5:
$lab6:
         lw       $8,quotient
         mul      $8,$8,$8
         lw       $9,candidat
         bge      $8,$9,$lab7
         li       $10,1
         sw       $10,loopend
         j        $lab8
$lab7:
$lab8:
         lw       $8,index
         li       $9,1
         add      $8,$8,$9
         sw       $8,index
         lw       $9,nth
         blt      $8,$9,$lab9
         li       $10,1
         sw       $10,loopend
         j        $lab10
$lab9:
$lab10:
         j        $lab3
$lab4:
         lw       $8,remaindr
         li       $9,0
         beq      $8,$9,$lab11
         lw       $9,nth
         lw       $16,96($sp)
         sll      $9,$9,2
         addu     $10,$16,$9
         lw       $11,candidat
         sw       $11,0($10)
         lw       $10,nth
         li       $12,1
         add      $10,$10,$12
         sw       $10,nth
         j        $lab12
$lab11:
$lab12:
         lw       $8,candidat
         li       $9,2
         add      $8,$8,$9
         sw       $8,candidat
         j        $lab1
$lab2:
         li       $8,0
         sw       $8,nth
$lab13:
         lw       $8,nth
         lw       $9,100($sp)
         bge      $8,$9,$lab14
         lw       $16,96($sp)
         sll      $8,$8,2
         addu     $17,$16,$8
         lw       $4,0($17)
         jal      print
         lw       $8,nth
         li       $9,1
         add      $8,$8,$9
         sw       $8,nth
         j        $lab13
$lab14:
         lw       $31,32+40($sp)
         ld       $22,24+40($sp)
         ld       $20,16+40($sp)
         ld       $18,8+40($sp)
         ld       $16,0+40($sp)
         addu     $sp,96
         j        $31
         .end     getPrime
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
         la       $4,primeNumbers
         li       $5,100
         jal      getPrime
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