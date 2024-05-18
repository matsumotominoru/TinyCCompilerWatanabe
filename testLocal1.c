/* testLocal1: local variable */

int a[10];

int x;

void f(int p[10])
{
  int i, j, k;
  i = 1;
  j = p[0];
  k =  i + j;
  x = k;
return;
}

void main()
{
  int b[10], m, n;
  m = 2;
  b[0] = a[1];
  b[m] = a[m];
  n = b[0] + b[m];
  a[1] = n;
  f(a);
  return;
}

