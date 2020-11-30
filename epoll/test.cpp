#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<ctype.h>
#include<string.h>
#include<sys/time.h>
#define MAX_LEN  1024*1024
void printCostTime(struct timeval *start,struct timeval *end)
{
        if(NULL == start || NULL == end)
        {
                    return;
                        
        }
            long cost = (end->tv_sec - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec)/1000;
                printf("cost time: %ld ms\n",cost);

}

int main(void)
{
        srand(time(NULL));
            int min = 'a';
                int max = 'z';
                    char *str = (char *)malloc(MAX_LEN);
                     if(NULL == str)
                     {
                                 printf("failed\n");
                                         return -1;
                                             
                     }
                         unsigned int i = 0;
                             while(i < MAX_LEN)//生成随机数
                             {
                                         str[i] = ( rand() % ( max - min  )  ) + min;
                                                 i++;
                                                     
                             }
                             str[MAX_LEN - 1] = 0; 
                                 struct timeval start,end;
                                     gettimeofday(&start,NULL);
                                         for(i = 0;i < strlen(str) ;i++)
                                         {
                                                     str[i]  = toupper( str[i]  );
                                                         
                                         }
                                             gettimeofday(&end,NULL);
                                                 printCostTime(&start,&end);
                                                     free(str);
                                                         str = NULL;
                                                             return 0;

}
