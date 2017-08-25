#include<stdio.h>
int num[9][9],xy[9][9];
int check(int x,int y)
{
		int i,m,n;
		for(i = 0;i < 9; i++)
			  if ((xy[x][y] == xy[i][y]&&i != x)||(xy[x][y] == xy[x][i]&&i != y))
					return 0;//列或行有重复的返回0
		for(i = 0,m = x/3*3,n = y/3*3;i < 9; i++)
			  if (xy[x][y] == xy[m + i / 3][n + i % 3]&&m + i / 3 != x&&n + i % 3 != y)
					return 0;//同一宫内有重复的返回0
		return 1;
}
void search(int x,int y)
{
		if (x == 9)
			  for(x = 0; x < 9; x++) {
					for(y = 0; y < 9; y++)
						  printf("%d ",xy[x][y]);
					printf("\n");
			  }
		else if (num[x][y])
			  search(x + (y + 1) / 9, (y + 1) % 9);
		     else
				   for(xy[x][y] = 1; xy[x][y] <= 9; xy[x][y]++)
						 if (check(x,y))
							   search(x + (y + 1) / 9,(y + 1)%9);
}
int main()
{
		int i,j;
		for(i = 0; i < 9; i++)
			  for(j = 0; j < 9; j++)
			  {
					  scanf("%d",&num[i][j]);
					  xy[i][j] = num[i][j];
			  }
		search(0,0);
		return 0;
}
