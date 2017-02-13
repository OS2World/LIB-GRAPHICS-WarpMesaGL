/* a_debug.h */
/* функции для устраивания очных ставок */
#include <stdlib.h>
#include <stdio.h>

static char DebugFname[]="debug.log";
static int mode = 0; /* 0- запись, 1- проверка */
static FILE *fp = NULL;

void _Optlink DebugClose(void)
{  fclose(fp);
}

static int OpenDebug(void)
{  if(mode==0)
       fp = fopen(DebugFname,"wb");
   else
       fp = fopen(DebugFname,"rb");
   if(fp == NULL)
      perror("Could not open file");
   atexit(DebugClose);
   return 0;
}

int debug_write(int *buff, int n)
{  static int raz=0;
   if(fp == NULL) OpenDebug();
   if(mode == 0)
   {  fwrite(buff,sizeof(int),n,fp);
      fflush(fp);
   } else {
      int i, buff1[1024];
      fread(buff1,sizeof(int),n,fp);
      for(i=0;i<n;i++)
      {  if(buff[i] != buff1[i])
         {  printf("Diff!!!! at %i element of %i iteration %i != %i\n",i,raz,buff[i],buff1[i]);
            exit(1);
         }
      }
   }
   printf("%i\n",raz); fflush(stdout);
   raz++;
   return 0;
}
