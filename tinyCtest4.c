/* TinyCtest4.c - Register allocation */
/*    Satoh mail (refered in gian mail 030127) */
/*    Correction arm.C, tinyC.C 030309 */

int a, b, c, d, e;
int main(){
  a = 3;
  a = 5 +( 5 + a );
  b = 3;
  c = 3 + a;
  d = 3 + (a + 3);
  e = a * 3 + b + 3;
  return 0;
}
