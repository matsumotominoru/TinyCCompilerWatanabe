/* TinyC test - Large basic block */

int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12;
int j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12;
int k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12;
int l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12;

void main()
{
	i1 = j1 + k1 - l1; 
	i2 = j2 + k2 - l2; 
	i3 = j3 + k3 - l3; 
	i4 = j4 + k4 - l4; 
	i5 = j5 + k5 - l5; 
{
	i6 = j6 + k6 - l6; 
	i7 = j7 + k7 - l7; 
	i8 = j8 + k8 - l8; 
	i9 = j9 + k9 - l9; 
}
}
