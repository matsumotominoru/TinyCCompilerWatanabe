 

# DO NOT DELETE THIS LINE -- make depend depends on it.

tinyC: tinyC.o arm.o mips.o sub.o
	g++ -o tinyC tinyC.o arm.o mips.o sub.o
arm.o: sub.h 
arm.o: arm.h arm.C
	g++ -c -g arm.C
mips.o: sub.h 
mips.o: arm.h mips.h mips.C
	g++ -c -g mips.C
sub.o: sub.h 
sub.o: arm.h mips.h sub.C
	g++ -c -g sub.C
tinyC.o: sub.h 
tinyC.o: arm.h mips.h tinyC.C
	g++ -c -g tinyC.C
