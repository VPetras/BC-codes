#include<stdio.h>

int main()
{
	int a[4];
	int i;

	for ( i = 0; i < 4; i++ )
		a[i] = 0;

	for ( i = 0; i < 4; i++ )
		printf("a[%d] = %d\n", i , a[i]);

	return 0;
}
