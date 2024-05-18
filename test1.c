/* test1:  Large number of parameters */

int x;

void f(int p0, int p1, int p2, int p3, int p4, int p5)
{
  x = p0 + p1 + p2 + p3 + p4 + p5;
}

void main()
{
  f(0, 1, 2, 3, 4, 5); 
  return;
}

