/* os2mesaDive.c  */
#include <stdio.h>
#include <stdlib.h>
#define INCL_DEV
#include "WarpGL.h"
#include "GL/os2mesa.h"

#include "macros.h"
#include "context.h"
#include "dd.h"
#include "matrix.h"
#include "depth.h"

#define  _MEERROR_H_
#include <mmioos2.h>                   /* It is from MMPM toolkit           */
#include <dive.h>
#include <fourcc.h>

#include "os2mesadef.h"
                #include "glutint.h"

#define POKA 0

#include "malloc.h"

BOOL  DiveFlush(PWMC pwc);
int DivePMInit(WMesaContext c);

//DIVE_CAPS DiveCaps = {0};
//FOURCC    fccFormats[100] = {0};        /* Color format code */


//
// Blit memory DC to screen DC
//
BOOL  DiveFlush(PWMC pwc)
{
    BOOL  bRet = 0;
    int   dwErr = 0,ulWidth,rc;
    POINTL aptl[4];
    int cx,cy, cx0,cy0;
    int i,csum=0;
    char *pstr;

    SETUP_BLITTER SetupBlitter;      /* structure for DiveSetupBlitter       */
 RECTL     rCtls[52];
 FOURCC  fcc, fccscr;
extern GLUTwindow * __glutCurrentWindow;
// ULONG     ulNumRcls;
/*********************************/
/* обнуляем все, что можно */
    pstr = (char *)&SetupBlitter;
   for(i=0; i < sizeof(SetupBlitter); i++,pstr++) *pstr = 0;
    pstr = (char *)&rCtls;
   for(i=0; i < sizeof(PRECTL); i++,pstr++) *pstr = 0;
/*
  {int rc;
      rc = _heapchk();

    if (_HEAPOK != rc) {
       switch(rc) {
          case _HEAPEMPTY:
             printf("The heap has not been initialized.");
             break;
          case _HEAPBADNODE:
             printf("A memory node is corrupted or the heap is damaged.");
             break;
          case _HEAPBADBEGIN:
             printf("The heap specified is not valid.");
             break;
       }
       exit(rc);
    }
  }
*/
/********************************/
   cx = pwc->width ;
   cy = pwc->height;
   cx0 = cx;
   cy0 = cy;

    ulWidth = pwc->bmi.cBitCount * (pwc->bmi.cx+7)/8;
    switch(pwc->bmi.cBitCount)
    {   case 32:
             fcc =  FOURCC_RGB4;
             fccscr = FOURCC_RGB4;

//             fccscr = FOURCC_SCRN;
//             fcc =  FOURCC_BGR4;
//             fccscr = FOURCC_BGR4;
           break;
        case 24:
             fcc =  FOURCC_BGR3;
//           fccscr = FOURCC_RGB3; //?? FOURCC_SCRN;
             fccscr = FOURCC_SCRN; // Matrox 256, 64K

//             if(pwc->useDive&0x100)
//                             cx = cx0 *2/3; /* Matrox & Dive ??? 64K */
//             else if(pwc->useDive&0x200)
//                             cx = cx0 /3;   /* Matrox & Dive ??? 256 */

           break;
        default:
             fcc =  FOURCC_LUT8;
             fccscr =  FOURCC_SCRN;
    }
//    ulWidth = 4 * pwc->bmi.cy;

    ulWidth = 0;  // ????
    if(pwc->bDiveHaveBuffer)
    {   rc = DiveFreeImageBuffer ( pwc->hDive, pwc->ulDiveBufferNumber );
        pwc->bDiveHaveBuffer = FALSE;
    }
    pwc->ulDiveBufferNumber = 0;
// Associate the canvas with a DIVE handle
   rc = DiveAllocImageBuffer (pwc->hDive, &pwc->ulDiveBufferNumber, fcc,
                          cx0, cy0,
                          ulWidth,
                          pwc->memb.pBmpBuffer);
  if(rc)
  {  printf("DiveAllocImageBuffer rc=%i\n",rc);
     rc = DiveFreeImageBuffer ( pwc->hDive, pwc->ulDiveBufferNumber );
     pwc->bDiveHaveBuffer = FALSE;
     return FALSE;
  }


//DiveEndImageBufferAccess
//DiveBeginImageBufferAccess(pwc->hDive,pwc->ulDiveBufferNumber
pwc->bDiveHaveBuffer = TRUE;

   SetupBlitter.ulStructLen = sizeof ( SETUP_BLITTER );
   SetupBlitter.fInvert      = TRUE;
   SetupBlitter.fccSrcColorFormat = fcc;

       SetupBlitter.ulSrcWidth  = cx;
       SetupBlitter.ulSrcHeight = cy;
   SetupBlitter.ulSrcPosX = 0;
   SetupBlitter.ulSrcPosY = 0;

   SetupBlitter.ulDitherType = 0;
   if(pwc->useDive&0x200)
                 SetupBlitter.ulDitherType = 1; //1;
   SetupBlitter.fccDstColorFormat = fccscr;

   SetupBlitter.ulDstWidth  = cx;
   SetupBlitter.ulDstHeight = cy;
    SetupBlitter.lDstPosX =  0;
    SetupBlitter.lDstPosY =  0;

   SetupBlitter.lScreenPosX = pwc->xDiveScr;
   SetupBlitter.lScreenPosY = pwc->yDiveScr;

   rCtls[0].xLeft = 0;
   rCtls[0].yBottom = 0;
   rCtls[0].xRight  = cx;
   rCtls[0].yTop    = cy;
//   SetupBlitter.ulNumDstRects = ulNumRcls;
   SetupBlitter.ulNumDstRects = 1;

   SetupBlitter.pVisDstRects = rCtls;
/* считаем контpольную сумму SetupBlitter */
/* pstr = (char *)&SetupBlitter;
   for(i=0; i < sizeof(SetupBlitter)- sizeof(PRECTL); i++,pstr++)
   {   csum += ((int) *pstr);
   }
   printf("SetutBlitter CSum =%i\n",csum); fflush(stdout);
*/
   rc = DiveSetupBlitter ( pwc->hDive, &SetupBlitter );
   if(rc)
   {    printf("SetutBlitterrc=%i\n",rc);
     rc = DiveFreeImageBuffer ( pwc->hDive, pwc->ulDiveBufferNumber );
     pwc->bDiveHaveBuffer = FALSE;
        return FALSE;
   }
/**************************************/
    rc = DiveBeginImageBufferAccess ( pwc->hDive,
                                     pwc->ulDiveBufferNumber,
                                     &pwc->memb.pBmpBuffer,
                                     (PULONG) &ulWidth,
                                     (PULONG) &cy0 );
if(rc)
  {  printf("DiveBeginImageBufferAccess rc=%i\n",rc);
     rc = DiveFreeImageBuffer ( pwc->hDive, pwc->ulDiveBufferNumber );
     pwc->bDiveHaveBuffer = FALSE;
     return FALSE;
  }

/************************************/


// blit Dive buffer to screen.


   if(!rc)
   {
       rc = DiveBlitImage (pwc->hDive,
                      pwc->ulDiveBufferNumber,
                      DIVE_BUFFER_SCREEN );
      if(rc)
      {     printf("Blitrc=%i,bn=%i",rc,pwc->ulDiveBufferNumber);
           rc = DiveFreeImageBuffer ( pwc->hDive, pwc->ulDiveBufferNumber );
            printf("DiveFreeImageBuffer rc=%i",rc);
           pwc->bDiveHaveBuffer = FALSE;
      } else {
        rc = DiveEndImageBufferAccess (pwc->hDive, pwc->ulDiveBufferNumber );
      }
   }

  return TRUE;
}

int DivePMInit(WMesaContext c)
{
   c->ulDiveBufferNumber = 0;
   c->bDiveHaveBuffer = FALSE;
   c->DiveCaps.pFormatData = c->fccFormats;
   c->DiveCaps.ulFormatLength = 100;
   c->DiveCaps.ulStructLen = sizeof(c->DiveCaps /* DIVE_CAPS */);
   c->xDiveScr = 0;
   c->yDiveScr = 0;
   if ( DiveQueryCaps ( &c->DiveCaps, DIVE_BUFFER_SCREEN ))
   {
          /*  The sample program can not run on this system environment. */
      return  1;
   }

/*
{   int i;
    FILE *fp;
    char *p, str[10];
    fp=fopen("testDive.txt","w");

      fprintf(fp,"ulStructLen           = %i\n",c->DiveCaps.ulStructLen);
      fprintf(fp,"ulPlaneCount          = %i\n",c->DiveCaps.ulPlaneCount);
      fprintf(fp,"fScreenDirect         = %i\n",c->DiveCaps.fScreenDirect);
      fprintf(fp,"fBankSwitched         = %i\n",c->DiveCaps.fBankSwitched);
      fprintf(fp,"ulDepth               = %i\n",c->DiveCaps.ulDepth);
      fprintf(fp,"ulHorizontalResolution= %i\n",c->DiveCaps.ulHorizontalResolution);
      fprintf(fp,"ulVerticalResolution  = %i\n",c->DiveCaps.ulVerticalResolution);
      fprintf(fp,"ulScanLineBytes       = %i\n",c->DiveCaps.ulScanLineBytes);
      p = &(c->DiveCaps.fccColorEncoding);
      str[0] = p[0], str[1] = p[1], str[2] = p[2], str[3] = p[3];
      str[4] = 0;
      fprintf(fp,"fccColorEncoding      = %s\n",str);

      fprintf(fp,"ulApertureSize        = %2i\n",c->DiveCaps.ulApertureSize);
      fprintf(fp,"ulInputFormats        = %2i\n",c->DiveCaps.ulInputFormats);
      fprintf(fp,"ulOutputFormats       = %2i\n",c->DiveCaps.ulOutputFormats);
      fprintf(fp,"ulFormatLength        = %2i\n",c->DiveCaps.ulFormatLength);


    fclose(fp);
 }
*/
//  PVOID      pFormatData;             /*  Pointer to color-format buffer FOURCCs. */

   if ( c->DiveCaps.ulDepth < 8 )
      return  2;
   if ( DiveOpen ( &(c->hDive), FALSE, 0 ) )
   {
       return 3;
   }
   printf("Dive open Ok\n"); fflush(stdout);


   return 0;
}

