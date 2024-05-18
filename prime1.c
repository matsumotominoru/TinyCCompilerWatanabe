/* Get prime numbers (slimed main) */
 int candidat, quotient, remaindr, index, nth, primenum, loopend ;
 int primeNumbers[100] ;
 int print(int pvar);
 
void getPrime(int primevec[100], int count)
{
 primevec[0] = 2 ;
 nth = 1 ;
 candidat = 3 ;
 while (nth<count) {
   remaindr = 1 ;
   index = 0 ;
   loopend = 0 ;
   while(loopend==0) {
     primenum = primevec[index] ;
     quotient = candidat / primenum ;
     remaindr = candidat - quotient*primenum ;
     if (remaindr==0)
       loopend = 1 ;
     if (quotient*quotient<candidat)
       loopend = 1 ;
     index = index + 1 ;
     if (index>=nth) 
       loopend = 1 ;
   }
   if (remaindr != 0) {
     primevec[nth] = candidat ;
     nth = nth + 1 ; 
   }
   candidat = candidat + 2 ; 
 }
 nth   = 0 ;
 while (nth<count) {
   print(primevec[nth]) ;
   nth   = nth + 1 ;  
 }
 return;
}
 
int main()
{
 getPrime(primeNumbers, 100);
  return 0;
}
