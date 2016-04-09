#include <stdio.h>

void main()
{
  int sum=0;
  float sum_1=0.0;
  int i;
  for(i=0;i<300;i++)
  {
	printf("%.1f\n",sum_1/60);
    sum_1=sum_1+12;
   }
}