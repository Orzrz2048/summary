#include<stdio.h>
#include<stdlib.h>

int leap(int i)//判断是否是闰年
{
		if(!(i%4)&&i%100||!(i%400))
			  return 1;//闰年
		else
			  return 0;//不是闰年
}

int monthday(int i)//判断每月对应天数
{
		switch(i)
		{
				case 1:case 3:case 5:case 7:case 8:case 10:case 12:
						return 31;
						break;
				case 4:case 6:case 9:case 11:
					    return 30;
						break;
				case 2:
						return 28;
						break;
		}
}

int monthcal(int i)
{
		int m=0;
		switch(i)
		{
				case 1:
						m=0;
						break;
				case 2:
						m=31;
						break;
				case 3:
						m=59;
						break;
				case 4:
						m=31+28+31;
						break;
				case 5:
						m=31+28+31+30;
						break;
				case 6:
						m=31+28+31+30+31;
						break;
			    case 7:
		                m=31+28+31+30+31+30;
						break;
                case 8:
						m=31+28+31+30+31+30+31;
						break;
				case 9:
				        m=31+28+31+30+31+30+31+31;
						break;
				case 10:
				        m=31+28+31+30+31+30+31+31+30;
						break;
				case 11:
						m=31+28+31+30+31+30+31+31+30+31;
						break;
				case 12:
						m=31+28+31+30+31+30+31+31+30+31+30;
						break;
		}
		return m;
}


int main()
{
		int year=0,month=0,i=0,j=0,day=0,week=0,m=0,n=0,md=0,k=1;
		printf("Please input year and month:\n");
		scanf("%d%d",&year,&month);
		while(getchar()!='\n');//接收年月，并消除错误输入

		if(year<1900||month<1||month>12)
		{
				printf("wrong input!!!\n");
				exit(1);
		}

		if(leap(year))
			  if(month > 2)
					day=(year - 1900) * 365 + (year - 1900) / 4 + monthcal(month);
		      else
					day=(year - 1900) * 365 + (year - 1900 - 1) / 4 + monthcal(month);
		else
					day=(year - 1900) * 365 + (year - 1900) / 4 + monthcal(month);
		printf("%d\n",day);
		printf("%d\n",monthcal(month));
        week=day%7 + 1;
		if(week == 0)
			  week = 7;
		printf("%d\n",week);

		//输出至屏幕
		printf("       %d年%d月      \n",year,month);
		printf(" 一 二 三 四 五 六 日\n");
		//打印第一行，先打印空格
		for(m=0,n=0;n<week-1;n++)
			  printf("   ");
		for(n=week-1;n<7;n++,++k)
			  printf("%3d",k);
		printf("\n");
		//确定打印天数
		if(leap(year)>0)
		{
				if(2==month)
					  md=29;
				else
					  md=monthday(month);

		}
		else
			  md=monthday(month);
		//打印剩余天数
		for(m=1;m<6;m++)
		{
				for(n=0;n<7;n++,++k)
				{
						if(k<=md)
							  printf("%3d",k);
				}
				printf("\n");
		}//printf("%d",md);检查md的值

		return 0;
}
