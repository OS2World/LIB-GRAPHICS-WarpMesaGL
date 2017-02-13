#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Convert(char *filefrom, char *fileto);

int main(int n, char *par[])
{
   if(n < 3)
   {  printf("Convert typedef functions to VAC3 acceptable form\n");
      printf("Convert FileFrom FileTo\n");
   }
   Convert(par[1], par[2]);
}

int Convert(char *filefrom, char *fileto)
{  FILE *fp, *fpto;
   char str[1024], *pstr;
   char str1[1024], *pstr1, *pstr2, *pstrout;
   char str2[1024];

   int i,is,n,n1, dn;  
   fp = fopen(filefrom,"r");
   fpto = fopen(fileto,"w");
   n = n1=0;
   for(;;)
   {  pstr = fgets(str, 1023,fp);
      if(pstr == NULL)
              break;
      is = 0;
      if(strstr(str,"typedef") )
      {  pstr1 = strstr(str,"(APIENTRY * P");
         pstrout = &str1[0];
         if(pstr1)
         {  n1++;
            dn = (int) (pstr1 - &str[0]); 
            dn += 10; 
            strncpy(str1,str,dn);
            str1[dn] = 0;
            is = 1;
            strcat(str1, &str[dn+3]);  
            fputs(str1,fpto);
            pstr1 = &str[dn+3];
            pstr2 = strstr(pstr1 ,")");
            *pstr2 = 0;
            fprintf(fpto,"typedef %s *P%s;\n",pstr1,pstr1 );
         }
      } 
      if(!is)
      { fputs(str,fpto);
      }   
      n++; 
   } 
   fclose(fp);
   fclose(fpto);
   printf("n=%i,n1=%i\n",n,n1);  
}