/* TinyC test - Register allocation */
/*    Nozaki mail 021216 */

int i, j;
int b0, b1;

void main()
{
  i = 0;
  j = 0;
  b0 = i + j;
  i = 0;
  j = 1;
  b1 = i + j;
  return 0;
}
