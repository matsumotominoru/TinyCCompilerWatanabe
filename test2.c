/* test2: constant subscript */

int a[10];

int x;

void f(int p[10])
{
  x = p[2];
  p[1] = 1;
  p[0] = 0;
return;
}

void main()
{
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;
  x = a[1];
  x = a[2];
  x = a[x]+1;
  f(a);
  return;
}

